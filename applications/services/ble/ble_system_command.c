#include "ble_system_command.h"

static bool ble_command_common_process(
    BleIntercomFrameGeneric* frame,
    void* context,
    BleIntercomFrameType frame_type) {
    Ble* instance = context;
    frame->header.source = BleIntercomFrameSourceSystem;
    frame->header.frame_type = frame_type;
    size_t frame_size = sizeof(BleIntercomFrameHeader) + frame->header.data_size;
    size_t tx =
        intercom_tx(instance->intercom_ch, frame, frame_size, FuriWaitForever);
    furi_assert(tx == frame_size);
    return true;
}

bool ble_command_request_process(BleIntercomFrameGeneric* frame, void* context) {
    return ble_command_common_process(frame, context, BleIntercomFrameTypeRequest);
}

bool ble_command_response_process(BleIntercomFrameGeneric* frame, void* context) {
    return ble_command_common_process(frame, context, BleIntercomFrameTypeResponse);
}
