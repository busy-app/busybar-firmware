#include "ble_service_target.h"
#include "../../worker/ble_worker.h"

#define TAG "BleService917"

static void ble_char_tx_done_cb(void* ctx) {
    BleCharacteristicObject* ch = ctx;
    uint16_t handle = ble_characteristic_get_handle(ch);
    const uint8_t cccd_value = ble_characteristic_get_cccd_value(ch);
    BLE_LOG_D("Upd resp, H: %04X, val: %02X", handle, cccd_value);
    ble_worker_receive_confirm(handle, cccd_value);
}

static void ble_characteristic_update_callback(size_t data_size, void* data, void* context) {
    BleCharacteristicObject* ch = context;
    const uint16_t handle = ble_characteristic_get_handle(ch);
    const uint8_t cccd_value = ble_characteristic_get_cccd_value(ch);
    ble_worker_send(handle, data_size, data, cccd_value);
}

static bool ble_service_command_handler_init(
    BleServiceObject* instance,
    BleIntercomFrameType frame_type,
    size_t data_size,
    const void* data) {
    UNUSED(frame_type);
    bool result = false;
    do {
        if(instance->config->init && !instance->config->init(instance)) {
            BLE_LOG_W("%s - init failed", instance->config->name);
            break;
        }

        if(data_size == 0) {
            ble_service_set_error(instance, "Empty init data");
            break;
        }

        if(!ble_service_parse_intercom_service_data(instance, data)) {
            ble_service_set_error(instance, "Failed to parse service data");
            break;
        }

        for(uint8_t i = 0; i < instance->config->char_count; i++) {
            BleCharacteristicObject* ch = instance->chars[i];
            ble_characteristic_register_tx_done_callback(ch, ble_char_tx_done_cb, ch);
            ble_characteristic_register_update_callback(
                ch, ble_characteristic_update_callback, ch);
        }

        if(!ble_worker_register_service(instance)) {
            ble_service_set_error(instance, "Failed to register service");
            break;
        }

        instance->ready = true;
        result = ble_service_send_data(
            instance, BleServiceCommandInit, BleIntercomFrameTypeResponse, false);

    } while(false);
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
    UNUSED(data_size);
    UNUSED(data);

    return ble_service_send_data(
        instance, BleServiceCommandUpdate, BleIntercomFrameTypeRequest, true);
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
        result = ble_service_command_handler_update(instance, frame_type, data_size, data);
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
    furi_assert(instance);
    furi_assert(index < instance->config->char_count);
    furi_assert(data);
    furi_assert(data_size > 0);
    furi_assert(handle > instance->handle);

    bool cccd_modified = false;
    if(ble_service_lock(instance)) {
        BleCharacteristicObject* ch = instance->chars[index];

        if(ble_characteristic_is_cccd_handle(ch, handle)) {
            uint8_t ccd_val = *((uint8_t*)data);
            ble_characteristic_set_cccd_value(ch, ccd_val); //INVOKE cccd_update callback
            cccd_modified = true;
        } else {
            ble_characteristic_set_data(ch, data, data_size);
            ble_service_enqueue_run(instance);
        }
        ble_service_unlock(instance);
    }
    return cccd_modified;
}
