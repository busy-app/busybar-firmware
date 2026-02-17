#include "http_api.h"

#include <matter/matter.h>
#include <cjson/cJSON.h>
#include <furi_hal_rtc.h>
#include <toolbox/value_index.h>

#define TAG "HttpMatter"

typedef struct {
    HttpHandlersList_t handlers;
} ApiMatterCtx;

static bool api_matter_commissioning_status(
    FuriString* path,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* untyped_ctx) {
    UNUSED(msg);
    UNUSED(untyped_ctx);

    if(!IS_HTTP_ENDPOINT(path)) return false;

    MatterSrv* matter = furi_record_open(RECORD_MATTER);
    MatterCommissionedFabrics fabrics = matter_commissioned_fabrics(matter);
    furi_record_close(RECORD_MATTER);

    cJSON* object = cJSON_CreateObject();

    cJSON_AddNumberToObject(object, "fabric_count", fabrics.count);
    cJSON* status_upd = cJSON_AddObjectToObject(object, "latest_commissioning_status");

    static const char* const status_string[MatterCommissioningStatusMAX] = {
        [MatterCommissioningStatusNeverStarted] = "never_started",
        [MatterCommissioningStatusStarted] = "started",
        [MatterCommissioningStatusComplete] = "completed_successfully",
        [MatterCommissioningStatusFailed] = "failed",
    };
    cJSON_AddStringToObject(status_upd, "value", status_string[fabrics.last_status]);

    // representing a millisecond timestamp as a number will have precision issues
    if(fabrics.last_status_at) {
        char timestamp[32];
        snprintf(timestamp, sizeof(timestamp), "%" PRIu64, fabrics.last_status_at);
        cJSON_AddStringToObject(status_upd, "timestamp", timestamp);
    }

    char* serialized = cJSON_PrintUnformatted(object);
    cJSON_Delete(object);
    MG_REPLY_OK_BODY(conn, serialized);
    free(serialized);

    return true;
}

static bool api_matter_enable_commissioning(
    FuriString* path,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* untyped_ctx) {
    UNUSED(msg);
    UNUSED(untyped_ctx);

    bool success = false;
    if(!IS_HTTP_ENDPOINT(path)) return success;

    FuriString* qr_code = furi_string_alloc();
    FuriString* manual_code = furi_string_alloc();

    do {
        MatterSrv* matter = furi_record_open(RECORD_MATTER);
        size_t seconds_left = matter_enable_commissioning(matter, qr_code, manual_code);
        furi_record_close(RECORD_MATTER);

        if(!seconds_left) break;

        cJSON* object = cJSON_CreateObject();

        time_t available_until = furi_hal_rtc_get_timestamp_ms() + (seconds_left * 1000);
        char timestamp[32];
        snprintf(timestamp, sizeof(timestamp), "%" PRIu64, available_until);
        cJSON_AddStringToObject(object, "available_until", timestamp);
        cJSON_AddStringToObject(object, "qr_code", furi_string_get_cstr(qr_code));
        cJSON_AddStringToObject(object, "manual_code", furi_string_get_cstr(manual_code));

        char* serialized = cJSON_PrintUnformatted(object);
        cJSON_Delete(object);
        MG_REPLY_OK_BODY(conn, serialized);
        free(serialized);
        success = true;
    } while(0);

    if(!success) MG_REPLY_ERROR(conn, 503, "Matter unavailable");

    furi_string_free(qr_code);
    furi_string_free(manual_code);

    return true;
}

static bool api_matter_factory_reset(
    FuriString* path,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* untyped_ctx) {
    UNUSED(msg);
    UNUSED(untyped_ctx);

    bool success = false;
    if(!IS_HTTP_ENDPOINT(path)) return success;

    MatterSrv* matter = furi_record_open(RECORD_MATTER);
    success = matter_factory_reset(matter);
    furi_record_close(RECORD_MATTER);

    if(success) {
        MG_REPLY_OK(conn);
    } else {
        MG_REPLY_ERROR(conn, 503, "Matter unavailable");
    }

    return true;
}

static bool api_matter_switch_get(
    FuriString* path,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* untyped_ctx) {
    UNUSED(msg);
    UNUSED(untyped_ctx);

    bool handled = true;

    if(!IS_HTTP_ENDPOINT(path)) return false;

    bool state;
    MatterSrv* matter = furi_record_open(RECORD_MATTER);
    bool result = matter_get_switch_state(matter, &state);
    furi_record_close(RECORD_MATTER);

    if(!result) {
        MG_REPLY_ERROR(conn, 503, "Matter unavailable");
        return handled;
    }

    cJSON* object = cJSON_CreateObject();

    cJSON_AddStringToObject(object, "type", "switch");
    cJSON_AddBoolToObject(object, "state", state);

    char* serialized = cJSON_PrintUnformatted(object);
    cJSON_Delete(object);
    MG_REPLY_OK_BODY(conn, serialized);
    free(serialized);
    return handled;
}

static bool api_matter_switch_set(
    FuriString* path,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* untyped_ctx) {
    UNUSED(untyped_ctx);

    bool success = false;
    bool matter_request_error = false;
    char* device_type = NULL;
    char* switch_startup = NULL;

    MatterSrv* matter = furi_record_open(RECORD_MATTER);
    do {
        if(!IS_HTTP_ENDPOINT(path)) break;

        if(!(device_type = mg_json_get_str(msg->body, "$.type"))) break;
        if(strcmp(device_type, "switch") != 0) break;

        bool has_switch_state = false;
        bool switch_state;
        has_switch_state = mg_json_get_bool(msg->body, "$.state", &switch_state);

        switch_startup = mg_json_get_str(msg->body, "$.startup");
        bool has_switch_startup = !!switch_startup;

        if(!has_switch_state && !has_switch_startup) break;

        if(has_switch_state) {
            if(!matter_set_switch_state(matter, switch_state)) {
                matter_request_error = true;
                break;
            }
        }

        if(has_switch_startup) {
            static const char* const switch_startup_modes[MatterSwitchStartupModeMAX] = {
                [MatterSwitchStartupModeOff] = "off",
                [MatterSwitchStartupModeOn] = "on",
                [MatterSwitchStartupModeToggle] = "toggle",
                [MatterSwitchStartupModeLast] = "last",
            };
            MatterSwitchStartupMode startup = value_index_string(
                switch_startup, switch_startup_modes, COUNT_OF(switch_startup_modes));
            furi_assert(startup < MatterSwitchStartupModeMAX);

            if(!matter_set_switch_startup_mode(matter, startup)) {
                matter_request_error = true;
                break;
            }
        }

        success = true;
    } while(0);
    furi_record_close(RECORD_MATTER);

    if(device_type) free(device_type);
    if(switch_startup) free(switch_startup);

    if(success) {
        MG_REPLY_OK(conn);
    } else {
        if(matter_request_error) {
            MG_REPLY_ERROR(conn, 503, "Matter unavailable");
        } else {
            MG_REPLY_BAD_REQUEST(conn);
        }
    }

    return true;
}

static const HttpHandler handlers_matter[] = {
    {
        .uri = "commissioning",
        .method = "GET",
        .type = HttpHandlerCustom,
        .on_request = api_matter_commissioning_status,
    },
    {
        .uri = "commissioning",
        .method = "POST",
        .type = HttpHandlerCustom,
        .on_request = api_matter_enable_commissioning,
    },
    {
        .uri = "commissioning",
        .method = "DELETE",
        .type = HttpHandlerCustom,
        .on_request = api_matter_factory_reset,
    },
    {
        .uri = "endpoint/1",
        .method = "GET",
        .type = HttpHandlerCustom,
        .on_request = api_matter_switch_get,
    },
    {
        .uri = "endpoint/1",
        .method = "POST",
        .type = HttpHandlerCustom,
        .on_request = api_matter_switch_set,
    },
};

void* http_api_matter_alloc(void) {
    ApiMatterCtx* context = malloc(sizeof(ApiMatterCtx));

    HttpHandlersList_init(context->handlers);
    for(size_t i = 0; i < COUNT_OF(handlers_matter); ++i) {
        http_handler_add(context->handlers, &handlers_matter[i]);
    }

    return context;
}

void http_api_matter_free(void* ctx) {
    furi_assert(ctx);
    ApiMatterCtx* context = ctx;

    HttpHandlersList_clear(context->handlers);

    free(context);
}

bool http_api_matter_callback(
    FuriString* path,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    ApiMatterCtx* context = ctx;
    return http_handle_request(path, context->handlers, conn, msg);
}
