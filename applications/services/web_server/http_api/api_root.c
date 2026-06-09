#include "http_api.h"
#include <version.h>
#include <json_helper.h>
#include <lwip/tcpip.h>
#include <time/time.h>
#include <datetime.h>
#include <sysctl/sysctl.h>

#define TAG "HttpApi"

// Read the 3-digit HTTP status code from the queued response in conn->send.
// "HTTP/1.x NNN ..." — prefix is 9 bytes, then 3 ASCII status digits.
// Returns -1 if no response has been queued yet or if the buffer does not
// start with an HTTP/1.x response line (e.g. after a partial flush).
int http_api_extract_status(const struct mg_connection* conn) {
    if(conn->send.len < 12) return -1;
    // Verify the response-line prefix before reading the status digits so that
    // a partial flush (which shifts conn->send.buf forward) or an unexpected
    // buffer state returns -1 instead of mis-parsed garbage.
    if(memcmp(conn->send.buf, "HTTP/1.", 7) != 0) return -1;
    if(conn->send.buf[8] != ' ') return -1;
    int status = 0;
    for(size_t i = 9; i < 12; i++) {
        char c = (char)conn->send.buf[i];
        if(c < '0' || c > '9') return -1;
        status = status * 10 + (c - '0');
    }
    return status;
}

// Emit an access log line at the configured verbosity level.
// Modelled on nginx combined format:
//   $remote_addr - $remote_user [$time_local] "$request" $status "$http_user_agent"
//
// Level 0 (default): errors only (status >= 400) — IP - - "METHOD URI" STATUS
// Level 1: all requests — IP - - "METHOD URI" STATUS ["request-id: ID"]
// Level 2: level 1 + User-Agent — IP - - "METHOD URI" STATUS "UA" ["request-id: ID"]
// Level 3: level 2 + timestamp — IP - - [TIMESTAMP] "METHOD URI" STATUS "UA" ["request-id: ID"]
void http_api_log_access(struct mg_connection* conn, struct mg_http_message* msg, int status_code) {
    int level = sysctl_get_websrv_accesslog_level();
    bool is_error = status_code >= 400;

    // Level 0: only log errors
    if(level <= 0 && !is_error) return;

    // x-request-id takes priority over x-trace-id
    struct mg_str* req_id = mg_http_get_header(msg, "x-request-id");
    if(!req_id) req_id = mg_http_get_header(msg, "x-trace-id");

    FuriString* line = furi_string_alloc();

    // $remote_addr - $remote_user  (all levels: real IP when available)
    {
        char ip[16] = "-";
        if(!conn->rem.is_ip6) {
            const uint8_t* a = conn->rem.addr.ip;
            snprintf(ip, sizeof(ip), "%u.%u.%u.%u", a[0], a[1], a[2], a[3]);
        }
        furi_string_cat_printf(line, "%s - - ", ip);
    }

    // [$time_local]  (level 3+)
    if(level >= 3) {
        char ts[DATETIME_TIMESTAMP_STR_LEN + 1];
        Time* time_svc = furi_record_open(RECORD_TIME);
        LocalTime lt = time_get_local_time(time_svc);
        furi_record_close(RECORD_TIME);
        datetime_format_timestamp(&lt, ts);
        furi_string_cat_printf(line, "[%s] ", ts);
    }

    // "$request"
    furi_string_cat_printf(
        line,
        "\"%.*s %.*s%s%.*s\"",
        (int)msg->method.len,
        msg->method.len ? msg->method.buf : "-",
        (int)msg->uri.len,
        msg->uri.len ? msg->uri.buf : "-",
        msg->query.len ? "?" : "",
        (int)msg->query.len,
        msg->query.len ? msg->query.buf : "");

    // $status
    if(status_code > 0) {
        furi_string_cat_printf(line, " %d", status_code);
    } else {
        furi_string_cat(line, " -");
    }

    // "$http_user_agent"  (level 2+)
    if(level >= 2) {
        struct mg_str* ua = mg_http_get_header(msg, "User-Agent");
        furi_string_cat_printf(
            line, " \"%.*s\"", ua ? (int)ua->len : 1, ua ? (ua->len ? ua->buf : "") : "-");
    }

    // request-id (custom field, level 1+)
    if(level >= 1 && req_id) {
        furi_string_cat_printf(
            line, " \"request-id: %.*s\"", (int)req_id->len, req_id->len ? req_id->buf : "");
    }

    FURI_LOG_I(TAG, "%s", furi_string_get_cstr(line));
    furi_string_free(line);
}

#define ACCESS_CFG_FILE    APP_DATA_PATH("access.json")
#define ACCESS_KEY_LEN_MIN 4
#define ACCESS_KEY_LEN_MAX 10

// Always accessible API endpoints
static const struct {
    const char* uri;
    HttpMethod method;
} api_access_whitelist[] = {
    {"version", HttpMethodGet},
    {"access", HttpMethodGet},
    {"transport", HttpMethodGet},
};

typedef struct {
    HttpHandlersList_t handlers;
    enum {
        ApiAccessDisabled = 0,
        ApiAccessEnabled,
        ApiAccessKeyRequired,
    } access_mode;
    FuriString* access_key;
} ApiRootCtx;

static bool http_api_version_callback(
    FuriString* path,
    HttpMethod method,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    UNUSED(method);
    UNUSED(msg);
    UNUSED(ctx);

    if(!IS_HTTP_ENDPOINT(path)) return false;

    FuriString* ver_str = furi_string_alloc();
    const uint8_t api_ver[] = API_VERSION;

    furi_string_printf(ver_str, "\"api_semver\":\"%u.%u.%u\"", api_ver[0], api_ver[1], api_ver[2]);

    MG_REPLY_OK_BODY(conn, "{%s}\n", furi_string_get_cstr(ver_str));
    furi_string_free(ver_str);

    return true;
}

static bool is_connection_on_netif(struct mg_connection* conn, NetworkNetif id) {
    if(conn->loc.is_ip6) return false;
    LOCK_TCPIP_CORE();
    struct netif* netif = network_find_netif(id);
    bool match = netif &&
                 (memcmp(conn->loc.addr.ip, netif_ip4_addr(netif), sizeof(ip4_addr_t)) == 0);
    UNLOCK_TCPIP_CORE();
    return match;
}

static bool http_api_transport_callback(
    FuriString* path,
    HttpMethod method,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    UNUSED(method);
    UNUSED(msg);
    UNUSED(ctx);

    if(!IS_HTTP_ENDPOINT(path)) return false;

    bool is_wifi = is_connection_on_netif(conn, NetworkNetifWifi);
    MG_REPLY_OK_BODY(conn, "{\"type\":\"%s\"}\n", is_wifi ? "wifi" : "usb");

    return true;
}

static bool validate_access_key(const char* key) {
    size_t key_len = strlen(key);
    if((key_len < ACCESS_KEY_LEN_MIN) || (key_len > ACCESS_KEY_LEN_MAX)) {
        return false;
    }
    for(size_t i = 0; i < key_len; i++) {
        char c = key[i];
        if((c < '0') || (c > '9')) {
            return false;
        }
    }
    return true;
}

static void http_api_access_get_callback(ApiRootCtx* context, struct mg_connection* conn) {
    FuriString* json_str = furi_string_alloc();

    char* access_mode_str = "disabled";
    if(context->access_mode == ApiAccessEnabled) {
        access_mode_str = "enabled";
    } else if(context->access_mode == ApiAccessKeyRequired) {
        access_mode_str = "key";
    }
    furi_string_cat_printf(json_str, "\"mode\":\"%s\",", access_mode_str);

    bool key_valid = validate_access_key(furi_string_get_cstr(context->access_key));
    furi_string_cat_printf(json_str, "\"key_valid\":%s", key_valid ? "true" : "false");

    MG_REPLY_OK_BODY(conn, "{%s}\n", furi_string_get_cstr(json_str));
    furi_string_free(json_str);
}

static void http_api_access_set_callback(
    ApiRootCtx* context,
    struct mg_connection* conn,
    struct mg_http_message* msg) {
    bool success = false;
    do {
        if(msg->query.len == 0) break;

        char mode_str[11];
        uint8_t access_mode;
        char access_key[ACCESS_KEY_LEN_MAX + 1];

        int mode_status = mg_http_get_var(&msg->query, "mode", mode_str, sizeof(mode_str));
        int key_status = mg_http_get_var(&msg->query, "key", access_key, sizeof(access_key));

        if(mode_status <= 0) break;

        if(strcmp(mode_str, "disabled") == 0) {
            access_mode = ApiAccessDisabled;
        } else if(strcmp(mode_str, "enabled") == 0) {
            access_mode = ApiAccessEnabled;
        } else if(strcmp(mode_str, "key") == 0) {
            access_mode = ApiAccessKeyRequired;
            if(key_status <= 0) break;
        } else {
            break;
        }

        if(key_status > 0) {
            if(validate_access_key(access_key) == false) {
                break;
            }
            furi_string_set(context->access_key, access_key);
            json_config_write_single_str(
                ACCESS_CFG_FILE, "access_key", furi_string_get_cstr(context->access_key));
        }
        context->access_mode = access_mode;
        json_config_write_single_int(ACCESS_CFG_FILE, "access_mode", context->access_mode);
        success = true;
    } while(0);

    if(success) {
        MG_REPLY_OK(conn);
    } else {
        MG_REPLY_BAD_REQUEST(conn);
    }
}

static bool http_api_is_access_allowed(
    ApiRootCtx* context,
    FuriString* path,
    HttpMethod method,
    struct mg_connection* conn,
    struct mg_http_message* msg) {
    // CORS preflight requests cannot carry credentials; always allow
    if(method == HttpMethodOptions) return true;

    for(size_t i = 0; i < COUNT_OF(api_access_whitelist); i++) {
        if(furi_string_equal(path, api_access_whitelist[i].uri) &&
           (method & api_access_whitelist[i].method)) {
            return true;
        }
    }

    bool is_usb = is_connection_on_netif(conn, NetworkNetifUsb);

    uint8_t* ip = conn->rem.addr.ip;
    bool is_localhost = !conn->rem.is_ip6;
    is_localhost &= (ip[0] == 127) && (ip[1] == 0) && (ip[2] == 0) && (ip[3] == 1);

    if(!is_usb && !is_localhost) {
        if(context->access_mode == ApiAccessEnabled) {
            return true;
        } else if(context->access_mode == ApiAccessKeyRequired) {
            furi_assert(context->access_key);
            struct mg_str request_key_temp;

            struct mg_str* request_key = NULL;
            char key_str[ACCESS_KEY_LEN_MAX + 1];
            if(method == HttpMethodWebSocket) {
                // Upgrade to WebSocket - get key from URI
                int key_len =
                    mg_http_get_var(&msg->query, "x-api-token", key_str, ACCESS_KEY_LEN_MAX + 1);
                if(key_len > 0) {
                    request_key_temp = mg_str_n(key_str, key_len);
                    request_key = &request_key_temp;
                }
            } else {
                // Usual request - get key from header
                request_key = mg_http_get_header(msg, "X-API-Token");
            }
            if(request_key != NULL) {
                struct mg_str access_key = mg_str(furi_string_get_cstr(context->access_key));
                if(mg_strcmp(*request_key, access_key) == 0) {
                    return true;
                }
            }
        }
        return false;
    }
    return true;
}

static bool http_api_is_version_allowed(
    HttpMethod method,
    struct mg_connection* conn,
    struct mg_http_message* msg) {
    struct mg_str* request_semver = NULL;

    char ver_str[8];
    struct mg_str ver_str_temp;
    if(method == HttpMethodWebSocket) {
        // Upgrade to WebSocket - get version from URI
        int ver_len = mg_http_get_var(&msg->query, "x-api-sem-ver", ver_str, sizeof(ver_str));
        if(ver_len > 0) {
            ver_str_temp = mg_str_n(ver_str, ver_len);
            request_semver = &ver_str_temp;
        }
    } else {
        // Usual request - get version from header
        request_semver = mg_http_get_header(msg, "X-API-Sem-Ver");
    }
    if(request_semver) {
        if(mg_match(msg->uri, mg_str("*/version"), NULL)) {
            return true;
        }
        uint8_t major_ver;
        const uint8_t api_ver[] = API_VERSION;

        struct mg_str major_ver_str;
        if(!mg_span(*request_semver, &major_ver_str, NULL, '.')) {
            MG_REPLY_ERROR_CLOSE(conn, 400, "Bad Request");
            return false;
        }

        if(!mg_str_to_num(major_ver_str, 10, &major_ver, sizeof(major_ver))) {
            MG_REPLY_ERROR_CLOSE(conn, 400, "Bad Request");
            return false;
        }

        if(major_ver != api_ver[0]) {
            MG_REPLY_ERROR_CLOSE(conn, 405, "Incompatible API version");
            return false;
        }
    }
    return true;
}

static const HttpHandler handlers_api_root[] = {
    {
        .uri = "version",
        .method = HttpMethodGet,
        .type = HttpHandlerCustom,
        .on_request = http_api_version_callback,
    },
    {
        .uri = "transport",
        .method = HttpMethodGet,
        .type = HttpHandlerCustom,
        .on_request = http_api_transport_callback,
    },
    {
        .uri = "assets",
        .method = HttpMethodAny,
        .type = HttpHandlerCustom,
        .ctx_alloc = http_api_assets_alloc,
        .ctx_free = http_api_assets_free,
        .on_request = http_api_assets_callback,
        .on_headers = http_api_assets_hdr_callback,
    },
    {
        .uri = "storage",
        .method = HttpMethodAny,
        .type = HttpHandlerCustom,
        .ctx_alloc = http_api_storage_alloc,
        .ctx_free = http_api_storage_free,
        .on_request = http_api_storage_callback,
        .on_headers = http_api_storage_hdr_callback,
    },
    {
        .uri = "display",
        .method = HttpMethodAny,
        .type = HttpHandlerCustom,
        .ctx_alloc = http_api_display_alloc,
        .ctx_free = http_api_display_free,
        .on_request = http_api_display_callback,
    },
    {
        .uri = "audio",
        .method = HttpMethodAny,
        .type = HttpHandlerCustom,
        .ctx_alloc = http_api_audio_alloc,
        .ctx_free = http_api_audio_free,
        .on_request = http_api_audio_callback,
    },
    {
        .uri = "input",
        .method = HttpMethodPost,
        .type = HttpHandlerCustom,
        .on_request = http_api_input_callback,
    },
    {
        .uri = "status",
        .method = HttpMethodAny,
        .type = HttpHandlerCustom,
        .on_request = http_api_status_callback,
        .ctx_alloc = http_api_status_alloc,
        .ctx_free = http_api_status_free,
    },
    {
        .uri = "status/ws",
        .method = HttpMethodWebSocket,
        .type = HttpHandlerCustom,
        .ctx_alloc = http_api_status_ws_alloc,
        .ctx_free = http_api_status_ws_free,
        .on_request = http_api_status_ws_callback,
    },
    {
        .uri = "wifi",
        .method = HttpMethodAny,
        .type = HttpHandlerCustom,
        .ctx_alloc = http_api_wifi_alloc,
        .ctx_free = http_api_wifi_free,
        .on_request = http_api_wifi_callback,
    },
    {
        .uri = "update",
        .method = HttpMethodAny,
        .type = HttpHandlerCustom,
        .ctx_alloc = http_api_update_alloc,
        .ctx_free = http_api_update_free,
        .on_request = http_api_update_callback,
        .on_headers = http_api_update_hdr_callback_root,
    },
    {
        .uri = "screen",
        .method = HttpMethodGet,
        .type = HttpHandlerCustom,
        .on_request = http_api_streaming_single_frame_callback,
    },
    {
        .uri = "ble",
        .method = HttpMethodAny,
        .type = HttpHandlerCustom,
        .ctx_alloc = http_api_ble_alloc,
        .ctx_free = http_api_ble_free,
        .on_request = http_api_ble_callback,
    },
    {
        .uri = "time",
        .method = HttpMethodAny,
        .type = HttpHandlerCustom,
        .ctx_alloc = http_api_time_alloc,
        .ctx_free = http_api_time_free,
        .on_request = http_api_time_callback,
    },
    {
        .uri = "name",
        .method = HttpMethodGet | HttpMethodPost,
        .type = HttpHandlerCustom,
        .on_request = http_api_name_callback,
    },
    {
        .uri = "account",
        .method = HttpMethodAny,
        .type = HttpHandlerCustom,
        .ctx_alloc = http_api_account_alloc,
        .ctx_free = http_api_account_free,
        .on_request = http_api_account_callback,
    },
    {
        .uri = "busy",
        .method = HttpMethodAny,
        .type = HttpHandlerCustom,
        .ctx_alloc = http_api_busy_alloc,
        .ctx_free = http_api_busy_free,
        .on_request = http_api_busy_callback,
    },
    {
        .uri = "smart_home",
        .method = HttpMethodAny,
        .type = HttpHandlerCustom,
        .ctx_alloc = http_api_smart_home_alloc,
        .ctx_free = http_api_smart_home_free,
        .on_request = http_api_smart_home_callback,
    },
};

void* http_api_root_alloc(void) {
    ApiRootCtx* context = malloc(sizeof(ApiRootCtx));
    HttpHandlersList_init(context->handlers);

    for(size_t i = COUNT_OF(handlers_api_root); i > 0; i--) {
        http_handler_add(context->handlers, &handlers_api_root[i - 1]);
    }

    context->access_key = furi_string_alloc();

    JsonConfig* cfg = json_config_alloc();
    JsonConfigStatus status = json_config_open(cfg, ACCESS_CFG_FILE);
    if(status != JsonConfigStatusError) {
        int access_mode = 0;
        int access_mode_default = ApiAccessDisabled;
        json_config_read_int(cfg, "access_mode", &access_mode, &access_mode_default);
        context->access_mode = access_mode;
        json_config_read_str(cfg, "access_key", context->access_key, NULL);
        if(context->access_mode == ApiAccessKeyRequired) {
            if(validate_access_key(furi_string_get_cstr(context->access_key)) == false) {
                context->access_mode = ApiAccessDisabled;
            }
        }
    } else {
        context->access_mode = ApiAccessDisabled;
    }
    json_config_free(cfg);

    return context;
}

void http_api_root_free(void* ctx) {
    furi_assert(ctx);
    ApiRootCtx* context = ctx;
    HttpHandlersList_clear(context->handlers);
    furi_string_free(context->access_key);
    free(context);
}

bool http_api_root_callback(
    FuriString* path,
    HttpMethod method,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    ApiRootCtx* context = ctx;
    bool handled;
    if(furi_string_equal(path, "access")) {
        if(method == HttpMethodGet) {
            http_api_access_get_callback(context, conn);
        } else if(method == HttpMethodPost) {
            http_api_access_set_callback(context, conn, msg);
        } else if(method == HttpMethodOptions) {
            http_reply_cors_preflight(conn, HttpMethodGet | HttpMethodPost);
        } else {
            http_reply_405_method_not_allowed(conn, HttpMethodPost | HttpMethodGet, false);
        }
        handled = true;
    } else {
        handled = http_handle_request(path, method, context->handlers, conn, msg);
    }
    // Logging is done at the http_event_handler level in web_server.c,
    // which covers both API and static-file routes in one place.
    return handled;
}

bool http_api_root_hdr_callback(
    FuriString* path,
    HttpMethod method,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    ApiRootCtx* context = ctx;

    // OPTIONS preflights fall through to MG_EV_HTTP_MSG for http_reply_cors_preflight()

    if(!http_api_is_access_allowed(context, path, method, conn, msg)) {
        MG_REPLY_ERROR_CLOSE(conn, 403, "Forbidden");
        http_api_log_access(conn, msg, 403);
        MG_CLOSE_AFTER_HEADERS(conn, msg);
        return true;
    }

    if(!http_api_is_version_allowed(method, conn, msg)) {
        // is_version_allowed already sent either 400 or 405
        http_api_log_access(conn, msg, http_api_extract_status(conn));
        MG_CLOSE_AFTER_HEADERS(conn, msg);
        return true;
    }

    // For routes with on_headers (uploads), on_request is skipped in web_server.c
    // when raw.on_data is set, so log here for those cases.
    bool handled = http_handle_headers(path, method, context->handlers, conn, msg);
    if(handled) {
        http_api_log_access(conn, msg, http_api_extract_status(conn));
    }
    return handled;
}
