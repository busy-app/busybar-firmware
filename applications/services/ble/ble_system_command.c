#include "ble_system_command.h"
#include "ble_i.h"

static bool ble_command_common_process(
    BleIntercomFrameGeneric* frame,
    void* context,
    BleIntercomFrameType frame_type) {
    Ble* instance = context;
    frame->header.source = BleIntercomFrameSourceSystem;
    frame->header.frame_type = frame_type;
    size_t frame_size = sizeof(BleIntercomFrameHeader) + frame->header.data_size;
    size_t tx = intercom_tx(instance->intercom_ch, frame, frame_size, BLE_INTERCOM_TX_TIMEOUT_MS);
    return tx == frame_size;
}

bool ble_command_request_process(BleIntercomFrameGeneric* frame, void* context) {
    return ble_command_common_process(frame, context, BleIntercomFrameTypeRequest);
}

bool ble_command_response_process(BleIntercomFrameGeneric* frame, void* context) {
    return ble_command_common_process(frame, context, BleIntercomFrameTypeResponse);
}

static void ble_deinit_services(Ble* instance) {
    for(size_t i = 0; i < BleServiceIndexCount; i++) {
        ble_service_deinit(instance->services[i]);
    }
}

bool ble_command_deinit_process(BleIntercomFrameGeneric* frame, void* context) {
    UNUSED(frame);
    Ble* instance = context;

    if(instance->status != BleServiceStatusError) {
        furi_string_printf(instance->error, "Intercom error");
        instance->status = BleServiceStatusError;

        ble_deinit_services(instance);
        ble_command_engine_unblock_with_result(instance->engine, NULL, 0, false);
    }
    return false;
}
