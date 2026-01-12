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

typedef struct {
    const char* const* strings;
    int strings_count;
} EnumSettingContext;

static const char* const security_mode_map[] = {
    [WifiSecurityModeOpen] = "open",
    [WifiSecurityModeWpa] = "wpa",
    [WifiSecurityModeWpa2] = "wpa2",
    [WifiSecurityModeWep] = "wep",
    [WifiSecurityModeWpaWpa2Mixed] = "wpa_wpa2_mixed",
    [WifiSecurityModeWpa3] = "wpa3",
    [WifiSecurityModeWpa3Transition] = "wpa3_transition",
};

static const char* const ip_management_map[] = {
    [WifiIpManagementStatic] = "static",
    [WifiIpManagementDynamic] = "dhcp",
};

static const char* const ip_type_map[] = {
    [WifiIpTypeV4] = "v4",
    [WifiIpTypeV6] = "v6",
};

static bool enum_setting_serialize(
    const SettingProviderSetting* setting,
    FuriString* string,
    const void* value) {
    const EnumSettingContext* context = setting->context;
    const SettingProviderCustomInterface* interface = setting->interface;

    int _value = 0;
    memcpy(&_value, value, interface->value_size);

    bool is_success;
    if(_value < context->strings_count) {
        furi_string_set(string, context->strings[_value]);
        is_success = true;
    } else {
        is_success = false;
    }

    return is_success;
}

static bool enum_setting_deserialize(
    const SettingProviderSetting* setting,
    void* value,
    const FuriString* string) {
    const EnumSettingContext* context = setting->context;
    const SettingProviderCustomInterface* interface = setting->interface;

    const char* _string = furi_string_get_cstr(string);

    bool is_success = false;
    for(int i = 0; i < context->strings_count; i++) {
        if(strcasecmp(_string, context->strings[i]) == 0) {
            memcpy(value, &i, interface->value_size);
            is_success = true;
            break;
        }
    }

    return is_success;
}

static bool
    ipv4_serialize(const SettingProviderSetting* setting, FuriString* string, const void* value) {
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
    ipv4_deserialize(const SettingProviderSetting* setting, void* value, const FuriString* string) {
    UNUSED(setting);

    WifiIpv4* ipv4 = value;
    unsigned int _ipv4[COUNT_OF(ipv4->bytes)];
    const int read_count = sscanf(
        furi_string_get_cstr(string), "%u.%u.%u.%u", &_ipv4[0], &_ipv4[1], &_ipv4[2], &_ipv4[3]);

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
    ipv6_serialize(const SettingProviderSetting* setting, FuriString* string, const void* value) {
    UNUSED(setting);
    UNUSED(value);
    UNUSED(string);

    furi_string_set_str(string, "not implemented");

    return true;
}

static bool
    ipv6_deserialize(const SettingProviderSetting* setting, void* value, const FuriString* string) {
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
                .max_length = SIZEOF_MEMBER(WifiCredentials, ssid),
                .is_valid_callback = NULL,
            },
        .field_offset = offsetof(WifiCredentials, ssid),
        .type = SettingProviderSettingTypeString,
    },
    {
        .name = "passphrase",
        .interface =
            &(const SettingProviderStringInterface){
                .default_value = DEFAULT_PASSPHRASE,
                .max_length = SIZEOF_MEMBER(WifiCredentials, passphrase),
                .is_valid_callback = NULL,
            },
        .field_offset = offsetof(WifiCredentials, passphrase),
        .type = SettingProviderSettingTypeString,
    },
    {
        .name = "security",
        .interface =
            &(const SettingProviderCustomInterface){
                .default_value = &(const WifiSecurityMode){DEFAULT_SECURITY_MODE},
                .serialize_callback = enum_setting_serialize,
                .deserialize_callback = enum_setting_deserialize,
                .value_size = SIZEOF_MEMBER(WifiCredentials, security_mode),
            },
        .context =
            &(EnumSettingContext){
                .strings = security_mode_map,
                .strings_count = COUNT_OF(security_mode_map),
            },
        .field_offset = offsetof(WifiCredentials, security_mode),
        .type = SettingProviderSettingTypeCustom,
    },
};

static const SettingProviderSetting ipv4_settings[] = {
    {
        .name = "address",
        .interface =
            &(const SettingProviderCustomInterface){
                .default_value = &(const WifiIpv4){.bytes = DEFAULT_IP4_ADDRESS},
                .serialize_callback = ipv4_serialize,
                .deserialize_callback = ipv4_deserialize,
                .value_size = SIZEOF_MEMBER(WifiIpv4Settings, address),
            },
        .field_offset = offsetof(WifiIpv4Settings, address),
        .type = SettingProviderSettingTypeCustom,
    },
    {
        .name = "mask",
        .interface =
            &(const SettingProviderCustomInterface){
                .default_value = &(const WifiIpv4){.bytes = DEFAULT_IP4_MASK},
                .serialize_callback = ipv4_serialize,
                .deserialize_callback = ipv4_deserialize,
                .value_size = SIZEOF_MEMBER(WifiIpv4Settings, mask),
            },
        .field_offset = offsetof(WifiIpv4Settings, mask),
        .type = SettingProviderSettingTypeCustom,
    },
    {
        .name = "gateway",
        .interface =
            &(const SettingProviderCustomInterface){
                .default_value = &(const WifiIpv4){.bytes = DEFAULT_IP4_GATEWAY},
                .serialize_callback = ipv4_serialize,
                .deserialize_callback = ipv4_deserialize,
                .value_size = SIZEOF_MEMBER(WifiIpv4Settings, gateway),
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
                .default_value = &(const WifiIpv6){.bytes = DEFAULT_IP6_LOCAL},
                .serialize_callback = ipv6_serialize,
                .deserialize_callback = ipv6_deserialize,
                .value_size = SIZEOF_MEMBER(WifiIpv6Settings, local),
            },
        .field_offset = offsetof(WifiIpv6Settings, local),
        .type = SettingProviderSettingTypeCustom,
    },
    {
        .name = "global",
        .interface =
            &(const SettingProviderCustomInterface){
                .default_value = &(const WifiIpv6){.bytes = DEFAULT_IP6_GLOBAL},
                .serialize_callback = ipv6_serialize,
                .deserialize_callback = ipv6_deserialize,
                .value_size = SIZEOF_MEMBER(WifiIpv6Settings, global),
            },
        .field_offset = offsetof(WifiIpv6Settings, global),
        .type = SettingProviderSettingTypeCustom,
    },
    {
        .name = "gateway",
        .interface =
            &(const SettingProviderCustomInterface){
                .default_value = &(const WifiIpv6){.bytes = DEFAULT_IP6_GATEWAY},
                .serialize_callback = ipv6_serialize,
                .deserialize_callback = ipv6_deserialize,
                .value_size = SIZEOF_MEMBER(WifiIpv6Settings, gateway),
            },
        .field_offset = offsetof(WifiIpv6Settings, gateway),
        .type = SettingProviderSettingTypeCustom,
    },
};

static const SettingProviderSetting ip_config_settings[] = {
    {
        .name = "management",
        .interface =
            &(const SettingProviderCustomInterface){
                .default_value = &(const WifiIpManagement){DEFAULT_IP_MANAGEMENT},
                .serialize_callback = enum_setting_serialize,
                .deserialize_callback = enum_setting_deserialize,
                .value_size = SIZEOF_MEMBER(WifiIpConfig, mgmt),
            },
        .context =
            &(EnumSettingContext){
                .strings = ip_management_map,
                .strings_count = COUNT_OF(ip_management_map),
            },
        .field_offset = offsetof(WifiIpConfig, mgmt),
        .type = SettingProviderSettingTypeCustom,
    },
    {
        .name = "type",
        .interface =
            &(const SettingProviderCustomInterface){
                .default_value = &(const WifiIpType){DEFAULT_IP_TYPE},
                .serialize_callback = enum_setting_serialize,
                .deserialize_callback = enum_setting_deserialize,
                .value_size = SIZEOF_MEMBER(WifiIpConfig, type),
            },
        .context =
            &(EnumSettingContext){
                .strings = ip_type_map,
                .strings_count = COUNT_OF(ip_type_map),
            },
        .field_offset = offsetof(WifiIpConfig, type),
        .type = SettingProviderSettingTypeCustom,
    },
    {
        .name = "ipv4",
        .interface =
            &(const SettingProviderStructureInterface){
                .is_valid_callback = NULL,
                .inner_settings = ipv4_settings,
                .inner_settings_count = COUNT_OF(ipv4_settings),
            },
        .field_offset = offsetof(WifiIpConfig, ip4),
        .type = SettingProviderSettingTypeStructure,
    },
    {
        .name = "ipv6",
        .interface =
            &(const SettingProviderStructureInterface){
                .is_valid_callback = NULL,
                .inner_settings = ipv6_settings,
                .inner_settings_count = COUNT_OF(ipv6_settings),
            },
        .field_offset = offsetof(WifiIpConfig, ip6),
        .type = SettingProviderSettingTypeStructure,
    },
};

const SettingProviderSetting wifi_v1_settings[] = {
    [WifiSettingV1IdxCredentials] =
        {
            .name = "credentials",
            .interface =
                &(const SettingProviderStructureInterface){
                    .is_valid_callback = NULL,
                    .inner_settings = credentials_settings,
                    .inner_settings_count = COUNT_OF(credentials_settings),
                },
            .field_offset = offsetof(WifiSettingsV1, credentials),
            .type = SettingProviderSettingTypeStructure,
        },
    [WifiSettingV1IdxIpConfig] =
        {
            .name = "ip_config",
            .interface =
                &(const SettingProviderStructureInterface){
                    .is_valid_callback = NULL,
                    .inner_settings = ip_config_settings,
                    .inner_settings_count = COUNT_OF(ip_config_settings),
                },
            .field_offset = offsetof(WifiSettingsV1, ip_config),
            .type = SettingProviderSettingTypeStructure,
        },
};

const SettingProviderSetting wifi_v1_settings_root = {
    .name = NULL,
    .interface =
        &(const SettingProviderStructureInterface){
            .is_valid_callback = NULL,
            .inner_settings = wifi_v1_settings,
            .inner_settings_count = COUNT_OF(wifi_v1_settings),
        },
    .field_offset = 0,
    .type = SettingProviderSettingTypeStructure,
};

static_assert(COUNT_OF(wifi_v1_settings) == WifiSettingV1IdxsCount);
