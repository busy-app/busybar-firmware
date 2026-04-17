/***************************************************************************/ /**
 * @file
 * @brief
 *******************************************************************************
 * # License
 * <b>Copyright 2019 Silicon Laboratories Inc. www.silabs.com</b>
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
#include "sl_constants.h"
#include "sl_si91x_types.h"
#include "sl_si91x_driver.h"
#include "sl_net_si91x.h"
#include "sl_net_constants.h"
#include "sli_net_utility.h"
#include "sl_net_si91x_integration_handler.h"
#include "sli_wifi_constants.h"
#if defined(SL_WIFI_COMPONENT_INCLUDED)
#include "sl_wifi.h"
#endif
#ifndef __ZEPHYR__
#ifndef SLI_SI91X_LWIP_HOSTED_NETWORK_STACK
#ifndef SLI_SI91X_NETWORK_DUAL_STACK // These headers are included only when neither LWIP nor dual stack is used.
#include "netinet_in.h"
#include "netinet6_in6.h"
#endif
#endif
#endif
#ifdef SLI_SI91X_OFFLOAD_NETWORK_STACK
#include "sl_si91x_socket_constants.h"
#include "sl_si91x_socket_utility.h"
#include "sl_ip_types.h"
#endif

#ifdef SLI_SI91X_SOCKETS
#include "sl_si91x_socket_utility.h"
#include "sl_si91x_socket_callback_framework.h"
#endif
#include "sl_si91x_core_utilities.h"

#ifdef SLI_SI91X_EMBEDDED_MQTT_CLIENT
#include "si91x_mqtt_client_callback_framework.h"
#include "si91x_mqtt_client_utility.h"
#endif

#ifdef SLI_SI91X_INTERNAL_SNTP_CLIENT
#include "si91x_sntp_client_callback_framework.h"
#endif

#ifdef SLI_SI91X_INTERNAL_HTTP_CLIENT
#include "sl_si91x_http_client_callback_framework.h"
#endif
#include "sli_wifi_utility.h"
extern sli_wifi_command_queue_t cmd_queues[SI91X_CMD_MAX];

void sli_handle_wifi_events(const sli_si91x_queue_packet_t *data, sl_wifi_system_packet_t *packet);

#ifdef SLI_SI91X_EMBEDDED_MQTT_CLIENT
/**
 * A internal function to handle to asynchronous mqtt client events.
 */
static void sli_handle_mqtt_client_asynch_events(sli_si91x_queue_packet_t *mqtt_asyn_packet)
{
  sl_wifi_system_packet_t *raw_rx_packet =
    (sl_wifi_system_packet_t *)sli_wifi_host_get_buffer_data(mqtt_asyn_packet->host_packet, 0, NULL);
  sl_mqtt_client_t *mqtt_client;

  raw_rx_packet->desc[12] = mqtt_asyn_packet->frame_status & 0xFF;        // Lower 8 bits
  raw_rx_packet->desc[13] = (mqtt_asyn_packet->frame_status >> 8) & 0xFF; // Upper 8 bits
  //Variable to indicate whether a disconnect event is related to a keep-alive terminate error.
  bool is_keep_alive_response_related_disconnect =
    (raw_rx_packet->command == SLI_WLAN_RSP_EMB_MQTT_CLIENT
     && mqtt_asyn_packet->frame_status == (SL_STATUS_SI91X_MQTT_KEEP_ALIVE_TERMINATE_ERROR & ~BIT(16)));

  // Since these responses are unsolicited, We need to create a context for them.
  if (raw_rx_packet->command == SLI_WLAN_RSP_MQTT_REMOTE_TERMINATE
      || raw_rx_packet->command == SLI_WLAN_RSP_EMB_MQTT_PUBLISH_PKT || raw_rx_packet->command == SLI_WIFI_RSP_JOIN
      || is_keep_alive_response_related_disconnect) {

    sli_si91x_get_mqtt_client(&mqtt_client);

    if (mqtt_client == NULL) {
      // Drop MQTT client event, if the client is either connecting or disconnected
      SL_DEBUG_LOG("Dropping mqtt client event, Si91x Event: %hu", raw_rx_packet->command);
      return;
    }

    if (mqtt_client->state == SL_MQTT_CLIENT_DISCONNECTED) {
      // Drop MQTT client event disconnect, if the client is already in disconnected
      // This can happen if MQTT client is already disconnected state and NWP sends a rejoin failure event.
      SL_DEBUG_LOG("Dropping mqtt disconnect event: %hu", raw_rx_packet->command);
      return;
    }

    // Send CONNECT_FAILED_EVENT if JOIN is received during TA_INIT state
    if ((raw_rx_packet->command == SLI_WIFI_RSP_JOIN || raw_rx_packet->command == SLI_WIFI_RSP_DISCONNECT)
        && mqtt_client->state == SL_MQTT_CLIENT_TA_INIT) {
      // Build MQTT SDK context for asynchronous MQTT events
      sli_si91x_build_mqtt_sdk_context_if_async(SL_MQTT_CLIENT_CONNECTED_EVENT,
                                                mqtt_client,
                                                NULL,
                                                NULL,
                                                0,
                                                (sl_si91x_mqtt_client_context_t **)&mqtt_asyn_packet->sdk_context);
    } else {
      // Build MQTT SDK context for asynchronous MQTT events
      sli_si91x_build_mqtt_sdk_context_if_async(
        (raw_rx_packet->command == SLI_WLAN_RSP_MQTT_REMOTE_TERMINATE || raw_rx_packet->command == SLI_WIFI_RSP_JOIN
         || is_keep_alive_response_related_disconnect)
          ? SL_MQTT_CLIENT_DISCONNECTED_EVENT
          : SL_MQTT_CLIENT_MESSAGED_RECEIVED_EVENT,
        mqtt_client,
        NULL,
        NULL,
        0,
        (sl_si91x_mqtt_client_context_t **)&mqtt_asyn_packet->sdk_context);
    }
  }

  sl_si91x_mqtt_client_context_t *sdk_context = (sl_si91x_mqtt_client_context_t *)mqtt_asyn_packet->sdk_context;

  if (sdk_context == NULL) {
    return;
  }

  SL_DEBUG_LOG("sli_handle_mqtt_client_asynch_events: event %x", sdk_context->event);

  uint16_t si91x_event_status = sli_get_si91x_frame_status(raw_rx_packet);
  sl_status_t event_status    = sli_convert_and_save_firmware_status(si91x_event_status);

  // Handle MQTT events
  sli_si91x_mqtt_event_handler(event_status, sdk_context, raw_rx_packet);
}
#endif

#ifdef SLI_SI91X_EMBEDDED_MQTT_CLIENT
static bool sli_handle_mqtt_client_events(sli_si91x_queue_packet_t *data, const sl_wifi_system_packet_t *packet)
{
  // Handle MQTT client-specific events
  if (packet->command == SLI_WLAN_REQ_EMB_MQTT_CLIENT || packet->command == SLI_WLAN_RSP_EMB_MQTT_PUBLISH_PKT
      || packet->command == SLI_WLAN_RSP_MQTT_REMOTE_TERMINATE || packet->command == SLI_WIFI_RSP_JOIN) {
    sli_handle_mqtt_client_asynch_events(data);
    return true;
  }
  return false;
}
#endif

#ifdef SLI_SI91X_INTERNAL_SNTP_CLIENT
static bool sli_handle_sntp_client_events(sli_si91x_queue_packet_t *data, const sl_wifi_system_packet_t *packet)
{
  // Handle SNTP client events
  if (packet->command == SLI_WLAN_RSP_SNTP_CLIENT) {
    sli_si91x_sntp_event_handler(data);
    return true;
  }
  return false;
}
#endif

#ifdef SLI_SI91X_SOCKETS
static void sli_handle_socket_events(sli_si91x_queue_packet_t *data, sl_wifi_system_packet_t *packet)
{
  // Handle SI91X socket-related events
  bool is_socket_command =
    (packet->command == SLI_WLAN_REQ_SOCKET_ACCEPT || packet->command == SLI_WLAN_RSP_REMOTE_TERMINATE
     || packet->command == SLI_RECEIVE_RAW_DATA || packet->command == SLI_WLAN_RSP_TCP_ACK_INDICATION
     || packet->command == SLI_WLAN_RSP_SELECT_REQUEST);
  if (is_socket_command) {
    sl_wifi_system_packet_t *raw_rx_packet = packet;
    uint16_t si91x_event_status            = sli_get_si91x_frame_status(raw_rx_packet);

    sl_status_t event_status = sli_convert_and_save_firmware_status(si91x_event_status);
    sli_si91x_socket_event_handler(event_status, (sli_si91x_socket_context_t *)data->sdk_context, raw_rx_packet);
  }
}
#endif

void sl_net_si91x_event_dispatch_handler(sli_si91x_queue_packet_t *data, sl_wifi_system_packet_t *packet)
{
  sl_status_t status;
  sl_net_event_t service_event;
#ifdef SLI_SI91X_INTERNAL_HTTP_CLIENT
  sl_http_client_event_t http_event;
#endif

#ifdef SLI_SI91X_EMBEDDED_MQTT_CLIENT
  if (sli_handle_mqtt_client_events(data, packet)) {
    return;
  }
#endif

#ifdef SLI_SI91X_INTERNAL_SNTP_CLIENT
  if (sli_handle_sntp_client_events(data, packet)) {
    return;
  }
#endif

#ifdef SLI_SI91X_SOCKETS
  sli_handle_socket_events(data, packet);
#endif

  status = sli_convert_si91x_event_to_sl_net_event(&packet->command, &service_event, packet);
  if (status == SL_STATUS_OK) {
    SL_DEBUG_LOG("><<<< Got net event : %u\n", service_event);
    sl_si91x_default_handler(service_event, data->host_packet);
  }
#ifdef SLI_SI91X_INTERNAL_HTTP_CLIENT
  // Check for sl_http_client_event_t
  else if (sli_convert_si91x_event_to_sl_http_client_event(&packet->command, &http_event) == SL_STATUS_OK) {
    SL_DEBUG_LOG("\r\n>>> HTTP Event received: %u <<<\r\n", http_event);
    sli_http_client_default_event_handler(http_event, data->host_packet, data->sdk_context);
  }
#endif
}

void sli_si91x_network_cleanup_handler()
{
#ifdef SLI_SI91X_EMBEDDED_MQTT_CLIENT
  sli_mqtt_client_cleanup();
#endif
}

#ifdef SLI_SI91X_OFFLOAD_NETWORK_STACK
sl_status_t sli_si91x_flush_all_socket_tx_queues_based_on_dest_ip_address(uint16_t frame_status,
                                                                          const sl_ip_address_t *dest_ip_add,
                                                                          uint8_t vap_id)
{
  sl_status_t status;

  // Loop through all sockets
  for (uint8_t index = 0; index < SLI_NUMBER_OF_SOCKETS; index++) {
    // Check if the socket exists and matches the required VAP ID
    if ((sli_si91x_sockets[index] != NULL) && (vap_id == sli_si91x_sockets[index]->vap_id)) {
      bool is_same = 0;
      if (dest_ip_add->type == SL_IPV4) {
        const struct sockaddr_in *socket_address = (struct sockaddr_in *)&sli_si91x_sockets[index]->remote_address;
        is_same = memcmp(dest_ip_add->ip.v4.bytes, &socket_address->sin_addr.s_addr, SL_IPV4_ADDRESS_LENGTH);
      } else {
        const struct sockaddr_in6 *ipv6_socket_address = &sli_si91x_sockets[index]->remote_address;
#ifdef SLI_SI91X_NETWORK_DUAL_STACK
        is_same = memcmp(dest_ip_add->ip.v6.bytes, &ipv6_socket_address->sin6_addr.un.u8_addr, SL_IPV6_ADDRESS_LENGTH);
#else
#ifndef __ZEPHYR__
        is_same = memcmp(dest_ip_add->ip.v6.bytes,
                         &ipv6_socket_address->sin6_addr.__u6_addr.__u6_addr8,
                         SL_IPV6_ADDRESS_LENGTH);
#else
        is_same = memcmp(dest_ip_add->ip.v6.bytes, &ipv6_socket_address->sin6_addr.s6_addr, SL_IPV6_ADDRESS_LENGTH);
#endif
#endif
      }
      if (!is_same) {
        // Flush the command queues for the current socket based on queue type
        status = sli_si91x_flush_socket_command_queues_based_on_queue_type(index, frame_status);
        // If flushing fails, return the error status immediately
        VERIFY_STATUS_AND_RETURN(status);

        status = sli_si91x_flush_socket_data_queues_based_on_queue_type(index);
        // If flushing fails, return the error status immediately
        VERIFY_STATUS_AND_RETURN(status);

        // update the socket state to disconnected.
        sli_si91x_sockets[index]->state             = DISCONNECTED;
        sli_si91x_sockets[index]->disconnect_reason = SLI_SI91X_BSD_DISCONNECT_REASON_INTERFACE_DOWN;
      }
    }
  }
  // Return SL_STATUS_OK if all sockets were processed successfully
  return SL_STATUS_OK;
}

void sli_si91x_flush_third_party_station_dependent_sockets(const sli_si91x_ap_disconnect_resp_t *ap_disconnect_resp)
{

  sl_ip_address_t dest_ip_add = { 0 };
  uint8_t vap_id = (SL_WIFI_CONCURRENT_MODE == sli_wifi_get_opermode()) ? SL_WIFI_AP_VAP_ID : SL_WIFI_CLIENT_VAP_ID;
  // IPv4 Address Handling
  if (ap_disconnect_resp->flag & BIT(0)) {
    dest_ip_add.type = SL_IPV4;
    memcpy(dest_ip_add.ip.v4.bytes, ap_disconnect_resp->ipv4_address, SL_IPV4_ADDRESS_LENGTH);
    if (!sli_wifi_is_ip_address_zero(&dest_ip_add)) {
      sli_si91x_flush_all_socket_tx_queues_based_on_dest_ip_address((uint16_t)SL_STATUS_WIFI_CONNECTION_LOST,
                                                                    &dest_ip_add,
                                                                    vap_id);
    }
  }

  // IPv6 Link-Local Address Handling
  if (ap_disconnect_resp->flag & BIT(1)) {
    dest_ip_add.type = SL_IPV6;
    memcpy(dest_ip_add.ip.v6.bytes, ap_disconnect_resp->link_local_address, SL_IPV6_ADDRESS_LENGTH);
    if (!sli_wifi_is_ip_address_zero(&dest_ip_add)) {
      sli_si91x_flush_all_socket_tx_queues_based_on_dest_ip_address((uint16_t)SL_STATUS_WIFI_CONNECTION_LOST,
                                                                    &dest_ip_add,
                                                                    vap_id);
    }
  }

  // IPv6 Global Address Handling
  if (ap_disconnect_resp->flag & BIT(2)) {
    dest_ip_add.type = SL_IPV6;
    memcpy(dest_ip_add.ip.v6.bytes, ap_disconnect_resp->global_address, SL_IPV6_ADDRESS_LENGTH);
    if (!sli_wifi_is_ip_address_zero(&dest_ip_add)) {
      sli_si91x_flush_all_socket_tx_queues_based_on_dest_ip_address((uint16_t)SL_STATUS_WIFI_CONNECTION_LOST,
                                                                    &dest_ip_add,
                                                                    vap_id);
    }
  }
}
#endif
