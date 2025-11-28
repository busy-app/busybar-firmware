#include "ble_service_device_events_i.h"

#define UUID_VALUE(i) \
    {0xAF, 0x56, 0x9d, i, 0x71, 0x6a, 0x45, 0x2d, 0xbe, 0x64, 0x66, 0xe4, 0x65, 0x76, 0x6c, 0x29}

#define EVENT_SERVICE_UUID UUID_VALUE(0)

#define EVENT_SERVICE_FLAGS_CHAR_UUID UUID_VALUE(1)

//==========================================================
static const BleCharacteristicDescriptor device_events_service_characteristics[] = {
    {
        .intercom_index = BleSrvDeviceEventsCharacterIndexFlags,
        .name = "Event Flags",
        .initial_data_size = sizeof(BleServiceDeviceEvents),
#if defined(BSB_MCU_SI917)
        .uuid = {.Char_UUID_128 = EVENT_SERVICE_FLAGS_CHAR_UUID},
        .uuid_size = 16,
        ///TODO: maybe BLE_ATT_PROPERTY_READ is optional in current context, and can be removed
        .char_properties = BLE_ATT_PROPERTY_READ | BLE_ATT_PROPERTY_INDICATE,
#endif
    },
};

const BleServiceDescriptor ble_service_config_device_events = {
    .name = "Device Events",
#if defined(BSB_MCU_SI917)
    .uuid = {.Char_UUID_128 = EVENT_SERVICE_UUID},
    .uuid_size = 16,
#endif
    .init = ble_service_device_events_init,
    .run = ble_service_device_events_run,
    .index = BleServiceIndexDeviceEvents,
    .init_method = BleServiceInitMethodRemote,
    .char_count = COUNT_OF(device_events_service_characteristics),
    .char_descriptors = device_events_service_characteristics,
};
