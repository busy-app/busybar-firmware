
/********************************************************************************
 * @file app_wifi_scan_xapi_cmd_lut_handler.c 
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

#include <stdio.h>

#include "app_internal_wifi_api.h"
#include "app_xapi.h"
#include "xapi.h"

void app_wifi_cmd_scan_start(const void *);
void app_wifi_cmd_scan_start_with_ssid(const void *);
void app_wifi_cmd_scan_set_advanced_configuration(const void *);
void app_wifi_cmd_scan_get_advanced_configuration(const void *);
void app_wifi_cmd_scan_stop(const void *);

const struct xapi_cmd app_internal_xapi_cmd_table_wifi_scan[] = {
  {
    .handler = (app_xapi_handler)app_wifi_cmd_scan_start,
    .class_id = APP_INTERNAL_WIFI_SCAN_CLASS_ID,
    .command_id = APP_WIFI_SCAN_START_COMMAND_ID,
    .metadata = {
      .sensitive = 0,
    }
  },
  {
    .handler = (app_xapi_handler)app_wifi_cmd_scan_start_with_ssid,
    .class_id = APP_INTERNAL_WIFI_SCAN_CLASS_ID,
    .command_id = APP_WIFI_SCAN_START_WITH_SSID_COMMAND_ID,
    .metadata = {
      .sensitive = 0,
    }
  },
  {
    .handler = (app_xapi_handler)app_wifi_cmd_scan_set_advanced_configuration,
    .class_id = APP_INTERNAL_WIFI_SCAN_CLASS_ID,
    .command_id = APP_WIFI_SCAN_SET_ADVANCED_CONFIGURATION_COMMAND_ID,
    .metadata = {
      .sensitive = 0,
    }
  },
  {
    .handler = (app_xapi_handler)app_wifi_cmd_scan_get_advanced_configuration,
    .class_id = APP_INTERNAL_WIFI_SCAN_CLASS_ID,
    .command_id = APP_WIFI_SCAN_GET_ADVANCED_CONFIGURATION_COMMAND_ID,
    .metadata = {
      .sensitive = 0,
    }
  },
  {
    .handler = (app_xapi_handler)app_wifi_cmd_scan_stop,
    .class_id = APP_INTERNAL_WIFI_SCAN_CLASS_ID,
    .command_id = APP_WIFI_SCAN_STOP_COMMAND_ID,
    .metadata = {
      .sensitive = 0,
    }
  },
  { 0 }
};