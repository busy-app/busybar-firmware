#include "ble_command_engine.h"
#include "ble_system_command.h"
#include "worker/ble_worker.h"

#define TAG "BLE_917"

BleIntercomFrameGeneric* ble_command_preprocess(Ble* instance, uint32_t events) {
    UNUSED(events);
    return (BleIntercomFrameGeneric*)&instance->mailbox;
}

static void ble_connection_changed_callback(
    void* ctx,
    bool connected,
    const uint8_t remote_dev_address[BLE_REMOTE_ADDRESS_SIZE]) {
    BLE_LOG_D("ble_connection_changed_callback");
    Ble* instance = ctx;

    furi_mutex_acquire(instance->ble_lock, FuriWaitForever);
    furi_semaphore_acquire(instance->mailbox_lock, FuriWaitForever);

    instance->state = connected ? BleServiceStateConnected : BleServiceStateAdvertising;
    BleIntercomFrameGeneric* frame = &instance->mailbox;
    frame->header.frame_type = BleIntercomFrameTypeRequest;
    frame->header.command = BleCommandConnectionUpdated;
    frame->header.source = BleIntercomFrameSourceSystem;
    frame->header.data_size = sizeof(connected) + BLE_REMOTE_ADDRESS_SIZE;
    frame->data[0] = connected;

    memcpy(instance->remote_device_address, remote_dev_address, BLE_REMOTE_ADDRESS_SIZE);
    memcpy(&frame->data[1], remote_dev_address, BLE_REMOTE_ADDRESS_SIZE);

    ble_command_request_process(frame, instance);
    furi_semaphore_release(instance->mailbox_lock);
    furi_mutex_release(instance->ble_lock);
}

static bool ble_command_init_request(BleIntercomFrameGeneric* frame, void* context) {
    BLE_LOG_D("BleCommandInit request");
    ble_worker_init(ble_connection_changed_callback, context);
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
    Ble* instance = context;

    ble_worker_start();

    instance->state = BleServiceStateAdvertising;
    frame->header.data_size = 0;
    frame->header.result = true;

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
    Ble* instance = context;

    ble_worker_stop();

    instance->state = BleServiceStateReady;
    frame->header.data_size = 0;
    frame->header.result = true;

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

    size_t response_size = sizeof(BleIntercomResponse) + sizeof(BleServiceState);
    frame->header.data_size = response_size;
    BleIntercomResponse* response = (BleIntercomResponse*)frame->data;

    response->result = true;
    response->data[0] = instance->state;

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

static bool ble_command_set_device_name_request(BleIntercomFrameGeneric* frame, void* context) {
    BLE_LOG_D("BleCommandSetDeviceName request");

    const char* name = (const char*)frame->data;
    ble_worker_set_name(name);

    return ble_command_response_process(frame, context);
}

static bool ble_command_set_device_name_response(BleIntercomFrameGeneric* frame, void* context) {
    UNUSED(frame);
    UNUSED(context);
    BLE_LOG_D("BleCommandSetDeviceName response");
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
    UNUSED(instance);
    UNUSED(retry_timeout);
    UNUSED(command);
    UNUSED(retry_event);
    furi_crash("Not implemented");
}
