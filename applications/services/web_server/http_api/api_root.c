#include "http_api.h"
#include <version.h>
#include <json_helper.h>
#include <usb_network/usb_network.h>

#define TAG "HttpApi"

#define ACCESS_CFG_FILE    APP_DATA_PATH("access.json")
#define ACCESS_KEY_LEN_MIN 4
#define ACCESS_KEY_LEN_MAX 10

typedef struct {
    HttpHandlersList_t handlers;
    enum {
        ApiAccessDisabled = 0,
        ApiAccessEnabled,
        ApiAccessKeyRequired,
    } access_mode;
    FuriString* access_key;
} ApiRootCtx;

bool http_api_version_callback(
    FuriString* path,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
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

static bool http_api_access_get_callback(ApiRootCtx* context, struct mg_connection* conn) {
    FuriString* json_str = furi_string_alloc();

    char* access_mode_str = "disabled";
    if(context->access_mode == ApiAccessEnabled) {
        access_mode_str = "enabled";
    } else if(context->access_mode == ApiAccessKeyRequired) {
        access_mode_str = "key";
    }
    furi_string_cat_printf(json_str, "\"mode\":\"%s\",", access_mode_str);

    size_t key_len = furi_string_size(context->access_key);
    bool key_valid = (key_len > ACCESS_KEY_LEN_MIN) && (key_len < ACCESS_KEY_LEN_MAX);
    furi_string_cat_printf(json_str, "\"key_valid\":%s", key_valid ? "true" : "false");

    MG_REPLY_OK_BODY(conn, "{%s}\n", furi_string_get_cstr(json_str));
    furi_string_free(json_str);
    return true;
}

static bool http_api_access_set_callback(
    ApiRootCtx* context,
    struct mg_connection* conn,
    struct mg_http_message* msg) {
    bool success = false;
    do {
        if(msg->query.len == 0) break;

        char mode_str[11];
        uint8_t access_mode;
        char access_key[ACCESS_KEY_LEN_MAX + 1];

        int mode_len = mg_http_get_var(&msg->query, "mode", mode_str, sizeof(mode_str));
        int key_len = mg_http_get_var(&msg->query, "key", access_key, ACCESS_KEY_LEN_MAX);

        if(mode_len <= 0) break;

        if(mode_len > 0) {
            if(strcmp(mode_str, "disabled") == 0) {
                access_mode = ApiAccessDisabled;
            } else if(strcmp(mode_str, "enabled") == 0) {
                access_mode = ApiAccessEnabled;
            } else if(strcmp(mode_str, "key") == 0) {
                access_mode = ApiAccessKeyRequired;
                if(key_len <= 0) break;
            } else {
                break;
            }
        }

        if(key_len > 0) {
            if((key_len < ACCESS_KEY_LEN_MIN) || (key_len > ACCESS_KEY_LEN_MAX)) {
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

    return true;
}

static bool http_api_access_callback(
    ApiRootCtx* context,
    struct mg_connection* conn,
    struct mg_http_message* msg) {
    if(mg_match(msg->method, mg_str("GET"), NULL)) {
        return http_api_access_get_callback(context, conn);
    } else if(mg_match(msg->method, mg_str("POST"), NULL)) {
        return http_api_access_set_callback(context, conn, msg);
    }
    return false;
}

static bool http_api_is_access_allowed(
    ApiRootCtx* context,
    struct mg_connection* conn,
    struct mg_http_message* msg) {
    uint8_t* ip = conn->rem.ip;
    UsbNetwork* usb_network = furi_record_open(RECORD_USB_NETWORK);
    bool is_usb_addr = usb_network_is_dhcp_addr(usb_network, ip);
    furi_record_close(RECORD_USB_NETWORK);

    bool is_localhost = (ip[0] == 127) && (ip[1] == 0) && (ip[2] == 0) && (ip[3] == 1);

    if(!is_usb_addr && !is_localhost) {
        if(context->access_mode == ApiAccessEnabled) {
            return true;
        } else if(context->access_mode == ApiAccessKeyRequired) {
            furi_assert(context->access_key);
            struct mg_str request_key_temp;

            struct mg_str* request_key = NULL;
            char key_str[ACCESS_KEY_LEN_MAX];
            if(mg_match(msg->method, mg_str("GET"), NULL) &&
               (mg_http_get_header(msg, "Sec-WebSocket-Key") != NULL)) {
                // Upgrade to WebSocket - get key from URI
                int key_len =
                    mg_http_get_var(&msg->query, "x-api-token", key_str, ACCESS_KEY_LEN_MAX);
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

static bool http_api_is_version_allowed(struct mg_connection* conn, struct mg_http_message* msg) {
    struct mg_str* request_semver = NULL;

    char ver_str[8];
    struct mg_str ver_str_temp;
    if(mg_match(msg->method, mg_str("GET"), NULL) &&
       (mg_http_get_header(msg, "Sec-WebSocket-Key") != NULL)) {
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
            MG_REPLY_BAD_REQUEST(conn);
            return false;
        }

        if(!mg_str_to_num(major_ver_str, 10, &major_ver, sizeof(major_ver))) {
            MG_REPLY_BAD_REQUEST(conn);
            return false;
        }

        if(major_ver != api_ver[0]) {
            MG_REPLY_INVALID_VERSION(conn);
            return false;
        }
    }
    return true;
}

static const HttpHandler handlers_api_root[] = {
    {
        .uri = "version",
        .method = "GET",
        .type = HttpHandlerCustom,
        .on_request = http_api_version_callback,
    },
    {
        .uri = "assets",
        .method = "*",
        .type = HttpHandlerCustom,
        .ctx_alloc = http_api_assets_alloc,
        .ctx_free = http_api_assets_free,
        .on_request = http_api_assets_callback,
        .on_headers = http_api_assets_hdr_callback,
    },
    {
        .uri = "storage",
        .method = "*",
        .type = HttpHandlerCustom,
        .ctx_alloc = http_api_storage_alloc,
        .ctx_free = http_api_storage_free,
        .on_request = http_api_storage_callback,
        .on_headers = http_api_storage_hdr_callback,
    },
    {
        .uri = "display",
        .method = "*",
        .type = HttpHandlerCustom,
        .ctx_alloc = http_api_display_alloc,
        .ctx_free = http_api_display_free,
        .on_request = http_api_display_callback,
    },
    {
        .uri = "audio",
        .method = "*",
        .type = HttpHandlerCustom,
        .ctx_alloc = http_api_audio_alloc,
        .ctx_free = http_api_audio_free,
        .on_request = http_api_audio_callback,
    },
    {
        .uri = "input",
        .method = "*",
        .type = HttpHandlerCustom,
        .ctx_alloc = http_api_input_alloc,
        .ctx_free = http_api_input_free,
        .on_request = http_api_input_callback,
    },
    {
        .uri = "status",
        .method = "GET",
        .type = HttpHandlerCustom,
        .on_request = http_api_status_callback,
    },
    {
        .uri = "wifi",
        .method = "*",
        .type = HttpHandlerCustom,
        .ctx_alloc = http_api_wifi_alloc,
        .ctx_free = http_api_wifi_free,
        .on_request = http_api_wifi_callback,
    },
    {
        .uri = "update",
        .method = "POST",
        .type = HttpHandlerCustom,
        .on_headers = http_api_update_hdr_callback,
    },
    {
        .uri = "screen",
        .method = "*",
        .type = HttpHandlerCustom,
        .on_request = http_api_streaming_single_frame_callback,
    },
    {
        .uri = "screen/ws",
        .method = "*",
        .type = HttpHandlerCustom,
        .ctx_alloc = http_api_streaming_ws_alloc,
        .ctx_free = http_api_streaming_ws_free,
        .on_request = http_api_streaming_ws_callback,
    },
    {
        .uri = "ble",
        .method = "*",
        .type = HttpHandlerCustom,
        .ctx_alloc = http_api_ble_alloc,
        .ctx_free = http_api_ble_free,
        .on_request = http_api_ble_callback,
    },
    {
        .uri = "time",
        .method = "*",
        .type = HttpHandlerCustom,
        .ctx_alloc = http_api_time_alloc,
        .ctx_free = http_api_time_free,
        .on_request = http_api_time_callback,
    },
    {

        .uri = "name",
        .method = "*",
        .type = HttpHandlerCustom,
        .on_request = http_api_name_callback,
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
        status = json_config_read_str(cfg, "access_key", context->access_key, NULL);
        size_t key_len = furi_string_size(context->access_key);
        if((status == JsonConfigStatusMissing) || (key_len < ACCESS_KEY_LEN_MIN) ||
           (key_len > ACCESS_KEY_LEN_MAX)) {
            context->access_mode = ApiAccessDisabled;
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
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    ApiRootCtx* context = ctx;
    if(furi_string_equal(path, "access")) {
        return http_api_access_callback(context, conn, msg);
    }
    return http_handle_request(path, context->handlers, conn, msg);
}

bool http_api_options_callback(
    FuriString* path,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    UNUSED(path);
    UNUSED(msg);
    UNUSED(ctx);
    MG_REPLY_OPTIONS(conn);
    return true;
}

bool http_api_root_hdr_callback(
    FuriString* path,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    ApiRootCtx* context = ctx;
    FURI_LOG_D(TAG, "%.*s %.*s", msg->method.len, msg->method.buf, msg->uri.len, msg->uri.buf);
    if(msg->query.len > 0) {
        FURI_LOG_D(TAG, "Query %.*s", msg->query.len, msg->query.buf);
    }

    if(!http_api_is_access_allowed(context, conn, msg)) {
        MG_REPLY_FORBIDDEN(conn);
        mg_iobuf_del(&conn->recv, 0, msg->head.len);
        conn->pfn = NULL;
        conn->is_draining = 1;
        return true;
    }

    if(!http_api_is_version_allowed(conn, msg)) {
        mg_iobuf_del(&conn->recv, 0, msg->head.len);
        conn->pfn = NULL;
        conn->is_draining = 1;
        return true;
    }

    return http_handle_headers(path, context->handlers, conn, msg);
}
