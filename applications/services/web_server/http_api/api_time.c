#include "http_api.h"

#include <sntp/sntp.h>
#include <furi_hal_rtc.h>

#define TAG "HttpTime"

#define API_TIME_H_TO_S(x) ((x) * 60 * 60)

static bool api_time_get_timestamp_callback(
    FuriString* path,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    UNUSED(msg);
    UNUSED(ctx);

    if(!IS_HTTP_ENDPOINT(path)) return false;

    DateTime datetime;
    furi_hal_rtc_get_datetime(&datetime);

    Sntp* sntp = furi_record_open(RECORD_SNTP);
    SntpSettings settings;
    sntp_get_settings(sntp, &settings);
    int timezone_offset = settings.timezone_offset;
    furi_record_close(RECORD_SNTP);

    /* ISO 8601 timestamp: YYYY-MM-DDTHH:MM:SS+HH */
    FuriString* json_str = furi_string_alloc_printf(
        "\"timestamp\":\"%04u-%02u-%02uT%02u:%02u:%02u%+03d\"",
        datetime.year,
        datetime.month,
        datetime.day,
        datetime.hour,
        datetime.minute,
        datetime.second,
        timezone_offset);

    MG_REPLY_OK_BODY(conn, "{%s}\n", furi_string_get_cstr(json_str));
    furi_string_free(json_str);

    return true;
}

static bool api_time_set_timestamp_callback(
    FuriString* path,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    UNUSED(ctx);

    if(!IS_HTTP_ENDPOINT(path)) return false;

    bool success = false;
    do {
        if(msg->query.len == 0) {
            break;
        }

        char timestamp_str[24];
        int len = mg_http_get_var(&msg->query, "timestamp", timestamp_str, sizeof(timestamp_str));
        if(len <= 0) {
            break;
        }

        bool is_utc = (timestamp_str[len - 1] == 'Z');

        /* Parse ISO 8601: YYYY-MM-DDTHH:MM:SS or YYYY-MM-DDTHH:MM:SSZ */
        unsigned int year, month, day, hour, minute, second;
        int parsed_count = sscanf(
            timestamp_str, "%u-%u-%uT%u:%u:%u", &year, &month, &day, &hour, &minute, &second);
        if(parsed_count != 6) {
            break;
        }

        DateTime datetime = {
            .year = year,
            .month = month,
            .day = day,
            .hour = hour,
            .minute = minute,
            .second = second,
        };

        uint32_t timestamp = datetime_datetime_to_timestamp(&datetime);

        if(is_utc) {
            Sntp* sntp = furi_record_open(RECORD_SNTP);
            SntpSettings settings;
            sntp_get_settings(sntp, &settings);
            int timezone_offset = settings.timezone_offset;
            furi_record_close(RECORD_SNTP);

            timestamp += API_TIME_H_TO_S(timezone_offset);
        }

        datetime_timestamp_to_datetime(timestamp, &datetime);

        if(datetime_validate_datetime(&datetime)) {
            furi_hal_rtc_set_datetime(&datetime);
            success = true;
        }
    } while(false);

    if(success) {
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

    bool success = false;
    do {
        if(msg->query.len == 0) {
            break;
        }

        char timezone_str[5];
        if(mg_http_get_var(&msg->query, "timezone", timezone_str, sizeof(timezone_str)) <= 0) {
            break;
        }

        int timezone_offset;
        if(sscanf(timezone_str, "%d", &timezone_offset) != 1) {
            break;
        }

        if(timezone_offset < SNTP_TIMEZONE_OFFSET_MIN ||
           timezone_offset > SNTP_TIMEZONE_OFFSET_MAX) {
            break;
        }

        Sntp* sntp = furi_record_open(RECORD_SNTP);
        SntpSettings settings;
        sntp_get_settings(sntp, &settings);
        settings.timezone_offset = timezone_offset;
        success = sntp_set_settings(sntp, &settings);
        furi_record_close(RECORD_SNTP);
    } while(false);

    if(success) {
        MG_REPLY_OK(conn);
    } else {
        MG_REPLY_BAD_REQUEST(conn);
    }

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
