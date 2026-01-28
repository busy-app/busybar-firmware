#include "http_api.h"

#include <sntp/sntp.h>
#include <furi_hal_rtc.h>
#include <datetime.h>
#include <furi.h>

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

    MG_REPLY_OK_BODY(conn, "{\"timestamp\":\"%s\"}\n", timestamp_buf);

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

typedef struct {
    const char* name;
    const char* abbr_formatter;
    const char* abbr_param;
    utz_offset_t offset;
} ZoneInfo;

static int compare_zone_info(const void* p1, const void* p2) {
    const ZoneInfo* z1 = p1;
    const ZoneInfo* z2 = p2;

    int r = utz_offset_cmp(&z1->offset, &z2->offset);
    if(r != 0) {
        return r;
    }
    return strcmp(z1->name, z2->name);
}

static ZoneInfo* compile_zone_list(void) {
    ZoneInfo* zone_infos = calloc(utz_num_zone_names, sizeof(ZoneInfo));
    if(!zone_infos) {
        return NULL;
    }
    DateTimeMs dt = furi_hal_rtc_get_datetime();
    size_t i = 0;
    for(const char* name = utz_zone_names; name && i != utz_num_zone_names;
        name = utz_next_zone_name(name), ++i) {
        utz_zone_t zone;
        bool ok = utz_get_zone_by_name(name, &zone);
        if(!ok) {
            // should never happen
            FURI_LOG_E(TAG, "Cannot get zone %s", name);
            break;
        } else {
            ZoneInfo* info = zone_infos + i;
            info->abbr_param = utz_get_current_offset(&zone, &dt.dt, &info->offset);
            info->name = zone.name;
            info->abbr_formatter = zone.abrev_formatter;
        }
    }
    if(i != utz_num_zone_names) {
        FURI_LOG_E(TAG, "Failed to fetch zones");
        free(zone_infos);
        return NULL;
    } else {
        qsort(zone_infos, utz_num_zone_names, sizeof(ZoneInfo), compare_zone_info);
        return zone_infos;
    }
}

static FuriString* format_zone_info_json(const ZoneInfo* info) {
    FuriString* abbr = furi_string_alloc_printf(info->abbr_formatter, info->abbr_param);

    char offset_buf[DATETIME_OFFSET_STR_LEN + 1];

    datetime_format_offset(&info->offset, offset_buf);

    FuriString* result = furi_string_alloc_printf(
        "{\"name\":\"%s\",\"offset\":\"%s\",\"abbr\":\"%s\"}",
        info->name,
        offset_buf,
        furi_string_get_cstr(abbr));
    furi_string_free(abbr);
    return result;
}

static FuriString* generate_zone_list_json(const ZoneInfo* infos) {
    if(!infos) {
        return NULL;
    }

    FuriString* r = furi_string_alloc_set("[");
    if(!r) {
        return NULL;
    }
    for(size_t i = 0; i != utz_num_zone_names; ++i) {
        FuriString* obj = format_zone_info_json(infos + i);

        if(i != 0) {
            furi_string_cat(r, ",");
        }

        furi_string_cat(r, obj);
        furi_string_free(obj);
    }
    furi_string_cat(r, "]");
    return r;
}

static bool api_time_get_timezone_list_callback(
    FuriString* path,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    UNUSED(ctx);
    UNUSED(msg);

    if(!IS_HTTP_ENDPOINT(path)) return false;

    ZoneInfo* infos = compile_zone_list();

    FuriString* result = generate_zone_list_json(infos);

    if(result) {
        MG_REPLY_OK_BODY(conn, "%s", furi_string_get_cstr(result));
        furi_string_free(result);
    } else {
        MG_REPLY_BAD_REQUEST(conn);
    }

    free(infos);

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
