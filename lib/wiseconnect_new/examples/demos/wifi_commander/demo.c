/********************************************************************************
 * @file  demo.c
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

/** @file
 *
 * File Description
 *
 */
#include "sl_constants.h"
#include "sl_status.h"
#include "sl_board_configuration.h"
#include "cmsis_os2.h"
#include "sl_utility.h"
#include <stdbool.h>
#include <string.h>

#include "app_wifi_api.h"
#include "app_ncp.h"
#include "app_wifi_xapi_cmd_rx_handler.h"
#include "app_simple_com.h"
#include "app_xapi_cmd_event.h"
#include "app_xapi_ap.h"
#include "sl_wifi_callback_framework.h"
/******************************************************
 *                    Constants
 ******************************************************/
#define APP_MAJOR_VERSION   4
#define APP_MINOR_VERSION   0
#define APP_PATCH_VERSION   0
#define FW_CHIP_ID          23
#define FW_ROM_ID           17
#define FW_MAJOR            2
#define FW_MINOR            15
#define FW_SECURITY_VERSION 5
#define FW_PATCH_NUM        0
#define FW_CUSTOMER_ID      0
#define FW_BUILD_NUM        2

/******************************************************
 *                   Enumerations
 ******************************************************/

/******************************************************
 *                 Type Definitions
 ******************************************************/

/******************************************************
 *                    Structures
 ******************************************************/

/******************************************************
 *               Static Function Declarations
 ******************************************************/
void application_start(const void *unused);

static sl_status_t wifi_ap_connected_event_handler(sl_wifi_event_t event,
                                                   sl_status_t status_code,
                                                   void *data,
                                                   uint32_t data_length,
                                                   void *arg);
static sl_status_t wifi_ap_disconnected_event_handler(sl_wifi_event_t event,
                                                      sl_status_t status_code,
                                                      void *data,
                                                      uint32_t data_length,
                                                      void *arg);
static sl_status_t wifi_client_scan_event_handler(sl_wifi_event_t event,
                                                  sl_status_t status_code,
                                                  sl_wifi_scan_result_t *result,
                                                  uint32_t result_length,
                                                  void *arg);
static sl_status_t wifi_scan_results_event(sl_wifi_scan_result_t *scan_result);

static sl_status_t wifi_client_join_callback_handler(sl_wifi_event_t event,
                                                     sl_status_t status_code,
                                                     char *result,
                                                     uint32_t result_length,
                                                     void *arg);

static sl_status_t wifi_client_module_status_handler(sl_wifi_event_t event,
                                                     sl_status_t status_code,
                                                     void *data,
                                                     uint32_t data_length,
                                                     void *arg);

/******************************************************
 *               Variable Definitions
 ******************************************************/

/******************************************************
 *               Function Definitions
 ******************************************************/

const osThreadAttr_t thread_attributes = {
  .name       = "application_thread",
  .attr_bits  = 0,
  .cb_mem     = 0,
  .cb_size    = 0,
  .stack_mem  = 0,
  .stack_size = 3072,
  .priority   = 0,
  .tz_module  = 0,
  .reserved   = 0,
};

void app_init(void)
{
  osThreadNew((osThreadFunc_t)application_start, NULL, &thread_attributes);
}

void wifi_set_all_event_group_cb(void)
{
  // This API would never fail, No return check required
  sl_wifi_set_callback_v2(SL_WIFI_CLIENT_CONNECTED_EVENTS, wifi_ap_connected_event_handler, NULL);
  sl_wifi_set_callback_v2(SL_WIFI_CLIENT_DISCONNECTED_EVENTS, wifi_ap_disconnected_event_handler, NULL);
  sl_wifi_set_callback_v2(SL_WIFI_SCAN_RESULT_EVENTS,
                          (sl_wifi_callback_function_v2_t)wifi_client_scan_event_handler,
                          NULL);
  sl_wifi_set_callback_v2(SL_WIFI_JOIN_EVENTS, (sl_wifi_callback_function_v2_t)wifi_client_join_callback_handler, NULL);
  sl_wifi_set_callback_v2(SL_WIFI_STATS_RESPONSE_EVENTS,
                          (sl_wifi_callback_function_v2_t)wifi_client_module_status_handler,
                          NULL);
  return;
}

void application_start(const void *unused)
{
  UNUSED_PARAMETER(unused);
  app_wifi_init_device();
  app_simple_com_os_task_init();
  app_simple_com_init();
  app_ncp_init();
  wifi_cmd_event_init();
  wifi_set_all_event_group_cb();
  app_wifi_evt_system_boot(APP_MAJOR_VERSION,
                           APP_MINOR_VERSION,
                           APP_PATCH_VERSION,
                           FW_CHIP_ID,
                           FW_ROM_ID,
                           FW_MAJOR,
                           FW_MINOR,
                           FW_SECURITY_VERSION,
                           FW_PATCH_NUM,
                           FW_CUSTOMER_ID,
                           FW_BUILD_NUM);
  while (1) {
    osDelay(10);
  }
}

static sl_status_t wifi_ap_connected_event_handler(sl_wifi_event_t event,
                                                   sl_status_t status_code,
                                                   void *data,
                                                   uint32_t data_length,
                                                   void *arg)
{
  // Suppress warnings for unused parameters
  UNUSED_PARAMETER(data_length);
  UNUSED_PARAMETER(arg);

  if (SL_WIFI_CHECK_IF_EVENT_FAILED(event)) {
    return status_code;
  }

  // Cast the void pointer to sl_mac_address_t* to access the MAC address of the connected client
  // Notify the xAPI event handler about the client connection
  app_wifi_evt_ap_client_connected_event((sl_mac_address_t *)data);

  return SL_STATUS_OK; // Return success status
}

static sl_status_t wifi_ap_disconnected_event_handler(sl_wifi_event_t event,
                                                      sl_status_t status_code,
                                                      void *data,
                                                      uint32_t data_length,
                                                      void *arg)
{
  // Suppress warnings for unused parameters
  UNUSED_PARAMETER(data_length);
  UNUSED_PARAMETER(arg);

  if (SL_WIFI_CHECK_IF_EVENT_FAILED(event)) {
    return status_code;
  }

  // Cast the void pointer to sl_mac_address_t* to access the MAC address of the disconnected client
  // Notify the xAPI event handler about the client disconnection
  app_wifi_evt_ap_client_disconnected_event((sl_mac_address_t *)data);

  return SL_STATUS_OK; // Return success status
}

static sl_status_t wifi_client_scan_event_handler(sl_wifi_event_t event,
                                                  sl_status_t status_code,
                                                  sl_wifi_scan_result_t *result,
                                                  uint32_t result_length,
                                                  void *arg)
{
  sl_status_t status = SL_STATUS_FAIL;
  UNUSED_PARAMETER(arg);

  if (SL_WIFI_CHECK_IF_EVENT_FAILED(event)) {
    //Raise a scan failed event
    app_wifi_evt_scan_error_event(SL_STATUS_FAIL);

    return status_code;
  }
  if (result_length != 0) {
    status = wifi_scan_results_event(result);
  }
  return status;
}

static sl_status_t wifi_client_join_callback_handler(sl_wifi_event_t event,
                                                     sl_status_t status_code,
                                                     char *result,
                                                     uint32_t result_length,
                                                     void *arg)
{
  UNUSED_PARAMETER(result);
  UNUSED_PARAMETER(arg);
  UNUSED_PARAMETER(result_length);

  if (SL_WIFI_CHECK_IF_EVENT_FAILED(event)) {
    app_wifi_evt_client_join_event(0);
    return status_code;
  }
  app_wifi_evt_client_join_event(1);
  return SL_STATUS_OK;
}

static sl_status_t wifi_client_module_status_handler(sl_wifi_event_t event,
                                                     sl_status_t status_code,
                                                     void *data,
                                                     uint32_t data_length,
                                                     void *arg)
{
  // Suppress warnings for unused parameters
  UNUSED_PARAMETER(arg);
  UNUSED_PARAMETER(data_length);

  if (SL_WIFI_CHECK_IF_EVENT_FAILED(event)) {
    return status_code;
  }
  // Cast the void pointer to sl_wifi_module_state_stats_response_t* to access the module state statistics
  sl_wifi_module_state_stats_response_t *notif = (sl_wifi_module_state_stats_response_t *)data;

  // Notify the xAPI event handler about the module state statistics
  /*
  * Module State Code (Upper Nibble) Description:
  * 0x50 - Current AP is best
  * 0x60 - Better AP found while roaming
  * 0x80 - Associated or joined to an Access point
  */
  if (notif->state_code == 0x60 || notif->state_code == 0x80) {
    app_wifi_evt_client_stats_response_event(notif->timestamp,
                                             notif->state_code,
                                             notif->reason_code,
                                             notif->channel,
                                             notif->rssi,
                                             notif->bssid);
  }

  return SL_STATUS_OK; // Return success status
}

static sl_status_t wifi_scan_results_event(sl_wifi_scan_result_t *scan_result)
{
  SL_WIFI_ARGS_CHECK_NULL_POINTER(scan_result);

  // Iterate through the scan results
  for (uint32_t a = 0; a < scan_result->scan_count; ++a) {
    // Invert the RSSI value (make it positive)
    scan_result->scan_info[a].rssi_val = -scan_result->scan_info[a].rssi_val;

    // Pass the data to the wifi_evt_scanning_scan_result_event function
    app_wifi_evt_scan_results_event(scan_result->scan_info[a].rf_channel,    // Channel number
                                    scan_result->scan_info[a].security_mode, // Security mode
                                    scan_result->scan_info[a].rssi_val,      // RSSI value
                                    scan_result->scan_info[a].network_type,  // Network type
                                    scan_result->scan_info[a].ssid,          // SSID buffer
                                    scan_result->scan_info[a].bssid          // BSSID
    );
  }

  // Notify that the scan is complete
  app_wifi_evt_scan_complete_event(SL_STATUS_OK);
  return SL_STATUS_OK;
}
