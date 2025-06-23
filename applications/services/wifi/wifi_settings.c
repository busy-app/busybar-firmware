#include "wifi_settings.h"

#include <cjson/cJSON.h>
#include <storage/storage.h>

#define TAG "WifiSettings"

#define WIFI_SETTINGS_FILE APP_DATA_PATH("settings.json")

#define WIFI_SETTINGS_CURRENT_VERSION (0)

#define VERSION_KEY "version"
#define ENABLED_KEY "enabled"

#define CREDENTIALS_KEY "credentials"

#define CREDENTIALS_SSID_KEY     "ssid"
#define CREDENTIALS_PSK_KEY      "psk"
#define CREDENTIALS_SECURITY_KEY "security"

#define IP_CONFIG_KEY "ip"

#define IP_TYPE_KEY       "type"
#define IP_MANAGEMENT_KEY "mgmt"
#define IP_SETTINGS_KEY   "settings"

#define IP4_SETTINGS_ADDRESS_KEY   "address"
#define IP4_SETTINGS_NETMASK_KEY   "netmask"
#define IP4_6_SETTINGS_GATEWAY_KEY "gateway"
#define IP6_SETTINGS_LOCAL_KEY     "local"
#define IP6_SETTINGS_GLOBAL_KEY    "global"

static const char* const wifi_settings_security_str[WifiSecurityModeMax] = {
    [WifiSecurityModeOpen] = "open",
    [WifiSecurityModeWpa] = "wpa",
    [WifiSecurityModeWpa2] = "wpa2",
    [WifiSecurityModeWep] = "wep",
    [WifiSecurityModeWpaEnterprise] = "wpa_enterprise",
    [WifiSecurityModeWpa2Enterprise] = "wpa2_enterprise",
    [WifiSecurityModeWpaWpa2Mixed] = "wpa_wpa2_mixed",
    [WifiSecurityModeWpa3] = "wpa3",
    [WifiSecurityModeWpa3Transition] = "wpa3_transition",
    [WifiSecurityModeWpa3Enterprise] = "wpa3_enterprise",
    [WifiSecurityModeWpa3TransitionEnterprise] = "wpa3_transition_enterprise",
};

static const char* const wifi_ip_management_str[WifiIpManagementMax] = {
    [WifiIpManagementStatic] = "static",
    [WifiIpManagementDynamic] = "dhcp",
};

static const char* const wifi_ip_type[WifiIpTypeMax] = {
    [WifiIpTypeV4] = "v4",
    [WifiIpTypeV6] = "v6",
};

static uint32_t wifi_settings_find_str_by_id(
    const char* needle,
    const char* const* haystack,
    uint32_t haystack_size) {
    uint32_t idx;

    for(idx = 0; idx < haystack_size; ++idx) {
        if(strcasecmp(needle, haystack[idx]) == 0) {
            break;
        }
    }

    return idx;
}

static void wifi_settings_serialize_credentials(cJSON* json, const WifiCredentials* credentials) {
    cJSON* credentials_json = cJSON_AddObjectToObject(json, CREDENTIALS_KEY);

    cJSON_AddStringToObject(credentials_json, CREDENTIALS_SSID_KEY, credentials->ssid);

    const WifiSecurityMode security_mode = credentials->security_mode;
    furi_assert(security_mode < WifiSecurityModeMax);

    if(security_mode != WifiSecurityModeOpen) {
        cJSON_AddStringToObject(credentials_json, CREDENTIALS_PSK_KEY, credentials->passphrase);
    }

    const char* security_str = wifi_settings_security_str[security_mode];
    cJSON_AddStringToObject(credentials_json, CREDENTIALS_SECURITY_KEY, security_str);
}

// TODO: The below functions should go to a library
static void wifi_settings_print_ipv4(const WifiIpv4* ip, char* buf, size_t buf_size) {
    const uint8_t* bytes = ip->bytes;
    snprintf(buf, buf_size, "%hhu.%hhu.%hhu.%hhu", bytes[0], bytes[1], bytes[2], bytes[3]);
}

static bool wifi_settings_read_ipv4(WifiIpv4* ip, const char* buf) {
    bool success = false;

    do {
        uint8_t* bytes = ip->bytes;
        const uint32_t ip_bytes_count = COUNT_OF(ip->bytes);

        // sscanf() doesn't seem to know how to read directly to %hhu values
        unsigned int tmp[ip_bytes_count];
        const int read_count = sscanf(buf, "%u.%u.%u.%u", &tmp[0], &tmp[1], &tmp[2], &tmp[3]);

        if(read_count != ip_bytes_count) {
            break;
        }

        uint32_t i;
        for(i = 0; i < ip_bytes_count; ++i) {
            if(tmp[i] > UINT8_MAX) {
                break;
            }

            bytes[i] = tmp[i];
        }

        if(i != ip_bytes_count) {
            break;
        }

        success = true;

    } while(false);

    return success;
}

static void wifi_settings_print_ipv6(const WifiIpv6* ip, char* buf, size_t buf_size) {
    UNUSED(ip);
    UNUSED(buf);
    UNUSED(buf_size);
    // TODO: Implement a common library
    furi_crash("Not implemented");
}

static bool wifi_settings_read_ipv6(WifiIpv6* ip, const char* buf) {
    UNUSED(ip);
    UNUSED(buf);
    // TODO: Implement a common library
    furi_crash("Not implemented");
}

static void wifi_serialize_ip4_settings(cJSON* json, const WifiIpv4Settings* settings) {
    cJSON* settings_json = cJSON_AddObjectToObject(json, IP_SETTINGS_KEY);

    char tmp[16];

    wifi_settings_print_ipv4(&settings->address, tmp, sizeof(tmp));
    cJSON_AddStringToObject(settings_json, IP4_SETTINGS_ADDRESS_KEY, tmp);

    wifi_settings_print_ipv4(&settings->mask, tmp, sizeof(tmp));
    cJSON_AddStringToObject(settings_json, IP4_SETTINGS_NETMASK_KEY, tmp);

    wifi_settings_print_ipv4(&settings->gateway, tmp, sizeof(tmp));
    cJSON_AddStringToObject(settings_json, IP4_6_SETTINGS_GATEWAY_KEY, tmp);
}

static void wifi_serialize_ip6_settings(cJSON* json, const WifiIpv6Settings* settings) {
    cJSON* settings_json = cJSON_AddObjectToObject(json, IP_SETTINGS_KEY);

    char tmp[40];

    wifi_settings_print_ipv6(&settings->local, tmp, sizeof(tmp));
    cJSON_AddStringToObject(settings_json, IP6_SETTINGS_LOCAL_KEY, tmp);

    wifi_settings_print_ipv6(&settings->global, tmp, sizeof(tmp));
    cJSON_AddStringToObject(settings_json, IP6_SETTINGS_GLOBAL_KEY, tmp);

    wifi_settings_print_ipv6(&settings->gateway, tmp, sizeof(tmp));
    cJSON_AddStringToObject(settings_json, IP4_6_SETTINGS_GATEWAY_KEY, tmp);
}

static void wifi_settings_serialize_ip_config(cJSON* json, const WifiIpConfig* ip_config) {
    cJSON* ip_config_json = cJSON_AddObjectToObject(json, IP_CONFIG_KEY);

    const WifiIpType ip_type = ip_config->type;
    furi_assert(ip_type < WifiIpTypeMax);

    const char* ip_version = wifi_ip_type[ip_type];
    cJSON_AddStringToObject(ip_config_json, IP_TYPE_KEY, ip_version);

    const WifiIpManagement ip_mgmt = ip_config->mgmt;
    furi_assert(ip_mgmt < WifiIpManagementMax);

    const char* mgmt_str = wifi_ip_management_str[ip_mgmt];
    cJSON_AddStringToObject(ip_config_json, IP_MANAGEMENT_KEY, mgmt_str);

    if(ip_mgmt == WifiIpManagementStatic) {
        if(ip_type == WifiIpTypeV4) {
            wifi_serialize_ip4_settings(ip_config_json, &ip_config->ip4);
        } else {
            wifi_serialize_ip6_settings(ip_config_json, &ip_config->ip6);
        }
    }
}

static bool wifi_settings_parse_credentials(cJSON* json, WifiCredentials* credentials) {
    bool success = false;

    do {
        if(!cJSON_IsObject(json)) {
            break;
        }

        cJSON* item;

        item = cJSON_GetObjectItem(json, CREDENTIALS_SSID_KEY);

        if(!cJSON_IsString(item)) {
            break;
        }

        strncpy(credentials->ssid, item->valuestring, sizeof(credentials->ssid));

        item = cJSON_GetObjectItem(json, CREDENTIALS_SECURITY_KEY);

        if(!cJSON_IsString(item)) {
            break;
        }

        credentials->security_mode = wifi_settings_find_str_by_id(
            item->valuestring, wifi_settings_security_str, WifiSecurityModeMax);

        if(credentials->security_mode >= WifiSecurityModeMax) {
            break;
        }

        if(credentials->security_mode != WifiSecurityModeOpen) {
            item = cJSON_GetObjectItem(json, CREDENTIALS_PSK_KEY);

            if(!cJSON_IsString(item)) {
                break;
            }

            strncpy(credentials->passphrase, item->valuestring, sizeof(credentials->passphrase));
        }

        success = true;

    } while(false);

    return success;
}

static bool wifi_settings_parse_ipv4_settings(cJSON* json, WifiIpv4Settings* settings) {
    bool success = false;

    do {
        if(!cJSON_IsObject(json)) {
            break;
        }

        cJSON* item;

        item = cJSON_GetObjectItem(json, IP4_SETTINGS_ADDRESS_KEY);

        if(!cJSON_IsString(item)) {
            break;
        }

        if(!wifi_settings_read_ipv4(&settings->address, item->valuestring)) {
            break;
        }

        item = cJSON_GetObjectItem(json, IP4_SETTINGS_NETMASK_KEY);

        if(!cJSON_IsString(item)) {
            break;
        }

        if(!wifi_settings_read_ipv4(&settings->mask, item->valuestring)) {
            break;
        }

        item = cJSON_GetObjectItem(json, IP4_6_SETTINGS_GATEWAY_KEY);

        if(!cJSON_IsString(item)) {
            break;
        }

        if(!wifi_settings_read_ipv4(&settings->gateway, item->valuestring)) {
            break;
        }

        success = true;

    } while(false);

    return success;
}

static bool wifi_settings_parse_ipv6_settings(cJSON* json, WifiIpv6Settings* settings) {
    bool success = false;

    do {
        if(!cJSON_IsObject(json)) {
            break;
        }

        cJSON* item;

        item = cJSON_GetObjectItem(json, IP6_SETTINGS_LOCAL_KEY);

        if(!cJSON_IsString(item)) {
            break;
        }

        if(!wifi_settings_read_ipv6(&settings->local, item->valuestring)) {
            break;
        }

        item = cJSON_GetObjectItem(json, IP6_SETTINGS_GLOBAL_KEY);

        if(!cJSON_IsString(item)) {
            break;
        }

        if(!wifi_settings_read_ipv6(&settings->global, item->valuestring)) {
            break;
        }

        item = cJSON_GetObjectItem(json, IP4_6_SETTINGS_GATEWAY_KEY);

        if(!cJSON_IsString(item)) {
            break;
        }

        if(!wifi_settings_read_ipv6(&settings->gateway, item->valuestring)) {
            break;
        }

        success = true;

    } while(false);

    return success;
}

static bool wifi_settings_parse_ip_config(cJSON* json, WifiIpConfig* ip_config) {
    bool success = false;

    do {
        if(!cJSON_IsObject(json)) {
            break;
        }

        cJSON* item;

        item = cJSON_GetObjectItem(json, IP_TYPE_KEY);

        if(!cJSON_IsString(item)) {
            break;
        }

        ip_config->type =
            wifi_settings_find_str_by_id(item->valuestring, wifi_ip_type, WifiIpTypeMax);

        if(ip_config->type >= WifiIpTypeMax) {
            break;
        }

        item = cJSON_GetObjectItem(json, IP_MANAGEMENT_KEY);

        if(!cJSON_IsString(item)) {
            break;
        }

        ip_config->mgmt = wifi_settings_find_str_by_id(
            item->valuestring, wifi_ip_management_str, WifiIpManagementMax);

        if(ip_config->mgmt >= WifiIpManagementMax) {
            break;
        }

        if(ip_config->mgmt == WifiIpManagementStatic) {
            item = cJSON_GetObjectItem(json, IP_SETTINGS_KEY);

            if(ip_config->type == WifiIpTypeV4) {
                if(!wifi_settings_parse_ipv4_settings(item, &ip_config->ip4)) {
                    break;
                }

            } else {
                if(!wifi_settings_parse_ipv6_settings(item, &ip_config->ip6)) {
                    break;
                }
            }
        }

        success = true;
    } while(false);

    return success;
}

static bool wifi_settings_parse(cJSON* json, WifiSettings* settings) {
    bool success = false;

    do {
        if(!cJSON_IsObject(json)) {
            break;
        }

        cJSON* item;

        item = cJSON_GetObjectItem(json, VERSION_KEY);

        if(!cJSON_IsNumber(item)) {
            break;
        }

        if(item->valueint != WIFI_SETTINGS_CURRENT_VERSION) {
            break;
        }

        item = cJSON_GetObjectItem(json, ENABLED_KEY);

        if(!cJSON_IsBool(item)) {
            break;
        }

        settings->enabled = cJSON_IsTrue(item);

        item = cJSON_GetObjectItem(json, CREDENTIALS_KEY);

        if(!wifi_settings_parse_credentials(item, &settings->credentials)) {
            break;
        }

        item = cJSON_GetObjectItem(json, IP_CONFIG_KEY);

        if(!wifi_settings_parse_ip_config(item, &settings->ip_config)) {
            break;
        }

        success = true;

    } while(false);

    return success;
}

// Public API

void wifi_settings_init_defaults(WifiSettings* settings) {
    memset(settings, 0, sizeof(WifiSettings));
}

bool wifi_settings_load(WifiSettings* settings) {
    furi_check(settings);

    bool success = false;

    Storage* storage = furi_record_open(RECORD_STORAGE);
    File* file = storage_file_alloc(storage);

    do {
        if(!storage_file_open(file, WIFI_SETTINGS_FILE, FSAM_READ, FSOM_OPEN_EXISTING)) {
            break;
        }

        const size_t file_size = storage_file_size(file);

        if(file_size == 0) {
            break;
        }

        char* buffer = malloc(file_size + 1);

        if(storage_file_read(file, buffer, file_size) != file_size) {
            break;
        }

        cJSON* root = cJSON_Parse(buffer);

        success = wifi_settings_parse(root, settings);

        cJSON_Delete(root);
        free(buffer);

    } while(false);

    storage_file_free(file);
    furi_record_close(RECORD_STORAGE);

    return success;
}

bool wifi_settings_save(const WifiSettings* settings) {
    furi_check(settings);

    bool success = false;

    Storage* storage = furi_record_open(RECORD_STORAGE);
    File* file = storage_file_alloc(storage);

    do {
        if(!storage_file_open(file, WIFI_SETTINGS_FILE, FSAM_WRITE, FSOM_CREATE_ALWAYS)) {
            break;
        }

        cJSON* root = cJSON_CreateObject();

        cJSON_AddNumberToObject(root, VERSION_KEY, WIFI_SETTINGS_CURRENT_VERSION);
        cJSON_AddBoolToObject(root, ENABLED_KEY, settings->enabled);

        wifi_settings_serialize_credentials(root, &settings->credentials);
        wifi_settings_serialize_ip_config(root, &settings->ip_config);

        char* buffer = cJSON_Print(root);

        const size_t buffer_len = strlen(buffer);
        const size_t bytes_written = storage_file_write(file, buffer, buffer_len);

        success = buffer_len == bytes_written;

        cJSON_Delete(root);
        free(buffer);

        success = true;

    } while(false);

    storage_file_free(file);
    furi_record_close(RECORD_STORAGE);

    return success;
}
