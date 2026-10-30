#include "http_api.h"
#include <low_power/low_power.h>

#include <toolbox/value_index.h>

#define TAG "HttpPower"

static bool api_power_sleep_get(
    FuriString* path,
    HttpMethod method,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    UNUSED(method);
    UNUSED(msg);
    UNUSED(ctx);

    if(!IS_HTTP_ENDPOINT(path)) return false;

    LowPowerSleepSettings settings;
    LowPower* low_power = furi_record_open(RECORD_LOW_POWER);
    low_power_get_sleep_settings(low_power, &settings);
    furi_record_close(RECORD_LOW_POWER);

    if(settings.mode == LowPowerModeDisabled) {
        MG_REPLY_OK_BODY(conn, "{\"sleep\":\"never\"}\n");
    } else if(settings.mode == LowPowerModeSleepAfterHoldoff) {
        MG_REPLY_OK_BODY(
            conn,
            "{\"sleep\":\"after_holdoff\",\"sleep_holdoff\":%lu}\n",
            settings.holdoff_duration);
    }

    return true;
}

static bool api_power_sleep_set(
    FuriString* path,
    HttpMethod method,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    UNUSED(method);
    UNUSED(ctx);

    if(!IS_HTTP_ENDPOINT(path)) return false;

    bool success = false;
    do {
        char* mode_str = mg_json_get_str(msg->body, "$.sleep");
        if(!mode_str) break;

        static const char* const mode_str_mapping[LowPowerModeMAX] = {
            [LowPowerModeSleepAfterHoldoff] = "after_holdoff",
            [LowPowerModeDisabled] = "never",
        };
        LowPowerMode mode =
            value_index_string(mode_str, mode_str_mapping, COUNT_OF(mode_str_mapping));
        free(mode_str);
        if(mode >= LowPowerModeMAX) break;

        double holdoff_double = 0;
        if(!mg_json_get_num(msg->body, "$.sleep_holdoff", &holdoff_double)) {
            if(mode == LowPowerModeSleepAfterHoldoff) break;
        }
        if(holdoff_double < 0) break;

        LowPowerSleepSettings settings = {
            .mode = mode,
            .holdoff_duration = holdoff_double,
        };

        LowPower* low_power = furi_record_open(RECORD_LOW_POWER);
        low_power_set_sleep_settings(low_power, &settings);
        furi_record_close(RECORD_LOW_POWER);
        success = true;
    } while(0);

    if(success) {
        MG_REPLY_OK(conn);
    } else {
        MG_REPLY_BAD_REQUEST(conn);
    }

    return true;
}

static bool api_power_sleep_get_or_set(
    FuriString* path,
    HttpMethod method,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    if(method == HttpMethodGet) {
        return api_power_sleep_get(path, method, conn, msg, ctx);
    } else if(method == HttpMethodPost) {
        return api_power_sleep_set(path, method, conn, msg, ctx);
    }

    return false;
}

static const HttpHandler api_power_handlers[] = {
    {
        .uri = "sleep",
        .method = HttpMethodGet | HttpMethodPost,
        .type = HttpHandlerCustom,
        .on_request = api_power_sleep_get_or_set,
    },
};

typedef struct {
    HttpHandlersList_t handlers;
} ApiPowerCtx;

void* http_api_power_alloc(void) {
    ApiPowerCtx* context = malloc(sizeof(ApiPowerCtx));
    HttpHandlersList_init(context->handlers);

    for(size_t i = COUNT_OF(api_power_handlers); i > 0; i--) {
        http_handler_add(context->handlers, &api_power_handlers[i - 1]);
    }
    return context;
}

void http_api_power_free(void* ctx) {
    furi_assert(ctx);
    ApiPowerCtx* context = ctx;
    HttpHandlersList_clear(context->handlers);
    free(context);
}

bool http_api_power_callback(
    FuriString* path,
    HttpMethod method,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    ApiPowerCtx* context = ctx;

    return http_handle_request(path, method, context->handlers, conn, msg);
}
