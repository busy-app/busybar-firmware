/********************************************************************************
 * @file  app_xapi_net_intf.c
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
#include "sl_wifi_callback_framework.h"
#include "sl_si91x_driver.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "app_xapi_cmd_event.h"
#include "app_xapi_global.h"

DEFINE_XAPI_CLASS(wifi, net_intf, WIFI, NET_INTF, NULL, NULL);

static sl_mac_address_t device_mac_address;

#define NET_INTERFACE_INVALID 0x00

static int init_status_interface = NET_INTERFACE_INVALID;
static sl_status_t app_network_event_handler(sl_net_event_t event,
                                             sl_status_t status,
                                             void *data,
                                             uint32_t data_length);

// Network event handler function
static sl_status_t app_network_event_handler(sl_net_event_t event, sl_status_t status, void *data, uint32_t data_length)
{
  // Suppress unused parameter warnings
  UNUSED_PARAMETER(data_length);

  switch (event) {
    case SL_NET_PING_RESPONSE_EVENT: {

      // Cast the data to the ping response structure
      sl_net_ping_response_t *response = (sl_net_ping_response_t *)data;

      // Initialize an IPv4 address structure
      ipv4_addr ipv4_address = { 0 };

      // If the status is not OK, trigger the ping response event with an error
      if (status != SL_STATUS_OK) {
        app_wifi_evt_ping_ipv4_response_event(status, 0, 0, &ipv4_address);
        break;
      }

      // Copy IPv4 address from the ping response
      ipv4_address.addr[0] = response->ping_address.ipv4_address[0];
      ipv4_address.addr[1] = response->ping_address.ipv4_address[1];
      ipv4_address.addr[2] = response->ping_address.ipv4_address[2];
      ipv4_address.addr[3] = response->ping_address.ipv4_address[3];

      // Trigger the ping response event with the IPv4 address and other details
      app_wifi_evt_ping_ipv4_response_event(status, response->ip_version, response->ping_size, &ipv4_address);
      break;
    }
    default:
      // Handle other events if necessary
      break;
  }

  return SL_STATUS_OK;
}

void app_wifi_cmd_net_intf_init(app_wifi_cmd_net_intf_init_t *msg)
{
  sl_status_t status = SL_STATUS_FAIL;

  if (msg->net_interface == APP_WIFI_NET_INTF_INTERFACE_AP) {
    status = sl_net_init(SL_NET_WIFI_AP_INTERFACE, get_default_device_configuration(), NULL, app_network_event_handler);
    if (status == SL_STATUS_OK) {
      init_status_interface = SL_NET_WIFI_AP_INTERFACE;
    }
  } else if (msg->net_interface == APP_WIFI_NET_INTF_INTERFACE_CLIENT) {
    status =
      sl_net_init(SL_NET_WIFI_CLIENT_INTERFACE, get_default_device_configuration(), NULL, app_network_event_handler);
    if (status == SL_STATUS_OK) {
      init_status_interface = SL_NET_WIFI_CLIENT_INTERFACE;
    }
  } else {
    status = SL_STATUS_NOT_SUPPORTED;
  }
  app_wifi_rsp_net_intf_init(status);
  return;
}

void app_wifi_cmd_net_intf_deinit(app_wifi_cmd_net_intf_deinit_t *msg)
{
  sl_status_t status = SL_STATUS_NOT_INITIALIZED;

  // Deinitialize only if the device is initialized and the requested interface matches
  if (init_status_interface != NET_INTERFACE_INVALID && msg->net_interface == init_status_interface) {
    status = sl_net_deinit(msg->net_interface);
  }

  // Reset init_status_interface flag and default network interface if deinit is successful
  if (status == SL_STATUS_OK) {
    init_status_interface = NET_INTERFACE_INVALID;
  }

  // Call response function with the status
  app_wifi_rsp_net_intf_deinit(status);
}

void app_wifi_cmd_net_intf_up(app_wifi_cmd_net_intf_up_t *cmd_input)
{
  // Fetch the queue handle using the getter API
  QueueHandle_t queue = wifi_cmd_get_queue_handle();

  // Check if the queue is valid
  if (queue == NULL) {
    app_wifi_evt_system_error(SL_STATUS_NOT_INITIALIZED, 0, NULL);
    app_wifi_rsp_net_intf_up(SL_STATUS_FAIL);
    return;
  }

  // Create an event_data_t structure to pass event and data
  event_data_t *event_data = malloc(sizeof(event_data_t));

  if (event_data == NULL) {
    // Handle memory allocation failure if necessary
    app_wifi_rsp_net_intf_up(SL_STATUS_ALLOCATION_FAILED);
    return;
  }

  // Allocate memory for the data (cmd_input)
  event_data->data = malloc(sizeof(app_wifi_cmd_net_intf_up_t));
  if (event_data->data == NULL) {
    // Handle memory allocation failure for the cmd_input data
    free(event_data);
    app_wifi_rsp_net_intf_up(SL_STATUS_ALLOCATION_FAILED);
    return;
  }

  // Set the event_id for this event
  event_data->event_id = APP_NET_UP_EVENT;

  // Copy the cmd_input data into the event_data->data
  memcpy(event_data->data, cmd_input, sizeof(app_wifi_cmd_net_intf_up_t));

  // Send the event data to the queue
  if (xQueueSend(queue, &event_data, portMAX_DELAY) != pdPASS) {
    free(event_data->data);
    free(event_data);
    app_wifi_rsp_net_intf_up(SL_STATUS_FAIL);
    return;
  }

  // Respond with the status after sending the event
  app_wifi_rsp_net_intf_up(SL_STATUS_OK);
  return;
}

void app_wifi_cmd_net_intf_down(app_wifi_cmd_net_intf_down_t *cmd_input)
{
  sl_status_t status = SL_STATUS_FAIL;

  status = sl_net_down(cmd_input->net_interface);

  app_wifi_rsp_net_intf_down(status);
  if (SL_STATUS_OK == status) {
    // 0 for invalid state
    set_default_net_interface(NET_INTERFACE_INVALID);
  }
  return;
}

void app_wifi_cmd_net_intf_set_device_config(app_wifi_cmd_net_intf_set_device_config_t *cmd_input)
{
  sl_status_t status = SL_STATUS_FAIL;

  sl_wifi_device_configuration_t *device_config_client = get_default_device_configuration();

  device_config_client->mac_address = &device_mac_address;

  if (cmd_input->mac_address.addr[0] == 0 && cmd_input->mac_address.addr[1] == 0 && cmd_input->mac_address.addr[2] == 0
      && cmd_input->mac_address.addr[3] == 0 && cmd_input->mac_address.addr[4] == 0
      && cmd_input->mac_address.addr[5] == 0) {
    device_config_client->mac_address = NULL;
  } else {
    memcpy(device_config_client->mac_address->octet, cmd_input->mac_address.addr, 6);
  }

  device_config_client->boot_option                         = cmd_input->boot_option;
  device_config_client->band                                = cmd_input->band;
  device_config_client->region_code                         = cmd_input->region_code;
  device_config_client->ta_pool.tx_ratio_in_buffer_pool     = cmd_input->tx_ratio_in_buffer_pool;
  device_config_client->ta_pool.rx_ratio_in_buffer_pool     = cmd_input->rx_ratio_in_buffer_pool;
  device_config_client->ta_pool.global_ratio_in_buffer_pool = cmd_input->global_ratio_in_buffer_pool;
  device_config_client->efuse_data_type                     = cmd_input->efuse_data_type;
  status                                                    = SL_STATUS_OK;

  app_wifi_rsp_net_intf_set_device_config(status);
  return;
}

void app_wifi_cmd_net_intf_get_init_status(app_wifi_cmd_net_intf_get_init_status_t *cmd_input)
{
  sl_status_t status = SL_STATUS_NOT_INITIALIZED;

  if (init_status_interface != NET_INTERFACE_INVALID && cmd_input->net_interface == init_status_interface) {
    status = SL_STATUS_OK;
  }
  app_wifi_rsp_net_intf_get_init_status(SL_STATUS_OK, (status == SL_STATUS_OK) ? 1 : 0);
}

void app_wifi_cmd_net_intf_get_network_status(app_wifi_cmd_net_intf_get_network_status_t *cmd_input)
{
  sl_status_t status                   = SL_STATUS_FAIL;
  sl_net_interface_t current_interface = get_default_net_interface();

  // Deinitialize only if the device is initialized and the requested interface matches
  if (current_interface != NET_INTERFACE_INVALID && cmd_input->net_interface == current_interface) {
    status = SL_STATUS_OK;
  }

  app_wifi_rsp_net_intf_get_network_status(SL_STATUS_OK, (status == SL_STATUS_OK) ? 1 : 0);
  return;
}

void app_wifi_cmd_net_intf_set_si91x_boot_config(app_wifi_cmd_net_intf_set_si91x_boot_config_t *cmd_input)
{
  sl_status_t status = SL_STATUS_FAIL;

  sl_wifi_system_boot_configuration_t *boot_config_client = get_default_device_boot_configuration();
  boot_config_client->oper_mode                           = cmd_input->oper_mode;
  boot_config_client->coex_mode                           = cmd_input->coex_mode;
  boot_config_client->ble_ext_feature_bit_map             = cmd_input->ble_ext_feature_bit_map;
  boot_config_client->ble_feature_bit_map                 = cmd_input->ble_feature_bit_map;
  boot_config_client->bt_feature_bit_map                  = cmd_input->bt_feature_bit_map;
  boot_config_client->config_feature_bit_map              = cmd_input->config_feature_bit_map;
  boot_config_client->custom_feature_bit_map              = cmd_input->custom_feature_bit_map;
  boot_config_client->ext_custom_feature_bit_map          = cmd_input->ext_custom_feature_bit_map;
  boot_config_client->ext_tcp_ip_feature_bit_map          = cmd_input->ext_tcp_ip_feature_bit_map;
  boot_config_client->feature_bit_map                     = cmd_input->feature_bit_map;
  boot_config_client->tcp_ip_feature_bit_map              = cmd_input->tcp_ip_feature_bit_map;
  status                                                  = SL_STATUS_OK;

  app_wifi_rsp_net_intf_set_si91x_boot_config(status);
  return;
}

void app_wifi_cmd_net_intf_get_device_config(const void *nil)
{
  (void)nil;

  sl_wifi_device_configuration_t *device_config_client = get_default_device_configuration();
  sl_mac_address_t default_mac_address                 = { 0 };

  // Use default MAC address if the device_config_client->mac_address is NULL
  sl_mac_address_t *mac_address = device_config_client->mac_address ? device_config_client->mac_address
                                                                    : &default_mac_address;
  app_wifi_rsp_net_intf_get_device_config(SL_STATUS_OK,
                                          device_config_client->boot_option,
                                          mac_address,
                                          device_config_client->band,
                                          device_config_client->region_code,
                                          device_config_client->ta_pool.tx_ratio_in_buffer_pool,
                                          device_config_client->ta_pool.rx_ratio_in_buffer_pool,
                                          device_config_client->ta_pool.global_ratio_in_buffer_pool,
                                          device_config_client->efuse_data_type);

  return;
}

void app_wifi_cmd_net_intf_get_si91x_boot_config(const void *nil)
{

  (void)nil;

  sl_wifi_system_boot_configuration_t *boot_config_client = get_default_device_boot_configuration();
  app_wifi_rsp_net_intf_get_si91x_boot_config(SL_STATUS_OK,
                                              boot_config_client->oper_mode,
                                              boot_config_client->coex_mode,
                                              boot_config_client->feature_bit_map,
                                              boot_config_client->tcp_ip_feature_bit_map,
                                              boot_config_client->custom_feature_bit_map,
                                              boot_config_client->ext_custom_feature_bit_map,
                                              boot_config_client->bt_feature_bit_map,
                                              boot_config_client->ext_tcp_ip_feature_bit_map,
                                              boot_config_client->ble_feature_bit_map,
                                              boot_config_client->ble_ext_feature_bit_map,
                                              boot_config_client->config_feature_bit_map);

  return;
}

static void copy_wifi_device_configuration(sl_wifi_device_configuration_t *device_config,
                                           const sl_wifi_device_configuration_t *default_config)
{
  // Assign core structure values
  device_config->boot_option     = default_config->boot_option;
  device_config->band            = default_config->band;
  device_config->region_code     = default_config->region_code;
  device_config->efuse_data_type = default_config->efuse_data_type;

  // Copy boot_config structure
  device_config->boot_config.oper_mode                  = default_config->boot_config.oper_mode;
  device_config->boot_config.coex_mode                  = default_config->boot_config.coex_mode;
  device_config->boot_config.feature_bit_map            = default_config->boot_config.feature_bit_map;
  device_config->boot_config.tcp_ip_feature_bit_map     = default_config->boot_config.tcp_ip_feature_bit_map;
  device_config->boot_config.custom_feature_bit_map     = default_config->boot_config.custom_feature_bit_map;
  device_config->boot_config.ext_custom_feature_bit_map = default_config->boot_config.ext_custom_feature_bit_map;
  device_config->boot_config.bt_feature_bit_map         = default_config->boot_config.bt_feature_bit_map;
  device_config->boot_config.ext_tcp_ip_feature_bit_map = default_config->boot_config.ext_tcp_ip_feature_bit_map;
  device_config->boot_config.ble_feature_bit_map        = default_config->boot_config.ble_feature_bit_map;
  device_config->boot_config.ble_ext_feature_bit_map    = default_config->boot_config.ble_ext_feature_bit_map;
  device_config->boot_config.config_feature_bit_map     = default_config->boot_config.config_feature_bit_map;
}

void app_wifi_cmd_net_intf_load_default_device_config(app_wifi_cmd_net_intf_load_default_device_config_t *cmd_input)
{
  sl_status_t status                            = SL_STATUS_OK;
  sl_wifi_device_configuration_t *device_config = get_default_device_configuration();
  // Reset the entire structure
  memset(device_config, 0, sizeof(sl_wifi_device_configuration_t));

  // Select the appropriate configuration and copy it
  if (cmd_input->net_interface == APP_WIFI_NET_INTF_INTERFACE_AP) {
    copy_wifi_device_configuration(device_config, &sl_wifi_default_ap_configuration);
  } else if (cmd_input->net_interface == APP_WIFI_NET_INTF_INTERFACE_CLIENT) {
    copy_wifi_device_configuration(device_config, &sl_wifi_default_client_configuration);
  } else {
    status = SL_STATUS_NOT_SUPPORTED;
  }

  app_wifi_rsp_net_intf_load_default_device_config(status);
  return;
}
