#include "http_api.h"
#include <wifi/wifi.h>
#include <lwip/ip4_addr.h>
#include <lwip/ip6_addr.h>

#define TAG "HTTP WiFi"

#define WIFI_SCAN_RESULT_COUNT 20U

typedef struct {
    HttpHandlersList_t handlers;
} ApiWifiCtx;

typedef struct {
    int code;
    const char* message;
} ApiWifiResponseData;

static const ApiWifiResponseData wifi_response_data[] = {
    [WifiStatusOk] = {.code = 200, "OK"},
    [WifiStatusError] = {.code = 500, "Generic error"},
    [WifiStatusNotInitialized] = {.code = 400, "Not initialized"},
    [WifiStatusAlreadyInitialized] = {.code = 400, "Already initialized"},
    [WifiStatusFailedToInitialize] = {.code = 503, "Failed to initialize"},
};

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

static const ApiWifiResponseData* api_wifi_get_response_data_from_status(WifiStatus status) {
    furi_assert(status < COUNT_OF(wifi_response_data));
    return &wifi_response_data[status];
}
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

static bool parse_octet(char* octet, WifiIpType type, int* value) {
    uint8_t base = (type == WifiIpTypeV4) ? 10 : 16;
    char* endptr;
    *value = strtoul(octet, &endptr, base);
    return (*endptr == '\0');
}

static bool validate_octet(int raw_octet, WifiIpType type) {
    UNUSED(raw_octet);
    UNUSED(type);
    uint16_t max_value = type == WifiIpTypeV4 ? UINT8_MAX : UINT16_MAX;
    return (raw_octet >= 0 && raw_octet <= max_value);
}

static bool parse_ip_address(WifiIpConfig* ip_config, char* str) {
    uint8_t i = 0;

    const char* separator;
    uint8_t length;

    if(ip_config->type == WifiIpTypeV4) {
        separator = ".";
        length = sizeof(ip_config->address.v4);
    } else {
        separator = ":";
        length = sizeof(ip_config->address.v6);
    }

    char* octet_str = strtok(str, separator);
    while(octet_str != NULL && i < length) {
        int raw_octet;
        if(!parse_octet(octet_str, ip_config->type, &raw_octet)) {
            FURI_LOG_W(
                TAG,
                "Failed to parse \"%s\" as IPv%d octet",
                octet_str,
                ip_config->type == WifiIpTypeV4 ? 4 : 6);
            break;
        }

        if(!validate_octet(raw_octet, ip_config->type)) {
            FURI_LOG_W(
                TAG,
                "Octet \"%d\" is not valid for ipv%d",
                raw_octet,
                ip_config->type == WifiIpTypeV4 ? 4 : 6);
            break;
        }

        if(ip_config->type == WifiIpTypeV4) {
            ip_config->address.v4[i] = (uint8_t)raw_octet;
            i++;
        } else {
            *((uint16_t*)(&ip_config->address.v6[i])) = raw_octet;
            i += 2;
        }

        octet_str = strtok(NULL, separator);
    }

    return i == length;
}

static void wifi_print_parsed_address(
    FuriString* str,
    WifiIpType type,
    const uint8_t* bytes,
    const uint8_t size) {
    furi_string_reset(str);
    const char separator = (type == WifiIpTypeV4) ? '.' : ':';

    for(size_t i = 0; i < size;) {
        if(type == WifiIpTypeV4) {
            furi_string_cat_printf(str, "%d%c", bytes[i], (i + 1 == size) ? 0 : separator);
            i++;
        } else {
            furi_string_cat_printf(
                str, "%X%c", *((uint16_t*)(&bytes[i])), (i + 2 == size) ? 0 : separator);
            i += 2;
        }
    }
}

static bool
    api_wifi_connect_callaback(struct mg_connection* conn, struct mg_http_message* msg, void* ctx) {
    UNUSED(msg);
    UNUSED(ctx);

    WifiCredentials credentials;
    WifiIpConfig ip_config;
    memset(&credentials, 0, sizeof(WifiCredentials));
    memset(&ip_config, 0, sizeof(WifiIpConfig));

    bool parse_result = false;
    do {
        strncpy(credentials.ssid, mg_json_get_str(msg->body, "$.SSID"), SSID_MAX_LEN);
        strncpy(
            credentials.passphrase, mg_json_get_str(msg->body, "$.Password"), PASSPHRASE_MAX_LEN);

        FuriString* buf;
        buf = furi_string_alloc_set_str(mg_json_get_str(msg->body, "$.Security"));
        WifiSecurityMode mode = api_wifi_get_security_mode_by_name(buf);

        if(mode == WifiSecurityModeMax) break;
        credentials.security_mode = mode;

        struct mg_str ip_config_json = mg_json_get_tok(msg->body, "$.ip_config");
        if(ip_config_json.len == 0) break;

        furi_string_set_str(buf, mg_json_get_str(ip_config_json, "$.ip_method"));
        if(furi_string_equal_str(buf, "dhcp")) {
            ip_config.mgmt = WifiIpManagementDynamic;
        } else if(furi_string_equal_str(buf, "static")) {
            ip_config.mgmt = WifiIpManagementStatic;
        } else
            break;

        if(ip_config.mgmt == WifiIpManagementStatic) {
            furi_string_set_str(buf, mg_json_get_str(ip_config_json, "$.ip_type"));
            if(furi_string_equal_str(buf, "ipv4")) {
                ip_config.type = WifiIpTypeV4;
            } else if(furi_string_equal_str(buf, "ipv6")) {
                ip_config.type = WifiIpTypeV6;
            } else
                break;

            char* str = mg_json_get_str(ip_config_json, "$.ip_address");
            if(!parse_ip_address(&ip_config, str)) break;

            wifi_print_parsed_address(
                buf,
                ip_config.type,
                (uint8_t*)&ip_config.address,
                ip_config.type == WifiIpTypeV4 ? 4 : 16);
            FURI_LOG_D(TAG, "IP: %s", furi_string_get_cstr(buf));
        }
        furi_string_free(buf);
        parse_result = true;
    } while(false);

    ///TODO: check parsing is ok
    UNUSED(parse_result);

    if(parse_result) {
        Wifi* wifi = furi_record_open(RECORD_WIFI);
        WifiStatus status = wifi_connect(wifi, &credentials, &ip_config);
        FURI_LOG_D(TAG, "Connect status: %X", status);

        furi_record_close(RECORD_WIFI);
        const ApiWifiResponseData* data = api_wifi_get_response_data_from_status(status);
        mg_http_reply(conn, data->code, "", data->message);
    } else {
        mg_http_reply(conn, 400, "", "Parsing failed");
    }
    return true;
}

static bool api_wifi_disconnect_callaback(
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    UNUSED(msg);
    UNUSED(ctx);

    FURI_LOG_D(TAG, "disconnect");

    Wifi* wifi = furi_record_open(RECORD_WIFI);
    WifiStatus status = wifi_disconnect(wifi);
    furi_record_close(RECORD_WIFI);

    const ApiWifiResponseData* data = api_wifi_get_response_data_from_status(status);
    mg_http_reply(conn, data->code, "", data->message);

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

    const ApiWifiResponseData* data = api_wifi_get_response_data_from_status(status);
    mg_http_reply(conn, data->code, "", data->message);

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

    const ApiWifiResponseData* data = api_wifi_get_response_data_from_status(status);
    mg_http_reply(conn, data->code, "", data->message);

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
