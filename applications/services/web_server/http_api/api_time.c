#include "http_api.h"

#include <time/time.h>
#include <furi_hal_rtc.h>
#include <datetime.h>
#include <furi.h>
#include <tzutil.h>
#include <utz/zones.h>

#define TAG "HttpTime"

#define API_TIME_H_TO_M(x) ((x) * 60)
#define API_TIME_M_TO_S(x) ((x) * 60)

static bool api_time_get_timestamp_callback(
    FuriString* path,
    HttpMethod method,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    UNUSED(msg);
    UNUSED(ctx);

    if(!IS_HTTP_ENDPOINT(path)) return false;
    if(method == HttpMethodOptions) {
        http_reply_cors_preflight(conn, HttpMethodGet);
        return true;
    } else if(method != HttpMethodGet) {
        http_reply_405_method_not_allowed(conn, HttpMethodGet, false);
        return true;
    }

    Time* time = furi_record_open(RECORD_TIME);
    LocalTime local_time = time_get_local_time(time);

    furi_record_close(RECORD_TIME);

    char timestamp_buf[DATETIME_TIMESTAMP_STR_LEN + 1];

    datetime_format_timestamp(&local_time, timestamp_buf);

    MG_REPLY_OK_BODY(conn, "{\"timestamp\":\"%s\"}\n", timestamp_buf);

    return true;
}

static bool api_time_set_timestamp_callback(
    FuriString* path,
    HttpMethod method,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    UNUSED(method);
    UNUSED(ctx);

    if(!IS_HTTP_ENDPOINT(path)) return false;

    bool is_success = false;
    do {
        if(msg->query.len == 0) {
            break;
        }

        char timestamp_str[DATETIME_TIMESTAMP_STR_LEN + 1];
        int len = mg_http_get_var(&msg->query, "timestamp", timestamp_str, sizeof(timestamp_str));
        if(len <= 0) {
            break;
        }

        DateTime datetime;
        if(!datetime_parse_timestamp(timestamp_str, &datetime)) {
            FURI_LOG_E(TAG, "Error parsing timestamp: %s", timestamp_str);
            break;
        }

        furi_hal_rtc_set_datetime(&(DateTimeMs){.dt = datetime, .millis = 0});

        is_success = true;
    } while(false);

    if(is_success) {
        MG_REPLY_OK(conn);
    } else {
        MG_REPLY_BAD_REQUEST(conn);
    }

    return true;
}

static FuriString* format_zone_info_json(const TzutilTzInfo* info) {
    char abbr[TZUTIL_MAX_ABBR_LEN + 1];
    tzutil_get_abbr(info, abbr, sizeof(abbr));

    char offset_buf[DATETIME_OFFSET_STR_LEN + 1];

    datetime_format_offset(&info->offset, offset_buf);

    FuriString* result = furi_string_alloc_printf(
        "{\"name\":\"%s\",\"offset\":\"%s\",\"abbr\":\"%s\"}", info->name, offset_buf, abbr);
    return result;
}

static void api_time_set_timezone(struct mg_connection* conn, struct mg_http_message* msg) {
    bool is_success = false;
    do {
        if(msg->query.len == 0) {
            break;
        }

        char timezone_str[UTZ_MAX_ZONE_NAME_LEN + 1];
        if(mg_http_get_var(&msg->query, "timezone", timezone_str, sizeof(timezone_str)) <= 0) {
            break;
        }

        utz_zone_t zone;
        if(!utz_get_zone_by_name(timezone_str, &zone)) {
            break;
        }

        FURI_LOG_D(TAG, "Set timezone %s", zone.name);

        Time* time = furi_record_open(RECORD_TIME);
        TimeSettings settings;
        time_get_settings(time, &settings);
        settings.timezone = zone;
        is_success = time_set_settings(time, &settings);
        furi_record_close(RECORD_TIME);
    } while(false);

    if(is_success) {
        MG_REPLY_OK(conn);
    } else {
        MG_REPLY_BAD_REQUEST(conn);
    }
}

static void api_time_get_timezone(struct mg_connection* conn, struct mg_http_message* msg) {
    UNUSED(msg);
    bool success = false;
    FuriString* response = NULL;

    do {
        Time* time = furi_record_open(RECORD_TIME);
        TimeSettings settings;
        time_get_settings(time, &settings);
        furi_record_close(RECORD_TIME);

        DateTime now = furi_hal_rtc_get_datetime().dt;
        TzutilTzInfo info;
        if(!tzutil_get_info_by_name(settings.timezone.name, &now, &info)) break;

        response = format_zone_info_json(&info);

        success = true;
    } while(false);

    if(success) {
        furi_check(response);
        MG_REPLY_OK_BODY(conn, "%s", furi_string_get_cstr(response));
        furi_string_free(response);
    } else {
        furi_check(!response);
        MG_REPLY_BAD_REQUEST(conn);
    }
}

static bool api_time_timezone_callback(
    FuriString* path,
    HttpMethod method,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    UNUSED(ctx);
    UNUSED(msg);

    if(!IS_HTTP_ENDPOINT(path)) return false;

    if(method == HttpMethodGet) {
        api_time_get_timezone(conn, msg);
    } else if(method == HttpMethodPost) {
        api_time_set_timezone(conn, msg);
    }

    return true;
}

static FuriString* generate_zone_list_json(const TzutilTzInfoList* infos) {
    furi_check(infos);

    FuriString* r = furi_string_alloc_set("{\"list\":[");
    for(size_t i = 0; i != infos->count; ++i) {
        FuriString* obj = format_zone_info_json(infos->entries + i);

        if(i != 0) {
            furi_string_cat(r, ",");
        }

        furi_string_cat(r, obj);
        furi_string_free(obj);
    }
    furi_string_cat(r, "]}");
    return r;
}

static bool api_time_get_timezone_list_callback(
    FuriString* path,
    HttpMethod method,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    UNUSED(method);
    UNUSED(ctx);
    UNUSED(msg);

    if(!IS_HTTP_ENDPOINT(path)) return false;

    DateTime now = furi_hal_rtc_get_datetime().dt;

    TzutilTzInfoList infos = tzutil_compile_zone_list(&now);

    FuriString* result = generate_zone_list_json(&infos);

    if(result) {
        MG_REPLY_OK_BODY(conn, "%s", furi_string_get_cstr(result));
        furi_string_free(result);
    } else {
        MG_REPLY_BAD_REQUEST(conn);
    }

    tzutil_info_list_free(&infos);

    return true;
}

static const HttpHandler api_time_handlers[] = {
    {
        .uri = "",
        .method = HttpMethodAny,
        .type = HttpHandlerCustom,
        .on_request = api_time_get_timestamp_callback,
    },
    {
        .uri = "timestamp",
        .method = HttpMethodPost,
        .type = HttpHandlerCustom,
        .on_request = api_time_set_timestamp_callback,
    },
    {
        .uri = "timezone",
        .method = HttpMethodGet | HttpMethodPost,
        .type = HttpHandlerCustom,
        .on_request = api_time_timezone_callback,
    },
    {
        .uri = "tzlist",
        .method = HttpMethodGet,
        .type = HttpHandlerCustom,
        .on_request = api_time_get_timezone_list_callback,
    },
};

typedef struct {
    HttpHandlersList_t handlers;
} ApiTimeCtx;

void* http_api_time_alloc(void) {
    ApiTimeCtx* context = malloc(sizeof(ApiTimeCtx));
    HttpHandlersList_init(context->handlers);

    for(size_t i = COUNT_OF(api_time_handlers); i > 0; i--) {
        http_handler_add(context->handlers, &api_time_handlers[i - 1]);
    }

    return context;
}

void http_api_time_free(void* ctx) {
    furi_assert(ctx);

    ApiTimeCtx* context = ctx;
    HttpHandlersList_clear(context->handlers);
    free(context);
}

bool http_api_time_callback(
    FuriString* path,
    HttpMethod method,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    ApiTimeCtx* context = ctx;

    return http_handle_request(path, method, context->handlers, conn, msg);
}
