/********************************************************************************
 * @file  app_xapi_global.h
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
#ifndef APP_XAPI_GLOBAL_H
#define APP_XAPI_GLOBAL_H

#include "sl_net_default_values.h"
#include "app_internal_wifi_api.h"

sl_net_wifi_ap_profile_t *get_default_ap_profile(void);

sl_net_wifi_client_profile_t *get_default_client_profile(void);

sl_wifi_device_configuration_t *get_default_device_configuration(void);

void set_default_net_interface(sl_net_interface_t net_interface);

sl_net_interface_t get_default_net_interface(void);

sl_wifi_system_boot_configuration_t *get_default_device_boot_configuration(void);

app_wifi_cmd_client_set_roam_configuration_t *get_default_roam_configuration(
  void); //Get the Wi-Fi roaming configuration.

#endif // APP_XAPI_GLOBAL_H
