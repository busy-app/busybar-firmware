/********************************************************************************
 * @file  app_xapi_net_profile.c
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
#include "sl_net_constants.h"
#include "sl_net.h"
#include "sl_wifi.h"
#include "app_wifi_api.h"
#include "sl_net_default_values.h"
#include "sl_wifi_callback_framework.h"
#include "sl_si91x_driver.h"
#include "FreeRTOS.h"
#include "task.h"
#include "app_xapi_global.h"

DEFINE_XAPI_CLASS(wifi, net_profile, WIFI, NET_PROFILE, NULL, NULL);

void app_wifi_cmd_net_profile_set(app_wifi_cmd_net_profile_set_t *cmd_input)
{
  sl_status_t status = SL_STATUS_FAIL;

  if (cmd_input->net_interface == APP_WIFI_NET_INTF_INTERFACE_AP) {
    status = sl_net_set_profile(SL_NET_WIFI_AP_INTERFACE, cmd_input->profile_id, get_default_ap_profile());
  } else if (cmd_input->net_interface == APP_WIFI_NET_INTF_INTERFACE_CLIENT) {
    status = sl_net_set_profile(SL_NET_WIFI_CLIENT_INTERFACE, cmd_input->profile_id, get_default_client_profile());
  } else {
    status = SL_STATUS_NOT_SUPPORTED;
  }

  app_wifi_rsp_net_profile_set(status);
  return;
}

void app_wifi_cmd_net_profile_delete(app_wifi_cmd_net_profile_delete_t *cmd_input)
{
  sl_status_t status = SL_STATUS_FAIL;

  if (cmd_input->net_interface == APP_WIFI_NET_INTF_INTERFACE_AP) {
    status = sl_net_delete_profile(SL_NET_WIFI_AP_INTERFACE, cmd_input->profile_id);
  } else {
    status = SL_STATUS_NOT_SUPPORTED;
  }

  app_wifi_rsp_net_profile_delete(status);
  return;
}

void app_wifi_cmd_net_profile_set_ap_ssid(app_wifi_cmd_net_profile_set_ap_ssid_t *cmd_input)
{
  if (cmd_input == NULL) {
    app_wifi_rsp_net_profile_set_ap_ssid(SL_STATUS_INVALID_PARAMETER);
    return;
  }

  sl_net_wifi_ap_profile_t *default_ap_profile  = get_default_ap_profile();
  sl_wifi_ap_configuration_t *default_ap_config = (sl_wifi_ap_configuration_t *)&default_ap_profile->config;

  // Update SSID length
  default_ap_config->ssid.length = cmd_input->ssid.len;

  // Copy SSID data
  memcpy(default_ap_config->ssid.value, cmd_input->ssid.data, cmd_input->ssid.len);

  // Ensure null-termination if ssid.value is used as a string
  if (cmd_input->ssid.len < sizeof(default_ap_config->ssid.value)) {
    default_ap_config->ssid.value[cmd_input->ssid.len] = '\0';
  }

  app_wifi_rsp_net_profile_set_ap_ssid(SL_STATUS_OK);
  return;
}

void app_wifi_cmd_net_profile_set_client_ssid(app_wifi_cmd_net_profile_set_client_ssid_t *cmd_input)
{
  if (cmd_input == NULL) {
    app_wifi_rsp_net_profile_set_client_ssid(SL_STATUS_INVALID_PARAMETER);
    return;
  }

  sl_net_wifi_client_profile_t *default_client_profile = get_default_client_profile();
  sl_wifi_client_configuration_t *default_client_config =
    (sl_wifi_client_configuration_t *)&default_client_profile->config;

  // Update SSID length
  default_client_config->ssid.length = cmd_input->ssid.len;

  // Copy SSID data
  memcpy(default_client_config->ssid.value, cmd_input->ssid.data, cmd_input->ssid.len);

  // Ensure null-termination if ssid.value is used as a string
  if (cmd_input->ssid.len < sizeof(default_client_config->ssid.value)) {
    default_client_config->ssid.value[cmd_input->ssid.len] = '\0';
  }

  app_wifi_rsp_net_profile_set_client_ssid(SL_STATUS_OK);
  return;
}

void app_wifi_cmd_net_profile_set_ap_config(app_wifi_cmd_net_profile_set_ap_config_t *cmd_input)
{

  if (cmd_input == NULL) {
    app_wifi_rsp_net_profile_set_ap_config(SL_STATUS_INVALID_PARAMETER);
    return;
  }

  sl_net_wifi_ap_profile_t *default_ap_profile  = get_default_ap_profile();
  sl_wifi_ap_configuration_t *default_ap_config = (sl_wifi_ap_configuration_t *)&default_ap_profile->config;

  default_ap_config->security            = cmd_input->ap_security;
  default_ap_config->encryption          = cmd_input->ap_encryption;
  default_ap_config->rate_protocol       = cmd_input->ap_rate_protocol;
  default_ap_config->options             = cmd_input->ap_options;
  default_ap_config->credential_id       = cmd_input->ap_credential_id;
  default_ap_config->keepalive_type      = cmd_input->ap_keepalive_type;
  default_ap_config->beacon_interval     = cmd_input->ap_beacon_interval;
  default_ap_config->client_idle_timeout = cmd_input->ap_client_idle_timeout;
  default_ap_config->dtim_beacon_count   = cmd_input->ap_dtim_beacon_count;
  default_ap_config->maximum_clients     = cmd_input->ap_maximum_clients;
  default_ap_config->beacon_stop         = cmd_input->ap_beacon_stop;
  default_ap_config->tdi_flags           = cmd_input->ap_tdi;
  default_ap_config->is_11n_enabled      = cmd_input->ap_is_11n_enabled;

  app_wifi_rsp_net_profile_set_ap_config(SL_STATUS_OK);
  return;
}

void app_wifi_cmd_net_profile_set_client_config(app_wifi_cmd_net_profile_set_client_config_t *cmd_input)
{

  if (cmd_input == NULL) {
    app_wifi_rsp_net_profile_set_client_config(SL_STATUS_INVALID_PARAMETER);
    return;
  }

  sl_net_wifi_client_profile_t *default_client_profile = get_default_client_profile();
  sl_wifi_client_configuration_t *default_client_config =
    (sl_wifi_client_configuration_t *)&default_client_profile->config;

  default_client_config->security   = cmd_input->client_security;
  default_client_config->encryption = cmd_input->client_encryption;
  memcpy(&default_client_config->bssid, &cmd_input->bssid, sizeof(app_wifi_bssid_t));
  default_client_config->client_options                    = cmd_input->client_options;
  default_client_config->credential_id                     = cmd_input->credential_id;
  default_client_config->channel_bitmap.channel_bitmap_2_4 = cmd_input->channel_bitmap_2_4g;
  default_client_config->bss_type                          = cmd_input->bss_type;

  app_wifi_rsp_net_profile_set_client_config(SL_STATUS_OK);
  return;
}

void app_wifi_cmd_net_profile_set_ap_channel(app_wifi_cmd_net_profile_set_ap_channel_t *cmd_input)
{
  if (cmd_input == NULL) {
    app_wifi_rsp_net_profile_set_ap_channel(SL_STATUS_INVALID_PARAMETER);
    return;
  }

  sl_net_wifi_ap_profile_t *default_ap_profile  = get_default_ap_profile();
  sl_wifi_ap_configuration_t *default_ap_config = (sl_wifi_ap_configuration_t *)&default_ap_profile->config;

  default_ap_config->channel.channel   = cmd_input->channel;
  default_ap_config->channel.band      = cmd_input->band;
  default_ap_config->channel.bandwidth = cmd_input->bandwidth;

  app_wifi_rsp_net_profile_set_ap_channel(SL_STATUS_OK);
  return;
}

void app_wifi_cmd_net_profile_set_client_channel(app_wifi_cmd_net_profile_set_client_channel_t *cmd_input)
{
  if (cmd_input == NULL) {
    app_wifi_rsp_net_profile_set_client_channel(SL_STATUS_INVALID_PARAMETER);
    return;
  }

  sl_net_wifi_client_profile_t *default_client_profile = get_default_client_profile();
  sl_wifi_client_configuration_t *default_client_config =
    (sl_wifi_client_configuration_t *)&default_client_profile->config;

  default_client_config->channel.channel   = cmd_input->channel;
  default_client_config->channel.band      = cmd_input->band;
  default_client_config->channel.bandwidth = cmd_input->bandwidth;

  app_wifi_rsp_net_profile_set_client_channel(SL_STATUS_OK);
  return;
}

void app_wifi_cmd_net_profile_set_ap_ip_config(app_wifi_cmd_net_profile_set_ap_ip_config_t *cmd_input)
{
  if (cmd_input == NULL) {
    app_wifi_rsp_net_profile_set_ap_ip_config(SL_STATUS_INVALID_PARAMETER);
    return;
  }

  sl_net_wifi_ap_profile_t *default_ap_profile    = get_default_ap_profile();
  sl_net_ip_configuration_t *default_ap_ip_config = (sl_net_ip_configuration_t *)&default_ap_profile->ip;

  default_ap_ip_config->mode = cmd_input->mode;
  default_ap_ip_config->type = cmd_input->type;

  app_wifi_rsp_net_profile_set_ap_ip_config(SL_STATUS_OK);
  return;
}

void app_wifi_cmd_net_profile_set_ap_ipv4_config(app_wifi_cmd_net_profile_set_ap_ipv4_config_t *cmd_input)
{
  if (cmd_input == NULL) {
    app_wifi_rsp_net_profile_set_ap_ipv4_config(SL_STATUS_INVALID_PARAMETER);
    return;
  }

  sl_net_wifi_ap_profile_t *default_ap_profile    = get_default_ap_profile();
  sl_net_ip_configuration_t *default_ap_ip_config = (sl_net_ip_configuration_t *)&default_ap_profile->ip;

  // The message is received with the byte order reversed, so it is being aligned to the correct byte order.
  // Set the IP address
  default_ap_ip_config->ip.v4.ip_address.bytes[0] = cmd_input->ipv4_address.addr[0];
  default_ap_ip_config->ip.v4.ip_address.bytes[1] = cmd_input->ipv4_address.addr[1];
  default_ap_ip_config->ip.v4.ip_address.bytes[2] = cmd_input->ipv4_address.addr[2];
  default_ap_ip_config->ip.v4.ip_address.bytes[3] = cmd_input->ipv4_address.addr[3];

  // Set the gateway IP address
  default_ap_ip_config->ip.v4.gateway.bytes[0] = cmd_input->gateway.addr[0];
  default_ap_ip_config->ip.v4.gateway.bytes[1] = cmd_input->gateway.addr[1];
  default_ap_ip_config->ip.v4.gateway.bytes[2] = cmd_input->gateway.addr[2];
  default_ap_ip_config->ip.v4.gateway.bytes[3] = cmd_input->gateway.addr[3];

  // Set the subnet mask
  default_ap_ip_config->ip.v4.netmask.bytes[0] = cmd_input->netmask.addr[0];
  default_ap_ip_config->ip.v4.netmask.bytes[1] = cmd_input->netmask.addr[1];
  default_ap_ip_config->ip.v4.netmask.bytes[2] = cmd_input->netmask.addr[2];
  default_ap_ip_config->ip.v4.netmask.bytes[3] = cmd_input->netmask.addr[3];

  app_wifi_rsp_net_profile_set_ap_ipv4_config(SL_STATUS_OK);
  return;
}

void app_wifi_cmd_net_profile_set_ap_ipv6_config(app_wifi_cmd_net_profile_set_ap_ipv6_config_t *cmd_input)
{
  if (cmd_input == NULL) {
    app_wifi_rsp_net_profile_set_ap_ipv6_config(SL_STATUS_INVALID_PARAMETER);
    return;
  }

  sl_net_wifi_ap_profile_t *default_ap_profile    = get_default_ap_profile();
  sl_net_ip_configuration_t *default_ap_ip_config = (sl_net_ip_configuration_t *)&default_ap_profile->ip;

  default_ap_ip_config->ip.v6.link_local_address.bytes[0]  = cmd_input->link_local_address.addr[0];
  default_ap_ip_config->ip.v6.link_local_address.bytes[1]  = cmd_input->link_local_address.addr[1];
  default_ap_ip_config->ip.v6.link_local_address.bytes[2]  = cmd_input->link_local_address.addr[2];
  default_ap_ip_config->ip.v6.link_local_address.bytes[3]  = cmd_input->link_local_address.addr[3];
  default_ap_ip_config->ip.v6.link_local_address.bytes[4]  = cmd_input->link_local_address.addr[4];
  default_ap_ip_config->ip.v6.link_local_address.bytes[5]  = cmd_input->link_local_address.addr[5];
  default_ap_ip_config->ip.v6.link_local_address.bytes[6]  = cmd_input->link_local_address.addr[6];
  default_ap_ip_config->ip.v6.link_local_address.bytes[7]  = cmd_input->link_local_address.addr[7];
  default_ap_ip_config->ip.v6.link_local_address.bytes[8]  = cmd_input->link_local_address.addr[8];
  default_ap_ip_config->ip.v6.link_local_address.bytes[9]  = cmd_input->link_local_address.addr[9];
  default_ap_ip_config->ip.v6.link_local_address.bytes[10] = cmd_input->link_local_address.addr[10];
  default_ap_ip_config->ip.v6.link_local_address.bytes[11] = cmd_input->link_local_address.addr[11];
  default_ap_ip_config->ip.v6.link_local_address.bytes[12] = cmd_input->link_local_address.addr[12];
  default_ap_ip_config->ip.v6.link_local_address.bytes[13] = cmd_input->link_local_address.addr[13];
  default_ap_ip_config->ip.v6.link_local_address.bytes[14] = cmd_input->link_local_address.addr[14];
  default_ap_ip_config->ip.v6.link_local_address.bytes[15] = cmd_input->link_local_address.addr[15];

  default_ap_ip_config->ip.v6.global_address.bytes[0]  = cmd_input->global_address.addr[0];
  default_ap_ip_config->ip.v6.global_address.bytes[1]  = cmd_input->global_address.addr[1];
  default_ap_ip_config->ip.v6.global_address.bytes[2]  = cmd_input->global_address.addr[2];
  default_ap_ip_config->ip.v6.global_address.bytes[3]  = cmd_input->global_address.addr[3];
  default_ap_ip_config->ip.v6.global_address.bytes[4]  = cmd_input->global_address.addr[4];
  default_ap_ip_config->ip.v6.global_address.bytes[5]  = cmd_input->global_address.addr[5];
  default_ap_ip_config->ip.v6.global_address.bytes[6]  = cmd_input->global_address.addr[6];
  default_ap_ip_config->ip.v6.global_address.bytes[7]  = cmd_input->global_address.addr[7];
  default_ap_ip_config->ip.v6.global_address.bytes[8]  = cmd_input->global_address.addr[8];
  default_ap_ip_config->ip.v6.global_address.bytes[9]  = cmd_input->global_address.addr[9];
  default_ap_ip_config->ip.v6.global_address.bytes[10] = cmd_input->global_address.addr[10];
  default_ap_ip_config->ip.v6.global_address.bytes[11] = cmd_input->global_address.addr[11];
  default_ap_ip_config->ip.v6.global_address.bytes[12] = cmd_input->global_address.addr[12];
  default_ap_ip_config->ip.v6.global_address.bytes[13] = cmd_input->global_address.addr[13];
  default_ap_ip_config->ip.v6.global_address.bytes[14] = cmd_input->global_address.addr[14];
  default_ap_ip_config->ip.v6.global_address.bytes[15] = cmd_input->global_address.addr[15];

  default_ap_ip_config->ip.v6.gateway.bytes[0]  = cmd_input->gateway_address.addr[0];
  default_ap_ip_config->ip.v6.gateway.bytes[1]  = cmd_input->gateway_address.addr[1];
  default_ap_ip_config->ip.v6.gateway.bytes[2]  = cmd_input->gateway_address.addr[2];
  default_ap_ip_config->ip.v6.gateway.bytes[3]  = cmd_input->gateway_address.addr[3];
  default_ap_ip_config->ip.v6.gateway.bytes[4]  = cmd_input->gateway_address.addr[4];
  default_ap_ip_config->ip.v6.gateway.bytes[5]  = cmd_input->gateway_address.addr[5];
  default_ap_ip_config->ip.v6.gateway.bytes[6]  = cmd_input->gateway_address.addr[6];
  default_ap_ip_config->ip.v6.gateway.bytes[7]  = cmd_input->gateway_address.addr[7];
  default_ap_ip_config->ip.v6.gateway.bytes[8]  = cmd_input->gateway_address.addr[8];
  default_ap_ip_config->ip.v6.gateway.bytes[9]  = cmd_input->gateway_address.addr[9];
  default_ap_ip_config->ip.v6.gateway.bytes[10] = cmd_input->gateway_address.addr[10];
  default_ap_ip_config->ip.v6.gateway.bytes[11] = cmd_input->gateway_address.addr[11];
  default_ap_ip_config->ip.v6.gateway.bytes[12] = cmd_input->gateway_address.addr[12];
  default_ap_ip_config->ip.v6.gateway.bytes[13] = cmd_input->gateway_address.addr[13];
  default_ap_ip_config->ip.v6.gateway.bytes[14] = cmd_input->gateway_address.addr[14];
  default_ap_ip_config->ip.v6.gateway.bytes[15] = cmd_input->gateway_address.addr[15];

  app_wifi_rsp_net_profile_set_ap_ipv6_config(SL_STATUS_OK);
  return;
}

void app_wifi_cmd_net_profile_get_ap_ssid(const void *nil)
{
  (void)nil;
  sl_net_wifi_ap_profile_t *default_ap_profile  = get_default_ap_profile();
  sl_wifi_ap_configuration_t *default_ap_config = (sl_wifi_ap_configuration_t *)&default_ap_profile->config;

  app_wifi_rsp_net_profile_get_ap_ssid(SL_STATUS_OK, default_ap_config->ssid.length, default_ap_config->ssid.value);
  return;
}

void app_wifi_cmd_net_profile_get_client_ssid(const void *nil)
{
  (void)nil;
  sl_net_wifi_client_profile_t *default_client_profile = get_default_client_profile();
  sl_wifi_client_configuration_t *default_client_config =
    (sl_wifi_client_configuration_t *)&default_client_profile->config;

  app_wifi_rsp_net_profile_get_client_ssid(SL_STATUS_OK,
                                           default_client_config->ssid.length,
                                           default_client_config->ssid.value);
  return;
}

void app_wifi_cmd_net_profile_get_ap_config(const void *nil)
{
  (void)nil;
  sl_net_wifi_ap_profile_t *default_ap_profile  = get_default_ap_profile();
  sl_wifi_ap_configuration_t *default_ap_config = (sl_wifi_ap_configuration_t *)&default_ap_profile->config;

  app_wifi_rsp_net_profile_get_ap_config(SL_STATUS_OK,
                                         default_ap_config->security,
                                         default_ap_config->encryption,
                                         default_ap_config->rate_protocol,
                                         default_ap_config->options,
                                         default_ap_config->credential_id,
                                         default_ap_config->keepalive_type,
                                         default_ap_config->beacon_interval,
                                         default_ap_config->client_idle_timeout,
                                         default_ap_config->dtim_beacon_count,
                                         default_ap_config->maximum_clients,
                                         default_ap_config->beacon_stop,
                                         default_ap_config->tdi_flags,
                                         default_ap_config->is_11n_enabled);
  return;
}

void app_wifi_cmd_net_profile_get_client_config(const void *nil)
{
  (void)nil;
  sl_net_wifi_client_profile_t *default_client_profile = get_default_client_profile();
  sl_wifi_client_configuration_t *default_client_config =
    (sl_wifi_client_configuration_t *)&default_client_profile->config;

  app_wifi_rsp_net_profile_get_client_config(SL_STATUS_OK,
                                             (const void *)&(default_client_config->bssid),
                                             default_client_config->bss_type,
                                             default_client_config->security,
                                             default_client_config->encryption,
                                             default_client_config->client_options,
                                             default_client_config->credential_id,
                                             default_client_config->channel_bitmap.channel_bitmap_2_4);
  return;
}

void app_wifi_cmd_net_profile_get_ap_channel(const void *nil)
{
  (void)nil;
  sl_net_wifi_ap_profile_t *default_ap_profile  = get_default_ap_profile();
  sl_wifi_ap_configuration_t *default_ap_config = (sl_wifi_ap_configuration_t *)&default_ap_profile->config;

  app_wifi_rsp_net_profile_get_ap_channel(SL_STATUS_OK,
                                          default_ap_config->channel.channel,
                                          default_ap_config->channel.band,
                                          default_ap_config->channel.bandwidth);
  return;
}

void app_wifi_cmd_net_profile_get_client_channel(const void *nil)
{
  (void)nil;
  sl_net_wifi_client_profile_t *default_client_profile = get_default_client_profile();
  sl_wifi_client_configuration_t *default_client_config =
    (sl_wifi_client_configuration_t *)&default_client_profile->config;

  app_wifi_rsp_net_profile_get_client_channel(SL_STATUS_OK,
                                              default_client_config->channel.channel,
                                              default_client_config->channel.band,
                                              default_client_config->channel.bandwidth);
  return;
}

void app_wifi_cmd_net_profile_get_ap_ip_config(const void *nil)
{
  (void)nil;
  sl_net_wifi_ap_profile_t *default_ap_profile    = get_default_ap_profile();
  sl_net_ip_configuration_t *default_ap_ip_config = (sl_net_ip_configuration_t *)&default_ap_profile->ip;

  app_wifi_rsp_net_profile_get_ap_ip_config(SL_STATUS_OK, default_ap_ip_config->mode, default_ap_ip_config->type);
  return;
}

void app_wifi_cmd_net_profile_get_ap_ipv4_config(const void *nil)
{
  (void)nil;
  sl_net_wifi_ap_profile_t *default_ap_profile    = get_default_ap_profile();
  sl_net_ip_configuration_t *default_ap_ip_config = (sl_net_ip_configuration_t *)&default_ap_profile->ip;

  app_wifi_rsp_net_profile_get_ap_ipv4_config(SL_STATUS_OK,
                                              &default_ap_ip_config->ip.v4.ip_address.bytes,
                                              &default_ap_ip_config->ip.v4.gateway.bytes,
                                              &default_ap_ip_config->ip.v4.netmask.bytes);
  return;
}

void app_wifi_cmd_net_profile_get_ap_ipv6_config(const void *nil)
{
  (void)nil;
  sl_net_wifi_ap_profile_t *default_ap_profile    = get_default_ap_profile();
  sl_net_ip_configuration_t *default_ap_ip_config = (sl_net_ip_configuration_t *)&default_ap_profile->ip;

  app_wifi_rsp_net_profile_get_ap_ipv6_config(SL_STATUS_OK,
                                              &default_ap_ip_config->ip.v6.link_local_address.bytes,
                                              &default_ap_ip_config->ip.v6.global_address.bytes,
                                              &default_ap_ip_config->ip.v6.gateway.bytes);
  return;
}
