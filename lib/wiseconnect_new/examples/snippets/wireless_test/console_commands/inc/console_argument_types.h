
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
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************
 *                     Macros
 ******************************************************/

#define CONSOLE_TYPE(name) CONSOLE_##name##_TYPE

/******************************************************
 *                   Enumerations
 ******************************************************/
typedef enum {
  CONSOLE_TYPE(ap_client_deauth),
  CONSOLE_TYPE(boolean),
  CONSOLE_TYPE(bsd_socket_family),
  CONSOLE_TYPE(bsd_socket_protocol),
  CONSOLE_TYPE(bsd_socket_type),
  CONSOLE_TYPE(continuous_flag),
  CONSOLE_TYPE(data_rate),
  CONSOLE_TYPE(enable_bg_scan),
  CONSOLE_TYPE(flags),
  CONSOLE_TYPE(get_option_id),
  CONSOLE_TYPE(ip_protocol),
  CONSOLE_TYPE(ipv4_or_ipv6),
  CONSOLE_TYPE(net_interface),
  CONSOLE_TYPE(operating_band),
  CONSOLE_TYPE(option_name),
  CONSOLE_TYPE(performance_mode),
  CONSOLE_TYPE(rate_protocol),
  CONSOLE_TYPE(set_option_id),
  CONSOLE_TYPE(sl_ip_address_type_t),
  CONSOLE_TYPE(sl_ip_management_t),
  CONSOLE_TYPE(sl_net_dns_resolution_ip_type_t),
  CONSOLE_TYPE(socket_domain),
  CONSOLE_TYPE(socket_protocol),
  CONSOLE_TYPE(socket_type),
  CONSOLE_TYPE(wifi_ap_flag),
  CONSOLE_TYPE(wifi_band),
  CONSOLE_TYPE(wifi_bandwidth),
  CONSOLE_TYPE(wifi_encryption),
  CONSOLE_TYPE(wifi_init_mode),
  CONSOLE_TYPE(wifi_init_region),
  CONSOLE_TYPE(ble_user_gain_table_region),
  CONSOLE_TYPE(wifi_interface),
  CONSOLE_TYPE(wifi_pll_mode),
  CONSOLE_TYPE(wifi_power_chain),
  CONSOLE_TYPE(wifi_security),
  CONSOLE_TYPE(wps_mode),
  CONSOLE_TYPE_COUNT // Equals the number of different types
} console_type_t;

typedef enum { ///< Factory default region
  FCC,         ///< United States
  ETSI,        ///< European Union
  TELEC,       ///< Japan
  WORLD_WIDE,  ///< World wide domain
  KCC          ///< Korea             ///< Singapore (not currently supported)
} sl_si91x_user_gain_table_region_code_t;

/******************************************************
 *                 Global Variables
 ******************************************************/

extern const char *ap_client_deauth_type[];
extern const char *boolean_type[];
extern const char *bsd_socket_family_type[];
extern const char *bsd_socket_protocol_type[];
extern const char *bsd_socket_type_type[];
extern const char *continuous_flag_type[];
extern const char *data_rate_type[];
extern const char *enable_bg_scan_type[];
extern const char *flags_type[];
extern const char *get_option_id_type[];
extern const char *ip_protocol_type[];
extern const char *ipv4_or_ipv6_type[];
extern const char *net_interface_type[];
extern const char *operating_band_type[];
extern const char *option_name_type[];
extern const char *performance_mode_type[];
extern const char *rate_protocol_type[];
extern const char *set_option_id_type[];
extern const char *sl_ip_address_type_t_type[];
extern const char *sl_ip_management_t_type[];
extern const char *sl_net_dns_resolution_ip_type_t_type[];
extern const char *socket_domain_type[];
extern const char *socket_protocol_type[];
extern const char *socket_type_type[];
extern const char *wifi_ap_flag_type[];
extern const char *wifi_band_type[];
extern const char *wifi_bandwidth_type[];
extern const char *wifi_encryption_type[];
extern const char *wifi_init_mode_type[];
extern const char *wifi_init_region_type[];
extern const char *ble_user_gain_table_region_type[];
extern const char *wifi_interface_type[];
extern const char *wifi_pll_mode_type[];
extern const char *wifi_power_chain_type[];
extern const char *wifi_security_type[];
extern const char *wps_mode_type[];

extern const arg_list_t console_argument_types[];
extern const value_list_t console_argument_values[];

#ifdef __cplusplus
} /*extern "C" */
#endif
