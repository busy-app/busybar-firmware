#include "busy_timer_common_i.h"

#include <furi.h>

static const char* const busy_timer_common_mode_names[BusyTimerModeMax] = {
    [BusyTimerModeInfinite] = "INFINITE",
    [BusyTimerModeSimple] = "SIMPLE",
    [BusyTimerModeInterval] = "INTERVAL",
};

static void busy_timer_common_serialize_timer_mode(cJSON* json, BusyTimerMode timer_mode) {
    furi_assert(timer_mode < BusyTimerModeMax);
    cJSON_AddStringToObject(
        json, KEY_COMMON_TIMER_SETTINGS_TYPE, busy_timer_common_mode_names[timer_mode]);
}

void busy_timer_common_serialize_app_config(cJSON* json, const BusyAppConfig* app_config) {
    cJSON* busy_bar_settings_json = cJSON_AddObjectToObject(json, KEY_COMMON_BUSY_BAR_SETTINGS);
    cJSON_AddStringToObject(
        busy_bar_settings_json, KEY_COMMON_BUSY_BAR_SETTINGS_THEME, app_config->theme_name);
    cJSON_AddBoolToObject(
        busy_bar_settings_json,
        KEY_COMMON_BUSY_BAR_SETTINGS_SHOW_WORK_PHASE_ONLY,
        app_config->is_show_work_only_enabled);
    cJSON_AddBoolToObject(
        busy_bar_settings_json,
        KEY_COMMON_BUSY_BAR_SETTINGS_TRIGGER_SMART_HOME,
        app_config->is_smart_home_enabled);
    cJSON_AddBoolToObject(
        busy_bar_settings_json,
        KEY_COMMON_BUSY_BAR_SETTINGS_SHOW_WORK_TIME,
        app_config->is_show_work_time_enabled);
}

void busy_timer_common_serialize_infinite_config(cJSON* json) {
    busy_timer_common_serialize_timer_mode(json, BusyTimerModeInfinite);
}

void busy_timer_common_serialize_simple_config(
    cJSON* json,
    const BusyTimerSimpleConfig* simple_config) {
    busy_timer_common_serialize_timer_mode(json, BusyTimerModeSimple);

    cJSON_AddNumberToObject(
        json, KEY_COMMON_SIMPLE_SETTINGS_TOTAL_TIME, simple_config->total_time_ms);
}

void busy_timer_common_serialize_interval_config(
    cJSON* json,
    const BusyTimerIntervalConfig* interval_config) {
    busy_timer_common_serialize_timer_mode(json, BusyTimerModeInterval);

    cJSON_AddNumberToObject(
        json, KEY_COMMON_INTERVAL_SETTINGS_WORK, interval_config->work_time_ms);
    cJSON_AddNumberToObject(
        json, KEY_COMMON_INTERVAL_SETTINGS_REST, interval_config->rest_time_ms);
    cJSON_AddNumberToObject(
        json, KEY_COMMON_INTERVAL_SETTINGS_CYCLES, interval_config->cycles_count);
    cJSON_AddBoolToObject(
        json, KEY_COMMON_INTERVAL_SETTINGS_AUTOSTART, interval_config->is_autostart_enabled);
}

bool busy_timer_common_deserialize_app_config(const cJSON* json, BusyAppConfig* app_config) {
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
            app_config->theme_name, cJSON_GetStringValue(item), sizeof(app_config->theme_name));

        item = cJSON_GetObjectItem(json, KEY_COMMON_BUSY_BAR_SETTINGS_SHOW_WORK_PHASE_ONLY);
        if(!cJSON_IsBool(item)) {
            break;
        }

        app_config->is_show_work_only_enabled = cJSON_IsTrue(item);

        item = cJSON_GetObjectItem(json, KEY_COMMON_BUSY_BAR_SETTINGS_TRIGGER_SMART_HOME);
        if(!cJSON_IsBool(item)) {
            break;
        }

        app_config->is_smart_home_enabled = cJSON_IsTrue(item);

        item = cJSON_GetObjectItem(json, KEY_COMMON_BUSY_BAR_SETTINGS_SHOW_WORK_TIME);
        if(item == NULL) {
            app_config->is_show_work_time_enabled = BUSY_APP_IS_SHOW_WORK_TIME_ENABLED_DEFAULT;
        } else if(!cJSON_IsBool(item)) {
            break;
        } else {
            app_config->is_show_work_time_enabled = cJSON_IsTrue(item);
        }

        success = true;
    } while(false);

    return success;
}

bool busy_timer_common_deserialize_timer_mode(const cJSON* json, BusyTimerMode* timer_mode) {
    bool success = false;

    do {
        cJSON* item = cJSON_GetObjectItem(json, KEY_COMMON_TIMER_SETTINGS_TYPE);

        if(!cJSON_IsString(item)) {
            break;
        }

        const char* mode_name = cJSON_GetStringValue(item);

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

bool busy_timer_common_deserialize_simple_config(
    const cJSON* json,
    BusyTimerSimpleConfig* simple_config) {
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

        simple_config->total_time_ms = cJSON_GetNumberValue(item);

        success = true;
    } while(false);

    return success;
}

bool busy_timer_common_deserialize_interval_config(
    const cJSON* json,
    BusyTimerIntervalConfig* interval_config) {
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

        interval_config->work_time_ms = cJSON_GetNumberValue(item);

        item = cJSON_GetObjectItem(json, KEY_COMMON_INTERVAL_SETTINGS_REST);
        if(!cJSON_IsNumber(item)) {
            break;
        }

        interval_config->rest_time_ms = cJSON_GetNumberValue(item);

        item = cJSON_GetObjectItem(json, KEY_COMMON_INTERVAL_SETTINGS_CYCLES);
        if(!cJSON_IsNumber(item)) {
            break;
        }

        interval_config->cycles_count = cJSON_GetNumberValue(item);

        item = cJSON_GetObjectItem(json, KEY_COMMON_INTERVAL_SETTINGS_AUTOSTART);
        if(!cJSON_IsBool(item)) {
            break;
        }

        interval_config->is_autostart_enabled = cJSON_IsTrue(item);

        success = true;
    } while(false);

    return success;
}

bool busy_timer_common_is_valid_card_id(const char* card_id) {
    uint32_t i;

    for(i = 0; i < BUSY_TIMER_CARD_ID_LEN; ++i) {
        const char c = card_id[i];

        if(c == '\0') {
            break;
        }

        if(i == 8 || i == 13 || i == 18 || i == 23) {
            if(c != '-') {
                break;
            }

        } else {
            if(!isxdigit(c)) {
                break;
            }
        }
    }

    return i == BUSY_TIMER_CARD_ID_LEN;
}

bool busy_timer_common_is_valid_simple_config(const BusyTimerSimpleConfig* simple_config) {
    bool is_valid = false;

    do {
        if(simple_config->total_time_ms > M_TO_MS(BUSY_TIMER_TIME_MAX_MN)) {
            break;
        }

        is_valid = true;
    } while(false);

    return is_valid;
}

bool busy_timer_common_is_valid_interval_config(const BusyTimerIntervalConfig* interval_config) {
    bool is_valid = false;

    do {
        if(interval_config->cycles_count < BUSY_TIMER_CYCLE_COUNT_MIN ||
           interval_config->cycles_count > BUSY_TIMER_CYCLE_COUNT_MAX) {
            break;
        }

        if(interval_config->work_time_ms < M_TO_MS(BUSY_TIMER_WORK_TIME_MIN_MN) ||
           interval_config->work_time_ms > M_TO_MS(BUSY_TIMER_WORK_TIME_MAX_MN)) {
            break;
        }

        if(interval_config->rest_time_ms < M_TO_MS(BUSY_TIMER_REST_TIME_MIN_MN) ||
           interval_config->rest_time_ms > M_TO_MS(BUSY_TIMER_REST_TIME_MAX_MN)) {
            break;
        }

        is_valid = true;
    } while(false);

    return is_valid;
}
