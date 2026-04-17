/***************************************************************************/ /**
 * @file  sl_sntp.c
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

#include "sl_slist.h"
#include "sl_si91x_driver.h"
#include "sl_status.h"
#include "sl_constants.h"
#include "sl_si91x_core_utilities.h"
#include "cmsis_os2.h"
#include "sli_wifi_utility.h"
#include "sl_sntp.h"
#include "si91x_sntp_client_types.h"
#include <string.h>

typedef struct {
  uint8_t callback_event_type;
  uint8_t *data;
  uint16_t data_length;
} sli_sntp_client_context_t;

static osMutexId_t sntp_mutex;
static sl_sntp_client_event_handler_t sntp_event_handler;

static sl_status_t sli_sntp_client_get_time_date(uint8_t *data,
                                                 uint16_t data_length,
                                                 uint32_t timeout,
                                                 uint16_t cmd_type)
{
  sl_status_t status                    = SL_STATUS_FAIL;
  sli_si91x_sntp_client_t client_req    = { 0 };
  sli_wifi_wait_period_t wait_time      = 0;
  sl_wifi_buffer_t *buffer              = NULL;
  const sl_wifi_system_packet_t *packet = NULL;
  sl_wifi_buffer_t *sdk_context         = NULL;
  sli_sntp_client_context_t *node       = NULL;
  uint16_t length                       = 0;
  uint16_t buffer_length                = 0;

  if ((timeout > 0) && ((data == NULL) || (data_length == 0))) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  client_req.command_type = (uint8_t)cmd_type;

  if (timeout > 0) {
    wait_time = SLI_WIFI_WAIT_FOR_RESPONSE(timeout);
  } else {
    status =
      sli_si91x_host_allocate_buffer(&sdk_context, SL_WIFI_CONTROL_BUFFER, sizeof(sli_sntp_client_context_t), 1000);
    VERIFY_STATUS_AND_RETURN(status);
    node = (sli_sntp_client_context_t *)sli_wifi_host_get_buffer_data(sdk_context, 0, &buffer_length);
    if (cmd_type == SLI_SI91X_SNTP_CLIENT_GETTIME) {
      node->callback_event_type = SL_SNTP_CLIENT_GET_TIME;
    } else {
      node->callback_event_type = SL_SNTP_CLIENT_GET_TIME_DATE;
    }
    node->data        = data;
    node->data_length = data_length;
    wait_time         = SLI_WIFI_RETURN_IMMEDIATELY;
  }

  status = sli_si91x_driver_send_command(SLI_WLAN_REQ_SNTP_CLIENT,
                                         SLI_SI91X_NETWORK_CMD,
                                         &client_req,
                                         sizeof(client_req),
                                         wait_time,
                                         (void *)sdk_context,
                                         &buffer);
  if ((status != SL_STATUS_OK) && (buffer != NULL)) {
    sli_si91x_host_free_buffer(buffer);
  }
  VERIFY_STATUS_AND_RETURN(status);

  packet = (sl_wifi_system_packet_t *)sli_wifi_host_get_buffer_data(buffer, 0, NULL);
  if (packet->length > data_length) {
    length = data_length;
  } else {
    length = packet->length;
  }
  memcpy(data, packet->data, length);
  status = SL_STATUS_OK;

  sli_si91x_host_free_buffer(buffer);
  return status;
}

sl_status_t sli_si91x_sntp_event_handler(sli_si91x_queue_packet_t *data)
{
  uint16_t status                     = 0;
  sl_sntp_client_response_t *response = (sl_sntp_client_response_t *)malloc(sizeof(sl_sntp_client_response_t));

  if (response == NULL) {
    return SL_STATUS_ALLOCATION_FAILED;
  }

  memset(response, 0, sizeof(sl_sntp_client_response_t));

  sl_wifi_system_packet_t *raw_rx_packet =
    (sl_wifi_system_packet_t *)sli_wifi_host_get_buffer_data(data->host_packet, 0, NULL);
  sl_wifi_buffer_t *sdk_context   = (sl_wifi_buffer_t *)data->sdk_context;
  sli_sntp_client_context_t *node = NULL;
  uint16_t buffer_length          = 0;

  node = (sli_sntp_client_context_t *)sli_wifi_host_get_buffer_data(sdk_context, 0, &buffer_length);

  osMutexAcquire(sntp_mutex, 0xFFFFFFFFUL);
  if (NULL != sntp_event_handler) {
    status                = sli_get_si91x_frame_status(raw_rx_packet);
    response->event_type  = node->callback_event_type;
    response->status      = sli_convert_and_save_firmware_status(status);
    response->data        = raw_rx_packet->data;
    response->data_length = raw_rx_packet->length;

    sntp_event_handler(response, node->data, node->data_length);
  }
  osMutexRelease(sntp_mutex);

  free(response);
  sli_si91x_host_free_buffer(sdk_context);
  return SL_STATUS_OK;
}

sl_status_t sl_sntp_client_start(sl_sntp_client_config_t *config, uint32_t timeout)
{
  sl_status_t status                 = SL_STATUS_FAIL;
  sli_si91x_sntp_client_t client_req = { 0 };
  sli_wifi_wait_period_t wait_time   = 0;
  sl_wifi_buffer_t *sdk_context      = NULL;
  sli_sntp_client_context_t *node    = NULL;
  uint16_t buffer_length             = 0;

  if (NULL == config) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  if (NULL == config->server_host_name) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  if ((SL_SNTP_BROADCAST_MODE != config->sntp_method) && (SL_SNTP_UNICAST_MODE != config->sntp_method)) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  if (NULL == sntp_mutex) {
    sntp_mutex = osMutexNew(NULL);
  }

  osMutexAcquire(sntp_mutex, 0xFFFFFFFFUL);
  sntp_event_handler = config->event_handler;
  osMutexRelease(sntp_mutex);

  client_req.command_type                = SLI_SI91X_SNTP_CLIENT_CREATE;
  client_req.sntp_method                 = config->sntp_method;
  *((uint16_t *)client_req.sntp_timeout) = config->sntp_timeout;

  // Check for IP version
  if (config->flags & 0x01) {
    client_req.ip_version = 6;
    memcpy(client_req.server_ip_address.ipv6_address, config->server_host_name, 16);
  } else {
    client_req.ip_version = 4;
    memcpy(client_req.server_ip_address.ipv4_address, config->server_host_name, 4);
  }

  if (timeout > 0) {
    wait_time = SLI_WIFI_WAIT_FOR_RESPONSE(timeout);
  } else {
    status =
      sli_si91x_host_allocate_buffer(&sdk_context, SL_WIFI_CONTROL_BUFFER, sizeof(sli_sntp_client_context_t), 1000);
    VERIFY_STATUS_AND_RETURN(status);
    node                      = sli_wifi_host_get_buffer_data(sdk_context, 0, &buffer_length);
    node->callback_event_type = SL_SNTP_CLIENT_START;
    node->data                = NULL;
    node->data_length         = 0;
    wait_time                 = SLI_WIFI_RETURN_IMMEDIATELY;
  }

  status = sli_si91x_driver_send_command(SLI_WLAN_REQ_SNTP_CLIENT,
                                         SLI_SI91X_NETWORK_CMD,
                                         &client_req,
                                         sizeof(client_req),
                                         wait_time,
                                         (void *)sdk_context,
                                         NULL);
  VERIFY_STATUS_AND_RETURN(status);
  return status;
}

sl_status_t sl_sntp_client_get_time(uint8_t *data, uint16_t data_length, uint32_t timeout)
{
  return sli_sntp_client_get_time_date(data, data_length, timeout, SLI_SI91X_SNTP_CLIENT_GETTIME);
}

sl_status_t sl_sntp_client_get_time_date(uint8_t *data, uint16_t data_length, uint32_t timeout)
{
  return sli_sntp_client_get_time_date(data, data_length, timeout, SLI_SI91X_SNTP_CLIENT_GETTIME_DATE);
}

sl_status_t sl_sntp_client_get_server_info(sl_sntp_server_info_t *data, uint32_t timeout)
{
  sl_status_t status                    = SL_STATUS_FAIL;
  sli_si91x_sntp_client_t client_req    = { 0 };
  sli_wifi_wait_period_t wait_time      = 0;
  sl_wifi_buffer_t *buffer              = NULL;
  const sl_wifi_system_packet_t *packet = NULL;
  uint16_t length                       = 0;
  sl_wifi_buffer_t *sdk_context         = NULL;
  sli_sntp_client_context_t *node       = NULL;
  uint16_t buffer_length                = 0;

  if ((timeout > 0) && (data == NULL)) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  client_req.command_type = SLI_SI91X_SNTP_CLIENT_GET_SERVER_INFO;

  if (timeout > 0) {
    wait_time = SLI_WIFI_WAIT_FOR_RESPONSE(timeout);
  } else {
    status =
      sli_si91x_host_allocate_buffer(&sdk_context, SL_WIFI_CONTROL_BUFFER, sizeof(sli_sntp_client_context_t), 1000);
    VERIFY_STATUS_AND_RETURN(status);
    node                      = sli_wifi_host_get_buffer_data(sdk_context, 0, &buffer_length);
    node->callback_event_type = SL_SNTP_CLIENT_GET_SERVER_INFO;
    node->data                = (uint8_t *)data;
    node->data_length         = sizeof(sl_sntp_server_info_t);
    wait_time                 = SLI_WIFI_RETURN_IMMEDIATELY;
  }

  status = sli_si91x_driver_send_command(SLI_WLAN_REQ_SNTP_CLIENT,
                                         SLI_SI91X_NETWORK_CMD,
                                         &client_req,
                                         sizeof(client_req),
                                         wait_time,
                                         (void *)sdk_context,
                                         &buffer);
  if ((status != SL_STATUS_OK) && (buffer != NULL)) {
    sli_si91x_host_free_buffer(buffer);
  }

  VERIFY_STATUS_AND_RETURN(status);
  packet = (sl_wifi_system_packet_t *)sli_wifi_host_get_buffer_data(buffer, 0, NULL);
  if (packet->length > sizeof(sl_sntp_server_info_t)) {
    length = sizeof(sl_sntp_server_info_t);
  } else {
    length = packet->length;
  }
  memcpy(data, packet->data, length);
  status = SL_STATUS_OK;

  sli_si91x_host_free_buffer(buffer);
  return status;
}

sl_status_t sl_sntp_client_stop(uint32_t timeout)
{
  sl_status_t status                 = SL_STATUS_FAIL;
  sli_wifi_wait_period_t wait_time   = 0;
  sli_si91x_sntp_client_t client_req = { 0 };
  sl_wifi_buffer_t *sdk_context      = NULL;
  sli_sntp_client_context_t *node    = NULL;
  uint16_t buffer_length             = 0;

  client_req.command_type = SLI_SI91X_SNTP_CLIENT_DELETE;

  if (timeout > 0) {
    wait_time = SLI_WIFI_WAIT_FOR_RESPONSE(timeout);
  } else {
    status =
      sli_si91x_host_allocate_buffer(&sdk_context, SL_WIFI_CONTROL_BUFFER, sizeof(sli_sntp_client_context_t), 1000);
    VERIFY_STATUS_AND_RETURN(status);
    node                      = sli_wifi_host_get_buffer_data(sdk_context, 0, &buffer_length);
    node->callback_event_type = SL_SNTP_CLIENT_STOP;
    node->data                = NULL;
    node->data_length         = 0;
    wait_time                 = SLI_WIFI_RETURN_IMMEDIATELY;
  }

  status = sli_si91x_driver_send_command(SLI_WLAN_REQ_SNTP_CLIENT,
                                         SLI_SI91X_NETWORK_CMD,
                                         &client_req,
                                         sizeof(client_req),
                                         wait_time,
                                         (void *)sdk_context,
                                         NULL);
  VERIFY_STATUS_AND_RETURN(status);
  return status;
}