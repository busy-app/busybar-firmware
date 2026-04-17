/********************************************************************************
 * @file  app_xapi_scan.c
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
#include "app_wifi_xapi_cmd_rx_handler.h"
#include "si91x_device.h"
#include "sl_net.h"
#include "sl_wifi.h"
#include "sl_wifi_callback_framework.h"
#include "sl_si91x_driver.h"
#include "FreeRTOS.h"
#include "task.h"
#include "sl_net_wifi_types.h"

DEFINE_XAPI_CLASS(wifi, scan, WIFI, SCAN, NULL, NULL);

void app_wifi_cmd_scan_start(app_wifi_cmd_scan_start_t *cmd_input)
{
  sl_status_t status     = SL_STATUS_FAIL;
  uint8_t is_in_progress = 0;

  // Initialize the scan_config and map fields from cmd_input in one go
  sl_wifi_scan_configuration_t scan_config = { .type                   = (sl_wifi_scan_type_t)cmd_input->mode,
                                               .flags                  = cmd_input->flags,
                                               .periodic_scan_interval = 0, // Set to zero
                                               .channel_bitmap_2g4     = cmd_input->channel_bitmap_2_4ghz,
                                               .channel_bitmap_5g      = { 0 }, // Initializes all elements to 0
                                               .lp_mode                = cmd_input->lp_mode };

  // Start the WiFi scan with the given configuration and no specific SSID
  status = sl_wifi_start_scan(cmd_input->wifi_interface, NULL, &scan_config);
  if (status == SL_STATUS_IN_PROGRESS) {
    status         = SL_STATUS_OK;
    is_in_progress = 1;
  }

  // Respond with the scan start status and whether it is in progress
  app_wifi_rsp_scan_start(status, is_in_progress);
  return;
}

void app_wifi_cmd_scan_start_with_ssid(app_wifi_cmd_scan_start_with_ssid_t *cmd_input)
{
  sl_status_t status     = SL_STATUS_FAIL;
  uint8_t is_in_progress = 0;

  // Initialize the scan_config and map fields from cmd_input in one go
  sl_wifi_scan_configuration_t scan_config = { .type                   = (sl_wifi_scan_type_t)cmd_input->mode,
                                               .flags                  = cmd_input->flags,
                                               .periodic_scan_interval = 0, // Set to zero
                                               .channel_bitmap_2g4     = cmd_input->channel_bitmap_2_4ghz,
                                               .channel_bitmap_5g      = { 0 }, // Initializes all elements to 0
                                               .lp_mode                = cmd_input->lp_mode };

  // Initialize the SSID structure and copy the SSID from cmd_input
  sl_wifi_ssid_t ssid = {
    .length = cmd_input->ssid.len,
  };
  memcpy(ssid.value, cmd_input->ssid.data, cmd_input->ssid.len);

  // Start the WiFi scan with the given configuration and specific SSID
  status = sl_wifi_start_scan(cmd_input->wifi_interface, &ssid, &scan_config);
  if (status == SL_STATUS_IN_PROGRESS) {
    status         = SL_STATUS_OK;
    is_in_progress = 1;
  }

  // Respond with the scan start status and whether it is in progress
  app_wifi_rsp_scan_start_with_ssid(status, is_in_progress);
  return;
}

void app_wifi_cmd_scan_set_advanced_configuration(app_wifi_cmd_scan_set_advanced_configuration_t *cmd_input)
{
  sl_status_t status                                                = SL_STATUS_FAIL;
  sl_wifi_advanced_scan_configuration_t advanced_scan_configuration = { 0 };

  // Map fields from cmd_input to advanced_scan_configuration
  advanced_scan_configuration.trigger_level        = cmd_input->trigger_level;
  advanced_scan_configuration.trigger_level_change = cmd_input->trigger_level_change;
  advanced_scan_configuration.active_channel_time  = cmd_input->active_channel_time;
  advanced_scan_configuration.passive_channel_time = cmd_input->passive_channel_time;
  advanced_scan_configuration.enable_instant_scan  = cmd_input->enable_instant_scan;
  advanced_scan_configuration.enable_multi_probe   = cmd_input->enable_multi_probe;

  // Set the advanced scan configuration
  status = sl_wifi_set_advanced_scan_configuration(&advanced_scan_configuration);

  // Respond with the status of setting the advanced scan configuration
  app_wifi_rsp_scan_set_advanced_configuration(status);
  return;
}

void app_wifi_cmd_scan_get_advanced_configuration(void)
{
  sl_status_t status                                                = SL_STATUS_FAIL;
  sl_wifi_advanced_scan_configuration_t advanced_scan_configuration = { 0 };

  // Get the current advanced scan configuration
  status = sl_wifi_get_advanced_scan_configuration(&advanced_scan_configuration);

  // Respond with the current advanced scan configuration
  app_wifi_rsp_scan_get_advanced_configuration(status,
                                               advanced_scan_configuration.trigger_level,
                                               advanced_scan_configuration.trigger_level_change,
                                               advanced_scan_configuration.active_channel_time,
                                               advanced_scan_configuration.passive_channel_time,
                                               advanced_scan_configuration.enable_instant_scan,
                                               advanced_scan_configuration.enable_multi_probe);
  return;
}

void app_wifi_cmd_scan_stop(app_wifi_cmd_scan_stop_t *cmd_input)
{
  sl_status_t status = SL_STATUS_FAIL;

  // Stop the WiFi scan on the specified interface
  status = sl_wifi_stop_scan(cmd_input->wifi_interface);

  // Respond with the status of stopping the scan
  app_wifi_rsp_scan_stop(status);

  return;
}
