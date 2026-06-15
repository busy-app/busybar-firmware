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

static inline uint8_t*
    ble_device_base_get_address_ptr(BleDeviceBase* instance, BleDeviceAddressType type) {
    furi_assert(type != BleDeviceAddressTypeUnknown && type < BleDeviceAddressTypeCount);
    return (type == BleDeviceAddressTypeResolvable) ? instance->resolvable_addr :
                                                      instance->dev_addr;
}

const uint8_t* ble_device_base_get_address(BleDeviceBase* instance, BleDeviceAddressType type) {
    furi_assert(instance);
    return ble_device_base_get_address_ptr(instance, type);
}

void ble_device_base_set_address(
    BleDeviceBase* instance,
    BleDeviceAddressType type,
    const uint8_t* const addr) {
    furi_assert(instance);
    furi_assert(addr);

    uint8_t* addr_ptr = ble_device_base_get_address_ptr(instance, type);
    memcpy(addr_ptr, addr, BLE_DEVICE_ADDRESS_LEN);
}

void ble_device_base_format_address(
    BleDeviceBase* instance,
    BleDeviceAddressType type,
    FuriString* output) {
    furi_assert(instance);
    furi_assert(output);

    uint8_t* addr_ptr = ble_device_base_get_address_ptr(instance, type);

    for(int8_t i = BLE_DEVICE_ADDRESS_LEN - 1; i >= 0; i--) {
        const char* format = (i == 0) ? "%02X" : "%02X:";
        furi_string_cat_printf(output, format, addr_ptr[i]);
    }
}

void ble_device_base_set_features(BleDeviceBase* instance, const uint8_t* features) {
    furi_assert(instance);
    memcpy(instance->features, features, sizeof(instance->features));
}

bool ble_device_base_is_feature_supported(BleDeviceBase* instance, BleDeviceFeatures feature) {
    furi_assert(instance);
    const uint8_t byte = feature / 8;
    const uint8_t bit = feature % 8;

    return ((instance->features[byte] & (1 << bit)) != 0);
}
