/********************************************************************************
 * @file  app_xapi_client.c
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
#include "app_xapi_global.h"

DEFINE_XAPI_CLASS(wifi, client, WIFI, CLIENT, NULL, NULL);

void app_wifi_cmd_client_set_roam_configuration(app_wifi_cmd_client_set_roam_configuration_t *cmd_input)
{
  sl_status_t status = SL_STATUS_FAIL;

  // Structure to hold the roam configuration parameters
  sl_wifi_roam_configuration_t roam_configuration;

  // Retrieve the default roam configuration
  app_wifi_cmd_client_set_roam_configuration_t *default_roam_configuration = get_default_roam_configuration();

  // Set the roam configuration parameters from the input command
  roam_configuration.trigger_level        = cmd_input->trigger_level;
  roam_configuration.trigger_level_change = cmd_input->trigger_level_change;

  // Apply the roam configuration to the specified WiFi interface
  status = sl_wifi_set_roam_configuration(cmd_input->wifi_interface, &roam_configuration);

  // If the operation is successful, update the default roam configuration
  if (status == SL_STATUS_OK) {
    default_roam_configuration->wifi_interface       = cmd_input->wifi_interface;
    default_roam_configuration->trigger_level        = cmd_input->trigger_level;
    default_roam_configuration->trigger_level_change = cmd_input->trigger_level_change;
  }

  // Send the response with the status of the operation
  app_wifi_rsp_client_set_roam_configuration(status);
  return;
}

void app_wifi_cmd_client_get_roam_configuration(app_wifi_cmd_client_get_roam_configuration_t *cmd_input)
{
  UNUSED_PARAMETER(cmd_input);

  // Retrieve the default roam configuration
  app_wifi_cmd_client_set_roam_configuration_t *roam_configuration = get_default_roam_configuration();

  // Check if the requested interface is supported
  if (cmd_input->wifi_interface != SL_WIFI_CLIENT_INTERFACE) {
    // Respond with not supported status if the interface is not the client interface
    app_wifi_rsp_client_get_roam_configuration(SL_STATUS_NOT_SUPPORTED, 0, 0);
    return;
  }

  // Respond with the current roam configuration for the client interface
  app_wifi_rsp_client_get_roam_configuration(SL_STATUS_OK,
                                             roam_configuration->trigger_level,
                                             roam_configuration->trigger_level_change);
  return;
}
