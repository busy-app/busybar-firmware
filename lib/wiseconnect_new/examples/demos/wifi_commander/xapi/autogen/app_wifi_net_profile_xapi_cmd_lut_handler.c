
/********************************************************************************
 * @file app_wifi_net_profile_xapi_cmd_lut_handler.c 
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

void app_wifi_cmd_net_profile_set(const void *);
void app_wifi_cmd_net_profile_delete(const void *);
void app_wifi_cmd_net_profile_set_ap_ssid(const void *);
void app_wifi_cmd_net_profile_set_ap_config(const void *);
void app_wifi_cmd_net_profile_set_ap_channel(const void *);
void app_wifi_cmd_net_profile_set_ap_ip_config(const void *);
void app_wifi_cmd_net_profile_set_ap_ipv4_config(const void *);
void app_wifi_cmd_net_profile_set_ap_ipv6_config(const void *);
void app_wifi_cmd_net_profile_get_ap_ssid(const void *);
void app_wifi_cmd_net_profile_get_ap_config(const void *);
void app_wifi_cmd_net_profile_get_ap_channel(const void *);
void app_wifi_cmd_net_profile_get_ap_ip_config(const void *);
void app_wifi_cmd_net_profile_get_ap_ipv4_config(const void *);
void app_wifi_cmd_net_profile_get_ap_ipv6_config(const void *);
void app_wifi_cmd_net_profile_set_client_ssid(const void *);
void app_wifi_cmd_net_profile_set_client_config(const void *);
void app_wifi_cmd_net_profile_set_client_channel(const void *);
void app_wifi_cmd_net_profile_get_client_ssid(const void *);
void app_wifi_cmd_net_profile_get_client_config(const void *);
void app_wifi_cmd_net_profile_get_client_channel(const void *);

const struct xapi_cmd app_internal_xapi_cmd_table_wifi_net_profile[] = {
  {
    .handler = (app_xapi_handler)app_wifi_cmd_net_profile_set,
    .class_id = APP_INTERNAL_WIFI_NET_PROFILE_CLASS_ID,
    .command_id = APP_WIFI_NET_PROFILE_SET_COMMAND_ID,
    .metadata = {
      .sensitive = 0,
    }
  },
  {
    .handler = (app_xapi_handler)app_wifi_cmd_net_profile_delete,
    .class_id = APP_INTERNAL_WIFI_NET_PROFILE_CLASS_ID,
    .command_id = APP_WIFI_NET_PROFILE_DELETE_COMMAND_ID,
    .metadata = {
      .sensitive = 0,
    }
  },
  {
    .handler = (app_xapi_handler)app_wifi_cmd_net_profile_set_ap_ssid,
    .class_id = APP_INTERNAL_WIFI_NET_PROFILE_CLASS_ID,
    .command_id = APP_WIFI_NET_PROFILE_SET_AP_SSID_COMMAND_ID,
    .metadata = {
      .sensitive = 0,
    }
  },
  {
    .handler = (app_xapi_handler)app_wifi_cmd_net_profile_set_ap_config,
    .class_id = APP_INTERNAL_WIFI_NET_PROFILE_CLASS_ID,
    .command_id = APP_WIFI_NET_PROFILE_SET_AP_CONFIG_COMMAND_ID,
    .metadata = {
      .sensitive = 0,
    }
  },
  {
    .handler = (app_xapi_handler)app_wifi_cmd_net_profile_set_ap_channel,
    .class_id = APP_INTERNAL_WIFI_NET_PROFILE_CLASS_ID,
    .command_id = APP_WIFI_NET_PROFILE_SET_AP_CHANNEL_COMMAND_ID,
    .metadata = {
      .sensitive = 0,
    }
  },
  {
    .handler = (app_xapi_handler)app_wifi_cmd_net_profile_set_ap_ip_config,
    .class_id = APP_INTERNAL_WIFI_NET_PROFILE_CLASS_ID,
    .command_id = APP_WIFI_NET_PROFILE_SET_AP_IP_CONFIG_COMMAND_ID,
    .metadata = {
      .sensitive = 0,
    }
  },
  {
    .handler = (app_xapi_handler)app_wifi_cmd_net_profile_set_ap_ipv4_config,
    .class_id = APP_INTERNAL_WIFI_NET_PROFILE_CLASS_ID,
    .command_id = APP_WIFI_NET_PROFILE_SET_AP_IPV4_CONFIG_COMMAND_ID,
    .metadata = {
      .sensitive = 0,
    }
  },
  {
    .handler = (app_xapi_handler)app_wifi_cmd_net_profile_set_ap_ipv6_config,
    .class_id = APP_INTERNAL_WIFI_NET_PROFILE_CLASS_ID,
    .command_id = APP_WIFI_NET_PROFILE_SET_AP_IPV6_CONFIG_COMMAND_ID,
    .metadata = {
      .sensitive = 0,
    }
  },
  {
    .handler = (app_xapi_handler)app_wifi_cmd_net_profile_get_ap_ssid,
    .class_id = APP_INTERNAL_WIFI_NET_PROFILE_CLASS_ID,
    .command_id = APP_WIFI_NET_PROFILE_GET_AP_SSID_COMMAND_ID,
    .metadata = {
      .sensitive = 0,
    }
  },
  {
    .handler = (app_xapi_handler)app_wifi_cmd_net_profile_get_ap_config,
    .class_id = APP_INTERNAL_WIFI_NET_PROFILE_CLASS_ID,
    .command_id = APP_WIFI_NET_PROFILE_GET_AP_CONFIG_COMMAND_ID,
    .metadata = {
      .sensitive = 0,
    }
  },
  {
    .handler = (app_xapi_handler)app_wifi_cmd_net_profile_get_ap_channel,
    .class_id = APP_INTERNAL_WIFI_NET_PROFILE_CLASS_ID,
    .command_id = APP_WIFI_NET_PROFILE_GET_AP_CHANNEL_COMMAND_ID,
    .metadata = {
      .sensitive = 0,
    }
  },
  {
    .handler = (app_xapi_handler)app_wifi_cmd_net_profile_get_ap_ip_config,
    .class_id = APP_INTERNAL_WIFI_NET_PROFILE_CLASS_ID,
    .command_id = APP_WIFI_NET_PROFILE_GET_AP_IP_CONFIG_COMMAND_ID,
    .metadata = {
      .sensitive = 0,
    }
  },
  {
    .handler = (app_xapi_handler)app_wifi_cmd_net_profile_get_ap_ipv4_config,
    .class_id = APP_INTERNAL_WIFI_NET_PROFILE_CLASS_ID,
    .command_id = APP_WIFI_NET_PROFILE_GET_AP_IPV4_CONFIG_COMMAND_ID,
    .metadata = {
      .sensitive = 0,
    }
  },
  {
    .handler = (app_xapi_handler)app_wifi_cmd_net_profile_get_ap_ipv6_config,
    .class_id = APP_INTERNAL_WIFI_NET_PROFILE_CLASS_ID,
    .command_id = APP_WIFI_NET_PROFILE_GET_AP_IPV6_CONFIG_COMMAND_ID,
    .metadata = {
      .sensitive = 0,
    }
  },
  {
    .handler = (app_xapi_handler)app_wifi_cmd_net_profile_set_client_ssid,
    .class_id = APP_INTERNAL_WIFI_NET_PROFILE_CLASS_ID,
    .command_id = APP_WIFI_NET_PROFILE_SET_CLIENT_SSID_COMMAND_ID,
    .metadata = {
      .sensitive = 0,
    }
  },
  {
    .handler = (app_xapi_handler)app_wifi_cmd_net_profile_set_client_config,
    .class_id = APP_INTERNAL_WIFI_NET_PROFILE_CLASS_ID,
    .command_id = APP_WIFI_NET_PROFILE_SET_CLIENT_CONFIG_COMMAND_ID,
    .metadata = {
      .sensitive = 0,
    }
  },
  {
    .handler = (app_xapi_handler)app_wifi_cmd_net_profile_set_client_channel,
    .class_id = APP_INTERNAL_WIFI_NET_PROFILE_CLASS_ID,
    .command_id = APP_WIFI_NET_PROFILE_SET_CLIENT_CHANNEL_COMMAND_ID,
    .metadata = {
      .sensitive = 0,
    }
  },
  {
    .handler = (app_xapi_handler)app_wifi_cmd_net_profile_get_client_ssid,
    .class_id = APP_INTERNAL_WIFI_NET_PROFILE_CLASS_ID,
    .command_id = APP_WIFI_NET_PROFILE_GET_CLIENT_SSID_COMMAND_ID,
    .metadata = {
      .sensitive = 0,
    }
  },
  {
    .handler = (app_xapi_handler)app_wifi_cmd_net_profile_get_client_config,
    .class_id = APP_INTERNAL_WIFI_NET_PROFILE_CLASS_ID,
    .command_id = APP_WIFI_NET_PROFILE_GET_CLIENT_CONFIG_COMMAND_ID,
    .metadata = {
      .sensitive = 0,
    }
  },
  {
    .handler = (app_xapi_handler)app_wifi_cmd_net_profile_get_client_channel,
    .class_id = APP_INTERNAL_WIFI_NET_PROFILE_CLASS_ID,
    .command_id = APP_WIFI_NET_PROFILE_GET_CLIENT_CHANNEL_COMMAND_ID,
    .metadata = {
      .sensitive = 0,
    }
  },
  { 0 }
};