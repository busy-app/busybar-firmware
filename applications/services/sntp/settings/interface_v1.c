#include "interface_v1.h"

#define IS_ENABLED_DEFAULT true

#define SERVER_ADDRESS_DEFAULT "udp://time.busy.app:123"

#define BOOT_DELAY_MIN     (1 * 60)
#define BOOT_DELAY_MAX     (10 * 60)
#define BOOT_DELAY_DEFAULT (5 * 60)

#define BACKGROUND_SYNC_INTERVAL_MIN     (2 * 60 * 60)
#define BACKGROUND_SYNC_INTERVAL_MAX     (48 * 60 * 60)
#define BACKGROUND_SYNC_INTERVAL_DEFAULT (3 * 60 * 60)

#define RETRY_SYNC_INTERVAL_MIN     (5 * 60)
#define RETRY_SYNC_INTERVAL_MAX     (60 * 60)
#define RETRY_SYNC_INTERVAL_DEFAULT (10 * 60)

static bool boot_delay_is_valid(const SettingProviderSetting* setting, int value) {
    UNUSED(setting);

    return (value >= BOOT_DELAY_MIN && value <= BOOT_DELAY_MAX);
}

static bool background_sync_interval_is_valid(const SettingProviderSetting* setting, int value) {
    UNUSED(setting);

    return (value >= BACKGROUND_SYNC_INTERVAL_MIN && value <= BACKGROUND_SYNC_INTERVAL_MAX);
}

static bool retry_sync_interval_is_valid(const SettingProviderSetting* setting, int value) {
    UNUSED(setting);

    return (value >= RETRY_SYNC_INTERVAL_MIN && value <= RETRY_SYNC_INTERVAL_MAX);
}

static bool serialize_uzone(
    const SettingProviderSetting* setting,
    FuriString* string,
    const void* value) {
    UNUSED(setting);

    const uzone_t *zone = value;

    furi_string_set_str(string, zone->name);

    return true;
}

bool deserialize_uzone(
    const SettingProviderSetting* setting,
    void* value,
    const FuriString* string) {
    UNUSED(setting);

    uzone_t *zone = value;

    return utz_get_zone_by_name(furi_string_get_cstr(string), zone);
}

size_t default_value_size;

const SettingProviderSetting sntp_v1_settings[] = {
    [SntpSettingV1IdxIsEnabled] =
        {
            .name = "is_enabled",
            .interface =
                &(const SettingProviderBoolInterface){
                    .default_value = IS_ENABLED_DEFAULT,
                },
            .field_offset = offsetof(SntpSettingsV1, is_enabled),
            .type = SettingProviderSettingTypeBool,
        },
    [SntpSettingV1IdxServerAddress] =
        {
            .name = "server_address",
            .interface =
                &(const SettingProviderStringInterface){
                    .default_value = SERVER_ADDRESS_DEFAULT,
                    .is_valid_callback = NULL,
                    .max_length = SIZEOF_MEMBER(SntpSettingsV1, server_address),
                },
            .field_offset = offsetof(SntpSettingsV1, server_address),
            .type = SettingProviderSettingTypeString,
        },
    [SntpSettingV1IdxTimezone] =
        {
            .name = "timezone",
            .interface =
                &(const SettingProviderCustomInterface){
                    .default_value = &utz_zone_default,
                    .default_value_size = sizeof(utz_zone_default),
                    .serialize_callback = serialize_uzone,
                    .deserialize_callback = deserialize_uzone,
                },
            .field_offset = offsetof(SntpSettingsV1, timezone),
            .type = SettingProviderSettingTypeCustom,
        },
    [SntpSettingV1IdxBootDelay] =
        {
            .name = "boot_delay",
            .interface =
                &(const SettingProviderIntInterface){
                    .default_value = BOOT_DELAY_DEFAULT,
                    .is_valid_callback = boot_delay_is_valid,
                },
            .field_offset = offsetof(SntpSettingsV1, boot_delay),
            .type = SettingProviderSettingTypeInt,
        },
    [SntpSettingV1IdxBackgroundSyncInterval] =
        {
            .name = "background_sync_interval",
            .interface =
                &(const SettingProviderIntInterface){
                    .default_value = BACKGROUND_SYNC_INTERVAL_DEFAULT,
                    .is_valid_callback = background_sync_interval_is_valid,
                },
            .field_offset = offsetof(SntpSettingsV1, background_sync_interval),
            .type = SettingProviderSettingTypeInt,
        },
    [SntpSettingV1IdxRetrySyncInterval] =
        {
            .name = "retry_sync_interval",
            .interface =
                &(const SettingProviderIntInterface){
                    .default_value = RETRY_SYNC_INTERVAL_DEFAULT,
                    .is_valid_callback = retry_sync_interval_is_valid,
                },
            .field_offset = offsetof(SntpSettingsV1, retry_sync_interval),
            .type = SettingProviderSettingTypeInt,
        },
};

static_assert(COUNT_OF(sntp_v1_settings) == SntpSettingV1IdxsCount);

const SettingProviderSetting sntp_v1_settings_root = {
    .name = NULL,
    .interface =
        &(const SettingProviderStructureInterface){
            .is_valid_callback = NULL,
            .inner_settings = sntp_v1_settings,
            .inner_settings_count = COUNT_OF(sntp_v1_settings),
        },
    .field_offset = 0,
    .type = SettingProviderSettingTypeStructure,
};
