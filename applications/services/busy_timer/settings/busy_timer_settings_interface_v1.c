#include "busy_timer_settings_interface_v1.h"

#include <furi_hal_rtc.h>

#define THEME_NAME_DEFAULT                ""
#define IS_SMART_HOME_ENABLED_DEFAULT     (true)
#define IS_SHOW_WORK_ONLY_ENABLED_DEFAULT (false)

#define TIME_DEFAULT_MN      (20)
#define WORK_TIME_DEFAULT_MN (20)
#define REST_TIME_DEFAULT_MN (5)
#define CYCLE_COUNT_DEFAULT  (3)

#define ENABLE_INTERVALS_DEFAULT (true)
#define ENABLE_AUTOSTART_DEFAULT (false)
#define ENABLE_DEMO_MODE_DEFAULT (false)

typedef enum {
    BusyTimerSettingsV1IdxAppConfig,
    BusyTimerSettingsV1IdxTimerConfig,
    BusyTimerSettingsV1IdxTimestamp,
    BusyTimerSettingsV1IdxMax,
} BusyTimerSettingsV1Idx;

typedef enum {
    BusyTimerSettingsV1AppConfigIdxThemeName,
    BusyTimerSettingsV1AppConfigIdxSmartHome,
    BusyTimerSettingsV1AppConfigIdxWorkOnly,
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

static const char* busy_timer_settings_v1_mode_names[BusyTimerModeMax] = {
    [BusyTimerModeInfinite] = "infinite",
    [BusyTimerModeSimple] = "simple",
    [BusyTimerModeInterval] = "interval",
};

static const char* busy_timer_settings_v1_theme_name_default[BusyTimerProfileIdMax] = {
    [BusyTimerProfileIdBusy] = "default",
    [BusyTimerProfileIdCustom] = "keep_out",
};

static const bool busy_timer_settings_v1_is_show_work_only_enabled[BusyTimerProfileIdMax] = {
    [BusyTimerProfileIdBusy] = false,
    [BusyTimerProfileIdCustom] = true,
};

static const BusyTimerMode busy_Timer_settings_v1_timer_mode_default[BusyTimerProfileIdMax] = {
    [BusyTimerProfileIdBusy] = BusyTimerModeInterval,
    [BusyTimerProfileIdCustom] = BusyTimerModeInfinite,
};

static const BusyTimerMode busy_timer_settings_v1_mode_default = BusyTimerModeMax;

static const time_t busy_timer_settings_v1_timestamp_default = 0;

static bool busy_timer_settings_v1_mode_serialize_cb(
    const SettingProviderSetting* setting,
    FuriString* string,
    const void* value) {
    UNUSED(setting);

    const BusyTimerMode mode = *(BusyTimerMode*)value;

    if(mode < BusyTimerModeMax) {
        furi_string_set(string, busy_timer_settings_v1_mode_names[mode]);
        return true;
    }

    return false;
}

static bool busy_timer_settings_v1_mode_deserialize_cb(
    const SettingProviderSetting* setting,
    void* value,
    const FuriString* string) {
    UNUSED(setting);

    BusyTimerMode mode;
    for(mode = 0; mode < BusyTimerModeMax; ++mode) {
        if(furi_string_equal(string, busy_timer_settings_v1_mode_names[mode])) {
            *(BusyTimerMode*)value = mode;
            break;
        }
    }

    return mode < BusyTimerModeMax;
}

static bool busy_timer_settings_v1_timestamp_serialize_cb(
    const SettingProviderSetting* setting,
    FuriString* string,
    const void* value) {
    UNUSED(setting);

    const time_t timestamp = *(time_t*)value;
    furi_string_printf(string, "%llu", timestamp);

    return true;
}

static bool busy_timer_settings_v1_timestamp_deserialize_cb(
    const SettingProviderSetting* setting,
    void* value,
    const FuriString* string) {
    UNUSED(setting);

    time_t* timestamp = value;
    const int num_read = sscanf(furi_string_get_cstr(string), "%llu", timestamp);

    return num_read == 1;
}

static const SettingProviderSetting busy_timer_settings_v1_app_config[] = {
    [BusyTimerSettingsV1AppConfigIdxThemeName] =
        {
            .name = "theme_name",
            .interface =
                &(const SettingProviderStringInterface){
                    .default_value = THEME_NAME_DEFAULT,
                    .max_length = BUSY_CONFIG_THEME_NAME_LEN,
                },
            .field_offset = offsetof(BusyAppConfig, theme_name),
            .type = SettingProviderSettingTypeString,
        },
    [BusyTimerSettingsV1AppConfigIdxSmartHome] =
        {
            .name = "is_smart_home_enabled",
            .interface =
                &(const SettingProviderBoolInterface){
                    .default_value = IS_SMART_HOME_ENABLED_DEFAULT,
                },
            .field_offset = offsetof(BusyAppConfig, is_smart_home_enabled),
            .type = SettingProviderSettingTypeBool,
        },
    [BusyTimerSettingsV1AppConfigIdxWorkOnly] =
        {
            .name = "is_show_work_only_enabled",
            .interface =
                &(const SettingProviderBoolInterface){
                    .default_value = IS_SHOW_WORK_ONLY_ENABLED_DEFAULT,
                },
            .field_offset = offsetof(BusyAppConfig, is_show_work_only_enabled),
            .type = SettingProviderSettingTypeBool,
        },
};

static const SettingProviderSetting busy_timer_settings_v1_timer_config[] = {
    [BusyTimerSettingsV1TimerConfigIdxMode] =
        {
            .name = "mode",
            .interface =
                &(const SettingProviderCustomInterface){
                    .default_value = &busy_timer_settings_v1_mode_default,
                    .default_value_size = sizeof(busy_timer_settings_v1_mode_default),
                    .serialize_callback = busy_timer_settings_v1_mode_serialize_cb,
                    .deserialize_callback = busy_timer_settings_v1_mode_deserialize_cb,
                },
            .field_offset = offsetof(BusyTimerConfig, mode),
            .type = SettingProviderSettingTypeCustom,
        },
    [BusyTimerSettingsV1TimerConfigIdxTime] =
        {
            .name = "simple_time_minutes",
            .interface =
                &(const SettingProviderIntInterface){
                    .default_value = TIME_DEFAULT_MN,
                },
            .field_offset = offsetof(BusyTimerConfig, time_mn),
            .type = SettingProviderSettingTypeInt,
        },
    [BusyTimerSettingsV1TimerConfigIdxWorkTime] =
        {
            .name = "work_time_minutes",
            .interface =
                &(const SettingProviderIntInterface){
                    .default_value = WORK_TIME_DEFAULT_MN,
                },
            .field_offset = offsetof(BusyTimerConfig, work_time_mn),
            .type = SettingProviderSettingTypeInt,
        },
    [BusyTimerSettingsV1TimerConfigIdxRestTime] =
        {
            .name = "rest_time_minutes",
            .interface =
                &(const SettingProviderIntInterface){
                    .default_value = REST_TIME_DEFAULT_MN,
                },
            .field_offset = offsetof(BusyTimerConfig, rest_time_mn),
            .type = SettingProviderSettingTypeInt,
        },
    [BusyTimerSettingsV1TimerConfigIdxCycleCount] =
        {
            .name = "work_cycles_count",
            .interface =
                &(const SettingProviderIntInterface){
                    .default_value = CYCLE_COUNT_DEFAULT,
                },
            .field_offset = offsetof(BusyTimerConfig, cycle_count),
            .type = SettingProviderSettingTypeInt,
        },
    [BusyTimerSettingsV1TimerConfigIdxEnableAutostart] =
        {
            .name = "is_autostart_enabled",
            .interface =
                &(const SettingProviderBoolInterface){
                    .default_value = ENABLE_AUTOSTART_DEFAULT,
                },
            .field_offset = offsetof(BusyTimerConfig, enable_autostart),
            .type = SettingProviderSettingTypeBool,
        },
    [BusyTimerSettingsV1TimerConfigIdxEnableDemoMode] =
        {
            .name = "is_demo_mode_enabled",
            .interface =
                &(const SettingProviderBoolInterface){
                    .default_value = ENABLE_DEMO_MODE_DEFAULT,
                },
            .field_offset = offsetof(BusyTimerConfig, enable_demo_mode),
            .type = SettingProviderSettingTypeBool,
        },
};

static const SettingProviderSetting busy_timer_settings_v1[] = {
    [BusyTimerSettingsV1IdxAppConfig] =
        {
            .name = "app_config",
            .interface =
                &(const SettingProviderStructureInterface){
                    .inner_settings = busy_timer_settings_v1_app_config,
                    .inner_settings_count = COUNT_OF(busy_timer_settings_v1_app_config),
                },
            .field_offset = offsetof(BusyTimerSettingsV1, app_config),
            .type = SettingProviderSettingTypeStructure,
        },
    [BusyTimerSettingsV1IdxTimerConfig] =
        {
            .name = "timer_config",
            .interface =
                &(const SettingProviderStructureInterface){
                    .inner_settings = busy_timer_settings_v1_timer_config,
                    .inner_settings_count = COUNT_OF(busy_timer_settings_v1_timer_config),
                },
            .field_offset = offsetof(BusyTimerSettingsV1, timer_config),
            .type = SettingProviderSettingTypeStructure,
        },
    [BusyTimerSettingsV1IdxTimestamp] =
        {
            .name = "timestamp",
            .interface =
                &(const SettingProviderCustomInterface){
                    .default_value = &busy_timer_settings_v1_timestamp_default,
                    .default_value_size = sizeof(busy_timer_settings_v1_timestamp_default),
                    .serialize_callback = busy_timer_settings_v1_timestamp_serialize_cb,
                    .deserialize_callback = busy_timer_settings_v1_timestamp_deserialize_cb,
                },
            .field_offset = offsetof(BusyTimerSettingsV1, timestamp),
            .type = SettingProviderSettingTypeCustom,
        },
};

const SettingProviderSetting busy_timer_settings_v1_root = {
    .interface =
        &(const SettingProviderStructureInterface){
            .inner_settings = busy_timer_settings_v1,
            .inner_settings_count = COUNT_OF(busy_timer_settings_v1),
        },
    .type = SettingProviderSettingTypeStructure,
};

bool busy_timer_settings_v1_apply_defaults(
    BusyTimerSettingsV1* settings_v1,
    BusyTimerProfileId profile_id) {
    furi_assert(settings_v1);
    furi_assert(profile_id < BusyTimerProfileIdMax);
    bool were_defaults_applied = false;

    BusyAppConfig* app_config = &settings_v1->app_config;
    BusyTimerConfig* timer_config = &settings_v1->timer_config;

    if(strcmp(app_config->theme_name, THEME_NAME_DEFAULT) == 0) {
        strcpy(app_config->theme_name, busy_timer_settings_v1_theme_name_default[profile_id]);
        app_config->is_show_work_only_enabled =
            busy_timer_settings_v1_is_show_work_only_enabled[profile_id];

        were_defaults_applied = true;
    }

    if(timer_config->mode == BusyTimerModeMax) {
        timer_config->mode = busy_Timer_settings_v1_timer_mode_default[profile_id];
        were_defaults_applied = true;
    }

    return were_defaults_applied;
}
