#include "wifi_backend_util.h"

#include <furi.h>

WifiStatus wifi_decode_sl_status(sl_status_t sl_status) {
    WifiStatus status;

    if(sl_status == SL_STATUS_OK) {
        status = WifiStatusOk;
    } else {
        // TODO: More error cases
        status = WifiStatusError;
    }

    return status;
}

sl_wifi_security_t wifi_encode_security_mode(WifiSecurityMode security_mode) {
    sl_wifi_security_t ret;

    if(security_mode < WifiSecurityModeMax) {
        ret = (sl_wifi_security_t)security_mode;
    } else {
        furi_crash("Invalid WifiSecurityMode value");
    }

    return ret;
}

WifiSecurityMode wifi_decode_security_mode(sl_wifi_security_t sl_security) {
    WifiSecurityMode ret;

    if(sl_security <= SL_WIFI_WPA3_TRANSITION_ENTERPRISE) {
        ret = (WifiSecurityMode)sl_security;
    } else {
        furi_crash("Invalid sl_wifi_security_t value");
    }

    return ret;
}

sl_ip_management_t wifi_encode_ip_management(WifiIpAddressMgmt mgmt) {
    sl_ip_management_t ret;

    if(mgmt == WifiIpAddressMgmtStatic) {
        ret = SL_IP_MANAGEMENT_STATIC_IP;
    } else if(mgmt == WifiIpAddressMgmtDhcp) {
        ret = SL_IP_MANAGEMENT_DHCP;
    } else {
        furi_crash("Invalid WifiIpAddressMgmt value");
    }

    return ret;
}

WifiIpAddressMgmt wifi_decode_ip_management(sl_ip_management_t sl_mgmt) {
    WifiIpAddressMgmt ret;

    if(sl_mgmt == SL_IP_MANAGEMENT_STATIC_IP) {
        ret = WifiIpAddressMgmtStatic;
    } else if(sl_mgmt == SL_IP_MANAGEMENT_DHCP) {
        ret = WifiIpAddressMgmtDhcp;
    } else {
        furi_crash("Invalid sl_ip_management_t value");
    }

    return ret;
}

sl_ip_address_type_t wifi_encode_ip_type(WifiIpAddressType type) {
    sl_ip_address_type_t ret;

    if(type == WifiIpAddressTypeV4) {
        ret = SL_IPV4;
    } else if(type == WifiIpAddressTypeV6) {
        ret = SL_IPV6;
    } else {
        furi_crash("Invalid WifiIpAddressType value");
    }

    return ret;
}

WifiIpAddressType wifi_decode_ip_type(sl_ip_address_type_t sl_type) {
    WifiIpAddressType ret;

    if(sl_type == SL_IPV4) {
        ret = WifiIpAddressTypeV4;
    } else if(sl_type == SL_IPV6) {
        ret = WifiIpAddressTypeV6;
    } else {
        furi_crash("Invalid sl_ip_address_type_t value");
    }

    return ret;
}

void wifi_encode_ssid(sl_wifi_ssid_t* sl_ssid, const char* ssid) {
    char* sl_ssid_str = (char*)sl_ssid->value;
    const size_t sl_ssid_capacity = sizeof(sl_ssid->value);

    strncpy(sl_ssid_str, ssid, sl_ssid_capacity);
    sl_ssid->length = strlen(sl_ssid_str);
}

void wifi_decode_ssid(char* ssid, const sl_wifi_ssid_t* sl_ssid) {
    strncpy(ssid, (char*)sl_ssid->value, sl_ssid->length);
    ssid[sl_ssid->length] = '\0';
}

void wifi_decode_ip_config(WifiIpAddress* dst, const sl_net_ip_configuration_t* src) {
    dst->mgmt = wifi_decode_ip_management(src->mode);
    dst->type = wifi_decode_ip_type(src->type);

    uint8_t* ip_dst;
    const uint8_t* ip_src;
    size_t ip_length;

    if(dst->type == WifiIpAddressTypeV4) {
        ip_dst = dst->v4;
        ip_src = src->ip.v4.ip_address.bytes;
        ip_length = sizeof(src->ip.v4.ip_address.bytes);

    } else {
        ip_dst = dst->v6;
        ip_src = src->ip.v6.global_address.bytes;
        ip_length = sizeof(src->ip.v6.global_address.bytes);
    }

    memcpy(ip_dst, ip_src, ip_length);
}
