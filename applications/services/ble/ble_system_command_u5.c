#include "ble_command_engine.h"
#include "ble_system_command.h"
#include "http/ble_http_repeater.h"

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

static bool ble_command_init_request(BleIntercomFrameGeneric* frame, void* context) {
    BLE_LOG_D("BleCommandInit request");

    return ble_command_request_process(frame, context);
}

static bool ble_command_init_response(BleIntercomFrameGeneric* frame, void* context) {
    UNUSED(frame);
    BLE_LOG_D("BleCommandInit response");
    Ble* instance = context;

    for(size_t i = 0; i < BLE_SERVICES_COUNT; i++) {
        ble_service_enqueue_init(instance->services[i]);
    }

    ///TODO: need to wait untill all services will call on_state changed callback
    ///And after that change state to Ready and release message
    ///But for now let's keep it as it is.
    instance->state = BleServiceStateReady;
    instance->current_message->result = true;
    api_lock_unlock(instance->current_message_api_lock);
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

    instance->current_message->result = true;
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

    instance->current_message->result = true;
    api_lock_unlock(instance->current_message_api_lock);
    ble_http_repeater_stop();
    return true;
}

static bool ble_command_get_state_request(BleIntercomFrameGeneric* frame, void* context) {
    BLE_LOG_D("BleCommandDisable request");
    frame->header.command = BleCommandGetState;
    return ble_command_request_process(frame, context);
}

static bool ble_command_get_state_response(BleIntercomFrameGeneric* frame, void* context) {
    UNUSED(frame);

    BLE_LOG_D("BleCommandDisable response");
    Ble* instance = context;
    ///TODO: this logic must be improved
    BLE_LOG_D("Local state: %d remote state: %d", instance->state, frame->data[0]);

    ///TODO: Temporary fix, in order to unblock ble_start()
    instance->current_message->result = true;
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
