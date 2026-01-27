#include "settings/settings.h"
#include "ble_command_engine.h"
#include "ble_system_command.h"
#include "ble/service/ble_service.h"
#include "http/ble_http_repeater.h"
#include "device_name/device_name.h"

#define TAG "BLE_U5"

BleIntercomFrameGeneric* ble_command_preprocess(Ble* instance, uint32_t events) {
    if(events & BleEventTypeFrameReceived)
        return &instance->mailbox;
    else if(events & BleEventTypeIncomingMessage)
        return (BleIntercomFrameGeneric*)&instance->current_command->header;
    else {
        BLE_LOG_W("Unknown event");
        return NULL;
    }
}

static void ble_restore_state_on_start(const Ble* instance) {
    BleSettings settings;
    ble_settings_load(&settings);
    if(settings.enabled) {
        furi_event_loop_set_custom_event(instance->event_loop, BleEventTypeEnableOnStart);
    }
}

static void ble_save_enabled_state(bool enabled) {
    BleSettings settings;
    ble_settings_load(&settings);
    settings.enabled = enabled;
    ble_settings_save(&settings);
}

static void ble_on_name_change_callback(const void* message, void* context) {
    UNUSED(message);
    Ble* instance = context;
    furi_event_loop_set_custom_event(instance->event_loop, BleEventTypeDeviceNameChanged);
}

static void ble_subscribe_on_name_change(Ble* instance) {
    DeviceName* name_record = furi_record_open(RECORD_DEVICE_NAME);
    FuriPubSub* pubsub = device_name_get_pubsub(name_record);
    furi_pubsub_subscribe(pubsub, ble_on_name_change_callback, instance);
    furi_record_close(RECORD_DEVICE_NAME);
}

static void ble_get_name_from_record(FuriString* output) {
    DeviceName* name_record = furi_record_open(RECORD_DEVICE_NAME);
    device_name_get(name_record, output);
    furi_record_close(RECORD_DEVICE_NAME);
}

static bool ble_command_set_device_name_request(BleIntercomFrameGeneric* frame, void* context) {
    BLE_LOG_D("BleCommandSetDeviceName request");

    FuriString* name = furi_string_alloc();
    ble_get_name_from_record(name);

    size_t name_size = furi_string_size(name);
    size_t new_msg_size = sizeof(BleIntercomFrameHeader) + name_size + 1;

    BleIntercomFrameGeneric* name_frame = malloc(new_msg_size);
    memcpy(&name_frame->header, &frame->header, sizeof(BleIntercomFrameHeader));
    name_frame->header.command = BleCommandSetDeviceName;
    name_frame->header.data_size = name_size;
    memcpy(name_frame->data, furi_string_get_cstr(name), name_size);

    bool result = ble_command_request_process(name_frame, context);
    free(name_frame);
    free(name);
    return result;
}

static bool ble_command_set_device_name_response(BleIntercomFrameGeneric* frame, void* context) {
    BLE_LOG_D("BleCommandSetDeviceName response");
    Ble* instance = context;
    instance->current_command->header.result = frame->header.result;

    const FuriThreadId owner_id = furi_mutex_get_owner(instance->current_command_lock);
    const FuriThreadId current_id = furi_thread_get_current_id();
    if(owner_id == current_id) {
        furi_mutex_release(instance->current_command_lock);
    }
    return true;
}

static void ble_service_init_wait_callback(BleServiceObject* service, bool result, void* ctx) {
    UNUSED(service);
    UNUSED(result);
    BLE_LOG_D("ble_service_init_wait_callback");
    Ble* instance = ctx;

    uint8_t total_ready;
    for(total_ready = 0; total_ready < BLE_SERVICES_COUNT; total_ready++) {
        if(!ble_service_is_ready(instance->services[total_ready])) break;
    }

    if(total_ready == BLE_SERVICES_COUNT) {
        instance->state = BleServiceStateReady;
        instance->current_command->header.result = true;
        ble_subscribe_on_name_change(instance);

        ble_set_service_post_process_callback(instance, NULL);

        ble_restore_state_on_start(instance);

        api_lock_unlock(instance->current_command_api_lock);
    }
}

static bool ble_command_init_request(BleIntercomFrameGeneric* frame, void* context) {
    BLE_LOG_D("BleCommandInit request");

    Ble* instance = context;
    const BleServiceState state = instance->state;

    ///TODO: replace this with some preprocess function which will check if command is allowed in this state
    bool result = false;
    if(state == BleServiceStateReset) {
        result = ble_command_request_process(frame, context);
    } else if(
        state == BleServiceStateReady || state == BleServiceStateAdvertising ||
        state == BleServiceStateConnected) {
        instance->current_command->header.result = true;
        api_lock_unlock(instance->current_command_api_lock);
    } else if(state == BleServiceStateError) {
        BLE_LOG_W("No init, error occured");

        instance->current_command->header.result = false;
        api_lock_unlock(instance->current_command_api_lock);
    }

    return result;
}

static bool ble_command_init_response(BleIntercomFrameGeneric* frame, void* context) {
    UNUSED(frame);
    BLE_LOG_D("BleCommandInit response");
    Ble* instance = context;

    ble_set_service_post_process_callback(instance, ble_service_init_wait_callback);

    for(size_t i = 0; i < BLE_SERVICES_COUNT; i++) {
        ble_service_enqueue_init(instance->services[i]);
    }

    return ble_command_set_device_name_request(frame, instance);
}

static bool ble_command_enable_request(BleIntercomFrameGeneric* frame, void* context) {
    BLE_LOG_D("BleCommandEnable request");
    Ble* instance = context;
    const BleServiceState state = instance->state;

    ///TODO: replace this with some preprocess function which will check if command is allowed in this state
    bool result = false;
    if(state == BleServiceStateReady) {
        result = ble_command_request_process(frame, context);
    } else if(state == BleServiceStateAdvertising || state == BleServiceStateConnected) {
        instance->current_command->header.result = true;
        api_lock_unlock(instance->current_command_api_lock);
    } else if(state == BleServiceStateError) {
        BLE_LOG_W("No enable, error occured");

        instance->current_command->header.result = false;
        api_lock_unlock(instance->current_command_api_lock);
    }

    return result;
}

static bool ble_command_enable_response(BleIntercomFrameGeneric* frame, void* context) {
    UNUSED(frame);
    BLE_LOG_D("BleCommandEnable response");
    Ble* instance = context;

    instance->current_command->header.result = frame->header.result;
    instance->state = frame->header.result ? BleServiceStateAdvertising : BleServiceStateError;

    ble_save_enabled_state(true);

    const FuriThreadId owner_id = furi_mutex_get_owner(instance->current_command_lock);
    const FuriThreadId current_id = furi_thread_get_current_id();
    if(owner_id == current_id) {
        furi_mutex_release(instance->current_command_lock);
    }

    api_lock_unlock(instance->current_command_api_lock);
    ble_http_repeater_start(instance);
    return true;
}

static bool ble_command_disable_request(BleIntercomFrameGeneric* frame, void* context) {
    BLE_LOG_D("BleCommandDisable request");
    Ble* instance = context;
    const BleServiceState state = instance->state;

    ///TODO: replace this with some preprocess function which will check if command is allowed in this state
    bool result = false;
    if(state == BleServiceStateError) {
        BLE_LOG_W("No disable, error occured");

        instance->current_command->header.result = result;
        api_lock_unlock(instance->current_command_api_lock);
    } else {
        result = ble_command_request_process(frame, context);
    }

    return result;
}

static bool ble_command_disable_response(BleIntercomFrameGeneric* frame, void* context) {
    UNUSED(frame);
    BLE_LOG_D("BleCommandDisable response");
    Ble* instance = context;

    instance->current_command->header.result = frame->header.result;
    instance->state = frame->header.result ? BleServiceStateReady : BleServiceStateError;

    ble_save_enabled_state(false);

    api_lock_unlock(instance->current_command_api_lock);
    ble_http_repeater_stop();
    return true;
}

static bool ble_command_get_status_request(BleIntercomFrameGeneric* frame, void* context) {
    BLE_LOG_D("BleCommandGetStatus request");
    frame->header.command = BleCommandGetStatus;
    return ble_command_request_process(frame, context);
}

static bool ble_command_get_status_response(BleIntercomFrameGeneric* frame, void* context) {
    BLE_LOG_D("BleCommandGetStatus response");
    Ble* instance = context;

    const BleStatus* response = (BleStatus*)frame->data;
    bool result = false;
    do {
        if(!frame->header.result) {
            instance->state = BleServiceStateError;
            BLE_LOG_W("Failed to get state from remote");
            break;
        }

        if(instance->state == BleServiceStateError) {
            instance->state = BleServiceStateError;
            BLE_LOG_W("Local service error");
            break;
        }

        if(response->state == BleServiceStateError) {
            instance->state = BleServiceStateError;
            BLE_LOG_W("Remote service error");
            break;
        }

        result = true;
    } while(false);

    instance->current_command->header.result = result;
    memcpy(instance->current_command->data, response, sizeof(BleStatus));

    api_lock_unlock(instance->current_command_api_lock);
    return true;
}

static bool ble_command_set_status_request(BleIntercomFrameGeneric* frame, void* context) {
    BLE_LOG_D("ble_command_set_status_request");

    Ble* instance = context;

    const BleStatus* response = (BleStatus*)frame->data;
    bool result = false;
    do {
        if(!frame->header.result) {
            instance->state = BleServiceStateError;
            BLE_LOG_W("Failed to get state from remote");
            break;
        }

        if(instance->state == BleServiceStateError) {
            instance->state = BleServiceStateError;
            BLE_LOG_W("Local service error");
            break;
        }

        if(response->state == BleServiceStateError) {
            instance->state = BleServiceStateError;
            BLE_LOG_W("Remote service error");
            break;
        }

        result = true;
    } while(false);

    memcpy(
        instance->remote_device_address,
        response->remote_device_address,
        BLE_REMOTE_DEVICE_ADDRESS_STRING_SIZE);

    furi_pubsub_publish(instance->on_status_change, NULL);

    return result;
}

static bool ble_command_forget_pairing_request(BleIntercomFrameGeneric* frame, void* context) {
    BLE_LOG_D("BleCommandForgetPairing request");
    frame->header.command = BleCommandForgetPairing;
    return ble_command_request_process(frame, context);
}

static bool ble_command_forget_pairing_response(BleIntercomFrameGeneric* frame, void* context) {
    BLE_LOG_D("BleCommandForgetPairing response");
    Ble* instance = context;

    instance->current_command->header.result = frame->header.result;
    api_lock_unlock(instance->current_command_api_lock);
    return true;
}

const BleCommandItem ble_commands[BleCommandCount] = {
    [BleCommandUnknown] =
        {
            .request = NULL,
            .response = NULL,
        },
    [BleCommandInit] =
        {
            .request = ble_command_init_request,
            .response = ble_command_init_response,
        },
    [BleCommandEnable] =
        {
            .request = ble_command_enable_request,
            .response = ble_command_enable_response,
        },
    [BleCommandDisable] =
        {
            .request = ble_command_disable_request,
            .response = ble_command_disable_response,
        },
    [BleCommandGetStatus] =
        {
            .request = ble_command_get_status_request,
            .response = ble_command_get_status_response,
        },
    [BleCommandSetStatus] =
        {
            .request = ble_command_set_status_request,
        },
    [BleCommandForgetPairing] =
        {
            .request = ble_command_forget_pairing_request,
            .response = ble_command_forget_pairing_response,
        },
    [BleCommandSetDeviceName] =
        {
            .request = ble_command_set_device_name_request,
            .response = ble_command_set_device_name_response,
        },
};

void ble_invoke_retry_command_on_internal_event(
    Ble* instance,
    BleSystemCommand command,
    BleEventType retry_event,
    uint32_t retry_timeout) {
    if(furi_mutex_acquire(instance->current_command_lock, retry_timeout) == FuriStatusOk) {
        BleIntercomFrameHeader* header = &instance->current_command->header;
        header->frame_type = BleIntercomFrameTypeRequest;
        header->command = command;
        header->source = BleIntercomFrameSourceSystem;
        header->data_size = 0;
        furi_event_loop_set_custom_event(instance->event_loop, BleEventTypeIncomingMessage);
    } else {
        BLE_LOG_W("Invoke retry");
        furi_event_loop_set_custom_event(instance->event_loop, retry_event);
    }
}
