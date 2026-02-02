
#include "ble_characteristic.h"
#include <furi.h>

#define TAG "BleChar"

struct BleCharacteristicObject {
    bool modified;
    uint8_t max_data_size;
    uint8_t data_size; ///TODO: set data_type of proper size
    uint8_t cccd_value;
    uint16_t cccd_handle;
    uint16_t handle; ///TODO: maybe add this only to 917
    void* data;

    BleDataUpdatedCallback update_cb;
    void* update_ctx;

    BleDataTransmitDoneCallback tx_done_cb;
    void* tx_done_ctx;

    const BleCharacteristicDescriptor* descriptor;
};

BleCharacteristicObject* ble_characteristic_alloc(const BleCharacteristicDescriptor* config) {
    furi_assert(config);
    BleCharacteristicObject* instance = malloc(sizeof(BleCharacteristicObject));
    instance->descriptor = config;
    if(config->initial_data_size > 0) {
        instance->data = malloc(config->initial_data_size);
        instance->max_data_size = config->initial_data_size;
        instance->data_size = config->initial_data_size;
    }
    return instance;
}

void ble_characteristic_free(BleCharacteristicObject* instance) {
    furi_assert(instance);
    if(instance->data) free(instance->data);
    free(instance);
}

const void* ble_characteristic_get_data(BleCharacteristicObject* instance) {
    furi_assert(instance);
    instance->modified = false;
    return instance->data;
}

size_t ble_characteristic_get_data_size(BleCharacteristicObject* instance) {
    furi_assert(instance);
    return instance->data_size;
}

void ble_characteristic_set_data(
    BleCharacteristicObject* instance,
    const void* data,
    const size_t data_size) {
    furi_assert(instance);
    furi_assert(data);
    furi_assert(data_size > 0);

    if(instance->data == NULL && instance->descriptor->initial_data_size == 0) {
        instance->data = malloc(data_size);
        instance->max_data_size = data_size;
    }

    furi_assert(instance->max_data_size >= data_size);
    memcpy(instance->data, data, data_size);
    instance->data_size = data_size;
    instance->modified = true;

    if(instance->update_cb) {
        instance->update_cb(data_size, instance->data, instance->update_ctx);
        instance->modified = false;
    }
}

void ble_characteristic_tx_done(BleCharacteristicObject* instance) {
    furi_assert(instance);
    if(instance->tx_done_cb) {
        instance->tx_done_cb(instance->tx_done_ctx);
        instance->modified = false;
    }
}

bool ble_characteristic_is_modified(BleCharacteristicObject* instance) {
    furi_assert(instance);
    return instance->modified;
}

const BleCharacteristicDescriptor*
    ble_characteristic_get_config(BleCharacteristicObject* instance) {
    furi_assert(instance);
    return instance->descriptor;
}

void ble_characteristic_set_handle(BleCharacteristicObject* instance, uint16_t handle) {
    furi_assert(instance);
    furi_assert(instance->handle == 0);
    instance->handle = handle;
}

uint16_t ble_characteristic_get_handle(BleCharacteristicObject* instance) {
    furi_assert(instance);
    return instance->handle;
}

void ble_characteristic_set_cccd_handle(BleCharacteristicObject* instance, uint16_t cccd_handle) {
    furi_assert(instance);
    furi_assert(instance->cccd_handle == 0);
    instance->cccd_handle = cccd_handle;
}

bool ble_characteristic_is_cccd_handle(BleCharacteristicObject* instance, uint16_t possible_cccd) {
    furi_assert(instance);
    return instance->cccd_handle == possible_cccd;
}

void ble_characteristic_set_cccd_value(BleCharacteristicObject* instance, uint8_t value) {
    furi_assert(instance);
    instance->cccd_value = value;
}

uint8_t ble_characteristic_get_cccd_value(BleCharacteristicObject* instance) {
    furi_assert(instance);
    return instance->cccd_value;
}

uint8_t ble_characteristic_fill_update_struct(
    BleCharacteristicObject* instance,
    BleCharacteristicData* output) {
    furi_assert(instance);
    furi_assert(output);

    output->header.index = instance->descriptor->intercom_index;
    output->header.data_size = instance->data_size;
    BLE_LOG_D("%s - char size: %d", instance->descriptor->name, instance->data_size);

    memcpy(output->data, instance->data, instance->data_size);
    instance->modified = false;
    return (instance->data_size + sizeof(BleCharacteristicDataHeader));
}

void ble_characteristic_register_update_callback(
    BleCharacteristicObject* instance,
    BleDataUpdatedCallback callback,
    void* ctx) {
    furi_assert(instance);

    if(callback) {
        instance->update_cb = callback;
        instance->update_ctx = ctx;
    } else {
        BLE_LOG_D("Reset update callback");
        instance->update_cb = NULL;
        instance->update_ctx = NULL;
    }
}

void ble_characteristic_register_tx_done_callback(
    BleCharacteristicObject* instance,
    BleDataTransmitDoneCallback callback,
    void* ctx) {
    furi_assert(instance);

    if(callback) {
        instance->tx_done_cb = callback;
        instance->tx_done_ctx = ctx;
    } else {
        BLE_LOG_D("Reset tx_done callback");
        instance->tx_done_cb = NULL;
        instance->tx_done_ctx = NULL;
    }
}
