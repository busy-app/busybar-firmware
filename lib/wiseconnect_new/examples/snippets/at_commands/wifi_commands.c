/*******************************************************************************
 * @file  wifi_commands.c
 * @brief Wi-fi AT Commands
 *******************************************************************************
 * # License
 * <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 ******************************************************************************/
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
#include "sl_si91x_hal_soc_soft_reset.h"
#include "sl_wifi_device.h"
#include "sl_wifi_constants.h"
#include "at_utility.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <inttypes.h>

/******************************************************
 *                    Constants
 ******************************************************/

#ifndef SCAN_RESULT_COUNT_EXTENDED_DEFAULT
#define SCAN_RESULT_COUNT_EXTENDED_DEFAULT 40
#endif

#ifndef ADV_SCAN_THRESHOLD
#define ADV_SCAN_THRESHOLD -40
#endif

#ifndef ADV_RSSI_TOLERANCE_THRESHOLD
#define ADV_RSSI_TOLERANCE_THRESHOLD 5
#endif

#ifndef ADV_ACTIVE_SCAN_DURATION
#define ADV_ACTIVE_SCAN_DURATION 15
#endif

#ifndef ADV_PASSIVE_SCAN_DURATION
#define ADV_PASSIVE_SCAN_DURATION 20
#endif

#ifndef ADV_MULTIPROBE
#define ADV_MULTIPROBE 1
#endif

#ifndef ADV_SCAN_PERIODICITY
#define ADV_SCAN_PERIODICITY 10
#endif

#ifndef ADV_INSTANT_SCAN
#define ADV_INSTANT_SCAN 1
#endif

// Rejoin Constants

// If 0 retries infinity times
#ifndef REJOIN_MAX_RETRY
#define REJOIN_MAX_RETRY 5
#endif

// Periodicity of rejoin attempt
#ifndef REJOIN_SCAN_INTERVAL
#define REJOIN_SCAN_INTERVAL 4
#endif

// Beacon missed count
#ifndef REJOIN_BEACON_MISSED_COUNT
#define REJOIN_BEACON_MISSED_COUNT 40
#endif

// 0 - Disable, 1 - Enable, retry for first time join failure
#ifndef REJOIN_FIRST_TIME_RETRY
#define REJOIN_FIRST_TIME_RETRY 0
#endif

// Powersave optimizations enable/disable
#define ENABLE_POWERSAVE_CLI 1

#define WIFI_TEST_IS_VALID_RATE(rate)                                                                                \
  (((rate) == SL_WIFI_DATA_RATE_1) || ((rate) == SL_WIFI_DATA_RATE_2) || ((rate) == SL_WIFI_DATA_RATE_5_5)           \
   || ((rate) == SL_WIFI_DATA_RATE_11) || ((rate) == SL_WIFI_DATA_RATE_6) || ((rate) == SL_WIFI_DATA_RATE_9)         \
   || ((rate) == SL_WIFI_DATA_RATE_12) || ((rate) == SL_WIFI_DATA_RATE_18) || ((rate) == SL_WIFI_DATA_RATE_24)       \
   || ((rate) == SL_WIFI_DATA_RATE_36) || ((rate) == SL_WIFI_DATA_RATE_48) || ((rate) == SL_WIFI_DATA_RATE_54)       \
   || ((rate) == SL_WIFI_DATA_RATE_MCS0) || ((rate) == SL_WIFI_DATA_RATE_MCS1) || ((rate) == SL_WIFI_DATA_RATE_MCS2) \
   || ((rate) == SL_WIFI_DATA_RATE_MCS3) || ((rate) == SL_WIFI_DATA_RATE_MCS4) || ((rate) == SL_WIFI_DATA_RATE_MCS5) \
   || ((rate) == SL_WIFI_DATA_RATE_MCS6) || ((rate) == SL_WIFI_DATA_RATE_MCS7)                                       \
   || ((rate) == SL_WIFI_DATA_RATE_MCS7_SG))

#define WIFI_TEST_IS_VALID_CHANNEL(channel)                                                                           \
  (((channel) >= 1 && (channel) <= 13)                                                                                \
   || ((channel) == 36 || (channel) == 40 || (channel) == 44 || (channel) == 48 || (channel) == 52 || (channel) == 56 \
       || (channel) == 60 || (channel) == 64 || (channel) == 149 || (channel) == 153 || (channel) == 157              \
       || (channel) == 161 || (channel) == 165))

#ifndef WIFI_BAND_DEFAULT
#define WIFI_BAND_DEFAULT SL_WIFI_BAND_2_4GHZ
#endif

#ifndef WIFI_BANDWIDTH_DEFAULT
#define WIFI_BANDWIDTH_DEFAULT SL_WIFI_BANDWIDTH_20MHz
#endif

#define WIFI_IS_VALID_CHANNEL(channel) WIFI_TEST_IS_VALID_CHANNEL(channel)

#define WIFI_IS_WIFI_BAND(band)                                                                  \
  ((band) == SL_WIFI_AUTO_BAND || (band) == SL_WIFI_BAND_900MHZ || (band) == SL_WIFI_BAND_2_4GHZ \
   || (band) == SL_WIFI_BAND_5GHZ || (band) == SL_WIFI_BAND_DUAL || (band) == SL_WIFI_BAND_6GHZ  \
   || (band) == SL_WIFI_BAND_60GHZ)

#define WIFI_IS_WIFIBANDWIDTH(bandwidth)                                               \
  ((bandwidth) == SL_WIFI_AUTO_BANDWIDTH || (bandwidth) == SL_WIFI_BANDWIDTH_10MHz     \
   || (bandwidth) == SL_WIFI_BANDWIDTH_20MHz || (bandwidth) == SL_WIFI_BANDWIDTH_40MHz \
   || (bandwidth) == SL_WIFI_BANDWIDTH_80MHz || (bandwidth) == SL_WIFI_BANDWIDTH_160MHz)

// sl_wifi_interface_t
#ifndef WIFI_NUM_OF_INTERFACE
#define WIFI_NUM_OF_INTERFACE (13)
#endif

/******************************************************
 *               Static Function Declarations
 ******************************************************/
typedef struct {
  sl_wifi_interface_t interface;
  sl_wifi_transmitter_test_info_t test_info;
} at_wifi_tx_test_info_t;

/******************************************************
 *               Extern Variable
 ******************************************************/

/******************************************************
 *               Global Variable
 ******************************************************/

static uint8_t scan_type          = 0;
static uint16_t scan_result_count = SCAN_RESULT_COUNT_EXTENDED_DEFAULT;

volatile bool scan_results_complete  = false;
volatile sl_status_t callback_status = SL_STATUS_OK;

/******************************************************
 *               Variable Definitions
 ******************************************************/

/// Wi-Fi client configuration
static const sl_wifi_device_configuration_t sl_wifi_default_client_configuration_cli = {
  .boot_option = LOAD_NWP_FW,
  .mac_address = NULL,
  .band        = SL_SI91X_WIFI_BAND_2_4GHZ,
  .region_code = WORLD_DOMAIN,
  .boot_config = { .oper_mode = SL_SI91X_CLIENT_MODE,
                   .coex_mode = SL_SI91X_WLAN_ONLY_MODE,
                   .feature_bit_map =
#ifdef SLI_SI91X_MCU_INTERFACE
                     (SL_WIFI_FEAT_SECURITY_OPEN | SL_WIFI_FEAT_WPS_DISABLE | SL_SI91X_FEAT_ULP_GPIO_BASED_HANDSHAKE),
#else
                     (SL_WIFI_FEAT_SECURITY_OPEN | SL_WIFI_FEAT_AGGREGATION),
#endif
                   .tcp_ip_feature_bit_map = (SL_SI91X_TCP_IP_FEAT_DHCPV4_CLIENT | SL_SI91X_TCP_IP_FEAT_DNS_CLIENT
                                              | SL_SI91X_TCP_IP_FEAT_SSL | SL_SI91X_TCP_IP_FEAT_MDNSD
#ifdef SLI_SI91X_ENABLE_IPV6
                                              | SL_SI91X_TCP_IP_FEAT_DHCPV6_CLIENT | SL_SI91X_TCP_IP_FEAT_IPV6
#endif
                                              | SL_SI91X_TCP_IP_FEAT_ICMP | SL_SI91X_TCP_IP_FEAT_HTTP_CLIENT
                                              | SL_SI91X_TCP_IP_FEAT_EXTENSION_VALID | SL_SI91X_TCP_IP_FEAT_OTAF),
                   .custom_feature_bit_map = SL_WIFI_SYSTEM_CUSTOM_FEAT_EXTENSION_VALID,
                   .ext_custom_feature_bit_map =
                     (SL_SI91X_EXT_FEAT_XTAL_CLK | SL_SI91X_EXT_FEAT_UART_SEL_FOR_DEBUG_PRINTS | MEMORY_CONFIG
#if ENABLE_POWERSAVE_CLI
                      | SL_WIFI_SYSTEM_EXT_FEAT_LOW_POWER_MODE
#endif
#if defined(SLI_SI917)
                      | SL_SI91X_EXT_FEAT_FRONT_END_SWITCH_PINS_ULP_GPIO_4_5_0
#endif
                      ),
                   .bt_feature_bit_map = 0,
                   .ext_tcp_ip_feature_bit_map =
                     (SL_SI91X_EXT_TCP_IP_WINDOW_SCALING | SL_SI91X_EXT_TCP_IP_TOTAL_SELECTS(10)
                      | SL_SI91X_EXT_FEAT_HTTP_OTAF_SUPPORT | SL_SI91X_CONFIG_FEAT_EXTENTION_VALID
                      | SL_SI91X_EXT_TCP_IP_SSL_16K_RECORD),
                   .ble_feature_bit_map     = 0,
                   .ble_ext_feature_bit_map = 0,
                   .config_feature_bit_map  = (SL_SI91X_FEAT_SLEEP_GPIO_SEL_BITMAP | SL_WIFI_ENABLE_ENHANCED_MAX_PSP) }
};

/// Wi-Fi AP configuration
static const sl_wifi_device_configuration_t sl_wifi_default_ap_configuration_cli = {
  .boot_option = LOAD_NWP_FW,
  .mac_address = NULL,
  .band        = SL_SI91X_WIFI_BAND_2_4GHZ,
  .region_code = US,
  .boot_config = { .oper_mode       = SL_SI91X_ACCESS_POINT_MODE,
                   .coex_mode       = SL_SI91X_WLAN_ONLY_MODE,
                   .feature_bit_map = SL_WIFI_FEAT_SECURITY_OPEN,
                   .tcp_ip_feature_bit_map =
                     (SL_SI91X_TCP_IP_FEAT_DHCPV4_SERVER | SL_SI91X_TCP_IP_FEAT_EXTENSION_VALID),
                   .custom_feature_bit_map     = SL_WIFI_SYSTEM_CUSTOM_FEAT_EXTENSION_VALID,
                   .ext_custom_feature_bit_map = (SL_SI91X_EXT_FEAT_XTAL_CLK | MEMORY_CONFIG
#if ENABLE_POWERSAVE_CLI
                                                  | SL_WIFI_SYSTEM_EXT_FEAT_LOW_POWER_MODE
#endif
#if defined(SLI_SI917)
                                                  | SL_SI91X_EXT_FEAT_FRONT_END_SWITCH_PINS_ULP_GPIO_4_5_0
#endif
                                                  ),
                   .bt_feature_bit_map         = 0,
                   .ext_tcp_ip_feature_bit_map = SL_SI91X_EXT_TCP_IP_SSL_16K_RECORD,
                   .ble_feature_bit_map        = 0,
                   .ble_ext_feature_bit_map    = 0,
                   .config_feature_bit_map     = 0 }
};

/// Wi-Fi concurrent (AP + STATION) configuration
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
#if defined(SLI_SI917)
                                                  | SL_SI91X_EXT_FEAT_FRONT_END_SWITCH_PINS_ULP_GPIO_4_5_0
#endif
                                                  ),
                   .bt_feature_bit_map = 0,
                   .ext_tcp_ip_feature_bit_map =
                     (SL_SI91X_CONFIG_FEAT_EXTENTION_VALID | SL_SI91X_EXT_TCP_IP_SSL_16K_RECORD),
                   .ble_feature_bit_map     = 0,
                   .ble_ext_feature_bit_map = 0,
                   .config_feature_bit_map  = SL_SI91X_FEAT_SLEEP_GPIO_SEL_BITMAP }
};

/// Wi-Fi transmit configuration
static const sl_wifi_device_configuration_t sl_wifi_default_transmit_test_configuration_cli = {
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
#if defined(SLI_SI917)
                                                  | SL_SI91X_EXT_FEAT_FRONT_END_SWITCH_PINS_ULP_GPIO_4_5_0
#endif
                                                  ),
                   .bt_feature_bit_map         = SL_SI91X_BT_RF_TYPE,
                   .ext_tcp_ip_feature_bit_map = SL_SI91X_CONFIG_FEAT_EXTENTION_VALID,
                   .ble_feature_bit_map        = 0,
                   .ble_ext_feature_bit_map    = 0,
                   .config_feature_bit_map     = SL_SI91X_FEAT_SLEEP_GPIO_SEL_BITMAP }
};

// The oper-mode defaults to 0 (client mode)
sl_wifi_device_configuration_t si91x_init_configuration = sl_wifi_default_client_configuration_cli;

typedef struct {
  sl_wifi_interface_t interface;
  sl_wifi_channel_t channel;
} at_wifi_statistic_t;

static at_wifi_statistic_t wifi_stat = { 0 };

static at_wifi_tx_test_info_t tx_test_info[WIFI_NUM_OF_INTERFACE] = { 0 };

/******************************************************
 *               Function Definitions
 ******************************************************/

static sl_wifi_transmitter_test_info_t *get_wifi_test_info(sl_wifi_interface_t interface)
{
  sl_wifi_transmitter_test_info_t *first_available_slot = NULL;

  for (uint8_t i = 0; i < WIFI_NUM_OF_INTERFACE; ++i) {
    if (tx_test_info[i].interface == interface) {
      return &tx_test_info[i].test_info;
    }

    if ((first_available_slot == NULL) && (tx_test_info[i].interface == SL_WIFI_INVALID_INTERFACE)) {
      first_available_slot      = &tx_test_info[i].test_info;
      tx_test_info[i].interface = interface;
    }
  }

  return first_available_slot;
}

// at+oper-mode=<oper-mode>
sl_status_t set_oper_mode_command_handler(console_args_t *arguments)
{
  CHECK_ARGUMENT_BITMAP(arguments, 0x01);
  // initial checks
  if (sl_si91x_is_device_initialized()) {
    return SL_STATUS_ALREADY_INITIALIZED;
  }

  uint16_t oper_mode = GET_OPTIONAL_COMMAND_ARG(arguments, 0, SL_SI91X_CLIENT_MODE, sl_wifi_operation_mode_t);
  switch (oper_mode) {
    case SL_SI91X_CONCURRENT_MODE:
      si91x_init_configuration = sl_wifi_default_concurrent_configuration_cli;
      break;
    case SL_SI91X_CLIENT_MODE:
      si91x_init_configuration = sl_wifi_default_client_configuration_cli;
      break;
    case SL_SI91X_ACCESS_POINT_MODE:
      si91x_init_configuration = sl_wifi_default_ap_configuration_cli;
      break;
    case SL_SI91X_TRANSMIT_TEST_MODE:
      si91x_init_configuration = sl_wifi_default_transmit_test_configuration_cli;
      break;
    default:
      return SL_STATUS_INVALID_PARAMETER;
  }
  PRINT_AT_CMD_SUCCESS;
  return SL_STATUS_OK;
}

// at+region-code=<region-code>
sl_status_t set_region_code_command_handler(console_args_t *arguments)
{
  CHECK_ARGUMENT_BITMAP(arguments, 0x01);
  // initial checks
  if (sl_si91x_is_device_initialized()) {
    return SL_STATUS_ALREADY_INITIALIZED;
  }

  uint8_t region_code                  = GET_OPTIONAL_COMMAND_ARG(arguments, 0, US, sl_wifi_region_code_t);
  si91x_init_configuration.region_code = region_code;

  PRINT_AT_CMD_SUCCESS;
  return SL_STATUS_OK;
}

// at+enable-ssl=<enable-ssl>
sl_status_t enable_ssl_command_handler(console_args_t *arguments)
{
  CHECK_ARGUMENT_BITMAP(arguments, 0x01);
  // initial checks
  if (sl_si91x_is_device_initialized()) {
    return SL_STATUS_ALREADY_INITIALIZED;
  }

  uint8_t enable_ssl = GET_OPTIONAL_COMMAND_ARG(arguments, 0, 0, uint8_t);

  if (enable_ssl == 1) {
    si91x_init_configuration.boot_config.tcp_ip_feature_bit_map |= SL_SI91X_TCP_IP_FEAT_SSL;
  } else if (enable_ssl == 0) {
    si91x_init_configuration.boot_config.tcp_ip_feature_bit_map &= ~SL_SI91X_TCP_IP_FEAT_SSL;
  } else {
    return SL_STATUS_INVALID_PARAMETER;
  }

  PRINT_AT_CMD_SUCCESS;
  return SL_STATUS_OK;
}

// at+soft-reset
sl_status_t soc_reset_command_handler(console_args_t *arguments)
{
  UNUSED_PARAMETER(arguments);

  PRINT_AT_CMD_SUCCESS;
  sl_si91x_soc_nvic_reset();
  return SL_STATUS_OK;
}

// at+mac-addr=<mac-address>
static sl_mac_address_t wifi_mac_address = { 0 };
sl_status_t set_mac_address_command_handler(console_args_t *arguments)
{
  CHECK_ARGUMENT_BITMAP(arguments, 0x01);
  // initial checks
  if (sl_si91x_is_device_initialized()) {
    return SL_STATUS_ALREADY_INITIALIZED;
  }

  const char *mac_addr = GET_OPTIONAL_COMMAND_ARG(arguments, 0, NULL, const char *);
  if (mac_addr == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  PRINT_AT_CMD_SUCCESS;
  if (si91x_init_configuration.mac_address == NULL) {
    si91x_init_configuration.mac_address = &wifi_mac_address;
  }
  convert_string_to_mac_address(mac_addr, si91x_init_configuration.mac_address);

  return SL_STATUS_OK;
}

static sl_status_t show_scan_results(sl_wifi_scan_result_t *scan_result)
{
  SL_WIFI_ARGS_CHECK_NULL_POINTER(scan_result);
  uint8_t *bssid = NULL;

  AT_PRINTF("at+WIFI_SCAN_RESULTS=%lu,,", scan_result->scan_count);
  if (scan_result->scan_count) {
    for (unsigned int a = 0; a < scan_result->scan_count; ++a) {
      bssid = (uint8_t *)&scan_result->scan_info[a].bssid;
      AT_PRINTF("%u,%u,-%u,%u,%s,",
                scan_result->scan_info[a].rf_channel,
                scan_result->scan_info[a].security_mode,
                scan_result->scan_info[a].rssi_val,
                scan_result->scan_info[a].network_type,
                scan_result->scan_info[a].ssid);
      AT_PRINTF("%02x:%02x:%02x:%02x:%02x:%02x,,", bssid[0], bssid[1], bssid[2], bssid[3], bssid[4], bssid[5]);
    }
  }
  AT_PRINTF("\r\n>\r\n");

  return SL_STATUS_OK;
}

static sl_status_t show_scan_results_extended_scan(uint16_t result_count,
                                                   sl_wifi_extended_scan_result_t *scan_results_p)
{
  SL_WIFI_ARGS_CHECK_NULL_POINTER(scan_results_p);
  uint8_t *bssid = NULL;

  AT_PRINTF("%u,", result_count);
  for (uint32_t a = 0; a < result_count; ++a) {
    bssid = (uint8_t *)&scan_results_p[a].bssid;
    AT_PRINTF("%u,%u,-%u,%u,%s,",
              scan_results_p[a].rf_channel,
              scan_results_p[a].security_mode,
              scan_results_p[a].rssi,
              scan_results_p[a].network_type,
              scan_results_p[a].ssid);
    AT_PRINTF("%02x:%02x:%02x:%02x:%02x:%02x,", bssid[0], bssid[1], bssid[2], bssid[3], bssid[4], bssid[5]);
  }
  AT_PRINTF("\r\n");
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
  sl_status_t status = SL_STATUS_OK;
  if (SL_WIFI_CHECK_IF_EVENT_FAILED(event)) {
    callback_status       = status_code;
    scan_results_complete = true;
    AT_PRINTF("ERROR %" PRIi32 "\r\n>\r\n", (int32_t)callback_status);
    return status_code;
  }

  if ((scan_type != SL_WIFI_SCAN_TYPE_EXTENDED) && (result_length != 0) && !status) {
    callback_status = show_scan_results(result);
  }

  scan_results_complete = true;
  return SL_STATUS_OK;
}

// at+wifi-scan-rslts=<max-results>,<channel-filter>,<security-mode-filter>,
//                    <rssi-filter>,<network-type-filter>
sl_status_t wifi_get_extended_scan_results_command_handler(console_args_t *arguments)
{
  CHECK_ARGUMENT_BITMAP(arguments, 0x01);
  UNUSED_PARAMETER(arguments);

  if (scan_type != SL_WIFI_SCAN_TYPE_EXTENDED) {
    return SL_STATUS_SI91X_COMMAND_GIVEN_IN_INVALID_STATE;
  }
  scan_result_count = GET_OPTIONAL_COMMAND_ARG(arguments, 0, SCAN_RESULT_COUNT_EXTENDED_DEFAULT, uint16_t);
  sl_wifi_extended_scan_result_parameters_t scan_result_parameters = { 0 };
  uint16_t result_count                                            = 0;

  // Allocate memory for scan results
  char *scan_results_p = malloc(sizeof(sl_wifi_extended_scan_result_t) * scan_result_count);
  if (scan_results_p == NULL) {
    return SL_STATUS_ALLOCATION_FAILED;
  }
  scan_result_parameters.scan_results = (sl_wifi_extended_scan_result_t *)scan_results_p;
  scan_result_parameters.array_length = scan_result_count;
  scan_result_parameters.result_count = &result_count;

  uint8_t channel                       = GET_OPTIONAL_COMMAND_ARG(arguments, 1, 0, uint8_t);
  scan_result_parameters.channel_filter = channel ? &channel : NULL;

  uint8_t security_mode                       = GET_OPTIONAL_COMMAND_ARG(arguments, 2, 0, uint8_t);
  scan_result_parameters.security_mode_filter = security_mode ? &security_mode : NULL;

  uint8_t rssi                       = GET_OPTIONAL_COMMAND_ARG(arguments, 3, 0, uint8_t);
  scan_result_parameters.rssi_filter = rssi ? &rssi : NULL;

  uint8_t network_type                       = GET_OPTIONAL_COMMAND_ARG(arguments, 4, 0, uint8_t);
  scan_result_parameters.network_type_filter = network_type ? &network_type : NULL;

  sl_status_t status = sl_wifi_get_stored_scan_results(SL_WIFI_CLIENT_2_4GHZ_INTERFACE, &scan_result_parameters);
  if (status != SL_STATUS_OK) {
    free(scan_results_p);
    return status;
  }

  PRINT_AT_CMD_SUCCESS;
  callback_status = show_scan_results_extended_scan(result_count, (sl_wifi_extended_scan_result_t *)scan_results_p);
  free(scan_results_p);

  return SL_STATUS_OK;
}

// at+WIFI_STATS=<interface>,<tx-pkts>,<reserved-1>,<tx-retries>,<crc-pass>,
//               <crc-fail>,<cca-stuck>,<cca-not-stuck>,<pkt-aborts>,
//               <false-rx-starts>,<cca-idle>,<reserved-2>,<rx-retries>,
//               <reserved-3>,<cal-rssi>,<reserved-4>,<tx-dropped>,
//               <max-cons-pkts-dropped>,<reserved-5>,<bss-brodcast-pkts>,
//               <bss-multicast-pkts>,<bss-filt-match-mult-pkts>
static sl_status_t wifi_stats_receive_handler(sl_wifi_event_t event,
                                              sl_status_t status_code,
                                              void *response,
                                              uint32_t result_length,
                                              void *arg)
{
  UNUSED_PARAMETER(result_length);
  UNUSED_PARAMETER(arg);

  if (SL_WIFI_CHECK_IF_EVENT_FAILED(event)) {
    callback_status = status_code;
    return status_code;
  }

  if (event == SL_WIFI_STATS_ASYNC_EVENT) {
    sl_wifi_async_stats_response_t *result = (sl_wifi_async_stats_response_t *)response;

    AT_PRINTF("at+WIFI_STATS=%d,%d,0,%d,%d,%d,%d,%d,%d,%d,%d,0,%d,0,%d,%d,%d,0,%d,%d",
              wifi_stat.interface,
              result->tx_pkts,
              result->tx_retries,
              result->crc_pass,
              result->crc_fail,
              result->cca_stk,
              result->cca_not_stk,
              result->pkt_abort,
              result->fls_rx_start,
              result->cca_idle,
              result->rx_retries,
              result->cal_rssi,
              result->max_cons_pkts_dropped,
              result->bss_broadcast_pkts,
              result->bss_multicast_pkts,
              result->bss_filter_matched_multicast_pkts);
    AT_PRINTF("\r\n>\r\n");

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

  AT_PRINTF("%ld", signal_strength);
  return status;
}

// at+wifi-start-scan=<interface>,<filter-ssid>,<scan-type>,<scan-flags>,
//                    <periodic-scan-interval>,<channel-bitmap-2g4>,
//                    <channel-bitmap-5g>,<enable-lp-mode>
sl_status_t wifi_scan_command_handler(console_args_t *arguments)
{
  CHECK_ARGUMENT_BITMAP(arguments, 0x1);
  sl_status_t status = SL_STATUS_OK;
  sl_wifi_ssid_t optional_ssid_arg;
  sl_wifi_scan_configuration_t wifi_scan_configuration = { 0 };
  callback_status                                      = status;
  scan_results_complete                                = false;

  sl_wifi_interface_t interface =
    GET_OPTIONAL_COMMAND_ARG(arguments, 0, SL_WIFI_INVALID_INTERFACE, sl_wifi_interface_t);
  if (interface == SL_WIFI_INVALID_INTERFACE) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  char *ssid                 = GET_OPTIONAL_COMMAND_ARG(arguments, 1, NULL, char *);
  scan_type                  = GET_OPTIONAL_COMMAND_ARG(arguments, 2, 0, uint8_t);
  uint16_t channel_bitmap_2g = GET_OPTIONAL_COMMAND_ARG(arguments, 5, 0, const uint16_t);

  if (ssid != NULL) {
    optional_ssid_arg.length = (uint8_t)sl_strnlen(ssid, sizeof(optional_ssid_arg.value));
    memcpy(optional_ssid_arg.value, ssid, optional_ssid_arg.length);
  }

  wifi_scan_configuration      = default_wifi_scan_configuration;
  wifi_scan_configuration.type = scan_type;

  if (scan_type == SL_WIFI_SCAN_TYPE_ADV_SCAN) {
    wifi_scan_configuration.periodic_scan_interval =
      GET_OPTIONAL_COMMAND_ARG(arguments, 4, ADV_SCAN_PERIODICITY, uint32_t);
  }

  wifi_scan_configuration.lp_mode = GET_OPTIONAL_COMMAND_ARG(arguments, 7, false, const bool);

  if (channel_bitmap_2g > 0) {
    interface |= SL_WIFI_2_4GHZ_INTERFACE;
    wifi_scan_configuration.channel_bitmap_2g4 = channel_bitmap_2g;
  }

  sl_wifi_set_scan_callback_v2(scan_callback_handler, NULL);

  status = sl_wifi_start_scan(interface, (ssid != NULL) ? &optional_ssid_arg : NULL, &wifi_scan_configuration);

  if (SL_STATUS_IN_PROGRESS == status) {
    PRINT_AT_CMD_SUCCESS;
    return SL_STATUS_OK;
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

// at+wifi-listen-interval=<wifi-interface>,<listen-interval>,
//                         <listen-interval-multiplier>
sl_status_t set_listen_interval_command_handler(const console_args_t *arguments)
{
  CHECK_ARGUMENT_BITMAP(arguments, 0x03);

  sl_wifi_interface_t interface        = GET_OPTIONAL_COMMAND_ARG(arguments, 0, 0, sl_wifi_interface_t);
  sl_wifi_listen_interval_v2_t profile = {
    .listen_interval            = GET_OPTIONAL_COMMAND_ARG(arguments, 1, 1000, uint32_t),
    .listen_interval_multiplier = GET_OPTIONAL_COMMAND_ARG(arguments, 2, 1, uint32_t),
  };
  sl_status_t status = sl_wifi_set_listen_interval_v2(interface, profile);
  VERIFY_STATUS_AND_RETURN(status);

  PRINT_AT_CMD_SUCCESS;
  return SL_STATUS_OK;
}

// at+wifi-get-listen-interval=<interface>
sl_status_t get_listen_interval_command_handler(const console_args_t *arguments)
{
  CHECK_ARGUMENT_BITMAP(arguments, 0x01);

  sl_wifi_interface_t interface =
    GET_OPTIONAL_COMMAND_ARG(arguments, 0, SL_WIFI_INVALID_INTERFACE, sl_wifi_interface_t);

  if (interface == SL_WIFI_INVALID_INTERFACE) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  sl_wifi_listen_interval_v2_t profile = { 0 };
  sl_status_t status                   = sl_wifi_get_listen_interval_v2(interface, &profile);
  VERIFY_STATUS_AND_RETURN(status);

  PRINT_AT_CMD_SUCCESS;
  AT_PRINTF("%" PRIi32 " %" PRIi32 "\r\n", profile.listen_interval, profile.listen_interval_multiplier);
  return SL_STATUS_OK;
}

// at+wifisys-fwver?
sl_status_t wifi_get_firmware_version_command_handler(console_args_t *arguments)
{
  UNUSED_PARAMETER(arguments);
  sl_wifi_firmware_version_t version = { 0 };

  sl_status_t status = sl_wifi_get_firmware_version(&version);
  VERIFY_STATUS_AND_RETURN(status);

  PRINT_AT_CMD_SUCCESS;
  AT_PRINTF("%x%x.%d.%d.%d.%d.%d.%d\r\n",
            version.chip_id,
            version.rom_id,
            version.major,
            version.minor,
            version.security_version,
            version.patch_num,
            version.customer_id,
            version.build_num);
  return status;
}

// at+wifi-net-info?
sl_status_t wifi_get_network_info_handler(const console_args_t *arguments)
{
  UNUSED_PARAMETER(arguments);

  sl_status_t status                = 0;
  sl_si91x_rsp_wireless_info_t info = { 0 };

  status = sl_wifi_get_wireless_info(&info);
  VERIFY_STATUS_AND_RETURN(status);

  PRINT_AT_CMD_SUCCESS;
  AT_PRINTF("%s %u \"%s\" ", info.wlan_state ? "CONNECTED" : "DISCONNECTED", info.channel_number, info.ssid);

  AT_PRINTF("%2X:%2X:%2X:%2X:%2X:%2X",
            info.mac_address[0],
            info.mac_address[1],
            info.mac_address[2],
            info.mac_address[3],
            info.mac_address[4],
            info.mac_address[5]);

  AT_PRINTF(" %u ", info.sec_type);
  for (uint8_t ii = 0; ii < 32; ii++) {
    AT_PRINTF("%x", info.psk_pmk[ii]);
  }
  AT_PRINTF(" %d.%d.%d.%d", info.ipv4_address[0], info.ipv4_address[1], info.ipv4_address[2], info.ipv4_address[3]);

  char temp_buffer[46] = { 0 };
  sl_inet_ntop6((const unsigned char *)(&info.ipv6_address), (char *)temp_buffer, sizeof(temp_buffer));
  AT_PRINTF(" %s\r\n", temp_buffer);

  return status;
}

// at+wifi-stats-start=<interface>,<channel>,<band>,<bandwidth>
sl_status_t wifi_statistics_start_command_handler(console_args_t *arguments)
{
  CHECK_ARGUMENT_BITMAP(arguments, 0x0F);

  if (!sl_si91x_is_device_initialized()) {
    return SL_STATUS_NOT_INITIALIZED;
  }

  sl_wifi_interface_t interface = GET_OPTIONAL_COMMAND_ARG(arguments, 0, SL_WIFI_CLIENT_INTERFACE, sl_wifi_interface_t);
  uint16_t channel              = GET_OPTIONAL_COMMAND_ARG(arguments, 1, 1, const uint16_t);
  sl_wifi_band_t band           = GET_OPTIONAL_COMMAND_ARG(arguments, 2, WIFI_BAND_DEFAULT, sl_wifi_band_t);
  sl_wifi_bandwidth_t bandwidth = GET_OPTIONAL_COMMAND_ARG(arguments, 3, WIFI_BANDWIDTH_DEFAULT, sl_wifi_bandwidth_t);

  if (!WIFI_IS_VALID_CHANNEL(channel) || !WIFI_IS_WIFI_BAND(band) || !WIFI_IS_WIFIBANDWIDTH(bandwidth)) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  wifi_stat.interface         = interface;
  wifi_stat.channel.channel   = channel;
  wifi_stat.channel.band      = band;
  wifi_stat.channel.bandwidth = bandwidth;

  sl_wifi_set_stats_callback_v2(wifi_stats_receive_handler, NULL);
  sl_status_t status = sl_wifi_start_statistic_report(wifi_stat.interface, wifi_stat.channel);
  if ((SL_STATUS_IN_PROGRESS == status) || (SL_STATUS_OK == status)) {
    PRINT_AT_CMD_SUCCESS;
    return SL_STATUS_OK;
  }

  return status;
}

// at+wifi-stats-stop=<interface>
sl_status_t wifi_statistics_stop_command_handler(console_args_t *arguments)
{
  CHECK_ARGUMENT_BITMAP(arguments, 0x01);

  if (!sl_si91x_is_device_initialized()) {
    return SL_STATUS_NOT_INITIALIZED;
  }

  sl_wifi_interface_t interface =
    GET_OPTIONAL_COMMAND_ARG(arguments, 0, SL_WIFI_INVALID_INTERFACE, sl_wifi_interface_t);

  if (interface == SL_WIFI_INVALID_INTERFACE) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  sl_status_t status = sl_wifi_stop_statistic_report(interface);
  if ((SL_STATUS_IN_PROGRESS == status) || (SL_STATUS_OK == status)) {
    PRINT_AT_CMD_SUCCESS;
    return SL_STATUS_OK;
  }

  return status;
}

// at+wifi-get-mac=<interface>
sl_status_t wifi_get_mac_address_command_handler(console_args_t *arguments)
{
  CHECK_ARGUMENT_BITMAP(arguments, 0x01);

  sl_status_t status        = SL_STATUS_OK;
  sl_mac_address_t mac_addr = { 0 };
  sl_wifi_interface_t interface =
    GET_OPTIONAL_COMMAND_ARG(arguments, 0, SL_WIFI_INVALID_INTERFACE, sl_wifi_interface_t);

  if (interface == SL_WIFI_INVALID_INTERFACE) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  status = sl_wifi_get_mac_address(interface, &mac_addr);
  VERIFY_STATUS_AND_RETURN(status);

  PRINT_AT_CMD_SUCCESS;
  AT_PRINTF("%02X:%02X:%02X:%02X:%02X:%02X",
            mac_addr.octet[0],
            mac_addr.octet[1],
            mac_addr.octet[2],
            mac_addr.octet[3],
            mac_addr.octet[4],
            mac_addr.octet[5]);
  return status;
}

// at+wifi-perf-prof=<profile>,<dtim-aligned-type>,<num-of-dtim-skip>,
//                   <listen-interval>,<monitor-interval>
sl_status_t wifi_set_performance_profile_command_handler(console_args_t *arguments)
{
  CHECK_ARGUMENT_BITMAP(arguments, 0x01);

  sl_wifi_performance_profile_v2_t performance_profile = { 0 };
  performance_profile.profile                          = GET_OPTIONAL_COMMAND_ARG(arguments, 0, 0, uint8_t);
  performance_profile.dtim_aligned_type                = GET_OPTIONAL_COMMAND_ARG(arguments, 1, 0, uint8_t);
  performance_profile.num_of_dtim_skip                 = GET_OPTIONAL_COMMAND_ARG(arguments, 2, 0, uint8_t);
  performance_profile.listen_interval                  = GET_OPTIONAL_COMMAND_ARG(arguments, 3, 0, uint32_t);
  performance_profile.monitor_interval                 = GET_OPTIONAL_COMMAND_ARG(arguments, 4, 0, uint16_t);

  sl_status_t status = sl_wifi_set_performance_profile_v2(&performance_profile);
  VERIFY_STATUS_AND_RETURN(status);

  PRINT_AT_CMD_SUCCESS;
  return status;
}

// at+wifi-perf-prof?
sl_status_t wifi_get_performance_profile_command_handler(console_args_t *arguments)
{
  UNUSED_PARAMETER(arguments);
  sl_status_t status;
  sl_wifi_performance_profile_v2_t performance_profile = { 0 };

  status = sl_wifi_get_performance_profile_v2(&performance_profile);
  VERIFY_STATUS_AND_RETURN(status);

  PRINT_AT_CMD_SUCCESS;
  AT_PRINTF("%d %d %d %ld %d",
            performance_profile.profile,
            performance_profile.dtim_aligned_type,
            performance_profile.num_of_dtim_skip,
            performance_profile.listen_interval,
            performance_profile.monitor_interval);
  return status;
}

// at+wifi-is-ifup=<interface>
sl_status_t wifi_is_interface_up_command_handler(console_args_t *arguments)
{
  CHECK_ARGUMENT_BITMAP(arguments, 0x01);
  sl_wifi_interface_t interface = (sl_wifi_interface_t)GET_COMMAND_ARG(arguments, 0);

  PRINT_AT_CMD_SUCCESS;
  AT_PRINTF("%d", sl_wifi_is_interface_up(interface));

  return SL_STATUS_OK;
}

// at+wifi-scan-advconf=<trigger-rssi-level>,<trigger-rssi-change>,
//                      <active-channel-time>,<passive-channel-time>,
//                      <enable-instant-scan>,<enable-multi-probe>
sl_status_t set_advanced_scan_configuration_command_handler(console_args_t *arguments)
{
  CHECK_ARGUMENT_BITMAP(arguments, 0x3F);
  sl_status_t status                                                = SL_STATUS_OK;
  sl_wifi_advanced_scan_configuration_t advanced_scan_configuration = {
    // Advance scanning parameters
    .trigger_level        = GET_OPTIONAL_COMMAND_ARG(arguments, 0, ADV_SCAN_THRESHOLD, int32_t),
    .trigger_level_change = GET_OPTIONAL_COMMAND_ARG(arguments, 1, ADV_RSSI_TOLERANCE_THRESHOLD, uint32_t),
    .active_channel_time  = GET_OPTIONAL_COMMAND_ARG(arguments, 2, ADV_ACTIVE_SCAN_DURATION, uint16_t),
    .passive_channel_time = GET_OPTIONAL_COMMAND_ARG(arguments, 3, ADV_PASSIVE_SCAN_DURATION, uint16_t),
    .enable_instant_scan  = GET_OPTIONAL_COMMAND_ARG(arguments, 4, ADV_INSTANT_SCAN, uint8_t),
    .enable_multi_probe   = GET_OPTIONAL_COMMAND_ARG(arguments, 5, ADV_MULTIPROBE, uint8_t),
  };

  status = sl_wifi_set_advanced_scan_configuration(&advanced_scan_configuration);
  VERIFY_STATUS_AND_RETURN(status);

  PRINT_AT_CMD_SUCCESS;
  return status;
}

// at+wifi-stop-scan=<interface>
sl_status_t wifi_stop_scan_command_handler(console_args_t *arguments)
{
  CHECK_ARGUMENT_BITMAP(arguments, 0x01);

  sl_wifi_interface_t interface =
    GET_OPTIONAL_COMMAND_ARG(arguments, 0, SL_WIFI_INVALID_INTERFACE, sl_wifi_interface_t);
  if (interface == SL_WIFI_INVALID_INTERFACE) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  sl_status_t status = sl_wifi_stop_scan(interface);
  VERIFY_STATUS_AND_RETURN(status);

  PRINT_AT_CMD_SUCCESS;
  return status;
}

// at+wifi-per-cnf-gen=<interface>,<power>,<rate>,<packet-length>,<mode>,
//                     <channel>,<rate-flags>,<channel-bw>,<aggr-enable>,
//                     <reserved>,<pkt-count>,<delay>
sl_status_t wifi_per_cnf_gen_command_handler(console_args_t *arguments)
{
  CHECK_ARGUMENT_BITMAP(arguments, 0x0FFF);

  if (!sl_si91x_is_device_initialized()) {
    return SL_STATUS_NOT_INITIALIZED;
  }

  sl_wifi_interface_t interface =
    GET_OPTIONAL_COMMAND_ARG(arguments, 0, SL_WIFI_INVALID_INTERFACE, sl_wifi_interface_t);
  uint16_t power       = GET_OPTIONAL_COMMAND_ARG(arguments, 1, 0, uint16_t);
  uint32_t rate        = GET_OPTIONAL_COMMAND_ARG(arguments, 2, 0, uint32_t);
  uint16_t length      = GET_OPTIONAL_COMMAND_ARG(arguments, 3, 0, uint16_t);
  uint16_t mode        = GET_OPTIONAL_COMMAND_ARG(arguments, 4, 0, uint16_t);
  uint16_t channel     = GET_OPTIONAL_COMMAND_ARG(arguments, 5, 0, uint16_t);
  uint16_t rate_flags  = GET_OPTIONAL_COMMAND_ARG(arguments, 6, 0, uint16_t);
  uint16_t channel_bw  = GET_OPTIONAL_COMMAND_ARG(arguments, 7, 0, uint16_t);
  uint16_t aggr_enable = GET_OPTIONAL_COMMAND_ARG(arguments, 8, 0, uint16_t);
  uint16_t reserved    = GET_OPTIONAL_COMMAND_ARG(arguments, 9, 0, uint16_t);
  uint16_t no_of_pkts  = GET_OPTIONAL_COMMAND_ARG(arguments, 10, 0, uint16_t);
  uint32_t delay       = GET_OPTIONAL_COMMAND_ARG(arguments, 11, 0, uint32_t);

  if (interface == SL_WIFI_INVALID_INTERFACE) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  // Validate power parameter (2-18 dBm, or 127 for max power)
  if ((power != 127) && (power < 2 || power > 18)) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  // Validate data rate
  if (!WIFI_TEST_IS_VALID_RATE(rate)) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  // Validate packet length based on mode
  if (mode == SL_WIFI_TEST_BURST_MODE) { // Burst mode
    if (length < 24 || length > 1500) {
      return SL_STATUS_INVALID_PARAMETER;
    }
  } else if (mode == SL_WIFI_TEST_CONTINOUS_MODE) { // Continuous mode
    if (length < 24 || length > 260) {
      return SL_STATUS_INVALID_PARAMETER;
    }
  } else {
    // MISRA
  }

  // Validate mode parameter
  if (mode > SL_WIFI_TEST_CONTINOUS_WAVE_MODE_OFF_CENTER_HIGH) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  // Validate channel for 2.4GHz (1-13) and 5GHz (36,40,44,48,52,56,60,64,149,153,157,161,165)
  if (!WIFI_TEST_IS_VALID_CHANNEL(channel)) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  sl_wifi_transmitter_test_info_t *test_info = get_wifi_test_info(interface);
  if (test_info == NULL) {
    return SL_STATUS_NO_MORE_RESOURCE;
  }

  // Store parameters to test_info
  test_info->power       = power;
  test_info->rate        = rate;
  test_info->length      = length;
  test_info->mode        = mode;
  test_info->channel     = channel;
  test_info->rate_flags  = rate_flags;
  test_info->channel_bw  = channel_bw;
  test_info->aggr_enable = aggr_enable;
  test_info->reserved    = reserved;
  test_info->no_of_pkts  = no_of_pkts;
  test_info->delay       = delay;

  PRINT_AT_CMD_SUCCESS;
  return SL_STATUS_OK;
}

// at+wifi-per-cnf-axppdu=<interface>,<enable-11ax>,<he-ppdu-type>,<ul-dl>,<bw>,
//                        <coding-type>,<spatial-reuse>,<nominal-pe>,<bss-color>
sl_status_t wifi_per_cnf_axppdu_command_handler(console_args_t *arguments)
{
  CHECK_ARGUMENT_BITMAP(arguments, 0x1FF);

  if (!sl_si91x_is_device_initialized()) {
    return SL_STATUS_NOT_INITIALIZED;
  }

  sl_wifi_interface_t interface =
    GET_OPTIONAL_COMMAND_ARG(arguments, 0, SL_WIFI_INVALID_INTERFACE, sl_wifi_interface_t);
  uint8_t enable_11ax   = GET_OPTIONAL_COMMAND_ARG(arguments, 1, 0, uint8_t);
  uint8_t he_ppdu_type  = GET_OPTIONAL_COMMAND_ARG(arguments, 2, 0, uint8_t);
  uint8_t ul_dl         = GET_OPTIONAL_COMMAND_ARG(arguments, 3, 0, uint8_t);
  uint8_t bw            = GET_OPTIONAL_COMMAND_ARG(arguments, 4, 0, uint8_t);
  uint8_t coding_type   = GET_OPTIONAL_COMMAND_ARG(arguments, 5, 0, uint8_t);
  uint8_t spatial_reuse = GET_OPTIONAL_COMMAND_ARG(arguments, 6, 0, uint8_t);
  uint8_t nominal_pe    = GET_OPTIONAL_COMMAND_ARG(arguments, 7, 0, uint8_t);
  uint8_t bss_color     = GET_OPTIONAL_COMMAND_ARG(arguments, 8, 0, uint8_t);

  // Validate enable_11ax: 0-disable, 1-enable
  // Validate he_ppdu_type: 0-HE SU PPDU, 1-HE ER SU PPDU, 2-HE TB PPDU, 3-HE MU PPDU
  // Validate ul_dl: Set it to 1 if PPDU is to be sent by station to AP; 0 if PPDU is to be sent by AP to station
  // Validate bw: 0 for 242-tone RU, 1 for upper 106-tone RU
  // Validate conding_type: 0-BCC 1-LDPC
  // Validate spatial_reuse: range 0-15
  // Validate nominal_pe: 0-0Us 1-8Us 2-16Us
  // Validate bss_color: range 0-63
  if ((interface == SL_WIFI_INVALID_INTERFACE) || (enable_11ax > 1) || (he_ppdu_type > 3) || (ul_dl > 1) || (bw > 1)
      || (coding_type > 1) || (spatial_reuse > 15) || (nominal_pe > 2) || (bss_color > 63)) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  sl_wifi_transmitter_test_info_t *test_info = get_wifi_test_info(interface);
  if (test_info == NULL) {
    return SL_STATUS_NO_MORE_RESOURCE;
  }

  // Store parameters to test_info
  test_info->enable_11ax   = enable_11ax;
  test_info->he_ppdu_type  = he_ppdu_type;
  test_info->ul_dl         = ul_dl;
  test_info->bw            = bw;
  test_info->coding_type   = coding_type;
  test_info->spatial_reuse = spatial_reuse;
  test_info->nominal_pe    = nominal_pe;
  test_info->bss_color     = bss_color;

  PRINT_AT_CMD_SUCCESS;
  return SL_STATUS_OK;
}

// at+wifi-per-cnf-axsiga=<interface>,<gi-ltf>,<num-he-ltf>,<beam-change>,<dcm>,
//                        <he-siga2-reserved>,<nsts-midamble>,<ru-allocation>,
//                        <stbc>,<tx-bf>
sl_status_t wifi_per_cnf_axsiga_command_handler(console_args_t *arguments)
{
  CHECK_ARGUMENT_BITMAP(arguments, 0x03FF);

  if (!sl_si91x_is_device_initialized()) {
    return SL_STATUS_NOT_INITIALIZED;
  }

  sl_wifi_interface_t interface =
    GET_OPTIONAL_COMMAND_ARG(arguments, 0, SL_WIFI_INVALID_INTERFACE, sl_wifi_interface_t);
  uint8_t gi_ltf             = GET_OPTIONAL_COMMAND_ARG(arguments, 1, 0, uint8_t);
  uint8_t num_he_ltf         = GET_OPTIONAL_COMMAND_ARG(arguments, 2, 0, uint8_t);
  uint8_t beam_change        = GET_OPTIONAL_COMMAND_ARG(arguments, 3, 0, uint8_t);
  uint8_t dcm                = GET_OPTIONAL_COMMAND_ARG(arguments, 4, 0, uint8_t);
  uint16_t he_siga2_reserved = GET_OPTIONAL_COMMAND_ARG(arguments, 5, 0, uint16_t);
  uint8_t nsts_midamble      = GET_OPTIONAL_COMMAND_ARG(arguments, 6, 0, uint8_t);
  uint8_t ru_allocation      = GET_OPTIONAL_COMMAND_ARG(arguments, 7, 0, uint8_t);
  uint8_t stbc               = GET_OPTIONAL_COMMAND_ARG(arguments, 8, 0, uint8_t);
  uint8_t tx_bf              = GET_OPTIONAL_COMMAND_ARG(arguments, 9, 0, uint8_t);

  // Validate interface
  // Validate gi_ltf: range 0-3
  // Validate num_he_ltf: range 0-7
  // Validate beam_change:
  // * 0 for pre-HE and HE fields are spatially mapped in the same way
  // * 1 for pre-HE and HE fields are spatially mapped differently.
  // Validate dcm: 0 - No DCM, 1 - DCM
  // Validate he_siga2_reserved: range 0-511
  // Validate nsts_midamble: range 0-7
  // Validate ru_allocation: range 0-255
  // Validate stbc: 0 for no STBC and 1 for STBC (only if DCM field is set to 0)
  // Validate tx_bf: 0 - no beamforming matrix, 1 - beamforming matrix.

  if (((interface == SL_WIFI_INVALID_INTERFACE)) || (gi_ltf > 3) || (num_he_ltf > 7) || (beam_change > 1) || (dcm > 1)
      || (he_siga2_reserved > 511) || (nsts_midamble > 7) || (stbc > 1) || (tx_bf > 1)) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  if ((stbc == 1) && (dcm == 1)) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  sl_wifi_transmitter_test_info_t *test_info = get_wifi_test_info(interface);
  if (test_info == NULL) {
    return SL_STATUS_NO_MORE_RESOURCE;
  }

  // Store parameters to test_info
  test_info->gi_ltf            = gi_ltf;
  test_info->n_heltf_tot       = num_he_ltf;
  test_info->beam_change       = beam_change;
  test_info->dcm               = dcm;
  test_info->he_siga2_reserved = he_siga2_reserved;
  test_info->nsts_midamble     = nsts_midamble;
  test_info->ru_allocation     = ru_allocation;
  test_info->stbc              = stbc;
  test_info->tx_bf             = tx_bf;

  PRINT_AT_CMD_SUCCESS;
  return SL_STATUS_OK;
}

// at+wifi-per-cnf-axsigb=<interface>,<sigb-compression-field>,<sigb-dcm>,<sigb-mcs>,
//                        <user-idx>,<user-sta-id>
sl_status_t wifi_per_cnf_axsigb_command_handler(console_args_t *arguments)
{
  CHECK_ARGUMENT_BITMAP(arguments, 0x03F);

  if (!sl_si91x_is_device_initialized()) {
    return SL_STATUS_NOT_INITIALIZED;
  }

  sl_wifi_interface_t interface =
    GET_OPTIONAL_COMMAND_ARG(arguments, 0, SL_WIFI_INVALID_INTERFACE, sl_wifi_interface_t);
  uint8_t sigb_compression_field = GET_OPTIONAL_COMMAND_ARG(arguments, 1, 0, uint8_t);
  uint8_t sigb_dcm               = GET_OPTIONAL_COMMAND_ARG(arguments, 2, 0, uint8_t);
  uint8_t sigb_mcs               = GET_OPTIONAL_COMMAND_ARG(arguments, 3, 0, uint8_t);
  uint8_t user_idx               = GET_OPTIONAL_COMMAND_ARG(arguments, 4, 0, uint8_t);
  uint16_t user_sta_id           = GET_OPTIONAL_COMMAND_ARG(arguments, 5, 0, uint16_t);

  // Validate interface
  // Validate sigb_compression_field: 0/1
  // Validate sigb_dcm: 0-disable, 1-enable
  // Validate sigb_mcs: range 0-15
  // Validate user_idx: range 0-8
  // Validate user_sta_id: range 0-2047
  if (((interface == SL_WIFI_INVALID_INTERFACE)) || (sigb_compression_field > 1) || (sigb_dcm > 1) || (sigb_mcs > 15)
      || (user_idx > 8) || (user_sta_id > 2047)) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  sl_wifi_transmitter_test_info_t *test_info = get_wifi_test_info(interface);
  if (test_info == NULL) {
    return SL_STATUS_NO_MORE_RESOURCE;
  }

  // Store parameters to test_info
  test_info->sigb_compression_field = sigb_compression_field;
  test_info->sigb_dcm               = sigb_dcm;
  test_info->sigb_mcs               = sigb_mcs;
  test_info->user_idx               = user_idx;
  test_info->user_sta_id            = user_sta_id;

  PRINT_AT_CMD_SUCCESS;
  return SL_STATUS_OK;
}

// at+wifi-per-start=<interface>,<enable>
sl_status_t wifi_per_start_command_handler(console_args_t *arguments)
{
  CHECK_ARGUMENT_BITMAP(arguments, 0x03);

  if (!sl_si91x_is_device_initialized()) {
    return SL_STATUS_NOT_INITIALIZED;
  }

  sl_wifi_interface_t interface =
    GET_OPTIONAL_COMMAND_ARG(arguments, 0, SL_WIFI_INVALID_INTERFACE, sl_wifi_interface_t);
  uint8_t enable = GET_OPTIONAL_COMMAND_ARG(arguments, 1, 0, uint8_t);

  // Validate interface
  // Validate enable parameter (0 or 1)
  if ((interface == SL_WIFI_INVALID_INTERFACE) || (enable > 1)) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  sl_wifi_transmitter_test_info_t *test_info = get_wifi_test_info(interface);
  if (test_info == NULL) {
    return SL_STATUS_NO_MORE_RESOURCE;
  }

  // Store parameters to test_info
  test_info->enable = enable;

  sl_status_t status = sl_wifi_transmit_test_start(interface, test_info);
  VERIFY_STATUS_AND_RETURN(status);

  PRINT_AT_CMD_SUCCESS;
  return SL_STATUS_OK;
}

// at+wifi-per-stop=<interface>
sl_status_t wifi_per_stop_command_handler(console_args_t *arguments)
{
  CHECK_ARGUMENT_BITMAP(arguments, 0x01);

  if (!sl_si91x_is_device_initialized()) {
    return SL_STATUS_NOT_INITIALIZED;
  }

  sl_wifi_interface_t interface =
    GET_OPTIONAL_COMMAND_ARG(arguments, 0, SL_WIFI_INVALID_INTERFACE, sl_wifi_interface_t);

  if (interface == SL_WIFI_INVALID_INTERFACE) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  sl_status_t status = sl_wifi_transmit_test_stop(interface);
  VERIFY_STATUS_AND_RETURN(status);

  PRINT_AT_CMD_SUCCESS;
  return SL_STATUS_OK;
}

void wifi_set_default_configuration(void)
{
  si91x_init_configuration = sl_wifi_default_client_configuration_cli;
}
