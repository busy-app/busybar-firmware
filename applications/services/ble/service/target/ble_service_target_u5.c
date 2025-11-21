#include "ble_service_target.h"

#define TAG "BleServiceU5"

static bool ble_service_command_allowed_by_state(
    const BleServiceCommandEnum command,
    const BleServiceState state) {
    UNUSED(state);
    UNUSED(command);
    return true;
}

static bool ble_service_target_init(BleServiceObject* instance) {
    BLE_LOG_D("%s - ble_service_target_init", instance->config->name);

    bool result = false;
    if(instance->config->init(instance)) {
        BLE_LOG_D("%s - request start remote", instance->config->name);

        size_t total_config_size = 0;
        BleIntercomServiceData* config =
            ble_service_create_intercom_service_data_pack(instance, false, &total_config_size);

        BLE_LOG_D("%s - config size: %d", instance->config->name, total_config_size);

        ble_service_prepare_send_intercom_frame(
            instance,
            BleIntercomFrameTypeRequest,
            BleServiceCommandInit,
            total_config_size,
            config);

        free(config);
        result = true;
    }

    return result;
}

static bool ble_service_command_handler_init(
    BleServiceObject* instance,
    BleIntercomFrameType frame_type,
    size_t data_size,
    void* data) {
    UNUSED(data);
    UNUSED(data_size);

    bool result = false;
    if(frame_type == BleIntercomFrameTypeRequest) {
        BLE_LOG_D("Init request");
        result = ble_service_target_init(instance);
    } else {
        BLE_LOG_D("Init response");
        ble_service_switch_state(instance, BleServiceStateReady);
        result = true;
    }
    return result;
}

static bool ble_service_update_request(BleServiceObject* instance, size_t data_size, void* data) {
    ///TODO: think of moving this to ble_service_parse_intercom_service_data
    if(data_size == 0) {
        BLE_LOG_W("Data_size == 0");
        return false;
    }

    bool result = ble_service_parse_intercom_service_data(instance, data, NULL);

    size_t total_size = sizeof(BleCharacteristicCountType);
    ble_service_prepare_send_intercom_frame(
        instance, BleIntercomFrameTypeResponse, BleServiceCommandUpdate, total_size, data);

    return result;
}

static bool ble_service_update_response(BleServiceObject* instance, size_t data_size, void* data) {
    UNUSED(data_size);

    const BleIntercomServiceData* service_config = data;
    uint8_t offset = 0;

    for(size_t i = 0; i < service_config->char_count; i++) {
        const BleCharacteristicData* char_init =
            (BleCharacteristicData*)((uint8_t*)service_config->chars_config + offset);

        BleCharacteristicObject* ch = instance->chars[char_init->header.index];
        ble_characteristic_tx_done(ch);
    }
    return true;
}

static bool ble_service_command_handler_update(
    BleServiceObject* instance,
    BleIntercomFrameType frame_type,
    size_t data_size,
    void* data) {
    if(frame_type == BleIntercomFrameTypeRequest) {
        return ble_service_update_request(instance, data_size, data);
    } else {
        return ble_service_update_response(instance, data_size, data);
    }
}

static bool ble_service_command_handler_run(
    BleServiceObject* instance,
    BleIntercomFrameType frame_type,
    size_t data_size,
    void* data) {
    BLE_LOG_D("ble_service_command_handler_run");

    UNUSED(frame_type);
    UNUSED(data_size);
    UNUSED(data);

    bool result = false;
    do {
        if(instance->config->run && !instance->config->run(instance)) {
            BLE_LOG_W("%s - run error", instance->config->name);
            break;
        }

        size_t total_size = 0;
        BleIntercomServiceData* config =
            ble_service_create_intercom_service_data_pack(instance, true, &total_size);

        BLE_LOG_D("%s - config size: %d", instance->config->name, total_size);

        ble_service_prepare_send_intercom_frame(
            instance, BleIntercomFrameTypeRequest, BleServiceCommandUpdate, total_size, config);

        free(config);

        result = true;
    } while(false);
    return result;
}

bool ble_service_target_execute(
    BleServiceObject* instance,
    BleIntercomFrameType frame_type,
    BleServiceCommandEnum command,
    size_t data_size,
    void* data) {
    BLE_LOG_D("%s - target_execute: %d", instance->config->name, command);

    bool result = false;
    if(ble_service_command_allowed_by_state(command, instance->state)) {
        switch(command) {
        case BleServiceCommandInit:
            result = ble_service_command_handler_init(instance, frame_type, data_size, data);
            break;
        case BleServiceCommandRun:
            ble_service_command_handler_run(instance, frame_type, data_size, data);
            break;
        case BleServiceCommandUpdate:
            ble_service_command_handler_update(instance, frame_type, data_size, data);
            break;
        default:
            __furi_crash("Unknown command");
            break;
        }
    }

    return result;
}
