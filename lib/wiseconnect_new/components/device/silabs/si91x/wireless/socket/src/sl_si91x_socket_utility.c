/***************************************************************************/ /**
 * @file  sl_si91x_socket_utility.c
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

#include "sl_si91x_socket_utility.h"
#include "sli_wifi_utility.h"
#include "sl_si91x_socket_types.h"
#include "sl_si91x_socket_callback_framework.h"
#include "sl_status.h"
#include "sl_constants.h"
#include "sl_si91x_driver.h"
#include "sl_si91x_protocol_types.h"
#include "sl_si91x_socket_constants.h"
#include "sl_si91x_host_interface.h"
#include "sl_si91x_core_utilities.h"
#include "sl_rsi_utility.h"
#include "sl_core.h"
#include "sli_wifi.h"
#include "sl_string.h"
#include <string.h>
#include <stdbool.h>
#include "sl_cmsis_utility.h"

/******************************************************
 *               Macro Definitions
 ******************************************************/
#define SLI_SI91X_SOCKET_ACCEPT_SUCCESS_EVENT (1 << 0)
#define SLI_SI91X_SOCKET_ACCEPT_FAILURE_EVENT (1 << 1)

#ifndef SL_SOCKET_DEFAULT_BUFFER_LIMIT
#define SL_SOCKET_DEFAULT_BUFFER_LIMIT 3
#endif

/******************************************************
 *                    Structures
 ******************************************************/

typedef struct {
  uint8_t in_use;
  uint8_t select_id;
  uint16_t frame_status;
  union {
    sl_si91x_socket_select_callback_t select_callback;
    sli_si91x_socket_select_rsp_t *response_data;
  };
} sli_si91x_select_request_t;

/******************************************************
 *               Static Function Declarations
 ******************************************************/

static void sli_si91x_clear_select_id(uint8_t flag);
static sli_si91x_select_request_t *sli_si91x_get_available_select_id(void);

/**
 * A internal function to check whether a particular port is available or not.
 * @param port_number port_number which needs to be verified for availability.
 * @return True if available else false.
 */
static bool sli_is_port_available(uint16_t port_number);

/******************************************************
 *               Variable Definitions
 ******************************************************/
sli_si91x_socket_t *sli_si91x_sockets[SLI_NUMBER_OF_SOCKETS]                                 = { 0 };
static sl_si91x_socket_remote_termination_callback_t user_remote_socket_termination_callback = NULL;
static osMutexId_t sli_si91x_socket_mutex                                                    = NULL;
static uint8_t sli_si91x_max_select_count                                                    = 0;

static sli_si91x_select_request_t *select_request_table = NULL;

sli_wifi_buffer_queue_t sli_si91x_select_response_queue;

extern sli_wifi_command_queue_t cmd_queues[SI91X_CMD_MAX];

osEventFlagsId_t si91x_socket_events        = 0;
osEventFlagsId_t si91x_socket_select_events = 0;

extern volatile uint32_t tx_socket_command_queues_status;

extern volatile uint32_t tx_socket_data_queues_status;

uint32_t sl_si91x_socket_selected_ciphers          = SL_SI91X_TLS_DEFAULT_CIPHERS;
uint32_t sl_si91x_socket_selected_extended_ciphers = SL_SI91X_TLS_EXT_CIPHERS;

/******************************************************
 *               Function Definitions
 ******************************************************/

void sli_handle_accept_response(sli_si91x_socket_t *si91x_client_socket,
                                const sli_si91x_rsp_ltcp_est_t *accept_response)
{
  //Verifying socket existence
  if (si91x_client_socket == NULL)
    return;
  // Update socket parameters based on the accept response
  si91x_client_socket->id                         = accept_response->socket_id;
  si91x_client_socket->local_address.sin6_port    = accept_response->src_port_num;
  si91x_client_socket->remote_address.sin6_port   = accept_response->dest_port;
  si91x_client_socket->mss                        = accept_response->mss;
  si91x_client_socket->state                      = CONNECTED;
  si91x_client_socket->remote_address.sin6_family = accept_response->ip_version == SL_IPV6_ADDRESS_LENGTH ? AF_INET6
                                                                                                          : AF_INET;

  if (si91x_client_socket->remote_address.sin6_family == AF_INET6) {
#ifdef SLI_SI91X_NETWORK_DUAL_STACK
    memcpy(si91x_client_socket->remote_address.sin6_addr.un.u8_addr,
           accept_response->dest_ip_addr.ipv6_address,
           SL_IPV6_ADDRESS_LENGTH);
#else
#ifndef __ZEPHYR__
    memcpy(si91x_client_socket->remote_address.sin6_addr.__u6_addr.__u6_addr8,
           accept_response->dest_ip_addr.ipv6_address,
           SL_IPV6_ADDRESS_LENGTH);
#else
    memcpy(si91x_client_socket->remote_address.sin6_addr.s6_addr,
           accept_response->dest_ip_addr.ipv6_address,
           SL_IPV6_ADDRESS_LENGTH);
#endif
#endif

  } else {
    memcpy(&((struct sockaddr_in *)&si91x_client_socket->remote_address)->sin_addr,
           accept_response->dest_ip_addr.ipv4_address,
           SL_IPV4_ADDRESS_LENGTH);
  }
}

#ifndef __ZEPHYR__
int sli_handle_select_response(const sli_si91x_socket_select_rsp_t *response,
                               fd_set *readfds,
                               fd_set *writefds,
                               fd_set *exception_fd)
#else
int sli_handle_select_response(const sli_si91x_socket_select_rsp_t *response,
                               sl_si91x_fdset_t *readfds,
                               sl_si91x_fdset_t *writefds,
                               sl_si91x_fdset_t *exception_fd)
#endif
{
  // To track of the total number of file descriptors set
  int total_fd_set_count = 0;

  // Clear file descriptor sets
  SLI_SI91X_NULL_SAFE_FD_ZERO(readfds);
  SLI_SI91X_NULL_SAFE_FD_ZERO(writefds);
  SLI_SI91X_NULL_SAFE_FD_ZERO(exception_fd);

  // Iterate through all host sockets
  for (int host_socket_index = 0; host_socket_index < SLI_NUMBER_OF_SOCKETS; host_socket_index++) {
    const sli_si91x_socket_t *socket = sli_get_si91x_socket(host_socket_index);
    //Verifying socket existence
    if (socket == NULL) {
      continue;
    }

    // Check if the read file descriptor set is provided and if the corresponding bit is set in the response
    if (readfds != NULL && (response->read_fds.fd_array[0] & (1 << socket->id))) {
#ifndef __ZEPHYR__
      FD_SET(host_socket_index, readfds);
#else
      SL_SI91X_FD_SET(host_socket_index, readfds);
#endif
      total_fd_set_count++;
    }

    // Check if the write file descriptor set is provided and if the corresponding bit is set in the response.
    if (writefds != NULL && (response->write_fds.fd_array[0] & (1 << socket->id))) {
#ifndef __ZEPHYR__
      FD_SET(host_socket_index, writefds);
#else
      SL_SI91X_FD_SET(host_socket_index, writefds);
#endif
      total_fd_set_count++;
    }
  }

  return total_fd_set_count;
}

void sli_si91x_set_accept_callback(sli_si91x_socket_t *server_socket,
                                   sl_si91x_socket_accept_callback_t callback,
                                   int32_t client_socket_id)
{
  // Set the user-defined accept callback function and the client socket ID
  server_socket->user_accept_callback = callback;
  server_socket->client_id            = client_socket_id;
}

void sli_si91x_set_remote_socket_termination_callback(sl_si91x_socket_remote_termination_callback_t callback)
{
  user_remote_socket_termination_callback = callback;
}

sl_status_t sli_si91x_socket_init(uint8_t max_select_count)
{
  // Check if the mutex for socket operations is already initialized.
  // If not, create a new mutex to ensure thread-safe access.
  if (sli_si91x_socket_mutex == NULL) {
    sli_si91x_socket_mutex = osMutexNew(NULL); // Create a new mutex.
    if (sli_si91x_socket_mutex == NULL) {
      return SL_STATUS_FAIL; // Return failure if mutex creation fails.
    }
  }

  // Check if the event flags object for socket events is already initialized.
  // If not, create a new event flag set to manage socket events.
  if (si91x_socket_events == NULL) {
    si91x_socket_events = osEventFlagsNew(NULL); // Create new event flags.
    if (si91x_socket_events == NULL) {
      return SL_STATUS_FAIL; // Return failure if event flag creation fails.
    }
  }

  // Check if the event flags object for socket select events is already initialized.
  // If not, create a new event flag set to manage socket select events.
  if (si91x_socket_select_events == NULL) {
    si91x_socket_select_events = osEventFlagsNew(NULL); // Create new event flags.
    if (si91x_socket_select_events == NULL) {
      return SL_STATUS_FAIL; // Return failure if event flag creation fails.
    }
  }

  /* 
  Allocate memory for the select request table based on the number of select instances.
  Heap memory is allocated for the number of instances of this structure based on 
  the number of selects configured by the user during device initialization in opermode.
  Each time a sync or async select command is sent to the firmware, the corresponding 
  structure is updated, and the instance is cleared when the response is received.
  */

  // Check if the select_request_table is uninitialized and max_select_count is valid.
  if (select_request_table == NULL && max_select_count != 0 && max_select_count <= 10) {
    sli_si91x_max_select_count = max_select_count; // Store the max number of selects.

    // Allocate memory for the select request table based on the max_select_count.
    select_request_table = calloc(max_select_count, sizeof(sli_si91x_select_request_t));

    // If memory allocation fails, return failure.
    if (select_request_table == NULL) {
      return SL_STATUS_FAIL; // Return failure if memory allocation fails.
    }
  }

  return SL_STATUS_OK; // Return success if initialization is successful.
}

sl_status_t sli_si91x_socket_deinit(void)
{
  // free the sli_si91x_socket_mutex
  if (sli_si91x_socket_mutex != NULL) {
    osMutexDelete(sli_si91x_socket_mutex);
    sli_si91x_socket_mutex = NULL;
  }
  if (si91x_socket_events != NULL) {
    osEventFlagsDelete(si91x_socket_events);
    si91x_socket_events = NULL;
  }
  if (si91x_socket_select_events != NULL) {
    osEventFlagsDelete(si91x_socket_select_events);
    si91x_socket_select_events = NULL;
  }
  if (select_request_table != NULL) {
    free(select_request_table);
    select_request_table = NULL;
  }
  return SL_STATUS_OK;
}

sl_status_t sli_si91x_vap_shutdown(uint8_t vap_id, sli_si91x_bsd_disconnect_reason_t disconnect_reason)
{
  // Iterate through all BSD sockets and modify the state those associated with the given VAP ID
  for (uint8_t socket_index = 0; socket_index < SLI_NUMBER_OF_SOCKETS; socket_index++) {
    if ((sli_si91x_sockets[socket_index] != NULL) && (sli_si91x_sockets[socket_index]->vap_id == vap_id)) {
      sli_si91x_sockets[socket_index]->state             = DISCONNECTED;
      sli_si91x_sockets[socket_index]->disconnect_reason = disconnect_reason;
    }
  }

  return SL_STATUS_OK;
}

void sli_si91x_handle_websocket(sli_si91x_socket_create_request_t *socket_create_request,
                                const sli_si91x_socket_t *si91x_bsd_socket)
{
  socket_create_request->ssl_bitmap |= SLI_SI91X_WEBSOCKET_FEAT;

  // Copy host name
  if (si91x_bsd_socket->websocket_info && si91x_bsd_socket->websocket_info->host_length > 0) {
    memcpy(socket_create_request->webs_host_name,
           si91x_bsd_socket->websocket_info->websocket_data,
           si91x_bsd_socket->websocket_info->host_length);
    socket_create_request->webs_host_name[si91x_bsd_socket->websocket_info->host_length] = '\0'; // Null-terminate
  }

  // Copy resource name
  if (si91x_bsd_socket->websocket_info && si91x_bsd_socket->websocket_info->resource_length > 0) {
    memcpy(socket_create_request->webs_resource_name,
           si91x_bsd_socket->websocket_info->websocket_data + si91x_bsd_socket->websocket_info->host_length,
           si91x_bsd_socket->websocket_info->resource_length);
    socket_create_request->webs_resource_name[si91x_bsd_socket->websocket_info->resource_length] =
      '\0'; // Null-terminate
  }

  // Copy subprotocol name
  if (si91x_bsd_socket->websocket_info && si91x_bsd_socket->websocket_info->subprotocol_length > 0) {
    memcpy(socket_create_request->webs_subprotocol_name,
           si91x_bsd_socket->websocket_info->websocket_data + si91x_bsd_socket->websocket_info->host_length
             + si91x_bsd_socket->websocket_info->resource_length,
           si91x_bsd_socket->websocket_info->subprotocol_length);
    socket_create_request->webs_subprotocol_name[si91x_bsd_socket->websocket_info->subprotocol_length] =
      '\0'; // Null-terminate
  }
}

sl_status_t sl_si91x_config_socket(sl_si91x_socket_config_t socket_config)
{
  sl_status_t status = SL_STATUS_OK;

  // Send the socket configuration command to the SI91X driver
  status = sli_si91x_driver_send_command(SLI_WLAN_REQ_SOCKET_CONFIG,
                                         SLI_SI91X_SOCKET_CMD,
                                         &socket_config,
                                         sizeof(socket_config),
                                         SLI_WLAN_RSP_SOCKET_CONFIG_WAIT_TIME,
                                         NULL,
                                         NULL);

  VERIFY_STATUS_AND_RETURN(status);
  return status;
}

/**
 * @brief Frees resources associated with a given socket.
 *
 * This function releases the memory allocated for a socket and 
 * deletes any associated OS event flags. It ensures proper cleanup 
 * by setting the socket pointer to NULL after freeing it.
 *
 * @param socket The socket index to be freed.
 */
void sli_si91x_free_socket(int socket)
{
  // Retrieve the socket object associated with the given socket index.
  sli_si91x_socket_t *si91x_socket = sli_get_si91x_socket(socket);

  // If the socket is invalid (NULL), no cleanup is needed; return early.
  if (si91x_socket == NULL) {
    return;
  }

  // Check if the socket has associated OS event flags.
  if (si91x_socket->socket_events != NULL) {
    // Delete the OS event flags associated with the socket to free resources.
    osEventFlagsDelete(si91x_socket->socket_events);

    // Set the event flags pointer to NULL to avoid dangling pointers.
    si91x_socket->socket_events = NULL;
  }

  // Free the domain name if it was allocated
  if (si91x_socket->domain_name != NULL) {
    free(si91x_socket->domain_name);
    si91x_socket->domain_name = NULL;
  }

  // Free the memory allocated for the socket structure.
  free(si91x_socket);

  // Set the global socket pointer to NULL to prevent future use of freed memory.
  sli_si91x_sockets[socket] = NULL;
}

// Get the SI91X socket with the specified index, if it is valid and not in RESET state
sli_si91x_socket_t *sli_get_si91x_socket(int32_t socket)
{
  if (socket < 0 || socket >= SLI_NUMBER_OF_SOCKETS) {
    return NULL;
  }
  return sli_si91x_sockets[socket];
}

sli_si91x_socket_t *sli_si91x_get_socket_from_id(int socket_id,
                                                 sli_si91x_bsd_socket_state_t excluded_state,
                                                 int16_t role)
{
  sli_si91x_socket_t *possible_socket = NULL;
  for (uint8_t index = 0; index < SLI_NUMBER_OF_SOCKETS; ++index) {
    sli_si91x_socket_t *socket = sli_si91x_sockets[index];
    if (socket != NULL && socket->id == socket_id && socket->state != excluded_state
        && (role == -1 || socket->role == role)) {
      if (socket->command_queue.command_in_flight == false) {
        possible_socket = socket;
      } else {
        return socket;
      }
    }
  }
  return possible_socket;
}

static sli_si91x_socket_t *sli_si91x_get_socket_from_port(uint16_t src_port)
{
  for (int i = 0; i < SLI_NUMBER_OF_SOCKETS; i++) {
    if (sli_si91x_sockets[i] == NULL) {
      continue;
    }
    if ((sli_si91x_sockets[i]->role == SLI_SI91X_SOCKET_TCP_SERVER)
        && (src_port == sli_si91x_sockets[i]->local_address.sin6_port)) {
      return sli_si91x_sockets[i];
    }
  }

  return NULL;
}

// Find and return an available socket and its index
void sli_get_free_socket(sli_si91x_socket_t **socket, int *socket_fd)
{
  *socket    = NULL;
  *socket_fd = -1;

  osMutexAcquire(sli_si91x_socket_mutex, 0xFFFFFFFFUL);
  // Iterate through all available sockets to find a free one
  for (uint8_t socket_index = 0; socket_index < SLI_NUMBER_OF_SOCKETS; socket_index++) {

    // If the socket is in use skip it
    if (sli_si91x_sockets[socket_index] != NULL) {
      continue;
    }

    // Allocate new socket
    // NOTE: The 'new_socket' variable is intentionally used to prevent race conditions between allocation and initialization.
    // Removing or optimizing this intermediate variable may lead to unexpected issues.
    sli_si91x_socket_t *new_socket = malloc(sizeof(sli_si91x_socket_t));
    if (new_socket != NULL) {
      memset(new_socket, 0, sizeof(sli_si91x_socket_t));
      sli_si91x_sockets[socket_index]                    = new_socket;
      sli_si91x_sockets[socket_index]->id                = -1;
      sli_si91x_sockets[socket_index]->index             = socket_index;
      sli_si91x_sockets[socket_index]->data_buffer_limit = SL_SOCKET_DEFAULT_BUFFER_LIMIT;

      // If a free socket is found, set the socket pointer to point to it
      *socket = sli_si91x_sockets[socket_index];
      // Set the socket_fd to the index of the free socket, which can be used as a file descriptor
      *socket_fd = socket_index;
    }
    // Exit the loop because a free socket has been found.
    break;
  }
  osMutexRelease(sli_si91x_socket_mutex);
}

static bool sli_is_port_available(uint16_t port_number)
{
  // Check whether local port is already used or not
  for (uint8_t socket_index = 0; socket_index < SLI_NUMBER_OF_SOCKETS; socket_index++) {
    if (sli_si91x_sockets[socket_index] != NULL
        && sli_si91x_sockets[socket_index]->local_address.sin6_port == port_number) {
      return false;
    }
  }

  return true;
}

/**
 * @brief This function is responsible to copy the TLS extension information provided by application into socket structure.
 * 
 * @param socket_tls_extensions pointer to TLS extension in socket structure
 * @param tls_extension pointer to the TLS information provided by application
 * @return sl_status_t possible return values are SL_STATUS_OK and SL_STATUS_SI91X_MEMORY_ERROR
 */
sl_status_t sli_si91x_add_tls_extension(sli_si91x_tls_extensions_t *socket_tls_extensions,
                                        const sl_si91x_socket_type_length_value_t *tls_extension)
{
  // To check if memory available for new extension in buffer of socket, max 256 Bytes only
  if (SLI_SI91X_MAX_SIZE_OF_EXTENSION_DATA - socket_tls_extensions->current_size_of_extensions
      < (int)(sizeof(sl_si91x_socket_type_length_value_t) + tls_extension->length)) {
    return SL_STATUS_SI91X_MEMORY_ERROR;
  }

  uint8_t extension_size = (uint8_t)(sizeof(sl_si91x_socket_type_length_value_t) + tls_extension->length);

  // copies TLS extension provided by app into SDK socket struct
  memcpy(&socket_tls_extensions->buffer[socket_tls_extensions->current_size_of_extensions],
         tls_extension,
         extension_size);
  socket_tls_extensions->current_size_of_extensions += extension_size;
  socket_tls_extensions->total_extensions++;

  return SL_STATUS_OK;
}

int32_t sli_get_socket_command_from_host_packet(sl_wifi_buffer_t *buffer)
{
  const sl_wifi_system_packet_t *packet = (sl_wifi_system_packet_t *)buffer->data;
  return (packet == NULL ? -1 : packet->command);
}

void sli_si91x_create_socket_request(sli_si91x_socket_t *si91x_bsd_socket,
                                     sli_si91x_socket_create_request_t *socket_create_request,
                                     int type,
                                     const int *backlog,
                                     sli_wifi_wait_period_t *wait_period)
{
  if (si91x_bsd_socket->local_address.sin6_family == AF_INET6) {
    socket_create_request->ip_version = SL_IPV6_VERSION;
#ifdef SLI_SI91X_NETWORK_DUAL_STACK
    memcpy(socket_create_request->dest_ip_addr.ipv6_address,
           si91x_bsd_socket->remote_address.sin6_addr.un.u8_addr,
           SL_IPV6_ADDRESS_LENGTH);
#else
#ifndef __ZEPHYR__
    memcpy(socket_create_request->dest_ip_addr.ipv6_address,
           si91x_bsd_socket->remote_address.sin6_addr.__u6_addr.__u6_addr8,
           SL_IPV6_ADDRESS_LENGTH);
#else
    memcpy(socket_create_request->dest_ip_addr.ipv6_address,
           si91x_bsd_socket->remote_address.sin6_addr.s6_addr,
           SL_IPV6_ADDRESS_LENGTH);
#endif
#endif
  } else {
    socket_create_request->ip_version = SL_IPV4_ADDRESS_LENGTH;
    memcpy(socket_create_request->dest_ip_addr.ipv4_address,
           &((struct sockaddr_in *)&si91x_bsd_socket->remote_address)->sin_addr,
           SL_IPV4_ADDRESS_LENGTH);
  }

  socket_create_request->local_port  = si91x_bsd_socket->local_address.sin6_port;
  socket_create_request->remote_port = si91x_bsd_socket->remote_address.sin6_port;

  // Fill socket type
  socket_create_request->socket_type = (uint16_t)type;

  if (type == SLI_SI91X_SOCKET_TCP_SERVER) {
    socket_create_request->max_count = (backlog == NULL) ? 0 : (uint16_t)*backlog;
    socket_create_request->socket_bitmap |= SLI_SI91X_SOCKET_FEAT_LTCP_ACCEPT;
    si91x_bsd_socket->socket_events = osEventFlagsNew(NULL);
  } else {
    socket_create_request->max_count = 0;
  }

  if (si91x_bsd_socket->recv_data_callback == NULL) {
    socket_create_request->socket_bitmap |= SLI_SI91X_SOCKET_FEAT_SYNCHRONOUS;
  }

  socket_create_request->socket_bitmap |= SLI_SI91X_SOCKET_FEAT_TCP_RX_WINDOW;

  // Set the RX window size
  socket_create_request->rx_window_size = SLI_TCP_RX_WINDOW_SIZE;

  // Fill VAP_ID
  socket_create_request->vap_id                = si91x_bsd_socket->vap_id;
  socket_create_request->tos                   = 0;
  socket_create_request->max_tcp_retries_count = si91x_bsd_socket->max_tcp_retries ? si91x_bsd_socket->max_tcp_retries
                                                                                   : SLI_MAX_TCP_RETRY_COUNT;
  socket_create_request->tcp_keepalive_initial_time = si91x_bsd_socket->tcp_keepalive_initial_time
                                                        ? si91x_bsd_socket->tcp_keepalive_initial_time
                                                        : SLI_DEFAULT_TCP_KEEP_ALIVE_TIME;
  socket_create_request->tcp_mss                    = si91x_bsd_socket->mss;

  // Check for SSL feature and fill it in SSL bitmap
  if ((si91x_bsd_socket->ssl_bitmap & SL_SI91X_ENABLE_TLS) || (si91x_bsd_socket->ssl_bitmap & SL_SI91X_ENABLE_DTLS)) {
    socket_create_request->ssl_bitmap         = si91x_bsd_socket->ssl_bitmap;
    socket_create_request->ssl_ciphers_bitmap = sl_si91x_socket_selected_ciphers;
#if defined(SLI_SI917)
    socket_create_request->ssl_ext_ciphers_bitmap = sl_si91x_socket_selected_extended_ciphers;
#endif
    // Check if cert index is not default index
    if (si91x_bsd_socket->certificate_index > SLI_SI91X_CERT_INDEX_0) {
      socket_create_request->socket_bitmap |= SLI_SI91X_SOCKET_FEAT_CERT_INDEX;
    }

    socket_create_request->socket_cert_inx = si91x_bsd_socket->certificate_index;

    // Check if extension is provided my application and memcopy until the provided size of extensions
    if (si91x_bsd_socket->tls_extensions.total_extensions > 0) {
      memcpy(socket_create_request->tls_extension_data,
             si91x_bsd_socket->tls_extensions.buffer,
             si91x_bsd_socket->tls_extensions.current_size_of_extensions);

      socket_create_request->total_extension_length = si91x_bsd_socket->tls_extensions.current_size_of_extensions;
      socket_create_request->no_of_tls_extensions   = si91x_bsd_socket->tls_extensions.total_extensions;
    }
    *wait_period = SLI_WIFI_WAIT_FOR_RESPONSE(SLI_WLAN_RSP_SOCKET_CREATE_WAIT_TIME);
  }

  // Check for HIGH_PERFORMANCE feature bit
  if (si91x_bsd_socket->ssl_bitmap & SLI_SI91X_HIGH_PERFORMANCE_SOCKET) {
    socket_create_request->ssl_bitmap |= SLI_SI91X_HIGH_PERFORMANCE_SOCKET;
  }

  // Check for Websocket feature bit
  if (si91x_bsd_socket->ssl_bitmap & SLI_SI91X_WEBSOCKET_FEAT) {
    sli_si91x_handle_websocket(socket_create_request, si91x_bsd_socket);
  }

  // Check for TCP ACK INDICATION feature bit
  if (si91x_bsd_socket->socket_bitmap & SLI_SI91X_SOCKET_FEAT_TCP_ACK_INDICATION) {
    socket_create_request->socket_bitmap |= SLI_SI91X_SOCKET_FEAT_TCP_ACK_INDICATION;
  }
#if defined(SLI_SI917)
  // Set socket's max retransmission timeout value and TOS (Type of Service) if applicable
  socket_create_request->max_retransmission_timeout_value = (uint8_t)si91x_bsd_socket->max_retransmission_timeout_value;
  socket_create_request->tos                              = (uint16_t)si91x_bsd_socket->tos;
#endif

  // Add domain_name if present
  if (si91x_bsd_socket->domain_name != NULL) {
    size_t domain_name_len =
      sl_strnlen((char *)si91x_bsd_socket->domain_name, sizeof(socket_create_request->domain_name) - 1);
    memcpy(socket_create_request->domain_name, si91x_bsd_socket->domain_name, domain_name_len);
    socket_create_request->domain_name[domain_name_len] = '\0';
  } else {
    socket_create_request->domain_name[0] = '\0';
  }
}

// Prepare socket request based on socket type and send the request down to the driver.
// socket type : [SL_SOCKET_TCP_SERVER, SL_SOCKET_TCP_CLIENT, SL_SOCKET_LUDP, SL_SOCKET_UDP_CLIENT]
sl_status_t sli_create_and_send_socket_request(int socketIdIndex, int type, const int *backlog)
{
  sl_status_t status                                               = SL_STATUS_OK;
  sli_si91x_socket_create_request_t socket_create_request          = { 0 };
  const sli_si91x_socket_create_response_t *socket_create_response = NULL;
  sli_si91x_socket_t *si91x_bsd_socket                             = sli_get_si91x_socket(socketIdIndex);
  sli_wifi_wait_period_t wait_period = SLI_WIFI_WAIT_FOR_RESPONSE(SLI_WLAN_RSP_SOCKET_CREATE_WAIT_TIME);
  //Verifying socket existence
  if (si91x_bsd_socket == NULL) {
    return -1;
  }
  if (type == SLI_SI91X_SOCKET_TCP_CLIENT) {
    wait_period = SLI_WIFI_WAIT_FOR_RESPONSE(100000); // timeout is 10 sec
  }

  sl_wifi_buffer_t *buffer        = NULL;
  sl_wifi_system_packet_t *packet = NULL;

  sli_si91x_create_socket_request(si91x_bsd_socket, &socket_create_request, type, backlog, &wait_period);

  // Store socket role for future references.
  si91x_bsd_socket->role = type;
  status                 = sli_si91x_driver_send_command(SLI_WLAN_REQ_SOCKET_CREATE,
                                         SLI_SI91X_SOCKET_CMD,
                                         &socket_create_request,
                                         sizeof(socket_create_request),
                                         wait_period,
                                         NULL,
                                         &buffer);

  // If the status is not OK and there's a buffer, free the buffer
  if ((status != SL_STATUS_OK) && (buffer != NULL)) {
    sli_si91x_host_free_buffer(buffer);
  }
  VERIFY_STATUS_AND_RETURN(status);

  // Extract socket creation response information
  packet                 = sli_wifi_host_get_buffer_data(buffer, 0, NULL);
  socket_create_response = (sli_si91x_socket_create_response_t *)packet->data;

  si91x_bsd_socket->id = (int32_t)(socket_create_response->socket_id[0] | (socket_create_response->socket_id[1] << 8));
  si91x_bsd_socket->local_address.sin6_port =
    (uint16_t)(socket_create_response->module_port[0] | (socket_create_response->module_port[1] << 8));

  if (type != SLI_SI91X_SOCKET_TCP_SERVER) {
    si91x_bsd_socket->remote_address.sin6_port =
      (uint16_t)(socket_create_response->dst_port[0] | socket_create_response->dst_port[1] << 8);
  }

  si91x_bsd_socket->mss = (uint16_t)((socket_create_response->mss[0]) | (socket_create_response->mss[1] << 8));

  // If socket is already bound to an local address and port, there is no need to copy it again.
  if (si91x_bsd_socket->state == BOUND) {
    sli_si91x_host_free_buffer(buffer);
    return SL_STATUS_OK;
  }

  // Copy the local address (IPv4 or IPv6) based on family type
  if (si91x_bsd_socket->local_address.sin6_family == AF_INET) {
    memcpy(&((struct sockaddr_in *)&si91x_bsd_socket->local_address)->sin_addr.s_addr,
           socket_create_response->module_ip_addr.ipv4_addr,
           SL_IPV4_ADDRESS_LENGTH);
  } else {
#ifdef SLI_SI91X_NETWORK_DUAL_STACK
    memcpy(si91x_bsd_socket->local_address.sin6_addr.un.u8_addr,
           socket_create_response->module_ip_addr.ipv6_addr,
           SL_IPV6_ADDRESS_LENGTH);
#else
#ifndef __ZEPHYR__
    memcpy(si91x_bsd_socket->local_address.sin6_addr.__u6_addr.__u6_addr8,
           socket_create_response->module_ip_addr.ipv6_addr,
           SL_IPV6_ADDRESS_LENGTH);
#else
    memcpy(si91x_bsd_socket->local_address.sin6_addr.s6_addr,
           socket_create_response->module_ip_addr.ipv6_addr,
           SL_IPV6_ADDRESS_LENGTH);
#endif
#endif
  }

  // Free the buffer
  sli_si91x_host_free_buffer(buffer);

  return SL_STATUS_OK;
}

int sli_si91x_socket(int family, int type, int protocol, sl_si91x_socket_receive_data_callback_t callback)
{
  // Validate the socket parameters
  SLI_SET_ERRNO_AND_RETURN_IF_TRUE(family != AF_INET && family != AF_INET6, EAFNOSUPPORT);
  SLI_SET_ERRNO_AND_RETURN_IF_TRUE(type != SOCK_STREAM && type != SOCK_DGRAM, EINVAL);
  SLI_SET_ERRNO_AND_RETURN_IF_TRUE(protocol != IPPROTO_TCP && protocol != IPPROTO_UDP && protocol != 0, EINVAL);
  SLI_SET_ERRNO_AND_RETURN_IF_TRUE((type == SOCK_STREAM && (protocol != IPPROTO_TCP && protocol != 0)), EPROTOTYPE);
  SLI_SET_ERRNO_AND_RETURN_IF_TRUE((type == SOCK_DGRAM && (protocol != IPPROTO_UDP && protocol != 0)), EPROTOTYPE);

  // Initialize a new socket structure
  sli_si91x_socket_t *si91x_socket;
  int socket_index = -1;

  sli_get_free_socket(&si91x_socket, &socket_index);

  // Check if there is enough memory to create the socket
  if (socket_index < 0) {
    SLI_SET_ERROR_AND_RETURN(ENOMEM);
  }

  // Populate the socket structure with provided parameters and callbacks
  si91x_socket->type                      = type;
  si91x_socket->local_address.sin6_family = (uint8_t)family;
  si91x_socket->protocol                  = protocol;
  si91x_socket->state                     = INITIALIZED;
  si91x_socket->recv_data_callback        = callback;
  si91x_socket->client_id                 = -1;

  // Return the socket index
  return socket_index;
}

int sli_si91x_accept(int socket, struct sockaddr *addr, socklen_t *addr_len, sl_si91x_socket_accept_callback_t callback)
{
  sl_status_t status                               = SL_STATUS_OK;
  sli_si91x_socket_t *si91x_client_socket          = NULL;
  sli_si91x_socket_t *si91x_server_socket          = sli_get_si91x_socket(socket);
  sli_si91x_socket_accept_request_t accept_request = { 0 };
  int32_t client_socket_id                         = -1;
  sl_wifi_buffer_t *buffer                         = NULL;

  // Check if the server socket is valid
  SLI_SET_ERRNO_AND_RETURN_IF_TRUE(si91x_server_socket == NULL, EBADF);
  SLI_SET_ERRNO_AND_RETURN_IF_TRUE(si91x_server_socket->type != SOCK_STREAM, EOPNOTSUPP);
  SLI_SET_ERRNO_AND_RETURN_IF_TRUE(si91x_server_socket->state != LISTEN, EINVAL);

  // Create a new instance for socket
  client_socket_id = sli_si91x_socket(si91x_server_socket->local_address.sin6_family,
                                      si91x_server_socket->type,
                                      si91x_server_socket->protocol,
                                      si91x_server_socket->recv_data_callback);

  si91x_client_socket = sli_get_si91x_socket(client_socket_id);
  //Verifying socket existence
  if (si91x_client_socket == NULL)
    return -1;
  memcpy(&si91x_client_socket->local_address, &si91x_server_socket->local_address, sizeof(struct sockaddr_in6));

  // Fill VAP_ID
  si91x_client_socket->vap_id = si91x_server_socket->vap_id;

  // Create accept request
  accept_request.socket_id   = (uint8_t)si91x_server_socket->id;
  accept_request.source_port = si91x_server_socket->local_address.sin6_port;

  // Set the callback and client socket ID.
  sli_si91x_set_accept_callback(si91x_server_socket, callback, client_socket_id);
  if (callback != NULL) {
    status = sli_si91x_send_socket_command(si91x_client_socket,
                                           SLI_WLAN_REQ_SOCKET_ACCEPT,
                                           &accept_request,
                                           sizeof(accept_request),
                                           SLI_WIFI_RETURN_IMMEDIATELY,
                                           NULL);
    SLI_SOCKET_VERIFY_STATUS_AND_RETURN(status, SL_STATUS_OK, SLI_SI91X_UNDEFINED_ERROR);
    return SL_STATUS_OK;
  } else {
    status = sli_si91x_send_socket_command(si91x_client_socket,
                                           SLI_WLAN_REQ_SOCKET_ACCEPT,
                                           &accept_request,
                                           sizeof(accept_request),
                                           SLI_WIFI_WAIT_FOR_EVER | SLI_WIFI_WAIT_FOR_RESPONSE_BIT,
                                           &buffer);
    SLI_SOCKET_VERIFY_STATUS_AND_RETURN(status, SL_STATUS_OK, SLI_SI91X_UNDEFINED_ERROR);
  }

  // If the accept request fails, clean up allocated memory and return an error
  if (status != SL_STATUS_OK) {
    sli_si91x_shutdown(client_socket_id, SHUTDOWN_BY_ID);
    if (buffer != NULL) {
      sli_si91x_host_free_buffer(buffer);
    }
    SLI_SET_ERROR_AND_RETURN(SLI_SI91X_UNDEFINED_ERROR);
  }

  sli_si91x_queue_packet_t *node = sli_wifi_host_get_buffer_data(buffer, 0, NULL);
  sl_wifi_buffer_t *response     = node->host_packet;
  sli_si91x_host_free_buffer(buffer);

  if (response == NULL) {
    SLI_SET_ERROR_AND_RETURN(SLI_SI91X_UNDEFINED_ERROR);
  }

  sl_wifi_system_packet_t *packet      = sli_wifi_host_get_buffer_data(response, 0, NULL);
  const sli_si91x_rsp_ltcp_est_t *ltcp = (sli_si91x_rsp_ltcp_est_t *)packet->data;

  sli_handle_accept_response(si91x_client_socket, ltcp);

  // If addr_len is NULL or invalid value, just return the client socket ID
  if (addr != NULL && *addr_len > 0) {
    // Copy the remote address to the provided sockaddr structure
    memcpy(addr,
           &si91x_client_socket->remote_address,
           (*addr_len > sizeof(struct sockaddr_in6)) ? sizeof(struct sockaddr_in6) : *addr_len);

    // Update addr_len based on the family of the local address
    *addr_len = si91x_client_socket->local_address.sin6_family == AF_INET ? sizeof(struct sockaddr_in)
                                                                          : sizeof(struct sockaddr_in6);
  }
  // Free resources and return the client socket ID
  sli_si91x_host_free_buffer(response);

  return client_socket_id;
}

// Shutdown a socket
int sli_si91x_shutdown(int socket, int how)
{
  sl_status_t status                                            = SL_STATUS_OK;
  sli_si91x_socket_close_request_t socket_close_request         = { 0 };
  const sl_si91x_socket_close_response_t *socket_close_response = NULL;
  sli_wifi_wait_period_t wait_period                            = SLI_WIFI_WAIT_FOR_RESPONSE(SLI_WIFI_WAIT_FOR_EVER);
  sl_wifi_buffer_t *buffer                                      = NULL;

  sli_si91x_socket_t *si91x_socket = sli_get_si91x_socket(socket);

  // Verify the socket's existence
  SLI_SET_ERRNO_AND_RETURN_IF_TRUE(si91x_socket == NULL, EBADF);

  // The firmware maps server socket and first client socket connected to the server would be assigned same firmware socket ID.
  // Therefore, if Dev attempts to close either first client or server, close request type needs to be set to SHUTDOWN_BY_PORT.
  int close_request_type = (si91x_socket->state == LISTEN) ? SHUTDOWN_BY_PORT : how;

  // Check the state of the socket and perform cleanup if necessary
  if (si91x_socket->state == BOUND            // Socket is in a bound state
      || si91x_socket->state == INITIALIZED   // Socket is in an initialized state
      || (si91x_socket->state == DISCONNECTED // Socket is disconnected
          && si91x_socket->disconnect_reason
               == SLI_SI91X_BSD_DISCONNECT_REASON_INTERFACE_DOWN) // Disconnection due to interface down
      || (si91x_socket->state == DISCONNECTED                     // Socket is disconnected
          && si91x_socket->disconnect_reason
               == SLI_SI91X_BSD_DISCONNECT_REASON_REMOTE_CLOSED // Disconnection due to remote side terminate
          && sli_is_tcp_auto_close_enabled())) {                // Check if TCP auto-close is enabled
    // Free the resources associated with this socket
    sli_si91x_free_socket(socket);

    // Return success as the cleanup is successfully performed
    return SLI_SI91X_NO_ERROR;
  }

  // Continuously checks if the transmit data queue of the specified socket is empty.
  // If the queue is not empty, it waits for 2 milliseconds before checking again.
  while (!sli_si91x_buffer_queue_empty(&si91x_socket->tx_data_queue)) {
    osDelay(SLI_SYSTEM_MS_TO_TICKS(2));
  }

  /*If socket is server socket, SHUTDOWN_BY_PORT is to be used irrespective of 'how' parameter.*/
  socket_close_request.socket_id   = (uint16_t)((close_request_type == SHUTDOWN_BY_ID) ? si91x_socket->id : 0);
  socket_close_request.port_number = (close_request_type == SHUTDOWN_BY_ID) ? 0 : si91x_socket->local_address.sin6_port;

  status = sli_si91x_send_socket_command(si91x_socket,
                                         SLI_WLAN_REQ_SOCKET_CLOSE,
                                         &socket_close_request,
                                         sizeof(socket_close_request),
                                         wait_period,
                                         &buffer);

  // If the status is not OK and there's a buffer, free the buffer
  if ((status != SL_STATUS_OK) && (buffer != NULL)) {
    sli_si91x_host_free_buffer(buffer);
  }
  SLI_SOCKET_VERIFY_STATUS_AND_RETURN(status, SL_STATUS_OK, SLI_SI91X_UNDEFINED_ERROR);

  sli_si91x_queue_packet_t *node = sli_wifi_host_get_buffer_data(buffer, 0, NULL);
  if (node->host_packet == NULL) {
    sli_si91x_host_free_buffer(buffer);
    return SL_STATUS_FAIL;
  }

  sl_wifi_buffer_t *response_buffer = node->host_packet;
  sli_si91x_host_free_buffer(buffer);

  sl_wifi_system_packet_t *packet = sli_wifi_host_get_buffer_data(response_buffer, 0, NULL);
  socket_close_response           = (sl_si91x_socket_close_response_t *)packet->data;

  if (close_request_type == SHUTDOWN_BY_ID && si91x_socket->id == socket_close_response->socket_id) {
    sli_si91x_free_socket(socket);
    sli_si91x_host_free_buffer(response_buffer);
    return SLI_SI91X_NO_ERROR;
  }
  // Reset sockets that match the close request
  for (uint8_t index = 0; index < SLI_NUMBER_OF_SOCKETS; index++) {
    const sli_si91x_socket_t *socket_id = sli_get_si91x_socket(index);
    //Verifying socket existence
    if (socket_id == NULL)
      continue;
    else if (close_request_type == SHUTDOWN_BY_PORT
             && socket_id->local_address.sin6_port == socket_close_response->port_number) {
      sli_si91x_free_socket(index);
    }
  }

  sli_si91x_host_free_buffer(response_buffer);

  return SLI_SI91X_NO_ERROR;
}

/**
 * @brief Helper: Handle socket accept request response
 */
static sl_status_t sli_handle_socket_accept_request(sl_wifi_system_packet_t *rx_packet)
{
  const sli_si91x_rsp_ltcp_est_t *accept_response = (sli_si91x_rsp_ltcp_est_t *)rx_packet->data;
  sli_si91x_socket_t *server_socket               = sli_si91x_get_socket_from_port(accept_response->src_port_num);

  if (server_socket == NULL) {
    return SL_STATUS_FAIL;
  }

  int32_t client_socket_id          = server_socket->client_id;
  server_socket->client_id          = -1;
  sli_si91x_socket_t *client_socket = sli_get_si91x_socket(client_socket_id);

  sli_handle_accept_response(client_socket, accept_response);

  if (server_socket->user_accept_callback != NULL) {
    server_socket->user_accept_callback(client_socket_id,
                                        (struct sockaddr *)&server_socket->remote_address,
                                        (uint8_t)server_socket->type);
  }

  return SL_STATUS_OK;
}

/**
 * @brief Helper: Handle remote socket termination
 */
static void sli_handle_remote_terminate(sl_wifi_system_packet_t *rx_packet)
{
  const sl_si91x_socket_close_response_t *remote_socket_closure = (sl_si91x_socket_close_response_t *)rx_packet->data;

  // Reset sockets that match the close request
  for (uint8_t index = 0; index < SLI_NUMBER_OF_SOCKETS; index++) {
    sli_si91x_socket_t *socket = sli_get_si91x_socket(index);

    if (socket == NULL || remote_socket_closure->socket_id != socket->id || socket->state == LISTEN) {
      continue;
    }

    socket->state             = DISCONNECTED;
    socket->disconnect_reason = SLI_SI91X_BSD_DISCONNECT_REASON_REMOTE_CLOSED;
    uint16_t frame_status     = sli_get_si91x_frame_status(rx_packet);
    frame_status = (frame_status == SL_STATUS_OK) ? (SL_STATUS_SI91X_SOCKET_CLOSED & 0xFFFF) : frame_status;

    sli_si91x_flush_socket_command_queues_based_on_queue_type(index, frame_status);
    sli_si91x_flush_socket_data_queues_based_on_queue_type(index);

    if (user_remote_socket_termination_callback != NULL) {
      user_remote_socket_termination_callback(socket->id,
                                              socket->local_address.sin6_port,
                                              remote_socket_closure->sent_bytes_count);
    }
    break;
  }
}

/**
 * @brief Helper: Find host socket index by firmware socket ID with break optimization
 */
static int sli_find_host_socket_by_firmware_id(int firmware_socket_id)
{
  for (uint8_t host_socket_index = 0; host_socket_index < SLI_NUMBER_OF_SOCKETS; host_socket_index++) {
    if ((sli_si91x_sockets[host_socket_index] != NULL)
        && (firmware_socket_id == sli_si91x_sockets[host_socket_index]->id)) {
      return host_socket_index;
    }
  }
  return -1; // Not found
}

/**
 * @brief Helper: Handle raw data receive event
 */
static sl_status_t sli_handle_raw_data_receive(sl_wifi_system_packet_t *rx_packet,
                                               sli_si91x_socket_context_t *sdk_context)
{
  const sl_si91x_socket_metadata_t *firmware_socket_response = (sl_si91x_socket_metadata_t *)rx_packet->data;
  uint8_t *data                                              = (rx_packet->data + firmware_socket_response->offset);

  // Find the host socket corresponding to the received data
  int32_t host_socket = sli_find_host_socket_by_firmware_id(firmware_socket_response->socket_id);

  // Retrieve the client socket
  const sli_si91x_socket_t *client_socket = sli_get_si91x_socket(host_socket);

  if (client_socket == NULL) {
    SL_CLEANUP_MALLOC(sdk_context);
    return SL_STATUS_FAIL;
  }

  // Call the user-defined receive data callback
  client_socket->recv_data_callback(host_socket, data, firmware_socket_response->length, firmware_socket_response);
  return SL_STATUS_OK;
}

/**
 * @brief Helper: Validate select response parameters
 */
static bool sli_is_valid_select_response(const sli_si91x_socket_select_rsp_t *response)
{
  if (response == NULL) {
    return false;
  }

  return (response->select_id < sli_si91x_max_select_count && select_request_table != NULL
          && select_request_table[response->select_id].in_use);
}

/**
 * @brief Helper: Allocate file descriptor sets for select operation
 */
static sl_status_t sli_allocate_fd_sets(void **read_fd, void **write_fd, void **exception_fd)
{
  // Initialize pointers to NULL for safe cleanup
  *read_fd      = NULL;
  *write_fd     = NULL;
  *exception_fd = NULL;

#ifndef __ZEPHYR__
  *read_fd      = malloc(sizeof(fd_set));
  *write_fd     = malloc(sizeof(fd_set));
  *exception_fd = malloc(sizeof(fd_set));
#else
  *read_fd = malloc(sizeof(sl_si91x_fdset_t));
  *write_fd = malloc(sizeof(sl_si91x_fdset_t));
  *exception_fd = malloc(sizeof(sl_si91x_fdset_t));
#endif

  // Check if any allocation failed
  if (*read_fd == NULL || *write_fd == NULL || *exception_fd == NULL) {
    return SL_STATUS_ALLOCATION_FAILED;
  }

  return SL_STATUS_OK;
}

/**
 * @brief Helper:Cleanup allocated file descriptor sets safely
 */
static void sli_cleanup_fd_sets(void *read_fd, void *write_fd, void *exception_fd)
{
  if (read_fd != NULL) {
    free(read_fd);
  }
  if (write_fd != NULL) {
    free(write_fd);
  }
  if (exception_fd != NULL) {
    free(exception_fd);
  }
}

/**
 * @brief Helper: Handle select request response
 */
static sl_status_t sli_handle_select_request(sl_wifi_system_packet_t *rx_packet,
                                             sli_si91x_socket_context_t *sdk_context)
{
  const sli_si91x_socket_select_rsp_t *socket_select_rsp = (sli_si91x_socket_select_rsp_t *)rx_packet->data;

  if (!sli_is_valid_select_response(socket_select_rsp)) {
    SL_DEBUG_LOG("\r\n INVALID SELECT ID\r\n");
    return SL_STATUS_OK;
  }

  sli_si91x_select_request_t *select_request = &select_request_table[socket_select_rsp->select_id];
  select_request->frame_status               = (uint16_t)(rx_packet->desc[12] + (rx_packet->desc[13] << 8));

  if (select_request->select_callback != NULL) {
    void *read_fd      = NULL;
    void *write_fd     = NULL;
    void *exception_fd = NULL;

    sl_status_t alloc_status = sli_allocate_fd_sets(&read_fd, &write_fd, &exception_fd);
    if (alloc_status != SL_STATUS_OK) {
      sli_cleanup_fd_sets(read_fd, write_fd, exception_fd);
      SL_DEBUG_LOG("\r\n HEAP EXHAUSTED DURING ALLOCATION \r\n");
      sli_si91x_clear_select_id(select_request->select_id);
      SL_CLEANUP_MALLOC(sdk_context);
      return SL_STATUS_ALLOCATION_FAILED;
    }

#ifndef __ZEPHYR__
    memset(read_fd, 0, sizeof(fd_set));
    memset(write_fd, 0, sizeof(fd_set));
    memset(exception_fd, 0, sizeof(fd_set));
#else
    memset(read_fd, 0, sizeof(sl_si91x_fdset_t));
    memset(write_fd, 0, sizeof(sl_si91x_fdset_t));
    memset(exception_fd, 0, sizeof(sl_si91x_fdset_t));
#endif

    // This function handles responses received from the SI91X socket driver
    sli_handle_select_response(socket_select_rsp, read_fd, write_fd, exception_fd);

    // Call the user-defined select callback function with the updated file descriptor sets and status
    select_request->select_callback(read_fd, write_fd, exception_fd, select_request->frame_status);

    sli_si91x_clear_select_id(select_request->select_id);
    sli_cleanup_fd_sets(read_fd, write_fd, exception_fd);
  } else {
    select_request->response_data = malloc(sizeof(sli_si91x_socket_select_rsp_t));
    if (select_request->response_data == NULL) {
      SL_DEBUG_LOG("\r\n HEAP EXHAUSTED DURING ALLOCATION \r\n");
    } else {
      memcpy(select_request->response_data, rx_packet->data, sizeof(sli_si91x_socket_select_rsp_t));
      osEventFlagsSet(si91x_socket_select_events, BIT(socket_select_rsp->select_id));
    }
  }

  return SL_STATUS_OK;
}

/**
 * @brief Helper: Handle TCP acknowledgment indication
 */
static int sli_handle_tcp_ack_indication(sl_wifi_system_packet_t *rx_packet, sli_si91x_socket_context_t *sdk_context)
{
  const sli_si91x_rsp_tcp_ack_t *tcp_ack = (sli_si91x_rsp_tcp_ack_t *)rx_packet->data;

  // Find the host socket associated with the TCP acknowledgment
  int32_t host_socket = sli_find_host_socket_by_firmware_id(tcp_ack->socket_id);

  // Retrieve the SI91X socket associated with the host socket
  sli_si91x_socket_t *si91x_socket = sli_get_si91x_socket(host_socket);

  if (si91x_socket == NULL) {
    SL_CLEANUP_MALLOC(sdk_context);
    return -1;
  }

  // Check if the SLI_SI91X_SOCKET_FEAT_TCP_ACK_INDICATION bit is set move the socket to CONNECTED state.
  if (si91x_socket->socket_bitmap & SLI_SI91X_SOCKET_FEAT_TCP_ACK_INDICATION) {
    si91x_socket->is_waiting_on_ack = false;
  }

  // Check if the SI91X socket and its data transfer callback function exist
  if (si91x_socket->data_transfer_callback != NULL) {
    si91x_socket->data_transfer_callback(host_socket, (uint8_t)(tcp_ack->length[0] | tcp_ack->length[1] << 8));
  }

  return 0;
}

sl_status_t sli_si91x_socket_event_handler(sl_status_t status,
                                           sli_si91x_socket_context_t *sdk_context,
                                           sl_wifi_system_packet_t *rx_packet)
{
  UNUSED_PARAMETER(status);

  int result = 0;

  switch (rx_packet->command) {
      // Handle connection establishment response
    case SLI_WLAN_REQ_SOCKET_ACCEPT: {
      result = sli_handle_socket_accept_request(rx_packet);
      if (result != SL_STATUS_OK) {
        return -1;
      }
      break;
    }
    // Handle remote socket termination response
    case SLI_WLAN_RSP_REMOTE_TERMINATE: {
      sli_handle_remote_terminate(rx_packet);
      break;
    }

    case SLI_RECEIVE_RAW_DATA: {
      // Handle the case when raw data is received
      result = sli_handle_raw_data_receive(rx_packet, sdk_context);
      if (result != SL_STATUS_OK) {
        return -1;
      }
      break;
    }

    case SLI_WLAN_RSP_SELECT_REQUEST: {
      // Handle select request response
      result = sli_handle_select_request(rx_packet, sdk_context);
      if (result != SL_STATUS_OK) {
        return result;
      }
      break;
    }

    // This block of code is executed when a TCP acknowledgment indication is received.
    case SLI_WLAN_RSP_TCP_ACK_INDICATION: {
      result = sli_handle_tcp_ack_indication(rx_packet, sdk_context);
      if (result == -1) {
        return -1;
      }
      break;
    }

    default:
      break;
  }
  // Cleanup any dynamically allocated memory in the SDK context
  SL_CLEANUP_MALLOC(sdk_context);

  return SL_STATUS_OK;
}

sl_status_t sli_si91x_send_socket_command(sli_si91x_socket_t *socket,
                                          uint32_t command,
                                          const void *data,
                                          uint32_t data_length,
                                          uint32_t wait_period,
                                          sl_wifi_buffer_t **response_buffer)

{
  sl_wifi_buffer_t *buffer        = NULL;
  sl_wifi_system_packet_t *packet = NULL;
  sl_wifi_buffer_t *node_buffer   = NULL;
  sli_si91x_queue_packet_t *node  = NULL;
  sl_status_t status;
  static uint8_t command_packet_id = 0;

  // Allocate a buffer for the command with appropriate size
  status = sli_si91x_allocate_command_buffer(&buffer,
                                             (void **)&packet,
                                             sizeof(sl_wifi_system_packet_t) + data_length,
                                             SLI_WIFI_ALLOCATE_COMMAND_BUFFER_WAIT_TIME);
  VERIFY_STATUS_AND_RETURN(status);

  // Allocate a queue node
  status = sli_si91x_allocate_command_buffer(&node_buffer,
                                             (void **)&node,
                                             sizeof(sli_si91x_queue_packet_t),
                                             SLI_WIFI_ALLOCATE_COMMAND_BUFFER_WAIT_TIME);
  if (status != SL_STATUS_OK) {
    sli_si91x_host_free_buffer(buffer);
    return status;
  }

  // Clear the packet descriptor and copy the command data if available
  memset(packet->desc, 0, sizeof(packet->desc));
  if (data != NULL) {
    memcpy(packet->data, data, data_length);
  }

  // Fill frame type
  packet->length  = data_length & 0xFFF;
  packet->command = (uint16_t)command;

  // Set flags
#ifdef TEST_USE_UNUSED_FLAGS
  packet->unused[SLI_SI91X_COMMAND_FLAGS_INDEX] = (wait_period & SLI_WIFI_WAIT_FOR_RESPONSE_BIT) ? (1 << 0) : 0;
  packet->unused[SLI_SI91X_COMMAND_FLAGS_INDEX] |= (response_buffer == NULL) ? (1 << 1) : 0;
  if (command == SLI_WLAN_REQ_SOCKET_ACCEPT) {
    packet->unused[SLI_SI91X_COMMAND_RESPONSE_INDEX] = SLI_WLAN_RSP_CONN_ESTABLISH;
  } else {
    packet->unused[SLI_SI91X_COMMAND_RESPONSE_INDEX] = command;
  }
#else
  node->flags = (wait_period & SLI_WIFI_WAIT_FOR_RESPONSE_BIT) ? SLI_WIFI_PACKET_RESPONSE_PACKET : 0;
#endif

  wait_period &= ~SLI_WIFI_WAIT_FOR_RESPONSE_BIT;

  if (wait_period != 0) {
    node->flags |= SLI_WIFI_PACKET_RESPONSE_STATUS;
  }

  // Set various properties of the node representing the command packet
  node->host_packet       = buffer;
  node->firmware_queue_id = SLI_WLAN_MGMT_Q;
  node->command_type      = SLI_SI91X_SOCKET_CMD;

  if (node->flags != SLI_WIFI_PACKET_WITH_ASYNC_RESPONSE) {
    node->command_tickcount = osKernelGetTickCount();
    // Calculate the wait time based on wait_period
    if ((wait_period & SLI_WIFI_WAIT_FOR_EVER) == SLI_WIFI_WAIT_FOR_EVER) {
      node->command_timeout = osWaitForever;
    } else {
      node->command_timeout = (wait_period & ~SLI_WIFI_WAIT_FOR_RESPONSE_BIT);
    }
  }
  node->sdk_context = NULL;
  node->event_mask  = (1 << socket->index);

  CORE_irqState_t state = CORE_EnterAtomic();
  if (socket->state == DISCONNECTED && command == SLI_WLAN_REQ_SOCKET_READ_DATA) {
    CORE_ExitAtomic(state);
    errno = ENOTCONN;
    return SL_STATUS_SI91X_SOCKET_IN_UNCONNECTED_STATE;
  }
  uint8_t this_packet_id = command_packet_id;
  command_packet_id++;
  buffer->id      = this_packet_id;
  node_buffer->id = this_packet_id;
  sli_wifi_append_to_buffer_queue(&socket->command_queue.tx_queue, node_buffer);
  tx_socket_command_queues_status |= (1 << socket->index);
  sli_wifi_set_event(SL_SI91X_SOCKET_COMMAND_TX_PENDING_EVENT);
  CORE_ExitAtomic(state);

  if (wait_period != 0) {

    uint16_t firmware_status = 0;
    sli_wifi_buffer_queue_t *rx_queue;
    if (command == SLI_WLAN_REQ_SOCKET_READ_DATA) {
      rx_queue = &socket->rx_data_queue;
    } else {
      rx_queue = &socket->command_queue.rx_queue;
    }

    status = sli_wifi_wait_for_response_packet(rx_queue,
                                               si91x_socket_events,
                                               (1 << socket->index),
                                               this_packet_id,
                                               wait_period,
                                               response_buffer);
    VERIFY_STATUS_AND_RETURN(status);

    if (command == SLI_WLAN_REQ_SOCKET_READ_DATA) {
      packet          = (sl_wifi_system_packet_t *)sli_wifi_host_get_buffer_data(*response_buffer, 0, NULL);
      firmware_status = (uint16_t)(packet->desc[12] + (packet->desc[13] << 8)); // Extract the frame status

    } else {
      // Process the response packet and return the firmware status
      node            = (sli_si91x_queue_packet_t *)sli_wifi_host_get_buffer_data(*response_buffer, 0, NULL);
      firmware_status = node->frame_status;
    }
    return sli_convert_and_save_firmware_status(firmware_status);
  } else {
    return SL_STATUS_OK;
  }
}

sl_status_t sli_si91x_send_socket_data(sli_si91x_socket_t *si91x_socket,
                                       const sli_si91x_socket_send_request_t *request,
                                       const void *data)
{
  sl_wifi_buffer_t *buffer        = NULL;
  sl_wifi_system_packet_t *packet = NULL;
  sli_si91x_socket_send_request_t *send;

  sl_status_t status     = SL_STATUS_OK;
  uint16_t header_length = (request->data_offset - sizeof(sli_si91x_socket_send_request_t));
  uint32_t data_length   = request->length;

  if (data == NULL) {
    return SL_STATUS_NULL_POINTER;
  }

  uint32_t start = osKernelGetTickCount();
  while (si91x_socket->data_buffer_limit != 0 && si91x_socket->data_buffer_count >= si91x_socket->data_buffer_limit) {
    osDelay(SLI_SYSTEM_MS_TO_TICKS(2));
    if ((osKernelGetTickCount() - start) > SLI_WIFI_ALLOCATE_COMMAND_BUFFER_WAIT_TIME) {
      return SL_STATUS_WIFI_BUFFER_ALLOC_FAIL;
    }
  }

  // Allocate a buffer for the socket data with appropriate size
  status = sli_si91x_host_allocate_buffer(
    &buffer,
    SL_WIFI_TX_FRAME_BUFFER,
    sizeof(sl_wifi_system_packet_t) + sizeof(sli_si91x_socket_send_request_t) + header_length + data_length,
    SLI_WIFI_ALLOCATE_COMMAND_BUFFER_WAIT_TIME);
  VERIFY_STATUS_AND_RETURN(status);

  packet = sli_wifi_host_get_buffer_data(buffer, 0, NULL);
  if (packet == NULL) {
    sli_si91x_host_free_buffer(buffer);
    return SL_STATUS_WIFI_BUFFER_ALLOC_FAIL;
  }

  // Atomic protection for data_buffer_count to prevent race condition
  CORE_irqState_t state1 = CORE_EnterAtomic();
  ++si91x_socket->data_buffer_count;
  CORE_ExitAtomic(state1);

  memset(packet->desc, 0, sizeof(packet->desc));

  send = (sli_si91x_socket_send_request_t *)packet->data;
  memcpy(send, request, sizeof(sli_si91x_socket_send_request_t));
  memcpy((send->send_buffer + header_length), data, data_length);

  // Fill frame type
  packet->length = (sizeof(sli_si91x_socket_send_request_t) + header_length + data_length) & 0xFFF;

  CORE_irqState_t state = CORE_EnterAtomic();
  sli_wifi_append_to_buffer_queue(&si91x_socket->tx_data_queue, buffer);
  tx_socket_data_queues_status |= (1 << si91x_socket->index);
  sli_wifi_set_event(SL_SI91X_SOCKET_DATA_TX_PENDING_EVENT);
  CORE_ExitAtomic(state);

  return SL_STATUS_OK;
}

/**
 * @brief Helper: Find socket ID by port number and LISTEN state
 * */
static int sli_find_socket_by_port_and_listen_state(uint16_t port)
{
  for (int i = 0; i < SLI_NUMBER_OF_SOCKETS; ++i) {
    if (sli_si91x_sockets[i] != NULL && sli_si91x_sockets[i]->local_address.sin6_port == port
        && sli_si91x_sockets[i]->state == LISTEN) {
      return sli_si91x_sockets[i]->id;
    }
  }
  return -1;
}

int sli_si91x_get_socket_id(sl_wifi_system_packet_t *packet)
{
  // Handle connection establishment response
  switch (packet->command) {
    case SLI_WLAN_RSP_CONN_ESTABLISH:
      return ((sli_si91x_rsp_ltcp_est_t *)packet->data)->socket_id;
    case SLI_WLAN_RSP_REMOTE_TERMINATE:
      return ((sl_si91x_socket_close_response_t *)packet->data)->socket_id;
    case SLI_RECEIVE_RAW_DATA:
      return *((uint8_t *)&(((sl_si91x_socket_metadata_t *)packet->data)->socket_id));
    case SLI_WLAN_RSP_SOCKET_READ_DATA:
      return packet->data[0];
    case SLI_WLAN_RSP_TCP_ACK_INDICATION:
      return ((sli_si91x_rsp_tcp_ack_t *)packet->data)->socket_id;
    case SLI_WLAN_RSP_SOCKET_CREATE:
      return (((sli_si91x_socket_create_response_t *)packet->data)->socket_id[0]
              + (((sli_si91x_socket_create_response_t *)packet->data)->socket_id[1] << 8));
    case SLI_WLAN_RSP_SOCKET_CLOSE:
      if (((sl_si91x_socket_close_response_t *)packet->data)->socket_id == 0) {
        const uint16_t port = ((sl_si91x_socket_close_response_t *)packet->data)->port_number;
        return sli_find_socket_by_port_and_listen_state(port);
      } else {
        return ((sl_si91x_socket_close_response_t *)packet->data)->socket_id;
      }
    case SLI_WLAN_RSP_SELECT_REQUEST:
      __attribute__((fallthrough));
    default:
      return -1;
  }
}

int sli_si91x_connect(int socket, const struct sockaddr *addr, socklen_t addr_len)
{
  errno = 0;

  sl_status_t status = SL_STATUS_FAIL;
  sli_si91x_socket_t *si91x_socket;

  // Retrieve the socket using the socket index
  si91x_socket = sli_get_si91x_socket(socket);

  // Check if the socket is valid.
  SLI_SET_ERRNO_AND_RETURN_IF_TRUE(si91x_socket == NULL, EBADF);

  // Check if the socket is already connected
  SLI_SET_ERRNO_AND_RETURN_IF_TRUE(si91x_socket->type == SOCK_STREAM && si91x_socket->state == CONNECTED, EISCONN);

  // Check the socket state based on its type
  SLI_SET_ERRNO_AND_RETURN_IF_TRUE(si91x_socket->type == SOCK_STREAM && si91x_socket->state > BOUND, EBADF);
  SLI_SET_ERRNO_AND_RETURN_IF_TRUE(si91x_socket->type == SOCK_DGRAM && si91x_socket->state != INITIALIZED
                                     && si91x_socket->state != BOUND && si91x_socket->state != UDP_UNCONNECTED_READY,
                                   EBADF);

  // Check if the provided sockaddr length is sufficient
  SLI_SET_ERRNO_AND_RETURN_IF_TRUE(
    (si91x_socket->local_address.sin6_family == AF_INET && addr_len < sizeof(struct sockaddr_in))
      || (si91x_socket->local_address.sin6_family == AF_INET6 && addr_len < sizeof(struct sockaddr_in6)),
    EINVAL);

  // Check if the provided sockaddr pointer is valid
  SLI_SET_ERRNO_AND_RETURN_IF_TRUE(addr == NULL, EFAULT);

  SLI_SET_ERRNO_AND_RETURN_IF_TRUE(si91x_socket->local_address.sin6_family != addr->sa_family, EAFNOSUPPORT);

  memcpy(&si91x_socket->remote_address,
         addr,
         (addr_len > sizeof(struct sockaddr_in6)) ? sizeof(struct sockaddr_in6) : addr_len);

  // Since socket is already created, there is no need to send create request again.
  if (si91x_socket->type == SOCK_DGRAM && si91x_socket->state == UDP_UNCONNECTED_READY) {
    si91x_socket->state = CONNECTED;

    return SLI_SI91X_NO_ERROR;
  }

  // Prepare socket request based on socket type and send the request to the bus driver
  if (si91x_socket->type == SOCK_STREAM) {
    status = sli_create_and_send_socket_request(socket, SLI_SI91X_SOCKET_TCP_CLIENT, NULL);
  } else if (si91x_socket->type == SOCK_DGRAM) {
    status = sli_create_and_send_socket_request(socket, SLI_SI91X_SOCKET_UDP_CLIENT, NULL);
  }

  // Verify the status of the socket operation and return errors if necessary
  SLI_SOCKET_VERIFY_STATUS_AND_RETURN(status, SL_STATUS_OK, SLI_SI91X_UNDEFINED_ERROR);

  // Update the socket state to "CONNECTED" and return success
  si91x_socket->state = CONNECTED;
  return SLI_SI91X_NO_ERROR;
}

int sli_si91x_bind(int socket_id, const struct sockaddr *addr, socklen_t addr_len)
{
  errno = 0;

  // Retrieve the SI91X socket associated with the given socket ID.
  sli_si91x_socket_t *si91x_socket         = sli_get_si91x_socket(socket_id);
  const struct sockaddr_in *socket_address = (const struct sockaddr_in *)addr;

  // Validate socket, address, and address length
  SLI_SET_ERRNO_AND_RETURN_IF_TRUE(si91x_socket == NULL || si91x_socket->state != INITIALIZED, EBADF);
  SLI_SET_ERRNO_AND_RETURN_IF_TRUE(
    (si91x_socket->local_address.sin6_family == AF_INET && addr_len < sizeof(struct sockaddr_in))
      || (si91x_socket->local_address.sin6_family == AF_INET6 && addr_len < sizeof(struct sockaddr_in6)),
    EINVAL);

  SLI_SET_ERRNO_AND_RETURN_IF_TRUE(addr == NULL, EFAULT);

  // Check whether local port is already used or not
  if (!sli_is_port_available(socket_address->sin_port)) {
    SLI_SET_ERROR_AND_RETURN(EADDRINUSE);
  }

  // Copy the provided address to the local address structure
  memcpy(&si91x_socket->local_address,
         addr,
         (addr_len > sizeof(struct sockaddr_in6)) ? sizeof(struct sockaddr_in6) : addr_len);

  si91x_socket->state = BOUND;

  // For UDP sockets, create and send a socket request.
  if (si91x_socket->type == SOCK_DGRAM) {
    sl_status_t socket_create_request_status =
      sli_create_and_send_socket_request(socket_id, SLI_SI91X_SOCKET_UDP_CLIENT, NULL);
    SLI_SOCKET_VERIFY_STATUS_AND_RETURN(socket_create_request_status, SLI_SI91X_NO_ERROR, SLI_SI91X_UNDEFINED_ERROR);

    si91x_socket->state = UDP_UNCONNECTED_READY;
  }

  return SLI_SI91X_NO_ERROR;
}

#ifndef __ZEPHYR__
bool sli_check_null_sets_and_range(int nfds,
                                   const fd_set *readfds,
                                   const fd_set *writefds,
                                   const struct timeval *timeout)
#else
bool sli_check_null_sets_and_range(int nfds,
                                   const sl_si91x_fdset_t *readfds,
                                   const sl_si91x_fdset_t *writefds,
                                   const struct timeval *timeout)
#endif
{

  // Check if all file descriptor sets are NULL
  if ((readfds == NULL) && (writefds == NULL)) {
    return true; // Invalid argument, no sets specified
  }

  // Check if the number of file descriptors (nfds) is within a valid range
  if (nfds < 0 || nfds > SLI_NUMBER_OF_SOCKETS) {
    return true; // Invalid argument, nfds out of range
  }

  // Check if the provided timeout is valid
  if ((timeout != NULL) && ((timeout->tv_sec < 0) || (timeout->tv_usec < 0))) {
    return true; // Invalid argument, negative timeout
  }
  return false;
}

#ifndef __ZEPHYR__
uint8_t sli_prepare_select_request(int nfds,
                                   const fd_set *readfds,
                                   const fd_set *writefds,
                                   sli_si91x_socket_select_req_t *request)
#else
uint8_t sli_prepare_select_request(int nfds,
                                   const sl_si91x_fdset_t *readfds,
                                   const sl_si91x_fdset_t *writefds,
                                   sli_si91x_socket_select_req_t *request)
#endif
{

  // Loop through the provided file descriptor sets and populate the select request structure
  for (uint8_t host_socket_index = 0; host_socket_index < nfds; host_socket_index++) {
    // Retrieve the si91x_socket associated with the index
    const sli_si91x_socket_t *socket = sli_get_si91x_socket(host_socket_index);

    // Throw error if the socket file descriptor set is invalid
    if (socket == NULL
#ifndef __ZEPHYR__
        && ((readfds != NULL && FD_ISSET(host_socket_index, readfds))
            || (writefds != NULL && FD_ISSET(host_socket_index, writefds)))) {
#else
        && ((readfds != NULL && SL_SI91X_FD_ISSET(host_socket_index, readfds))
            || (writefds != NULL && SL_SI91X_FD_ISSET(host_socket_index, writefds)))) {
#endif
      return EBADF; // Bad file descriptor
    }

    // The code will reach this if clause in the case of a socket being NULL and the socket being neither set in readfds nor writefds.
    // Continue to next socket if this one is not in use
    if (socket == NULL) {
      continue;
    }

    // Check if the socket is set for read operations in the readfds set
    // Set the corresponding bit in the read file descriptor set
#ifndef __ZEPHYR__
    if ((readfds != NULL) && (FD_ISSET(host_socket_index, readfds))) {
#else
    if ((readfds != NULL) && (SL_SI91X_FD_ISSET(host_socket_index, readfds))) {
#endif
      request->read_fds.fd_array[0] |= (1U << socket->id);
    }

    // Check if the socket is set for write operations in the writefds set
    // Set the corresponding bit in the write file descriptor set
#ifndef __ZEPHYR__
    if ((writefds != NULL) && (FD_ISSET(host_socket_index, writefds))) {
#else
    if ((writefds != NULL) && (SL_SI91X_FD_ISSET(host_socket_index, writefds))) {
#endif
      request->write_fds.fd_array[0] |= (1U << socket->id);
    }

    // Update the maximum file descriptor number encountered
    if (request->num_fd <= socket->id) {
      request->num_fd = (uint8_t)(socket->id + 1);
    }
  }
  return 0; // Indicate success
}

void sli_handle_timeout(const struct timeval *timeout, sli_si91x_socket_select_req_t *request, uint32_t *wait_time)
{
  request->select_timeout.tv_sec  = timeout->tv_sec;
  request->select_timeout.tv_usec = timeout->tv_usec;
  // Convert timeout to milliseconds and add extra wait time for the response
  *wait_time = ((request->select_timeout.tv_sec * 1000) + (request->select_timeout.tv_usec / 1000)
                + SLI_SI91X_HOST_WAIT_FOR_SELECT_RSP);
}

sl_status_t sli_wait_for_select_response(const sli_si91x_socket_select_req_t *request, uint32_t wait_time)
{
  // Start measuring the time for the select operation
  uint32_t start_time   = osKernelGetTickCount();
  uint32_t elapsed_time = 0;
  sl_status_t status    = SL_STATUS_OK;

  if (wait_time != osWaitForever) {
    wait_time = SLI_SYSTEM_MS_TO_TICKS(wait_time);
  }

  do {
    // Wait for the select response event (using the select_id)
    uint32_t events =
      osEventFlagsWait(si91x_socket_select_events, BIT(request->select_id), osFlagsWaitAny, (wait_time - elapsed_time));

    // Handle cases where the wait times out or resources are unavailable
    if (events == (uint32_t)osErrorTimeout || events == (uint32_t)osErrorResource) {
      return SL_STATUS_TIMEOUT; // Set status to timeout if no response was received
    }

    // Check if the response data for the select request is available
    if (select_request_table[request->select_id].response_data != NULL) {
      break; // Exit the loop if response is received
    }

    // Update the elapsed time
    elapsed_time = sl_si91x_host_elapsed_time(start_time);
  } while (elapsed_time <= wait_time);

  return status;
}

#ifndef __ZEPHYR__
int32_t sli_handle_select_result(const sli_si91x_select_request_t *select_request,
                                 fd_set *readfds,
                                 fd_set *writefds,
                                 fd_set *exceptfds)
#else
int32_t sli_handle_select_result(const sli_si91x_select_request_t *select_request,
                                 sl_si91x_fdset_t *readfds,
                                 sl_si91x_fdset_t *writefds,
                                 sl_si91x_fdset_t *exceptfds)
#endif
{
  // Initialize the total file descriptor count
  int32_t total_fd_set_count = -1;
  // If the firmware status is OK, process the select response and update the file descriptor sets
  if (select_request_table[select_request->select_id].frame_status == SL_STATUS_OK) {
    total_fd_set_count = sli_handle_select_response(select_request_table[select_request->select_id].response_data,
                                                    readfds,
                                                    writefds,
                                                    exceptfds);
  }

  // Free the memory allocated for the response data
  free(select_request_table[select_request->select_id].response_data);
  // Clear the select ID in the internal table
  sli_si91x_clear_select_id(select_request->select_id);

  // Return the total number of file descriptors set in the read, write, or exception sets
  return total_fd_set_count;
}

#ifndef __ZEPHYR__
int sli_si91x_select(int nfds,
                     fd_set *readfds,
                     fd_set *writefds,
                     fd_set *exceptfds,
                     const struct timeval *timeout,
                     sl_si91x_socket_select_callback_t callback)
#else
int sli_si91x_select(int nfds,
                     sl_si91x_fdset_t *readfds,
                     sl_si91x_fdset_t *writefds,
                     sl_si91x_fdset_t *exceptfds,
                     const struct timeval *timeout,
                     sl_si91x_socket_select_callback_t callback)
#endif
{
  UNUSED_PARAMETER(exceptfds);                       // exceptfds is not supported by the firmware, so it is unused
  sl_status_t status                 = SL_STATUS_OK; // Initialize status
  uint32_t select_response_wait_time = 0;            // Time to wait for the select response

  // Define a structure to hold the select request parameters
  sli_si91x_socket_select_req_t request = { 0 };

  if (sli_check_null_sets_and_range(nfds, readfds, writefds, timeout)) {
    SLI_SET_ERROR_AND_RETURN(EINVAL);
  }

  // Prepare the select request structure
  uint8_t error = sli_prepare_select_request(nfds, readfds, writefds, &request);
  if (error != 0) {
    SLI_SET_ERROR_AND_RETURN(EBADF);
  }

  if (timeout != NULL) {
    sli_handle_timeout(timeout, &request, &select_response_wait_time);
  } else {
    // If no timeout is specified, set the request to indicate no timeout and wait indefinitely
    request.no_timeout        = 1;
    select_response_wait_time = osWaitForever;
  }

  // Get an available select ID from the internal table
  sli_si91x_select_request_t *select_request = sli_si91x_get_available_select_id();
  // If no select ID is available, return an error
  SLI_SET_ERRNO_AND_RETURN_IF_TRUE((select_request == NULL), EPERM);
  // Assign the callback function for this select request
  select_request->select_callback = callback;
  // Set the select_id in the request structure
  request.select_id = select_request->select_id;

  // Send the select request asynchronously to the firmware
  status =
    sli_si91x_driver_send_async_command(SLI_WLAN_REQ_SELECT_REQUEST, SLI_SI91X_SOCKET_CMD, &request, sizeof(request));
  if (status != SL_STATUS_OK) {
    // If sending the command fails, clear the select ID and return
    sli_si91x_clear_select_id(request.select_id);
  }
  // Verify that the command was sent successfully
  SLI_SOCKET_VERIFY_STATUS_AND_RETURN(status, SL_STATUS_OK, SLI_SI91X_UNDEFINED_ERROR);

  // If a callback was provided, return immediately (non-blocking)
  if (callback != NULL) {
    return SLI_WIFI_RETURN_IMMEDIATELY;
  }

  status = sli_wait_for_select_response(&request, select_response_wait_time);

  // If the select operation timed out or failed, clear the select ID and exit
  if (status != SL_STATUS_OK) {
    sli_si91x_clear_select_id(request.select_id);
  }
  // Verify the status and return if an error occurred
  SLI_SOCKET_VERIFY_STATUS_AND_RETURN(status, SL_STATUS_OK, SLI_SI91X_UNDEFINED_ERROR);

  sli_convert_and_save_firmware_status(select_request_table[request.select_id].frame_status);

  return sli_handle_select_result(select_request, readfds, writefds, exceptfds);
}

static sli_si91x_select_request_t *sli_si91x_get_available_select_id(void)
{
  // Check if there are any available select request entries.
  if (sli_si91x_max_select_count == 0) {
    return NULL; // Return NULL if no requests can be processed.
  }

  // Enter atomic section to ensure thread-safe access to the select_request_table.
  CORE_irqState_t state = CORE_EnterAtomic();

  // Iterate over the select request table to find an available entry.
  for (unsigned int i = 0; i < sli_si91x_max_select_count; i++) {
    // Check if the current entry is not in use.
    if (!select_request_table[i].in_use) {
      // Assign the current index as the select ID.
      select_request_table[i].select_id = (uint8_t)i;

      // Mark the entry as in use.
      select_request_table[i].in_use = 1;

      // Exit atomic section after modifying the entry.
      CORE_ExitAtomic(state);

      // Return the pointer to the available select request entry.
      return &select_request_table[i];
    }
  }

  // Exit atomic section if no available entry was found.
  CORE_ExitAtomic(state);

  // Return NULL to indicate that no available select ID was found.
  return NULL;
}

static void sli_si91x_clear_select_id(uint8_t id)
{
  // Check if the select request table is empty or if the provided ID is out of range.
  if (sli_si91x_max_select_count == 0 || id >= sli_si91x_max_select_count) {
    return; // If no requests or invalid ID, exit early.
  }

  // Enter atomic section to ensure thread-safe access to the select_request_table.
  CORE_irqState_t state = CORE_EnterAtomic();

  // Mark the entry corresponding to the provided ID as not in use.
  select_request_table[id].in_use = 0;

  // Exit atomic section after the operation is complete.
  CORE_ExitAtomic(state);
}

void sli_si91x_set_socket_event(uint32_t event_mask)
{
  osEventFlagsSet(si91x_socket_events, event_mask);
}

sl_status_t sli_si91x_flush_select_request_table(uint16_t error_code)
{
  // Iterate over all entries in the select_request_table
  for (unsigned int i = 0; i < sli_si91x_max_select_count; i++) {
    // Check if the current select_request_table entry is in use
    if (select_request_table[i].in_use) {
      // If a callback function exists for this entry, it is async
      if (select_request_table[i].select_callback != NULL) {
        select_request_table[i].in_use = 0; // Mark as not in use
      } else {
        // If there is no callback, the request needs to be reset and so the response data pointer is cleared, indicating no data is available
        select_request_table[i].response_data = NULL;
        // Set the frame status to indicate rejoin failure in the request
        select_request_table[i].frame_status = error_code;
        // Set the appropriate event flag for the socket associated with the select_id
        osEventFlagsSet(si91x_socket_select_events, BIT(select_request_table[i].select_id));
      }
    }
  }
  // Return SL_STATUS_OK to indicate the function completed successfully
  return SL_STATUS_OK;
}

sl_status_t sli_si91x_udp_connect_if_unconnected(sli_si91x_socket_t *si91x_socket,
                                                 const struct sockaddr *to_addr,
                                                 socklen_t to_addr_len,
                                                 int socket_id)
{
  sl_status_t status = SL_STATUS_OK;

  if (si91x_socket->type == SOCK_DGRAM && (si91x_socket->state == BOUND || si91x_socket->state == INITIALIZED)) {
    if (si91x_socket->ssl_bitmap & SL_SI91X_ENABLE_DTLS) {
      memcpy(&si91x_socket->remote_address,
             to_addr,
             (to_addr_len > sizeof(struct sockaddr_in6)) ? sizeof(struct sockaddr_in6) : to_addr_len);
    }
    status = sli_create_and_send_socket_request(socket_id, SLI_SI91X_SOCKET_UDP_CLIENT, NULL);

    if (status != SL_STATUS_OK) {
      return status;
    }
    si91x_socket->state = UDP_UNCONNECTED_READY;
  }

  return status;
}

void sl_si91x_set_socket_cipherlist(uint32_t cipher_list)
{
  sl_si91x_socket_selected_ciphers = cipher_list;
}

void sl_si91x_set_extended_socket_cipherlist(uint32_t extended_cipher_list)
{
  sl_si91x_socket_selected_extended_ciphers = extended_cipher_list;
}