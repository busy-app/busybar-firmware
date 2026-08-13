#include "ble_i.h"
#include "worker/ble_worker.h"
#include "worker/ble_worker_util.h"

//#define BLE_DEBUG_PRINT_SERVICE_DATA_AFTER_INIT

#define TAG "BLE_917"

static void
    ble_connection_changed_callback(void* ctx, bool connected, const uint8_t* remote_dev_address) {
    BLE_LOG_D("ble_connection_changed_callback");
    Ble* instance = ctx;

    BleState state = {0};
    if(connected) {
        state.status = BleServiceStatusConnected;
        memcpy(state.remote_device_address, remote_dev_address, BLE_REMOTE_ADDRESS_STRING_SIZE);
    } else {
        const bool paired = ble_worker_pairing_exists();
        state.status = paired ? BleServiceStatusConnectable : BleServiceStatusAdvertising;
    }

    ble_command_engine_put_command_no_wait(
        instance->engine, BleCommandSetStatus, &state, sizeof(BleState));
}

static void ble_service_init_wait_callback(BleServiceObject* service, bool result, void* ctx) {
    UNUSED(service);
    UNUSED(result);
    BLE_LOG_D("ble_service_init_wait_callback");
    Ble* instance = ctx;

    uint8_t total_ready;
    for(total_ready = 0; total_ready < BleServiceIndexCount; total_ready++) {
        if(!ble_service_is_ready(instance->services[total_ready])) break;
    }

    if(total_ready == BleServiceIndexCount) {
        instance->status = BleServiceStatusReady;
        ble_set_service_post_process_callback(instance, NULL);
#ifdef BLE_DEBUG_PRINT_SERVICE_DATA_AFTER_INIT
        ble_print_service_hierarchy();
#endif
    }
}

static bool ble_command_init_request(BleIntercomFrameGeneric* frame, void* context) {
    BLE_LOG_D("BleCommandInit request");

    Ble* instance = context;
    instance->worker = ble_worker_init(ble_connection_changed_callback, context);
    ble_set_service_post_process_callback(instance, ble_service_init_wait_callback);

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

    const bool paired = ble_worker_pairing_exists();
    instance->status = paired ? BleServiceStatusConnectable : BleServiceStatusAdvertising;

    frame->header.data_size = sizeof(BleServiceStatus);
    BleServiceStatus* resp_status = (BleServiceStatus*)frame->data;
    *resp_status = instance->status;
    frame->header.result = true;

    return ble_command_response_process(frame, context);
}

static bool ble_command_deinit_request(BleIntercomFrameGeneric* frame, void* context) {
    BLE_LOG_D("BleCommandDeinit request");
    ble_worker_stop();
    return ble_command_deinit_process(frame, context);
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
    bool result = false;
    if(instance->status != BleServiceStatusError) {
        instance->status = BleServiceStatusReady;
        frame->header.data_size = 0;
        frame->header.result = true;
        result = ble_command_response_process(frame, context);
    }
    return result;
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

    size_t response_size = sizeof(BleState);
    frame->header.data_size = response_size;
    frame->header.result = true;

    BleState* response = (BleState*)frame->data;
    response->status = instance->status;

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

static bool ble_command_set_status_request(BleIntercomFrameGeneric* frame, void* context) {
    BLE_LOG_D("ble_command_set_status_request");
    Ble* instance = context;
    frame->header.result = true;

    BleState* state = (BleState*)frame->data;
    instance->status = state->status;
    memcpy(
        instance->remote_device_address,
        state->remote_device_address,
        BLE_REMOTE_ADDRESS_STRING_SIZE);

    bool result = ble_command_request_process(frame, context);
    ble_command_engine_unblock_with_result(instance->engine, NULL, 0, result);
    return result;
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
    [BleCommandDeinit] =
        {
            .request = ble_command_deinit_request,
            .response = NULL,
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
    [BleCommandSetStatus] =
        {
            .request = ble_command_set_status_request,
        },
    [BleCommandForgetPairing] =
        {
            .request = ble_command_forget_pairing_request,
            .response = ble_command_forget_pairing_response,
        },
};
