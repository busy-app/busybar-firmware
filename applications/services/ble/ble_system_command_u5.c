#include "settings/settings.h"
#include "ble_command_engine.h"
#include "ble_system_command.h"
#include "ble/service/ble_service.h"
#include "http/ble_http_repeater.h"
#include "streaming/ble_streaming.h"
#include "device_name/device_name.h"

#define TAG "BLE_U5"

BleIntercomFrameGeneric*
    ble_command_extract_frame(Ble* instance, BleCommandEngineExtractFrameSource source) {
    if(source == BleCommandEngineExtractFrameSourceIntercomBuffer)
        return &instance->mailbox;
    else if(source == BleCommandEngineExtractFrameSourceCommandBuffer)
        return (BleIntercomFrameGeneric*)&instance->current_command->header;
    else {
        furi_crash("Unknown source");
    }
}

void ble_command_unblock_with_result(Ble* instance, bool result) {
    if(api_lock_is_locked(instance->current_command_api_lock)) {
        instance->current_command->header.result = result;
        api_lock_unlock(instance->current_command_api_lock);
    }

    const FuriThreadId owner_id = furi_mutex_get_owner(instance->current_command_lock);
    const FuriThreadId current_id = furi_thread_get_current_id();
    if(owner_id == current_id) {
        furi_mutex_release(instance->current_command_lock);
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
    name_frame->header.data_size = name_size + 1;
    memcpy(name_frame->data, furi_string_get_cstr(name), name_size);

    bool result = ble_command_request_process(name_frame, context);
    free(name_frame);
    free(name);
    return result;
}

static bool ble_command_set_device_name_response(BleIntercomFrameGeneric* frame, void* context) {
    BLE_LOG_D("BleCommandSetDeviceName response");
    Ble* instance = context;
    ble_command_unblock_with_result(instance, frame->header.result);
    return true;
}

static void ble_service_init_wait_callback(BleServiceObject* service, bool result, void* ctx) {
    UNUSED(service);
    UNUSED(result);
    BLE_LOG_D("ble_service_init_wait_callback");
    Ble* instance = ctx;

    uint8_t total_ready;
    for(total_ready = 0; total_ready < BleServiceIndexCount; total_ready++) {
        if(!ble_service_is_ready(instance->services[total_ready])) break;
    }

    if(total_ready == BleServiceIndexCount) {
        instance->status = BleServiceStatusReady;
        ble_subscribe_on_name_change(instance);

        ble_set_service_post_process_callback(instance, NULL);

        ble_restore_state_on_start(instance);
        ble_command_unblock_with_result(instance, true);
    }
}

static bool ble_command_init_request(BleIntercomFrameGeneric* frame, void* context) {
    BLE_LOG_D("BleCommandInit request");

    Ble* instance = context;
    const BleServiceStatus state = instance->status;

    bool result = false;
    if(state == BleServiceStatusReset) {
        result = ble_command_request_process(frame, context);
    } else if(
        state == BleServiceStatusReady || state == BleServiceStatusAdvertising ||
        state == BleServiceStatusConnected || state == BleServiceStatusConnectable) {
        ble_command_unblock_with_result(instance, true);
    } else if(state == BleServiceStatusError) {
        BLE_LOG_W("No init, error occurred");
        ble_command_unblock_with_result(instance, false);
    }

    return result;
}

static bool ble_command_init_response(BleIntercomFrameGeneric* frame, void* context) {
    BLE_LOG_D("BleCommandInit response");
    Ble* instance = context;

    ble_set_service_post_process_callback(instance, ble_service_init_wait_callback);

    for(size_t i = 0; i < BleServiceIndexCount; i++) {
        ble_service_enqueue_init(instance->services[i]);
    }

    return ble_command_set_device_name_request(frame, instance);
}

static bool ble_command_deinit_request(BleIntercomFrameGeneric* frame, void* context) {
    BLE_LOG_D("BleCommandDeinit request");
    return ble_command_deinit_process(frame, context);
}

static bool ble_command_enable_request(BleIntercomFrameGeneric* frame, void* context) {
    BLE_LOG_D("BleCommandEnable request");
    Ble* instance = context;
    const BleServiceStatus state = instance->status;

    ///TODO: replace this with some preprocess function which will check if command is allowed in this state
    bool result = false;
    if(state == BleServiceStatusReady) {
        result = ble_command_request_process(frame, context);
    } else if(
        state == BleServiceStatusAdvertising || state == BleServiceStatusConnected ||
        state == BleServiceStatusConnectable) {
        ble_command_unblock_with_result(instance, true);
    } else if(state == BleServiceStatusError) {
        BLE_LOG_W("No enable, error occurred");
        ble_command_unblock_with_result(instance, false);
    }

    return result;
}

static bool ble_command_enable_response(BleIntercomFrameGeneric* frame, void* context) {
    BLE_LOG_D("BleCommandEnable response");
    Ble* instance = context;

    if(frame->header.result && frame->header.data_size == sizeof(BleServiceStatus)) {
        BleServiceStatus* resp_status = (BleServiceStatus*)frame->data;
        instance->status = *resp_status;
        ble_save_enabled_state(true);
    }

    ble_command_unblock_with_result(instance, frame->header.result);
    ble_http_repeater_start(instance);

    BleState status = {
        .status = instance->status,
    };
    furi_pubsub_publish(instance->on_status_change, &status);
    return true;
}

static bool ble_command_disable_request(BleIntercomFrameGeneric* frame, void* context) {
    BLE_LOG_D("BleCommandDisable request");
    Ble* instance = context;
    const BleServiceStatus state = instance->status;

    ///TODO: replace this with some preprocess function which will check if command is allowed in this state
    bool result = false;
    if(state == BleServiceStatusError) {
        BLE_LOG_W("No disable, error occurred");
        ble_command_unblock_with_result(instance, result);
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
    instance->status = frame->header.result ? BleServiceStatusReady : BleServiceStatusError;

    ble_save_enabled_state(false);

    ble_command_unblock_with_result(instance, frame->header.result);
    ble_http_repeater_stop();

    BleState status = {
        .status = instance->status,
    };
    furi_pubsub_publish(instance->on_status_change, &status);
    return true;
}

static bool ble_command_get_status_request(BleIntercomFrameGeneric* frame, void* context) {
    BLE_LOG_D("BleCommandGetStatus request");
    Ble* instance = context;
    frame->header.command = BleCommandGetStatus;
    const BleServiceStatus state = instance->status;
    bool result = false;
    if(state == BleServiceStatusError) {
        BLE_LOG_W("No status, error occurred");

        ble_command_unblock_with_result(instance, result);
    } else {
        result = ble_command_request_process(frame, context);
    }
    return result;
}

static bool ble_command_get_status_response(BleIntercomFrameGeneric* frame, void* context) {
    BLE_LOG_D("BleCommandGetStatus response");
    Ble* instance = context;

    const BleState* response = (BleState*)frame->data;
    bool result = false;
    do {
        if(!frame->header.result) {
            instance->status = BleServiceStatusError;
            BLE_LOG_W("Failed to get state from remote");
            break;
        }

        if(instance->status == BleServiceStatusError) {
            BLE_LOG_W("Local service error");
            break;
        }

        if(response->status == BleServiceStatusError) {
            instance->status = BleServiceStatusError;
            BLE_LOG_W("Remote service error");
            break;
        }

        result = true;
    } while(false);

    memcpy(instance->current_command->data, response, sizeof(BleState));
    ble_command_unblock_with_result(instance, result);
    return true;
}

static bool ble_command_set_status_request(BleIntercomFrameGeneric* frame, void* context) {
    BLE_LOG_D("ble_command_set_status_request");

    Ble* instance = context;

    const BleState* response = (BleState*)frame->data;
    bool result = false;
    do {
        if(!frame->header.result) {
            instance->status = BleServiceStatusError;
            BLE_LOG_W("Failed to get state from remote");
            break;
        }

        if(instance->status == BleServiceStatusError) {
            BLE_LOG_W("Local service error");
            break;
        }

        if(response->status == BleServiceStatusError) {
            instance->status = BleServiceStatusError;
            BLE_LOG_W("Remote service error");
            break;
        }

        instance->status = response->status;
        result = true;
    } while(false);

    memcpy(
        instance->remote_device_address,
        response->remote_device_address,
        BLE_REMOTE_DEVICE_ADDRESS_STRING_SIZE);

    furi_pubsub_publish(instance->on_status_change, (void*)response);

    ble_streaming_update(instance->streaming, instance->status);

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

    ble_command_unblock_with_result(instance, frame->header.result);
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
    [BleCommandDeinit] =
        {
            .request = ble_command_deinit_request,
            .response = NULL,
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
        furi_event_loop_set_custom_event(instance->event_loop, BleEventTypeApiCommand);
    } else {
        BLE_LOG_W("Invoke retry");
        furi_event_loop_set_custom_event(instance->event_loop, retry_event);
    }
}
