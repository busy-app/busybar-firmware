#include "ble_command_engine.h"
#include "ble_system_command.h"
#include "worker/ble_worker.h"

#define TAG "BLE_917"

BleIntercomFrameGeneric* ble_command_preprocess(Ble* instance, uint32_t events) {
    UNUSED(events);
    return (BleIntercomFrameGeneric*)&instance->mailbox;
}

static bool ble_command_init_request(BleIntercomFrameGeneric* frame, void* context) {
    BLE_LOG_D("BleCommandInit request");
    ble_worker_init();
    return ble_command_response_process(frame, context);
}

static bool ble_command_init_response(BleIntercomFrameGeneric* frame, void* context) {
    UNUSED(frame);
    UNUSED(context);
    BLE_LOG_D("BleCommandInit response");
    return true;
}

static bool ble_command_enable_request(BleIntercomFrameGeneric* frame, void* context) {
    BLE_LOG_D("BleCommandEnable request");
    ble_worker_start();

    return ble_command_response_process(frame, context);
}

static bool ble_command_enable_response(BleIntercomFrameGeneric* frame, void* context) {
    UNUSED(frame);
    UNUSED(context);
    BLE_LOG_D("BleCommandEnable response");
    return true;
}

static bool ble_command_disable_request(BleIntercomFrameGeneric* frame, void* context) {
    BLE_LOG_D("BleCommandDisable request");
    ble_worker_stop();
    return ble_command_response_process(frame, context);
}

static bool ble_command_disable_response(BleIntercomFrameGeneric* frame, void* context) {
    UNUSED(frame);
    UNUSED(context);
    BLE_LOG_D("BleCommandDisable response");
    return true;
}

static bool ble_command_get_state_request(BleIntercomFrameGeneric* frame, void* context) {
    BLE_LOG_D("BleCommandDisable request");
    Ble* instance = context;
    frame->header.source = BleIntercomFrameSourceSystem;
    frame->header.frame_type = BleIntercomFrameTypeResponse;
    frame->header.data_size = sizeof(BleServiceState);
    frame->data[0] = instance->state;

    return ble_command_response_process(frame, context);
}

static bool ble_command_get_state_response(BleIntercomFrameGeneric* frame, void* context) {
    UNUSED(frame);
    UNUSED(context);
    BLE_LOG_D("BleCommandDisable response");
    return true;
}

static bool ble_command_forget_pairing_request(BleIntercomFrameGeneric* frame, void* context) {
    BLE_LOG_D("BleCommandForgetPairing request");
    bool result = ble_worker_forget_pairing();
    frame->header.data_size = 1;
    frame->data[0] = result;
    return ble_command_response_process(frame, context);
}

static bool ble_command_forget_pairing_response(BleIntercomFrameGeneric* frame, void* context) {
    UNUSED(frame);
    UNUSED(context);
    BLE_LOG_D("BleCommandForgetPairing response");
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
    UNUSED(instance);
    UNUSED(retry_timeout);
    UNUSED(command);
    UNUSED(retry_event);
    furi_crash("Not implemented");
}
