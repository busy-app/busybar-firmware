/***************************************************************************/ /**
 * @file
 * @brief sl_si91x_http_client_callback_framework.c
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

#include "sl_si91x_http_client_callback_framework.h"
#include "sl_constants.h"
#include <string.h>
#include "sli_wifi_utility.h"
/******************************************************
 *                    Constants
 ******************************************************/
//! HTTP OFFSET
#define SLI_SI91X_HTTP_OFFSET 12

//! HTTP PUT OFFSET
#define SLI_SI91X_HTTP_PUT_OFFSET 16

/******************************************************
 *                    Structures
 ******************************************************/
/**
 * Callback entry structure for sl_http_client events
 */
typedef struct {
  sl_http_client_t client_handle;                   ///< HTTP client handle
  sl_http_client_event_handler_t callback_function; ///< User provided callback function pointer
} sl_http_client_callback_entry_t;

/******************************************************
 *                 Global Variables
 ******************************************************/
static sl_http_client_callback_entry_t registered_callback[SL_HTTP_CLIENT_MAX_EVENT] = { 0 };

/******************************************************
 *               Function Declarations
 ******************************************************/
static sl_http_client_callback_entry_t *sli_get_http_client_callback_entry(sl_http_client_event_t event);
static sl_status_t sl_si91x_http_client_put_delete(void);

/******************************************************
 *               Function Definitions
 ******************************************************/
sl_status_t sli_http_client_register_callback(sl_http_client_event_t event,
                                              sl_http_client_t client_handle,
                                              sl_http_client_event_handler_t function)
{
  // Get the callback entry associated with the specified event
  sl_http_client_callback_entry_t *entry = sli_get_http_client_callback_entry(event);

  // check if the entry is valid
  if (entry != NULL) {
    entry->client_handle     = client_handle;
    entry->callback_function = function;
    return SL_STATUS_OK;
  }
  return SL_STATUS_FAIL;
}

static sl_status_t sl_si91x_http_client_put_delete(void)
{
  sl_status_t status = SL_STATUS_OK;

  sl_si91x_http_client_put_request_t *request =
    (sl_si91x_http_client_put_request_t *)malloc(sizeof(sl_si91x_http_client_put_request_t));
  if (request == NULL) {
    return SL_STATUS_ALLOCATION_FAILED;
  }
  memset(request, 0, sizeof(sl_si91x_http_client_put_request_t));
  request->command_type = SLI_SI91X_HTTP_CLIENT_PUT_DELETE;
  uint16_t packet_len   = sizeof(sl_si91x_http_client_put_request_t) - SLI_SI91X_HTTP_CLIENT_PUT_MAX_BUFFER_LENGTH;

  status = sli_si91x_driver_send_command(SLI_WLAN_REQ_HTTP_CLIENT_PUT,
                                         SLI_SI91X_NETWORK_CMD,
                                         request,
                                         packet_len,
                                         SLI_WLAN_RSP_HTTP_CLIENT_PUT_WAIT_TIME,
                                         NULL,
                                         NULL);
  // Free the memory allocated
  free(request);

  // Verify the status
  VERIFY_STATUS_AND_RETURN(status);
  return status;
}

static sl_http_client_callback_entry_t *sli_get_http_client_callback_entry(sl_http_client_event_t event)
{
  return &registered_callback[event];
}

sl_status_t sli_http_client_default_event_handler(sl_http_client_event_t event,
                                                  sl_wifi_buffer_t *buffer,
                                                  void *sdk_context)
{
  const sl_http_client_callback_entry_t *entry = sli_get_http_client_callback_entry(event);

  // Get the packet data from the buffer
  sl_wifi_system_packet_t *packet = (sl_wifi_system_packet_t *)sli_wifi_host_get_buffer_data(buffer, 0, NULL);

  // Convert the firmware status to a library status
  sl_status_t status = sli_convert_and_save_firmware_status(sli_get_si91x_frame_status(packet));

  // Initialize an HTTP client response structure
  sl_http_client_response_t *http_response = (sl_http_client_response_t *)malloc(sizeof(sl_http_client_response_t));
  if (http_response == NULL) {
    return SL_STATUS_ALLOCATION_FAILED;
  }
  memset(http_response, 0, sizeof(sl_http_client_response_t));

  uint16_t end_of_data;
  uint16_t http_server_response;

  http_response->status = status;

  //! TBD
  http_response->response_headers = NULL;

  if ((entry == NULL) && (entry->callback_function == NULL)) {
    free(http_response);
    return SL_STATUS_FAIL;
  }

  SL_DEBUG_LOG("\r\n>>> %s : %x <<<\r\n", __func__, status);

  // Handle different HTTP client response types based on the packet's command
  switch (packet->command) {
    case SLI_WLAN_RSP_HTTP_CLIENT_GET:
    case SLI_WLAN_RSP_HTTP_CLIENT_POST:
    case SLI_WLAN_RSP_HTTP_CLIENT_POST_DATA: {
      // Handle GET, POST, and POST_DATA responses
      if (status == SL_STATUS_OK) {
        // Extract http server response from packet
        memcpy(&http_server_response, packet->data + 2, sizeof(uint16_t));

        http_response->http_response_code = http_server_response;

        // Check http server response and fill other callback response data
        // http server response - success codes 200 to 299, failure codes 300 to 499
        // http_server_response will be zero, if FW returns error for any http client cmd
        if (http_server_response == 0 || (http_server_response >= 200 && http_server_response <= 499)) {
          // Extract end of data indication from packet
          memcpy(&end_of_data, packet->data, sizeof(uint16_t));

          http_response->end_of_data = end_of_data;
          http_response->data_buffer = &packet->data[SLI_SI91X_HTTP_OFFSET];
          http_response->data_length = packet->length - SLI_SI91X_HTTP_OFFSET;
        }
      } else if (status == SL_STATUS_SI91X_HTTP_GET_CMD_IN_PROGRESS) {
        // Don't trigger the callback, If the HTTP GET execution is in progress
        free(http_response);
        return status;
      }
      break;
    }

    case SLI_WLAN_RSP_HTTP_CLIENT_PUT: {
      // Handle PUT responses
      uint8_t http_cmd_type                                 = *packet->data;
      const sli_si91x_http_client_put_pkt_rsp_t *response   = (sli_si91x_http_client_put_pkt_rsp_t *)&packet->data;
      const sli_si91x_http_put_pkt_server_rsp_t *server_rsp = (sli_si91x_http_put_pkt_server_rsp_t *)&packet->data;

      // Delete HTTP PUT client if PUT request fails
      if (status != SL_STATUS_OK) {
        sl_si91x_http_client_put_delete();
        break;
      }

      // Check for HTTP_CLIENT_PUT_PKT command
      if (http_cmd_type == SLI_SI91X_HTTP_CLIENT_PUT_PKT) {
        http_response->data_length = 0;
        http_response->end_of_data = response->end_of_file;
      }
      // Check for HTTP Client PUT response from server
      else if (http_cmd_type == SLI_SI91X_HTTP_CLIENT_PUT_OFFSET_PKT) {
        http_response->data_length = (uint16_t)server_rsp->data_len;
        // Firmware semantics: more(code) 8 => more segments pending from server; 9 => last segment from server
        http_response->end_of_data = server_rsp->more;
      }

      http_response->data_buffer = &packet->data[SLI_SI91X_HTTP_PUT_OFFSET];

      // 917 does not support HTTP Server response for SL_HTTP_PUT request
      http_response->http_response_code = 0;

      // Delete only after the last server response segment is received
      if (http_response->end_of_data == 9) {
        sl_si91x_http_client_put_delete();
      }
      break;
    }
    default:
      break;
  }
  status = entry->callback_function(&entry->client_handle, event, http_response, sdk_context);
  free(http_response);
  return status;
}
