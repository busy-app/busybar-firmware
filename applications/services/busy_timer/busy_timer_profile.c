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

static void busy_timer_profile_serialize_profile_info(
    cJSON* json,
    const BusyTimerProfileInfo* profile_info) {
    cJSON_AddNumberToObject(json, KEY_PROFILE_SORT_ORDER, profile_info->sort_order);
    cJSON_AddStringToObject(json, KEY_PROFILE_TITLE, profile_info->title);
    cJSON_AddStringToObject(json, KEY_PROFILE_ID, profile_info->card_id);
}

static void busy_timer_profile_serialize_timer_settings(
    cJSON* json,
    const BusyTimerProfileSettings* timer_settings) {
    const BusyTimerMode timer_mode = timer_settings->mode;

    busy_timer_common_serialize_timer_mode(json, timer_mode);

    if(timer_mode == BusyTimerModeSimple) {
        busy_timer_common_serialize_simple_settings(json, &timer_settings->simple);
    } else if(timer_mode == BusyTimerModeInfinite) {
        busy_timer_common_serialize_interval_settings(json, &timer_settings->interval);
    }
}

// Profile deserialization

static bool busy_timer_profile_deserialize_profile_info(
    const cJSON* json,
    BusyTimerProfileInfo* profile_info) {
    bool success = false;

    do {
        const cJSON* item;

        item = cJSON_GetObjectItem(json, KEY_PROFILE_SORT_ORDER);
        if(!cJSON_IsNumber(item)) {
            break;
        }

        profile_info->sort_order = cJSON_GetNumberValue(item);

        item = cJSON_GetObjectItem(json, KEY_PROFILE_TITLE);
        if(!cJSON_IsString(item)) {
            break;
        }

        strlcpy(profile_info->title, cJSON_GetStringValue(item), sizeof(profile_info->title));

        item = cJSON_GetObjectItem(json, KEY_PROFILE_ID);
        if(!cJSON_IsString(item)) {
            break;
        }

        strlcpy(profile_info->card_id, cJSON_GetStringValue(item), sizeof(profile_info->card_id));

        success = true;
    } while(false);

    return success;
}

static bool busy_timer_profile_deserialize_timer_settings(
    const cJSON* json,
    BusyTimerProfileSettings* timer_settings) {
    bool success = false;

    do {
        if(!cJSON_IsObject(json)) {
            break;
        }

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

    return success;
}

// Public functions

char* busy_timer_profile_serialize(const BusyTimerProfile* profile) {
    cJSON* json = cJSON_CreateObject();

    busy_timer_profile_serialize_profile_info(json, &profile->info);

    busy_timer_profile_serialize_timer_settings(json, &profile->timer_settings);

    busy_timer_common_serialize_busy_bar_settings(json, &profile->busy_bar_settings);

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

        if(!busy_timer_profile_deserialize_profile_info(json, &profile->info)) {
            break;
        }

        const cJSON* item;

        item = cJSON_GetObjectItem(json, KEY_PROFILE_TIMER_SETTINGS);
        if(!busy_timer_profile_deserialize_timer_settings(item, &profile->timer_settings)) {
            break;
        }

        item = cJSON_GetObjectItem(json, KEY_COMMON_BUSY_BAR_SETTINGS);
        if(!busy_timer_common_deserialize_busy_bar_settings(item, &profile->busy_bar_settings)) {
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
    UNUSED(profile);
    // TODO: Implementation
    return true;
}
