#include "usb_network_settings_interface_v1.h"

typedef enum {
    UsbNetworkSettingsIpConfigV1IdxAddress,
    UsbNetworkSettingsIpConfigV1IdxNetmask,
    UsbNetworkSettingsIpConfigV1IdxGateway,
    UsbNetworkSettingsIpConfigV1IdxMax,
} UsbNetworkSettingsIpConfigV1Idx;

static bool usb_network_settings_v1_ip_address_serialize_callback(
    const SettingProviderSetting* setting,
    const void* value,
    FuriString* string) {
    UNUSED(setting);
    furi_assert(value);
    furi_assert(string);

    const UsbNetworkIpAddress* ip_address = value;
    const uint8_t* bytes = ip_address->bytes;

    furi_string_printf(string, "%hhu.%hhu.%hhu.%hhu", bytes[0], bytes[1], bytes[2], bytes[3]);

    return true;
}

static bool usb_network_settings_v1_ip_address_deserialize_callback(
    const SettingProviderSetting* setting,
    const char* string,
    void* value) {
    UNUSED(setting);
    furi_assert(string);
    furi_assert(value);

    bool success = false;

    do {
        uint32_t items[4];
        // sscanf() cannot read into uint8_t variables
        const int num_read =
            sscanf(string, "%lu.%lu.%lu.%lu", &items[0], &items[1], &items[2], &items[3]);

        if(num_read != COUNT_OF(items)) {
            break;
        }

        UsbNetworkIpAddress* ip_address = value;

        uint32_t num_written;
        for(num_written = 0; num_written < COUNT_OF(items); ++num_written) {
            const uint32_t item = items[num_written];
            if(item > UINT8_MAX) {
                break;
            }

            ip_address->bytes[num_written] = item;
        }

        if(num_written != COUNT_OF(items)) {
            break;
        }

        success = true;
    } while(false);

    return success;
}

static const SettingProviderSetting usb_network_settings_ip_config_v1[] = {
    [UsbNetworkSettingsIpConfigV1IdxAddress] =
        {
            .name = "address",
            .interface =
                &(const SettingProviderCustomInterface){
                    .serialize_callback = usb_network_settings_v1_ip_address_serialize_callback,
                    .deserialize_callback = usb_network_settings_v1_ip_address_deserialize_callback,
                    .default_value =
                        &(const UsbNetworkIpAddress){
                            .bytes = {10, 0, 4, 20},
                        },
                    .default_value_size = sizeof(UsbNetworkIpAddress),
                },
            .field_offset = offsetof(UsbNetworkIpConfig, address),
            .type = SettingProviderSettingTypeCustom,
        },
    [UsbNetworkSettingsIpConfigV1IdxNetmask] =
        {
            .name = "netmask",
            .interface =
                &(const SettingProviderCustomInterface){
                    .serialize_callback = usb_network_settings_v1_ip_address_serialize_callback,
                    .deserialize_callback = usb_network_settings_v1_ip_address_deserialize_callback,
                    .default_value =
                        &(const UsbNetworkIpAddress){
                            .bytes = {255, 255, 255, 0},
                        },
                    .default_value_size = sizeof(UsbNetworkIpAddress),
                },
            .field_offset = offsetof(UsbNetworkIpConfig, netmask),
            .type = SettingProviderSettingTypeCustom,
        },
    [UsbNetworkSettingsIpConfigV1IdxGateway] =
        {
            .name = "gateway",
            .interface =
                &(const SettingProviderCustomInterface){
                    .serialize_callback = usb_network_settings_v1_ip_address_serialize_callback,
                    .deserialize_callback = usb_network_settings_v1_ip_address_deserialize_callback,
                    .default_value =
                        &(const UsbNetworkIpAddress){
                            .bytes = {0, 0, 0, 0},
                        },
                    .default_value_size = sizeof(UsbNetworkIpAddress),
                },
            .field_offset = offsetof(UsbNetworkIpConfig, gateway),
            .type = SettingProviderSettingTypeCustom,
        },
};

static_assert(COUNT_OF(usb_network_settings_ip_config_v1) == UsbNetworkSettingsIpConfigV1IdxMax);

const SettingProviderSetting usb_network_settings_v1[] = {
    [UsbNetworkSettingsV1IdxIpConfig] =
        {
            .name = "ip_config",
            .interface =
                &(const SettingProviderStructInterface){
                    .inner_settings = usb_network_settings_ip_config_v1,
                    .inner_settings_count = COUNT_OF(usb_network_settings_ip_config_v1),
                },
            .field_offset = offsetof(UsbNetworkSettingsV1, ip_config),
            .type = SettingProviderSettingTypeStruct,
        },
};

const SettingProviderSetting usb_network_settings_v1_root = {
    .name = NULL,
    .interface =
        &(const SettingProviderStructInterface){
            .inner_settings = usb_network_settings_v1,
            .inner_settings_count = COUNT_OF(usb_network_settings_v1),
        },
    .type = SettingProviderSettingTypeStruct,
};

static_assert(COUNT_OF(usb_network_settings_v1) == UsbNetworkSettingsV1IdxMax);
