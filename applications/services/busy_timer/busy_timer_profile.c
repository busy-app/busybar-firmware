#include "busy_timer_profile.h"
#include "busy_timer_common_i.h"

#include <furi.h>

#include <cjson/cJSON.h>

#define KEY_PROFILE_SORT_ORDER     "sort_order"
#define KEY_PROFILE_TITLE          "title"
#define KEY_PROFILE_TIMER_SETTINGS "timer_settings"
#define KEY_PROFILE_TIMESTAMP      "profile_timestamp_ms"
#define KEY_PROFILE_ID             "id"

#define KEY_PROFILE_TIMER_SETTINGS_TYPE "type"

// Profile serialization

static void busy_timer_profile_serialize_metadata(cJSON* json, const BusyTimerMetadata* metadata) {
    cJSON_AddNumberToObject(json, KEY_PROFILE_SORT_ORDER, metadata->sort_order);
    cJSON_AddStringToObject(json, KEY_PROFILE_TITLE, metadata->title);
    cJSON_AddStringToObject(json, KEY_PROFILE_ID, metadata->card_id);
}

static void busy_timer_profile_serialize_timer_settings(
    cJSON* json,
    const BusyTimerConfig* timer_settings) {
    cJSON* timer_settings_json = cJSON_AddObjectToObject(json, KEY_PROFILE_TIMER_SETTINGS);

    const BusyTimerMode timer_mode = timer_settings->mode;

    if(timer_mode == BusyTimerModeInfinite) {
        busy_timer_common_serialize_infinite_config(timer_settings_json);
    } else if(timer_mode == BusyTimerModeSimple) {
        busy_timer_common_serialize_simple_config(timer_settings_json, &timer_settings->simple);
    } else if(timer_mode == BusyTimerModeInterval) {
        busy_timer_common_serialize_interval_config(
            timer_settings_json, &timer_settings->interval);
    }
}

// Profile deserialization

static bool
    busy_timer_profile_deserialize_metadata(const cJSON* json, BusyTimerMetadata* metadata) {
    bool success = false;

    do {
        const cJSON* item;

        item = cJSON_GetObjectItem(json, KEY_PROFILE_SORT_ORDER);
        if(!cJSON_IsNumber(item)) {
            break;
        }

        metadata->sort_order = cJSON_GetNumberValue(item);

        item = cJSON_GetObjectItem(json, KEY_PROFILE_TITLE);
        if(!cJSON_IsString(item)) {
            break;
        }

        strlcpy(metadata->title, cJSON_GetStringValue(item), sizeof(metadata->title));

        item = cJSON_GetObjectItem(json, KEY_PROFILE_ID);
        if(!cJSON_IsString(item)) {
            break;
        }

        strlcpy(metadata->card_id, cJSON_GetStringValue(item), sizeof(metadata->card_id));

        success = true;
    } while(false);

    return success;
}

static bool busy_timer_profile_deserialize_timer_settings(
    const cJSON* json,
    BusyTimerConfig* timer_settings) {
    bool success = false;

    do {
        if(!cJSON_IsObject(json)) {
            break;
        }

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

static bool
    busy_timer_profile_handle_missing_app_config(const cJSON* json, BusyAppConfig* app_config) {
    bool success = false;

    if(json == NULL || cJSON_IsNull(json)) {
        memset(app_config, 0, sizeof(BusyAppConfig));
        app_config->is_show_work_time_enabled = BUSY_APP_IS_SHOW_WORK_TIME_ENABLED_DEFAULT;
        success = true;
    }

    return success;
}

// Public functions

char* busy_timer_profile_serialize(const BusyTimerProfile* profile) {
    cJSON* json = cJSON_CreateObject();

    busy_timer_profile_serialize_metadata(json, &profile->metadata);

    busy_timer_profile_serialize_timer_settings(json, &profile->timer_config);

    busy_timer_common_serialize_app_config(json, &profile->app_config);

    cJSON_AddNumberToObject(json, KEY_PROFILE_TIMESTAMP, profile->timestamp_ms);

    char* json_text = cJSON_PrintUnformatted(json);

    cJSON_Delete(json);
    return json_text;
}

bool busy_timer_profile_deserialize(
    BusyTimerProfile* profile,
    const char* json_text,
    size_t json_text_len) {
    bool success = false;

    cJSON* json = cJSON_ParseWithLength(json_text, json_text_len);

    do {
        if(!cJSON_IsObject(json)) {
            break;
        }

        if(!busy_timer_profile_deserialize_metadata(json, &profile->metadata)) {
            break;
        }

        const cJSON* item;

        item = cJSON_GetObjectItem(json, KEY_PROFILE_TIMER_SETTINGS);
        if(!busy_timer_profile_deserialize_timer_settings(item, &profile->timer_config)) {
            break;
        }

        item = cJSON_GetObjectItem(json, KEY_COMMON_BUSY_BAR_SETTINGS);
        if(!busy_timer_common_deserialize_app_config(item, &profile->app_config) &&
           !busy_timer_profile_handle_missing_app_config(item, &profile->app_config)) {
            break;
        }

        item = cJSON_GetObjectItem(json, KEY_PROFILE_TIMESTAMP);
        if(!cJSON_IsNumber(item)) {
            break;
        }

        profile->timestamp_ms = cJSON_GetNumberValue(item);

        success = true;
    } while(false);

    cJSON_Delete(json);

    return success;
}

bool busy_timer_profile_is_valid(const BusyTimerProfile* profile) {
    bool is_valid = false;

    do {
        if(!busy_timer_common_is_valid_card_id(profile->metadata.card_id)) {
            break;
        }

        const BusyTimerConfig* timer_settings = &profile->timer_config;
        const BusyTimerMode timer_mode = timer_settings->mode;

        if(timer_mode == BusyTimerModeInfinite) {
            // Nothing to validate
        } else if(timer_mode == BusyTimerModeSimple) {
            if(!busy_timer_common_is_valid_simple_config(&timer_settings->simple)) {
                break;
            }
        } else if(timer_mode == BusyTimerModeInterval) {
            if(!busy_timer_common_is_valid_interval_config(&timer_settings->interval)) {
                break;
            }
        } else {
            break;
        }

        is_valid = true;
    } while(false);

    return is_valid;
}
