#include "http_api.h"

#include <busy_timer/busy_timer.h>

#define TAG "HttpBusy"

typedef struct {
    HttpHandlersList_t handlers;
} ApiBusyCtx;

static const char* api_busy_profile_path_table[BusyTimerProfileIdMax] = {
    [BusyTimerProfileIdBusy] = "busy",
    [BusyTimerProfileIdCustom] = "custom",
};

static BusyTimerProfileId api_busy_find_profile_id_by_path(const FuriString* name) {
    BusyTimerProfileId id;

    for(id = 0; id < BusyTimerProfileIdMax; ++id) {
        if(furi_string_equal(name, api_busy_profile_path_table[id])) {
            break;
        }
    }

    return id;
}

static void api_busy_get_snapshot(struct mg_connection* conn, struct mg_http_message* msg) {
    UNUSED(msg);

    BusyTimer* timer = furi_record_open(RECORD_BUSY_TIMER);
    BusyTimerSnapshot snapshot;
    busy_timer_get_snapshot(timer, &snapshot);
    furi_record_close(RECORD_BUSY_TIMER);

    char* json_text = busy_timer_snapshot_serialize(&snapshot);
    furi_check(json_text);

    MG_REPLY_OK_BODY(conn, json_text);
    free(json_text);
}

static void api_busy_set_snapshot(struct mg_connection* conn, struct mg_http_message* msg) {
    bool success = false;
    const char* error_msg;

    do {
        BusyTimerSnapshot snapshot;
        if(!busy_timer_snapshot_deserialize(&snapshot, msg->body.buf, msg->body.len)) {
            error_msg = "Failed to parse snapshot";
            break;
        }

        BusyTimer* timer = furi_record_open(RECORD_BUSY_TIMER);
        busy_timer_set_snapshot(timer, &snapshot, BusyTimerSessionSourceHttpApi);
        furi_record_close(RECORD_BUSY_TIMER);

        success = true;
    } while(false);

    if(success) {
        MG_REPLY_OK(conn);
    } else {
        MG_REPLY_ERROR(conn, 400, error_msg);
    }
}

static bool api_busy_snapshot_callback(
    FuriString* path,
    HttpMethod method,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    UNUSED(path);
    UNUSED(ctx);

    if(method == HttpMethodGet) {
        api_busy_get_snapshot(conn, msg);
    } else if(method == HttpMethodPut) {
        api_busy_set_snapshot(conn, msg);
    }

    return true;
}

static void api_busy_get_profile(
    FuriString* path,
    struct mg_connection* conn,
    struct mg_http_message* msg) {
    UNUSED(msg);

    const BusyTimerProfileId profile_id = api_busy_find_profile_id_by_path(path);

    if(profile_id < BusyTimerProfileIdMax) {
        BusyTimerProfile profile;

        BusyTimer* timer = furi_record_open(RECORD_BUSY_TIMER);
        busy_timer_get_profile(timer, profile_id, &profile);
        furi_record_close(RECORD_BUSY_TIMER);

        char* json_text = busy_timer_profile_serialize(&profile);
        MG_REPLY_OK_BODY(conn, json_text);
        free(json_text);

    } else {
        MG_REPLY_ERROR(conn, 400, "Invalid profile slot name");
    }
}

static void api_busy_set_profile(
    FuriString* path,
    struct mg_connection* conn,
    struct mg_http_message* msg) {
    bool success = false;
    const char* error_msg;

    do {
        const BusyTimerProfileId profile_id = api_busy_find_profile_id_by_path(path);

        if(profile_id >= BusyTimerProfileIdMax) {
            error_msg = "Invalid profile slot name";
            break;
        }

        BusyTimerProfile profile;
        if(!busy_timer_profile_deserialize(&profile, msg->body.buf, msg->body.len)) {
            error_msg = "Failed to parse profile";
            break;
        }

        BusyTimer* timer = furi_record_open(RECORD_BUSY_TIMER);
        busy_timer_set_profile(timer, profile_id, &profile);
        furi_record_close(RECORD_BUSY_TIMER);

        success = true;
    } while(false);

    if(success) {
        MG_REPLY_OK(conn);
    } else {
        MG_REPLY_ERROR(conn, 400, error_msg);
    }
}

static bool api_busy_profile_callback(
    FuriString* path,
    HttpMethod method,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    UNUSED(ctx);

    if(method == HttpMethodGet) {
        api_busy_get_profile(path, conn, msg);
    } else if(method == HttpMethodPut) {
        api_busy_set_profile(path, conn, msg);
    }

    return true;
}

static const HttpHandler handlers_busy[] = {
    {
        .uri = "snapshot",
        .method = HttpMethodGet | HttpMethodPut,
        .type = HttpHandlerCustom,
        .on_request = api_busy_snapshot_callback,
    },
    {
        .uri = "profiles",
        .method = HttpMethodGet | HttpMethodPut,
        .type = HttpHandlerCustom,
        .on_request = api_busy_profile_callback,
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
    HttpMethod method,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    ApiBusyCtx* context = ctx;
    return http_handle_request(path, method, context->handlers, conn, msg);
}
