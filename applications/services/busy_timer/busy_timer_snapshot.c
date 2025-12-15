#include "busy_timer_snapshot.h"

#include <furi.h>

#include <cjson/cJSON.h>

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

#define KEY_SNAPSHOT_INTERVAL_SETTINGS_WORK      "interval_work_ms"
#define KEY_SNAPSHOT_INTERVAL_SETTINGS_REST      "interval_rest_ms"
#define KEY_SNAPSHOT_INTERVAL_SETTINGS_CYCLES    "interval_work_cycles_count"
#define KEY_SNAPSHOT_INTERVAL_SETTINGS_AUTOSTART "is_autostart_enabled"

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

static void busy_timer_snapshot_serialize_snapshot_interval_settings(
    cJSON* json,
    const BusyTimerIntervalSettings* settings) {
    cJSON* settings_json = cJSON_AddObjectToObject(json, KEY_SNAPSHOT_INTERVAL_SETTINGS);
    cJSON_AddStringToObject(
        settings_json, KEY_SNAPSHOT_TYPE, snapshot_type_values[BusyTimerSnapshotTypeInterval]);
    cJSON_AddNumberToObject(
        settings_json, KEY_SNAPSHOT_INTERVAL_SETTINGS_WORK, settings->work_time_ms);
    cJSON_AddNumberToObject(
        settings_json, KEY_SNAPSHOT_INTERVAL_SETTINGS_REST, settings->rest_time_ms);
    cJSON_AddNumberToObject(
        settings_json, KEY_SNAPSHOT_INTERVAL_SETTINGS_CYCLES, settings->cycles_count);
    cJSON_AddBoolToObject(
        settings_json, KEY_SNAPSHOT_INTERVAL_SETTINGS_AUTOSTART, settings->is_autostart_enabled);
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

    busy_timer_snapshot_serialize_snapshot_interval_settings(json, &interval->settings);
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

        strcpy(common->card_id, str_val);

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

static bool busy_timer_snapshot_deserialize_interval_settings(
    const cJSON* json,
    BusyTimerIntervalSettings* settings) {
    bool success = false;

    do {
        if(!cJSON_IsObject(json)) {
            break;
        }

        const cJSON* item;

// TODO: Remove after the mobile apps have been fixed
#ifdef INTERVAL_SETTINGS_TYPE_CHECK
        item = cJSON_GetObjectItem(json, KEY_SNAPSHOT_TYPE);
        if(!cJSON_IsString(item)) {
            break;
        }

        const char* type_str = cJSON_GetStringValue(item);
        furi_check(type_str);

        if(strcmp(type_str, snapshot_type_values[BusyTimerSnapshotTypeInterval]) != 0) {
            break;
        }
#endif
        item = cJSON_GetObjectItem(json, KEY_SNAPSHOT_INTERVAL_SETTINGS_WORK);
        if(!cJSON_IsNumber(item)) {
            break;
        }

        settings->work_time_ms = cJSON_GetNumberValue(item);

        item = cJSON_GetObjectItem(json, KEY_SNAPSHOT_INTERVAL_SETTINGS_REST);
        if(!cJSON_IsNumber(item)) {
            break;
        }

        settings->rest_time_ms = cJSON_GetNumberValue(item);

        item = cJSON_GetObjectItem(json, KEY_SNAPSHOT_INTERVAL_SETTINGS_CYCLES);
        if(!cJSON_IsNumber(item)) {
            break;
        }

        settings->cycles_count = cJSON_GetNumberValue(item);

        item = cJSON_GetObjectItem(json, KEY_SNAPSHOT_INTERVAL_SETTINGS_AUTOSTART);
        if(!cJSON_IsBool(item)) {
            break;
        }

        settings->is_autostart_enabled = cJSON_IsTrue(item);

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
        if(!busy_timer_snapshot_deserialize_interval_settings(item, &interval->settings)) {
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

        success = true;
    } while(false);

    return success;
}

// UUID format check
bool busy_timer_snapshot_is_valid_card_id(const char* card_id) {
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

// Public functions

char* busy_timer_snapshot_serialize(const BusyTimerSnapshot* snapshot) {
    cJSON* json = cJSON_CreateObject();

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

    cJSON_AddNumberToObject(json, KEY_TIMESTAMP, snapshot->timestamp_ms);

    char* json_text = cJSON_PrintUnformatted(json);

    cJSON_Delete(json);
    return json_text;
}

bool busy_timer_snapshot_deserialize(BusyTimerSnapshot* snapshot, const char* json_text) {
    bool success = false;

    cJSON* json = cJSON_Parse(json_text);

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

        success = true;
    } while(false);

    cJSON_Delete(json);

    return success;
}

bool busy_timer_snapshot_is_valid(const BusyTimerSnapshot* snapshot) {
    furi_check(snapshot);

    bool is_valid = false;

    do {
        const BusyTimerSnapshotType type = snapshot->type;

        if(type == BusyTimerSnapshotTypeNotStarted) {
            // Nothing to check
        } else if(type == BusyTimerSnapshotTypeInfinite) {
            const BusyTimerSnapshotInfinite* infinite = &snapshot->infinite;
            if(!busy_timer_snapshot_is_valid_card_id(infinite->common.card_id)) {
                break;
            }

        } else if(type == BusyTimerSnapshotTypeSimple) {
            const BusyTimerSnapshotSimple* simple = &snapshot->simple;
            if(!busy_timer_snapshot_is_valid_card_id(simple->common.card_id)) {
                break;
            }

            if(simple->time_left_ms > M_TO_MS(BUSY_TIMER_TIME_MAX_MN)) {
                break;
            }

        } else if(type == BusyTimerSnapshotTypeInterval) {
            const BusyTimerSnapshotInterval* interval = &snapshot->interval;
            if(!busy_timer_snapshot_is_valid_card_id(interval->common.card_id)) {
                break;
            }

            const BusyTimerIntervalSettings* settings = &interval->settings;
            if(settings->cycles_count < BUSY_TIMER_CYCLE_COUNT_MIN ||
               settings->cycles_count > BUSY_TIMER_CYCLE_COUNT_MAX) {
                break;
            }

            if(settings->work_time_ms < M_TO_MS(BUSY_TIMER_WORK_TIME_MIN_MN) ||
               settings->work_time_ms > M_TO_MS(BUSY_TIMER_WORK_TIME_MAX_MN)) {
                break;
            }

            if(settings->rest_time_ms < M_TO_MS(BUSY_TIMER_REST_TIME_MIN_MN) ||
               settings->rest_time_ms > M_TO_MS(BUSY_TIMER_REST_TIME_MAX_MN)) {
                break;
            }

            const BusyTimerIntervalState* state = &interval->state;
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

        } else {
            break;
        }

        is_valid = true;

    } while(false);

    return is_valid;
}
