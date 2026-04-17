
#include "socket.h"
#include "sl_net_constants.h"
#include "sl_ip_types.h"
#include "iot_socket.h"
#include "sl_net_constants.h"
#include "sl_wifi_constants.h"
#include "sl_ip_types.h"
#include "sl_wifi_device.h"
#include "socket.h"
#include "netinet_in.h"
#include "console_types.h"
#include "console_argument_types.h"
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************
 *                 Global Variables
 ******************************************************/

const char *ap_client_deauth_type[]     = { "deauth_client", "deauth_client_unspecified", NULL };
const char *boolean_type[]              = { "True", NULL };
const char *bsd_socket_family_type[]    = { "AF_INET", "AF_INET6", NULL };
const char *bsd_socket_protocol_type[]  = { "IPPROTO_TCP", "IPPROTO_UDP", NULL };
const char *bsd_socket_type_type[]      = { "SOCK_DGRAM", "SOCK_STREAM", NULL };
const char *continuous_flag_type[]      = { "continuous", "one_time", NULL };
const char *data_rate_type[]            = { "11Mbps",   "12Mbps",   "14.4Mbps", "17.2Mbps", "18Mbps",   "1Mbps",
                                            "21.7Mbps", "24Mbps",   "25.8Mbps", "28.9Mbps", "2Mbps",    "34.4Mbps",
                                            "36Mbps",   "43.3Mbps", "48Mbps",   "5.5Mbps",  "51.6Mbps", "54Mbps",
                                            "57.8Mbps", "65Mbps",   "68.8Mbps", "6Mbps",    "7.2Mbps",  "72.2Mbps",
                                            "77.4Mbps", "8.6Mbps",  "86Mbps",   "9Mbps",    "auto",     NULL };
const char *enable_bg_scan_type[]       = { "bg_scan", NULL };
const char *flags_type[]                = { "bind_and_listen", "connect", NULL };
const char *get_option_id_type[]        = { "keepalivetimeout", "recvtimeout", "sendtimeout", "sockettype", NULL };
const char *ip_protocol_type[]          = { "tcp", "udp", NULL };
const char *ipv4_or_ipv6_type[]         = { "ipv4", "ipv6", NULL };
const char *net_interface_type[]        = { "ap", "bluetooth", "client", "ethernet", "thread", "zwave", NULL };
const char *operating_band_type[]       = { "2.4g", "5g", "dual", NULL };
const char *option_name_type[]          = { "SL_SO_CERT_INDEX", "SO_KEEPALIVE", "SO_MAX_RETRANSMISSION_TIMEOUT_VALUE",
                                            "SO_RCVTIMEO",      "TCP_ULP",      NULL };
const char *performance_mode_type[]     = { "high_performance",
                                            "power_save",
                                            "power_save_low_latency",
                                            "deep_sleep_without_ram_retention",
                                            "deep_sleep_with_ram_retention",
                                            NULL };
const char *rate_protocol_type[]        = { "802.11ax", "802.11b", "802.11g", "802.11n", "auto", NULL };
const char *set_option_id_type[]        = { "fionbio", "keepalivetimeout", "recvtimeout", "sendtimeout", NULL };
const char *sl_ip_address_type_t_type[] = { "ipv4", "ipv6", "ipv6_global", "ipv6_link_local", "ipv6_site_local", NULL };
const char *sl_ip_management_t_type[]   = { "dhcp", "static", NULL };
const char *sl_net_dns_resolution_ip_type_t_type[] = { "ipv4", "ipv6", NULL };
const char *socket_domain_type[]                   = { "af_inet", "af_inet6", NULL };
const char *socket_protocol_type[]  = { "ip_icmp", "ip_proto", "ip_raw", "ip_tcp", "ip_udp", "ip_udp_lite", NULL };
const char *socket_type_type[]      = { "dgram", "stream", NULL };
const char *wifi_ap_flag_type[]     = { "hidden_ssid", NULL };
const char *wifi_band_type[]        = { "2.4g", "5g", "60g", "6g", "900m", "auto", NULL };
const char *wifi_bandwidth_type[]   = { "10m", "160m", "20m", "40m", "80m", NULL };
const char *wifi_encryption_type[]  = { "ccmp", "fast", "open", "peap", "tkip", "tls", "ttls", "wep", NULL };
const char *wifi_init_mode_type[]   = { "ap",  "apsta",         "ble_coex", "client", "client_ipv6",
                                        "eap", "transmit_test", "ble",      NULL };
const char *wifi_init_region_type[] = { "default", "us", "eu", "jp", "world", "kr", "sg", "cn", NULL };
const char *ble_user_gain_table_region_type[] = { "FCC", "ETSI", "TELEC", "WORLD_WIDE", "KCC", NULL };
const char *wifi_interface_type[]             = { "ap", "ap_5g", "client", "client_5g", NULL };
const char *wifi_pll_mode_type[]              = { "pll_mode_20mhz", "pll_mode_40mhz", NULL };
const char *wifi_power_chain_type[]           = { "hp_chain", "lp_chain", NULL };
const char *wifi_security_type[]              = { "open",
                                                  "wep",
                                                  "wpa",
                                                  "wpa2",
                                                  "wpa2_enterprise",
                                                  "wpa3",
                                                  "wpa3_transition",
                                                  "wpa_enterprise",
                                                  "wpa_wpa2_mixed",
                                                  "wpa3_enterprise",
                                                  "wpa3_transition_enterprise",
                                                  NULL };
const char *wps_mode_type[]                   = { "pin", "push_button", NULL };

const arg_list_t console_argument_types[] = {
  [CONSOLE_TYPE(ap_client_deauth)]                = ap_client_deauth_type,
  [CONSOLE_TYPE(boolean)]                         = boolean_type,
  [CONSOLE_TYPE(bsd_socket_family)]               = bsd_socket_family_type,
  [CONSOLE_TYPE(bsd_socket_protocol)]             = bsd_socket_protocol_type,
  [CONSOLE_TYPE(bsd_socket_type)]                 = bsd_socket_type_type,
  [CONSOLE_TYPE(continuous_flag)]                 = continuous_flag_type,
  [CONSOLE_TYPE(data_rate)]                       = data_rate_type,
  [CONSOLE_TYPE(enable_bg_scan)]                  = enable_bg_scan_type,
  [CONSOLE_TYPE(flags)]                           = flags_type,
  [CONSOLE_TYPE(get_option_id)]                   = get_option_id_type,
  [CONSOLE_TYPE(ip_protocol)]                     = ip_protocol_type,
  [CONSOLE_TYPE(ipv4_or_ipv6)]                    = ipv4_or_ipv6_type,
  [CONSOLE_TYPE(net_interface)]                   = net_interface_type,
  [CONSOLE_TYPE(operating_band)]                  = operating_band_type,
  [CONSOLE_TYPE(option_name)]                     = option_name_type,
  [CONSOLE_TYPE(performance_mode)]                = performance_mode_type,
  [CONSOLE_TYPE(rate_protocol)]                   = rate_protocol_type,
  [CONSOLE_TYPE(set_option_id)]                   = set_option_id_type,
  [CONSOLE_TYPE(sl_ip_address_type_t)]            = sl_ip_address_type_t_type,
  [CONSOLE_TYPE(sl_ip_management_t)]              = sl_ip_management_t_type,
  [CONSOLE_TYPE(sl_net_dns_resolution_ip_type_t)] = sl_net_dns_resolution_ip_type_t_type,
  [CONSOLE_TYPE(socket_domain)]                   = socket_domain_type,
  [CONSOLE_TYPE(socket_protocol)]                 = socket_protocol_type,
  [CONSOLE_TYPE(socket_type)]                     = socket_type_type,
  [CONSOLE_TYPE(wifi_ap_flag)]                    = wifi_ap_flag_type,
  [CONSOLE_TYPE(wifi_band)]                       = wifi_band_type,
  [CONSOLE_TYPE(wifi_bandwidth)]                  = wifi_bandwidth_type,
  [CONSOLE_TYPE(wifi_encryption)]                 = wifi_encryption_type,
  [CONSOLE_TYPE(wifi_init_mode)]                  = wifi_init_mode_type,
  [CONSOLE_TYPE(wifi_init_region)]                = wifi_init_region_type,
  [CONSOLE_TYPE(ble_user_gain_table_region)]      = ble_user_gain_table_region_type,
  [CONSOLE_TYPE(wifi_interface)]                  = wifi_interface_type,
  [CONSOLE_TYPE(wifi_pll_mode)]                   = wifi_pll_mode_type,
  [CONSOLE_TYPE(wifi_power_chain)]                = wifi_power_chain_type,
  [CONSOLE_TYPE(wifi_security)]                   = wifi_security_type,
  [CONSOLE_TYPE(wps_mode)]                        = wps_mode_type,
};

const value_list_t console_argument_values[] = {
  [CONSOLE_TYPE(ap_client_deauth)]    = (const uint32_t[]){ SL_WIFI_DEAUTH, SL_WIFI_DEAUTH_UNSPECIFIED },
  [CONSOLE_TYPE(boolean)]             = (const uint32_t[]){ 1 },
  [CONSOLE_TYPE(bsd_socket_family)]   = (const uint32_t[]){ 2, 24 },
  [CONSOLE_TYPE(bsd_socket_protocol)] = (const uint32_t[]){ 6, 17 },
  [CONSOLE_TYPE(bsd_socket_type)]     = (const uint32_t[]){ 2, 1 },
  [CONSOLE_TYPE(continuous_flag)]     = (const uint32_t[]){ 1, 0 },
  [CONSOLE_TYPE(data_rate)] =
    (const uint32_t[]){ SL_WIFI_RATE_11B_11,    SL_WIFI_RATE_11G_12,    SL_WIFI_RATE_11N_MCS1,  SL_WIFI_RATE_11AX_MCS1,
                        SL_WIFI_RATE_11G_18,    SL_WIFI_RATE_11B_1,     SL_WIFI_RATE_11N_MCS2,  SL_WIFI_RATE_11G_24,
                        SL_WIFI_RATE_11AX_MCS2, SL_WIFI_RATE_11N_MCS3,  SL_WIFI_RATE_11B_2,     SL_WIFI_RATE_11AX_MCS3,
                        SL_WIFI_RATE_11G_36,    SL_WIFI_RATE_11N_MCS4,  SL_WIFI_RATE_11G_48,    SL_WIFI_RATE_11B_5_5,
                        SL_WIFI_RATE_11AX_MCS4, SL_WIFI_RATE_11G_54,    SL_WIFI_RATE_11N_MCS5,  SL_WIFI_RATE_11N_MCS6,
                        SL_WIFI_RATE_11AX_MCS5, SL_WIFI_RATE_11G_6,     SL_WIFI_RATE_11N_MCS0,  SL_WIFI_RATE_11N_MCS7,
                        SL_WIFI_RATE_11AX_MCS6, SL_WIFI_RATE_11AX_MCS0, SL_WIFI_RATE_11AX_MCS7, SL_WIFI_RATE_11G_9,
                        SL_WIFI_AUTO_RATE },
  [CONSOLE_TYPE(enable_bg_scan)]   = (const uint32_t[]){ 1 },
  [CONSOLE_TYPE(flags)]            = (const uint32_t[]){ 1, 0 },
  [CONSOLE_TYPE(get_option_id)]    = (const uint32_t[]){ 4, 2, 3, 5 },
  [CONSOLE_TYPE(ip_protocol)]      = (const uint32_t[]){ IOT_SOCKET_IPPROTO_TCP, IOT_SOCKET_IPPROTO_UDP },
  [CONSOLE_TYPE(ipv4_or_ipv6)]     = (const uint32_t[]){ 1, 2 },
  [CONSOLE_TYPE(net_interface)]    = (const uint32_t[]){ SL_NET_WIFI_AP_INTERFACE,
                                                         SL_NET_BLUETOOTH_INTERFACE,
                                                         SL_NET_WIFI_CLIENT_INTERFACE,
                                                         SL_NET_ETHERNET_INTERFACE,
                                                         SL_NET_THREAD_INTERFACE,
                                                         SL_NET_ZWAVE_INTERFACE },
  [CONSOLE_TYPE(operating_band)]   = (const uint32_t[]){ 0, 1, 2 },
  [CONSOLE_TYPE(option_name)]      = (const uint32_t[]){ 4134, 8, 12306, 4102, 31 },
  [CONSOLE_TYPE(performance_mode)] = (const uint32_t[]){ HIGH_PERFORMANCE,
                                                         ASSOCIATED_POWER_SAVE,
                                                         ASSOCIATED_POWER_SAVE_LOW_LATENCY,
                                                         DEEP_SLEEP_WITHOUT_RAM_RETENTION,
                                                         DEEP_SLEEP_WITH_RAM_RETENTION },
  [CONSOLE_TYPE(rate_protocol)]    = (const uint32_t[]){ SL_WIFI_RATE_PROTOCOL_AX_ONLY,
                                                         SL_WIFI_RATE_PROTOCOL_B_ONLY,
                                                         SL_WIFI_RATE_PROTOCOL_G_ONLY,
                                                         SL_WIFI_RATE_PROTOCOL_N_ONLY,
                                                         SL_WIFI_RATE_PROTOCOL_AUTO },
  [CONSOLE_TYPE(set_option_id)]    = (const uint32_t[]){ 1, 4, 2, 3 },
  [CONSOLE_TYPE(sl_ip_address_type_t)] =
    (const uint32_t[]){ SL_IPV4, SL_IPV6, SL_IPV6_GLOBAL, SL_IPV6_LINK_LOCAL, SL_IPV6_SITE_LOCAL },
  [CONSOLE_TYPE(sl_ip_management_t)] = (const uint32_t[]){ SL_IP_MANAGEMENT_DHCP, SL_IP_MANAGEMENT_STATIC_IP },
  [CONSOLE_TYPE(sl_net_dns_resolution_ip_type_t)] = (const uint32_t[]){ SL_NET_DNS_TYPE_IPV4, SL_NET_DNS_TYPE_IPV6 },
  [CONSOLE_TYPE(socket_domain)]                   = (const uint32_t[]){ 2, 0 },
  [CONSOLE_TYPE(socket_protocol)] =
    (const uint32_t[]){ IPPROTO_ICMP, IPPROTO_IP, IPPROTO_RAW, IPPROTO_TCP, IPPROTO_UDP, IPPROTO_UDPLITE },
  [CONSOLE_TYPE(socket_type)]      = (const uint32_t[]){ IOT_SOCKET_SOCK_DGRAM, IOT_SOCKET_SOCK_STREAM },
  [CONSOLE_TYPE(wifi_ap_flag)]     = (const uint32_t[]){ SL_WIFI_HIDDEN_SSID },
  [CONSOLE_TYPE(wifi_band)]        = (const uint32_t[]){ SL_WIFI_BAND_2_4GHZ,
                                                         SL_WIFI_BAND_5GHZ,
                                                         SL_WIFI_BAND_60GHZ,
                                                         SL_WIFI_BAND_6GHZ,
                                                         SL_WIFI_BAND_900MHZ,
                                                         SL_WIFI_AUTO_BAND },
  [CONSOLE_TYPE(wifi_bandwidth)]   = (const uint32_t[]){ SL_WIFI_BANDWIDTH_10MHz,
                                                         SL_WIFI_BANDWIDTH_160MHz,
                                                         SL_WIFI_BANDWIDTH_20MHz,
                                                         SL_WIFI_BANDWIDTH_40MHz,
                                                         SL_WIFI_BANDWIDTH_80MHz },
  [CONSOLE_TYPE(wifi_encryption)]  = (const uint32_t[]){ SL_WIFI_CCMP_ENCRYPTION,
                                                         SL_WIFI_EAP_FAST_ENCRYPTION,
                                                         SL_WIFI_NO_ENCRYPTION,
                                                         SL_WIFI_PEAP_MSCHAPV2_ENCRYPTION,
                                                         SL_WIFI_TKIP_ENCRYPTION,
                                                         SL_WIFI_EAP_TLS_ENCRYPTION,
                                                         SL_WIFI_EAP_TTLS_ENCRYPTION,
                                                         SL_WIFI_WEP_ENCRYPTION },
  [CONSOLE_TYPE(wifi_init_mode)]   = (const uint32_t[]){ 1, 2, 4, 0, 5, 3, 6, 7 },
  [CONSOLE_TYPE(wifi_init_region)] = (const uint32_t[]){ DEFAULT_REGION, US, EU, JP, WORLD_DOMAIN, KR, SG, CN },
  [CONSOLE_TYPE(ble_user_gain_table_region)] = (const uint32_t[]){ FCC, ETSI, TELEC, WORLD_WIDE, KCC },
  [CONSOLE_TYPE(wifi_interface)]             = (const uint32_t[]){ SL_WIFI_AP_2_4GHZ_INTERFACE,
                                                                   SL_WIFI_AP_5GHZ_INTERFACE,
                                                                   SL_WIFI_CLIENT_2_4GHZ_INTERFACE,
                                                                   SL_WIFI_CLIENT_5GHZ_INTERFACE },
  [CONSOLE_TYPE(wifi_pll_mode)]              = (const uint32_t[]){ SL_WIFI_PLL_MODE_20MHZ, SL_WIFI_PLL_MODE_40MHZ },
  [CONSOLE_TYPE(wifi_power_chain)]           = (const uint32_t[]){ SL_WIFI_HP_CHAIN, SL_WIFI_LP_CHAIN },
  [CONSOLE_TYPE(wifi_security)]              = (const uint32_t[]){ SL_WIFI_OPEN,
                                                                   SL_WIFI_WEP,
                                                                   SL_WIFI_WPA,
                                                                   SL_WIFI_WPA2,
                                                                   SL_WIFI_WPA2_ENTERPRISE,
                                                                   SL_WIFI_WPA3,
                                                                   SL_WIFI_WPA3_TRANSITION,
                                                                   SL_WIFI_WPA_ENTERPRISE,
                                                                   SL_WIFI_WPA_WPA2_MIXED,
                                                                   SL_WIFI_WPA3_ENTERPRISE,
                                                                   SL_WIFI_WPA3_TRANSITION_ENTERPRISE },
  [CONSOLE_TYPE(wps_mode)]                   = (const uint32_t[]){ SL_WIFI_WPS_PIN_MODE, SL_WIFI_WPS_PUSH_BUTTON_MODE },
};

#ifdef __cplusplus
} /*extern "C" */
#endif