#pragma once

#include "ble_service.h"
#include "ble_service_config_types.h"

typedef struct BleCharacteristicObject BleCharacteristicObject;

BleCharacteristicObject* ble_characteristic_alloc(
    const BleCharacteristicDescriptor* config,
    BleServiceObject* parent_service);
void ble_characteristic_free(BleCharacteristicObject* instance);
void ble_characteristic_reset(BleCharacteristicObject* instance);

const void* ble_characteristic_get_data(BleCharacteristicObject* instance);
size_t ble_characteristic_get_data_size(BleCharacteristicObject* instance);

void ble_characteristic_set_data(
    BleCharacteristicObject* instance,
    const void* data,
    const size_t data_size);

bool ble_characteristic_is_modified(BleCharacteristicObject* instance);

void ble_characteristic_tx_done(BleCharacteristicObject* instance);

const BleCharacteristicDescriptor*
    ble_characteristic_get_config(BleCharacteristicObject* instance);
void ble_characteristic_set_handle(BleCharacteristicObject* instance, uint16_t handle);
uint16_t ble_characteristic_get_handle(BleCharacteristicObject* instance);

void ble_characteristic_set_cccd_handle(BleCharacteristicObject* instance, uint16_t cccd_handle);
bool ble_characteristic_is_cccd_handle(BleCharacteristicObject* instance, uint16_t possible_cccd);
void ble_characteristic_set_cccd_value(BleCharacteristicObject* instance, uint8_t value);
uint8_t ble_characteristic_get_cccd_value(BleCharacteristicObject* instance);

size_t ble_characteristic_encode(BleCharacteristicObject* instance, BleCharacteristicData* output);
bool ble_characteristic_decode(
    BleCharacteristicObject* instance,
    const BleCharacteristicData* input);

void ble_characteristic_register_update_callback(
    BleCharacteristicObject* instance,
    BleDataUpdatedCallback callback,
    void* ctx);

void ble_characteristic_register_tx_done_callback(
    BleCharacteristicObject* instance,
    BleDataTransmitDoneCallback callback,
    void* ctx);
