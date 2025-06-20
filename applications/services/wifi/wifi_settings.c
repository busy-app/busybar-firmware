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

#define IP_VERSION_KEY    "version"
#define IP_MANAGEMENT_KEY "mgmt"
#define IP4_ADDRESS_KEY   "address"
#define IP4_NETMASK_KEY   "netmask"
#define IP4_6_GATEWAY_KEY "gateway"
#define IP6_LOCAL_KEY     "local"
#define IP6_GLOBAL_KEY    "global"

static const char* wifi_settings_security_str[WifiSecurityModeMax] = {
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

static const char* wifi_ip_management_str[WifiIpManagementMax] = {
    [WifiIpManagementStatic] = "static",
    [WifiIpManagementDynamic] = "dhcp",
};

static const int wifi_ip_version[WifiIpTypeMax] = {
    [WifiIpTypeV4] = 4,
    [WifiIpTypeV6] = 6,
};

static void wifi_settings_serialize_credentials(cJSON* json, const WifiCredentials* credentials) {
    cJSON* credentials_json = cJSON_AddObjectToObject(json, CREDENTIALS_KEY);

    cJSON_AddStringToObject(credentials_json, CREDENTIALS_SSID_KEY, credentials->ssid);

    const WifiSecurityMode security_mode = credentials->security_mode;
    furi_assert(security_mode < WifiSecurityModeMax);

    if(security_mode != WifiSecurityModeOpen) {
        cJSON_AddStringToObject(credentials_json, CREDENTIALS_SSID_KEY, credentials->passphrase);
    }

    const char* security_str = wifi_settings_security_str[security_mode];
    cJSON_AddStringToObject(credentials_json, CREDENTIALS_SECURITY_KEY, security_str);
}

static void wifi_settings_print_ip4(const WifiIpv4* ip, char* buf, size_t buf_size) {
    const uint8_t* bytes = ip->bytes;
    snprintf(buf, buf_size, "%hhu.%hhu.%hhu.%hhu", bytes[0], bytes[1], bytes[2], bytes[3]);
}

static void wifi_settings_print_ip6(const WifiIpv6* ip, char* buf, size_t buf_size) {
    char* p_buf = buf;

    const uint8_t* bytes = ip->bytes;
    const size_t num_bytes = sizeof(ip->bytes);

    for(uint32_t i = 0; i < num_bytes; i += 2) {
        p_buf += snprintf(p_buf, buf_size - (p_buf - buf), "%hhx:%hhx", bytes[i + 1], bytes[i]);

        if(i < num_bytes - 2) {
            p_buf += snprintf(p_buf, buf_size - (p_buf - buf), ":");
        }
    }
}

static void wifi_serialize_ip4_settings(cJSON* json, const WifiIpv4Settings* settings) {
    char tmp[16];

    if(settings->address.value) {
        wifi_settings_print_ip4(&settings->address, tmp, sizeof(tmp));
        cJSON_AddStringToObject(json, IP4_ADDRESS_KEY, tmp);
    }

    if(settings->mask.value) {
        wifi_settings_print_ip4(&settings->mask, tmp, sizeof(tmp));
        cJSON_AddStringToObject(json, IP4_NETMASK_KEY, tmp);
    }

    if(settings->gateway.value) {
        wifi_settings_print_ip4(&settings->gateway, tmp, sizeof(tmp));
        cJSON_AddStringToObject(json, IP4_6_GATEWAY_KEY, tmp);
    }
}

static void wifi_serialize_ip6_settings(cJSON* json, const WifiIpv6Settings* settings) {
    char tmp[40];

    if(settings->local.value) {
        wifi_settings_print_ip6(&settings->local, tmp, sizeof(tmp));
        cJSON_AddStringToObject(json, IP6_LOCAL_KEY, tmp);
    }

    if(settings->global.value) {
        wifi_settings_print_ip6(&settings->global, tmp, sizeof(tmp));
        cJSON_AddStringToObject(json, IP6_GLOBAL_KEY, tmp);
    }

    if(settings->gateway.value) {
        wifi_settings_print_ip6(&settings->gateway, tmp, sizeof(tmp));
        cJSON_AddStringToObject(json, IP4_6_GATEWAY_KEY, tmp);
    }
}

static void wifi_settings_serialize_ip_config(cJSON* json, const WifiIpConfig* ip_config) {
    cJSON* ip_config_json = cJSON_AddObjectToObject(json, IP_CONFIG_KEY);

    const WifiIpType ip_type = ip_config->type;
    furi_assert(ip_type < WifiIpTypeMax);

    const int ip_version = wifi_ip_version[ip_type];
    cJSON_AddNumberToObject(ip_config_json, IP_VERSION_KEY, ip_version);

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

// Public API
bool wifi_settings_load(WifiSettings* settings) {
    furi_check(settings);

    bool success = false;

    do {
        // success = true;

    } while(false);

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
