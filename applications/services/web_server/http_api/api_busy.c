#include "http_api.h"

#include <busy_timer/busy_timer.h>

#include <cjson/cJSON.h>

#define TAG "HttpBusy"

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

typedef struct {
    HttpHandlersList_t handlers;
} ApiBusyCtx;

static const char* const snapshot_type_values[BusyTimerSnapshotTypeMax] = {
    [BusyTimerSnapshotTypeNotStarted] = "NOT_STARTED",
    [BusyTimerSnapshotTypeInfinite] = "INFINITE",
    [BusyTimerSnapshotTypeSimple] = "SIMPLE",
    [BusyTimerSnapshotTypeInterval] = "INTERVAL",
};

static void
    api_busy_serialize_snapshot_common(cJSON* json, const BusyTimerSnapshotCommon* common) {
    cJSON_AddNumberToObject(json, KEY_SNAPSHOT_COMMON_CARD_ID, common->card_id);
    cJSON_AddBoolToObject(json, KEY_SNAPSHOT_COMMON_IS_PAUSED, common->is_paused);
}

static void
    api_busy_serialize_snapshot_infinite(cJSON* json, const BusyTimerSnapshotInfinite* infinite) {
    api_busy_serialize_snapshot_common(json, &infinite->common);
}

static void
    api_busy_serialize_snapshot_simple(cJSON* json, const BusyTimerSnapshotSimple* simple) {
    api_busy_serialize_snapshot_common(json, &simple->common);

    cJSON_AddNumberToObject(json, KEY_SNAPSHOT_SIMPLE_TIME_LEFT, simple->time_left_ms);
}

static void api_busy_serialize_snapshot_interval_settings(
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

static void
    api_busy_serialize_snapshot_interval(cJSON* json, const BusyTimerSnapshotInterval* interval) {
    api_busy_serialize_snapshot_common(json, &interval->common);

    cJSON_AddNumberToObject(json, KEY_SNAPSHOT_INTERVAL_CURRENT_ID, interval->state.index);
    cJSON_AddNumberToObject(
        json, KEY_SNAPSHOT_INTERVAL_CURRENT_TOTAL, interval->state.time_total_ms);
    cJSON_AddNumberToObject(
        json, KEY_SNAPSHOT_INTERVAL_CURRENT_LEFT, interval->state.time_left_ms);

    if(interval->has_settings) {
        api_busy_serialize_snapshot_interval_settings(json, &interval->settings);
    } else {
        cJSON_AddNullToObject(json, KEY_SNAPSHOT_INTERVAL_SETTINGS);
    }
}

static void api_busy_serialize_snapshot_root(cJSON* json, const BusyTimerSnapshot* snapshot) {
    cJSON* snapshot_json = cJSON_AddObjectToObject(json, KEY_SNAPSHOT);

    const BusyTimerSnapshotType snapshot_type = snapshot->type;
    furi_check(snapshot_type < BusyTimerSnapshotTypeMax);

    cJSON_AddStringToObject(snapshot_json, KEY_SNAPSHOT_TYPE, snapshot_type_values[snapshot_type]);

    if(snapshot_type == BusyTimerSnapshotTypeInfinite) {
        api_busy_serialize_snapshot_infinite(snapshot_json, &snapshot->infinite);
    } else if(snapshot_type == BusyTimerSnapshotTypeSimple) {
        api_busy_serialize_snapshot_simple(snapshot_json, &snapshot->simple);
    } else if(snapshot_type == BusyTimerSnapshotTypeInterval) {
        api_busy_serialize_snapshot_interval(snapshot_json, &snapshot->interval);
    }

    cJSON_AddNumberToObject(json, KEY_TIMESTAMP, snapshot->timestamp_ms);
}

static bool api_busy_parse_snapshot_common(const cJSON* json, BusyTimerSnapshotCommon* common) {
    bool success = false;

    do {
        const cJSON* item;

        item = cJSON_GetObjectItem(json, KEY_SNAPSHOT_COMMON_CARD_ID);
        if(!cJSON_IsNumber(item)) {
            break;
        }

        common->card_id = cJSON_GetNumberValue(item);

        item = cJSON_GetObjectItem(json, KEY_SNAPSHOT_COMMON_IS_PAUSED);
        if(!cJSON_IsBool(item)) {
            break;
        }

        common->is_paused = cJSON_IsTrue(item);

        success = true;
    } while(false);

    return success;
}

static bool
    api_busy_parse_snapshot_infinite(const cJSON* json, BusyTimerSnapshotInfinite* infinite) {
    return api_busy_parse_snapshot_common(json, &infinite->common);
}

static bool api_busy_parse_snapshot_simple(const cJSON* json, BusyTimerSnapshotSimple* simple) {
    bool success = false;

    do {
        if(!api_busy_parse_snapshot_common(json, &simple->common)) {
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

static bool
    api_busy_parse_interval_settings(const cJSON* json, BusyTimerIntervalSettings* settings) {
    bool success = false;

    do {
        if(!cJSON_IsObject(json)) {
            break;
        }

        const cJSON* item;

        item = cJSON_GetObjectItem(json, KEY_SNAPSHOT_TYPE);
        if(!cJSON_IsString(item)) {
            break;
        }

        const char* type_str = cJSON_GetStringValue(item);
        furi_check(type_str);

        if(strcmp(type_str, snapshot_type_values[BusyTimerSnapshotTypeInterval]) != 0) {
            break;
        }

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

static bool
    api_busy_parse_snapshot_interval(const cJSON* json, BusyTimerSnapshotInterval* interval) {
    bool success = false;

    do {
        if(!api_busy_parse_snapshot_common(json, &interval->common)) {
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

        if(cJSON_IsObject(item)) {
            if(!api_busy_parse_interval_settings(item, &interval->settings)) break;
            interval->has_settings = true;

        } else if(cJSON_IsNull(item)) {
            interval->has_settings = false;
        } else {
            break;
        }

        success = true;
    } while(false);

    return success;
}

static BusyTimerSnapshotType api_busy_parse_snapshot_type(const cJSON* json) {
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

static bool api_busy_parse_snapshot(const cJSON* json, BusyTimerSnapshot* snapshot) {
    bool success = false;

    do {
        if(!cJSON_IsObject(json)) {
            break;
        }

        const cJSON* item;

        item = cJSON_GetObjectItem(json, KEY_SNAPSHOT_TYPE);
        const BusyTimerSnapshotType snapshot_type = api_busy_parse_snapshot_type(item);

        if(snapshot_type == BusyTimerSnapshotTypeNotStarted) {
            /* Nothing */
        } else if(snapshot_type == BusyTimerSnapshotTypeInfinite) {
            if(!api_busy_parse_snapshot_infinite(json, &snapshot->infinite)) break;
        } else if(snapshot_type == BusyTimerSnapshotTypeSimple) {
            if(!api_busy_parse_snapshot_simple(json, &snapshot->simple)) break;
        } else if(snapshot_type == BusyTimerSnapshotTypeInterval) {
            if(!api_busy_parse_snapshot_interval(json, &snapshot->interval)) break;
        } else {
            break;
        }

        snapshot->type = snapshot_type;

        success = true;
    } while(false);

    return success;
}

static bool api_busy_parse_snapshot_root(const cJSON* json, BusyTimerSnapshot* snapshot) {
    bool success = false;

    do {
        if(!cJSON_IsObject(json)) {
            break;
        }

        const cJSON* item;

        item = cJSON_GetObjectItem(json, KEY_SNAPSHOT);
        if(!api_busy_parse_snapshot(item, snapshot)) {
            break;
        }

        item = cJSON_GetObjectItem(json, KEY_TIMESTAMP);
        if(!cJSON_IsNumber(item)) {
            break;
        }

        snapshot->timestamp_ms = cJSON_GetNumberValue(item);

        success = true;
    } while(false);

    return success;
}

static bool api_busy_get_snapshot_callback(
    FuriString* path,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    UNUSED(ctx);
    UNUSED(msg);

    if(!IS_HTTP_ENDPOINT(path)) {
        return false;
    }

    BusyTimer* timer = furi_record_open(RECORD_BUSY_TIMER);

    BusyTimerSnapshot snapshot;
    busy_timer_get_snapshot(timer, &snapshot);

    furi_record_close(RECORD_BUSY_TIMER);

    cJSON* root = cJSON_CreateObject();
    api_busy_serialize_snapshot_root(root, &snapshot);

    char* body = cJSON_Print(root);
    furi_check(body);

    MG_REPLY_OK_BODY(conn, body);

    cJSON_Delete(root);
    free(body);

    return true;
}

static bool api_busy_set_snapshot_callback(
    FuriString* path,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    UNUSED(ctx);

    if(!IS_HTTP_ENDPOINT(path)) {
        return false;
    }

    bool success = false;
    const char* error_msg;
    cJSON* root = cJSON_Parse(msg->body.buf);

    BusyTimer* timer = furi_record_open(RECORD_BUSY_TIMER);

    do {
        BusyTimerSnapshot snapshot;
        if(!api_busy_parse_snapshot_root(root, &snapshot)) {
            error_msg = "Failed to parse snapshot";
            break;
        }

        if(!busy_timer_set_snapshot(timer, &snapshot)) {
            error_msg = "Failed to set snapshot";
            break;
        }

        success = true;
    } while(false);

    furi_record_close(RECORD_BUSY_TIMER);

    cJSON_Delete(root);

    if(success) {
        MG_REPLY_OK(conn);
    } else {
        MG_REPLY_ERROR(conn, 400, error_msg);
    }

    return true;
}

static const HttpHandler handlers_busy[] = {
    {
        .uri = "snapshot",
        .method = "GET",
        .type = HttpHandlerCustom,
        .on_request = api_busy_get_snapshot_callback,
    },
    {
        .uri = "snapshot",
        .method = "POST",
        .type = HttpHandlerCustom,
        .on_request = api_busy_set_snapshot_callback,
    },
};

void* http_api_busy_alloc(void) {
    ApiBusyCtx* context = malloc(sizeof(ApiBusyCtx));

    HttpHandlersList_init(context->handlers);
    for(size_t i = 0; i < COUNT_OF(handlers_busy); ++i) {
        http_handler_add(context->handlers, &handlers_busy[i]);
    }

    return context;
}

void http_api_busy_free(void* ctx) {
    furi_assert(ctx);
    ApiBusyCtx* context = ctx;

    HttpHandlersList_clear(context->handlers);
    free(context);
}

bool http_api_busy_callback(
    FuriString* path,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    ApiBusyCtx* context = ctx;
    return http_handle_request(path, context->handlers, conn, msg);
}
