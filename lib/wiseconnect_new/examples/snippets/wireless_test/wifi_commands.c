/*******************************************************************************
* @file  wifi_commands.c
* @brief 
*******************************************************************************
* # License
* <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
*******************************************************************************
*
* The licensor of this software is Silicon Laboratories Inc. Your use of this
* software is governed by the terms of Silicon Labs Master Software License
* Agreement (MSLA) available at
* www.silabs.com/about-us/legal/master-software-license-agreement. This
* software is distributed to you in Source Code format and is governed by the
* sections of the MSLA applicable to Source Code.
*
******************************************************************************/

/** @file
 *
 * File Description
 *
 */
#include "console.h"
#include "sl_utility.h"
#include "sl_board_configuration.h"
#include "sl_constants.h"
#include "sl_wifi.h"
#include "sl_wifi_device.h"
#include "sl_wifi_callback_framework.h"
#include "errno.h"
#include "sl_string.h"
#include "sl_ip_types.h"
#include "cmsis_os2.h"
#include "sl_utility.h"
#include "sl_net.h"
#include "sl_net_wifi_types.h"
#include "cacert.pem.h"
#include "wifiuser.pem.h"
#include "aws_starfield_ca.pem.h"
#include "aws_client_certificate.pem.crt.h"
#include "aws_client_private_key.pem.key.h"
#include "sl_si91x_driver.h"

#include <stdio.h>
#include <string.h>
#include <stdbool.h>

/******************************************************
 *                    Constants
 ******************************************************/

#define SOFT_AP_PSK          "MY_AP_PASSPHRASE"
#define SOFT_AP_SSID         "MY_AP_SSID"
#define WIFI_CONNECT_TIMEOUT 1000
#define WIFI_SCAN_TIMEOUT    10000
#define TWT_SCAN_TIMEOUT     10000

#define WIFI_MODE_STA   0
#define WIFI_MODE_AP    1
#define WIFI_MODE_APSTA 2
#define WIFI_MODE_EAP   3

#define WIFI_MODE_BLE_COEX      4 // BLE Coex mode
#define WIFI_TRANSMIT_TEST_MODE 6
#define BLE_MODE                7

#define OPENSSL_SERVER 0
#define AWS_SERVER     1
#define AZURE_SERVER   2

#define ADV_SCAN_THRESHOLD           -40
#define ADV_RSSI_TOLERANCE_THRESHOLD 5
#define ADV_ACTIVE_SCAN_DURATION     15
#define ADV_PASSIVE_SCAN_DURATION    20
#define ADV_MULTIPROBE               0
#define ADV_SCAN_PERIODICITY         10

#define MAX_RECEIVE_STATS_COUNT 5

// Rejoin Constants
#define REJOIN_MAX_RETRY           5  // If 0 retries infinity times
#define REJOIN_SCAN_INTERVAL       4  // Periodicity of rejoin attempt
#define REJOIN_BEACON_MISSED_COUNT 40 // Beacon missed count
#define REJOIN_FIRST_TIME_RETRY    0  // 0 - Disable, 1 - Enable, retry for first time join failure

//Powersave optimizations enable/disable
#define ENABLE_POWERSAVE_CLI 1

/******************************************************
 *               Static Function Declarations
 ******************************************************/

static sl_status_t twt_callback_handler(sl_wifi_event_t event,
                                        sl_status_t status_code,
                                        sl_wifi_twt_response_t *result,
                                        uint32_t result_length,
                                        void *arg);
/******************************************************
 *               Extern Variable
 ******************************************************/

/******************************************************
 *               Global Variable
 ******************************************************/

static uint8_t stats_count = 0;
uint16_t total_crc_pass    = 0;
uint16_t total_crc_fail    = 0;
uint16_t crc_pass          = 0;
uint16_t crc_fail          = 0;

volatile bool scan_results_complete      = false;
volatile sl_status_t callback_status     = SL_STATUS_OK;
volatile bool stop_wifi_statistic_report = false;
osThreadId_t wifi_statistic_thread_id    = NULL;
static uint32_t max_receive_stats_count  = 0;
float pass_avg                           = 0;
float fail_avg                           = 0;

typedef struct {
  sl_wifi_interface_t interface;
  uint16_t channel;
  uint32_t max_stats_count;
} wifi_statistic_params_t;

static wifi_statistic_params_t statistic_params = { 0 };

typedef struct {
  sl_wifi_operation_mode_t operation_mode;
  sl_wifi_band_mode_t band;
  sl_wifi_region_code_t region_code;
} set_region_struct_t;

/******************************************************
 *               Variable Definitions
 ******************************************************/

/// Wi-Fi client configuration for Wireless Test
static const sl_wifi_device_configuration_t sl_wifi_default_client_configuration_cli = {
  .boot_option = LOAD_NWP_FW,
  .mac_address = NULL,
  .band        = SL_SI91X_WIFI_BAND_2_4GHZ,
  .region_code = WORLD_DOMAIN,
  .boot_config = { .oper_mode = SL_SI91X_CLIENT_MODE,
                   .coex_mode = SL_SI91X_WLAN_ONLY_MODE,
                   .feature_bit_map =
#ifdef SLI_SI91X_MCU_INTERFACE
                     (SL_WIFI_FEAT_SECURITY_OPEN | SL_WIFI_FEAT_WPS_DISABLE),
#else
                     (SL_WIFI_FEAT_SECURITY_OPEN | SL_WIFI_FEAT_AGGREGATION),
#endif
                   .tcp_ip_feature_bit_map =
                     (SL_SI91X_TCP_IP_FEAT_DHCPV4_CLIENT | SL_SI91X_TCP_IP_FEAT_DNS_CLIENT | SL_SI91X_TCP_IP_FEAT_SSL
#ifdef SLI_SI91X_ENABLE_IPV6
                      | SL_SI91X_TCP_IP_FEAT_DHCPV6_CLIENT | SL_SI91X_TCP_IP_FEAT_IPV6
#endif
                      | SL_SI91X_TCP_IP_FEAT_ICMP | SL_SI91X_TCP_IP_FEAT_EXTENSION_VALID),
                   .custom_feature_bit_map = SL_WIFI_SYSTEM_CUSTOM_FEAT_EXTENSION_VALID,
                   .ext_custom_feature_bit_map =
                     (SL_SI91X_EXT_FEAT_XTAL_CLK | SL_SI91X_EXT_FEAT_UART_SEL_FOR_DEBUG_PRINTS | MEMORY_CONFIG
#if ENABLE_POWERSAVE_CLI
                      | SL_WIFI_SYSTEM_EXT_FEAT_LOW_POWER_MODE
#endif
#ifdef SLI_SI917
                      | SL_SI91X_EXT_FEAT_FRONT_END_SWITCH_PINS_ULP_GPIO_4_5_0
#endif
                      ),
                   .bt_feature_bit_map = 0,
                   .ext_tcp_ip_feature_bit_map =
                     (SL_SI91X_EXT_TCP_IP_WINDOW_SCALING | SL_SI91X_EXT_TCP_IP_TOTAL_SELECTS(10)
                      | SL_SI91X_CONFIG_FEAT_EXTENTION_VALID),
                   .ble_feature_bit_map     = 0,
                   .ble_ext_feature_bit_map = 0,
                   .config_feature_bit_map  = (SL_SI91X_FEAT_SLEEP_GPIO_SEL_BITMAP | SL_WIFI_ENABLE_ENHANCED_MAX_PSP) }
};

/// Wi-Fi AP configuration for Wireless Test
static const sl_wifi_device_configuration_t sl_wifi_default_ap_configuration_cli = {
  .boot_option = LOAD_NWP_FW,
  .mac_address = NULL,
  .band        = SL_SI91X_WIFI_BAND_2_4GHZ,
  .region_code = US,
  .boot_config = { .oper_mode                  = SL_SI91X_ACCESS_POINT_MODE,
                   .coex_mode                  = SL_SI91X_WLAN_ONLY_MODE,
                   .feature_bit_map            = SL_WIFI_FEAT_SECURITY_OPEN,
                   .tcp_ip_feature_bit_map     = SL_SI91X_TCP_IP_FEAT_DHCPV4_SERVER,
                   .custom_feature_bit_map     = SL_WIFI_SYSTEM_CUSTOM_FEAT_EXTENSION_VALID,
                   .ext_custom_feature_bit_map = (
#if ENABLE_POWERSAVE_CLI
                     SL_WIFI_SYSTEM_EXT_FEAT_LOW_POWER_MODE
#else
                     0,
#endif
#ifdef SLI_SI917
                     | SL_SI91X_EXT_FEAT_FRONT_END_SWITCH_PINS_ULP_GPIO_4_5_0
#endif
                     ),
                   .bt_feature_bit_map         = 0,
                   .ext_tcp_ip_feature_bit_map = 0,
                   .ble_feature_bit_map        = 0,
                   .ble_ext_feature_bit_map    = 0,
                   .config_feature_bit_map     = 0 }
};

/// Wi-Fi concurrent (AP + STATION) configuration for Wireless Test
static const sl_wifi_device_configuration_t sl_wifi_default_concurrent_configuration_cli = {
  .boot_option = LOAD_NWP_FW,
  .mac_address = NULL,
  .band        = SL_SI91X_WIFI_BAND_2_4GHZ,
  .region_code = US,
  .boot_config = { .oper_mode              = SL_SI91X_CONCURRENT_MODE,
                   .coex_mode              = SL_SI91X_WLAN_ONLY_MODE,
                   .feature_bit_map        = SL_WIFI_FEAT_AGGREGATION,
                   .tcp_ip_feature_bit_map = (SL_SI91X_TCP_IP_FEAT_DHCPV4_CLIENT | SL_SI91X_TCP_IP_FEAT_DHCPV4_SERVER
                                              | SL_SI91X_TCP_IP_FEAT_ICMP | SL_SI91X_TCP_IP_FEAT_EXTENSION_VALID),
                   .custom_feature_bit_map = SL_WIFI_SYSTEM_CUSTOM_FEAT_EXTENSION_VALID,
                   .ext_custom_feature_bit_map = (SL_SI91X_EXT_FEAT_XTAL_CLK | MEMORY_CONFIG
#if ENABLE_POWERSAVE_CLI
                                                  | SL_WIFI_SYSTEM_EXT_FEAT_LOW_POWER_MODE
#endif
#ifdef SLI_SI917
                                                  | SL_SI91X_EXT_FEAT_FRONT_END_SWITCH_PINS_ULP_GPIO_4_5_0
#endif
                                                  ),
                   .bt_feature_bit_map         = 0,
                   .ext_tcp_ip_feature_bit_map = SL_SI91X_CONFIG_FEAT_EXTENTION_VALID,
                   .ble_feature_bit_map        = 0,
                   .ble_ext_feature_bit_map    = 0,
                   .config_feature_bit_map     = SL_SI91X_FEAT_SLEEP_GPIO_SEL_BITMAP }
};

/// Wi-Fi enterprise client configuration for Wireless Test
static const sl_wifi_device_configuration_t sl_wifi_default_enterprise_client_configuration_cli = {
  .boot_option = LOAD_NWP_FW,
  .mac_address = NULL,
  .band        = SL_SI91X_WIFI_BAND_2_4GHZ,
  .region_code = US,
  .boot_config = { .oper_mode              = SL_SI91X_ENTERPRISE_CLIENT_MODE,
                   .coex_mode              = SL_SI91X_WLAN_ONLY_MODE,
                   .feature_bit_map        = (SL_WIFI_FEAT_SECURITY_OPEN | SL_WIFI_FEAT_AGGREGATION),
                   .tcp_ip_feature_bit_map = (SL_SI91X_TCP_IP_FEAT_DHCPV4_CLIENT | SL_SI91X_TCP_IP_FEAT_ICMP
                                              | SL_SI91X_TCP_IP_FEAT_EXTENSION_VALID),
                   .custom_feature_bit_map = SL_WIFI_SYSTEM_CUSTOM_FEAT_EXTENSION_VALID,
                   .ext_custom_feature_bit_map =
                     (SL_SI91X_EXT_FEAT_XTAL_CLK | SL_SI91X_EXT_FEAT_UART_SEL_FOR_DEBUG_PRINTS | MEMORY_CONFIG
#if ENABLE_POWERSAVE_CLI
                      | SL_WIFI_SYSTEM_EXT_FEAT_LOW_POWER_MODE
#endif
#ifdef SLI_SI917
                      | SL_SI91X_EXT_FEAT_FRONT_END_SWITCH_PINS_ULP_GPIO_4_5_0
#endif
                      ),
                   .bt_feature_bit_map = 0,
                   .ext_tcp_ip_feature_bit_map =
                     (SL_SI91X_EXT_TCP_IP_WINDOW_SCALING | SL_SI91X_EXT_TCP_IP_TOTAL_SELECTS(10)),
                   .ble_feature_bit_map     = 0,
                   .ble_ext_feature_bit_map = 0,
                   .config_feature_bit_map  = 0 }
};

/// Wi-Fi transmit configuration for Wireless Test
static const sl_wifi_device_configuration_t sl_wifi_transmit_test_configuration_cli = {
  .boot_option = LOAD_NWP_FW,
  .mac_address = NULL,
  .band        = SL_SI91X_WIFI_BAND_2_4GHZ,
  .region_code = WORLD_DOMAIN,
  .boot_config = { .oper_mode = SL_SI91X_TRANSMIT_TEST_MODE,
                   .coex_mode = SL_SI91X_WLAN_ONLY_MODE,
                   .feature_bit_map =
#ifdef SLI_SI91X_MCU_INTERFACE
                     (SL_WIFI_FEAT_SECURITY_OPEN | SL_WIFI_FEAT_WPS_DISABLE),
#else
                     (SL_WIFI_FEAT_SECURITY_OPEN),
#endif
                   .tcp_ip_feature_bit_map =
                     (SL_SI91X_TCP_IP_FEAT_DHCPV4_CLIENT | SL_SI91X_TCP_IP_FEAT_EXTENSION_VALID),
                   .custom_feature_bit_map     = SL_WIFI_SYSTEM_CUSTOM_FEAT_EXTENSION_VALID,
                   .ext_custom_feature_bit_map = (MEMORY_CONFIG
#ifdef SLI_SI917
                                                  | SL_SI91X_EXT_FEAT_FRONT_END_SWITCH_PINS_ULP_GPIO_4_5_0
#endif
                                                  ),
                   .bt_feature_bit_map         = SL_SI91X_BT_RF_TYPE,
                   .ext_tcp_ip_feature_bit_map = SL_SI91X_CONFIG_FEAT_EXTENTION_VALID,
                   .ble_feature_bit_map        = 0,
                   .ble_ext_feature_bit_map    = 0,
                   .config_feature_bit_map     = SL_SI91X_FEAT_SLEEP_GPIO_SEL_BITMAP }
};

extern const sl_wifi_device_configuration_t sl_wifi_ble_configuration_cli;

sl_wifi_device_configuration_t si91x_init_configuration = {
  .boot_option = LOAD_NWP_FW,
  .mac_address = NULL,
  .band        = SL_SI91X_WIFI_BAND_2_4GHZ,
  .boot_config = { .oper_mode              = SL_SI91X_CLIENT_MODE,
                   .coex_mode              = SL_SI91X_WLAN_ONLY_MODE,
                   .feature_bit_map        = (SL_WIFI_FEAT_EAP_LEAP_IN_COEX),
                   .tcp_ip_feature_bit_map = (SL_SI91X_TCP_IP_FEAT_LOAD_PUBLIC_PRIVATE_CERTS),
                   .custom_feature_bit_map = SL_SI91X_CUSTOM_FEAT_HTTP_HTTPS_AUTH,
                   .ext_custom_feature_bit_map =
                     (SL_SI91X_EXT_FEAT_1P8V_SUPPORT | SL_SI91X_EXT_FEAT_UART_SEL_FOR_DEBUG_PRINTS
                      | SL_SI91X_EXT_FEAT_XTAL_CLK),
                   .bt_feature_bit_map         = 0,
                   .ext_tcp_ip_feature_bit_map = 0,
                   .ble_feature_bit_map        = 0,
                   .ble_ext_feature_bit_map    = 0,
                   .config_feature_bit_map     = (SL_SI91X_FEAT_DVS_SEL_CONFIG_3 | SL_SI91X_XTAL_GOODTIME_2000us) }
};

static sl_net_wifi_eap_credential_entry_t wifi_client_enterprise_eap_credential = {
  .type                = SL_NET_WIFI_PSK,
  .data_length         = sizeof(sl_wifi_eap_credential_t),
  .data.username       = "user1",
  .data.password       = "password1",
  .data.certificate_id = SL_NET_DEFAULT_WIFI_CLIENT_CREDENTIAL_ID
};

sl_wifi_twt_request_t default_twt_setup_configuration = {
  .twt_enable              = 1,
  .twt_flow_id             = 1,
  .wake_duration           = 0x60,
  .wake_duration_unit      = 0,
  .wake_duration_tol       = 0x60,
  .wake_int_exp            = 13,
  .wake_int_exp_tol        = 13,
  .wake_int_mantissa       = 0x1D4C,
  .wake_int_mantissa_tol   = 0x1D4C,
  .implicit_twt            = 1,
  .un_announced_twt        = 1,
  .triggered_twt           = 0,
  .twt_channel             = 0,
  .twt_protection          = 0,
  .restrict_tx_outside_tsp = 1,
  .twt_retry_limit         = 6,
  .twt_retry_interval      = 10,
  .req_type                = 1,
  .negotiation_type        = 0,
};

sl_wifi_twt_request_t default_twt_teardown_configuration = {
  .twt_enable  = 0,
  .twt_flow_id = 0xFF,
};

const structure_descriptor_entry_t rs91x_chip_config[] = {
  SL_CONSOLE_STRUCTURE_ENTRY("opermode", boot_config.oper_mode, sl_wifi_device_configuration_t, CONSOLE_VARIABLE_UINT),
  SL_CONSOLE_STRUCTURE_ENTRY("features",
                             boot_config.feature_bit_map,
                             sl_wifi_device_configuration_t,
                             CONSOLE_VARIABLE_UINT),
  SL_CONSOLE_STRUCTURE_ENTRY("tcp",
                             boot_config.tcp_ip_feature_bit_map,
                             sl_wifi_device_configuration_t,
                             CONSOLE_VARIABLE_UINT),
  SL_CONSOLE_STRUCTURE_ENTRY("custom",
                             boot_config.custom_feature_bit_map,
                             sl_wifi_device_configuration_t,
                             CONSOLE_VARIABLE_UINT),
  SL_CONSOLE_STRUCTURE_ENTRY("ext_custom",
                             boot_config.ext_custom_feature_bit_map,
                             sl_wifi_device_configuration_t,
                             CONSOLE_VARIABLE_UINT),
  SL_CONSOLE_STRUCTURE_ENTRY("bt",
                             boot_config.bt_feature_bit_map,
                             sl_wifi_device_configuration_t,
                             CONSOLE_VARIABLE_UINT),
  SL_CONSOLE_STRUCTURE_ENTRY("ext_tcp",
                             boot_config.ext_tcp_ip_feature_bit_map,
                             sl_wifi_device_configuration_t,
                             CONSOLE_VARIABLE_UINT),
  SL_CONSOLE_STRUCTURE_ENTRY("ble",
                             boot_config.ble_feature_bit_map,
                             sl_wifi_device_configuration_t,
                             CONSOLE_VARIABLE_UINT),
  SL_CONSOLE_STRUCTURE_ENTRY("ext_ble",
                             boot_config.ble_ext_feature_bit_map,
                             sl_wifi_device_configuration_t,
                             CONSOLE_VARIABLE_UINT),
  SL_CONSOLE_STRUCTURE_ENTRY("config",
                             boot_config.config_feature_bit_map,
                             sl_wifi_device_configuration_t,
                             CONSOLE_VARIABLE_UINT),
};

const sl_wifi_data_rate_t rate              = SL_WIFI_DATA_RATE_6;
sl_wifi_request_tx_test_info_t tx_test_info = {
  .enable      = 1,
  .power       = 127,
  .rate        = rate,
  .length      = 100,
  .mode        = 0,
  .channel     = 1,
  .aggr_enable = 0,
#ifdef SLI_SI917
  .enable_11ax            = 0,
  .coding_type            = 0,
  .nominal_pe             = 0,
  .ul_dl                  = 0,
  .he_ppdu_type           = 0,
  .beam_change            = 0,
  .bw                     = 0,
  .stbc                   = 0,
  .tx_bf                  = 0,
  .gi_ltf                 = 0,
  .dcm                    = 0,
  .nsts_midamble          = 0,
  .spatial_reuse          = 0,
  .bss_color              = 0,
  .he_siga2_reserved      = 0,
  .ru_allocation          = 0,
  .n_heltf_tot            = 0,
  .sigb_dcm               = 0,
  .sigb_mcs               = 0,
  .user_sta_id            = 0,
  .user_idx               = 0,
  .sigb_compression_field = 0,
#endif
};

/******************************************************
 *               Function Definitions
 ******************************************************/
static const char *get_wifi_interface_name(sl_wifi_interface_t interface)
{
  switch (interface) {
    case SL_WIFI_CLIENT_2_4GHZ_INTERFACE:
      return "2.4GHz client";
    case SL_WIFI_AP_2_4GHZ_INTERFACE:
      return "2.4GHz access point";
    case SL_WIFI_CLIENT_5GHZ_INTERFACE:
      return "5GHz client";
    case SL_WIFI_AP_5GHZ_INTERFACE:
      return "5GHz access point";
    default:
      return "Unknown";
  }
}

static const char *get_sl_wifi_protocol_name(sl_wifi_rate_protocol_t rate_protocol)
{
  switch (rate_protocol) {
    case SL_WIFI_RATE_PROTOCOL_B_ONLY:
      return "802.11b";
    case SL_WIFI_RATE_PROTOCOL_G_ONLY:
      return "802.11g";
    case SL_WIFI_RATE_PROTOCOL_N_ONLY:
      return "802.11n";
    case SL_WIFI_RATE_PROTOCOL_AC_ONLY:
      return "802.11ac";
    case SL_WIFI_RATE_PROTOCOL_AX_ONLY:
      return "802.11ax";
    case SL_WIFI_RATE_PROTOCOL_AUTO:
      return "AUTO";
    default:
      return "Unknown";
  }
}

static const char *get_sl_wifi_rate_name(sl_wifi_rate_t mask)
{
  switch (mask) {
    case SL_WIFI_AUTO_RATE:
      return "AUTO";
    case SL_WIFI_RATE_11B_1:
      return "1Mbps";
    case SL_WIFI_RATE_11B_2:
      return "2Mbps";
    case SL_WIFI_RATE_11B_5_5:
      return "5.5Mbps";
    case SL_WIFI_RATE_11B_11:
      return "11Mbps";
    case SL_WIFI_RATE_11G_6:
      return "6Mbps";
    case SL_WIFI_RATE_11G_9:
      return "9Mbps";
    case SL_WIFI_RATE_11G_12:
      return "12Mbps";
    case SL_WIFI_RATE_11G_18:
      return "18Mbps";
    case SL_WIFI_RATE_11G_24:
      return "24Mbps";
    case SL_WIFI_RATE_11G_36:
      return "36Mbps";
    case SL_WIFI_RATE_11G_48:
      return "48Mbps";
    case SL_WIFI_RATE_11G_54:
      return "54Mbps";
    case SL_WIFI_RATE_11N_MCS0:
      return "7.2Mbps";
    case SL_WIFI_RATE_11N_MCS1:
      return "14.4Mbps";
    case SL_WIFI_RATE_11N_MCS2:
      return "21.7Mbps";
    case SL_WIFI_RATE_11N_MCS3:
      return "28.9Mbps";
    case SL_WIFI_RATE_11N_MCS4:
      return "43.3Mbps";
    case SL_WIFI_RATE_11N_MCS5:
      return "57.8Mbps";
    case SL_WIFI_RATE_11N_MCS6:
      return "65Mbps";
    case SL_WIFI_RATE_11N_MCS7:
      return "72.2Mbps";
    case SL_WIFI_RATE_11AX_MCS0:
      return "8.6Mbps";
    case SL_WIFI_RATE_11AX_MCS1:
      return "17.2Mbps";
    case SL_WIFI_RATE_11AX_MCS2:
      return "25.8Mbps";
    case SL_WIFI_RATE_11AX_MCS3:
      return "34.4Mbps";
    case SL_WIFI_RATE_11AX_MCS4:
      return "51.6Mbps";
    case SL_WIFI_RATE_11AX_MCS5:
      return "68.8Mbps";
    case SL_WIFI_RATE_11AX_MCS6:
      return "77.4Mbps";
    case SL_WIFI_RATE_11AX_MCS7:
      return "86Mbps";
    default:
      return "Unknown";
  }
}

static inline const char *get_performance_profile_name(sl_wifi_system_performance_profile_t profile)
{
  switch (profile) {
    case SL_WIFI_SYSTEM_HIGH_PERFORMANCE:
      return "High Performance";
    case SL_WIFI_SYSTEM_ASSOCIATED_POWER_SAVE:
      return "Associated power save";
    case SL_WIFI_SYSTEM_ASSOCIATED_POWER_SAVE_LOW_LATENCY:
      return "Associated power save low latency";
    case SL_WIFI_SYSTEM_DEEP_SLEEP_WITHOUT_RAM_RETENTION:
      return "Standby low power";
    default:
      return "Unknown";
  }
}

sl_status_t wifi_init_command_handler(console_args_t *arguments)
{
  sl_status_t status;
  sl_wifi_device_configuration_t config;

  const int mode = GET_OPTIONAL_COMMAND_ARG(arguments, 0, WIFI_MODE_APSTA, const int);

  switch (mode) {
    case WIFI_MODE_STA:
      config = sl_wifi_default_client_configuration_cli;
      break;
    case WIFI_MODE_AP:
      config = sl_wifi_default_ap_configuration_cli;
      break;
    case WIFI_MODE_APSTA:
      config = sl_wifi_default_concurrent_configuration_cli;
      break;
    case WIFI_MODE_EAP:
      config = sl_wifi_default_enterprise_client_configuration_cli;
      break;
    case WIFI_TRANSMIT_TEST_MODE:
      config = sl_wifi_transmit_test_configuration_cli;
      break;
    case BLE_MODE:
      config = sl_wifi_ble_configuration_cli;
      break;
    default:
      printf("Selected Wi-Fi mode is not supported. Try 'help'");
      return SL_STATUS_WIFI_UNKNOWN_INTERFACE;
  }

  config.band = GET_OPTIONAL_COMMAND_ARG(arguments, 1, SL_SI91X_WIFI_BAND_2_4GHZ, const int);

  status = sl_wifi_init(&config, NULL, sl_wifi_default_event_handler);
  VERIFY_STATUS_AND_RETURN(status);

  switch (mode) {
    case WIFI_MODE_STA:
      printf("Started client mode");
      break;
    case WIFI_MODE_AP:
      printf("Started AP mode");
      break;
    case WIFI_MODE_APSTA:
      printf("Started STA + AP mode");
      break;
    case WIFI_MODE_EAP:
      printf("Started enterprise client mode");
      break;
    case WIFI_MODE_BLE_COEX:
      printf("Started BLE Coex mode");
      break;
    case BLE_MODE:
      printf("Started BLE mode");
      break;
    default:
      break;
  }

  return status;
}

sl_status_t wifi_set_antenna_command_handler(console_args_t *arguments)
{
  sl_wifi_interface_t interface =
    GET_OPTIONAL_COMMAND_ARG(arguments, 0, SL_WIFI_CLIENT_2_4GHZ_INTERFACE, sl_wifi_interface_t);
  sl_wifi_antenna_t antenna = GET_OPTIONAL_COMMAND_ARG(arguments, 1, SL_WIFI_ANTENNA_INTERNAL, sl_wifi_antenna_t);
  return sl_wifi_set_antenna(interface, antenna);
}

static sl_status_t show_scan_results(sl_wifi_scan_result_t *scan_result)
{
  SL_WIFI_ARGS_CHECK_NULL_POINTER(scan_result);
  uint8_t *bssid = NULL;

  printf("%lu scan results:\n", scan_result->scan_count);
  if (scan_result->scan_count)
    printf("\n   %s %24s %s", "SSID", "SECURITY", "NETWORK");
  printf("%12s %12s %s\n", "BSSID", "CHANNEL", "RSSI");
  for (unsigned int a = 0; a < scan_result->scan_count; ++a) {
    bssid = (uint8_t *)&scan_result->scan_info[a].bssid;
    printf("%-24s %4u,  %4u, ",
           scan_result->scan_info[a].ssid,
           scan_result->scan_info[a].security_mode,
           scan_result->scan_info[a].network_type);
    printf("  %02x:%02x:%02x:%02x:%02x:%02x, %4u,  -%u\n",
           bssid[0],
           bssid[1],
           bssid[2],
           bssid[3],
           bssid[4],
           bssid[5],
           scan_result->scan_info[a].rf_channel,
           scan_result->scan_info[a].rssi_val);
  }
  printf("End");

  return SL_STATUS_OK;
}

sl_status_t scan_callback_handler(sl_wifi_event_t event,
                                  sl_status_t status_code,
                                  sl_wifi_scan_result_t *result,
                                  uint32_t result_length,
                                  void *arg)
{
  UNUSED_PARAMETER(result_length);
  UNUSED_PARAMETER(arg);
  if (SL_WIFI_CHECK_IF_EVENT_FAILED(event)) {
    callback_status       = status_code;
    scan_results_complete = true;
    return status_code;
  }

  if (result_length != 0) {
    callback_status = show_scan_results(result);
  }

  scan_results_complete = true;
  return SL_STATUS_OK;
}

sl_status_t wifi_stats_receive_handler(sl_wifi_event_t event,
                                       sl_status_t status_code,
                                       void *reponse,
                                       uint32_t result_length,
                                       void *arg)
{
  UNUSED_PARAMETER(result_length);
  UNUSED_PARAMETER(arg);

  if (stop_wifi_statistic_report) {
    return SL_STATUS_OK;
  }

  if (SL_WIFI_CHECK_IF_EVENT_FAILED(event)) {
    callback_status = status_code;
    return status_code;
  }

  if (event == SL_WIFI_STATS_ASYNC_EVENT) {
    sl_wifi_async_stats_response_t *result = (sl_wifi_async_stats_response_t *)reponse;

    printf("%s: WIFI STATS Recieved packet# %d\n", __func__, stats_count);
    printf("stats : crc_pass %d, crc_fail %d, cal_rssi :%d\n", result->crc_pass, result->crc_fail, result->cal_rssi);
    stats_count++;

    float p = result->crc_pass;
    float f = result->crc_fail;
    float t = p + f;

    float per_pass = (p * 100 / t);
    float per_fail = (f * 100 / t);

    pass_avg += per_pass;
    fail_avg += per_fail;

    total_crc_pass += result->crc_pass;
    total_crc_fail += result->crc_fail;

    callback_status = SL_STATUS_OK;
  }

  return SL_STATUS_OK;
}

sl_status_t wifi_get_signal_strength_command_handler(console_args_t *arguments)
{
  int32_t signal_strength = 0;
  sl_status_t status      = SL_STATUS_OK;
  sl_wifi_interface_t interface =
    GET_OPTIONAL_COMMAND_ARG(arguments, 0, SL_WIFI_CLIENT_2_4GHZ_INTERFACE, sl_wifi_interface_t);

  status = sl_wifi_get_signal_strength(interface, &signal_strength);
  VERIFY_STATUS_AND_RETURN(status);

  printf("%ld", signal_strength);
  return status;
}

sl_status_t wifi_scan_command_handler(console_args_t *arguments)
{
  sl_status_t status = SL_STATUS_OK;
  sl_wifi_ssid_t optional_ssid_arg;
  sl_wifi_scan_configuration_t wifi_scan_configuration = { 0 };
  callback_status                                      = status;
  scan_results_complete                                = false;

  char *ssid = GET_OPTIONAL_COMMAND_ARG(arguments, 0, NULL, char *);
  sl_wifi_interface_t interface =
    GET_OPTIONAL_COMMAND_ARG(arguments, 1, SL_WIFI_CLIENT_2_4GHZ_INTERFACE, sl_wifi_interface_t);
  bool advanced_scan_enabled = GET_OPTIONAL_COMMAND_ARG(arguments, 2, false, const bool);
  uint16_t channel           = GET_OPTIONAL_COMMAND_ARG(arguments, 3, 0, const uint16_t);
  bool synchronous           = GET_OPTIONAL_COMMAND_ARG(arguments, 4, false, const bool);

  if (ssid != NULL) {
    optional_ssid_arg.length = (uint8_t)sl_strnlen(ssid, sizeof(optional_ssid_arg.value));
    memcpy(optional_ssid_arg.value, ssid, optional_ssid_arg.length);
  }

  if (advanced_scan_enabled) {
    wifi_scan_configuration.type = SL_WIFI_SCAN_TYPE_ADV_SCAN;

    wifi_scan_configuration.periodic_scan_interval =
      GET_OPTIONAL_COMMAND_ARG(arguments, 5, ADV_SCAN_PERIODICITY, uint32_t);
  } else {
    wifi_scan_configuration = default_wifi_scan_configuration;

    if (channel && (interface & SL_WIFI_2_4GHZ_INTERFACE)) {
      wifi_scan_configuration.channel_bitmap_2g4 = (1 << (channel - 1));
    }
  }

  if (!synchronous) {
    sl_wifi_set_scan_callback_v2(scan_callback_handler, NULL);
  }

  status = sl_wifi_start_scan(interface, (ssid != NULL) ? &optional_ssid_arg : NULL, &wifi_scan_configuration);

  if (synchronous) {
    sl_wifi_scan_result_t *scan_results = NULL;

    status = sl_wifi_wait_for_scan_results(&scan_results, SL_WIFI_MAX_SCANNED_AP);

    if (status != SL_STATUS_OK || scan_results == NULL) {
      return SL_STATUS_FAIL;
    }

    show_scan_results(scan_results);
    free(scan_results);

    return SL_STATUS_OK;
  }

  if (SL_STATUS_IN_PROGRESS == status) {
    printf("Scanning...\r\n");
    const uint32_t start = osKernelGetTickCount();

    while (!scan_results_complete && (osKernelGetTickCount() - start) <= WIFI_SCAN_TIMEOUT) {
      osThreadYield();
    }

    status = scan_results_complete ? callback_status : SL_STATUS_TIMEOUT;
  }

  return status;
}

sl_status_t join_callback_handler(sl_wifi_event_t event,
                                  sl_status_t status_code,
                                  char *result,
                                  uint32_t result_length,
                                  void *arg)
{
  UNUSED_PARAMETER(arg);
  if (SL_WIFI_CHECK_IF_EVENT_FAILED(event)) {
    printf("F: Join Event received with %lu bytes payload\n", result_length);
    callback_status = status_code;
    return status_code;
  }

  printf("%c: Join Event received with %lu bytes payload\n", *result, result_length);

  callback_status = SL_STATUS_OK;
  return SL_STATUS_OK;
}

sl_status_t wifi_set_tx_power_command_handler(console_args_t *arguments)
{
  sl_wifi_max_tx_power_t max_tx_power = { 0 };
  max_tx_power.scan_tx_power          = GET_OPTIONAL_COMMAND_ARG(arguments, 0, 0x1F, uint8_t);
  max_tx_power.join_tx_power          = GET_OPTIONAL_COMMAND_ARG(arguments, 1, 0x1F, uint8_t);

  return sl_wifi_set_max_tx_power(SL_WIFI_CLIENT_2_4GHZ_INTERFACE, max_tx_power);
}

sl_status_t wifi_get_tx_power_command_handler(console_args_t *arguments)
{
  UNUSED_PARAMETER(arguments);
  sl_wifi_max_tx_power_t max_tx_power = { 0 };
  sl_status_t status                  = sl_wifi_get_max_tx_power(SL_WIFI_CLIENT_2_4GHZ_INTERFACE, &max_tx_power);

  if (status == SL_STATUS_OK) {
    printf("Scan Power value: %d", max_tx_power.scan_tx_power);
    printf("Join Power value: %d", max_tx_power.join_tx_power);
  }
  return status;
}

sl_status_t wifi_connect_command_handler(console_args_t *arguments)
{
  sl_status_t status                = SL_STATUS_OK;
  sl_wifi_credential_id_t id        = SL_NET_DEFAULT_WIFI_CLIENT_CREDENTIAL_ID;
  sl_wifi_client_configuration_t ap = { 0 };
  char *ssid                        = (char *)arguments->arg[0];
  const char *password              = GET_OPTIONAL_COMMAND_ARG(arguments, 1, NULL, const char *);
  sl_wifi_security_t security_type  = GET_OPTIONAL_COMMAND_ARG(arguments, 2, SL_WIFI_WPA2, sl_wifi_security_t);
  const char *user_name             = GET_OPTIONAL_COMMAND_ARG(arguments, 3, NULL, const char *);
  sl_wifi_encryption_t encryption_type =
    GET_OPTIONAL_COMMAND_ARG(arguments, 4, SL_WIFI_DEFAULT_ENCRYPTION, sl_wifi_encryption_t);
  uint32_t timeout_ms  = GET_OPTIONAL_COMMAND_ARG(arguments, 5, 0, uint32_t);
  ap.ssid.length       = GET_OPTIONAL_COMMAND_ARG(arguments, 6, strlen(ssid), uint8_t);
  ap.channel.channel   = GET_OPTIONAL_COMMAND_ARG(arguments, 7, 0, uint8_t);
  ap.channel.band      = GET_OPTIONAL_COMMAND_ARG(arguments, 8, SL_WIFI_AUTO_BAND, sl_wifi_band_t);
  ap.channel.bandwidth = GET_OPTIONAL_COMMAND_ARG(arguments, 9, SL_WIFI_BANDWIDTH_10MHz, sl_wifi_bandwidth_t);
  sl_wifi_interface_t interface =
    GET_OPTIONAL_COMMAND_ARG(arguments, 10, SL_WIFI_CLIENT_2_4GHZ_INTERFACE, sl_wifi_interface_t);

  SL_DEBUG_LOG("ssid=%s\n", ssid);
  SL_DEBUG_LOG("secType=%u\n", security_type);
  SL_DEBUG_LOG("encType=%u\n", encryption_type);
  SL_DEBUG_LOG("userName=%s\n", user_name);
  SL_DEBUG_LOG("pwd=%s\n", password);

  if (timeout_ms == 0) {
    sl_wifi_set_join_callback_v2(join_callback_handler, NULL);
  }

  if ((security_type == SL_WIFI_WPA_ENTERPRISE) || ((security_type == SL_WIFI_WPA2_ENTERPRISE))
      || (security_type == SL_WIFI_WPA3_ENTERPRISE) || (security_type == SL_WIFI_WPA3_TRANSITION_ENTERPRISE)) {
    if (user_name == NULL) {
      return SL_STATUS_INVALID_CREDENTIALS;
    }

    if (encryption_type == SL_WIFI_EAP_TLS_ENCRYPTION) {
      status =
        sl_net_set_credential(SL_NET_WIFI_EAP_CLIENT_CREDENTIAL_ID, SL_NET_CERTIFICATE, wifiuser, sizeof(wifiuser) - 1);
      VERIFY_STATUS_AND_RETURN(status);

      SL_DEBUG_LOG("Certificate set\n");
    }

    wifi_client_enterprise_eap_credential.data.certificate_id = id;
    memcpy(wifi_client_enterprise_eap_credential.data.username, user_name, strlen(user_name));
    memcpy(wifi_client_enterprise_eap_credential.data.password, password, strlen(password));
    status = sl_net_set_credential(id,
                                   SL_NET_EAP_CLIENT_CREDENTIAL,
                                   &(wifi_client_enterprise_eap_credential.data),
                                   sizeof(sl_wifi_eap_credential_t));
    VERIFY_STATUS_AND_RETURN(status);
  } else {
    if (security_type != SL_WIFI_OPEN) {
      status = sl_net_set_credential(id, SL_NET_WIFI_PSK, password, strlen(password));
      VERIFY_STATUS_AND_RETURN(status);
      SL_DEBUG_LOG("Credentials set\n");
    }
  }

  memcpy(ap.ssid.value, ssid, ap.ssid.length);
  ap.security      = security_type;
  ap.encryption    = encryption_type;
  ap.credential_id = id;

  SL_DEBUG_LOG("Connecting\n");
  status = sl_wifi_connect(interface, &ap, timeout_ms);
  if (status == SL_STATUS_IN_PROGRESS) {
    callback_status = SL_STATUS_IN_PROGRESS;
    while (callback_status == SL_STATUS_IN_PROGRESS) {
      osThreadYield();
    }
    status = callback_status;
  }

  return status;
}

sl_status_t wifi_disconnect_command_handler(console_args_t *arguments)
{
  sl_wifi_interface_t interface = GET_OPTIONAL_COMMAND_ARG(arguments, 0, SL_WIFI_CLIENT_INTERFACE, sl_wifi_interface_t);

  return sl_wifi_disconnect(interface);
}

sl_status_t wifi_get_firmware_version_command_handler(console_args_t *arguments)
{
  UNUSED_PARAMETER(arguments);
  sl_wifi_firmware_version_t version = { 0 };

  sl_status_t status = sl_wifi_get_firmware_version(&version);
  VERIFY_STATUS_AND_RETURN(status);

  print_firmware_version(&version);
  return status;
}

sl_status_t wifi_get_statistics_command_handler(console_args_t *arguments)
{
  UNUSED_PARAMETER(arguments);
  sl_status_t status              = SL_STATUS_OK;
  sl_wifi_statistics_t wifi_stats = { 0 };

  status = sl_wifi_get_statistics(SL_WIFI_CLIENT_INTERFACE, &wifi_stats);
  VERIFY_STATUS_AND_RETURN(status);

  printf("beacon_lost_count: %lu\r\n", wifi_stats.beacon_lost_count);
  printf("beacon_rx_count: %lu\r\n", wifi_stats.beacon_rx_count);
  printf("mcast_rx_count: %lu\r\n", wifi_stats.mcast_rx_count);
  printf("mcast_tx_count: %lu\r\n", wifi_stats.mcast_tx_count);
  printf("overrun_count: %lu\r\n", wifi_stats.overrun_count);
  printf("ucast_rx_count: %lu\r\n", wifi_stats.ucast_rx_count);
  printf("ucast_tx_count: %lu", wifi_stats.ucast_tx_count);
  return status;
}

sl_status_t wifi_get_operational_statistics_command_handler(console_args_t *arguments)
{
  UNUSED_PARAMETER(arguments);
  sl_status_t status                                      = SL_STATUS_OK;
  sl_wifi_operational_statistics_t operational_statistics = { 0 };

  status = sl_wifi_get_operational_statistics(SL_WIFI_CLIENT_INTERFACE, &operational_statistics);
  VERIFY_STATUS_AND_RETURN(status);

  printf("Operating mode: %d\r\n", operational_statistics.operating_mode);
  printf("Dtim period: %d\r\n", operational_statistics.dtim_period);
  printf("Ideal beacon info: %d\r\n", operational_statistics.ideal_beacon_info[0]);
  printf("Busy beacon info: %d\r\n", operational_statistics.busy_beacon_info[0]);
  printf("Busy interval: %d\r\n", operational_statistics.beacon_interval[0]);
  return status;
}

void wifi_statistic_thread(const void *arg)
{
  UNUSED_PARAMETER(arg);
  sl_status_t status;
  sl_wifi_interface_t interface;
  sl_wifi_channel_t channel;

  interface               = statistic_params.interface;
  channel.channel         = statistic_params.channel;
  max_receive_stats_count = statistic_params.max_stats_count;
  stats_count             = 0;
  sl_wifi_set_stats_callback_v2(wifi_stats_receive_handler, NULL);

  status = sl_wifi_start_statistic_report(interface, channel);
  if (SL_STATUS_IN_PROGRESS == status) {
    callback_status = SL_STATUS_IN_PROGRESS;
    while (stats_count <= max_receive_stats_count) {
      if (stop_wifi_statistic_report) {
        break;
      }
      osThreadYield();
      if (stats_count == max_receive_stats_count && callback_status != SL_STATUS_IN_PROGRESS) {
        stop_wifi_statistic_report = true;

        printf("\r\nCRC Average pass%% = %.6f,         CRC Average fail%% = %.6f \r\n",
               pass_avg / max_receive_stats_count,
               fail_avg / max_receive_stats_count);
        printf("Total : total_crc_pass %d, total_crc_fail %d \n", total_crc_pass, total_crc_fail);
        printf("%s: Stop Statistics Report\n", __func__);
        pass_avg       = 0;
        fail_avg       = 0;
        total_crc_pass = 0;
        total_crc_fail = 0;

        sl_wifi_stop_statistic_report(interface);
        osDelay(100);
        break;
      }
    }
  }

  wifi_statistic_thread_id = NULL;
  osThreadExit();
}

sl_status_t wifi_start_statistic_report_command_handler(console_args_t *arguments)
{
  statistic_params.interface = GET_OPTIONAL_COMMAND_ARG(arguments, 0, SL_WIFI_CLIENT_INTERFACE, sl_wifi_interface_t);
  statistic_params.channel   = GET_OPTIONAL_COMMAND_ARG(arguments, 1, 1, uint16_t);
  statistic_params.max_stats_count = GET_OPTIONAL_COMMAND_ARG(arguments, 2, 10, uint32_t);

  stop_wifi_statistic_report = false;
  wifi_statistic_thread_id   = osThreadNew((osThreadFunc_t)wifi_statistic_thread, NULL, NULL);
  if (wifi_statistic_thread_id == NULL) {
    return SL_STATUS_FAIL;
  }

  return SL_STATUS_OK;
}

sl_status_t wifi_stop_statistic_report_command_handler(console_args_t *arguments)
{
  if (wifi_statistic_thread_id != NULL) {
    stop_wifi_statistic_report = true;
    osThreadTerminate(wifi_statistic_thread_id);
    wifi_statistic_thread_id = NULL;
  }
  sl_status_t status            = SL_STATUS_OK;
  sl_wifi_interface_t interface = GET_OPTIONAL_COMMAND_ARG(arguments, 0, SL_WIFI_CLIENT_INTERFACE, sl_wifi_interface_t);

  printf("\r\n CRC Average pass%% = %.6f,         CRC Average fail%% = %.6f \r\n",
         pass_avg / max_receive_stats_count,
         fail_avg / max_receive_stats_count);
  printf("Total : total_crc_pass %d, total_crc_fail %d \n", total_crc_pass, total_crc_fail);
  printf("%s: Stop Statistics Report\n", __func__);
  pass_avg       = 0;
  fail_avg       = 0;
  total_crc_pass = 0;
  total_crc_fail = 0;

  status = sl_wifi_stop_statistic_report(interface);
  VERIFY_STATUS_AND_RETURN(status);
  return status;
}

sl_status_t wifi_get_mac_address_command_handler(console_args_t *arguments)
{
  sl_status_t status            = SL_STATUS_OK;
  sl_mac_address_t mac_addr     = { 0 };
  sl_wifi_interface_t interface = GET_OPTIONAL_COMMAND_ARG(arguments, 0, SL_WIFI_CLIENT_INTERFACE, sl_wifi_interface_t);

  status = sl_wifi_get_mac_address(interface, &mac_addr);
  VERIFY_STATUS_AND_RETURN(status);

  printf("%x:%x:%x:%x:%x:%x",
         mac_addr.octet[0],
         mac_addr.octet[1],
         mac_addr.octet[2],
         mac_addr.octet[3],
         mac_addr.octet[4],
         mac_addr.octet[5]);
  return status;
}

sl_status_t wifi_set_channel_command_handler(console_args_t *arguments)
{
  sl_status_t status;
  sl_wifi_interface_t interface  = (sl_wifi_interface_t)GET_COMMAND_ARG(arguments, 0);
  uint16_t channel               = (uint16_t)GET_COMMAND_ARG(arguments, 1);
  sl_wifi_channel_t channel_info = { .channel = channel };

  status = sl_wifi_set_channel(interface, channel_info);
  VERIFY_STATUS_AND_RETURN(status);

  printf("Channel is set to: %d\r\n", channel);
  return status;
}

sl_status_t wifi_get_channel_command_handler(console_args_t *arguments)
{
  sl_status_t status             = SL_STATUS_OK;
  sl_wifi_interface_t interface  = (sl_wifi_interface_t)GET_COMMAND_ARG(arguments, 0);
  sl_wifi_channel_t channel_info = { 0 };
  char *band_string;

  status = sl_wifi_get_channel(interface, &channel_info);
  VERIFY_STATUS_AND_RETURN(status);

  switch (channel_info.band) {
    case SL_WIFI_BAND_2_4GHZ:
      band_string = "2.4 Ghz";
      break;
    case SL_WIFI_BAND_5GHZ:
      band_string = "5 GHz";
      break;
    default:
      band_string = "Auto Band";
      break;
  }
  printf("The operating channel: %d and band: %s\r\n", channel_info.channel, band_string);
  return status;
}

sl_status_t wifi_disconnect_ap_client_command_handler(console_args_t *arguments)
{
  sl_status_t status;

  sl_mac_address_t *mac_address = (sl_mac_address_t *)GET_COMMAND_ARG(arguments, 0);
  sl_wifi_deauth_reason_t deauth_reason =
    GET_OPTIONAL_COMMAND_ARG(arguments, 1, SL_WIFI_DEAUTH_UNSPECIFIED, sl_wifi_deauth_reason_t);

  status = sl_wifi_disconnect_ap_client(SL_WIFI_AP_INTERFACE, mac_address, deauth_reason);
  VERIFY_STATUS_AND_RETURN(status);

  return status;
}

sl_status_t wifi_get_ap_client_info_command_handler(console_args_t *argument)
{
  UNUSED_PARAMETER(argument);
  sl_status_t status;
  sl_wifi_client_info_response_t client_info = { 0 };

  status = sl_wifi_get_ap_client_info(SL_WIFI_AP_INTERFACE, &client_info);

  if (client_info.client_count <= 0) {
    printf("No clients!!");
    return status;
  }

  for (uint16_t station_info_index = 0; station_info_index < client_info.client_count; station_info_index++) {
    sl_wifi_client_info_t *station_info = &client_info.client_info[station_info_index];
    printf("%d) MAC Address is %x:%x:%x:%x:%x:%x",
           station_info_index + 1,
           station_info->mac_adddress.octet[0],
           station_info->mac_adddress.octet[1],
           station_info->mac_adddress.octet[2],
           station_info->mac_adddress.octet[3],
           station_info->mac_adddress.octet[4],
           station_info->mac_adddress.octet[5]);

    print_sl_ip_address(&station_info->ip_address);
  }

  return status;
}

sl_status_t wifi_set_performance_profile_command_handler(console_args_t *argument)
{
  sl_wifi_system_performance_profile_t profile         = GET_COMMAND_ARG(argument, 0);
  sl_wifi_performance_profile_v2_t performance_profile = { 0 };
  performance_profile.profile                          = profile;

  return sl_wifi_set_performance_profile_v2(&performance_profile);
}

sl_status_t wifi_get_performance_profile_command_handler(console_args_t *arguments)
{
  UNUSED_PARAMETER(arguments);
  sl_status_t status;
  sl_wifi_performance_profile_v2_t performance_profile = { 0 };

  status = sl_wifi_get_performance_profile_v2(&performance_profile);
  VERIFY_STATUS_AND_RETURN(status);

  printf("%s", get_performance_profile_name(performance_profile.profile));

  return status;
}

sl_status_t wifi_deinit_command_handler(console_args_t *arguments)
{
  UNUSED_PARAMETER(arguments);
  return sl_wifi_deinit();
}

sl_status_t wifi_start_ap_command_handler(console_args_t *arguments)
{
  sl_status_t status;
  sl_wifi_ap_configuration_t ap_configuration = { 0 };
  sl_wifi_credential_t cred                   = { 0 };
  char *ssid                                  = GET_OPTIONAL_COMMAND_ARG(arguments, 0, SOFT_AP_SSID, char *);

  memcpy(ap_configuration.ssid.value, ssid, strlen(ssid));
  ap_configuration.ssid.length = (uint8_t)strlen((char *)ap_configuration.ssid.value);

  ap_configuration.security =
    GET_OPTIONAL_COMMAND_ARG(arguments, 1, default_wifi_ap_configuration.security, sl_wifi_security_t);
  ap_configuration.encryption =
    GET_OPTIONAL_COMMAND_ARG(arguments, 2, default_wifi_ap_configuration.encryption, sl_wifi_encryption_t);

  ap_configuration.channel.channel =
    GET_OPTIONAL_COMMAND_ARG(arguments, 3, default_wifi_ap_configuration.channel.channel, uint16_t);
  ap_configuration.channel.band =
    GET_OPTIONAL_COMMAND_ARG(arguments, 4, default_wifi_ap_configuration.channel.band, sl_wifi_band_t);
  ap_configuration.channel.bandwidth =
    GET_OPTIONAL_COMMAND_ARG(arguments, 5, default_wifi_ap_configuration.channel.bandwidth, sl_wifi_bandwidth_t);

  ap_configuration.rate_protocol =
    GET_OPTIONAL_COMMAND_ARG(arguments, 6, default_wifi_ap_configuration.rate_protocol, sl_wifi_rate_protocol_t);
  ap_configuration.options =
    GET_OPTIONAL_COMMAND_ARG(arguments, 7, default_wifi_ap_configuration.options, sl_wifi_ap_flag_t);
  ap_configuration.credential_id =
    GET_OPTIONAL_COMMAND_ARG(arguments, 8, default_wifi_ap_configuration.credential_id, sl_wifi_credential_id_t);
  ap_configuration.client_idle_timeout =
    GET_OPTIONAL_COMMAND_ARG(arguments, 9, default_wifi_ap_configuration.client_idle_timeout, uint32_t);
  ap_configuration.dtim_beacon_count =
    GET_OPTIONAL_COMMAND_ARG(arguments, 10, default_wifi_ap_configuration.dtim_beacon_count, uint16_t);
  ap_configuration.maximum_clients =
    GET_OPTIONAL_COMMAND_ARG(arguments, 11, default_wifi_ap_configuration.maximum_clients, uint8_t);

  ap_configuration.keepalive_type =
    GET_OPTIONAL_COMMAND_ARG(arguments, 12, default_wifi_ap_configuration.keepalive_type, uint8_t);
  ap_configuration.beacon_interval =
    GET_OPTIONAL_COMMAND_ARG(arguments, 13, default_wifi_ap_configuration.beacon_interval, uint16_t);

  ap_configuration.credential_id = SL_NET_DEFAULT_WIFI_AP_CREDENTIAL_ID;

  cred.type = SL_WIFI_PSK_CREDENTIAL;
  memcpy(cred.psk.value, SOFT_AP_PSK, strlen(SOFT_AP_PSK));

  status = sl_net_set_credential(ap_configuration.credential_id, SL_NET_WIFI_PSK, SOFT_AP_PSK, strlen(SOFT_AP_PSK));
  VERIFY_STATUS_AND_RETURN(status);

  status = sl_wifi_start_ap(SL_WIFI_AP_2_4GHZ_INTERFACE, &ap_configuration);
  VERIFY_STATUS_AND_RETURN(status);

  return status;
}

sl_status_t wifi_load_certificate_handler(console_args_t *arguments)
{
  sl_status_t status = SL_STATUS_OK;
  uint8_t load       = GET_OPTIONAL_COMMAND_ARG(arguments, 0, 1, uint8_t);
  uint8_t type       = GET_OPTIONAL_COMMAND_ARG(arguments, 2, 0, uint8_t);

  if ((load > 1) || (type > 2)) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  if (load == 0) {
    return status;
  }

  switch (type) {
    case OPENSSL_SERVER:
      status = sl_net_set_credential(SL_NET_TLS_SERVER_CREDENTIAL_ID(0),
                                     SL_NET_SIGNING_CERTIFICATE,
                                     cacert,
                                     sizeof(cacert) - 1);
      SL_VERIFY_SUCCESS_OR_EXIT(status);
      break;

    case AWS_SERVER:
      status = sl_net_set_credential(SL_NET_TLS_SERVER_CREDENTIAL_ID(0),
                                     SL_NET_SIGNING_CERTIFICATE,
                                     aws_starfield_ca,
                                     sizeof(aws_starfield_ca) - 1);
      SL_VERIFY_SUCCESS_OR_EXIT(status);

      status = sl_net_set_credential(SL_NET_TLS_CLIENT_CREDENTIAL_ID(0),
                                     SL_NET_CERTIFICATE,
                                     aws_client_certificate,
                                     sizeof(aws_client_certificate) - 1);
      SL_VERIFY_SUCCESS_OR_EXIT(status);

      status = sl_net_set_credential(SL_NET_TLS_CLIENT_CREDENTIAL_ID(0),
                                     SL_NET_PRIVATE_KEY,
                                     aws_client_private_key,
                                     sizeof(aws_client_private_key) - 1);
      SL_VERIFY_SUCCESS_OR_EXIT(status);
      break;

    case AZURE_SERVER:
    default:
      printf("Unsupported Server type");
      return SL_STATUS_FAIL;
  }

exit:
  if (status != SL_STATUS_OK) {
    printf("Loading TLS certificate Failed, Error Code : 0x%lX", status);
  }
  return status;
}

sl_status_t wifi_is_interface_up_command_handler(console_args_t *arguments)
{
  sl_wifi_interface_t interface = (sl_wifi_interface_t)GET_COMMAND_ARG(arguments, 0);

  if (sl_wifi_is_interface_up(interface)) {
    printf("Yes");
    return SL_STATUS_OK;
  } else {
    printf("No");
    return SL_STATUS_WIFI_INTERFACE_NOT_UP;
  }
}

sl_status_t wifi_get_default_interface_command_handler(console_args_t *arguments)
{
  UNUSED_PARAMETER(arguments);
  sl_wifi_interface_t interface = sl_wifi_get_default_interface();

  if (interface == SL_WIFI_INVALID_INTERFACE) {
    return SL_STATUS_NOT_INITIALIZED;
  }

  printf("%s", get_wifi_interface_name(interface));

  return SL_STATUS_OK;
}

sl_status_t wifi_set_default_interface_command_handler(console_args_t *arguments)
{
  sl_wifi_interface_t interface = (sl_wifi_interface_t)GET_COMMAND_ARG(arguments, 0);

  sl_wifi_set_default_interface(interface);

  return SL_STATUS_OK;
}

sl_status_t wifi_set_transmit_rate_command_handler(console_args_t *arguments)
{
  sl_wifi_rate_protocol_t rate_protocol = (sl_wifi_rate_protocol_t)GET_COMMAND_ARG(arguments, 0);
  sl_wifi_rate_t mask                   = (sl_wifi_rate_t)GET_COMMAND_ARG(arguments, 1);
  sl_wifi_interface_t interface =
    GET_OPTIONAL_COMMAND_ARG(arguments, 2, SL_WIFI_CLIENT_2_4GHZ_INTERFACE, sl_wifi_interface_t);

  sl_status_t status = sl_wifi_set_transmit_rate(interface, rate_protocol, mask);

  if (status != SL_STATUS_OK) {
    printf("\n Failed to configure transmit rate!\n");
  } else {
    printf("\n Transmit rate configured successfully!\n");
  }

  return status;
}

sl_status_t wifi_set_roam_configuration_command_handler(console_args_t *arguments)
{
  sl_status_t status = SL_STATUS_OK;
  sl_wifi_roam_configuration_t roam_configuration;
  sl_wifi_interface_t interface = GET_OPTIONAL_COMMAND_ARG(arguments, 0, SL_WIFI_CLIENT_INTERFACE, sl_wifi_interface_t);

  roam_configuration.trigger_level        = (int32_t)GET_COMMAND_ARG(arguments, 1);
  roam_configuration.trigger_level_change = (uint32_t)GET_COMMAND_ARG(arguments, 2);

  status = sl_wifi_set_roam_configuration(interface, &roam_configuration);

  return status;
}

sl_status_t wifi_get_transmit_rate_command_handler(console_args_t *arguments)
{
  sl_wifi_interface_t interface =
    GET_OPTIONAL_COMMAND_ARG(arguments, 0, SL_WIFI_CLIENT_2_4GHZ_INTERFACE, sl_wifi_interface_t);
  sl_wifi_rate_protocol_t rate_protocol = GET_OPTIONAL_COMMAND_ARG(arguments, 1, 0, sl_wifi_rate_protocol_t);
  sl_wifi_rate_t mask                   = GET_OPTIONAL_COMMAND_ARG(arguments, 2, 0, sl_wifi_rate_t);

  sl_status_t status = sl_wifi_get_transmit_rate(interface, &rate_protocol, &mask);

  if (status == SL_STATUS_OK) {
    printf("\n wifi protocol is: %s\n wifi rate is: %s\n",
           get_sl_wifi_protocol_name(rate_protocol),
           get_sl_wifi_rate_name(mask));
  }

  return status;
}

sl_status_t wifi_get_ap_client_list_command_handler(console_args_t *arguments)
{
  sl_status_t status;
  sl_mac_address_t client_list[SL_WIFI_MAX_CLIENT_COUNT] = { 0 };

  sl_wifi_interface_t interface = GET_OPTIONAL_COMMAND_ARG(arguments, 0, SL_WIFI_AP_INTERFACE, sl_wifi_interface_t);

  status = sl_wifi_get_ap_client_list(interface, SL_WIFI_MAX_CLIENT_COUNT, client_list);

  printf("Mac address list of clients connected to AP:");
  for (uint16_t index = 0; client_list[index].octet[0] != 0; index++) {
    printf("\n %d -> %x:%x:%x:%x:%x:%x",
           index,
           client_list[index].octet[0],
           client_list[index].octet[1],
           client_list[index].octet[2],
           client_list[index].octet[3],
           client_list[index].octet[4],
           client_list[index].octet[5]);
  }

  return status;
}

sl_status_t wifi_get_ap_client_count_command_handler(console_args_t *arguments)
{
  sl_status_t status;
  uint32_t client_list_count;

  sl_wifi_interface_t interface = GET_OPTIONAL_COMMAND_ARG(arguments, 0, SL_WIFI_AP_INTERFACE, sl_wifi_interface_t);

  status = sl_wifi_get_ap_client_count(interface, &client_list_count);

  if (status == SL_STATUS_OK) {
    printf("Client count = %lu\n", client_list_count);
  }

  return status;
}

sl_status_t wifi_generate_wps_pin_command_handler(console_args_t *arguments)
{
  UNUSED_PARAMETER(arguments);
  sl_wifi_wps_pin_t wps_pin;

  sl_status_t status = sl_wifi_generate_wps_pin(&wps_pin);

  if (status == SL_STATUS_OK) {
    printf("\n wps pin: %s\n", wps_pin.digits);
  }

  return status;
}

sl_status_t wifi_start_wps(console_args_t *arguments)
{
  sl_wifi_wps_pin_t pin         = { 0 };
  sl_wifi_interface_t interface = arguments->arg[0];
  sl_wifi_wps_mode_t wps_mode   = arguments->arg[1];

  char *wps_pin = GET_OPTIONAL_COMMAND_ARG(arguments, 2, NULL, char *);

  if (wps_pin != NULL) {
    memcpy(pin.digits, wps_pin, sizeof(pin.digits));
  }

  return sl_wifi_start_wps(interface, wps_mode, wps_pin == NULL ? NULL : &pin);
}
sl_status_t sl_wifi_set_advanced_scan_configuration_command_handler(console_args_t *arguments)
{
  sl_status_t status                                                = SL_STATUS_OK;
  sl_wifi_advanced_scan_configuration_t advanced_scan_configuration = { 0 };

  // Advance scanning parameters
  advanced_scan_configuration.active_channel_time =
    GET_OPTIONAL_COMMAND_ARG(arguments, 0, ADV_ACTIVE_SCAN_DURATION, uint16_t);
  advanced_scan_configuration.passive_channel_time =
    GET_OPTIONAL_COMMAND_ARG(arguments, 1, ADV_PASSIVE_SCAN_DURATION, uint16_t);
  advanced_scan_configuration.trigger_level = GET_OPTIONAL_COMMAND_ARG(arguments, 2, ADV_SCAN_THRESHOLD, int32_t);
  advanced_scan_configuration.trigger_level_change =
    GET_OPTIONAL_COMMAND_ARG(arguments, 3, ADV_RSSI_TOLERANCE_THRESHOLD, uint16_t);
  advanced_scan_configuration.enable_multi_probe = GET_OPTIONAL_COMMAND_ARG(arguments, 4, ADV_MULTIPROBE, uint8_t);

  status = sl_wifi_set_advanced_scan_configuration(&advanced_scan_configuration);

  return status;
}

sl_status_t sl_wifi_get_advanced_scan_configuration_command_handler(console_args_t *arguments)
{
  UNUSED_PARAMETER(arguments);
  sl_wifi_advanced_scan_configuration_t advanced_scan_configuration = { 0 };

  sl_status_t status = sl_wifi_get_advanced_scan_configuration(&advanced_scan_configuration);

  if (status == SL_STATUS_OK) {
    printf("\nAdvanced scan configurations:\n active channel time: %d\n passive channel time: %d\n trigger "
           "level: %d\n trigger level change: %lu\n enable multi probe: %d\n",
           advanced_scan_configuration.active_channel_time,
           advanced_scan_configuration.passive_channel_time,
           (int)advanced_scan_configuration.trigger_level,
           advanced_scan_configuration.trigger_level_change,
           advanced_scan_configuration.enable_multi_probe);
  }

  return status;
}

sl_status_t wifi_stop_scan_command_handler(console_args_t *arguments)
{
  sl_wifi_interface_t interface = GET_OPTIONAL_COMMAND_ARG(arguments, 0, SL_WIFI_CLIENT_INTERFACE, sl_wifi_interface_t);
  return sl_wifi_stop_scan(interface);
}

sl_status_t wifi_get_ap_configuration_command_handler(console_args_t *arguments)
{
  sl_status_t status;
  sl_wifi_ap_configuration_t configuration;
  sl_wifi_interface_t interface =
    GET_OPTIONAL_COMMAND_ARG(arguments, 0, SL_WIFI_AP_2_4GHZ_INTERFACE, sl_wifi_interface_t);

  status = sl_wifi_get_ap_configuration(interface, &configuration);
  VERIFY_STATUS_AND_RETURN(status);

  printf("SSID: %s\nsecurity: %d\nencryption: %d\nchannel: %d\nrate_protocol: %d\noptions: %d\ncredential_id: "
         "%lu\nclient idle timeout: %lu\ndtim beacon count: %d\nmaximum client: %d\n",
         configuration.ssid.value,
         configuration.security,
         configuration.encryption,
         configuration.channel.channel,
         configuration.rate_protocol,
         configuration.options,
         configuration.credential_id,
         configuration.client_idle_timeout,
         configuration.dtim_beacon_count,
         configuration.maximum_clients);
  return SL_STATUS_OK;
}

sl_status_t wifi_set_ap_configuration_command_handler(console_args_t *arguments)
{
  sl_wifi_ap_configuration_t configuration = { 0 };
  sl_wifi_interface_t interface            = SL_WIFI_AP_INTERFACE;
  sl_status_t status;

  const char *ssid         = GET_OPTIONAL_COMMAND_ARG(arguments, 0, SOFT_AP_SSID, char *);
  const size_t ssid_length = strlen(ssid);
  memcpy(configuration.ssid.value, ssid, ssid_length);

  configuration.ssid.length     = (uint8_t)ssid_length;
  configuration.security        = GET_OPTIONAL_COMMAND_ARG(arguments, 1, SL_WIFI_WPA2, sl_wifi_security_t);
  configuration.encryption      = GET_OPTIONAL_COMMAND_ARG(arguments, 2, SL_WIFI_CCMP_ENCRYPTION, sl_wifi_encryption_t);
  configuration.channel.channel = GET_OPTIONAL_COMMAND_ARG(arguments, 3, 11, uint8_t);
  configuration.rate_protocol =
    GET_OPTIONAL_COMMAND_ARG(arguments, 4, SL_WIFI_RATE_PROTOCOL_AUTO, sl_wifi_rate_protocol_t);
  configuration.options             = GET_OPTIONAL_COMMAND_ARG(arguments, 5, 0, uint8_t);
  configuration.credential_id       = GET_OPTIONAL_COMMAND_ARG(arguments, 6, 0, sl_wifi_credential_id_t);
  configuration.client_idle_timeout = GET_OPTIONAL_COMMAND_ARG(arguments, 7, 255, uint32_t);
  configuration.dtim_beacon_count   = GET_OPTIONAL_COMMAND_ARG(arguments, 8, 4, uint8_t);
  configuration.maximum_clients     = GET_OPTIONAL_COMMAND_ARG(arguments, 9, 4, uint8_t);

  status = sl_wifi_set_ap_configuration(interface, &configuration);
  VERIFY_STATUS_AND_RETURN(status);

  return SL_STATUS_OK;
}

sl_status_t wifi_test_client_configuration_command_handler(console_args_t *arguments)
{
  sl_status_t status                = SL_STATUS_OK;
  sl_wifi_credential_id_t id        = SL_NET_DEFAULT_WIFI_CLIENT_CREDENTIAL_ID;
  sl_wifi_interface_t interface     = SL_WIFI_CLIENT_2_4GHZ_INTERFACE;
  sl_wifi_client_configuration_t ap = { 0 };
  sl_wifi_credential_t cred         = { 0 };
  char *ssid                        = (char *)arguments->arg[0];
  char *password                    = (char *)arguments->arg[1];
  sl_wifi_security_t secType        = GET_OPTIONAL_COMMAND_ARG(arguments, 2, SL_WIFI_WPA2, sl_wifi_security_t);
  const char *user_name             = GET_OPTIONAL_COMMAND_ARG(arguments, 3, NULL, const char *);
  sl_wifi_encryption_t encType =
    GET_OPTIONAL_COMMAND_ARG(arguments, 4, SL_WIFI_DEFAULT_ENCRYPTION, sl_wifi_encryption_t);
  uint32_t timeout_ms  = GET_OPTIONAL_COMMAND_ARG(arguments, 5, 0, uint32_t);
  ap.ssid.length       = GET_OPTIONAL_COMMAND_ARG(arguments, 6, strlen(ssid), uint8_t);
  ap.channel.channel   = GET_OPTIONAL_COMMAND_ARG(arguments, 7, 0, uint8_t);
  ap.channel.band      = GET_OPTIONAL_COMMAND_ARG(arguments, 8, SL_WIFI_AUTO_BAND, sl_wifi_band_t);
  ap.channel.bandwidth = GET_OPTIONAL_COMMAND_ARG(arguments, 9, SL_WIFI_BANDWIDTH_10MHz, sl_wifi_bandwidth_t);

  UNUSED_PARAMETER(user_name);

  cred.type = SL_WIFI_PSK_CREDENTIAL;
  memcpy(cred.psk.value, password, strlen(password));

  if (timeout_ms == 0) {
    sl_wifi_set_join_callback_v2(join_callback_handler, NULL);
  }

  status = sl_net_set_credential(id, SL_NET_WIFI_PSK, password, strlen(password));
  if (status != SL_STATUS_OK) {
    return status;
  }

  memcpy(ap.ssid.value, ssid, ap.ssid.length);
  ap.security      = secType;
  ap.encryption    = encType;
  ap.credential_id = id;

  status = sl_wifi_test_client_configuration(interface, &ap, timeout_ms);

  if (SL_STATUS_IN_PROGRESS == status) {
    callback_status = SL_STATUS_IN_PROGRESS;
    while (callback_status == SL_STATUS_IN_PROGRESS) {
      osThreadYield();
    }
    status = callback_status;
    if (status == SL_STATUS_OK) {
      status = sl_wifi_disconnect(SL_WIFI_CLIENT_INTERFACE);
    }
  }
  VERIFY_STATUS_AND_RETURN(status);

  return SL_STATUS_OK;
}

sl_status_t sl_wifi_send_raw_data_command_handler(console_args_t *arguments)
{
  uint8_t buffer[300]           = { 0 };
  sl_wifi_interface_t interface = GET_OPTIONAL_COMMAND_ARG(arguments, 0, SL_WIFI_2_4GHZ_INTERFACE, sl_wifi_interface_t);
  uint16_t data_length;

  /*PING packet */

  //destination mac
  buffer[0] = 0x48;
  buffer[1] = 0x51;
  buffer[2] = 0xc5;
  buffer[3] = 0x09;
  buffer[4] = 0x7e;
  buffer[5] = 0xfa;

  //src mac
  buffer[6]  = 0x80;
  buffer[7]  = 0xc9;
  buffer[8]  = 0x55;
  buffer[9]  = 0x3d;
  buffer[10] = 0x23;
  buffer[11] = 0x90;

  //ip type
  buffer[12] = 0x08;

  //version and header length
  buffer[14] = 0x45;

  //total lenght
  buffer[17] = 0x26;

  //identification
  buffer[19] = 0x04;

  //time to live
  buffer[22] = 0x80;

  //protocol
  buffer[23] = 0x01;

  //header checksum
  buffer[24] = 0xb7;
  buffer[25] = 0xe1;

  //src ip
  buffer[26] = 192;
  buffer[27] = 168;
  buffer[28] = 0;
  buffer[29] = 202;

  //dst ip
  buffer[30] = 192;
  buffer[31] = 168;
  buffer[32] = 0;
  buffer[33] = 215;

  //ping type (request)
  buffer[34] = 0x8;

  //checksum
  buffer[36] = 0xe3;
  buffer[37] = 0x1c;

  //identifier le
  buffer[39] = 0xca;

  //data;
  buffer[43] = 0x01;
  buffer[44] = 0x02;
  buffer[45] = 0x03;
  buffer[46] = 0x04;
  buffer[47] = 0x05;
  buffer[48] = 0x06;
  buffer[49] = 0x07;
  buffer[50] = 0x08;
  buffer[51] = 0x09;
  buffer[52] = 0x00;

  data_length = sizeof(buffer);

  return sl_wifi_send_raw_data_frame(interface, buffer, data_length);
}

sl_status_t sl_wifi_enable_twt(console_args_t *arguments)
{
  UNUSED_PARAMETER(arguments);
  sl_wifi_set_twt_config_callback_v2(twt_callback_handler, NULL);
  sl_wifi_performance_profile_v2_t performance_profile = { .twt_request = default_twt_setup_configuration };
  sl_status_t status                                   = SL_STATUS_OK;
  status = sl_wifi_enable_target_wake_time(&performance_profile.twt_request);
  // A small delay is added so that the asynchronous response from TWT is printed in correct format.
  osDelay(100);
  return status;
}

sl_status_t sl_wifi_disable_twt(console_args_t *arguments)
{
  UNUSED_PARAMETER(arguments);
  sl_wifi_set_twt_config_callback_v2(twt_callback_handler, NULL);
  sl_wifi_performance_profile_v2_t performance_profile = { .twt_request = default_twt_teardown_configuration };
  sl_status_t status                                   = SL_STATUS_OK;
  status = sl_wifi_disable_target_wake_time(&performance_profile.twt_request);
  // A small delay is added so that the asynchronous response from TWT is printed in correct format.
  osDelay(100);
  return status;
}

static sl_status_t twt_callback_handler(sl_wifi_event_t event,
                                        sl_status_t status_code,
                                        sl_wifi_twt_response_t *result,
                                        uint32_t result_length,
                                        void *arg)
{
  UNUSED_PARAMETER(result_length);
  UNUSED_PARAMETER(arg);

  if (SL_WIFI_CHECK_IF_EVENT_FAILED(event)) {
    return status_code;
  }

  switch (event) {
    case SL_WIFI_TWT_RESPONSE_EVENT:
      printf("TWT Setup success");
      break;
    case SL_WIFI_TWT_UNSOLICITED_SESSION_SUCCESS_EVENT:
      printf("Unsolicited TWT Setup success");
      break;
    case SL_WIFI_TWT_AP_REJECTED_EVENT:
      printf("TWT Setup Failed. TWT Setup rejected by AP");
      break;
    case SL_WIFI_TWT_OUT_OF_TOLERANCE_EVENT:
      printf("TWT Setup Failed. TWT response out of tolerance limits");
      break;
    case SL_WIFI_TWT_RESPONSE_NOT_MATCHED_EVENT:
      printf("TWT Setup Failed. TWT Response not matched with the request parameters");
      break;
    case SL_WIFI_TWT_UNSUPPORTED_RESPONSE_EVENT:
      printf("TWT Setup Failed. TWT Response Unsupported");
      break;
    case SL_WIFI_TWT_FAIL_MAX_RETRIES_REACHED_EVENT:
      printf("TWT Setup Failed. Max retries reached");
      break;
    case SL_WIFI_TWT_INACTIVE_DUE_TO_ROAMING_EVENT:
      printf("TWT session inactive due to roaming");
      break;
    case SL_WIFI_TWT_INACTIVE_DUE_TO_DISCONNECT_EVENT:
      printf("TWT session inactive due to wlan disconnection");
      break;
    case SL_WIFI_TWT_TEARDOWN_SUCCESS_EVENT:
      printf("TWT session teardown success");
      break;
    case SL_WIFI_TWT_AP_TEARDOWN_SUCCESS_EVENT:
      printf("TWT session teardown from AP");
      break;
    case SL_WIFI_TWT_INACTIVE_NO_AP_SUPPORT_EVENT:
      printf("Connected AP Does not support TWT");
      break;
    default:
      printf("TWT Setup Failed.");
  }
  if (event < SL_WIFI_TWT_TEARDOWN_SUCCESS_EVENT) {
    printf("\r\n wake duration : 0x%X", result->wake_duration);
    printf("\r\n wake_duration_unit: 0x%X", result->wake_duration_unit);
    printf("\r\n wake_int_exp : 0x%X", result->wake_int_exp);
    printf("\r\n negotiation_type : 0x%X", result->negotiation_type);
    printf("\r\n wake_int_mantissa : 0x%X", result->wake_int_mantissa);
    printf("\r\n implicit_twt : 0x%X", result->implicit_twt);
    printf("\r\n un_announced_twt : 0x%X", result->un_announced_twt);
    printf("\r\n triggered_twt : 0x%X", result->triggered_twt);
    printf("\r\n twt_channel : 0x%X", result->twt_channel);
    printf("\r\n twt_protection : 0x%X", result->twt_protection);
    printf("\r\n twt_flow_id : 0x%X\r\n", result->twt_flow_id);
  } else if (event <= SL_WIFI_TWT_AP_TEARDOWN_SUCCESS_EVENT) {
    printf("\r\n twt_flow_id : 0x%X", result->twt_flow_id);
    printf("\r\n negotiation_type : 0x%X\r\n", result->negotiation_type);
  }
  return SL_STATUS_OK;
}

sl_status_t sl_wifi_filter_broadcast_command_handler(console_args_t *arguments)
{
  UNUSED_PARAMETER(arguments);
  sl_status_t status                     = SL_STATUS_OK;
  uint16_t beacon_drop_threshold         = 5000;
  uint8_t filter_bcast_in_tim            = 1;
  uint8_t filter_bcast_tim_till_next_cmd = 1;
  status = sl_wifi_filter_broadcast(beacon_drop_threshold, filter_bcast_in_tim, filter_bcast_tim_till_next_cmd);
  VERIFY_STATUS_AND_RETURN(status);
  return SL_STATUS_OK;
}

sl_status_t sl_wifi_assert_command_handler()
{
  return sl_si91x_assert();
}

sl_status_t sl_wifi_update_gain_table_command_handler(console_args_t *arguments)
{
  sl_status_t status = SL_STATUS_OK;
  uint8_t band       = (uint8_t)GET_COMMAND_ARG(arguments, 0);
  uint8_t bandwidth  = (uint8_t)GET_COMMAND_ARG(arguments, 1);

//! Update Bump up offset for 52 Tone RU as per Region
#define X_BUMP_UP_OFFSET_52_TONE_RU 1

//! Update Bump up offset for 106 Tone RU as per Region
#define Y_BUMP_UP_OFFSET_106_TONE_RU 2

  /* clang-format off */
  uint8_t su_gain_table_payload[]  = {
    4,//NUM_OF_REGIONS
        0, 0xB,//NUM_OF_CHANNELS
    //   rate,  11b, 11g, 11n, 11ax(SU) 11ax(TB)
            1,  32,  22,  22,  20,      14,
            2,  32,  26,  26,  24,      20,
            3,  34,  28,  28,  26,      30,
            4,  36,  34,  32,  30,      32,
            5,  36,  34,  34,  32,      32,
            6,  36,  36,  36,  34,      34,
            7,  36,  34,  34,  32,      28,
            8,  36,  32,  32,  30,      28,
            9,  36,  30,  30,  28,      28,
           10,  32,  28,  28,  24,      14,
           11,  32,  22,  22,  20,      14,
        1,0x11,
           255, 34,  36,  36,  36,      24,
        2,0x22,
           13,  34,  38,  36,  36,      24,
           14,  38,   0,   0,   0,       0,
        5, 0x25,
           1,   32,  22,  22,  20,      14,
           2,   32,  26,  26,  24,      20,
           11,  32,  22,  22,  20,      14,
           13,  34,  36,  36,  36,      24,
           14,  38,  0,   0,    0,      0,
    };
  /* clang-format on */

  status = sl_wifi_update_su_gain_table(band,
                                        bandwidth,
                                        su_gain_table_payload,
                                        sizeof(su_gain_table_payload),
                                        X_BUMP_UP_OFFSET_52_TONE_RU,
                                        Y_BUMP_UP_OFFSET_106_TONE_RU);
  VERIFY_STATUS_AND_RETURN(status);

  return SL_STATUS_OK;
}

sl_status_t sl_wifi_set_11ax_config_command_handler(console_args_t *arguments)
{
  sl_status_t status     = SL_STATUS_OK;
  uint8_t guard_interval = (uint8_t)GET_COMMAND_ARG(arguments, 0);
  status                 = sl_wifi_set_11ax_config(guard_interval);
  VERIFY_STATUS_AND_RETURN(status);

  return SL_STATUS_OK;
}

sl_status_t sl_si91x_get_ram_log_command_handler(console_args_t *arguments)
{
  uint32_t address = (uint32_t)GET_COMMAND_ARG(arguments, 0);
  uint32_t length  = (uint32_t)GET_COMMAND_ARG(arguments, 1);
  if (length == 0) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  return sl_si91x_get_ram_log(address, length);
}

sl_status_t sl_wifi_transmit_test_start_command_handler(console_args_t *arguments)
{
  sl_status_t status   = SL_STATUS_OK;
  tx_test_info.power   = (uint16_t)GET_COMMAND_ARG(arguments, 0);
  tx_test_info.rate    = (uint32_t)GET_COMMAND_ARG(arguments, 1);
  tx_test_info.length  = (uint16_t)GET_COMMAND_ARG(arguments, 2);
  tx_test_info.mode    = (uint16_t)GET_COMMAND_ARG(arguments, 3);
  tx_test_info.channel = (uint16_t)GET_COMMAND_ARG(arguments, 4);
  status               = sl_si91x_transmit_test_start(&tx_test_info);
  VERIFY_STATUS_AND_RETURN(status);
  return SL_STATUS_OK;
}

sl_status_t sl_wifi_ax_transmit_test_start_command_handler(console_args_t *arguments)
{
  sl_status_t status                  = SL_STATUS_OK;
  tx_test_info.power                  = (uint16_t)GET_COMMAND_ARG(arguments, 0);
  tx_test_info.rate                   = (uint32_t)GET_COMMAND_ARG(arguments, 1);
  tx_test_info.length                 = (uint16_t)GET_COMMAND_ARG(arguments, 2);
  tx_test_info.mode                   = (uint16_t)GET_COMMAND_ARG(arguments, 3);
  tx_test_info.channel                = (uint16_t)GET_COMMAND_ARG(arguments, 4);
  tx_test_info.aggr_enable            = (uint16_t)GET_COMMAND_ARG(arguments, 5);
  tx_test_info.enable_11ax            = (uint8_t)GET_COMMAND_ARG(arguments, 6);
  tx_test_info.coding_type            = (uint8_t)GET_COMMAND_ARG(arguments, 7);
  tx_test_info.nominal_pe             = (uint8_t)GET_COMMAND_ARG(arguments, 8);
  tx_test_info.ul_dl                  = (uint8_t)GET_COMMAND_ARG(arguments, 9);
  tx_test_info.he_ppdu_type           = (uint8_t)GET_COMMAND_ARG(arguments, 10);
  tx_test_info.beam_change            = (uint8_t)GET_COMMAND_ARG(arguments, 11);
  tx_test_info.bw                     = (uint8_t)GET_COMMAND_ARG(arguments, 12);
  tx_test_info.stbc                   = (uint8_t)GET_COMMAND_ARG(arguments, 13);
  tx_test_info.tx_bf                  = (uint8_t)GET_COMMAND_ARG(arguments, 14);
  tx_test_info.gi_ltf                 = (uint8_t)GET_COMMAND_ARG(arguments, 15);
  tx_test_info.dcm                    = (uint8_t)GET_COMMAND_ARG(arguments, 16);
  tx_test_info.nsts_midamble          = (uint8_t)GET_COMMAND_ARG(arguments, 17);
  tx_test_info.spatial_reuse          = (uint8_t)GET_COMMAND_ARG(arguments, 18);
  tx_test_info.bss_color              = (uint8_t)GET_COMMAND_ARG(arguments, 19);
  tx_test_info.he_siga2_reserved      = (uint16_t)GET_COMMAND_ARG(arguments, 20);
  tx_test_info.ru_allocation          = (uint8_t)GET_COMMAND_ARG(arguments, 21);
  tx_test_info.n_heltf_tot            = (uint8_t)GET_COMMAND_ARG(arguments, 22);
  tx_test_info.sigb_dcm               = (uint8_t)GET_COMMAND_ARG(arguments, 23);
  tx_test_info.sigb_mcs               = (uint8_t)GET_COMMAND_ARG(arguments, 24);
  tx_test_info.user_sta_id            = (uint16_t)GET_COMMAND_ARG(arguments, 25);
  tx_test_info.user_idx               = (uint8_t)GET_COMMAND_ARG(arguments, 26);
  tx_test_info.sigb_compression_field = (uint8_t)GET_COMMAND_ARG(arguments, 27);
  status                              = sl_si91x_transmit_test_start(&tx_test_info);
  VERIFY_STATUS_AND_RETURN(status);
  return SL_STATUS_OK;
}
sl_status_t sl_wifi_transmit_test_stop_command_handler(console_args_t *arguments)
{
  UNUSED_PARAMETER(arguments);
  sl_status_t status = SL_STATUS_OK;
  status             = sl_si91x_transmit_test_stop();
  VERIFY_STATUS_AND_RETURN(status);
  return SL_STATUS_OK;
}

sl_status_t wifi_set_advanced_client_config_command_handler(console_args_t *arguments)
{
  sl_status_t status;
  sl_wifi_advanced_client_configuration_t config = {
    .max_retry_attempts      = GET_OPTIONAL_COMMAND_ARG(arguments, 0, REJOIN_MAX_RETRY, uint32_t),
    .scan_interval           = GET_OPTIONAL_COMMAND_ARG(arguments, 1, REJOIN_SCAN_INTERVAL, uint32_t),
    .beacon_missed_count     = GET_OPTIONAL_COMMAND_ARG(arguments, 2, REJOIN_BEACON_MISSED_COUNT, uint32_t),
    .first_time_retry_enable = GET_OPTIONAL_COMMAND_ARG(arguments, 3, REJOIN_FIRST_TIME_RETRY, uint32_t)
  };
  status = sl_wifi_set_advanced_client_configuration(SL_WIFI_CLIENT_INTERFACE, &config);
  VERIFY_STATUS_AND_RETURN(status);
  return SL_STATUS_OK;
}

sl_status_t wifi_get_pairwise_master_key_command_handler(console_args_t *arguments)
{
  sl_status_t status              = SL_STATUS_OK;
  uint8_t pairwise_master_key[32] = { 0 };
  const uint8_t type              = (uint8_t)GET_COMMAND_ARG(arguments, 0);
  char *ssid                      = (char *)GET_COMMAND_ARG(arguments, 1);
  const char *pre_shared_key      = (char *)GET_COMMAND_ARG(arguments, 2);

  sl_wifi_ssid_t ssid_arg;
  sl_wifi_interface_t interface =
    GET_OPTIONAL_COMMAND_ARG(arguments, 3, SL_WIFI_CLIENT_2_4GHZ_INTERFACE, sl_wifi_interface_t);

  ssid_arg.length = (uint8_t)sl_strnlen(ssid, sizeof(ssid_arg.value));
  memcpy(ssid_arg.value, ssid, ssid_arg.length);

  status = sl_wifi_get_pairwise_master_key(interface, type, &ssid_arg, pre_shared_key, pairwise_master_key);
  VERIFY_STATUS_AND_RETURN(status);
  return SL_STATUS_OK;
}

sl_status_t wifi_configure_timeout_command_handler(console_args_t *arguments)
{
  sl_status_t status                         = SL_STATUS_OK;
  const sl_si91x_timeout_type_t timeout_type = (sl_si91x_timeout_type_t)GET_COMMAND_ARG(arguments, 0);
  const uint16_t timeout_value               = (uint16_t)GET_COMMAND_ARG(arguments, 1);

  status = sl_si91x_configure_timeout(timeout_type, timeout_value);
  VERIFY_STATUS_AND_RETURN(status);
  return SL_STATUS_OK;
}

sl_status_t sl_si91x_calibration_write_command_handler(console_args_t *arguments)
{
  sl_status_t status = SL_STATUS_OK;
  sl_si91x_calibration_write_t calib_pkt = {
       .target = GET_OPTIONAL_COMMAND_ARG(arguments, 5, 1, const uint8_t), ///< Burn into efuse or flash, @ref SI91X_BURN_TARGET_OPTIONS
       .reserved0   = {0, }, ///< Reserved bits
       .flags       = (uint32_t)GET_COMMAND_ARG(arguments, 0),       ///< Calibration Flags, @ref SI91X_CALIBRATION_FLAGS
       .gain_offset = {
           (int8_t)GET_COMMAND_ARG(arguments, 1),
           (int8_t)GET_COMMAND_ARG(arguments, 2),
           (int8_t)GET_COMMAND_ARG(arguments, 3)  ///< gain offset
       },
       .xo_ctune    = GET_OPTIONAL_COMMAND_ARG(arguments, 4, 0, const int8_t),      ///< xo ctune
  };
  status = sl_si91x_calibration_write(calib_pkt);
  VERIFY_STATUS_AND_RETURN(status);
  return SL_STATUS_OK;
}

sl_status_t sl_si91x_calibration_read_command_handler(console_args_t *arguments)
{
  sl_status_t status                 = SL_STATUS_OK;
  sl_si91x_calibration_read_t target = {
    0,
  };
  sl_si91x_calibration_read_t calib_read_pkt = {
    0,
  };

  target.target = GET_OPTIONAL_COMMAND_ARG(arguments, 0, 1, const uint8_t);

  status = sl_si91x_calibration_read(target, &calib_read_pkt);
  if (status == SL_STATUS_OK) {
    printf("target %d, reserved:%d "
           "gain_offset_low:%d, gain_offset_2:%d, gain_offset_3:%d,xo_tune:%d\r\n",
           calib_read_pkt.target,
           calib_read_pkt.reserved0[0],
           calib_read_pkt.gain_offset[0],
           calib_read_pkt.gain_offset[1],
           calib_read_pkt.gain_offset[2],
           calib_read_pkt.xo_ctune);
  }
  VERIFY_STATUS_AND_RETURN(status);
  return SL_STATUS_OK;
}

sl_status_t sl_si91x_frequency_offset_command_handler(console_args_t *arguments)
{
  sl_status_t status = SL_STATUS_OK;

  sl_si91x_freq_offset_t freq_calib_pkt  = { 0 };
  freq_calib_pkt.frequency_offset_in_khz = (int32_t)GET_COMMAND_ARG(arguments, 0);

  status = sl_si91x_frequency_offset(&freq_calib_pkt);

  VERIFY_STATUS_AND_RETURN(status);
  return SL_STATUS_OK;
}

sl_status_t set_region_configuration_handler(console_args_t *arguments)
{
  sl_status_t status = SL_STATUS_OK;
  static set_region_struct_t set_region_config;
  //set_region_config.operation_mode            = GET_COMMAND_ARG(arguments,0);// GET_OPTIONAL_COMMAND_ARG(arguments, 0, SL_SI91X_CLIENT_MODE, const uint8_t);
  const int mode                = GET_OPTIONAL_COMMAND_ARG(arguments, 0, WIFI_MODE_STA, const int);
  set_region_config.band        = SL_SI91X_WIFI_BAND_2_4GHZ;
  set_region_config.region_code = GET_COMMAND_ARG(arguments, 1);

  switch (mode) {
    case WIFI_MODE_STA:
      set_region_config.operation_mode = SL_SI91X_CLIENT_MODE;
      break;
    case WIFI_MODE_AP:
      set_region_config.operation_mode = SL_SI91X_ACCESS_POINT_MODE;
      break;
    case WIFI_MODE_APSTA:
      set_region_config.operation_mode = SL_SI91X_CONCURRENT_MODE;
      break;
    case WIFI_MODE_EAP:
      set_region_config.operation_mode = SL_SI91X_ENTERPRISE_CLIENT_MODE;
      break;
    case WIFI_TRANSMIT_TEST_MODE:
      set_region_config.operation_mode = SL_SI91X_TRANSMIT_TEST_MODE;
      break;
    case BLE_MODE:
      set_region_config.operation_mode = SL_SI91X_CLIENT_MODE;
      break;
    default:
      printf("Selected Wi-Fi mode is not supported. Try 'help'");
      return SL_STATUS_WIFI_UNKNOWN_INTERFACE;
  }
  status =
    sl_si91x_set_device_region(set_region_config.operation_mode, set_region_config.band, set_region_config.region_code);
  VERIFY_STATUS_AND_RETURN(status);
  return SL_STATUS_OK;
}

sl_status_t wifi_config_pll_mode_handler(console_args_t *arguments)
{
  sl_status_t status          = SL_STATUS_OK;
  sl_wifi_pll_mode_t pll_mode = GET_COMMAND_ARG(arguments, 0);

  printf("Configuring WiFi PLL mode to: %s\n",
         (pll_mode == SL_WIFI_PLL_MODE_20MHZ) ? "PLL_MODE_20MHZ (20MHz)" : "PLL_MODE_40MHZ (40MHz)");

  status = sl_wifi_config_pll_mode(pll_mode);
  VERIFY_STATUS_AND_RETURN(status);

  printf("WiFi PLL mode configured successfully\n");
  return SL_STATUS_OK;
}

sl_status_t wifi_config_power_chain_handler(console_args_t *arguments)
{
  sl_status_t status                = SL_STATUS_OK;
  sl_wifi_power_chain_t power_chain = GET_COMMAND_ARG(arguments, 0);

  printf("Configuring WiFi power chain to: %s\n",
         (power_chain == SL_WIFI_HP_CHAIN) ? "HP_CHAIN (High Power)" : "LP_CHAIN (Low Power)");

  status = sl_wifi_config_power_chain(power_chain);
  VERIFY_STATUS_AND_RETURN(status);

  printf("WiFi power chain configured successfully\n");
  return SL_STATUS_OK;
}