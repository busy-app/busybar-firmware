#include "interface_v1.h"

#define DEFAULT_SSID          ""
#define DEFAULT_PASSPHRASE    ""
#define DEFAULT_SECURITY_MODE WifiSecurityModeOpen

#define DEFAULT_IP_MANAGEMENT WifiIpManagementDynamic
#define DEFAULT_IP_TYPE       WifiIpTypeV4

#define DEFAULT_IP4_ADDRESS {0, 0, 0, 0}
#define DEFAULT_IP4_GATEWAY {0, 0, 0, 0}
#define DEFAULT_IP4_MASK    {0, 0, 0, 0}

#define DEFAULT_IP6_LOCAL   {0, 0, 0, 0}
#define DEFAULT_IP6_GLOBAL  {0, 0, 0, 0}
#define DEFAULT_IP6_GATEWAY {0, 0, 0, 0}

static const char* const security_mode_string_map[] = {
    [WifiSecurityModeOpen] = "open",
    [WifiSecurityModeWpa] = "wpa",
    [WifiSecurityModeWpa2] = "wpa2",
    [WifiSecurityModeWep] = "wep",
    [WifiSecurityModeWpaWpa2Mixed] = "wpa_wpa2_mixed",
    [WifiSecurityModeWpa3] = "wpa3",
    [WifiSecurityModeWpa3Transition] = "wpa3_transition",
    [WifiSecurityModeUnsupported] = "unsupported",
};

static_assert(COUNT_OF(security_mode_string_map) == WifiSecurityModeMax);

static const char* const ip_management_string_map[] = {
    [WifiIpManagementStatic] = "static",
    [WifiIpManagementDynamic] = "dhcp",
};

static_assert(COUNT_OF(ip_management_string_map) == WifiIpManagementMax);

static const char* const ip_type_string_map[] = {
    [WifiIpTypeV4] = "v4",
    [WifiIpTypeV6] = "v6",
};

static_assert(COUNT_OF(ip_type_string_map) == WifiIpTypeMax);

static bool
    ipv4_serialize(const SettingProviderSetting* setting, const void* value, FuriString* string) {
    UNUSED(setting);

    const WifiIpv4* ipv4 = value;
    furi_string_printf(
        string,
        "%hhu.%hhu.%hhu.%hhu",
        ipv4->bytes[0],
        ipv4->bytes[1],
        ipv4->bytes[2],
        ipv4->bytes[3]);

    return true;
}

static bool
    ipv4_deserialize(const SettingProviderSetting* setting, const char* string, void* value) {
    UNUSED(setting);

    WifiIpv4* ipv4 = value;
    unsigned int _ipv4[COUNT_OF(ipv4->bytes)];
    const int read_count =
        sscanf(string, "%u.%u.%u.%u", &_ipv4[0], &_ipv4[1], &_ipv4[2], &_ipv4[3]);

    bool is_success = (read_count == COUNT_OF(_ipv4));
    for(size_t i = 0; i < COUNT_OF(_ipv4) && is_success; i++) {
        unsigned int ipv4_octet = _ipv4[i];

        if(ipv4_octet <= UINT8_MAX) {
            ipv4->bytes[i] = ipv4_octet;
        } else {
            is_success = false;
        }
    }

    return is_success;
}

static bool
    ipv6_serialize(const SettingProviderSetting* setting, const void* value, FuriString* string) {
    UNUSED(setting);
    UNUSED(value);
    UNUSED(string);

    furi_string_set_str(string, "not implemented");

    return true;
}

static bool
    ipv6_deserialize(const SettingProviderSetting* setting, const char* string, void* value) {
    UNUSED(setting);
    UNUSED(value);
    UNUSED(string);

    return true;
}

static const SettingProviderSetting credentials_settings[] = {
    {
        .name = "ssid",
        .interface =
            &(const SettingProviderStringInterface){
                .default_value = DEFAULT_SSID,
                .max_size = SIZEOF_MEMBER(WifiCredentials, ssid),
            },
        .field_offset = offsetof(WifiCredentials, ssid),
        .type = SettingProviderSettingTypeString,
    },
    {
        .name = "passphrase",
        .interface =
            &(const SettingProviderStringInterface){
                .default_value = DEFAULT_PASSPHRASE,
                .max_size = SIZEOF_MEMBER(WifiCredentials, passphrase),
            },
        .field_offset = offsetof(WifiCredentials, passphrase),
        .type = SettingProviderSettingTypeString,
    },
    {
        .name = "security",
        .interface =
            &(const SettingProviderEnumInterface){
                .string_map = security_mode_string_map,
                .string_map_length = COUNT_OF(security_mode_string_map),
                .type_size = SIZEOF_MEMBER(WifiCredentials, security_mode),
                .default_value = &(const WifiSecurityMode){DEFAULT_SECURITY_MODE},
            },
        .field_offset = offsetof(WifiCredentials, security_mode),
        .type = SettingProviderSettingTypeEnum,
    },
};

static const SettingProviderSetting ipv4_settings[] = {
    {
        .name = "address",
        .interface =
            &(const SettingProviderCustomInterface){
                .serialize_callback = ipv4_serialize,
                .deserialize_callback = ipv4_deserialize,
                .default_value = &(const WifiIpv4){.bytes = DEFAULT_IP4_ADDRESS},
                .default_value_size = SIZEOF_MEMBER(WifiIpv4Settings, address),
            },
        .field_offset = offsetof(WifiIpv4Settings, address),
        .type = SettingProviderSettingTypeCustom,
    },
    {
        .name = "mask",
        .interface =
            &(const SettingProviderCustomInterface){
                .serialize_callback = ipv4_serialize,
                .deserialize_callback = ipv4_deserialize,
                .default_value = &(const WifiIpv4){.bytes = DEFAULT_IP4_MASK},
                .default_value_size = SIZEOF_MEMBER(WifiIpv4Settings, mask),
            },
        .field_offset = offsetof(WifiIpv4Settings, mask),
        .type = SettingProviderSettingTypeCustom,
    },
    {
        .name = "gateway",
        .interface =
            &(const SettingProviderCustomInterface){
                .serialize_callback = ipv4_serialize,
                .deserialize_callback = ipv4_deserialize,
                .default_value = &(const WifiIpv4){.bytes = DEFAULT_IP4_GATEWAY},
                .default_value_size = SIZEOF_MEMBER(WifiIpv4Settings, gateway),
            },
        .field_offset = offsetof(WifiIpv4Settings, gateway),
        .type = SettingProviderSettingTypeCustom,
    },
};

static const SettingProviderSetting ipv6_settings[] = {
    {
        .name = "local",
        .interface =
            &(const SettingProviderCustomInterface){
                .serialize_callback = ipv6_serialize,
                .deserialize_callback = ipv6_deserialize,
                .default_value = &(const WifiIpv6){.bytes = DEFAULT_IP6_LOCAL},
                .default_value_size = SIZEOF_MEMBER(WifiIpv6Settings, local),
            },
        .field_offset = offsetof(WifiIpv6Settings, local),
        .type = SettingProviderSettingTypeCustom,
    },
    {
        .name = "global",
        .interface =
            &(const SettingProviderCustomInterface){
                .serialize_callback = ipv6_serialize,
                .deserialize_callback = ipv6_deserialize,
                .default_value = &(const WifiIpv6){.bytes = DEFAULT_IP6_GLOBAL},
                .default_value_size = SIZEOF_MEMBER(WifiIpv6Settings, global),
            },
        .field_offset = offsetof(WifiIpv6Settings, global),
        .type = SettingProviderSettingTypeCustom,
    },
    {
        .name = "gateway",
        .interface =
            &(const SettingProviderCustomInterface){
                .serialize_callback = ipv6_serialize,
                .deserialize_callback = ipv6_deserialize,
                .default_value = &(const WifiIpv6){.bytes = DEFAULT_IP6_GATEWAY},
                .default_value_size = SIZEOF_MEMBER(WifiIpv6Settings, gateway),
            },
        .field_offset = offsetof(WifiIpv6Settings, gateway),
        .type = SettingProviderSettingTypeCustom,
    },
};

static const SettingProviderSetting ip_config_settings[] = {
    {
        .name = "management",
        .interface =
            &(const SettingProviderEnumInterface){
                .string_map = ip_management_string_map,
                .string_map_length = COUNT_OF(ip_management_string_map),
                .type_size = SIZEOF_MEMBER(WifiIpConfig, mgmt),
                .default_value = &(const WifiIpManagement){DEFAULT_IP_MANAGEMENT},
            },
        .field_offset = offsetof(WifiIpConfig, mgmt),
        .type = SettingProviderSettingTypeEnum,
    },
    {
        .name = "type",
        .interface =
            &(const SettingProviderEnumInterface){
                .string_map = ip_type_string_map,
                .string_map_length = COUNT_OF(ip_type_string_map),
                .type_size = SIZEOF_MEMBER(WifiIpConfig, type),
                .default_value = &(const WifiIpType){DEFAULT_IP_TYPE},
            },
        .field_offset = offsetof(WifiIpConfig, type),
        .type = SettingProviderSettingTypeEnum,
    },
    {
        .name = "ipv4",
        .interface =
            &(const SettingProviderStructInterface){
                .inner_settings = ipv4_settings,
                .inner_settings_count = COUNT_OF(ipv4_settings),
            },
        .field_offset = offsetof(WifiIpConfig, ip4),
        .type = SettingProviderSettingTypeStruct,
    },
    {
        .name = "ipv6",
        .interface =
            &(const SettingProviderStructInterface){
                .inner_settings = ipv6_settings,
                .inner_settings_count = COUNT_OF(ipv6_settings),
            },
        .field_offset = offsetof(WifiIpConfig, ip6),
        .type = SettingProviderSettingTypeStruct,
    },
};

const SettingProviderSetting wifi_v1_settings[] = {
    [WifiSettingV1IdxCredentials] =
        {
            .name = "credentials",
            .interface =
                &(const SettingProviderStructInterface){
                    .inner_settings = credentials_settings,
                    .inner_settings_count = COUNT_OF(credentials_settings),
                },
            .field_offset = offsetof(WifiSettingsV1, credentials),
            .type = SettingProviderSettingTypeStruct,
        },
    [WifiSettingV1IdxIpConfig] =
        {
            .name = "ip_config",
            .interface =
                &(const SettingProviderStructInterface){
                    .inner_settings = ip_config_settings,
                    .inner_settings_count = COUNT_OF(ip_config_settings),
                },
            .field_offset = offsetof(WifiSettingsV1, ip_config),
            .type = SettingProviderSettingTypeStruct,
        },
};

const SettingProviderSetting wifi_v1_settings_root = {
    .name = NULL,
    .interface =
        &(const SettingProviderStructInterface){
            .inner_settings = wifi_v1_settings,
            .inner_settings_count = COUNT_OF(wifi_v1_settings),
        },
    .field_offset = 0,
    .type = SettingProviderSettingTypeStruct,
};

static_assert(COUNT_OF(wifi_v1_settings) == WifiSettingV1IdxsCount);
