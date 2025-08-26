#include "ble_command.h"

#define TAG "BLE_U5"

BleIntercomFrameGeneric* ble_command_preprocess(Ble* instance, uint32_t events) {
    return (events == BleEventTypeFrameReceived) ?
               &instance->mailbox :
               (BleIntercomFrameGeneric*)&instance->current_message->header;
}

void ble_command_postprocess(Ble* instance, uint32_t events, bool result) {
    UNUSED(result);
    if(events == BleEventTypeFrameReceived) {
        furi_semaphore_release(instance->mailbox_lock);
    }
}

void ble_command_handler_enable(Ble* instance, BleIntercomFrameGeneric* frame) {
    if(frame->header.frame_type == BleIntercomFrameTypeResponse) {
        BLE_LOG_D("BleCommandEnable response");
        instance->current_message->result = true;
        api_lock_unlock(instance->current_message->lock);
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
    } else {
        BLE_LOG_D("BleCommandDisable request");

        frame->header.frame_type = BleIntercomFrameTypeRequest;
        size_t frame_size = sizeof(BleIntercomFrameHeader) + frame->header.data_size;
        size_t tx = intercom_tx(instance->intercom, IntercomChannelBle, frame, frame_size, 100);
        furi_assert(tx == frame_size);
    }
}

void ble_command_handler_get_status(Ble* instance, BleIntercomFrameStatus* frame) {
    if(frame->header.frame_type == BleIntercomFrameTypeRequest) {
        BLE_LOG_D("GetStatus request");

        frame->header.frame_type = BleIntercomFrameTypeRequest;
        frame->header.command = BleCommandGetStatus;
        frame->header.data_size = 0;

        size_t frame_size = sizeof(BleIntercomFrameHeader);
        size_t tx = intercom_tx(instance->intercom, IntercomChannelBle, frame, frame_size, 100);
        furi_assert(tx == frame_size);
    } else {
        if(instance->state == BleServiceStateReset && frame->state == BleServiceStateReset) {
            BLE_LOG_D("Enqueue services start...");
            for(size_t i = 0; i < BLE_SERVICES_COUNT; i++) {
                ble_service_enqueue_init(instance->services[i]);
            }
            instance->state = BleServiceStateInitialization;
            furi_event_loop_timer_stop(instance->init_timer);
        }

        ///TODO: Temporary fix, in order to unblock ble_start()
        instance->current_message->result = true;
        api_lock_unlock(instance->current_message->lock);
    }
}
