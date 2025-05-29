#include "http_api.h"
#include <wifi/wifi.h>
#include <lwip/ip4_addr.h>
#include <lwip/ip6_addr.h>

#define TAG "HTTP WiFi"

#define WIFI_SCAN_RESULT_COUNT 20U

#define WIFI_JSON_KEY_STATE      "State"
#define WIFI_JSON_KEY_SECURITY   "Security"
#define WIFI_JSON_KEY_SSID       "SSID"
#define WIFI_JSON_KEY_RSSI       "RSSI"
#define WIFI_JSON_KEY_PASSWORD   "Password"
#define WIFI_JSON_KEY_COUNT      "count"
#define WIFI_JSON_KEY_NETWORKS   "networks"
#define WIFI_JSON_KEY_IP_CONFIG  "ip_config"
#define WIFI_JSON_KEY_IP_METHOD  "ip_method"
#define WIFI_JSON_KEY_IP_TYPE    "ip_type"
#define WIFI_JSON_KEY_IP_ADDRESS "ip_address"

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

static const char* wifi_ip_method[] = {
    [WifiIpManagementStatic] = "static",
    [WifiIpManagementDynamic] = "dhcp",
};

static const char* wifi_ip_type[] = {
    [WifiIpTypeV4] = "ipv4",
    [WifiIpTypeV6] = "ipv6",
};

static const char* wifi_state[] = {
    [WifiStateDeinit] = "disabled",
    [WifiStateDown] = "enabled",
    [WifiStateUp] = "connected",
};

static const ApiWifiResponseData* api_wifi_get_response_data_from_status(WifiStatus status) {
    furi_assert(status < COUNT_OF(wifi_response_data));
    return &wifi_response_data[status];
}

static bool api_wifi_parse_value_from_array(
    const FuriString* value_str,
    const char** array,
    size_t length,
    int* result) {
    bool state = false;
    for(size_t i = 0; i < length; i++) {
        if(!furi_string_equal_str(value_str, array[i])) continue;
        *result = i;
        state = true;
        break;
    }
    return state;
}

static bool api_wifi_get_security_mode_by_name(const FuriString* name, WifiSecurityMode* mode) {
    int value = WifiSecurityModeMax;
    bool result =
        api_wifi_parse_value_from_array(name, security_modes, WifiSecurityModeMax, &value);
    *mode = (WifiSecurityMode)value;
    return result;
}

static bool api_wifi_parse_ip_method(FuriString* method_str, WifiIpManagement* method) {
    int value = 0;
    bool result = api_wifi_parse_value_from_array(
        method_str, wifi_ip_method, COUNT_OF(wifi_ip_method), &value);
    *method = (WifiIpManagement)value;
    return result;
}

static bool aip_wifi_parse_ip_type(FuriString* ip_type_str, WifiIpType* ip_type) {
    int value = 0;
    bool result =
        api_wifi_parse_value_from_array(ip_type_str, wifi_ip_type, COUNT_OF(wifi_ip_type), &value);
    *ip_type = (WifiIpType)value;
    return result;
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
        FuriString* response =
            furi_string_alloc_printf("{\"" WIFI_JSON_KEY_COUNT "\":%u,\r\n", result_count);
        furi_string_cat_printf(response, "\"" WIFI_JSON_KEY_NETWORKS "\" : [\r\n");
        for(size_t i = 0; i < result_count; i++) {
            furi_string_cat_printf(
                response, "{\r\n\"" WIFI_JSON_KEY_SSID "\": \"%s\",\r\n", results[i].ssid);

            furi_string_cat_printf(
                response,
                "\"" WIFI_JSON_KEY_SECURITY "\": \"%s\",\r\n",
                security_modes[results[i].security_mode]);
            furi_string_cat_printf(
                response, "\"" WIFI_JSON_KEY_RSSI "\": %d\r\n", results[i].rssi);
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

static inline bool parse_octet(char* octet, WifiIpType type, int* value) {
    uint8_t base = (type == WifiIpTypeV4) ? 10 : 16;
    char* endptr;
    *value = strtoul(octet, &endptr, base);
    return (*endptr == '\0');
}

static inline bool validate_octet(int raw_octet, WifiIpType type) {
    UNUSED(raw_octet);
    UNUSED(type);
    uint16_t max_value = type == WifiIpTypeV4 ? UINT8_MAX : UINT16_MAX;
    return (raw_octet >= 0 && raw_octet <= max_value);
}

static bool api_wifi_parse_ip_address(WifiIpConfig* ip_config, char* str) {
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

static bool api_wifi_parse_ip_config(struct mg_str ip_config_json, WifiIpConfig* const ip_config) {
    bool result = false;
    FuriString* buf = furi_string_alloc();
    do {
        if(ip_config_json.len == 0) break;

        furi_string_set_str(buf, mg_json_get_str(ip_config_json, "$." WIFI_JSON_KEY_IP_METHOD ""));
        if(!api_wifi_parse_ip_method(buf, &ip_config->mgmt)) break;

        if(ip_config->mgmt == WifiIpManagementDynamic) {
            result = true;
            break;
        }

        furi_string_set_str(buf, mg_json_get_str(ip_config_json, "$." WIFI_JSON_KEY_IP_TYPE ""));
        if(!aip_wifi_parse_ip_type(buf, &ip_config->type)) break;

        char* str = mg_json_get_str(ip_config_json, "$." WIFI_JSON_KEY_IP_ADDRESS "");
        if(!api_wifi_parse_ip_address(ip_config, str)) break;

        ///TODO: Remove this, or make it debug
        wifi_print_parsed_address(
            buf,
            ip_config->type,
            (uint8_t*)&ip_config->address,
            ip_config->type == WifiIpTypeV4 ? 4 : 16);
        FURI_LOG_D(TAG, "IP: %s", furi_string_get_cstr(buf));
        result = true;
    } while(false);
    furi_string_free(buf);
    return result;
}

static bool api_wifi_connect_parse_config(
    struct mg_str body,
    WifiCredentials* credentials,
    WifiIpConfig* ip_config) {
    bool parse_result = false;
    do {
        strncpy(
            credentials->ssid, mg_json_get_str(body, "$." WIFI_JSON_KEY_SSID ""), SSID_MAX_LEN);
        strncpy(
            credentials->passphrase,
            mg_json_get_str(body, "$." WIFI_JSON_KEY_PASSWORD ""),
            PASSPHRASE_MAX_LEN);

        FuriString* buf;
        buf = furi_string_alloc_set_str(mg_json_get_str(body, "$." WIFI_JSON_KEY_SECURITY ""));
        if(!api_wifi_get_security_mode_by_name(buf, &credentials->security_mode)) break;

        struct mg_str ip_config_json = mg_json_get_tok(body, "$." WIFI_JSON_KEY_IP_CONFIG "");
        if(!api_wifi_parse_ip_config(ip_config_json, ip_config)) break;

        furi_string_free(buf);
        parse_result = true;
    } while(false);
    return parse_result;
}

static bool
    api_wifi_connect_callaback(struct mg_connection* conn, struct mg_http_message* msg, void* ctx) {
    UNUSED(ctx);

    WifiCredentials credentials = {0};
    WifiIpConfig ip_config = {0};

    bool parse_result = api_wifi_connect_parse_config(msg->body, &credentials, &ip_config);
    int status_code;
    const char* response_msg;

    if(parse_result) {
        Wifi* wifi = furi_record_open(RECORD_WIFI);
        WifiStatus status = wifi_connect(wifi, &credentials, &ip_config);
        FURI_LOG_D(TAG, "Connect status: %X", status);

        furi_record_close(RECORD_WIFI);
        const ApiWifiResponseData* data = api_wifi_get_response_data_from_status(status);
        status_code = data->code;
        response_msg = data->message;
    } else {
        status_code = 400;
        response_msg = "Parsing failed";
    }
    mg_http_reply(conn, status_code, "", response_msg);
    return true;
}

static bool api_wifi_disconnect_callaback(
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    UNUSED(msg);
    UNUSED(ctx);

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

    ///TODO: implemet after configs
    mg_http_reply(conn, 400, "", "Not implemented");
    return true;
}

static bool
    api_wifi_enable_callaback(struct mg_connection* conn, struct mg_http_message* msg, void* ctx) {
    UNUSED(ctx);
    UNUSED(msg);

    Wifi* wifi = furi_record_open(RECORD_WIFI);
    WifiStatus status = wifi_init(wifi);
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

    WifiInfo info = {0};
    Wifi* wifi = furi_record_open(RECORD_WIFI);
    WifiStatus status = wifi_get_info(wifi, &info);
    furi_record_close(RECORD_WIFI);

    if(status == WifiStatusOk) {
        FuriString* response = furi_string_alloc_printf(
            "{\"" WIFI_JSON_KEY_STATE "\":\"%s\",\r\n", wifi_state[info.state]);

        if(info.state != WifiStateDeinit) {
            furi_string_cat_printf(response, "\"" WIFI_JSON_KEY_SSID "\": \"%s\",\r\n", info.ssid);
            furi_string_cat_printf(
                response,
                "\"" WIFI_JSON_KEY_SECURITY "\": \"%s\",\r\n",
                security_modes[info.securiy_mode]);

            if(info.state == WifiStateUp) {
                furi_string_cat_printf(response, "\"" WIFI_JSON_KEY_IP_CONFIG "\":{\r\n");
                furi_string_cat_printf(
                    response,
                    "\"" WIFI_JSON_KEY_IP_METHOD "\":\"%s\",\r\n",
                    wifi_ip_method[info.ip_config.mgmt]);
                furi_string_cat_printf(
                    response,
                    "\"" WIFI_JSON_KEY_IP_TYPE "\":\"%s\",\r\n",
                    wifi_ip_type[info.ip_config.type]);

                FuriString* b2 = furi_string_alloc();

                wifi_print_parsed_address(
                    b2,
                    info.ip_config.type,
                    (uint8_t*)&info.ip_config.address,
                    info.ip_config.type == WifiIpTypeV4 ? 4 : 16);

                furi_string_cat_printf(
                    response,
                    "\"" WIFI_JSON_KEY_IP_ADDRESS "\":\"%s\"\r\n}\r\n",
                    furi_string_get_cstr(b2));
                furi_string_free(b2);
            }
        }
        furi_string_cat_printf(response, "}");

        mg_http_reply(
            conn, 200, "Content-Type: application/json\r\n", furi_string_get_cstr(response));

        furi_string_free(response);
    } else {
        const ApiWifiResponseData* data = api_wifi_get_response_data_from_status(status);
        mg_http_reply(conn, data->code, "", data->message);
    }
    return true;
}

static const HttpHandler handlers_wifi[] = {
    {
        .uri = "/api/v0/wifi/networks",
        .method = "GET",
        .type = HttpHandlerCustom,
        .on_request = api_wifi_get_networks_callaback,
    },
    {
        .uri = "/api/v0/wifi/connect",
        .method = "POST",
        .type = HttpHandlerCustom,
        .on_request = api_wifi_connect_callaback,
    },
    {
        .uri = "/api/v0/wifi/disconnect",
        .method = "POST",
        .type = HttpHandlerCustom,
        .on_request = api_wifi_disconnect_callaback,
    },
    {
        .uri = "/api/v0/wifi/forget",
        .method = "POST",
        .type = HttpHandlerCustom,
        .on_request = api_wifi_forget_callaback,
    },
    {
        .uri = "/api/v0/wifi/enable",
        .method = "POST",
        .type = HttpHandlerCustom,
        .on_request = api_wifi_enable_callaback,
    },
    {
        .uri = "/api/v0/wifi/disable",
        .method = "POST",
        .type = HttpHandlerCustom,
        .on_request = api_wifi_disable_callaback,
    },
    {
        .uri = "/api/v0/wifi/status",
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
