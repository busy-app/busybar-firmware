#include "generic_access_i.h"

static const BleCharacteristicConfig generic_access_service_characteristics[] = {
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

const BleServiceConfig ble_service_generic_access = {
    .name = "Generic Access",
#if defined(BSB_MCU_SI917)
    .uuid = {.Char_UUID_16 = 0x1800},
    .uuid_size = 2,
#endif
    .init = ble_service_generic_access_init,
    .run = ble_service_generic_access_run,
    .index = BleServiceIndexGenericAccess,
    .char_count = COUNT_OF(generic_access_service_characteristics),
    .char_configs = generic_access_service_characteristics,
};
