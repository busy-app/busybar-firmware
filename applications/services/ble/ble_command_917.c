#include "ble_command.h"
#include "worker/ble_worker.h"

#define TAG "BLE_917"

BleIntercomFrameGeneric* ble_command_preprocess(Ble* instance, uint32_t events) {
    UNUSED(events);
    return &instance->mailbox;
}

void ble_command_handler_enable(Ble* instance, BleIntercomFrameGeneric* frame) {
    if(frame->header.frame_type == BleIntercomFrameTypeResponse) {
        BLE_LOG_D("BleCommandEnable response");
    } else {
        BLE_LOG_D("BleCommandEnable request");
        ble_worker_start();
        frame->header.frame_type = BleIntercomFrameTypeResponse;
        size_t frame_size = sizeof(BleIntercomFrameHeader) + frame->header.data_size;
        size_t tx = intercom_tx(instance->intercom_ch, frame, frame_size, 100);
        furi_assert(tx == frame_size);
    }
}

void ble_command_handler_disable(Ble* instance, BleIntercomFrameGeneric* frame) {
    if(frame->header.frame_type == BleIntercomFrameTypeResponse) {
        BLE_LOG_D("BleCommandDisable response");
    } else {
        BLE_LOG_D("BleCommandDisable request");
        ble_worker_stop();
        frame->header.frame_type = BleIntercomFrameTypeResponse;
        size_t frame_size = sizeof(BleIntercomFrameHeader) + frame->header.data_size;
        size_t tx = intercom_tx(instance->intercom_ch, frame, frame_size, 100);
        furi_assert(tx == frame_size);
    }
}

void ble_command_handler_get_status(Ble* instance, BleIntercomFrameStatus* frame) {
    if(frame->header.frame_type == BleIntercomFrameTypeResponse) {
        BLE_LOG_W("No need response");
    } else {
        BLE_LOG_D("GetStatus request");
        frame->header.frame_type = BleIntercomFrameTypeResponse;
        frame->header.data_size = sizeof(BleServiceState);
        frame->state = instance->state;

        ble_worker_init();

        size_t frame_size = sizeof(BleIntercomFrameStatus);
        size_t tx = intercom_tx(instance->intercom_ch, frame, frame_size, 100);
        furi_assert(tx == frame_size);
    }
}

void ble_command_postprocess(Ble* instance, uint32_t events, bool result) {
    UNUSED(result);
    if(events == BleEventTypeFrameReceived) {
        furi_semaphore_release(instance->mailbox_lock);
    }
}
