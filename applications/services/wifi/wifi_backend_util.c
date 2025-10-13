#include "wifi_backend_util.h"

#include <furi.h>

// Do not support 4 Enterprize security modes: SL_WIFI_WPA_ENTERPRISE, SL_WIFI_WPA2_ENTERPRISE,
// SL_WIFI_WPA3_ENTERPRISE and SL_WIFI_WPA3_TRANSITION_ENTERPRISE
#define WIFI_SECURITY_MODE_NOT_SUPPORTED_NUM (4)
static_assert(
    ((int)WifiSecurityModeWpa3Transition + WIFI_SECURITY_MODE_NOT_SUPPORTED_NUM) ==
        (int)SL_WIFI_WPA3_TRANSITION_ENTERPRISE,
    "Security mode enum mismatch");

WifiStatus wifi_decode_sl_status(sl_status_t sl_status) {
    WifiStatus status;

    if(sl_status == SL_STATUS_OK) {
        status = WifiStatusOk;
    } else if(sl_status == SL_STATUS_NOT_INITIALIZED) {
        status = WifiStatusNotInitialized;
    } else if(sl_status == SL_STATUS_ALREADY_INITIALIZED) {
        status = WifiStatusAlreadyInitialized;
    } else if(sl_status == SL_STATUS_INITIALIZATION) {
        status = WifiStatusFailedToInitialize;
    } else if(sl_status == SL_STATUS_SI91X_SCAN_ISSUED_IN_ASSOCIATED_STATE) {
        status = WifiStatusAlreadyConnected;
    } else if(sl_status == SL_STATUS_SI91X_NO_AP_FOUND) {
        status = WifiStatusAccessPointNotFound;
    } else if(sl_status == SL_STATUS_SI91X_COMMAND_GIVEN_IN_INVALID_STATE) {
        status = WifiStatusNotValidForThisCommand;
    } else {
        // TODO: More error cases
        status = WifiStatusError;
    }

    return status;
}

sl_ip_management_t wifi_encode_ip_management(WifiIpManagement mgmt) {
    sl_ip_management_t ret;

    if(mgmt == WifiIpManagementStatic) {
        ret = SL_IP_MANAGEMENT_STATIC_IP;
    } else if(mgmt == WifiIpManagementDynamic) {
        ret = SL_IP_MANAGEMENT_DHCP;
    } else {
        furi_crash("Invalid WifiIpAddressMgmt value");
    }

    return ret;
}

sl_wifi_security_t wifi_encode_security_mode(WifiSecurityMode security_mode) {
    sl_wifi_security_t ret;

    static const sl_wifi_security_t lut[WifiSecurityModeMax] = {
        [WifiSecurityModeOpen] = SL_WIFI_OPEN,
        [WifiSecurityModeWpa] = SL_WIFI_WPA,
        [WifiSecurityModeWpa2] = SL_WIFI_WPA2,
        [WifiSecurityModeWep] = SL_WIFI_WEP,
        [WifiSecurityModeWpaWpa2Mixed] = SL_WIFI_WPA_WPA2_MIXED,
        [WifiSecurityModeWpa3] = SL_WIFI_WPA3,
        [WifiSecurityModeWpa3Transition] = SL_WIFI_WPA3_TRANSITION,
    };

    if(security_mode < WifiSecurityModeMax) {
        ret = lut[security_mode];
    } else {
        furi_crash("Invalid WifiSecurityMode value");
    }

    return ret;
}

sl_ip_address_type_t wifi_encode_ip_version(WifiIpType type) {
    sl_ip_address_type_t ret;

    if(type == WifiIpTypeV4) {
        ret = SL_IPV4;
    } else if(type == WifiIpTypeV6) {
        ret = SL_IPV6;
    } else {
        furi_crash("Invalid WifiIpAddressType value");
    }

    return ret;
}

void wifi_encode_ssid(sl_wifi_ssid_t* sl_ssid, const char* ssid) {
    char* sl_ssid_str = (char*)sl_ssid->value;
    const size_t sl_ssid_capacity = sizeof(sl_ssid->value);

    strncpy(sl_ssid_str, ssid, sl_ssid_capacity);
    sl_ssid->length = strlen(sl_ssid_str);
}

WifiSecurityMode wifi_decode_security_mode(sl_wifi_security_t sl_security) {
    WifiSecurityMode ret;

    static const WifiSecurityMode lut[] = {
        [SL_WIFI_OPEN] = WifiSecurityModeOpen,
        [SL_WIFI_WPA] = WifiSecurityModeWpa,
        [SL_WIFI_WPA2] = WifiSecurityModeWpa2,
        [SL_WIFI_WEP] = WifiSecurityModeWep,
        [SL_WIFI_WPA_ENTERPRISE] = WifiSecurityModeUnsupported,
        [SL_WIFI_WPA2_ENTERPRISE] = WifiSecurityModeUnsupported,
        [SL_WIFI_WPA_WPA2_MIXED] = WifiSecurityModeWpaWpa2Mixed,
        [SL_WIFI_WPA3] = WifiSecurityModeWpa3,
        [SL_WIFI_WPA3_TRANSITION] = WifiSecurityModeWpa3Transition,
        [SL_WIFI_WPA3_ENTERPRISE] = WifiSecurityModeUnsupported,
        [SL_WIFI_WPA3_TRANSITION_ENTERPRISE] = WifiSecurityModeUnsupported,
    };

    if(sl_security <= SL_WIFI_WPA3_TRANSITION_ENTERPRISE) {
        ret = lut[sl_security];
        if(ret == WifiSecurityModeMax) {
            furi_crash("Enterprise sl_wifi_security_t not supported");
        }
    } else {
        furi_crash("Invalid sl_wifi_security_t value");
    }

    return ret;
}

void wifi_decode_ssid(char* ssid, const sl_wifi_ssid_t* sl_ssid) {
    strncpy(ssid, (char*)sl_ssid->value, sl_ssid->length);
    ssid[sl_ssid->length] = '\0';
}

static WifiIpManagement wifi_decode_ip_management(sl_ip_management_t sl_mgmt) {
    WifiIpManagement ret;

    if(sl_mgmt == SL_IP_MANAGEMENT_STATIC_IP) {
        ret = WifiIpManagementStatic;
    } else if(sl_mgmt == SL_IP_MANAGEMENT_DHCP) {
        ret = WifiIpManagementDynamic;
    } else {
        furi_crash("Invalid sl_ip_management_t value");
    }

    return ret;
}

static WifiIpType wifi_decode_ip_version(sl_ip_address_type_t sl_type) {
    WifiIpType ret;

    if(sl_type == SL_IPV4) {
        ret = WifiIpTypeV4;
    } else if(sl_type == SL_IPV6) {
        ret = WifiIpTypeV6;
    } else {
        furi_crash("Invalid sl_ip_address_type_t value");
    }

    return ret;
}

void wifi_decode_ip_config(WifiIpConfig* config, const sl_net_ip_configuration_t* sl_config) {
    config->mgmt = wifi_decode_ip_management(sl_config->mode);
    config->type = wifi_decode_ip_version(sl_config->type);

    const uint8_t* ip_ptr;
    uint8_t* dest;
    size_t ip_length;

    if(config->type == WifiIpTypeV4) {
        ip_ptr = sl_config->ip.v4.ip_address.bytes;
        ip_length = sizeof(sl_config->ip.v4.ip_address.bytes);
        dest = config->ip4.address.bytes;
    } else if(config->type == WifiIpTypeV6) {
        ip_ptr = sl_config->ip.v6.global_address.bytes;
        ip_length = sizeof(sl_config->ip.v6.global_address.bytes);
        dest = config->ip6.global.bytes;
    } else {
        furi_crash("Invalid WifiIpType value");
    }

    memcpy(dest, ip_ptr, ip_length);
}
