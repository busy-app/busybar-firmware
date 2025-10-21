#include "ble_service_target.h"

#define TAG "BleServiceU5"

static bool
    ble_service_command_allowed_by_state(const BleCommand command, const BleServiceState state) {
    UNUSED(state);
    UNUSED(command);
    return true;
}

static bool ble_service_target_init(BleServiceObject* instance) {
    BLE_LOG_D("%s - ble_service_target_init", instance->config->name);

    bool result = false;
    if(instance->config->init(instance)) {
        BLE_LOG_D("%s - request start remote", instance->config->name);

        size_t total_data_size = 0;
        uint8_t chars_count = instance->config->char_count;
        for(size_t i = 0; i < chars_count; i++) {
            total_data_size += ble_characteristic_get_data_size(instance->chars[i]);
        }

        size_t total_config_size = sizeof(BleCharacteristicDataHeader) * chars_count +
                                   total_data_size + sizeof(BleCharacteristicCountType);
        BleIntercomServiceData* config = malloc(total_config_size);

        config->char_count = chars_count;
        uint8_t offset = 0;
        for(size_t i = 0; i < chars_count; i++) {
            BleCharacteristicObject* ch_obj = instance->chars[i];

            BleCharacteristicData* char_init =
                (BleCharacteristicData*)((uint8_t*)config->chars_config + offset);

            offset += ble_characteristic_fill_update_struct(ch_obj, char_init);
        }

        BLE_LOG_D("%s - config size: %d", instance->config->name, total_config_size);

        ble_service_prepare_send_intercom_frame(
            instance,
            BleIntercomFrameTypeRequest,
            BleCommandServiceInit,
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
    if(data_size == 0) {
        BLE_LOG_W("Data_size == 0");
        return false;
    }

    const BleIntercomServiceData* service_config = data;
    uint8_t offset = 0;

    for(size_t i = 0; i < service_config->char_count; i++) {
        const BleCharacteristicData* char_init =
            (BleCharacteristicData*)((uint8_t*)service_config->chars_config + offset);
        size_t data_size = char_init->header.data_size;

        BleCharacteristicObject* ch = instance->chars[char_init->header.index];
        ble_characteristic_set_data(ch, char_init->data, data_size);

        BLE_LOG_D(
            "Ch: %s new data: %s",
            ble_characteristic_get_config(ch)->name,
            (const char*)char_init->data);

        offset += (data_size + sizeof(BleCharacteristicDataHeader));
    }

    size_t total_size = sizeof(BleCharacteristicCountType);
    ble_service_prepare_send_intercom_frame(
        instance, BleIntercomFrameTypeResponse, BleCommandServiceUpdate, total_size, data);

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
            instance, BleIntercomFrameTypeRequest, BleCommandServiceUpdate, total_size, config);

        free(config);

        result = true;
    } while(false);
    return result;
}

bool ble_service_target_execute(
    BleServiceObject* instance,
    BleIntercomFrameType frame_type,
    BleCommand command,
    size_t data_size,
    void* data) {
    BLE_LOG_D("%s - target_execute: %d", instance->config->name, command);

    bool result = false;
    if(ble_service_command_allowed_by_state(command, instance->state)) {
        switch(command) {
        case BleCommandServiceInit:
            result = ble_service_command_handler_init(instance, frame_type, data_size, data);
            break;
        case BleCommandServiceRun:
            ble_service_command_handler_run(instance, frame_type, data_size, data);
            break;
        case BleCommandServiceUpdate:
            ble_service_command_handler_update(instance, frame_type, data_size, data);
            break;
        default:
            break;
        }
    }

    return result;
}
