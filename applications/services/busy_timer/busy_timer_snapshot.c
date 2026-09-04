#include "busy_timer_snapshot_i.h"
#include "busy_timer_common_i.h"

#include <furi.h>
#include <furi_hal_rtc.h>

#define KEY_SNAPSHOT  "snapshot"
#define KEY_TIMESTAMP "snapshot_timestamp_ms"

#define KEY_SNAPSHOT_TYPE "type"

#define KEY_SNAPSHOT_COMMON_CARD_ID   "card_id"
#define KEY_SNAPSHOT_COMMON_IS_PAUSED "is_paused"

#define KEY_SNAPSHOT_SIMPLE_TIME_LEFT "time_left_ms"

#define KEY_SNAPSHOT_INTERVAL_CURRENT_ID    "current_interval"
#define KEY_SNAPSHOT_INTERVAL_CURRENT_TOTAL "current_interval_time_total_ms"
#define KEY_SNAPSHOT_INTERVAL_CURRENT_LEFT  "current_interval_time_left_ms"
#define KEY_SNAPSHOT_INTERVAL_SETTINGS      "interval_settings"

#define TIMESTAMP_TOLERANCE_MS (M_TO_MS(1))

static const char* const snapshot_type_values[BusyTimerSnapshotTypeMax] = {
    [BusyTimerSnapshotTypeNotStarted] = "NOT_STARTED",
    [BusyTimerSnapshotTypeInfinite] = "INFINITE",
    [BusyTimerSnapshotTypeSimple] = "SIMPLE",
    [BusyTimerSnapshotTypeInterval] = "INTERVAL",
};

// Snapshot serialization

static void busy_timer_snapshot_serialize_snapshot_common(
    cJSON* json,
    const BusyTimerSnapshotCommon* common) {
    cJSON_AddStringToObject(json, KEY_SNAPSHOT_COMMON_CARD_ID, common->card_id);
    cJSON_AddBoolToObject(json, KEY_SNAPSHOT_COMMON_IS_PAUSED, common->is_paused);
}

static void busy_timer_snapshot_serialize_snapshot_infinite(
    cJSON* json,
    const BusyTimerSnapshotInfinite* infinite) {
    busy_timer_snapshot_serialize_snapshot_common(json, &infinite->common);
}

static void busy_timer_snapshot_serialize_snapshot_simple(
    cJSON* json,
    const BusyTimerSnapshotSimple* simple) {
    busy_timer_snapshot_serialize_snapshot_common(json, &simple->common);

    cJSON_AddNumberToObject(json, KEY_SNAPSHOT_SIMPLE_TIME_LEFT, simple->time_left_ms);
}

static void busy_timer_snapshot_serialize_snapshot_interval(
    cJSON* json,
    const BusyTimerSnapshotInterval* interval) {
    busy_timer_snapshot_serialize_snapshot_common(json, &interval->common);

    cJSON_AddNumberToObject(json, KEY_SNAPSHOT_INTERVAL_CURRENT_ID, interval->state.index);
    cJSON_AddNumberToObject(
        json, KEY_SNAPSHOT_INTERVAL_CURRENT_TOTAL, interval->state.time_total_ms);
    cJSON_AddNumberToObject(
        json, KEY_SNAPSHOT_INTERVAL_CURRENT_LEFT, interval->state.time_left_ms);

    cJSON* settings_json = cJSON_AddObjectToObject(json, KEY_SNAPSHOT_INTERVAL_SETTINGS);
    busy_timer_common_serialize_interval_config(settings_json, &interval->config);
}

// Snapshot deserialization

static bool busy_timer_snapshot_deserialize_snapshot_common(
    const cJSON* json,
    BusyTimerSnapshotCommon* common) {
    bool success = false;

    do {
        const cJSON* item;
        const char* str_val;

        item = cJSON_GetObjectItem(json, KEY_SNAPSHOT_COMMON_CARD_ID);
        if(!cJSON_IsString(item)) {
            break;
        }

        str_val = cJSON_GetStringValue(item);
        if(strlen(str_val) != BUSY_TIMER_CARD_ID_LEN) {
            break;
        }

        strlcpy(common->card_id, str_val, sizeof(common->card_id));

        item = cJSON_GetObjectItem(json, KEY_SNAPSHOT_COMMON_IS_PAUSED);
        if(!cJSON_IsBool(item)) {
            break;
        }

        common->is_paused = cJSON_IsTrue(item);

        success = true;
    } while(false);

    return success;
}

static bool busy_timer_snapshot_deserialize_snapshot_infinite(
    const cJSON* json,
    BusyTimerSnapshotInfinite* infinite) {
    return busy_timer_snapshot_deserialize_snapshot_common(json, &infinite->common);
}

static bool busy_timer_snapshot_deserialize_snapshot_simple(
    const cJSON* json,
    BusyTimerSnapshotSimple* simple) {
    bool success = false;

    do {
        if(!busy_timer_snapshot_deserialize_snapshot_common(json, &simple->common)) {
            break;
        }

        const cJSON* item;

        item = cJSON_GetObjectItem(json, KEY_SNAPSHOT_SIMPLE_TIME_LEFT);
        if(!cJSON_IsNumber(item)) {
            break;
        }

        simple->time_left_ms = cJSON_GetNumberValue(item);

        success = true;
    } while(false);

    return success;
}

static bool busy_timer_snapshot_deserialize_snapshot_interval(
    const cJSON* json,
    BusyTimerSnapshotInterval* interval) {
    bool success = false;

    do {
        if(!busy_timer_snapshot_deserialize_snapshot_common(json, &interval->common)) {
            break;
        }

        const cJSON* item;

        item = cJSON_GetObjectItem(json, KEY_SNAPSHOT_INTERVAL_CURRENT_ID);
        if(!cJSON_IsNumber(item)) {
            break;
        }

        BusyTimerIntervalState* state = &interval->state;

        state->index = cJSON_GetNumberValue(item);

        item = cJSON_GetObjectItem(json, KEY_SNAPSHOT_INTERVAL_CURRENT_TOTAL);
        if(!cJSON_IsNumber(item)) {
            break;
        }

        state->time_total_ms = cJSON_GetNumberValue(item);

        item = cJSON_GetObjectItem(json, KEY_SNAPSHOT_INTERVAL_CURRENT_LEFT);
        if(!cJSON_IsNumber(item)) {
            break;
        }

        state->time_left_ms = cJSON_GetNumberValue(item);

        item = cJSON_GetObjectItem(json, KEY_SNAPSHOT_INTERVAL_SETTINGS);
        if(!busy_timer_common_deserialize_interval_config(item, &interval->config)) {
            break;
        }

        success = true;
    } while(false);

    return success;
}

static BusyTimerSnapshotType busy_timer_snapshot_deserialize_snapshot_type(const cJSON* json) {
    BusyTimerSnapshotType ret = BusyTimerSnapshotTypeMax;

    do {
        if(!cJSON_IsString(json)) {
            break;
        }

        const char* type_str = cJSON_GetStringValue(json);
        furi_check(type_str);

        for(BusyTimerSnapshotType i = 0; i < BusyTimerSnapshotTypeMax; ++i) {
            if(strcmp(type_str, snapshot_type_values[i]) == 0) {
                ret = i;
                break;
            }
        }

    } while(false);

    return ret;
}

static bool
    busy_timer_snapshot_deserialize_snapshot(const cJSON* json, BusyTimerSnapshot* snapshot) {
    bool success = false;

    do {
        if(!cJSON_IsObject(json)) {
            break;
        }

        const cJSON* item;

        item = cJSON_GetObjectItem(json, KEY_SNAPSHOT_TYPE);
        const BusyTimerSnapshotType snapshot_type =
            busy_timer_snapshot_deserialize_snapshot_type(item);

        if(snapshot_type == BusyTimerSnapshotTypeNotStarted) {
            /* Nothing */
        } else if(snapshot_type == BusyTimerSnapshotTypeInfinite) {
            if(!busy_timer_snapshot_deserialize_snapshot_infinite(json, &snapshot->infinite))
                break;
        } else if(snapshot_type == BusyTimerSnapshotTypeSimple) {
            if(!busy_timer_snapshot_deserialize_snapshot_simple(json, &snapshot->simple)) break;
        } else if(snapshot_type == BusyTimerSnapshotTypeInterval) {
            if(!busy_timer_snapshot_deserialize_snapshot_interval(json, &snapshot->interval))
                break;
        } else {
            break;
        }

        snapshot->type = snapshot_type;

        item = cJSON_GetObjectItem(json, KEY_COMMON_BUSY_BAR_SETTINGS);
        if(!busy_timer_common_deserialize_app_config(item, &snapshot->app_config)) {
            // TODO: Remove the default value and make it an error in the future
            snapshot->app_config = (const BusyAppConfig){
                .theme_name = BUSY_APP_THEME_NAME_DEFAULT,
                .is_smart_home_enabled = BUSY_APP_IS_SMART_HOME_ENABLED_DEFAULT,
                .is_show_work_only_enabled = BUSY_APP_IS_SHOW_WORK_ONLY_ENABLED_DEFAULT,
                .is_show_work_time_enabled = BUSY_APP_IS_SHOW_WORK_TIME_ENABLED_DEFAULT,
            };
        }

        success = true;
    } while(false);

    return success;
}

static bool busy_timer_snapshot_is_valid_timestamp(time_t timestamp_ms) {
    bool is_valid = true;

    const time_t now_timestamp_ms = furi_hal_rtc_get_timestamp_ms();

    if(timestamp_ms > now_timestamp_ms) {
        if(timestamp_ms - now_timestamp_ms > TIMESTAMP_TOLERANCE_MS) {
            is_valid = false;
        }
    }

    return is_valid;
}

static bool busy_timer_snapshot_is_valid_interval_state(const BusyTimerIntervalState* state) {
    bool is_valid = false;

    do {
        if(state->index > (BUSY_TIMER_CYCLE_COUNT_MAX * 2) - 2) {
            break;
        }

        if(state->time_left_ms > state->time_total_ms) {
            break;
        }

        const bool is_rest = state->index % 2;
        const uint32_t upper_bound_ms = is_rest ? M_TO_MS(BUSY_TIMER_REST_TIME_MAX_MN) :
                                                  M_TO_MS(BUSY_TIMER_WORK_TIME_MAX_MN);

        if(state->time_left_ms > upper_bound_ms) {
            break;
        }

        is_valid = true;
    } while(false);

    return is_valid;
}

// Internal functions

bool busy_timer_snapshot_serialize_raw(const BusyTimerSnapshot* snapshot, cJSON* json) {
    cJSON* snapshot_json = cJSON_AddObjectToObject(json, KEY_SNAPSHOT);

    const BusyTimerSnapshotType snapshot_type = snapshot->type;
    furi_check(snapshot_type < BusyTimerSnapshotTypeMax);

    cJSON_AddStringToObject(snapshot_json, KEY_SNAPSHOT_TYPE, snapshot_type_values[snapshot_type]);

    if(snapshot_type == BusyTimerSnapshotTypeInfinite) {
        busy_timer_snapshot_serialize_snapshot_infinite(snapshot_json, &snapshot->infinite);
    } else if(snapshot_type == BusyTimerSnapshotTypeSimple) {
        busy_timer_snapshot_serialize_snapshot_simple(snapshot_json, &snapshot->simple);
    } else if(snapshot_type == BusyTimerSnapshotTypeInterval) {
        busy_timer_snapshot_serialize_snapshot_interval(snapshot_json, &snapshot->interval);
    }

    busy_timer_common_serialize_app_config(snapshot_json, &snapshot->app_config);

    cJSON_AddNumberToObject(json, KEY_TIMESTAMP, snapshot->timestamp_ms);

    return true;
}

bool busy_timer_snapshot_deserialize_raw(BusyTimerSnapshot* snapshot, const cJSON* json) {
    bool success = false;

    do {
        if(!cJSON_IsObject(json)) {
            break;
        }

        const cJSON* item;

        item = cJSON_GetObjectItem(json, KEY_SNAPSHOT);
        if(!busy_timer_snapshot_deserialize_snapshot(item, snapshot)) {
            break;
        }

        item = cJSON_GetObjectItem(json, KEY_TIMESTAMP);
        if(!cJSON_IsNumber(item)) {
            break;
        }

        snapshot->timestamp_ms = cJSON_GetNumberValue(item);

        if(!busy_timer_snapshot_is_valid(snapshot)) {
            break;
        }

        success = true;
    } while(false);

    return success;
}

// Public functions

char* busy_timer_snapshot_serialize(const BusyTimerSnapshot* snapshot) {
    cJSON* json = cJSON_CreateObject();
    char* json_text = NULL;

    if(busy_timer_snapshot_serialize_raw(snapshot, json)) {
        json_text = cJSON_PrintUnformatted(json);
    }

    cJSON_Delete(json);
    return json_text;
}

bool busy_timer_snapshot_deserialize(
    BusyTimerSnapshot* snapshot,
    const char* json_text,
    size_t json_text_len) {
    cJSON* json = cJSON_ParseWithLength(json_text, json_text_len);

    const bool success = busy_timer_snapshot_deserialize_raw(snapshot, json);

    cJSON_Delete(json);
    return success;
}

bool busy_timer_snapshot_is_valid(const BusyTimerSnapshot* snapshot) {
    furi_check(snapshot);

    bool is_valid = false;

    do {
        if(!busy_timer_snapshot_is_valid_timestamp(snapshot->timestamp_ms)) {
            break;
        }

        const BusyTimerSnapshotType type = snapshot->type;

        if(type == BusyTimerSnapshotTypeNotStarted) {
            // Nothing to check
        } else if(type == BusyTimerSnapshotTypeInfinite) {
            const BusyTimerSnapshotInfinite* infinite = &snapshot->infinite;
            if(!busy_timer_common_is_valid_card_id(infinite->common.card_id)) {
                break;
            }

        } else if(type == BusyTimerSnapshotTypeSimple) {
            const BusyTimerSnapshotSimple* simple = &snapshot->simple;
            if(!busy_timer_common_is_valid_card_id(simple->common.card_id)) {
                break;
            }

            if(simple->time_left_ms > M_TO_MS(BUSY_TIMER_TIME_MAX_MN)) {
                break;
            }

        } else if(type == BusyTimerSnapshotTypeInterval) {
            const BusyTimerSnapshotInterval* interval = &snapshot->interval;
            if(!busy_timer_common_is_valid_card_id(interval->common.card_id)) {
                break;
            }

            if(!busy_timer_common_is_valid_interval_config(&interval->config)) {
                break;
            }

            if(!busy_timer_snapshot_is_valid_interval_state(&interval->state)) {
                break;
            }

        } else {
            break;
        }

        is_valid = true;

    } while(false);

    return is_valid;
}
