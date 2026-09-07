#include "ble_service_target.h"

#define TAG "BleServiceU5"

static bool ble_service_init_request(BleServiceObject* instance) {
    BLE_LOG_D("%s - ble_service_target_init", instance->config->name);

    bool result = false;
    if(instance->config->init(instance)) {
        BLE_LOG_D("%s - request start remote", instance->config->name);

        result = ble_service_send_data(
            instance, BleServiceCommandInit, BleIntercomFrameTypeRequest, false);
    }

    return result;
}

static bool
    ble_service_init_response(BleServiceObject* instance, size_t data_size, const void* data) {
    UNUSED(data_size);

    if(!ble_service_parse_intercom_service_data(instance, data)) {
        BLE_LOG_W("Update command error");
    }

    instance->ready = true;

    BLE_LOG_D("%s - Ready", instance->config->name);
    return instance->ready;
}

static bool ble_service_command_handler_init(
    BleServiceObject* instance,
    BleIntercomFrameType frame_type,
    size_t data_size,
    const void* data) {
    bool result = false;
    if(frame_type == BleIntercomFrameTypeRequest) {
        BLE_LOG_D("Init request");
        result = ble_service_init_request(instance);
    } else {
        BLE_LOG_D("Init response");
        result = ble_service_init_response(instance, data_size, data);
    }
    return result;
}

static bool ble_service_command_handler_deinit(
    BleServiceObject* instance,
    BleIntercomFrameType frame_type,
    size_t data_size,
    const void* data) {
    UNUSED(instance);
    UNUSED(frame_type);
    UNUSED(data);
    UNUSED(data_size);
    BLE_LOG_D("%s - ble_service_command_handler_deinit", instance->config->name);
    return false;
}

static bool ble_service_command_handler_update(
    BleServiceObject* instance,
    BleIntercomFrameType frame_type,
    size_t data_size,
    const void* data) {
    UNUSED(frame_type);
    UNUSED(data_size);

    bool result = false;
    if(!ble_service_parse_intercom_service_data(instance, data)) {
        BLE_LOG_W("%s - update decode error", instance->config->name);
    } else {
        result = ble_service_send_data(
            instance, BleServiceCommandUpdate, BleIntercomFrameTypeResponse, true);
    }

    return result;
}

static bool ble_service_command_handler_run(
    BleServiceObject* instance,
    BleIntercomFrameType frame_type,
    size_t data_size,
    const void* data) {
    BLE_LOG_D("ble_service_command_handler_run");

    UNUSED(frame_type);

    bool result = false;
    do {
        if(instance->config->run && !instance->config->run(instance, data_size, data)) {
            BLE_LOG_W("%s - run error", instance->config->name);
            break;
        }

        result = ble_service_send_data(
            instance, BleServiceCommandUpdate, BleIntercomFrameTypeRequest, true);
    } while(false);
    return result;
}

bool ble_service_target_execute(
    BleServiceObject* instance,
    BleIntercomFrameType frame_type,
    BleServiceCommandEnum command,
    size_t data_size,
    const void* data) {
    BLE_LOG_D("%s - target_execute: %d", instance->config->name, command);

    bool result = false;
    switch(command) {
    case BleServiceCommandInit:
        result = ble_service_command_handler_init(instance, frame_type, data_size, data);
        break;
    case BleServiceCommandDeinit:
        result = ble_service_command_handler_deinit(instance, frame_type, data_size, data);
        break;
    case BleServiceCommandRun:
        ble_service_command_handler_run(instance, frame_type, data_size, data);
        break;
    case BleServiceCommandUpdate:
        ble_service_command_handler_update(instance, frame_type, data_size, data);
        break;
    default:
        furi_crash("Unknown command");
        break;
    }

    return result;
}

bool ble_service_write_char_data_or_cccd_by_handle(
    BleServiceObject* instance,
    uint8_t index,
    const uint16_t handle,
    const void* data,
    const size_t data_size) {
    UNUSED(instance);
    UNUSED(index);
    UNUSED(handle);
    UNUSED(data);
    UNUSED(data_size);
    BLE_LOG_W("%s - not implemented!", __func__);
    return false;
}
