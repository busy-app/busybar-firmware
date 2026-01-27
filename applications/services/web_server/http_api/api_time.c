#include "http_api.h"

#include <sntp/sntp.h>
#include <furi_hal_rtc.h>
#include <datetime.h>

#define TAG "HttpTime"

#define API_TIME_H_TO_M(x) ((x) * 60)
#define API_TIME_M_TO_S(x) ((x) * 60)

static bool api_time_get_timestamp_callback(
    FuriString* path,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    UNUSED(msg);
    UNUSED(ctx);

    if(!IS_HTTP_ENDPOINT(path)) return false;

    Sntp* sntp = furi_record_open(RECORD_SNTP);
    LocalTime local_time = sntp_get_local_time(sntp);

    furi_record_close(RECORD_SNTP);

    char timestamp_buf[DATETIME_TIMESTAMP_STR_LEN + 1];

    datetime_format_timestamp(&local_time, timestamp_buf);

    MG_REPLY_OK_BODY(
        conn,
        "{\"timestamp\":\"%s\"}\n",
        timestamp_buf);

    return true;
}

static bool api_time_set_timestamp_callback(
    FuriString* path,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
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

        time_t timestamp = datetime_datetime_to_timestamp(&datetime);

        datetime_timestamp_to_datetime(timestamp, &datetime);

        furi_hal_rtc_set_datetime(&datetime);

        is_success = true;
    } while(false);

    if(is_success) {
        MG_REPLY_OK(conn);
    } else {
        MG_REPLY_BAD_REQUEST(conn);
    }

    return true;
}

static bool api_time_set_timezone_callback(
    FuriString* path,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    UNUSED(ctx);

    if(!IS_HTTP_ENDPOINT(path)) return false;

    bool is_success = false;
    do {
        if(msg->query.len == 0) {
            break;
        }

        char timezone_str[48]; /* reasonably long */
        if(mg_http_get_var(&msg->query, "timezone", timezone_str, sizeof(timezone_str)) <= 0) {
            break;
        }

        utz_zone_t zone;
        if(!utz_get_zone_by_name(timezone_str, &zone)) {
            break;
        }

        FURI_LOG_D(TAG, "Set timezone %s", zone.name);

        Sntp* sntp = furi_record_open(RECORD_SNTP);
        SntpSettings settings;
        sntp_get_settings(sntp, &settings);
        settings.timezone = zone;
        is_success = sntp_set_settings(sntp, &settings);
        furi_record_close(RECORD_SNTP);
    } while(false);

    if(is_success) {
        MG_REPLY_OK(conn);
    } else {
        MG_REPLY_BAD_REQUEST(conn);
    }

    return true;
}

static bool api_time_get_timezone_callback(
    FuriString* path,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    UNUSED(ctx);
    UNUSED(msg);

    if(!IS_HTTP_ENDPOINT(path)) return false;

    bool is_success = true;

    Sntp* sntp = furi_record_open(RECORD_SNTP);
    SntpSettings settings;
    sntp_get_settings(sntp, &settings);

    furi_record_close(RECORD_SNTP);

    if(is_success) {
        MG_REPLY_OK_BODY(conn, "{\"timezone\":\"%s\"}\n", settings.timezone.name);
    } else {
        MG_REPLY_BAD_REQUEST(conn);
    }

    return true;
}

static bool api_time_get_timezone_list_callback(
    FuriString* path,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    UNUSED(ctx);
    UNUSED(msg);

    if(!IS_HTTP_ENDPOINT(path)) return false;

    bool is_success = true;

    FuriString *result = furi_string_alloc_set_str("[");
    const char *item = utz_zone_names;
    furi_string_cat_printf(result, "\"%s\"", item);
    while((item = utz_next_zone_name(item))) {
        furi_string_cat_printf(result, ",\"%s\"", item);
    }
    furi_string_cat_str(result, "]");

    if(is_success) {
        MG_REPLY_OK_BODY(conn, "%s", furi_string_get_cstr(result));
    } else {
        MG_REPLY_BAD_REQUEST(conn);
    }

    furi_string_free(result);

    return true;
}

static const HttpHandler api_time_handlers[] = {
    {
        .uri = "",
        .method = "GET",
        .type = HttpHandlerCustom,
        .on_request = api_time_get_timestamp_callback,
    },
    {
        .uri = "timestamp",
        .method = "POST",
        .type = HttpHandlerCustom,
        .on_request = api_time_set_timestamp_callback,
    },
    {
        .uri = "timezone",
        .method = "POST",
        .type = HttpHandlerCustom,
        .on_request = api_time_set_timezone_callback,
    },
    {
        .uri = "timezone",
        .method = "GET",
        .type = HttpHandlerCustom,
        .on_request = api_time_get_timezone_callback,
    },
    {
        .uri = "tzlist",
        .method = "GET",
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
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    ApiTimeCtx* context = ctx;

    return http_handle_request(path, context->handlers, conn, msg);
}
