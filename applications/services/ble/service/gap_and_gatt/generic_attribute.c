#include "generic_attribute.h"
#include "../ble_service_i.h"
#include <furi_hal_info.h>

#define TAG "BleGATT"

typedef enum {
    BleGenericAttributeCharacterServiceChanged,
} BleSrvGenericAttribute;

static bool ble_service_generic_attribute_init(void* object) {
    UNUSED(object);
    return true;
}

//==========================================================
static const BleCharacteristicConfig generic_attribute_service_characteristics[] = {
    {
        .intercom_index = BleGenericAttributeCharacterServiceChanged,
        .name = "Service Changed",
        .initial_data_size = sizeof(uint32_t),
#if defined(BSB_MCU_SI917)
        .uuid = {.Char_UUID_16 = 0x2A05},
        .uuid_size = 2,
        .char_properties = BLE_ATT_PROPERTY_INDICATE,
#endif
    },
};

const BleServiceConfig ble_service_generic_attribute = {
    .name = "Generic Attribute",
#if defined(BSB_MCU_SI917)
    .uuid = {.Char_UUID_16 = 0x1801},
    .uuid_size = 2,
#endif
    .init = ble_service_generic_attribute_init,
    .index = BleServiceIndexGenericAttribute,
    .char_count = COUNT_OF(generic_attribute_service_characteristics),
    .char_configs = generic_attribute_service_characteristics,
};
