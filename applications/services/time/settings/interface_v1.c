#include "interface_v1.h"
#include <utz/utz.h>

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

#define TIME_DEFAULT_TIMEZONE "New York"

#define TIME_FORMAT_DEFAULT TimeSettingTimeFormat24h

static const char* const time_format_string_map[] = {
    [TimeSettingTimeFormat24h] = "24h",
    [TimeSettingTimeFormat12h] = "12h",
};

static_assert(COUNT_OF(time_format_string_map) == TimeSettingTimeFormatCount);

typedef struct {
    int min;
    int max;
} IntMinMaxValidationContext;

static bool int_min_max_validate(const SettingProviderSetting* setting, int value) {
    const IntMinMaxValidationContext* context = setting->context;
    return value >= context->min && value <= context->max;
}

static bool
    serialize_uzone(const SettingProviderSetting* setting, const void* value, FuriString* string) {
    UNUSED(setting);

    const utz_zone_t* zone = value;

    furi_string_set_str(string, zone->name);

    return true;
}

static bool
    deserialize_uzone(const SettingProviderSetting* setting, const char* string, void* value) {
    UNUSED(setting);

    utz_zone_t* zone = value;

    return utz_get_zone_by_name(string, zone);
}

void time_v1_settings_default_timezone(void* container) {
    furi_check(utz_get_zone_by_name(TIME_DEFAULT_TIMEZONE, container));
}

const SettingProviderSetting time_v1_settings[] = {
    [TimeSettingV1IdxIsEnabled] =
        {
            .name = "is_enabled",
            .interface =
                &(const SettingProviderBoolInterface){
                    .default_value = IS_ENABLED_DEFAULT,
                },
            .field_offset = offsetof(TimeSettingsV1, is_enabled),
            .type = SettingProviderSettingTypeBool,
        },
    [TimeSettingV1IdxServerAddress] =
        {
            .name = "server_address",
            .interface =
                &(const SettingProviderStringInterface){
                    .default_value = SERVER_ADDRESS_DEFAULT,
                    .max_size = SIZEOF_MEMBER(TimeSettingsV1, server_address),
                },
            .field_offset = offsetof(TimeSettingsV1, server_address),
            .type = SettingProviderSettingTypeString,
        },
    [TimeSettingV1IdxTimezone] =
        {
            .name = "timezone",
            .interface =
                &(const SettingProviderCustomInterface){
                    .serialize_callback = serialize_uzone,
                    .deserialize_callback = deserialize_uzone,
                    .default_value_callback = time_v1_settings_default_timezone,
                    .default_value_size = sizeof(utz_zone_t),
                },
            .field_offset = offsetof(TimeSettingsV1, timezone),
            .type = SettingProviderSettingTypeCustom,
        },
    [TimeSettingV1IdxBootDelay] =
        {
            .name = "boot_delay",
            .interface =
                &(const SettingProviderIntInterface){
                    .is_valid_callback = int_min_max_validate,
                    .default_value = BOOT_DELAY_DEFAULT,
                },
            .context =
                &(const IntMinMaxValidationContext){
                    .min = BOOT_DELAY_MIN,
                    .max = BOOT_DELAY_MAX,
                },
            .field_offset = offsetof(TimeSettingsV1, boot_delay),
            .type = SettingProviderSettingTypeInt,
        },
    [TimeSettingV1IdxBackgroundSyncInterval] =
        {
            .name = "background_sync_interval",
            .interface =
                &(const SettingProviderIntInterface){
                    .is_valid_callback = int_min_max_validate,
                    .default_value = BACKGROUND_SYNC_INTERVAL_DEFAULT,
                },
            .context =
                &(const IntMinMaxValidationContext){
                    .min = BACKGROUND_SYNC_INTERVAL_MIN,
                    .max = BACKGROUND_SYNC_INTERVAL_MAX,
                },
            .field_offset = offsetof(TimeSettingsV1, background_sync_interval),
            .type = SettingProviderSettingTypeInt,
        },
    [TimeSettingV1IdxRetrySyncInterval] =
        {
            .name = "retry_sync_interval",
            .interface =
                &(const SettingProviderIntInterface){
                    .is_valid_callback = int_min_max_validate,
                    .default_value = RETRY_SYNC_INTERVAL_DEFAULT,
                },
            .context =
                &(const IntMinMaxValidationContext){
                    .min = RETRY_SYNC_INTERVAL_MIN,
                    .max = RETRY_SYNC_INTERVAL_MAX,
                },
            .field_offset = offsetof(TimeSettingsV1, retry_sync_interval),
            .type = SettingProviderSettingTypeInt,
        },
    [TimeSettingV1IdxTimeFormat] =
        {
            .name = "time_format",
            .interface =
                &(const SettingProviderEnumInterface){
                    .string_map = time_format_string_map,
                    .string_map_length = COUNT_OF(time_format_string_map),
                    .type_size = SIZEOF_MEMBER(TimeSettingsV1, time_format),
                    .default_value = &(const TimeSettingTimeFormat){TIME_FORMAT_DEFAULT},
                },
            .field_offset = offsetof(TimeSettingsV1, time_format),
            .type = SettingProviderSettingTypeEnum,
        },
};

static_assert(COUNT_OF(time_v1_settings) == TimeSettingV1IdxsCount);

const SettingProviderSetting time_v1_settings_root = {
    .name = NULL,
    .interface =
        &(const SettingProviderStructInterface){
            .inner_settings = time_v1_settings,
            .inner_settings_count = COUNT_OF(time_v1_settings),
        },
    .field_offset = 0,
    .type = SettingProviderSettingTypeStruct,
};
