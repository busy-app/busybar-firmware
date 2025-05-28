#include "http_api.h"
#include <wifi/wifi.h>
#include <lwip/ip4_addr.h>
#include <lwip/ip6_addr.h>

#define TAG "HTTP WiFi"

#define WIFI_SCAN_RESULT_COUNT 20U

typedef struct {
    HttpHandlersList_t handlers;
} ApiWifiCtx;

static const char* security_modes[WifiSecurityModeMax] = {
    [WifiSecurityModeOpen] = "Open",
    [WifiSecurityModeWpa] = "WPA",
    [WifiSecurityModeWpa2] = "WPA2",
    [WifiSecurityModeWep] "WEP",
    [WifiSecurityModeWpaEnterprise] = "WPA (Enterprise)",
    [WifiSecurityModeWpa2Enterprise] = "WPA2 (Enterprise)",
    [WifiSecurityModeWpaWpa2Mixed] = "WPA/WPA2",
    [WifiSecurityModeWpa3] = "WPA3",
    [WifiSecurityModeWpa3Transition] = "WPA2/WPA3",
    [WifiSecurityModeWpa3Enterprise] = "WPA3 (Enterprise)",
    [WifiSecurityModeWpa3TransitionEnterprise] = "WPA2/WPA3 (Enterprise)",
};

static WifiSecurityMode api_wifi_get_security_mode_by_name(const FuriString* name) {
    WifiSecurityMode mode = WifiSecurityModeMax;
    for(size_t i = 0; i < WifiSecurityModeMax; i++) {
        if(!furi_string_equal_str(name, security_modes[i])) continue;
        mode = (WifiSecurityMode)i;
        break;
    }
    return mode;
}

static bool api_wifi_get_networks_callaback(
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    UNUSED(ctx);
    UNUSED(msg);

    FURI_LOG_D(TAG, "networks");

    Wifi* wifi = furi_record_open(RECORD_WIFI);

    WifiScanResult* results = malloc(sizeof(WifiScanResult) * WIFI_SCAN_RESULT_COUNT);
    uint8_t result_count = 0;
    WifiStatus status = wifi_scan(wifi, results, &result_count, WIFI_SCAN_RESULT_COUNT);

    if(status == WifiStatusOk) {
        FuriString* response = furi_string_alloc_printf("{\"count\":%u,\r\n", result_count);
        furi_string_cat_printf(response, "\"networks\" : [\r\n");
        for(size_t i = 0; i < result_count; i++) {
            furi_string_cat_printf(response, "{\r\n\"SSID\": \"%s\",\r\n", results[i].ssid);

            furi_string_cat_printf(
                response, "\"Security\": \"%s\",\r\n", security_modes[results[i].security_mode]);
            furi_string_cat_printf(response, "\"RSSI\": %d\r\n", results[i].rssi);
            furi_string_cat_printf(response, i + 1 == result_count ? "}\r\n" : "},\r\n");
        }
        furi_string_cat_printf(response, "]\r\n}\r\n");
        mg_http_reply(
            conn, 200, "Content-Type: application/json\r\n", furi_string_get_cstr(response));
        furi_string_free(response);
    } else {
        mg_http_reply(
            conn,
            500,
            "",
            status == WifiStatusNotInitialized ? "WiFi not enabled" : "Generic error");
    }

    free(results);
    furi_record_close(RECORD_WIFI);

    return true;
}

static bool
    api_wifi_connect_callaback(struct mg_connection* conn, struct mg_http_message* msg, void* ctx) {
    UNUSED(msg);
    UNUSED(ctx);

    FURI_LOG_D(TAG, "connect");

    WifiCredentials credentials;
    WifiIpConfig ip_config;

    do {
        strncpy(credentials.ssid, mg_json_get_str(msg->body, "$.SSID"), SSID_MAX_LEN);
        strncpy(
            credentials.passphrase, mg_json_get_str(msg->body, "$.Password"), PASSPHRASE_MAX_LEN);
        long raw_security_mode = mg_json_get_long(msg->body, "$.Security", WifiSecurityModeMax);
        FURI_LOG_D(TAG, "Raw security: %ld", raw_security_mode);
        if(raw_security_mode >= WifiSecurityModeMax) break;
        credentials.security_mode = (WifiSecurityMode)raw_security_mode;
    } while(false);

    ///TODO: check parsing is ok

    Wifi* wifi = furi_record_open(RECORD_WIFI);
    WifiStatus status = wifi_connect(wifi, &credentials, &ip_config);
    UNUSED(status);
    furi_record_close(RECORD_WIFI);
    mg_http_reply(conn, 200, "", "OK");
    return true;
}

static bool api_wifi_disconnect_callaback(
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    UNUSED(msg);
    UNUSED(ctx);

    FURI_LOG_D(TAG, "disconnect");
    mg_http_reply(conn, 200, "", "OK");
    return true;
}

static bool
    api_wifi_forget_callaback(struct mg_connection* conn, struct mg_http_message* msg, void* ctx) {
    UNUSED(msg);
    UNUSED(ctx);

    FURI_LOG_D(TAG, "forget");
    mg_http_reply(conn, 200, "", "OK");
    return true;
}

static bool
    api_wifi_enable_callaback(struct mg_connection* conn, struct mg_http_message* msg, void* ctx) {
    UNUSED(ctx);
    UNUSED(msg);

    Wifi* wifi = furi_record_open(RECORD_WIFI);

    WifiStatus status = wifi_init(wifi);

    FURI_LOG_D(TAG, "Enable status: %X", status);
    furi_record_close(RECORD_WIFI);

    int response_code = 403;
    FuriString* response = furi_string_alloc();
    if(status == WifiStatusOk) {
        response_code = 200;
        furi_string_set_str(response, "OK");
    } else if(status == WifiStatusAlreadyInitialized) {
        response_code = 400;
        furi_string_set_str(response, "Already initialized");
    } else if(status == WifiStatusFailedToInitialize) {
        response_code = 503;
        furi_string_set_str(response, "Failed to initialize");
    } else {
        response_code = 500;
        furi_string_printf(response, "Generic error: %X", status);
    }

    mg_http_reply(conn, response_code, "", furi_string_get_cstr(response));
    furi_string_free(response);
    return true;
}

static bool
    api_wifi_disable_callaback(struct mg_connection* conn, struct mg_http_message* msg, void* ctx) {
    UNUSED(ctx);
    UNUSED(msg);

    Wifi* wifi = furi_record_open(RECORD_WIFI);

    WifiStatus status = wifi_deinit(wifi);

    FURI_LOG_D(TAG, "Disable status: %X", status);
    furi_record_close(RECORD_WIFI);

    int response_code = 403;
    FuriString* response = furi_string_alloc();
    if(status == WifiStatusOk) {
        response_code = 200;
        furi_string_set_str(response, "OK");
    } else if(status == WifiStatusNotInitialized) {
        response_code = 400;
        furi_string_set_str(response, "Not initialized");
    } else {
        response_code = 500;
        furi_string_printf(response, "Generic error: %X", status);
    }

    mg_http_reply(conn, response_code, "", furi_string_get_cstr(response));
    furi_string_free(response);
    return true;
}

static bool api_wifi_get_status_callaback(
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    UNUSED(msg);
    UNUSED(ctx);

    mg_http_reply(conn, 200, "", "OK");
    return true;
}

static const HttpHandler handlers_wifi[] = {
    {
        .uri = "/*/networks",
        .method = "GET",
        .type = HttpHandlerCustom,
        .on_request = api_wifi_get_networks_callaback,
    },
    {
        .uri = "/*/connect",
        .method = "POST",
        .type = HttpHandlerCustom,
        .on_request = api_wifi_connect_callaback,
    },
    {
        .uri = "/*/disconnect",
        .method = "POST",
        .type = HttpHandlerCustom,
        .on_request = api_wifi_disconnect_callaback,
    },
    {
        .uri = "/*/forget",
        .method = "POST",
        .type = HttpHandlerCustom,
        .on_request = api_wifi_forget_callaback,
    },
    {
        .uri = "/*/enable",
        .method = "POST",
        .type = HttpHandlerCustom,
        .on_request = api_wifi_enable_callaback,
    },
    {
        .uri = "/*/disable",
        .method = "POST",
        .type = HttpHandlerCustom,
        .on_request = api_wifi_disable_callaback,
    },
    {
        .uri = "/*/status",
        .method = "GET",
        .type = HttpHandlerCustom,
        .on_request = api_wifi_get_status_callaback,
    },
};

void* http_api_wifi_alloc(void) {
    ApiWifiCtx* context = malloc(sizeof(ApiWifiCtx));
    HttpHandlersList_init(context->handlers);
    FURI_LOG_D(TAG, "Ctx alloc");
    for(size_t i = COUNT_OF(handlers_wifi); i > 0; i--) {
        http_handler_add(context->handlers, &handlers_wifi[i - 1]);
    }

    return context;
}

void http_api_wifi_free(void* ctx) {
    furi_assert(ctx);
    ApiWifiCtx* context = ctx;
    FURI_LOG_D(TAG, "Ctx free");
    HttpHandlersList_clear(context->handlers);
    free(context);
}

bool http_api_wifi_callback(struct mg_connection* conn, struct mg_http_message* msg, void* ctx) {
    ApiWifiCtx* context = ctx;
    return http_handle_request(context->handlers, conn, msg);
}

bool http_api_wifi_hdr_callback(struct mg_connection* conn, struct mg_http_message* msg, void* ctx) {
    ApiWifiCtx* context = ctx;
    FURI_LOG_D(TAG, "Header CB");
    return http_handle_headers(context->handlers, conn, msg);
}
