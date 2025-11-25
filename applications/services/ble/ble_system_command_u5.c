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
        return (BleIntercomFrameGeneric*)&instance->current_message->header;
    else {
        BLE_LOG_W("Unknown event");
        return NULL;
    }
}

void ble_set_service_post_process_callback(Ble* ble, BleServicePostProcessCallback callback) {
    furi_assert(ble);
    if(callback) BLE_LOG_I("Subscribe for post process");
    ble->service_post_process_callback = callback;
}

static void ble_on_name_change_callback(const void* message, void* context) {
    UNUSED(message);
    Ble* instance = context;
    furi_event_loop_set_custom_event(instance->event_loop, BleEventTypeDeviceNameChanged);
}

static void ble_subscribe_on_name_change(Ble* instance) {
    DeviceName* name_record = furi_record_open(RECORD_DEVICE_NAME);
    FuriPubSub* pubsub = device_name_get_on_change_pub_sub(name_record);
    furi_pubsub_subscribe(pubsub, ble_on_name_change_callback, instance);
    furi_record_close(RECORD_DEVICE_NAME);
}

static void ble_get_name_from_record(FuriString* output) {
    DeviceName* name_record = furi_record_open(RECORD_DEVICE_NAME);
    device_name_get(name_record, output);
    furi_record_close(RECORD_DEVICE_NAME);
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
        instance->current_message->result = true;
        ble_subscribe_on_name_change(instance);

        ble_set_service_post_process_callback(instance, NULL);
        api_lock_unlock(instance->current_message_api_lock);
    }
}

static bool ble_command_init_request(BleIntercomFrameGeneric* frame, void* context) {
    BLE_LOG_D("BleCommandInit request");

    return ble_command_request_process(frame, context);
}

static bool ble_command_init_response(BleIntercomFrameGeneric* frame, void* context) {
    UNUSED(frame);
    BLE_LOG_D("BleCommandInit response");
    Ble* instance = context;

    ble_set_service_post_process_callback(instance, ble_service_init_wait_callback);

    for(size_t i = 0; i < BLE_SERVICES_COUNT; i++) {
        ble_service_enqueue_init(instance->services[i]);
    }

    return true;
}

static bool ble_command_enable_request(BleIntercomFrameGeneric* frame, void* context) {
    BLE_LOG_D("BleCommandEnable request");
    return ble_command_request_process(frame, context);
}

static bool ble_command_enable_response(BleIntercomFrameGeneric* frame, void* context) {
    UNUSED(frame);
    BLE_LOG_D("BleCommandEnable response");
    Ble* instance = context;

    const BleIntercomResponse* response = (const BleIntercomResponse*)frame->data;
    instance->current_message->result = response->result;
    instance->state = response->result ? BleServiceStateAdvertising : BleServiceStateError;

    api_lock_unlock(instance->current_message_api_lock);
    ble_http_repeater_start(instance);
    return true;
}

static bool ble_command_disable_request(BleIntercomFrameGeneric* frame, void* context) {
    BLE_LOG_D("BleCommandDisable request");
    return ble_command_request_process(frame, context);
}

static bool ble_command_disable_response(BleIntercomFrameGeneric* frame, void* context) {
    UNUSED(frame);
    BLE_LOG_D("BleCommandDisable response");
    Ble* instance = context;

    const BleIntercomResponse* response = (const BleIntercomResponse*)frame->data;
    instance->current_message->result = response->result;
    instance->state = response->result ? BleServiceStateReady : BleServiceStateError;

    api_lock_unlock(instance->current_message_api_lock);
    ble_http_repeater_stop();
    return true;
}

static bool ble_command_get_state_request(BleIntercomFrameGeneric* frame, void* context) {
    BLE_LOG_D("BleCommandGetState request");
    frame->header.command = BleCommandGetState;
    return ble_command_request_process(frame, context);
}

static bool ble_command_get_state_response(BleIntercomFrameGeneric* frame, void* context) {
    BLE_LOG_D("BleCommandGetState response");
    Ble* instance = context;

    const BleIntercomResponse* response = (BleIntercomResponse*)frame->data;
    ///TODO: create structure type for state response and use it instead of data[0]
    const BleServiceState remote_state = response->data[0];

    bool result = false;
    do {
        if(!response->result) {
            instance->state = BleServiceStateError;
            BLE_LOG_W("Failed to get state from remote");
            break;
        }

        if(instance->state == BleServiceStateError) {
            instance->state = BleServiceStateError;
            BLE_LOG_W("Local service error");
            break;
        }

        if(remote_state == BleServiceStateError) {
            instance->state = BleServiceStateError;
            BLE_LOG_W("Remote service error");
            break;
        }

        result = true;
    } while(false);

    instance->current_message->result = result;
    BleServiceState* state = (BleServiceState*)instance->current_message->data;
    *state = instance->state;
    api_lock_unlock(instance->current_message_api_lock);
    return true;
}

static bool ble_command_forget_pairing_request(BleIntercomFrameGeneric* frame, void* context) {
    BLE_LOG_D("BleCommandForgetPairing request");
    frame->header.command = BleCommandForgetPairing;
    return ble_command_request_process(frame, context);
}

static bool ble_command_forget_pairing_response(BleIntercomFrameGeneric* frame, void* context) {
    BLE_LOG_D("BleCommandForgetPairing response");
    Ble* instance = context;

    instance->current_message->result = frame->data[0];
    api_lock_unlock(instance->current_message_api_lock);
    return true;
}

static bool ble_command_get_pairing_request(BleIntercomFrameGeneric* frame, void* context) {
    BLE_LOG_D("BleCommandGetPairing request");
    frame->header.command = BleCommandGetPairing;
    return ble_command_request_process(frame, context);
}

static bool ble_command_get_pairing_response(BleIntercomFrameGeneric* frame, void* context) {
    BLE_LOG_D("BleCommandGetPairing response");
    Ble* instance = context;

    const BleIntercomResponse* response = (BleIntercomResponse*)frame->data;
    const BlePairingState response_pairing = *((BlePairingState*)response->data);
    furi_assert(response_pairing < BlePairingStateCount);

    instance->current_message->result = response->result;
    BlePairingState* result_pairing = (BlePairingState*)instance->current_message->data;
    *result_pairing = response_pairing;
    api_lock_unlock(instance->current_message_api_lock);
    return true;
}

static bool ble_command_set_device_name_request(BleIntercomFrameGeneric* frame, void* context) {
    BLE_LOG_D("BleCommandSetDeviceName request");

    FuriString* name = furi_string_alloc();
    ble_get_name_from_record(name);

    size_t name_size = furi_string_size(name) + 1;
    const size_t new_msg_size = sizeof(BleIntercomFrameHeader) + name_size;

    BleIntercomFrameGeneric* name_frame = malloc(new_msg_size);

    memcpy(&name_frame->header, &frame->header, sizeof(BleIntercomFrameHeader));
    name_frame->header.data_size = name_size;
    memcpy(name_frame->data, furi_string_get_cstr(name), name_size);
    name_frame->data[name_size] = 0;

    bool result = ble_command_request_process(name_frame, context);
    free(name_frame);
    free(name);
    return result;
}

static bool ble_command_set_device_name_response(BleIntercomFrameGeneric* frame, void* context) {
    UNUSED(frame);
    UNUSED(context);
    BLE_LOG_D("BleCommandSetDeviceName response");
    Ble* instance = context;
    instance->current_message->result = frame->data[0];

    const FuriThreadId owner_id = furi_mutex_get_owner(instance->current_message_lock);
    const FuriThreadId current_id = furi_thread_get_current_id();
    if(owner_id == current_id) {
        furi_mutex_release(instance->current_message_lock);
    } else {
        api_lock_unlock(instance->current_message_api_lock);
    }
    return true;
}

static bool ble_command_connection_updated_request(BleIntercomFrameGeneric* frame, void* context) {
    BLE_LOG_D("ble_command_connection_updated_request");

    Ble* instance = context;

    const bool connected = frame->data[0];
    const uint8_t* addr = &frame->data[1];
    instance->state = connected ? BleServiceStateConnected : BleServiceStateAdvertising;
    memcpy(instance->remote_device_address, addr, BLE_REMOTE_DEVICE_ADDRESS_SIZE);

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
    [BleCommandGetState] =
        {
            .request = ble_command_get_state_request,
            .response = ble_command_get_state_response,
        },
    [BleCommandForgetPairing] =
        {
            .request = ble_command_forget_pairing_request,
            .response = ble_command_forget_pairing_response,
        },
    [BleCommandGetPairing] =
        {

            .request = ble_command_get_pairing_request,
            .response = ble_command_get_pairing_response,
        },
    [BleCommandSetDeviceName] =
        {
            .request = ble_command_set_device_name_request,
            .response = ble_command_set_device_name_response,
        },
    [BleCommandConnectionUpdated] =
        {
            .request = ble_command_connection_updated_request,
        },
};

void ble_invoke_retry_command_on_internal_event(
    Ble* instance,
    BleSystemCommand command,
    BleEventType retry_event,
    uint32_t retry_timeout) {
    if(furi_mutex_acquire(instance->current_message_lock, retry_timeout) == FuriStatusOk) {
        BleIntercomFrameHeader* header = &instance->current_message->header;
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
