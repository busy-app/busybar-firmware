/*******************************************************************************
 * @file  ble_commands.c
 * @brief BLE AT Commands
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

#include "sl_board_configuration.h"
#include "sl_rsi_utility.h"
#include "sl_utility.h"
#include "console.h"
#include "sl_net.h"
#include "at_utility.h"
#include "at_command_data_mode.h"
#include "ble_config.h"
#include "rsi_ble.h"
#include "rsi_ble_apis.h"
#include "rsi_ble_common_config.h"
#include "rsi_bt_common.h"
#include "rsi_bt_common_apis.h"
#include <string.h>
#include <inttypes.h>

/******************************************************
 *                    Constants
 ******************************************************/

// Macros for BLE scanning
#define BLE_SCAN_IS_STATUS(status) (((status) == 0) || ((status) == 1))

#define BLE_SCAN_IS_SCAN_TYPE(scan_type) (((scan_type) == SCAN_TYPE_ACTIVE) || ((scan_type) == SCAN_TYPE_PASSIVE))

#define BLE_SCAN_IS_FILTER_TYPE(filter_type) \
  (((filter_type) == SCAN_FILTER_TYPE_ONLY_ACCEPT_LIST) || ((filter_type) == SCAN_FILTER_TYPE_ALL))

#define BLE_SCAN_IS_OWN_ADDR_TYPE(addr_type)                                \
  (((addr_type) == LE_PUBLIC_ADDRESS) || ((addr_type) == LE_RANDOM_ADDRESS) \
   || ((addr_type) == LE_RESOLVABLE_PUBLIC_ADDRESS) || ((addr_type) == LE_RESOLVABLE_RANDOM_ADDRESS))

#define BLE_SCAN_IS_SCAN_INT(scan_int) (((scan_int) >= 0x0004) && ((scan_int) <= 0x4000))

#define BLE_SCAN_IS_SCAN_WIN(scan_win) (((scan_win) >= 0x0004) && ((scan_win) <= 0x4000))

// Macros for BLE advertising
#define BLE_ADV_IS_STATUS(status) (((status) == 0) || ((status) == 1))

#define BLE_ADV_IS_ADV_TYPE(adv_type)                                                   \
  (((adv_type) == UNDIR_CONN) || ((adv_type) == DIR_CONN) || ((adv_type) == UNDIR_SCAN) \
   || ((adv_type) == UNDIR_NON_CONN) || ((adv_type) == DIR_CONN_LOW_DUTY_CYCLE))

#define BLE_ADV_IS_FILTER_TYPE(filter_type)                                                                         \
  (((filter_type) == ALLOW_SCAN_REQ_ANY_CONN_REQ_ANY) || ((filter_type) == ALLOW_SCAN_REQ_ACCEPT_LIST_CONN_REQ_ANY) \
   || ((filter_type) == ALLOW_SCAN_REQ_ANY_CONN_REQ_ACCEPT_LIST)                                                    \
   || ((filter_type) == ALLOW_SCAN_REQ_ACCEPT_LIST_CONN_REQ_ACCEPT_LIST))

#define BLE_ADV_IS_DIRECT_ADDR_TYPE(direct_addr_type)                                     \
  (((direct_addr_type) == LE_PUBLIC_ADDRESS) || ((direct_addr_type) == LE_RANDOM_ADDRESS) \
   || ((direct_addr_type) == LE_RESOLVABLE_PUBLIC_ADDRESS) || ((direct_addr_type) == LE_RESOLVABLE_RANDOM_ADDRESS))

#define BLE_ADV_IS_ADV_INT_MIN(adv_int_min) (((adv_int_min) >= 0x0020) && ((adv_int_min) <= 0x4000))

#define BLE_ADV_IS_ADV_INT_MAX(adv_int_max) (((adv_int_max) >= 0x0020) && ((adv_int_max) <= 0x4000))

#define BLE_ADV_IS_OWN_ADDR_TYPE(own_addr_type)                                     \
  (((own_addr_type) == LE_PUBLIC_ADDRESS) || ((own_addr_type) == LE_RANDOM_ADDRESS) \
   || ((own_addr_type) == LE_RESOLVABLE_PUBLIC_ADDRESS) || ((own_addr_type) == LE_RESOLVABLE_RANDOM_ADDRESS))

#define BLE_ADV_IS_ADV_CHANNEL_MAP(adv_channel_map) \
  (((adv_channel_map) == 0x01) || ((adv_channel_map) == 0x03) || ((adv_channel_map) == 0x07))

// Verify RSI status and return
#define VERIFY_RSI_STATUS_AND_RETURN(err) \
  do {                                    \
    if (err != RSI_SUCCESS) {             \
      return (sl_status_t)err;            \
    }                                     \
  } while (0)

/******************************************************
 *               Variable Definitions
 ******************************************************/

static uint8_t adv_type = 0;

// BLE scanning configuration
static bool scan_configured        = false;
static rsi_ble_req_scan_t ble_scan = { 0 };

// BLE advertising configuration
static bool adv_configured       = false;
static rsi_ble_req_adv_t ble_adv = { 0 };

// BLE advertising data
// The maximum length of advertising data payload is 31 bytes.
static bool adv_data_configured                  = false;
static uint32_t adv_data_len                     = 0;
static uint8_t adv_data[RSI_MAX_ADV_REPORT_SIZE] = { 0 };

extern sl_wifi_device_configuration_t si91x_init_configuration;

static sl_status_t ble_adv_data_send_buffer_handler(uint8_t *buffer, uint32_t length, void *user_data)
{
  UNUSED_PARAMETER(user_data);

  if (length > RSI_MAX_ADV_REPORT_SIZE) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  adv_data_len = length;
  memcpy(adv_data, buffer, length);

  adv_data_configured = true;

  int32_t status = rsi_ble_set_advertise_data(adv_data, adv_data_len);
  VERIFY_RSI_STATUS_AND_RETURN(status);

  PRINT_AT_CMD_SUCCESS;
  return SL_STATUS_OK;
}

static void rsi_ble_on_adv_report_event(rsi_ble_event_adv_report_t *adv_report)
{
  if (adv_report == NULL) {
    return;
  }

  AT_PRINTF("at+91X_BLE_ADV_REPORT=%d,%02X:%02X:%02X:%02X:%02X:%02X,%d,%d,%d,",
            adv_report->dev_addr_type,
            adv_report->dev_addr[5],
            adv_report->dev_addr[4],
            adv_report->dev_addr[3],
            adv_report->dev_addr[2],
            adv_report->dev_addr[1],
            adv_report->dev_addr[0],
            adv_report->rssi,
            adv_report->report_type,
            adv_report->adv_data_len);
  at_print_char_buffer((char *)adv_report->adv_data, adv_report->adv_data_len);
  AT_PRINTF("\r\n>\r\n");
}

static sl_status_t ble_add_acc_type_buffer_handler(uint8_t *buffer, uint32_t length, void *user_data)
{
  uint8_t ad_type = *((uint8_t *)user_data);

  int32_t status = rsi_ble_acceptlist_on_type(1, ad_type, (uint8_t)length, buffer);
  VERIFY_RSI_STATUS_AND_RETURN(status);

  PRINT_AT_CMD_SUCCESS;
  return SL_STATUS_OK;
}

// at+coex-mode=<coex-mode>
sl_status_t coex_mode_command_handler(console_args_t *arguments)
{
  CHECK_ARGUMENT_BITMAP(arguments, 0x01);

  if (sl_si91x_is_device_initialized()) {
    return SL_STATUS_ALREADY_INITIALIZED;
  }

  uint16_t coex_mode = GET_OPTIONAL_COMMAND_ARG(arguments, 0, SL_SI91X_WLAN_BLE_MODE, uint16_t);

  si91x_init_configuration.boot_config.coex_mode = coex_mode;

  if (coex_mode == SL_WIFI_SYSTEM_WLAN_BLE_MODE) {
#if (RSI_SEL_ANTENNA == RSI_SEL_INTERNAL_ANTENNA)
    si91x_init_configuration.boot_config.bt_feature_bit_map |= SL_SI91X_ENABLE_BLE_PROTOCOL | SL_SI91X_BT_RF_TYPE;
#else
    si91x_init_configuration.boot_config.bt_feature_bit_map |= SL_SI91X_ENABLE_BLE_PROTOCOL;
#endif
    si91x_init_configuration.boot_config.ext_custom_feature_bit_map |= SL_SI91X_EXT_FEAT_BT_CUSTOM_FEAT_ENABLE;
    si91x_init_configuration.boot_config.ble_feature_bit_map |=
      (SL_SI91X_BLE_MAX_NBR_PERIPHERALS(RSI_BLE_MAX_NBR_PERIPHERALS)
       | SL_SI91X_BLE_MAX_NBR_CENTRALS(RSI_BLE_MAX_NBR_CENTRALS)
       | SL_SI91X_BLE_MAX_NBR_ATT_SERV(RSI_BLE_MAX_NBR_ATT_SERV) | SL_SI91X_BLE_MAX_NBR_ATT_REC(RSI_BLE_MAX_NBR_ATT_REC)
       | SL_SI91X_FEAT_BLE_CUSTOM_FEAT_EXTENTION_VALID | SL_SI91X_BLE_PWR_INX(RSI_BLE_PWR_INX)
       | SL_SI91X_BLE_PWR_SAVE_OPTIONS(RSI_BLE_PWR_SAVE_OPTIONS) | SL_SI91X_916_BLE_COMPATIBLE_FEAT_ENABLE
#if RSI_BLE_GATT_ASYNC_ENABLE
       | SL_SI91X_BLE_GATT_ASYNC_ENABLE
#endif
      );
    si91x_init_configuration.boot_config.ble_ext_feature_bit_map |=
      (SL_SI91X_BLE_NUM_CONN_EVENTS(RSI_BLE_NUM_CONN_EVENTS) | SL_SI91X_BLE_NUM_REC_BYTES(RSI_BLE_NUM_REC_BYTES)
#if RSI_BLE_INDICATE_CONFIRMATION_FROM_HOST
       | SL_SI91X_BLE_INDICATE_CONFIRMATION_FROM_HOST
#endif
#if RSI_BLE_MTU_EXCHANGE_FROM_HOST
       | SL_SI91X_BLE_MTU_EXCHANGE_FROM_HOST
#endif
#if RSI_BLE_SET_SCAN_RESP_DATA_FROM_HOST
       | SL_SI91X_BLE_SET_SCAN_RESP_DATA_FROM_HOST
#endif
#if RSI_BLE_DISABLE_CODED_PHY_FROM_HOST
       | SL_SI91X_BLE_DISABLE_CODED_PHY_FROM_HOST
#endif
#if BLE_SIMPLE_GATT
       | SL_SI91X_BLE_GATT_INIT
#endif
#if RSI_BLE_ENABLE_ADV_EXTN
       | SL_SI91X_BLE_ENABLE_ADV_EXTN
#endif
#if RSI_BLE_AE_MAX_ADV_SETS
       | SL_SI91X_BLE_AE_MAX_ADV_SETS(RSI_BLE_AE_MAX_ADV_SETS)
#endif
      );
  }

  PRINT_AT_CMD_SUCCESS;
  return SL_STATUS_OK;
}

// at+net-info=<interface>
sl_status_t net_info_command_handler(console_args_t *arguments)
{
  CHECK_ARGUMENT_BITMAP(arguments, 0x01);

  if (!sl_si91x_is_device_initialized()) {
    return SL_STATUS_NOT_INITIALIZED;
  }

  sl_net_interface_info_t info;
  sl_net_interface_t interface =
    GET_OPTIONAL_COMMAND_ARG(arguments, 0, SL_NET_WIFI_CLIENT_INTERFACE, sl_net_interface_t);

  sl_status_t status = sl_net_get_interface_info(interface, &info);
  VERIFY_STATUS_AND_RETURN(status);

  char temp_buffer[46] = { 0 };
  sl_inet_ntop6((const unsigned char *)(&info.ipv6_address), (char *)temp_buffer, sizeof(temp_buffer));

  PRINT_AT_CMD_SUCCESS;

  // <ipv4-address> <ipv6-address> <wlan-state> <channel-number> <ssid>
  // <sec-type> <psk-pmk> <bssid> <wireless-mode> <mac-address>

  // Print IPv4 address
  AT_PRINTF("%d.%d.%d.%d ",
            info.ipv4_address.bytes[0],
            info.ipv4_address.bytes[1],
            info.ipv4_address.bytes[2],
            info.ipv4_address.bytes[3]);

  // Print IPv6 address
  AT_PRINTF("%s ", temp_buffer);

  // Print WLAN state and channel number
  AT_PRINTF("%d %d ", info.hw_info.wifi_info.wlan_state, info.hw_info.wifi_info.channel_number);

  // Print SSID, security type
  AT_PRINTF("%s %d ", info.hw_info.wifi_info.ssid, info.hw_info.wifi_info.sec_type);

  // Print PSK/PMK
  for (uint8_t i = 0; i < SL_WIFI_MAX_PSK_LENGTH; ++i) {
    AT_PRINTF("%02X", info.hw_info.wifi_info.psk_pmk[i]);
  }

  // Print BSSID
  AT_PRINTF(" %02X:%02X:%02X:%02X:%02X:%02X ",
            info.hw_info.wifi_info.bssid[0],
            info.hw_info.wifi_info.bssid[1],
            info.hw_info.wifi_info.bssid[2],
            info.hw_info.wifi_info.bssid[3],
            info.hw_info.wifi_info.bssid[4],
            info.hw_info.wifi_info.bssid[5]);

  // Print wireless mode
  AT_PRINTF("%d ", info.hw_info.wifi_info.wireless_mode);

  // Print MAC address
  AT_PRINTF("%02X:%02X:%02X:%02X:%02X:%02X\r\n",
            info.hw_info.wifi_info.mac_address[0],
            info.hw_info.wifi_info.mac_address[1],
            info.hw_info.wifi_info.mac_address[2],
            info.hw_info.wifi_info.mac_address[3],
            info.hw_info.wifi_info.mac_address[4],
            info.hw_info.wifi_info.mac_address[5]);

  return SL_STATUS_OK;
}

// at+ble-enable-gatt=<enable-gatt>
sl_status_t ble_enable_gatt_command_handler(console_args_t *arguments)
{
  CHECK_ARGUMENT_BITMAP(arguments, 0x01);

  if (sl_si91x_is_device_initialized()) {
    return SL_STATUS_ALREADY_INITIALIZED;
  }

  uint8_t enable_gatt = GET_OPTIONAL_COMMAND_ARG(arguments, 0, 0, uint8_t);

  if (enable_gatt == 1) {
    si91x_init_configuration.boot_config.ble_ext_feature_bit_map |= SL_SI91X_BLE_GATT_INIT;
  } else if (enable_gatt == 0) {
    si91x_init_configuration.boot_config.ble_ext_feature_bit_map &= ~SL_SI91X_BLE_GATT_INIT;
  } else {
    return SL_STATUS_INVALID_PARAMETER;
  }

  PRINT_AT_CMD_SUCCESS;
  return SL_STATUS_OK;
}

// at+ble-max-per=<max-peripherals>
sl_status_t ble_max_per_command_handler(console_args_t *arguments)
{
  CHECK_ARGUMENT_BITMAP(arguments, 0x01);

  if (sl_si91x_is_device_initialized()) {
    return SL_STATUS_ALREADY_INITIALIZED;
  }

  uint8_t max_peripherals = GET_OPTIONAL_COMMAND_ARG(arguments, 0, 0, uint8_t);

  // @note Maximum number of BLE peripherals is 8
  if (max_peripherals > 8) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  // @note Bits 12-15 are used to set MAX_NBR_PERIPHERALS.
  si91x_init_configuration.boot_config.ble_feature_bit_map &= ~SL_SI91X_BLE_MAX_NBR_PERIPHERALS(0x0F);
  si91x_init_configuration.boot_config.ble_feature_bit_map |= SL_SI91X_BLE_MAX_NBR_PERIPHERALS(max_peripherals);

  PRINT_AT_CMD_SUCCESS;
  return SL_STATUS_OK;
}

// at+ble-max-cent=<max-centrals>
sl_status_t ble_max_cent_command_handler(console_args_t *arguments)
{
  CHECK_ARGUMENT_BITMAP(arguments, 0x01);

  if (sl_si91x_is_device_initialized()) {
    return SL_STATUS_ALREADY_INITIALIZED;
  }

  uint8_t max_centrals = GET_OPTIONAL_COMMAND_ARG(arguments, 0, 0, uint8_t);

  // @note Maximum number of BLE Centrals is 2
  if (max_centrals > 2) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  // @note Bits 27-28 are used to set MAX_NBR_CENTRALS.
  si91x_init_configuration.boot_config.ble_feature_bit_map &= ~SL_SI91X_BLE_MAX_NBR_CENTRALS(0x03);
  si91x_init_configuration.boot_config.ble_feature_bit_map |= SL_SI91X_BLE_MAX_NBR_CENTRALS(max_centrals);

  PRINT_AT_CMD_SUCCESS;
  return SL_STATUS_OK;
}

// at+ble-max-gatt-srv=<max-gatt-services>
sl_status_t ble_max_gatt_srv_command_handler(console_args_t *arguments)
{
  CHECK_ARGUMENT_BITMAP(arguments, 0x01);

  if (sl_si91x_is_device_initialized()) {
    return SL_STATUS_ALREADY_INITIALIZED;
  }

  uint8_t max_gatt_services = GET_OPTIONAL_COMMAND_ARG(arguments, 0, 0, uint8_t);

  // @note Maximum number of services is 10
  if (max_gatt_services > 10) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  // @note Bits 8-11 are used to set MAX_NBR_ATT_SERV.
  si91x_init_configuration.boot_config.ble_feature_bit_map &= ~SL_SI91X_BLE_MAX_NBR_ATT_SERV(0x0F);
  si91x_init_configuration.boot_config.ble_feature_bit_map |= SL_SI91X_BLE_MAX_NBR_ATT_SERV(max_gatt_services);

  PRINT_AT_CMD_SUCCESS;
  return SL_STATUS_OK;
}

// at+ble-max-attr=<max-attributes>
sl_status_t ble_max_attr_command_handler(console_args_t *arguments)
{
  CHECK_ARGUMENT_BITMAP(arguments, 0x01);

  if (sl_si91x_is_device_initialized()) {
    return SL_STATUS_ALREADY_INITIALIZED;
  }

  uint8_t max_attributes = GET_OPTIONAL_COMMAND_ARG(arguments, 0, 0, uint8_t);

  // @note Maximum number of BLE attributes is 124
  if (max_attributes > 124) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  // @note Bits 0 - 7 are used to set MAX_NBR_ATT_REC.
  si91x_init_configuration.boot_config.ble_feature_bit_map &= ~SL_SI91X_BLE_MAX_NBR_ATT_REC(0xFF);
  si91x_init_configuration.boot_config.ble_feature_bit_map |= SL_SI91X_BLE_MAX_NBR_ATT_REC(max_attributes);

  PRINT_AT_CMD_SUCCESS;
  return SL_STATUS_OK;
}

// at+91x-ble-scan-conf=<status>,<scan-type>,<enable-acceptlist>,
//                      <own-addr-type>,<scan-interval>,<scan-window>
sl_status_t ble_scan_conf_command_handler(console_args_t *arguments)
{
  CHECK_ARGUMENT_BITMAP(arguments, 0x3F);

  if (!sl_si91x_is_device_initialized()) {
    return SL_STATUS_NOT_INITIALIZED;
  }

  if (si91x_init_configuration.boot_config.coex_mode != SL_WIFI_SYSTEM_WLAN_BLE_MODE) {
    return SL_STATUS_INVALID_MODE;
  }

  // Get values from console arguments
  uint8_t status        = GET_OPTIONAL_COMMAND_ARG(arguments, 0, 0, uint8_t);
  uint8_t scan_type     = GET_OPTIONAL_COMMAND_ARG(arguments, 1, 0, uint8_t);
  uint8_t filter_type   = GET_OPTIONAL_COMMAND_ARG(arguments, 2, 0, uint8_t);
  uint8_t own_addr_type = GET_OPTIONAL_COMMAND_ARG(arguments, 3, 0, uint8_t);
  uint16_t scan_int     = GET_OPTIONAL_COMMAND_ARG(arguments, 4, 0, uint16_t);
  uint16_t scan_win     = GET_OPTIONAL_COMMAND_ARG(arguments, 5, 0, uint16_t);

  // Verify the input values
  if (!BLE_SCAN_IS_STATUS(status) || !BLE_SCAN_IS_SCAN_TYPE(scan_type) || !BLE_SCAN_IS_FILTER_TYPE(filter_type)
      || !BLE_SCAN_IS_OWN_ADDR_TYPE(own_addr_type) || !BLE_SCAN_IS_SCAN_INT(scan_int)
      || !BLE_SCAN_IS_SCAN_WIN(scan_win)) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  // Store the BLE scan configuration
  ble_scan.status        = status;
  ble_scan.scan_type     = scan_type;
  ble_scan.filter_type   = filter_type;
  ble_scan.own_addr_type = own_addr_type;
  ble_scan.scan_int      = scan_int;
  ble_scan.scan_win      = scan_win;

  // Enable scanning configuration
  scan_configured = true;

  PRINT_AT_CMD_SUCCESS;
  return SL_STATUS_OK;
}

// at+91x-ble-scan-conf?
sl_status_t ble_scan_conf_query_command_handler(console_args_t *arguments)
{
  UNUSED_PARAMETER(arguments);

  if (!scan_configured) {
    return SL_STATUS_NOT_INITIALIZED;
  }

  PRINT_AT_CMD_SUCCESS;
  AT_PRINTF("%d %d %d %d %d %d\r\n",
            ble_scan.status,
            ble_scan.scan_type,
            ble_scan.filter_type,
            ble_scan.own_addr_type,
            ble_scan.scan_int,
            ble_scan.scan_win);
  return SL_STATUS_OK;
}

// at+91x-ble-add-acc=<dev-addr-type>,<dev-address>
sl_status_t ble_add_acc_command_handler(console_args_t *arguments)
{
  CHECK_ARGUMENT_BITMAP(arguments, 0x03);

  if (!sl_si91x_is_device_initialized()) {
    return SL_STATUS_NOT_INITIALIZED;
  }

  if (si91x_init_configuration.boot_config.coex_mode != SL_WIFI_SYSTEM_WLAN_BLE_MODE) {
    return SL_STATUS_INVALID_MODE;
  }

  uint8_t remote_address_type = GET_OPTIONAL_COMMAND_ARG(arguments, 0, 0, uint8_t);
  const char *remote_dev_addr = GET_OPTIONAL_COMMAND_ARG(arguments, 1, NULL, const char *);

  if (remote_dev_addr == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  uint8_t device_address[RSI_DEV_ADDR_LEN] = { 0 };
  rsi_ascii_dev_address_to_6bytes_rev(device_address, (int8_t *)remote_dev_addr);

  int32_t status = rsi_ble_addto_acceptlist((int8_t *)device_address, remote_address_type);
  VERIFY_RSI_STATUS_AND_RETURN(status);

  PRINT_AT_CMD_SUCCESS;
  return SL_STATUS_OK;
}

// at+91x-ble-del-acc=<dev-addr-type>,<dev-address>
sl_status_t ble_del_acc_command_handler(console_args_t *arguments)
{
  CHECK_ARGUMENT_BITMAP(arguments, 0x03);

  if (!sl_si91x_is_device_initialized()) {
    return SL_STATUS_NOT_INITIALIZED;
  }

  if (si91x_init_configuration.boot_config.coex_mode != SL_WIFI_SYSTEM_WLAN_BLE_MODE) {
    return SL_STATUS_INVALID_MODE;
  }

  uint8_t remote_address_type = GET_OPTIONAL_COMMAND_ARG(arguments, 0, 0, uint8_t);
  const char *remote_dev_addr = GET_OPTIONAL_COMMAND_ARG(arguments, 1, NULL, const char *);

  if (remote_dev_addr == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  uint8_t device_address[RSI_DEV_ADDR_LEN] = { 0 };
  rsi_ascii_dev_address_to_6bytes_rev(device_address, (int8_t *)remote_dev_addr);

  // TODO: Should test case of deletion but not added
  int32_t status = rsi_ble_deletefrom_acceptlist((int8_t *)device_address, remote_address_type);
  VERIFY_RSI_STATUS_AND_RETURN(status);

  PRINT_AT_CMD_SUCCESS;
  return SL_STATUS_OK;
}

// at+91x-ble-clear-acc
sl_status_t ble_clear_acc_command_handler(console_args_t *arguments)
{
  UNUSED_PARAMETER(arguments);

  if (!sl_si91x_is_device_initialized()) {
    return SL_STATUS_NOT_INITIALIZED;
  }

  if (si91x_init_configuration.boot_config.coex_mode != SL_WIFI_SYSTEM_WLAN_BLE_MODE) {
    return SL_STATUS_INVALID_MODE;
  }

  int32_t status = rsi_ble_clear_acceptlist();
  VERIFY_RSI_STATUS_AND_RETURN(status);

  PRINT_AT_CMD_SUCCESS;
  return SL_STATUS_OK;
}

// at+91x-ble-add-acc-type=<ad-type>,<value-length>
sl_status_t ble_add_acc_type_command_handler(console_args_t *arguments)
{
  CHECK_ARGUMENT_BITMAP(arguments, 0x03);

  if (!sl_si91x_is_device_initialized()) {
    return SL_STATUS_NOT_INITIALIZED;
  }

  if (si91x_init_configuration.boot_config.coex_mode != SL_WIFI_SYSTEM_WLAN_BLE_MODE) {
    return SL_STATUS_INVALID_MODE;
  }

  uint8_t ad_type      = GET_OPTIONAL_COMMAND_ARG(arguments, 0, 0, uint8_t);
  uint8_t value_length = GET_OPTIONAL_COMMAND_ARG(arguments, 1, 0, uint8_t);

  if (((ad_type != 8) && (ad_type != 9)) || (value_length == 0)) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  adv_type = ad_type;

  sl_status_t status = at_command_goto_data_mode(ble_add_acc_type_buffer_handler, value_length, &adv_type);
  VERIFY_STATUS_AND_RETURN(status);

  PRINT_AT_CMD_SUCCESS;
  return SL_STATUS_OK;
}

// at+91x-ble-del-acc-type=<ad-type>
sl_status_t ble_del_acc_type_command_handler(console_args_t *arguments)
{
  CHECK_ARGUMENT_BITMAP(arguments, 0x01);

  if (!sl_si91x_is_device_initialized()) {
    return SL_STATUS_NOT_INITIALIZED;
  }

  if (si91x_init_configuration.boot_config.coex_mode != SL_WIFI_SYSTEM_WLAN_BLE_MODE) {
    return SL_STATUS_INVALID_MODE;
  }

  uint8_t ad_type = GET_OPTIONAL_COMMAND_ARG(arguments, 0, 0, uint8_t);

  if ((ad_type != 8) && (ad_type != 9)) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  int32_t status = rsi_ble_acceptlist_on_type(0, ad_type, 0, NULL);
  VERIFY_RSI_STATUS_AND_RETURN(status);

  PRINT_AT_CMD_SUCCESS;
  return SL_STATUS_OK;
}

// at+91x-ble-scan-start
sl_status_t ble_scan_start_command_handler(console_args_t *arguments)
{
  UNUSED_PARAMETER(arguments);

  if (!sl_si91x_is_device_initialized()) {
    return SL_STATUS_NOT_INITIALIZED;
  }

  if (si91x_init_configuration.boot_config.coex_mode != SL_WIFI_SYSTEM_WLAN_BLE_MODE) {
    return SL_STATUS_INVALID_MODE;
  }

  int32_t status = RSI_SUCCESS;

  if (!scan_configured) {
    status = rsi_ble_start_scanning();
  } else {
    status = rsi_ble_start_scanning_with_values(&ble_scan);
  }

  rsi_ble_gap_register_callbacks(rsi_ble_on_adv_report_event, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);

  VERIFY_RSI_STATUS_AND_RETURN(status);

  PRINT_AT_CMD_SUCCESS;
  return SL_STATUS_OK;
}

// at+91x-ble-scan-stop
sl_status_t ble_scan_stop_command_handler(console_args_t *arguments)
{
  UNUSED_PARAMETER(arguments);

  if (!sl_si91x_is_device_initialized()) {
    return SL_STATUS_NOT_INITIALIZED;
  }

  if (si91x_init_configuration.boot_config.coex_mode != SL_WIFI_SYSTEM_WLAN_BLE_MODE) {
    return SL_STATUS_INVALID_MODE;
  }

  int32_t status = rsi_ble_stop_scanning();
  VERIFY_RSI_STATUS_AND_RETURN(status);

  PRINT_AT_CMD_SUCCESS;
  return SL_STATUS_OK;
}

// at+91x-ble-adv-conf=<status>,<adv-type>,<filter-type>,<direct-addr-type>,
//                     <direct-addr>,<adv-int-min>,<adv-int-max>,<own-addr-type>,
//                     <adv-channel-map>
sl_status_t ble_adv_conf_command_handler(console_args_t *arguments)
{
  CHECK_ARGUMENT_BITMAP(arguments, 0x01FF);

  if (!sl_si91x_is_device_initialized()) {
    return SL_STATUS_NOT_INITIALIZED;
  }

  // Get values from console arguments
  uint8_t status           = GET_OPTIONAL_COMMAND_ARG(arguments, 0, 0, uint8_t);
  uint8_t adv_type         = GET_OPTIONAL_COMMAND_ARG(arguments, 1, 0, uint8_t);
  uint8_t filter_type      = GET_OPTIONAL_COMMAND_ARG(arguments, 2, 0, uint8_t);
  uint8_t direct_addr_type = GET_OPTIONAL_COMMAND_ARG(arguments, 3, 0, uint8_t);
  const char *direct_addr  = GET_OPTIONAL_COMMAND_ARG(arguments, 4, NULL, const char *);
  uint16_t adv_int_min     = GET_OPTIONAL_COMMAND_ARG(arguments, 5, 0, uint16_t);
  uint16_t adv_int_max     = GET_OPTIONAL_COMMAND_ARG(arguments, 6, 0, uint16_t);
  uint8_t own_addr_type    = GET_OPTIONAL_COMMAND_ARG(arguments, 7, 0, uint8_t);
  uint8_t adv_channel_map  = GET_OPTIONAL_COMMAND_ARG(arguments, 8, 0, uint8_t);

  // Verify the input values
  if (!BLE_ADV_IS_STATUS(status) || !BLE_ADV_IS_ADV_TYPE(adv_type) || !BLE_ADV_IS_FILTER_TYPE(filter_type)
      || !BLE_ADV_IS_DIRECT_ADDR_TYPE(direct_addr_type) || !BLE_ADV_IS_ADV_INT_MIN(adv_int_min)
      || !BLE_ADV_IS_ADV_INT_MAX(adv_int_max) || !BLE_ADV_IS_OWN_ADDR_TYPE(own_addr_type)
      || !BLE_ADV_IS_ADV_CHANNEL_MAP(adv_channel_map) || (direct_addr == NULL) || (adv_int_min > adv_int_max)) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  // Store the BLE advertising configuration
  ble_adv.status           = status;
  ble_adv.adv_type         = adv_type;
  ble_adv.filter_type      = filter_type;
  ble_adv.direct_addr_type = direct_addr_type;
  (void)rsi_ascii_dev_address_to_6bytes_rev(ble_adv.direct_addr, (int8_t *)direct_addr);
  ble_adv.adv_int_min     = adv_int_min;
  ble_adv.adv_int_max     = adv_int_max;
  ble_adv.own_addr_type   = own_addr_type;
  ble_adv.adv_channel_map = adv_channel_map;

  // Enable advertising configuration
  adv_configured = true;

  PRINT_AT_CMD_SUCCESS;
  return SL_STATUS_OK;
}

// at+91x-ble-adv-conf?
sl_status_t ble_adv_conf_query_command_handler(console_args_t *arguments)
{
  UNUSED_PARAMETER(arguments);

  if (!adv_configured) {
    return SL_STATUS_NOT_INITIALIZED;
  }

  PRINT_AT_CMD_SUCCESS;
  AT_PRINTF("%d 0x%02X %d %d %02X:%02X:%02X:%02X:%02X:%02X 0x%04X 0x%04X %d %d\r\n",
            ble_adv.status,
            ble_adv.adv_type,
            ble_adv.filter_type,
            ble_adv.direct_addr_type,
            ble_adv.direct_addr[5],
            ble_adv.direct_addr[4],
            ble_adv.direct_addr[3],
            ble_adv.direct_addr[2],
            ble_adv.direct_addr[1],
            ble_adv.direct_addr[0],
            ble_adv.adv_int_min,
            ble_adv.adv_int_max,
            ble_adv.own_addr_type,
            ble_adv.adv_channel_map);
  return SL_STATUS_OK;
}

// at+91x-ble-adv-data=<data-len>
sl_status_t ble_adv_data_command_handler(console_args_t *arguments)
{
  CHECK_ARGUMENT_BITMAP(arguments, 0x01);

  if (!sl_si91x_is_device_initialized()) {
    return SL_STATUS_NOT_INITIALIZED;
  }

  if (si91x_init_configuration.boot_config.coex_mode != SL_WIFI_SYSTEM_WLAN_BLE_MODE) {
    return SL_STATUS_INVALID_MODE;
  }

  uint32_t data_len = GET_OPTIONAL_COMMAND_ARG(arguments, 0, 0, uint32_t);
  if ((data_len == 0) || (data_len > RSI_MAX_ADV_REPORT_SIZE)) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  sl_status_t status = at_command_goto_data_mode(ble_adv_data_send_buffer_handler, data_len, NULL);
  VERIFY_STATUS_AND_RETURN(status);

  PRINT_AT_CMD_SUCCESS;
  return SL_STATUS_OK;
}

// at+91x-ble-adv-data?
sl_status_t ble_adv_data_query_command_handler(console_args_t *arguments)
{
  UNUSED_PARAMETER(arguments);

  if (!adv_data_configured) {
    return SL_STATUS_NOT_INITIALIZED;
  }

  PRINT_AT_CMD_SUCCESS;
  AT_PRINTF("%" PRIu32 "", adv_data_len);
  at_print_char_buffer((char *)adv_data, adv_data_len);
  AT_PRINTF("\r\n>\r\n");

  return SL_STATUS_OK;
}

// at+91x-ble-adv-start
sl_status_t ble_adv_start_command_handler(console_args_t *arguments)
{
  UNUSED_PARAMETER(arguments);

  if (!sl_si91x_is_device_initialized()) {
    return SL_STATUS_NOT_INITIALIZED;
  }

  if (si91x_init_configuration.boot_config.coex_mode != SL_WIFI_SYSTEM_WLAN_BLE_MODE) {
    return SL_STATUS_INVALID_MODE;
  }

  int32_t status = RSI_SUCCESS;

  if (!adv_configured) {
    status = rsi_ble_start_advertising();
  } else {
    status = rsi_ble_start_advertising_with_values(&ble_adv);
  }

  VERIFY_RSI_STATUS_AND_RETURN(status);

  PRINT_AT_CMD_SUCCESS;
  return SL_STATUS_OK;
}

// at+91x-ble-adv-stop
sl_status_t ble_adv_stop_command_handler(console_args_t *arguments)
{
  UNUSED_PARAMETER(arguments);

  if (!sl_si91x_is_device_initialized()) {
    return SL_STATUS_NOT_INITIALIZED;
  }

  if (si91x_init_configuration.boot_config.coex_mode != SL_WIFI_SYSTEM_WLAN_BLE_MODE) {
    return SL_STATUS_INVALID_MODE;
  }

  int32_t status = rsi_ble_stop_advertising();
  VERIFY_RSI_STATUS_AND_RETURN(status);

  PRINT_AT_CMD_SUCCESS;
  return SL_STATUS_OK;
}
