#include "ble_command_engine.h"
#include "ble_system_command.h"
#include "worker/ble_worker.h"

#define TAG "BLE_917"

BleIntercomFrameGeneric* ble_command_preprocess(Ble* instance, uint32_t events) {
    UNUSED(events);
    return (BleIntercomFrameGeneric*)&instance->mailbox;
}

static void
    ble_connection_changed_callback(void* ctx, bool connected, const uint8_t* remote_dev_address) {
    BLE_LOG_D("ble_connection_changed_callback");
    Ble* instance = ctx;

    furi_mutex_acquire(instance->ble_lock, FuriWaitForever);
    furi_semaphore_acquire(instance->mailbox_lock, FuriWaitForever);

    instance->state = connected ? BleServiceStateConnected : BleServiceStateAdvertising;

    BleStatus status = {
        .state = instance->state,
        .pairing = ble_worker_pairing_exists() ? BlePairingStatePaired : BlePairingStateNotPaired,
    };
    memcpy(
        status.remote_device_address, remote_dev_address, BLE_REMOTE_DEVICE_ADDRESS_STRING_SIZE);
    memcpy(instance->remote_device_address, remote_dev_address, BLE_REMOTE_ADDRESS_STRING_SIZE);

    BleIntercomFrameGeneric* frame = &instance->mailbox;
    frame->header.frame_type = BleIntercomFrameTypeRequest;
    frame->header.command = BleCommandSetStatus;
    frame->header.source = BleIntercomFrameSourceSystem;
    frame->header.data_size = sizeof(BleStatus);
    frame->header.result = true;

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

static bool ble_command_get_status_request(BleIntercomFrameGeneric* frame, void* context) {
    BLE_LOG_D("BleCommandGetStatus request");
    Ble* instance = context;

    size_t response_size = sizeof(BleStatus);
    frame->header.data_size = response_size;
    frame->header.result = true;

    BleStatus* response = (BleStatus*)frame->data;
    response->state = instance->state;
    if(instance->state != BleServiceStateReset) {
        response->pairing = ble_worker_pairing_exists() ? BlePairingStatePaired :
                                                          BlePairingStateNotPaired;
    } else {
        response->pairing = BlePairingStateUnkown;
    }

    memcpy(
        response->remote_device_address,
        instance->remote_device_address,
        BLE_REMOTE_ADDRESS_STRING_SIZE);

    return ble_command_response_process(frame, context);
}

static bool ble_command_get_status_response(BleIntercomFrameGeneric* frame, void* context) {
    UNUSED(frame);
    UNUSED(context);
    BLE_LOG_D("BleCommandGetStatus response");
    return true;
}

static bool ble_command_forget_pairing_request(BleIntercomFrameGeneric* frame, void* context) {
    BLE_LOG_D("BleCommandForgetPairing request");
    bool result = ble_worker_forget_pairing();
    frame->header.result = result;
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
    frame->header.result = true;
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
    [BleCommandGetStatus] =
        {
            .request = ble_command_get_status_request,
            .response = ble_command_get_status_response,
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
