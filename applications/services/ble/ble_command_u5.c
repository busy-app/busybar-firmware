#include "ble_command.h"
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

void ble_command_handler_init(Ble* instance, BleIntercomFrameGeneric* frame) {
    if(frame->header.frame_type == BleIntercomFrameTypeResponse) {
        BLE_LOG_D("BleCommandInit response");

        for(size_t i = 0; i < BLE_SERVICES_COUNT; i++) {
            ble_service_enqueue_init(instance->services[i]);
        }

        ///TODO: need to wait untill all services will call on_state changed callback
        ///And after that change state to Ready and release message
        ///But for now let's keep it as it is.
        instance->state = BleServiceStateInitialization;
        instance->current_message->result = true;
        api_lock_unlock(instance->current_message->lock);
    } else {
        BLE_LOG_D("BleCommandInit request");

        frame->header.frame_type = BleIntercomFrameTypeRequest;
        size_t frame_size = sizeof(BleIntercomFrameHeader) + frame->header.data_size;
        size_t tx = intercom_tx(instance->intercom, IntercomChannelBle, frame, frame_size, 100);
        furi_assert(tx == frame_size);
    }
}

void ble_command_handler_enable(Ble* instance, BleIntercomFrameGeneric* frame) {
    if(frame->header.frame_type == BleIntercomFrameTypeResponse) {
        BLE_LOG_D("BleCommandEnable response");
        instance->current_message->result = true;
        api_lock_unlock(instance->current_message->lock);
        ble_http_repeater_start(instance);
    } else {
        BLE_LOG_D("BleCommandEnable request");

        frame->header.frame_type = BleIntercomFrameTypeRequest;
        size_t frame_size = sizeof(BleIntercomFrameHeader) + frame->header.data_size;
        size_t tx = intercom_tx(instance->intercom, IntercomChannelBle, frame, frame_size, 100);
        furi_assert(tx == frame_size);
    }
}

void ble_command_handler_disable(Ble* instance, BleIntercomFrameGeneric* frame) {
    if(frame->header.frame_type == BleIntercomFrameTypeResponse) {
        BLE_LOG_D("BleCommandDisable response");
        instance->current_message->result = true;
        api_lock_unlock(instance->current_message->lock);
        ble_http_repeater_stop();
    } else {
        BLE_LOG_D("BleCommandDisable request");

        frame->header.frame_type = BleIntercomFrameTypeRequest;
        size_t frame_size = sizeof(BleIntercomFrameHeader) + frame->header.data_size;
        size_t tx = intercom_tx(instance->intercom, IntercomChannelBle, frame, frame_size, 100);
        furi_assert(tx == frame_size);
    }
}

void ble_command_handler_get_state(Ble* instance, BleIntercomFrameStatus* frame) {
    if(frame->header.frame_type == BleIntercomFrameTypeRequest) {
        BLE_LOG_D("GetStatus request");

        frame->header.frame_type = BleIntercomFrameTypeRequest;
        frame->header.command = BleCommandGetState;
        frame->header.data_size = 0;

        size_t frame_size = sizeof(BleIntercomFrameHeader);
        size_t tx = intercom_tx(instance->intercom, IntercomChannelBle, frame, frame_size, 100);
        furi_assert(tx == frame_size);
    } else {
        BLE_LOG_D("GetStatus response");
        BLE_LOG_D("Local state: %d remote state: %d", instance->state, frame->state);

        ///TODO: Temporary fix, in order to unblock ble_start()
        instance->current_message->result = true;
        BleServiceState* state = (BleServiceState*)instance->current_message->data;
        *state = instance->state;
        api_lock_unlock(instance->current_message->lock);
    }
}
