/***************************************************************************//**
 * @file sl_wisun_br_si917.c
 * @brief Use SI917 for Wi-FI connectivity
 *******************************************************************************
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
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

#include <inttypes.h>
#include "sl_status.h"
#include "sl_wifi.h"
#include "sl_wifi_credentials.h"
#include "sl_wifi_callback_framework.h"
#include "sl_rsi_utility.h"
#include "sl_wisun_br_lwip.h"
#include "sl_wisun_br_wifi.h"
#include "sl_wisun_trace_api.h"

#define WIFI_CREDENTIAL_ID  0

static sl_mac_address_t mac_address;
static sl_wisun_br_wifi_join_handler_t wifi_join_handler;

sl_status_t sl_si91x_host_process_data_frame(sl_wifi_interface_t interface, sl_wifi_buffer_t *buffer)
{
  sl_wifi_system_packet_t *packet;

  (void)interface;
  packet = (sl_wifi_system_packet_t *)sl_si91x_host_get_buffer_data(buffer, 0, NULL);
  if (!packet) {
    // Discard received data frame
    return SL_STATUS_OK;
  }

  return sl_wisun_br_lwip_wan_input(packet->data, packet->length);
}

static void wifi_on_wan_link_state_changed(bool link_up)
{
  if (link_up) {
    if (wifi_join_handler) {
      wifi_join_handler(true);
    }
  } else {
    if (wifi_join_handler) {
      wifi_join_handler(false);
    }
  }
}

static sl_status_t wifi_on_wan_output(const uint8_t *data, size_t data_length)
{
  return sl_wifi_send_raw_data_frame(SL_WIFI_CLIENT_INTERFACE, data, data_length);
}

static sl_status_t wifi_on_join(sl_wifi_event_t event, char *data, uint32_t data_length, void *arg)
{
  (void)event;
  (void)data_length;
  (void)arg;

  if (*data == 'C') {
    sl_wisun_br_lwip_wan_up(mac_address.octet);
  } else {
    if (wifi_join_handler) {
      wifi_join_handler(false);
    }
    sl_wisun_br_lwip_wan_down();
  }

  return SL_STATUS_OK;
}

sl_status_t sl_wisun_br_wifi_connect(const uint8_t *ssid,
                                     uint8_t ssid_length,
                                     wifi_security_type_t security_type,
                                     const uint8_t *passphrase)
{
  sl_status_t status;
  sl_wifi_client_configuration_t wifi_config = { 0 };

  status = sl_wifi_set_credential(WIFI_CREDENTIAL_ID,
                                  SL_WIFI_PSK_CREDENTIAL,
                                  passphrase,
                                  sizeof(sl_wifi_psk_credential_t));
  if (status != SL_STATUS_OK) {
    sl_wisun_trace_error("sl_wisun_br_wifi_connect: sl_wifi_set_credential error 0x%08"PRIx32, status);
    return status;
  }

  switch (security_type) {
    //case WIFI_SECURITY_TYPE_NONE:
    //  wifi_config.security = SL_WIFI_OPEN;
    //  wifi_config.encryption = SL_WIFI_NO_ENCRYPTION;
    //  break;
    //case WIFI_SECURITY_TYPE_WPA_PSK:
    //  wifi_config.security = SL_WIFI_WPA;
    //  wifi_config.encryption = SL_WIFI_DEFAULT_ENCRYPTION;
    //  break;
    case WIFI_SECURITY_TYPE_WPA2_TKIP:
      wifi_config.security = SL_WIFI_WPA2;
      wifi_config.encryption = SL_WIFI_TKIP_ENCRYPTION;
      break;
    case WIFI_SECURITY_TYPE_WPA2_CCMP:
      wifi_config.security = SL_WIFI_WPA2;
      wifi_config.encryption = SL_WIFI_CCMP_ENCRYPTION;
      break;
    default:
      sl_wisun_trace_error("sl_wisun_br_wifi_connect: unsupported security type %u", security_type);
      return SL_STATUS_NOT_SUPPORTED;
  }

  wifi_config.bss_type = SL_WIFI_BSS_TYPE_INFRASTRUCTURE;
  wifi_config.credential_id = WIFI_CREDENTIAL_ID;
  memcpy(wifi_config.ssid.value, ssid, ssid_length);
  wifi_config.ssid.length = ssid_length;

  status = sl_wifi_connect(SL_WIFI_CLIENT_INTERFACE, &wifi_config, 0);
  if (status != SL_STATUS_IN_PROGRESS) {
    sl_wisun_trace_error("sl_wisun_br_wifi_connect: sl_wifi_connect error 0x%08"PRIx32, status);
    return status;
  }

  return SL_STATUS_OK;
}

sl_status_t sl_wisun_br_wifi_disconnect(void)
{
  sl_status_t status;
  status = sl_wifi_disconnect(SL_WIFI_CLIENT_INTERFACE);
  if (status == SL_STATUS_OK) {
    sl_wisun_br_lwip_wan_down();
  }
  return status;
}

sl_status_t sl_wisun_br_wifi_get_info(bool *connected,
                                      uint16_t *channel_number,
                                      uint8_t mac_address[6],
                                      uint8_t ipv6_address[16])
{
  sl_status_t status;
  sl_wifi_interface_info_t info;

  status = sl_wifi_get_interface_info(SL_WIFI_CLIENT_INTERFACE, &info);
  if (status != SL_STATUS_OK) {
    return status;
  }

  *connected = info.wlan_state;
  *channel_number = info.channel_number;
  memcpy(mac_address, info.mac_address, 6);

  status = sl_wisun_br_lwip_get_wan_address(ipv6_address);
  if (status != SL_STATUS_OK) {
    memset(ipv6_address, 0, 16);
  }

  return SL_STATUS_OK;
}

sl_status_t sl_wisun_br_wifi_set_join_handler(sl_wisun_br_wifi_join_handler_t handler)
{
  wifi_join_handler = handler;
  return SL_STATUS_OK;
}

sl_status_t sl_wisun_br_wifi_init(void)
{
  sl_status_t status;
  sl_wifi_device_configuration_t device_config = sl_wifi_default_client_configuration;

  device_config.region_code = EU,
  device_config.boot_config.feature_bit_map |= SL_SI91X_FEAT_AGGREGATION;
  device_config.boot_config.tcp_ip_feature_bit_map = SL_SI91X_TCP_IP_FEAT_BYPASS;
  device_config.boot_config.custom_feature_bit_map |= SL_SI91X_CUSTOM_FEAT_ASYNC_CONNECTION_STATUS;

  status = sl_wifi_init(&device_config, NULL, sl_wifi_default_event_handler);
  if (status != SL_STATUS_OK) {
    sl_wisun_trace_error("sl_wisun_br_wifi_init: sl_wifi_init error 0x%08"PRIx32, status);
    return status;
  }

  status = sl_wifi_get_mac_address(SL_WIFI_CLIENT_INTERFACE, &mac_address);
  if (status != SL_STATUS_OK) {
    sl_wisun_trace_error("sl_wisun_br_wifi_init: sl_wifi_get_mac_address error 0x%08"PRIx32, status);
    return status;
  }

  sl_wifi_set_join_callback(wifi_on_join, NULL);
  sl_wisun_br_lwip_set_wan_output_handler(wifi_on_wan_output);
  sl_wisun_br_lwip_set_wan_link_state_handler(wifi_on_wan_link_state_changed);

  return SL_STATUS_OK;
}
