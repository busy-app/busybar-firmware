#include "busy_timer_common_i.h"

#include <furi.h>

static const char* const busy_timer_common_mode_names[BusyTimerModeMax] = {
    [BusyTimerModeInfinite] = "INFINITE",
    [BusyTimerModeSimple] = "SIMPLE",
    [BusyTimerModeInterval] = "INTERVAL",
};

void busy_timer_common_serialize_busy_bar_settings(cJSON* json, const BusyAppConfig* bsb_settings) {
    cJSON* busy_bar_settings_json = cJSON_AddObjectToObject(json, KEY_COMMON_BUSY_BAR_SETTINGS);
    cJSON_AddStringToObject(
        busy_bar_settings_json, KEY_COMMON_BUSY_BAR_SETTINGS_THEME, bsb_settings->theme_name);
    cJSON_AddBoolToObject(
        busy_bar_settings_json,
        KEY_COMMON_BUSY_BAR_SETTINGS_SHOW_WORK_PHASE_ONLY,
        bsb_settings->is_show_work_only_enabled);
    cJSON_AddBoolToObject(
        busy_bar_settings_json,
        KEY_COMMON_BUSY_BAR_SETTINGS_TRIGGER_SMART_HOME,
        bsb_settings->is_smart_home_enabled);
}

void busy_timer_common_serialize_infinite_settings(cJSON* json) {
    cJSON_AddStringToObject(
        json, KEY_COMMON_TIMER_SETTINGS_TYPE, busy_timer_common_mode_names[BusyTimerModeInfinite]);
}

void busy_timer_common_serialize_timer_mode(cJSON* json, BusyTimerMode timer_mode) {
    furi_assert(timer_mode < BusyTimerModeMax);
    cJSON_AddStringToObject(
        json, KEY_COMMON_TIMER_SETTINGS_TYPE, busy_timer_common_mode_names[timer_mode]);
}

void busy_timer_common_serialize_simple_settings(
    cJSON* json,
    const BusyTimerSimpleSettings* simple_settings) {
    cJSON_AddStringToObject(
        json, KEY_COMMON_TIMER_SETTINGS_TYPE, busy_timer_common_mode_names[BusyTimerModeSimple]);
    cJSON_AddNumberToObject(
        json, KEY_COMMON_SIMPLE_SETTINGS_TOTAL_TIME, simple_settings->total_time_ms);
}

void busy_timer_common_serialize_interval_settings(
    cJSON* json,
    const BusyTimerIntervalSettings* interval_settings) {
    cJSON_AddStringToObject(
        json, KEY_COMMON_TIMER_SETTINGS_TYPE, busy_timer_common_mode_names[BusyTimerModeInterval]);
    cJSON_AddNumberToObject(
        json, KEY_COMMON_INTERVAL_SETTINGS_WORK, interval_settings->work_time_ms);
    cJSON_AddNumberToObject(
        json, KEY_COMMON_INTERVAL_SETTINGS_REST, interval_settings->rest_time_ms);
    cJSON_AddNumberToObject(
        json, KEY_COMMON_INTERVAL_SETTINGS_CYCLES, interval_settings->cycles_count);
    cJSON_AddBoolToObject(
        json, KEY_COMMON_INTERVAL_SETTINGS_AUTOSTART, interval_settings->is_autostart_enabled);
}

bool busy_timer_common_deserialize_busy_bar_settings(
    const cJSON* json,
    BusyAppConfig* bsb_settings) {
    bool success = false;

    do {
        if(!cJSON_IsObject(json)) {
            break;
        }

        const cJSON* item;

        item = cJSON_GetObjectItem(json, KEY_COMMON_BUSY_BAR_SETTINGS_THEME);
        if(!cJSON_IsString(item)) {
            break;
        }

        strlcpy(
            bsb_settings->theme_name,
            cJSON_GetStringValue(item),
            sizeof(bsb_settings->theme_name));

        item = cJSON_GetObjectItem(json, KEY_COMMON_BUSY_BAR_SETTINGS_SHOW_WORK_PHASE_ONLY);
        if(!cJSON_IsBool(item)) {
            break;
        }

        bsb_settings->is_show_work_only_enabled = cJSON_IsTrue(item);

        item = cJSON_GetObjectItem(json, KEY_COMMON_BUSY_BAR_SETTINGS_TRIGGER_SMART_HOME);
        if(!cJSON_IsBool(item)) {
            break;
        }

        bsb_settings->is_smart_home_enabled = cJSON_IsTrue(item);

        success = true;
    } while(false);

    return success;
}

bool busy_timer_common_deserialize_timer_mode(const cJSON* json, BusyTimerMode* timer_mode) {
    bool success = false;

    do {
        if(!cJSON_IsString(json)) {
            break;
        }

        const char* mode_name = cJSON_GetStringValue(json);

        BusyTimerMode found_mode;
        for(found_mode = 0; found_mode < BusyTimerModeMax; ++found_mode) {
            if(strcmp(busy_timer_common_mode_names[found_mode], mode_name) == 0) {
                break;
            }
        }

        if(found_mode >= BusyTimerModeMax) {
            break;
        }

        *timer_mode = found_mode;
        success = true;

    } while(false);

    return success;
}

bool busy_timer_common_deserialize_simple_settings(
    const cJSON* json,
    BusyTimerSimpleSettings* simple_settings) {
    bool success = false;

    do {
        if(!cJSON_IsObject(json)) {
            break;
        }

        const cJSON* item;

        item = cJSON_GetObjectItem(json, KEY_COMMON_SIMPLE_SETTINGS_TOTAL_TIME);
        if(!cJSON_IsNumber(item)) {
            break;
        }

        simple_settings->total_time_ms = cJSON_GetNumberValue(item);

        success = true;
    } while(false);

    return success;
}

bool busy_timer_common_deserialize_interval_settings(
    const cJSON* json,
    BusyTimerIntervalSettings* interval_settings) {
    bool success = false;

    do {
        if(!cJSON_IsObject(json)) {
            break;
        }

        const cJSON* item;

        item = cJSON_GetObjectItem(json, KEY_COMMON_INTERVAL_SETTINGS_WORK);
        if(!cJSON_IsNumber(item)) {
            break;
        }

        interval_settings->work_time_ms = cJSON_GetNumberValue(item);

        item = cJSON_GetObjectItem(json, KEY_COMMON_INTERVAL_SETTINGS_REST);
        if(!cJSON_IsNumber(item)) {
            break;
        }

        interval_settings->rest_time_ms = cJSON_GetNumberValue(item);

        item = cJSON_GetObjectItem(json, KEY_COMMON_INTERVAL_SETTINGS_CYCLES);
        if(!cJSON_IsNumber(item)) {
            break;
        }

        interval_settings->cycles_count = cJSON_GetNumberValue(item);

        item = cJSON_GetObjectItem(json, KEY_COMMON_INTERVAL_SETTINGS_AUTOSTART);
        if(!cJSON_IsBool(item)) {
            break;
        }

        interval_settings->is_autostart_enabled = cJSON_IsTrue(item);

        success = true;
    } while(false);

    return success;
}
