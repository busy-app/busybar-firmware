#include "ble_service_device_info.h"

#include "../ble_service_i.h"
#include <furi_hal_info.h>

#define TAG "BleDevInfo"

typedef struct {
    FuriString* hw_revision;
    FuriString* sw_revision;
    FuriString* serial_number;
} BleDeviceInfoDataContext;

typedef enum {
    BleSrvDeviceInfoCharacterIndexSerialNumber,
    BleSrvDeviceInfoCharacterIndexHardwareRevision,
    BleSrvDeviceInfoCharacterIndexSoftwareRevision,
} BleSrvDeviceInfoCharacterIndex;

#if defined(BSB_MCU_SI917)
static bool ble_service_device_info_init_917(void* object) {
    UNUSED(object);
    return true;
}
#else
static void device_info_callback(const char* key, const char* value, bool last, void* context) {
    UNUSED(last);
    UNUSED(value);

    BleDeviceInfoDataContext* device_info = context;

    FuriString* key_str = furi_string_alloc_set_str(key);

    if(furi_string_equal_str(key_str, "u5_firmware_commit")) {
        device_info->sw_revision = furi_string_alloc_printf(value);
    } else if(furi_string_equal_str(key_str, "u5_hardware_uid")) {
        device_info->serial_number = furi_string_alloc_printf(value);
    } else if(furi_string_equal_str(key_str, "u5_firmware_target")) {
        device_info->hw_revision = furi_string_alloc_printf(value);
    }

    furi_string_free(key_str);
}

static bool ble_service_device_info_init_u5(void* object) {
    furi_assert(object);

    BLE_LOG_D("device_info_init");

    BleServiceObject* instance = object;

    BleDeviceInfoDataContext device_info;

    furi_hal_info_get(device_info_callback, '_', &device_info);

    BleCharacteristicObject* ch = instance->chars[BleSrvDeviceInfoCharacterIndexSerialNumber];
    ble_characteristic_set_data(
        ch,
        furi_string_get_cstr(device_info.serial_number),
        furi_string_size(device_info.serial_number));

    ch = instance->chars[BleSrvDeviceInfoCharacterIndexHardwareRevision];
    ble_characteristic_set_data(
        ch,
        furi_string_get_cstr(device_info.hw_revision),
        furi_string_size(device_info.hw_revision));

    ch = instance->chars[BleSrvDeviceInfoCharacterIndexSoftwareRevision];
    ble_characteristic_set_data(
        ch,
        furi_string_get_cstr(device_info.sw_revision),
        furi_string_size(device_info.sw_revision));

    furi_string_free(device_info.hw_revision);
    furi_string_free(device_info.sw_revision);
    furi_string_free(device_info.serial_number);

    return true;
}
#endif

//==========================================================
static const BleCharacteristicConfig device_info_service_characteristics[] = {
    {
        .intercom_index = BleSrvDeviceInfoCharacterIndexSerialNumber,
        .name = "Serial Number",
#if defined(BSB_MCU_SI917)
        .uuid = {.Char_UUID_16 = 0x2A25},
        .uuid_size = 2,
        .char_properties = BLE_ATT_PROPERTY_READ,
#endif
    },
    {
        .intercom_index = BleSrvDeviceInfoCharacterIndexHardwareRevision,
        .name = "Hardware Revision",
#if defined(BSB_MCU_SI917)
        .uuid = {.Char_UUID_16 = 0x2A27},
        .uuid_size = 2,
        .char_properties = BLE_ATT_PROPERTY_READ,
#endif
    },
    {
        .intercom_index = BleSrvDeviceInfoCharacterIndexSoftwareRevision,
        .name = "Software Revision",
#if defined(BSB_MCU_SI917)
        .uuid = {.Char_UUID_16 = 0x2A26},
        .uuid_size = 2,
        .char_properties = BLE_ATT_PROPERTY_READ,
#endif
    },
};

const BleServiceConfig ble_service_config_device_info = {
    .name = "Device Information",
#if defined(BSB_MCU_SI917)
    .uuid = {.Char_UUID_16 = 0x180A},
    .uuid_size = 2,
    .init = ble_service_device_info_init_917,
#else
    .init = ble_service_device_info_init_u5,
#endif
    .index = BleServiceIndexDeviceInfo,
    .char_count = COUNT_OF(device_info_service_characteristics),
    .char_configs = device_info_service_characteristics,
};
