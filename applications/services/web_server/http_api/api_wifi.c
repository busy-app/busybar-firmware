#include "http_api.h"
#include <wifi/wifi.h>
#include <cjson/cJSON.h>

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
#define WIFI_JSON_KEY_IP_ADDRESS "address"
#define WIFI_JSON_KEY_IP_MASK    "mask"
#define WIFI_JSON_KEY_IP_GATEWAY "gateway"

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
    [WifiStatusAlreadyConnected] = {.code = 400, "Already connected"},
};

static const char* const security_modes[WifiSecurityModeMax] = {
    [WifiSecurityModeOpen] = "Open",
    [WifiSecurityModeWpa] = "WPA",
    [WifiSecurityModeWpa2] = "WPA2",
    [WifiSecurityModeWep] = "WEP",
    [WifiSecurityModeWpaEnterprise] = "WPA (Enterprise)",
    [WifiSecurityModeWpa2Enterprise] = "WPA2 (Enterprise)",
    [WifiSecurityModeWpaWpa2Mixed] = "WPA/WPA2",
    [WifiSecurityModeWpa3] = "WPA3",
    [WifiSecurityModeWpa3Transition] = "WPA2/WPA3",
    [WifiSecurityModeWpa3Enterprise] = "WPA3 (Enterprise)",
    [WifiSecurityModeWpa3TransitionEnterprise] = "WPA2/WPA3 (Enterprise)",
};

static const char* const wifi_ip_method[] = {
    [WifiIpManagementStatic] = "static",
    [WifiIpManagementDynamic] = "dhcp",
};

static const char* const wifi_ip_type[] = {
    [WifiIpTypeV4] = "ipv4",
    [WifiIpTypeV6] = "ipv6",
};

static const char* const wifi_state[] = {
    [WifiStateDeinit] = "disabled",
    [WifiStateDown] = "enabled",
    [WifiStateUp] = "connected",
};

static const ApiWifiResponseData* api_wifi_get_response_data_from_status(WifiStatus status) {
    if(status >= COUNT_OF(wifi_response_data)) {
        FURI_LOG_W(TAG, "Unknown wifi status: %d, return generic error", status);
        status = WifiStatusError;
    }
    return &wifi_response_data[status];
}

static bool api_wifi_parse_value_from_array(
    const FuriString* value_str,
    const char* const* array,
    size_t length,
    int* result) {
    bool success = false;
    for(size_t i = 0; i < length; i++) {
        if(!furi_string_equal_str(value_str, array[i])) continue;
        *result = i;
        success = true;
        break;
    }
    return success;
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

    Wifi* wifi = furi_record_open(RECORD_WIFI);
    WifiScanResult* results = malloc(sizeof(WifiScanResult) * WIFI_SCAN_RESULT_COUNT);
    uint8_t result_count = 0;
    WifiStatus status = wifi_scan(wifi, results, &result_count, WIFI_SCAN_RESULT_COUNT);
    furi_record_close(RECORD_WIFI);

    if(status == WifiStatusOk) {
        cJSON* response = cJSON_CreateObject();

        cJSON_AddNumberToObject(response, WIFI_JSON_KEY_COUNT, result_count);
        cJSON* array = cJSON_AddArrayToObject(response, WIFI_JSON_KEY_NETWORKS);

        for(size_t i = 0; i < result_count; i++) {
            cJSON* item = cJSON_CreateObject();
            cJSON_AddStringToObject(item, WIFI_JSON_KEY_SSID, results[i].ssid);

            WifiSecurityMode mode = results[i].security_mode;
            cJSON_AddStringToObject(item, WIFI_JSON_KEY_SECURITY, security_modes[mode]);
            cJSON_AddNumberToObject(item, WIFI_JSON_KEY_RSSI, results[i].rssi);
            cJSON_AddItemToArray(array, item);
        }

        MG_REPLY_OK_BODY(conn, cJSON_Print(response));
        cJSON_Delete(response);
    } else {
        const ApiWifiResponseData* data = api_wifi_get_response_data_from_status(status);
        MG_REPLY_ERROR(conn, data->code, data->message);
    }

    free(results);
    return true;
}

bool api_wifi_parse_ip_address(
    FuriString* address_str,
    WifiIpType type,
    uint8_t* result_bytes,
    FuriString* error_msg) {
    bool result = false;
    do {
        struct mg_addr addr;
        const char* address_cstr = furi_string_get_cstr(address_str);
        struct mg_str str = mg_str(address_cstr);
        if(!mg_aton(str, &addr)) {
            furi_string_printf(error_msg, "Unable to parse %s as a valid ip", address_cstr);
            break;
        }

        if(type == WifiIpTypeV4 && addr.is_ip6) {
            furi_string_printf(error_msg, "Address %s is not IPv4", address_cstr);
            break;
        }

        const uint8_t size = addr.is_ip6 ? 16 : 4;
        memcpy(result_bytes, addr.ip, size);

        if(type == WifiIpTypeV6) {
            for(size_t i = 0; i < size; i += 4) {
                *((uint32_t*)&result_bytes[i]) = mg_ntohl(*((uint32_t*)&result_bytes[i]));
            }
        }

        result = true;
    } while(false);
    return result;
}

static void api_wifi_print_ip_address(FuriString* str, WifiIpConfig* ip_config) {
    furi_string_reset(str);
    const WifiIpType type = ip_config->type;

    if(type == WifiIpTypeV4) {
        const uint8_t* bytes = ip_config->ip4.address.bytes;
        furi_string_cat_printf(str, "%d.%d.%d.%d", bytes[0], bytes[1], bytes[2], bytes[3]);
    } else {
        uint8_t n = COUNT_OF(ip_config->ip6.global.value);
        for(size_t i = 0; i < n; i++) {
            uint16_t w1 = (ip_config->ip6.global.value[i] >> 16);
            uint16_t w2 = (ip_config->ip6.global.value[i] & 0xFFFF);
            furi_string_cat_printf(str, "%X:%X%c", w1, w2, (i + 1 == n) ? 0 : ':');
        }
    }
}

static bool api_wifi_mg_json_get_str_key(
    struct mg_str body,
    const char* key,
    FuriString* output,
    FuriString* error_msg) {
    bool result = false;
    do {
        furi_string_printf(output, "$.%s", key);
        char* mg_str = mg_json_get_str(body, furi_string_get_cstr(output));

        if(mg_str == NULL) {
            furi_string_printf(error_msg, "Key %s is missing", key);
            furi_string_reset(output);
            break;
        }

        furi_string_set_str(output, mg_str);
        result = true;

    } while(false);
    return result;
}

static bool api_wifi_parse_ipv6_settings(
    struct mg_str ip_config_json,
    WifiIpv6Settings* ip6_settings,
    FuriString* error_msg) {
    bool result = false;
    FuriString* buf = furi_string_alloc();
    do {
        if(!api_wifi_mg_json_get_str_key(ip_config_json, WIFI_JSON_KEY_IP_ADDRESS, buf, error_msg))
            break;
        if(!api_wifi_parse_ip_address(buf, WifiIpTypeV6, ip6_settings->global.bytes, error_msg))
            break;

        if(!api_wifi_mg_json_get_str_key(ip_config_json, WIFI_JSON_KEY_IP_GATEWAY, buf, error_msg))
            break;
        if(!api_wifi_parse_ip_address(buf, WifiIpTypeV6, ip6_settings->gateway.bytes, error_msg))
            break;

        result = true;
    } while(false);

    furi_string_free(buf);
    return result;
}

static bool api_wifi_parse_ipv4_settings(
    struct mg_str ip_config_json,
    WifiIpv4Settings* ip4_settings,
    FuriString* error_msg) {
    bool result = false;
    FuriString* buf = furi_string_alloc();
    do {
        if(!api_wifi_mg_json_get_str_key(ip_config_json, WIFI_JSON_KEY_IP_ADDRESS, buf, error_msg))
            break;
        if(!api_wifi_parse_ip_address(buf, WifiIpTypeV4, ip4_settings->address.bytes, error_msg))
            break;

        if(!api_wifi_mg_json_get_str_key(ip_config_json, WIFI_JSON_KEY_IP_MASK, buf, error_msg))
            break;
        if(!api_wifi_parse_ip_address(buf, WifiIpTypeV4, ip4_settings->mask.bytes, error_msg))
            break;

        if(!api_wifi_mg_json_get_str_key(ip_config_json, WIFI_JSON_KEY_IP_GATEWAY, buf, error_msg))
            break;
        if(!api_wifi_parse_ip_address(buf, WifiIpTypeV4, ip4_settings->gateway.bytes, error_msg))
            break;

        result = true;
    } while(false);
    furi_string_free(buf);

    return result;
}

static bool api_wifi_parse_ip_config(
    struct mg_str ip_config_json,
    WifiIpConfig* ip_config,
    FuriString* error_msg) {
    bool result = false;
    FuriString* buf = furi_string_alloc();
    do {
        if(!api_wifi_mg_json_get_str_key(ip_config_json, WIFI_JSON_KEY_IP_METHOD, buf, error_msg))
            break;
        if(!api_wifi_parse_ip_method(buf, &ip_config->mgmt)) {
            furi_string_printf(error_msg, "%s is not valid ip_method", furi_string_get_cstr(buf));
            break;
        }

        if(!api_wifi_mg_json_get_str_key(ip_config_json, WIFI_JSON_KEY_IP_TYPE, buf, error_msg)) {
            result = (ip_config->mgmt == WifiIpManagementDynamic);
            break;
        }

        if(!aip_wifi_parse_ip_type(buf, &ip_config->type)) {
            furi_string_printf(error_msg, "%s is not valid ip_type", furi_string_get_cstr(buf));
            break;
        }

        if(ip_config->mgmt == WifiIpManagementDynamic) {
            result = true;
            break;
        }

        if(ip_config->type == WifiIpTypeV4)
            result = api_wifi_parse_ipv4_settings(ip_config_json, &ip_config->ip4, error_msg);
        else
            result = api_wifi_parse_ipv6_settings(ip_config_json, &ip_config->ip6, error_msg);
    } while(false);
    furi_string_free(buf);
    return result;
}

static bool api_wifi_connect_parse_config(
    struct mg_str body,
    WifiCredentials* credentials,
    WifiIpConfig* ip_config,
    FuriString* error_msg) {
    bool parse_result = false;
    FuriString* buf = furi_string_alloc();
    do {
        if(!api_wifi_mg_json_get_str_key(body, WIFI_JSON_KEY_SSID, buf, error_msg)) break;
        strncpy(credentials->ssid, furi_string_get_cstr(buf), SSID_MAX_LEN);

        if(!api_wifi_mg_json_get_str_key(body, WIFI_JSON_KEY_PASSWORD, buf, error_msg)) break;
        strncpy(credentials->passphrase, furi_string_get_cstr(buf), PASSPHRASE_MAX_LEN);

        if(!api_wifi_mg_json_get_str_key(body, WIFI_JSON_KEY_SECURITY, buf, error_msg)) break;
        if(!api_wifi_get_security_mode_by_name(buf, &credentials->security_mode)) {
            furi_string_printf(
                error_msg, "%s is not valid security mode", furi_string_get_cstr(buf));
            break;
        }

        struct mg_str ip_config_json = mg_json_get_tok(body, "$." WIFI_JSON_KEY_IP_CONFIG "");
        if(ip_config_json.len == 0) {
            furi_string_printf(error_msg, "%s is missing", WIFI_JSON_KEY_IP_CONFIG);
            break;
        }
        if(!api_wifi_parse_ip_config(ip_config_json, ip_config, error_msg)) break;

        parse_result = true;
    } while(false);
    furi_string_free(buf);

    return parse_result;
}

static bool
    api_wifi_connect_callaback(struct mg_connection* conn, struct mg_http_message* msg, void* ctx) {
    UNUSED(ctx);

    WifiCredentials credentials = {0};
    WifiIpConfig ip_config = {0};

    FuriString* error_msg = furi_string_alloc();
    bool parse_result =
        api_wifi_connect_parse_config(msg->body, &credentials, &ip_config, error_msg);
    int status_code;
    const char* response_msg;

    if(parse_result) {
        Wifi* wifi = furi_record_open(RECORD_WIFI);
        WifiStatus status = wifi_connect(wifi, &credentials, &ip_config);
        furi_record_close(RECORD_WIFI);

        const ApiWifiResponseData* data = api_wifi_get_response_data_from_status(status);
        status_code = data->code;
        response_msg = data->message;
    } else {
        status_code = 400;
        response_msg = furi_string_get_cstr(error_msg);
    }

    if(status_code == 200)
        MG_REPLY_OK(conn);
    else
        MG_REPLY_ERROR(conn, status_code, response_msg);

    furi_string_free(error_msg);
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
    if(data->code == 200)
        MG_REPLY_OK(conn);
    else
        MG_REPLY_ERROR(conn, data->code, data->message);

    return true;
}

static bool
    api_wifi_forget_callaback(struct mg_connection* conn, struct mg_http_message* msg, void* ctx) {
    UNUSED(msg);
    UNUSED(ctx);

    ///TODO: implemet after configs
    MG_REPLY_ERROR(conn, 400, "Not implemented");
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
    if(data->code == 200)
        MG_REPLY_OK(conn);
    else
        MG_REPLY_ERROR(conn, data->code, data->message);

    return true;
}

static bool
    api_wifi_disable_callaback(struct mg_connection* conn, struct mg_http_message* msg, void* ctx) {
    UNUSED(ctx);
    UNUSED(msg);

    Wifi* wifi = furi_record_open(RECORD_WIFI);
    WifiStatus status = wifi_deinit(wifi);
    furi_record_close(RECORD_WIFI);

    const ApiWifiResponseData* data = api_wifi_get_response_data_from_status(status);
    if(data->code == 200)
        MG_REPLY_OK(conn);
    else
        MG_REPLY_ERROR(conn, data->code, data->message);

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
        cJSON* response = cJSON_CreateObject();

        cJSON_AddStringToObject(response, WIFI_JSON_KEY_STATE, wifi_state[info.state]);

        if(info.state == WifiStateUp) {
            cJSON_AddStringToObject(response, WIFI_JSON_KEY_SSID, info.ssid);

            const char* security_mode = security_modes[info.securiy_mode];
            cJSON_AddStringToObject(response, WIFI_JSON_KEY_SECURITY, security_mode);

            if(info.state == WifiStateUp) {
                cJSON* ip_config_json = cJSON_CreateObject();
                cJSON_AddStringToObject(
                    ip_config_json, WIFI_JSON_KEY_IP_METHOD, wifi_ip_method[info.ip_config.mgmt]);
                cJSON_AddStringToObject(
                    ip_config_json, WIFI_JSON_KEY_IP_TYPE, wifi_ip_type[info.ip_config.type]);

                FuriString* ip_str = furi_string_alloc();
                api_wifi_print_ip_address(ip_str, &info.ip_config);
                cJSON_AddStringToObject(
                    ip_config_json, WIFI_JSON_KEY_IP_ADDRESS, furi_string_get_cstr(ip_str));
                furi_string_free(ip_str);

                cJSON_AddItemToObject(response, WIFI_JSON_KEY_IP_CONFIG, ip_config_json);
            }
        }

        MG_REPLY_OK_BODY(conn, cJSON_Print(response));
        cJSON_Delete(response);
    } else {
        const ApiWifiResponseData* data = api_wifi_get_response_data_from_status(status);
        MG_REPLY_ERROR(conn, data->code, data->message);
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
    for(size_t i = COUNT_OF(handlers_wifi); i > 0; i--) {
        http_handler_add(context->handlers, &handlers_wifi[i - 1]);
    }

    return context;
}

void http_api_wifi_free(void* ctx) {
    furi_assert(ctx);
    ApiWifiCtx* context = ctx;
    HttpHandlersList_clear(context->handlers);
    free(context);
}

bool http_api_wifi_callback(struct mg_connection* conn, struct mg_http_message* msg, void* ctx) {
    ApiWifiCtx* context = ctx;
    return http_handle_request(context->handlers, conn, msg);
}
