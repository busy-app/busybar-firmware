#pragma once

#include <furi.h>

#define BLE_DEVICE_ADDRESS_LEN (6)

typedef struct BleDeviceBase BleDeviceBase;

typedef enum {
    BleDeviceRoleUnknown,
    BleDeviceRoleRemote,
    BleDeviceRoleCentral,

    BleDeviceRoleCount,
} BleDeviceRole;

typedef enum {
    BleDeviceAddressTypeUnknown,
    BleDeviceAddressTypeOrigin,
    BleDeviceAddressTypeResolvable,

    BleDeviceAddressTypeCount,
} BleDeviceAddressType;

BleDeviceBase* ble_device_base_alloc(BleDeviceRole role);
void ble_device_base_free(BleDeviceBase* instance);

const uint8_t* ble_device_base_get_address(BleDeviceBase* instance, BleDeviceAddressType type);
void ble_device_base_set_address(
    BleDeviceBase* instance,
    BleDeviceAddressType type,
    const uint8_t* const addr);
