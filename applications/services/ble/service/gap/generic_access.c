#include "generic_access.h"
#include "../ble_service_i.h"
#include <furi_hal_info.h>

#define TAG "BleGAP"

typedef enum {
    BleGenericAccessCharacterDeviceName,
    BleGenericAccessCharacterAppearance,
} BleSrvGenericAccess;

#if defined(BSB_MCU_SI917)
static bool ble_service_generic_access_init_917(void* object) {
    UNUSED(object);
    BLE_LOG_W("Init!");
    return true;
}
#else
static bool ble_service_generic_access_init_u5(void* object) {
    BleServiceObject* instance = object;
    BLE_LOG_W("Init!");

    FuriString* name = furi_string_alloc_set_str("Busy Bar");
    BleCharacteristicObject* ch = instance->chars[BleGenericAccessCharacterDeviceName];
    ble_characteristic_set_data(ch, furi_string_get_cstr(name), furi_string_size(name));

    uint16_t appearance = 0x00C0;
    ch = instance->chars[BleGenericAccessCharacterAppearance];
    ble_characteristic_set_data(ch, &appearance, sizeof(appearance));

    return true;
}
#endif

//==========================================================
static const BleCharacteristicDescriptor generic_access_service_characteristics[] = {
    {
        .intercom_index = BleGenericAccessCharacterDeviceName,
        .name = "Device Name",
#if defined(BSB_MCU_SI917)
        .uuid = {.Char_UUID_16 = 0x2A00},
        .uuid_size = 2,
        .char_properties = BLE_ATT_PROPERTY_READ,
#endif
    },
    {
        .intercom_index = BleGenericAccessCharacterAppearance,
        .name = "Appearance",
#if defined(BSB_MCU_SI917)
        .uuid = {.Char_UUID_16 = 0x2A01},
        .uuid_size = 2,
        .char_properties = BLE_ATT_PROPERTY_READ,
#endif
    },
};

const BleServiceDescriptor ble_service_generic_access = {
    .name = "Generic Access",
#if defined(BSB_MCU_SI917)
    .uuid = {.Char_UUID_16 = 0x1800},
    .uuid_size = 2,
    .init = ble_service_generic_access_init_917,
#else
    .init = ble_service_generic_access_init_u5,
#endif
    .index = BleServiceIndexGenericAccess,
    .init_method = BleServiceInitMethodRemote,
    .char_count = COUNT_OF(generic_access_service_characteristics),
    .char_descriptors = generic_access_service_characteristics,
};
