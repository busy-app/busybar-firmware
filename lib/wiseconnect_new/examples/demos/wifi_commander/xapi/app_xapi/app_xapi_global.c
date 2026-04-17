/********************************************************************************
 * @file  app_xapi_global.c
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
#include "sl_net_default_values.h"
#include "app_xapi_global.h"

static sl_net_wifi_ap_profile_t wifi_default_ap_profile = DEFAULT_WIFI_ACCESS_POINT_PROFILE;

static sl_net_wifi_client_profile_t wifi_default_client_profile = DEFAULT_WIFI_CLIENT_PROFILE;

static sl_wifi_device_configuration_t default_device_configuration;

// 0 for invalid state
static sl_net_interface_t default_net_interface;

//Static variable to store the Wi-Fi roaming configuration
static app_wifi_cmd_client_set_roam_configuration_t default_roam_configuration;

// Function to get the Wi-Fi roaming configuration.
app_wifi_cmd_client_set_roam_configuration_t *get_default_roam_configuration(void)
{
  return &default_roam_configuration;
}

sl_net_wifi_ap_profile_t *get_default_ap_profile(void)
{
  return &wifi_default_ap_profile;
}

sl_net_wifi_client_profile_t *get_default_client_profile(void)
{
  return &wifi_default_client_profile;
}

void set_default_net_interface(sl_net_interface_t net_interface)
{
  default_net_interface = net_interface;
  return;
}

sl_net_interface_t get_default_net_interface(void)
{
  return default_net_interface;
}

sl_wifi_device_configuration_t *get_default_device_configuration(void)
{
  return &default_device_configuration;
}

sl_wifi_system_boot_configuration_t *get_default_device_boot_configuration(void)
{
  return &default_device_configuration.boot_config;
}
