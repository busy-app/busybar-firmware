#include "ble_device_base.h"

struct BleDeviceBase {
    BleDeviceRole role;
    BleDeviceAddressType addr_type;

    uint8_t dev_addr[BLE_DEVICE_ADDRESS_LEN];
    uint8_t resolvable_addr[BLE_DEVICE_ADDRESS_LEN];

    uint8_t features[8];
};

BleDeviceBase* ble_device_base_alloc(BleDeviceRole role) {
    furi_assert(role != BleDeviceRoleUnknown && role < BleDeviceRoleCount);
    BleDeviceBase* instance = malloc(sizeof(BleDeviceBase));

    instance->role = role;
    return instance;
}

void ble_device_base_free(BleDeviceBase* instance) {
    furi_assert(instance);
    free(instance);
}

const uint8_t* ble_device_base_get_address(BleDeviceBase* instance, BleDeviceAddressType type) {
    furi_assert(instance);
    furi_assert(type != BleDeviceAddressTypeUnknown && type < BleDeviceAddressTypeCount);

    uint8_t* addr_ptr = (type == BleDeviceAddressTypeResolvable) ? instance->resolvable_addr :
                                                                   instance->dev_addr;
    return addr_ptr;
}

void ble_device_base_set_address(
    BleDeviceBase* instance,
    BleDeviceAddressType type,
    const uint8_t* const addr) {
    furi_assert(instance);
    furi_assert(addr);
    furi_assert(type != BleDeviceAddressTypeUnknown && type < BleDeviceAddressTypeCount);

    uint8_t* addr_ptr = (type == BleDeviceAddressTypeResolvable) ? instance->resolvable_addr :
                                                                   instance->dev_addr;
    memcpy(addr_ptr, addr, BLE_DEVICE_ADDRESS_LEN);
}
