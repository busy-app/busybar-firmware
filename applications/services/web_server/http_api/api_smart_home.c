#include "http_api.h"

#include <matter/matter.h>
#include <cjson/cJSON.h>
#include <furi_hal_rtc.h>
#include <toolbox/value_index.h>

#define TAG "HttpSmartHome"

typedef struct {
    HttpHandlersList_t handlers;
} ApiMatterCtx;

static void
    api_smart_home_pairing_status(struct mg_connection* conn, struct mg_http_message* msg) {
    UNUSED(msg);

    Matter* matter = furi_record_open(RECORD_MATTER);
    MatterCommissionedFabrics fabrics;
    const MatterStatus status = matter_get_commissioned_fabrics(matter, &fabrics);
    furi_record_close(RECORD_MATTER);

    if(status != MatterStatusOk) {
        MG_REPLY_ERROR(conn, 503, "Smart home unavailable");
        return;
    }

    cJSON* object = cJSON_CreateObject();

    cJSON_AddNumberToObject(object, "fabric_count", fabrics.count);
    cJSON* status_upd = cJSON_AddObjectToObject(object, "latest_pairing_status");

    static const char* const status_string[MatterCommissioningStatusMAX] = {
        [MatterCommissioningStatusNeverStarted] = "never_started",
        [MatterCommissioningStatusStarted] = "started",
        [MatterCommissioningStatusComplete] = "completed_successfully",
        [MatterCommissioningStatusFailed] = "failed",
    };
    cJSON_AddStringToObject(status_upd, "value", status_string[fabrics.last_status]);

    if(fabrics.last_status_at) {
        cJSON_AddNumberToObject(status_upd, "timestamp", fabrics.last_status_at / 1000);
    }

    char* serialized = cJSON_PrintUnformatted(object);
    cJSON_Delete(object);
    MG_REPLY_OK_BODY(conn, serialized);
    free(serialized);
}

static void
    api_smart_home_enable_pairing(struct mg_connection* conn, struct mg_http_message* msg) {
    UNUSED(msg);

    bool success = false;

    do {
        Matter* matter = furi_record_open(RECORD_MATTER);

        MatterCommissioningInfo info;
        const MatterStatus status = matter_enable_commissioning(matter, &info);

        furi_record_close(RECORD_MATTER);

        if(status != MatterStatusOk) {
            break;
        }

        cJSON* object = cJSON_CreateObject();

        time_t available_until = furi_hal_rtc_get_timestamp_ms() + (info.window_duration_s * 1000);
        char timestamp[32];
        snprintf(timestamp, sizeof(timestamp), "%" PRIu64, available_until);
        cJSON_AddStringToObject(object, "available_until", timestamp);
        cJSON_AddStringToObject(object, "qr_code", info.qr_code);
        cJSON_AddStringToObject(object, "manual_code", info.manual_code);

        char* serialized = cJSON_PrintUnformatted(object);
        cJSON_Delete(object);
        MG_REPLY_OK_BODY(conn, serialized);
        free(serialized);
        success = true;
    } while(0);

    if(!success) MG_REPLY_ERROR(conn, 503, "Smart home unavailable");
}

static void api_smart_home_factory_reset(struct mg_connection* conn, struct mg_http_message* msg) {
    UNUSED(msg);

    Matter* matter = furi_record_open(RECORD_MATTER);
    MatterStatus matter_status = matter_factory_reset(matter, MatterRebootAutomatically);
    furi_record_close(RECORD_MATTER);

    if(matter_status == MatterStatusOk) {
        MG_REPLY_OK(conn);
    } else {
        MG_REPLY_ERROR(conn, 503, "Smart home unavailable");
    }
}

static bool api_smart_home_pairing_callback(
    FuriString* path,
    HttpMethod method,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* untyped_ctx) {
    UNUSED(msg);
    UNUSED(untyped_ctx);

    if(!IS_HTTP_ENDPOINT(path)) return false;

    if(method == HttpMethodGet) {
        api_smart_home_pairing_status(conn, msg);
    } else if(method == HttpMethodPost) {
        api_smart_home_enable_pairing(conn, msg);
    } else if(method == HttpMethodDelete) {
        api_smart_home_factory_reset(conn, msg);
    }

    return true;
}

static void api_smart_home_switch_get(struct mg_connection* conn, struct mg_http_message* msg) {
    UNUSED(msg);

    Matter* matter = furi_record_open(RECORD_MATTER);

    MatterSwitchState switch_state;
    furi_state_get(matter_get_switch_state(matter), &switch_state);
    furi_record_close(RECORD_MATTER);

    if(switch_state == MatterSwitchStateUnknown) {
        MG_REPLY_ERROR(conn, 503, "Smart home unavailable");
        return;
    }

    cJSON* object = cJSON_CreateObject();

    cJSON_AddBoolToObject(object, "state", switch_state == MatterSwitchStateOn);

    char* serialized = cJSON_PrintUnformatted(object);
    cJSON_Delete(object);
    MG_REPLY_OK_BODY(conn, serialized);
    free(serialized);
}

static void api_smart_home_switch_set(struct mg_connection* conn, struct mg_http_message* msg) {
    bool success = false;
    bool matter_request_error = false;
    char* switch_startup = NULL;

    Matter* matter = furi_record_open(RECORD_MATTER);
    do {
        bool has_switch_state = false;
        bool switch_state;
        has_switch_state = mg_json_get_bool(msg->body, "$.state", &switch_state);

        switch_startup = mg_json_get_str(msg->body, "$.startup");
        bool has_switch_startup = !!switch_startup;

        if(!has_switch_state && !has_switch_startup) break;

        if(has_switch_state) {
            const MatterSwitchState switch_val = switch_state ? MatterSwitchStateOn :
                                                                MatterSwitchStateOff;

            if(matter_set_switch_state(matter, switch_val) != MatterStatusOk) {
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

            if(matter_set_switch_startup_mode(matter, startup) != MatterStatusOk) {
                matter_request_error = true;
                break;
            }
        }

        success = true;
    } while(0);
    furi_record_close(RECORD_MATTER);

    if(switch_startup) free(switch_startup);

    if(success) {
        MG_REPLY_OK(conn);
    } else {
        if(matter_request_error) {
            MG_REPLY_ERROR(conn, 503, "Smart home unavailable");
        } else {
            MG_REPLY_BAD_REQUEST(conn);
        }
    }
}

static bool api_smart_home_switch_callback(
    FuriString* path,
    HttpMethod method,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* untyped_ctx) {
    UNUSED(untyped_ctx);

    if(!IS_HTTP_ENDPOINT(path)) return false;

    if(method == HttpMethodGet) {
        api_smart_home_switch_get(conn, msg);
    } else if(method == HttpMethodPost) {
        api_smart_home_switch_set(conn, msg);
    }

    return true;
}

static const HttpHandler handlers_matter[] = {
    {
        .uri = "pairing",
        .method = HttpMethodGet | HttpMethodPost | HttpMethodDelete,
        .type = HttpHandlerCustom,
        .on_request = api_smart_home_pairing_callback,
    },
    {
        .uri = "switch",
        .method = HttpMethodGet | HttpMethodPost,
        .type = HttpHandlerCustom,
        .on_request = api_smart_home_switch_callback,
    },
};

void* http_api_smart_home_alloc(void) {
    ApiMatterCtx* context = malloc(sizeof(ApiMatterCtx));

    HttpHandlersList_init(context->handlers);
    for(size_t i = 0; i < COUNT_OF(handlers_matter); ++i) {
        http_handler_add(context->handlers, &handlers_matter[i]);
    }

    return context;
}

void http_api_smart_home_free(void* ctx) {
    furi_assert(ctx);
    ApiMatterCtx* context = ctx;

    HttpHandlersList_clear(context->handlers);

    free(context);
}

bool http_api_smart_home_callback(
    FuriString* path,
    HttpMethod method,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    ApiMatterCtx* context = ctx;
    return http_handle_request(path, method, context->handlers, conn, msg);
}
