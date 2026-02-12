#include "busy_timer_settings_interface_v1.h"

#include "../busy_timer_common_i.h"

#define STRING_VALUE_DEFAULT ""

#define SORT_ORDER_DEFAULT (-1)

typedef enum {
    BusyTimerSettingsV1IdxAppConfig,
    BusyTimerSettingsV1IdxTimerConfig,
    BusyTimerSettingsV1IdxProfileInfo,
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

typedef enum {
    BusyTimerSettingsV1ProfileInfoIdxSortOrder,
    BusyTimerSettingsV1ProfileInfoIdxTitle,
    BusyTimerSettingsV1ProfileInfoIdxId,
    BusyTimerSettingsV1ProfileInfoIdxMax,
} BusyTimerSettingsV1ProfileInfoIdx;

// NOTE: Wastes ~64 bytes of flash space for convenience
// NOTE: Wastes ~200 bytes of flash space for convenience

static const BusyTimerSettingsV1 busy_timer_settings_v1_defaults[BusyTimerProfileIdMax] = {
    [BusyTimerProfileIdBusy] =
        {
            .busy_bar_settings =
                {
                    .theme_name = "default",
                    .is_show_work_only_enabled = false,
                    .is_smart_home_enabled = true,
                },
            .timer_settings =
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
            .profile_info =
                {
                    .sort_order = SORT_ORDER_DEFAULT,
                    .title = "busy",
                    .card_id = "00000000-0000-0000-0000-000000000000",
                },
        },
    [BusyTimerProfileIdCustom] =
        {
            .busy_bar_settings =
                {
                    .theme_name = "keep_out",
                    .is_show_work_only_enabled = true,
                    .is_smart_home_enabled = true,
                },
            .timer_settings =
                {
                    .mode = BusyTimerModeInfinite,
                },
            .profile_info =
                {
                    .sort_order = SORT_ORDER_DEFAULT,
                    .title = "custom",
                    .card_id = "00000000-0000-0000-0000-000000000001",
                },
        },
};

static const BusyTimerProfileSettings busy_timer_settings_v1_timer_settings_default = {
    .mode = BusyTimerModeMax,
    .interval = {0},
};

static const time_t busy_timer_settings_v1_timestamp_default = 0;

static bool busy_timer_settings_v1_timer_settings_serialize_cb(
    const SettingProviderSetting* setting,
    FuriString* string,
    const void* value) {
    UNUSED(setting);

    const BusyTimerProfileSettings* timer_settings = value;

    cJSON* json = cJSON_CreateObject();

    if(timer_settings->mode == BusyTimerModeInfinite) {
        busy_timer_common_serialize_infinite_settings(json);
    } else if(timer_settings->mode == BusyTimerModeSimple) {
        busy_timer_common_serialize_simple_settings(json, &timer_settings->simple);
    } else if(timer_settings->mode == BusyTimerModeInterval) {
        busy_timer_common_serialize_interval_settings(json, &timer_settings->interval);
    }

    char* json_text = cJSON_PrintUnformatted(json);
    furi_check(json_text);

    furi_string_set(string, json_text);
    free(json_text);

    cJSON_Delete(json);

    return true;
}

static bool busy_timer_settings_v1_timer_settings_deserialize_cb(
    const SettingProviderSetting* setting,
    void* value,
    const FuriString* string) {
    UNUSED(setting);

    bool success = false;

    cJSON* json = cJSON_Parse(furi_string_get_cstr(string));

    do {
        if(!cJSON_IsObject(json)) {
            break;
        }

        BusyTimerProfileSettings* timer_settings = value;

        if(!busy_timer_common_deserialize_timer_mode(json, &timer_settings->mode)) {
            break;
        }

        if(timer_settings->mode == BusyTimerModeSimple) {
            if(!busy_timer_common_deserialize_simple_settings(json, &timer_settings->simple)) {
                break;
            }

        } else if(timer_settings->mode == BusyTimerModeInterval) {
            if(!busy_timer_common_deserialize_interval_settings(json, &timer_settings->interval)) {
                break;
            }
        }

        success = true;
    } while(false);

    cJSON_Delete(json);

    return success;
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

static const SettingProviderSetting busy_timer_settings_v1_busy_bar_settings[] = {
    [BusyTimerSettingsV1AppConfigIdxThemeName] =
        {
            .name = "theme_name",
            .interface =
                &(const SettingProviderStringInterface){
                    .default_value = STRING_VALUE_DEFAULT,
                    // Including zero terminator
                    .max_length = SIZEOF_MEMBER(BusyAppConfig, theme_name),
                },
            .field_offset = offsetof(BusyAppConfig, theme_name),
            .type = SettingProviderSettingTypeString,
        },
    [BusyTimerSettingsV1AppConfigIdxSmartHome] =
        {
            .name = "is_smart_home_enabled",
            .interface =
                &(const SettingProviderBoolInterface){
                    .default_value = BUSY_BAR_IS_SMART_HOME_ENABLED_DEFAULT,
                },
            .field_offset = offsetof(BusyAppConfig, is_smart_home_enabled),
            .type = SettingProviderSettingTypeBool,
        },
    [BusyTimerSettingsV1AppConfigIdxWorkOnly] =
        {
            .name = "is_show_work_only_enabled",
            .interface =
                &(const SettingProviderBoolInterface){
                    .default_value = BUSY_BAR_IS_SHOW_WORK_ONLY_ENABLED_DEFAULT,
                },
            .field_offset = offsetof(BusyAppConfig, is_show_work_only_enabled),
            .type = SettingProviderSettingTypeBool,
        },
};

static const SettingProviderSetting busy_timer_settings_v1_profile_info[] = {
    [BusyTimerSettingsV1ProfileInfoIdxSortOrder] =
        {
            .name = "sort_order",
            .interface =
                &(const SettingProviderIntInterface){
                    .default_value = 0,
                },
            .field_offset = offsetof(BusyTimerProfileInfo, sort_order),
            .type = SettingProviderSettingTypeInt,
        },
    [BusyTimerSettingsV1ProfileInfoIdxTitle] =
        {
            .name = "title",
            .interface =
                &(const SettingProviderStringInterface){
                    .default_value = STRING_VALUE_DEFAULT,
                    // Including zero terminator
                    .max_length = SIZEOF_MEMBER(BusyTimerProfileInfo, title),
                },
            .field_offset = offsetof(BusyTimerProfileInfo, title),
            .type = SettingProviderSettingTypeString,
        },
    [BusyTimerSettingsV1ProfileInfoIdxId] =
        {
            .name = "id",
            .interface =
                &(const SettingProviderStringInterface){
                    .default_value = STRING_VALUE_DEFAULT,
                    // Including zero terminator
                    .max_length = SIZEOF_MEMBER(BusyTimerProfileInfo, card_id),
                },
            .field_offset = offsetof(BusyTimerProfileInfo, card_id),
            .type = SettingProviderSettingTypeString,
        },
};

static const SettingProviderSetting busy_timer_settings_v1[] = {
    [BusyTimerSettingsV1IdxAppConfig] =
        {
            .name = "busy_bar_settings",
            .interface =
                &(const SettingProviderStructureInterface){
                    .inner_settings = busy_timer_settings_v1_busy_bar_settings,
                    .inner_settings_count = COUNT_OF(busy_timer_settings_v1_busy_bar_settings),
                },
            .field_offset = offsetof(BusyTimerSettingsV1, busy_bar_settings),
            .type = SettingProviderSettingTypeStructure,
        },
    [BusyTimerSettingsV1IdxTimerConfig] =
        {
            .name = "timer_settings",
            .interface =
                &(const SettingProviderCustomInterface){
                    .default_value = &busy_timer_settings_v1_timer_settings_default,
                    .serialize_callback = busy_timer_settings_v1_timer_settings_serialize_cb,
                    .deserialize_callback = busy_timer_settings_v1_timer_settings_deserialize_cb,
                    .default_value_size = sizeof(busy_timer_settings_v1_timer_settings_default),
                },
            .field_offset = offsetof(BusyTimerSettingsV1, timer_settings),
            .type = SettingProviderSettingTypeCustom,
        },
    [BusyTimerSettingsV1IdxProfileInfo] =
        {
            .name = "profile_metadata",
            .interface =
                &(const SettingProviderStructureInterface){
                    .inner_settings = busy_timer_settings_v1_profile_info,
                    .inner_settings_count = COUNT_OF(busy_timer_settings_v1_profile_info),
                },
            .field_offset = offsetof(BusyTimerSettingsV1, profile_info),
            .type = SettingProviderSettingTypeStructure,
        },
    [BusyTimerSettingsV1IdxTimestamp] =
        {
            .name = "timestamp_ms",
            .interface =
                &(const SettingProviderCustomInterface){
                    .default_value = &busy_timer_settings_v1_timestamp_default,
                    .default_value_size = sizeof(busy_timer_settings_v1_timestamp_default),
                    .serialize_callback = busy_timer_settings_v1_timestamp_serialize_cb,
                    .deserialize_callback = busy_timer_settings_v1_timestamp_deserialize_cb,
                },
            .field_offset = offsetof(BusyTimerSettingsV1, timestamp_ms),
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

    const bool are_values_missing =
        (settings_v1->timer_settings.mode == BusyTimerModeMax) ||
        (strcmp(settings_v1->busy_bar_settings.theme_name, STRING_VALUE_DEFAULT) == 0) ||
        (strcmp(settings_v1->profile_info.card_id, STRING_VALUE_DEFAULT) == 0);

    if(are_values_missing) {
        *settings_v1 = busy_timer_settings_v1_defaults[profile_id];
    }

    return are_values_missing;
}
