/*******************************************************************************
 * @file
 * @brief Wireless Roaming Example Application
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

#include "sl_status.h"
#include "sl_board_configuration.h"
#include "cmsis_os2.h"
#include "errno.h"
#include "sl_wifi.h"
#include "sl_net.h"
#include "sl_net_si91x.h"
#include "sl_wifi_callback_framework.h"
#include <string.h>

#ifdef SLI_SI91X_MCU_INTERFACE
#include "sl_si91x_m4_ps.h"
#include "sl_si91x_power_manager.h"
#endif

/******************************************************
  *                      Macros
  ******************************************************/
#define WIFI_SCAN_TIMEOUT 10000

#define ADV_SCAN_THRESHOLD              -40
#define ADV_RSSI_TOLERANCE_THRESHOLD    5
#define ADV_ACTIVE_SCAN_DURATION        15
#define ADV_PASSIVE_SCAN_DURATION       20
#define ADV_MULTIPROBE                  1
#define ADV_SCAN_PERIODICITY            10
#define BROADCAST_DROP_THRESHOLD        5000
#define BROADCAST_IN_TIM                1
#define BROADCAST_TIM_TILL_NEXT_COMMAND 1
#define ENABLE_INSTANT_SCAN             1
#define ROAMING_THRESHOLD               -50
#define ROAMING_TRIGGER_DELTA_CHANGE    5

/******************************************************
  *               Function Declarations
  ******************************************************/
static void application_start(void *argument);
sl_status_t send_data(void);
static sl_status_t module_status_handler(sl_wifi_event_t event,
                                         sl_status_t status_code,
                                         void *data,
                                         uint32_t data_length,
                                         void *arg);
void print_status_info(uint8_t state_code, uint8_t reason_code);
sl_status_t show_scan_results(sl_wifi_scan_result_t *scan_result);
sl_status_t scan_callback_handler(sl_wifi_event_t event,
                                  sl_status_t status_code,
                                  sl_wifi_scan_result_t *result,
                                  uint32_t result_length,
                                  void *arg);

/******************************************************
  *               Static Inline Functions
  ******************************************************/

/******************************************************
  *               Variable Definitions
  ******************************************************/
const osThreadAttr_t thread_attributes = {
  .name       = "app",
  .attr_bits  = 0,
  .cb_mem     = 0,
  .cb_size    = 0,
  .stack_mem  = 0,
  .stack_size = 3072,
  .priority   = osPriorityLow,
  .tz_module  = 0,
  .reserved   = 0,
};

static const sl_wifi_device_configuration_t station_init_configuration = {
  .boot_option = LOAD_NWP_FW,
  .mac_address = NULL,
  .band        = SL_SI91X_WIFI_BAND_2_4GHZ,
  .boot_config = { .oper_mode = SL_SI91X_CLIENT_MODE,
                   .coex_mode = SL_SI91X_WLAN_ONLY_MODE,
                   .feature_bit_map =
                     (SL_WIFI_FEAT_SECURITY_OPEN | SL_WIFI_FEAT_AGGREGATION | SL_SI91X_FEAT_ULP_GPIO_BASED_HANDSHAKE
#ifdef SLI_SI91X_MCU_INTERFACE
                      | SL_WIFI_FEAT_WPS_DISABLE
#endif
                      ),
                   .tcp_ip_feature_bit_map = SL_SI91X_TCP_IP_FEAT_DHCPV4_CLIENT,
                   .custom_feature_bit_map =
                     (SL_WIFI_SYSTEM_CUSTOM_FEAT_EXTENSION_VALID | SL_WIFI_CUSTOM_FEAT_ASYNC_CONNECTION_STATUS
                      | SL_WIFI_CUSTOM_FEAT_ROAM_WITH_DEAUTH_OR_NULL_DATA),
                   .ext_custom_feature_bit_map =
                     (SL_WIFI_SYSTEM_EXT_FEAT_LOW_POWER_MODE | SL_SI91X_EXT_FEAT_XTAL_CLK | MEMORY_CONFIG
#ifdef SLI_SI917
                      | SL_SI91X_EXT_FEAT_FRONT_END_SWITCH_PINS_ULP_GPIO_4_5_0
#endif
                      ),
                   .bt_feature_bit_map         = 0,
                   .ext_tcp_ip_feature_bit_map = 0,
                   .ble_feature_bit_map        = 0,
                   .ble_ext_feature_bit_map    = 0,
                   .config_feature_bit_map = (SL_SI91X_FEAT_SLEEP_GPIO_SEL_BITMAP | SL_WIFI_ENABLE_ENHANCED_MAX_PSP) }
};

volatile bool scan_results_complete  = false;
volatile sl_status_t callback_status = SL_STATUS_OK;

/******************************************************
  *               Function Definitions
  ******************************************************/
void app_init(void)
{
  osThreadNew((osThreadFunc_t)application_start, NULL, &thread_attributes);
}

static void application_start(void *argument)
{
  UNUSED_PARAMETER(argument);
  sl_status_t status;
  sl_wifi_performance_profile_v2_t performance_profile              = { .profile = ASSOCIATED_POWER_SAVE_LOW_LATENCY };
  sl_wifi_scan_configuration_t wifi_scan_configuration              = { 0 };
  sl_wifi_advanced_scan_configuration_t advanced_scan_configuration = { 0 };
  sl_wifi_roam_configuration_t roam_configuration;

  //! Initialize the Wi-Fi client interface
  status = sl_net_init(SL_NET_WIFI_CLIENT_INTERFACE, &station_init_configuration, NULL, NULL);
  if (status != SL_STATUS_OK) {
    printf("\r\nFailed to start Wi-Fi Client interface: 0x%lx\r\n", status);
    return;
  }
  printf("\r\nWi-Fi client init success\r\n");

  //! Bring up the Wi-Fi client interface
  status = sl_net_up(SL_NET_WIFI_CLIENT_INTERFACE, SL_NET_DEFAULT_WIFI_CLIENT_PROFILE_ID);
  if (status != SL_STATUS_OK) {
    printf("\r\nFailed to bring Wi-Fi client interface up: 0x%lx\r\n", status);
    return;
  }
  printf("\r\nWi-Fi client connected\r\n");

  //! Filter broadcast packets
  status = sl_wifi_filter_broadcast(BROADCAST_DROP_THRESHOLD, BROADCAST_IN_TIM, BROADCAST_TIM_TILL_NEXT_COMMAND);
  if (status != SL_STATUS_OK) {
    printf("\r\nsl_wifi_filter_broadcast Failed, Error Code : 0x%lX\r\n", status);
    return;
  }

  //! set performance profile
  status = sl_wifi_set_performance_profile_v2(&performance_profile);
  if (status != SL_STATUS_OK) {
    printf("\r\nPower save configuration Failed, Error Code : 0x%lX\r\n", status);
    return;
  }
  printf("\r\nPower save configuration success\r\n");

  //! Register the module status handler
  sl_wifi_set_callback_v2(SL_WIFI_STATS_RESPONSE_EVENTS, module_status_handler, NULL);

  //! Set the advanced scan configuration
  advanced_scan_configuration.active_channel_time  = ADV_ACTIVE_SCAN_DURATION;
  advanced_scan_configuration.passive_channel_time = ADV_PASSIVE_SCAN_DURATION;
  advanced_scan_configuration.trigger_level        = ADV_SCAN_THRESHOLD;
  advanced_scan_configuration.trigger_level_change = ADV_RSSI_TOLERANCE_THRESHOLD;
  advanced_scan_configuration.enable_multi_probe   = ADV_MULTIPROBE;
  advanced_scan_configuration.enable_instant_scan  = ENABLE_INSTANT_SCAN;

  status = sl_wifi_set_advanced_scan_configuration(&advanced_scan_configuration);
  if (status != SL_STATUS_OK) {
    printf("\r\nsl_wifi_set_advanced_scan_configuration failed with status %lx\r\n", status);
    return;
  }

  //! Register the scan callback handler
  wifi_scan_configuration.type                   = SL_WIFI_SCAN_TYPE_ADV_SCAN;
  wifi_scan_configuration.periodic_scan_interval = ADV_SCAN_PERIODICITY;
  scan_results_complete                          = false;
  callback_status                                = SL_STATUS_FAIL;

  sl_wifi_set_scan_callback_v2(scan_callback_handler, NULL);

  //! Set roaming configuration
  roam_configuration.trigger_level        = ROAMING_THRESHOLD;
  roam_configuration.trigger_level_change = ROAMING_TRIGGER_DELTA_CHANGE;

  status = sl_wifi_set_roam_configuration(SL_WIFI_CLIENT_2_4GHZ_INTERFACE, &roam_configuration);
  if (status != SL_STATUS_OK) {
    printf("\r\nRoam failed with status %lx\r\n", status);
    return;
  } else {
    printf("\r\nRoaming configuration set successfully\r\n");
  }

  //! Start the scan
  status = sl_wifi_start_scan(SL_WIFI_CLIENT_2_4GHZ_INTERFACE, NULL, &wifi_scan_configuration);
  if (SL_STATUS_IN_PROGRESS == status) {
    printf("\r\nScanning...\r\n");
    const uint32_t start = osKernelGetTickCount();

    while (!scan_results_complete && (osKernelGetTickCount() - start) <= WIFI_SCAN_TIMEOUT) {
      osThreadYield();
    }

    status = scan_results_complete ? callback_status : SL_STATUS_TIMEOUT;
  }

  if (status != SL_STATUS_OK) {
    printf("\r\nscan failed with status %lx\r\n", status);
    return;
  }

#ifdef SLI_SI91X_MCU_INTERFACE
#if (SL_SI91X_TICKLESS_MODE == 0)
  sl_si91x_power_manager_sleep();
  while (1) {
    osThreadYield();
  }
#else
  osSemaphoreId_t wait_semaphore;
  wait_semaphore = osSemaphoreNew(1, 0, NULL);
  if (wait_semaphore == NULL) {
    printf("Failed to create semaphore\r\n");
    return;
  }
  // Waiting forever using semaphore to put M4 to sleep in tick less mode
  osSemaphoreAcquire(wait_semaphore, osWaitForever);
#endif
#else
  while (1) {
    osThreadYield();
  }
#endif
}

static sl_status_t module_status_handler(sl_wifi_event_t event,
                                         sl_status_t status_code,
                                         void *data,
                                         uint32_t data_length,
                                         void *arg)
{
  UNUSED_PARAMETER(arg);

  if (SL_WIFI_CHECK_IF_EVENT_FAILED(event)) {
    return status_code;
  }

  sl_wifi_module_state_stats_response_t *notif = (sl_wifi_module_state_stats_response_t *)data;

  printf("\r\n---> Module status handler event with length : %lu\r\n", data_length);
  printf("  <> Timestamp : %lu, state_code : 0x%02X, reason_code : 0x%02X, channel : %u, rssi : -%u.\n",
         notif->timestamp,
         notif->state_code,
         notif->reason_code,
         notif->channel,
         notif->rssi);
  printf("  <> BSSID : %x:%x:%x:%x:%x:%x.\n",
         notif->bssid[0],
         notif->bssid[1],
         notif->bssid[2],
         notif->bssid[3],
         notif->bssid[4],
         notif->bssid[5]);

  //! Display the status information
  print_status_info(notif->state_code, notif->reason_code);

  return SL_STATUS_OK;
}

sl_status_t show_scan_results(sl_wifi_scan_result_t *scan_result)
{
  SL_WIFI_ARGS_CHECK_NULL_POINTER(scan_result);
  uint8_t *bssid = NULL;
  printf("%ld scan results:\n", scan_result->scan_count);
  if (scan_result->scan_count)
    printf("\n   %s %24s %s", "SSID", "SECURITY", "NETWORK");
  printf("%12s %12s %s\n", "BSSID", "CHANNEL", "RSSI");
  for (uint32_t a = 0; a < scan_result->scan_count; ++a) {
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

void print_status_info(uint8_t state_code, uint8_t reason_code)
{
  /**
   * Module State Code (Upper Nibble) Description:
   * 0x00 - Startup (Initial state or idle state)
   * 0x10 - Beacon Loss
   * 0x20 - De-authentication from AP
   * 0x50 - Current AP is best
   * 0x60 - Better AP found while roaming
   * 0x70 - No AP found
   * 0x80 - Associated or joined to an Access point
   * 0x90 - Unassociated (Disconnected from host or join failure)
   * 
   */
  switch (state_code & 0xF0) {
    case 0x00:
      printf("State: Startup (Initial state or idle state)\n");
      break;
    case 0x10:
      printf("State: Beacon Loss\n");
      break;
    case 0x20:
      printf("State: De-authentication from AP\n");
      break;
    case 0x50:
      printf("State: Current AP is best\n");
      break;
    case 0x60:
      printf("State: Better AP found while roaming\n");
      break;
    case 0x70:
      printf("State: No AP found\n");
      break;
    case 0x80:
      printf("State: Associated or joined to an Access point\n");
      break;
    case 0x90:
      printf("State: Unassociated (Disconnected from host or join failure)\n");
      break;
    default:
      printf("State: Unknown state code\n");
      break;
  }

  /**
   * Reason Code Details:
   * 0x00 - No reason specified (Initial state or idle state)
   * 0x01 - No response from AP for authentication request (Authentication denial)
   * 0x02 - Association denial (Association timeout or failure due to unknown reasons)
   * 0x03 - User-configured AP is not present
   * 0x05 - Four-way Handshake failure
   * 0x06 - Deauthentication from user
   * 0x07 - PSK not configured
   * 0x08 - Key-handshake failure during rejoin/roaming/after connection (Disconnection from supplicant)
   * 0x09 - Roaming not enabled
   * 0x10 - Beacon Loss (failover Roam)
   * 0x20 - De-authentication from AP
   * 0x28 - TLS CA Cert not present
   * 0x29 - TLS PRIVATE key not present
   * 0x2A - TLS Client Cert not present
   * 0x2B - TLS no Cert present
   * 0x2C - PEAP CA Cert not present
   * 0x2D - Server Cert Invalid Key Type
   * 0x2E - Server Intermediate CA Invalid Key Type
   * 0x2F - Server Root CA Invalid Key Type
   * 0x30 - Client Cert Invalid Key Type
   * 0x31 - Client Root CA Invalid Key Type
   * 0x37 - Server Cert 4096-bit length support is not enabled
   * 0x38 - Server Intermediate CA 4096-bit length support is not enabled
   * 0x39 - Server Root CA 4096-bit length support is not enabled
   * 0x3A - Client Cert 4096-bit length support is not enabled
   * 0x3B - Client Root CA 4096-bit length support is not enabled
   * 0x3C - Server Cert Invalid Sign Alg
   * 0x3D - Server Intermediate CA Invalid Sign Alg
   * 0x3E - Server Root CA Invalid Sign Length
   * 0x3F - Client Cert Invalid Sign Alg
   * 0x40 - Client Root CA Invalid Sign Length
   * 0x41 - Server Intermediate CA not Present
   * 0x42 - Server Root CA Parse Error
   * 0x43 - Server Intermediate Root CA Parse Error
   * 0x44 - Server Cert Parse Error
   * 0x45 - Client Cert Parse Error
   * 0x46 - Incorrect Private Key Password
   * 0x47 - EAP Failure Received
   * 0x48 - Client Cert Bad Date Error
   * 0x49 - Server Cert Bad Date Error
   * 0x4A - Server Root CA Bad Date Error
   * 0x4B - Client Root CA Bad Date Error
   * 0x4C - Server Intermediate Root CA Bad Date Error
   * 0x4D - Pem Header Error
   * 0x4E - Pem Footer Error
   * 0x4F - Client Intermediate CA Invalid Sign Length
   * 0x50 - Client Intermediate CA Invalid Length
   * 0x52 - Client Intermediate CA invalid Key Type
   * 0x53 - Pem Error
   * 0x54 - Pathlen certificate is Invalid
   * 
   */
  switch (reason_code) {
    case 0x00:
      printf("Reason: Initial state or idle state\n");
      break;
    case 0x01:
      printf("Reason: No response from AP for authentication request (Authentication denial)\n");
      break;
    case 0x02:
      printf("Reason: Association denial (Association timeout or failure)\n");
      break;
    case 0x03:
      printf("Reason: User-configured AP is not present\n");
      break;
    case 0x05:
      printf("Reason: Four-way Handshake failure\n");
      break;
    case 0x06:
      printf("Reason: Deauthentication from user\n");
      break;
    case 0x07:
      printf("Reason: PSK not configured\n");
      break;
    case 0x08:
      printf("Reason: Key-handshake failure during rejoin/roaming/after connection\n");
      break;
    case 0x09:
      printf("Reason: Roaming not enabled\n");
      break;
    case 0x10:
      printf("Reason: Beacon Loss (failover Roam)\n");
      break;
    case 0x20:
      printf("Reason: De-authentication from AP\n");
      break;
    case 0x28:
      printf("Reason: TLS CA Cert not present\n");
      break;
    case 0x29:
      printf("Reason: TLS PRIVATE key not present\n");
      break;
    case 0x2A:
      printf("Reason: TLS Client Cert not present\n");
      break;
    case 0x2B:
      printf("Reason: TLS no Cert present\n");
      break;
    case 0x2C:
      printf("Reason: PEAP CA Cert not present\n");
      break;
    case 0x2D:
      printf("Reason: Server Cert Invalid Key Type\n");
      break;
    case 0x2E:
      printf("Reason: Server Intermediate CA Invalid Key Type\n");
      break;
    case 0x2F:
      printf("Reason: Server Root CA Invalid Key Type\n");
      break;
    case 0x30:
      printf("Reason: Client Cert Invalid Key Type\n");
      break;
    case 0x31:
      printf("Reason: Client Root CA Invalid Key Type\n");
      break;
    case 0x37:
      printf("Reason: Server Cert 4096-bit length support is not enabled\n");
      break;
    case 0x38:
      printf("Reason: Server Intermediate CA 4096-bit length support is not enabled\n");
      break;
    case 0x39:
      printf("Reason: Server Root CA 4096-bit length support is not enabled\n");
      break;
    case 0x3A:
      printf("Reason: Client Cert 4096-bit length support is not enabled\n");
      break;
    case 0x3B:
      printf("Reason: Client Root CA 4096-bit length support is not enabled\n");
      break;
    case 0x3C:
      printf("Reason: Server Cert Invalid Sign Alg\n");
      break;
    case 0x3D:
      printf("Reason: Server Intermediate CA Invalid Sign Alg\n");
      break;
    case 0x3E:
      printf("Reason: Server Root CA Invalid Sign Length\n");
      break;
    case 0x3F:
      printf("Reason: Client Cert Invalid Sign Alg\n");
      break;
    case 0x40:
      printf("Reason: Client Root CA Invalid Sign Length\n");
      break;
    case 0x41:
      printf("Reason: Server Intermediate CA not Present\n");
      break;
    case 0x42:
      printf("Reason: Server Root CA Parse Error\n");
      break;
    case 0x43:
      printf("Reason: Server Intermediate Root CA Parse Error\n");
      break;
    case 0x44:
      printf("Reason: Server Cert Parse Error\n");
      break;
    case 0x45:
      printf("Reason: Client Cert Parse Error\n");
      break;
    case 0x46:
      printf("Reason: Incorrect Private Key Password\n");
      break;
    case 0x47:
      printf("Reason: EAP Failure Received\n");
      break;
    case 0x48:
      printf("Reason: Client Cert Bad Date Error\n");
      break;
    case 0x49:
      printf("Reason: Server Cert Bad Date Error\n");
      break;
    case 0x4A:
      printf("Reason: Server Root CA Bad Date Error\n");
      break;
    case 0x4B:
      printf("Reason: Client Root CA Bad Date Error\n");
      break;
    case 0x4C:
      printf("Reason: Server Intermediate Root CA Bad Date Error\n");
      break;
    case 0x4D:
      printf("Reason: Pem Header Error\n");
      break;
    case 0x4E:
      printf("Reason: Pem Footer Error\n");
      break;
    case 0x4F:
      printf("Reason: Client Intermediate CA Invalid Sign Length\n");
      break;
    case 0x50:
      printf("Reason: Client Intermediate CA Invalid Length\n");
      break;
    case 0x52:
      printf("Reason: Client Intermediate CA invalid Key Type\n");
      break;
    case 0x53:
      printf("Reason: Pem Error\n");
      break;
    case 0x54:
      printf("Reason: Pathlen certificate is Invalid\n");
      break;
    default:
      printf("Reason: Unknown reason code\n");
      break;
  }
}
