
/********************************************************************************
 * @file app_wifi_net_cred_xapi_cmd_lut_handler.c 
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

void app_wifi_cmd_net_cred_set(const void *);
void app_wifi_cmd_net_cred_get(const void *);
void app_wifi_cmd_net_cred_delete(const void *);

const struct xapi_cmd app_internal_xapi_cmd_table_wifi_net_cred[] = {
  {
    .handler = (app_xapi_handler)app_wifi_cmd_net_cred_set,
    .class_id = APP_INTERNAL_WIFI_NET_CRED_CLASS_ID,
    .command_id = APP_WIFI_NET_CRED_SET_COMMAND_ID,
    .metadata = {
      .sensitive = 0,
    }
  },
  {
    .handler = (app_xapi_handler)app_wifi_cmd_net_cred_get,
    .class_id = APP_INTERNAL_WIFI_NET_CRED_CLASS_ID,
    .command_id = APP_WIFI_NET_CRED_GET_COMMAND_ID,
    .metadata = {
      .sensitive = 0,
    }
  },
  {
    .handler = (app_xapi_handler)app_wifi_cmd_net_cred_delete,
    .class_id = APP_INTERNAL_WIFI_NET_CRED_CLASS_ID,
    .command_id = APP_WIFI_NET_CRED_DELETE_COMMAND_ID,
    .metadata = {
      .sensitive = 0,
    }
  },
  { 0 }
};