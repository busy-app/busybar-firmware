#pragma once

#include <sl_net_wifi_types.h>

#include "wifi_common.h"

/* Conversion from Furi types to SL types */
sl_wifi_security_t wifi_encode_security_mode(WifiSecurityMode security_mode);

sl_ip_management_t wifi_encode_ip_management(WifiIpAddressMgmt mgmt);

sl_ip_address_type_t wifi_encode_ip_type(WifiIpAddressType type);

void wifi_encode_ssid(sl_wifi_ssid_t* sl_ssid, const char* ssid);

/* Conversion from SL types to Furi types */
WifiStatus wifi_decode_sl_status(sl_status_t sl_status);

WifiSecurityMode wifi_decode_security_mode(sl_wifi_security_t sl_security);

WifiIpAddressMgmt wifi_decode_ip_management(sl_ip_management_t sl_mgmt);

WifiIpAddressType wifi_decode_ip_type(sl_ip_address_type_t sl_type);

void wifi_decode_ssid(char* ssid, const sl_wifi_ssid_t* sl_ssid);

void wifi_decode_ip_config(WifiIpAddress* dst, const sl_net_ip_configuration_t* src);
