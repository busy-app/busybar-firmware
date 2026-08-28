#include "busy_timer_settings_interface_v1.h"

#include <toolbox/strint.h>

#include "../busy_timer_common_i.h"

#define STRING_VALUE_DEFAULT ""

#define SORT_ORDER_DEFAULT (-1)

typedef enum {
    BusyTimerSettingsV1IdxProfile,
    BusyTimerSettingsV1IdxDemoMode,
    BusyTimerSettingsV1IdxMax,
} BusyTimerSettingsV1Idx;

typedef enum {
    BusyTimerSettingsV1ProfileIdxAppConfig,
    BusyTimerSettingsV1ProfileIdxTimerConfig,
    BusyTimerSettingsV1ProfileIdxMetadata,
    BusyTimerSettingsV1ProfileIdxTimestamp,
    BusyTimerSettingsV1ProfileIdxMax,
} BusyTimerSettingsV1ProfileIdx;

typedef enum {
    BusyTimerSettingsV1AppConfigIdxThemeName,
    BusyTimerSettingsV1AppConfigIdxSmartHome,
    BusyTimerSettingsV1AppConfigIdxWorkOnly,
    BusyTimerSettingsV1AppConfigIdxShowWorkTime,
    BusyTimerSettingsV1AppConfigIdxMax,
} BusyTimerSettingsV1AppConfigIdx;

typedef enum {
    BusyTimerSettingsV1TimerConfigIdxMode,
    BusyTimerSettingsV1TimerConfigIdxTime,
    BusyTimerSettingsV1TimerConfigIdxWorkTime,
    BusyTimerSettingsV1TimerConfigIdxRestTime,
    BusyTimerSettingsV1TimerConfigIdxCycleCount,
    BusyTimerSettingsV1TimerConfigIdxEnableAutostart,
    BusyTimerSettingsV1TimerConfigIdxEnableDemoMode,
    BusyTimerSettingsV1TimerConfigIdxMax,
} BusyTimerSettingsV1TimerConfigIdx;

typedef enum {
    BusyTimerSettingsV1ProfileInfoIdxSortOrder,
    BusyTimerSettingsV1ProfileInfoIdxTitle,
    BusyTimerSettingsV1ProfileInfoIdxId,
    BusyTimerSettingsV1ProfileInfoIdxMax,
} BusyTimerSettingsV1ProfileInfoIdx;

// NOTE: Wastes ~264 bytes of flash space for convenience
static const BusyTimerSettingsV1 busy_timer_settings_v1_defaults[BusyTimerProfileIdMax] = {
    [BusyTimerProfileIdBusy] =
        {
            .profile =
                {
                    .app_config =
                        {
                            .theme_name = BUSY_APP_THEME_NAME_DEFAULT,
                            .is_show_work_only_enabled = false,
                            .is_smart_home_enabled = true,
                            .is_show_work_time_enabled = BUSY_APP_IS_SHOW_WORK_TIME_ENABLED_DEFAULT,
                        },
                    .timer_config =
                        {
                            .mode = BusyTimerModeInterval,
                            .interval =
                                {
                                    .work_time_ms = M_TO_MS(BUSY_TIMER_WORK_TIME_DEFAULT_MN),
                                    .rest_time_ms = M_TO_MS(BUSY_TIMER_REST_TIME_DEFAULT_MN),
                                    .cycles_count = BUSY_TIMER_CYCLE_COUNT_DEFAULT,
                                    .is_autostart_enabled = BUSY_TIMER_ENABLE_AUTOSTART_DEFAULT,
                                },
                        },
                    .metadata =
                        {
                            .sort_order = SORT_ORDER_DEFAULT,
                            .title = "BUSY",
                            .card_id = "00000000-0000-0000-0000-000000000000",
                        },
                    .timestamp_ms = 0,
                },
            .is_demo_mode_enabled = BUSY_TIMER_ENABLE_DEMO_MODE_DEFAULT,
        },
    [BusyTimerProfileIdCustom] =
        {
            .profile =
                {
                    .app_config =
                        {
                            .theme_name = BUSY_APP_THEME_NAME_CUSTOM_DEFAULT,
                            .is_show_work_only_enabled = true,
                            .is_smart_home_enabled = true,
                            .is_show_work_time_enabled = BUSY_APP_IS_SHOW_WORK_TIME_ENABLED_DEFAULT,
                        },
                    .timer_config =
                        {
                            .mode = BusyTimerModeInfinite,
                        },
                    .metadata =
                        {
                            .sort_order = SORT_ORDER_DEFAULT,
                            .title = "ZEN",
                            .card_id = "00000000-0000-0000-0000-000000000001",
                        },
                    .timestamp_ms = 0,
                },
            .is_demo_mode_enabled = BUSY_TIMER_ENABLE_DEMO_MODE_DEFAULT,
        },
};

static const BusyTimerConfig busy_timer_settings_v1_timer_config_default = {
    .mode = BusyTimerModeMax,
    .interval = {0},
};

static const time_t busy_timer_settings_v1_timestamp_default = 0;

static bool busy_timer_settings_v1_timer_config_serialize_cb(
    const SettingProviderSetting* setting,
    const void* value,
    cJSON* json) {
    UNUSED(setting);

    const BusyTimerConfig* timer_settings = value;

    if(timer_settings->mode == BusyTimerModeInfinite) {
        busy_timer_common_serialize_infinite_config(json);
    } else if(timer_settings->mode == BusyTimerModeSimple) {
        busy_timer_common_serialize_simple_config(json, &timer_settings->simple);
    } else if(timer_settings->mode == BusyTimerModeInterval) {
        busy_timer_common_serialize_interval_config(json, &timer_settings->interval);
    }

    return true;
}

static bool busy_timer_settings_v1_timer_config_deserialize_cb(
    const SettingProviderSetting* setting,
    const cJSON* json,
    void* value) {
    UNUSED(setting);

    bool success = false;

    do {
        if(!cJSON_IsObject(json)) {
            break;
        }

        BusyTimerConfig* timer_settings = value;

        if(!busy_timer_common_deserialize_timer_mode(json, &timer_settings->mode)) {
            break;
        }

        if(timer_settings->mode == BusyTimerModeSimple) {
            if(!busy_timer_common_deserialize_simple_config(json, &timer_settings->simple)) {
                break;
            }

        } else if(timer_settings->mode == BusyTimerModeInterval) {
            if(!busy_timer_common_deserialize_interval_config(json, &timer_settings->interval)) {
                break;
            }
        }

        success = true;
    } while(false);

    return success;
}

static bool busy_timer_settings_v1_timestamp_serialize_cb(
    const SettingProviderSetting* setting,
    const void* value,
    FuriString* string) {
    UNUSED(setting);

    const time_t timestamp = *(time_t*)value;
    furi_string_printf(string, "%llu", timestamp);

    return true;
}

static bool busy_timer_settings_v1_timestamp_deserialize_cb(
    const SettingProviderSetting* setting,
    const char* string,
    void* value) {
    UNUSED(setting);

    return strint_to_int64(string, NULL, value, 10) == StrintParseNoError;
}

static const SettingProviderSetting busy_timer_settings_v1_app_config[] = {
    [BusyTimerSettingsV1AppConfigIdxThemeName] =
        {
            .name = "theme_name",
            .interface =
                &(const SettingProviderStringInterface){
                    .default_value = STRING_VALUE_DEFAULT,
                    // Including zero terminator
                    .max_size = SIZEOF_MEMBER(BusyAppConfig, theme_name),
                },
            .field_offset = offsetof(BusyAppConfig, theme_name),
            .type = SettingProviderSettingTypeString,
        },
    [BusyTimerSettingsV1AppConfigIdxSmartHome] =
        {
            .name = "is_smart_home_enabled",
            .interface =
                &(const SettingProviderBoolInterface){
                    .default_value = BUSY_APP_IS_SMART_HOME_ENABLED_DEFAULT,
                },
            .field_offset = offsetof(BusyAppConfig, is_smart_home_enabled),
            .type = SettingProviderSettingTypeBool,
        },
    [BusyTimerSettingsV1AppConfigIdxWorkOnly] =
        {
            .name = "is_show_work_only_enabled",
            .interface =
                &(const SettingProviderBoolInterface){
                    .default_value = BUSY_APP_IS_SHOW_WORK_ONLY_ENABLED_DEFAULT,
                },
            .field_offset = offsetof(BusyAppConfig, is_show_work_only_enabled),
            .type = SettingProviderSettingTypeBool,
        },
    [BusyTimerSettingsV1AppConfigIdxShowWorkTime] =
        {
            .name = "is_show_work_time_enabled",
            .interface =
                &(const SettingProviderBoolInterface){
                    .default_value = BUSY_APP_IS_SHOW_WORK_TIME_ENABLED_DEFAULT,
                },
            .field_offset = offsetof(BusyAppConfig, is_show_work_time_enabled),
            .type = SettingProviderSettingTypeBool,
        },
};

static const SettingProviderSetting busy_timer_settings_v1_metadata[] = {
    [BusyTimerSettingsV1ProfileInfoIdxSortOrder] =
        {
            .name = "sort_order",
            .interface =
                &(const SettingProviderIntInterface){
                    .default_value = 0,
                },
            .field_offset = offsetof(BusyTimerMetadata, sort_order),
            .type = SettingProviderSettingTypeInt,
        },
    [BusyTimerSettingsV1ProfileInfoIdxTitle] =
        {
            .name = "title",
            .interface =
                &(const SettingProviderStringInterface){
                    .default_value = STRING_VALUE_DEFAULT,
                    // Including zero terminator
                    .max_size = SIZEOF_MEMBER(BusyTimerMetadata, title),
                },
            .field_offset = offsetof(BusyTimerMetadata, title),
            .type = SettingProviderSettingTypeString,
        },
    [BusyTimerSettingsV1ProfileInfoIdxId] =
        {
            .name = "id",
            .interface =
                &(const SettingProviderStringInterface){
                    .default_value = STRING_VALUE_DEFAULT,
                    // Including zero terminator
                    .max_size = SIZEOF_MEMBER(BusyTimerMetadata, card_id),
                },
            .field_offset = offsetof(BusyTimerMetadata, card_id),
            .type = SettingProviderSettingTypeString,
        },
};

static const SettingProviderSetting busy_timer_settings_v1_profile[] = {
    [BusyTimerSettingsV1ProfileIdxAppConfig] =
        {
            .name = "app_config",
            .interface =
                &(const SettingProviderStructInterface){
                    .inner_settings = busy_timer_settings_v1_app_config,
                    .inner_settings_count = COUNT_OF(busy_timer_settings_v1_app_config),
                },
            .field_offset = offsetof(BusyTimerProfile, app_config),
            .type = SettingProviderSettingTypeStruct,
        },
    [BusyTimerSettingsV1ProfileIdxTimerConfig] =
        {
            .name = "timer_config",
            .interface =
                &(const SettingProviderRawInterface){
                    .default_value = &busy_timer_settings_v1_timer_config_default,
                    .serialize_callback = busy_timer_settings_v1_timer_config_serialize_cb,
                    .deserialize_callback = busy_timer_settings_v1_timer_config_deserialize_cb,
                    .default_value_size = sizeof(busy_timer_settings_v1_timer_config_default),
                },
            .field_offset = offsetof(BusyTimerProfile, timer_config),
            .type = SettingProviderSettingTypeRaw,
        },
    [BusyTimerSettingsV1ProfileIdxMetadata] =
        {
            .name = "metadata",
            .interface =
                &(const SettingProviderStructInterface){
                    .inner_settings = busy_timer_settings_v1_metadata,
                    .inner_settings_count = COUNT_OF(busy_timer_settings_v1_metadata),
                },
            .field_offset = offsetof(BusyTimerProfile, metadata),
            .type = SettingProviderSettingTypeStruct,
        },
    [BusyTimerSettingsV1ProfileIdxTimestamp] =
        {
            .name = "timestamp_ms",
            .interface =
                &(const SettingProviderCustomInterface){
                    .default_value = &busy_timer_settings_v1_timestamp_default,
                    .default_value_size = sizeof(busy_timer_settings_v1_timestamp_default),
                    .serialize_callback = busy_timer_settings_v1_timestamp_serialize_cb,
                    .deserialize_callback = busy_timer_settings_v1_timestamp_deserialize_cb,
                },
            .field_offset = offsetof(BusyTimerProfile, timestamp_ms),
            .type = SettingProviderSettingTypeCustom,
        },
};

static const SettingProviderSetting busy_timer_settings_v1[] = {
    [BusyTimerSettingsV1IdxProfile] =
        {
            .name = "profile",
            .interface =
                &(const SettingProviderStructInterface){
                    .inner_settings = busy_timer_settings_v1_profile,
                    .inner_settings_count = COUNT_OF(busy_timer_settings_v1_profile),
                },
            .field_offset = offsetof(BusyTimerSettingsV1, profile),
            .type = SettingProviderSettingTypeStruct,
        },
    [BusyTimerSettingsV1IdxDemoMode] =
        {
            .name = "is_demo_mode_enabled",
            .interface =
                &(const SettingProviderBoolInterface){
                    .default_value = BUSY_TIMER_ENABLE_DEMO_MODE_DEFAULT,
                },
            .field_offset = offsetof(BusyTimerSettingsV1, is_demo_mode_enabled),
            .type = SettingProviderSettingTypeBool,
        },
};

const SettingProviderSetting busy_timer_settings_v1_root = {
    .interface =
        &(const SettingProviderStructInterface){
            .inner_settings = busy_timer_settings_v1,
            .inner_settings_count = COUNT_OF(busy_timer_settings_v1),
        },
    .type = SettingProviderSettingTypeStruct,
};

bool busy_timer_settings_v1_apply_defaults(
    BusyTimerSettingsV1* settings_v1,
    BusyTimerProfileId profile_id) {
    furi_assert(settings_v1);
    furi_assert(profile_id < BusyTimerProfileIdMax);

    const bool are_values_missing =
        (settings_v1->profile.timer_config.mode == BusyTimerModeMax) ||
        (strcmp(settings_v1->profile.app_config.theme_name, STRING_VALUE_DEFAULT) == 0) ||
        (strcmp(settings_v1->profile.metadata.card_id, STRING_VALUE_DEFAULT) == 0);

    if(are_values_missing) {
        *settings_v1 = busy_timer_settings_v1_defaults[profile_id];
    }

    return are_values_missing;
}
