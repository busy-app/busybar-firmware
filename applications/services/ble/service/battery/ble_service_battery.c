#include "ble_service_battery_i.h"

#define TAG "BleBattery"

//==========================================================
static const BleCharacteristicConfig battery_service_characteristics[] = {
    {
        .intercom_index = BleSrvBatteryCharacterIndexBatteryLevel,
        .name = "Battery Level",
        .initial_data_size = sizeof(uint8_t),
#if defined(BSB_MCU_SI917)
        .uuid = {.Char_UUID_16 = 0x2A19},
        .uuid_size = 2,
        .char_properties = BLE_ATT_PROPERTY_READ | BLE_ATT_PROPERTY_NOTIFY,
#endif
    },
    {
        .intercom_index = BleSrvBatteryCharacterIndexBatteryStatus,
        .name = "Battery Status",
        .initial_data_size = sizeof(BatteryStatusInfo),
#if defined(BSB_MCU_SI917)
        .uuid = {.Char_UUID_16 = 0x2BED},
        .uuid_size = 2,
        .char_properties = BLE_ATT_PROPERTY_READ | BLE_ATT_PROPERTY_NOTIFY,
#endif
    },
};

const BleServiceConfig ble_service_config_battery = {
    .name = "Battery Service",
#if defined(BSB_MCU_SI917)
    .uuid = {.Char_UUID_16 = 0x180F},
    .uuid_size = 2,
#endif
    .init = ble_service_battery_init,
    .run = ble_service_battery_run,
    .index = BleServiceIndexBattery,
    .char_count = COUNT_OF(battery_service_characteristics),
    .char_configs = battery_service_characteristics,
};
