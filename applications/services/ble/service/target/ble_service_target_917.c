#include "ble_service_target.h"
#include "../../worker/ble_worker.h"

#define TAG "BleService917"

static bool ble_service_target_init(BleServiceObject* instance, size_t data_size, void* data) {
    BLE_LOG_D("%s - ble_service_target_init", instance->config->name);

    do {
        if(data_size == 0) {
            ble_service_set_error(instance, "Empty init data");
            break;
        }

        if(!ble_service_parse_intercom_service_data(instance, data, NULL)) {
            ble_service_set_error(instance, "Failed to parse service data");
            break;
        }

        if(!ble_worker_register_service(instance)) {
            ble_service_set_error(instance, "Failed to register service");
            break;
        }
        instance->ready = true;
    } while(false);

    BLE_LOG_I("%s - %s", instance->config->name, instance->ready ? "Ready" : "Not ready");

    ble_service_prepare_send_intercom_response_frame(
        instance,
        BleServiceCommandInit,
        instance->ready,
        furi_string_size(instance->error),
        furi_string_get_cstr(instance->error));

    return true;
}

static bool ble_service_command_handler_init(
    BleServiceObject* instance,
    BleIntercomFrameType frame_type,
    size_t data_size,
    void* data) {
    bool result = false;
    if(frame_type == BleIntercomFrameTypeRequest) {
        BLE_LOG_D("Init request");
        result = ble_service_target_init(instance, data_size, data);
    } else {
        BLE_LOG_D("Init response");
    }
    return result;
}

static void ble_service_update_characteristic_extra_action(BleCharacteristicObject* ch) {
    const uint16_t handle = ble_characteristic_get_handle(ch);
    const uint8_t cccd_value = ble_characteristic_get_cccd_value(ch);
    size_t data_size = ble_characteristic_get_data_size(ch);
    ble_worker_send(handle, data_size, ble_characteristic_get_data(ch), cccd_value);
}

static bool ble_service_update_request(BleServiceObject* instance, size_t data_size, void* data) {
    if(data_size == 0) {
        BLE_LOG_W("Data_size == 0");
        return false;
    }

    ble_service_parse_intercom_service_data(
        instance, data, ble_service_update_characteristic_extra_action);

    ble_service_prepare_send_intercom_frame(
        instance, BleIntercomFrameTypeResponse, BleServiceCommandUpdate, 0, NULL);

    return true;
}

static bool ble_service_update_response(BleServiceObject* instance, size_t data_size, void* data) {
    UNUSED(data_size);

    const BleIntercomServiceData* service_config = data;
    uint8_t offset = 0;

    for(size_t i = 0; i < service_config->char_count; i++) {
        const BleCharacteristicData* char_init =
            (BleCharacteristicData*)((uint8_t*)service_config->chars_config + offset);

        BleCharacteristicObject* ch = instance->chars[char_init->header.index];
        uint16_t handle = ble_characteristic_get_handle(ch);
        const uint8_t cccd_value = ble_characteristic_get_cccd_value(ch);
        ble_worker_receive_confirm(handle, cccd_value);
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
        size_t total_data_size = 0;
        const uint8_t chars_count_max = instance->config->char_count;
        uint8_t chars_count = 0;
        for(size_t i = 0; i < chars_count_max; i++) {
            if(!ble_characteristic_is_modified(instance->chars[i])) continue;
            total_data_size += ble_characteristic_get_data_size(instance->chars[i]);
            chars_count++;
        }

        size_t total_size = sizeof(BleCharacteristicDataHeader) * chars_count + total_data_size +
                            sizeof(BleCharacteristicCountType);
        BleIntercomServiceData* config = malloc(total_size);

        config->char_count = chars_count;
        uint8_t offset = 0;
        for(size_t i = 0; i < chars_count_max; i++) {
            BleCharacteristicObject* ch_obj = instance->chars[i];

            if(!ble_characteristic_is_modified(ch_obj)) continue;
            BleCharacteristicData* char_init =
                (BleCharacteristicData*)((uint8_t*)config->chars_config + offset);

            offset += ble_characteristic_fill_update_struct(ch_obj, char_init);
        }

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
    switch(command) {
    case BleServiceCommandInit:
        result = ble_service_command_handler_init(instance, frame_type, data_size, data);
        break;
    case BleServiceCommandRun:
        ble_service_command_handler_run(instance, frame_type, data_size, data);
        break;
    case BleServiceCommandUpdate:
        result = ble_service_command_handler_update(instance, frame_type, data_size, data);
        break;
    default:
        __furi_crash("Unknown command");
        break;
    }

    return result;
}
