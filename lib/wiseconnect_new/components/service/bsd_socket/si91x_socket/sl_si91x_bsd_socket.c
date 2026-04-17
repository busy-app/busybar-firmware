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
#include <stdint.h>
#include "sl_si91x_socket_utility.h"
#include "netdb.h"
#include "sl_si91x_driver.h"
#include "sl_rsi_utility.h"
#include "sl_si91x_status.h"
#include "sl_si91x_protocol_types.h"
#include "sl_si91x_host_interface.h"
#include "sl_ip_types.h"
#include "sl_net_dns.h"
#include "netinet_in.h"
#include "netinet6_in6.h"
#include "errno.h"
#ifndef __ZEPHYR__
#include "socket.h"
#else
#include <sys/socket.h>
#endif
#include "select.h"
#include "sl_bsd_utility.h"
#include "sl_si91x_socket_constants.h"
#include "sl_si91x_socket_support.h"
#include "sli_wifi_constants.h"
#include <sl_string.h>
#include "sli_wifi_utility.h"
/******************************************************

 *                    Constants
 ******************************************************/

#define SLI_SI91X_SOCKET_FEAT_TCP_ACK_INDICATION BIT(2)

#define SLI_SI91X_BSD_SOCKET_LOCAL_ADDRESS 0
#define SLI_SI91X_BSD_SOCKET_PEER_ADDRESS  1

#define SLI_TCP_HEADER_LENGTH                    56
#define SLI_UDP_HEADER_LENGTH                    44
#define SLI_TCP_V6_HEADER_LENGTH                 76
#define SLI_UDP_V6_HEADER_LENGTH                 64
#define SLI_SOCKET_LISTEN_RESPONSE_PACKET_LENGTH 42

#define SLI_SI91X_SSL_HEADER_SIZE_IPV4 90
#define SLI_SI91X_SSL_HEADER_SIZE_IPV6 110

#define SLI_WEBSOCKET_SMALL_FRAME_THRESHOLD 126
#define SLI_WEBSOCKET_SMALL_FRAME_OFFSET    6
#define SLI_WEBSOCKET_LARGE_FRAME_OFFSET    8

/******************************************************
 *               Variable Definitions
 ******************************************************/
// Declaration of the global error number variable for hostname resolution errors
static int herrno;
// Structure to hold host entry information for hostname resolution
static struct hostent host_ent;
// IP address structure for host IP address resolution
static sl_ip_address_t host_ip_address;

/******************************************************
 *               Function Definitions
 ******************************************************/

static int32_t sli_map_tos_to_nwp(const void *option_value)
{
  int value = *(const int *)option_value; // Cast the void pointer to an int pointer and dereference it

  // This check is placed to maintain backward compatibility with internal stack, soon to be depricated for future releases to maintain standard TOS values.
  if ((value >= 0) && (value <= 7))
    return *(const int32_t *)option_value;

  switch (value) {
    case IPTOS_PREC_NETCONTROL:
      return 7;
    case IPTOS_PREC_INTERNETCONTROL:
      return 6;
    case IPTOS_PREC_CRITIC_ECP:
      return 5;
    case IPTOS_PREC_FLASHOVERRIDE:
      return 4;
    case IPTOS_PREC_FLASH:
      return 3;
    case IPTOS_PREC_IMMEDIATE:
      return 2;
    case IPTOS_PREC_PRIORITY:
      return 1;
    case IPTOS_PREC_ROUTINE:
      return 0;
    default:
      return -1;
  }
}

// Static helper function to set WebSocket data offset
static void sli_si91x_set_websocket_offset(sli_si91x_socket_send_request_t *request, size_t data_len)
{
  if (data_len < SLI_WEBSOCKET_SMALL_FRAME_THRESHOLD) {
    request->data_offset += SLI_WEBSOCKET_SMALL_FRAME_OFFSET;
  } else {
    request->data_offset += SLI_WEBSOCKET_LARGE_FRAME_OFFSET;
  }
}

int *__h_errno_location(void)
{
  return &herrno;
}

int16_t sl_si91x_get_socket_mss(int32_t socketIndex)
{
  // Retrieve the SI91X socket associated with the given socket index
  const sli_si91x_socket_t *si91x_socket = sli_get_si91x_socket(socketIndex);
  //Verifying socket existence
  if (si91x_socket == NULL)
    return -1;

  // Calculate and return the MSS based on the socket type and state
  if ((si91x_socket->type == SOCK_STREAM) && si91x_socket->state == CONNECTED) {
    if (si91x_socket->ssl_bitmap & SL_SI91X_ENABLE_TLS) {
      // Subtract TLS header size from MSS for TLS connections
      size_t max_size = (si91x_socket->local_address.sin6_family == AF_INET)
                          ? si91x_socket->mss - SLI_SI91X_SSL_HEADER_SIZE_IPV4
                          : si91x_socket->mss - SLI_SI91X_SSL_HEADER_SIZE_IPV6;
      return (int16_t)max_size;
    } else {
      // Adjust MSS based on IP version (IPv4 or IPv6) for non TLS connections
      size_t max_size = (si91x_socket->local_address.sin6_family == AF_INET)
                          ? si91x_socket->mss
                          : si91x_socket->mss - (SLI_TCP_V6_HEADER_LENGTH - SLI_TCP_HEADER_LENGTH);
      return (int16_t)max_size;
    }
  } else if (si91x_socket->type == SOCK_DGRAM) {
    size_t max_size = (si91x_socket->local_address.sin6_family == AF_INET) ? SLI_DEFAULT_DATAGRAM_MSS_SIZE_IPV4
                                                                           : SLI_DEFAULT_DATAGRAM_MSS_SIZE_IPV6;
    return (int16_t)max_size;
  }
  return -1;
}

// This API is presently not exposed externally, and also not used internally in any of the APIs. This is retained in the code only as a quick reference for any future requests for this feature.
static sl_status_t __attribute__((unused))
sli_si91x_get_nwp_socket_info(sl_si91x_socket_info_response_t *socket_info_response)
{
  SL_WIFI_ARGS_CHECK_NULL_POINTER(socket_info_response);

  sl_status_t status                                  = SL_STATUS_FAIL;
  sl_wifi_buffer_t *buffer                            = NULL;
  const sli_si91x_network_params_response_t *response = NULL;

  status = sli_si91x_driver_send_command(SLI_WIFI_REQ_QUERY_NETWORK_PARAMS,
                                         SLI_WIFI_WLAN_CMD,
                                         NULL,
                                         0,
                                         SLI_WIFI_WAIT_FOR_RESPONSE(SLI_WIFI_RSP_QUERY_NETWORK_PARAMS_WAIT_TIME),
                                         NULL,
                                         &buffer);

  if ((status != SL_STATUS_OK) && (buffer != NULL)) {
    sli_si91x_host_free_buffer(buffer);
  }

  VERIFY_STATUS_AND_RETURN(status);

  sl_wifi_system_packet_t *packet = (sl_wifi_system_packet_t *)sli_wifi_host_get_buffer_data(buffer, 0, NULL);
  response                        = (sli_si91x_network_params_response_t *)packet->data;

  memcpy(&socket_info_response->number_of_opened_sockets,
         response->num_open_socks,
         sizeof(socket_info_response->number_of_opened_sockets));
  memcpy(socket_info_response->socket_info,
         response->socket_info,
         (sizeof(sl_si91x_sock_info_query_t) * socket_info_response->number_of_opened_sockets));

  sli_si91x_host_free_buffer(buffer);
  return SL_STATUS_OK;
}

sl_status_t sl_si91x_get_socket_info(sl_si91x_socket_info_response_t *socket_info_response)
{
  SL_WIFI_ARGS_CHECK_NULL_POINTER(socket_info_response);

  // Initialize the response structure
  memset(socket_info_response, 0, sizeof(sl_si91x_socket_info_response_t));

  uint16_t open_socket_count = 0;

  // Iterate through all sockets
  for (uint8_t socket_index = 0; socket_index < SL_SI91X_SOCKET_INFO_RESPONSE_SOCKETS_COUNT; socket_index++) {
    const sli_si91x_socket_t *socket = sli_get_si91x_socket(socket_index);

    // Skip if socket is not allocated or not in use
    if (socket == NULL || socket->state == RESET) {
      continue;
    }

    // Check if we have room in the response array
    if (open_socket_count >= SL_SI91X_SOCKET_INFO_RESPONSE_SOCKETS_COUNT) {
      break;
    }

    // Populate socket information
    sl_si91x_sock_info_query_t *socket_info = &socket_info_response->socket_info[open_socket_count];

    // Socket ID
    socket_info->sock_id[0] = (socket_index >> 8) & 0xFF;
    socket_info->sock_id[1] = socket_index & 0xFF;

    // Socket type
    socket_info->sock_type[0] = (socket->type >> 8) & 0xFF;
    socket_info->sock_type[1] = socket->type & 0xFF;

    // Source port (local port)
    socket_info->source_port[0] = (socket->local_address.sin6_port >> 8) & 0xFF;
    socket_info->source_port[1] = socket->local_address.sin6_port & 0xFF;

    // Destination port (remote port)
    socket_info->dest_port[0] = (socket->remote_address.sin6_port >> 8) & 0xFF;
    socket_info->dest_port[1] = socket->remote_address.sin6_port & 0xFF;

    // Destination IP address
    if (socket->local_address.sin6_family == AF_INET) {
      // IPv4 address
      memcpy(socket_info->dest_ip_address.ipv4_address,
             &((const struct sockaddr_in *)&socket->remote_address)->sin_addr.s_addr,
             SL_IPV4_ADDRESS_LENGTH);
    } else {
      // IPv6 address
#ifdef SLI_SI91X_NETWORK_DUAL_STACK
      memcpy(socket_info->dest_ip_address.ipv6_address,
             socket->remote_address.sin6_addr.un.u8_addr,
             SL_IPV6_ADDRESS_LENGTH);
#else
#ifndef __ZEPHYR__
      memcpy(socket_info->dest_ip_address.ipv6_address,
             socket->remote_address.sin6_addr.__u6_addr.__u6_addr8,
             SL_IPV6_ADDRESS_LENGTH);
#else
      memcpy(socket_info->dest_ip_address.ipv6_address,
             socket->remote_address.sin6_addr.s6_addr,
             SL_IPV6_ADDRESS_LENGTH);
#endif
#endif
    }

    open_socket_count++;
  }

  // Set the total number of opened sockets
  socket_info_response->number_of_opened_sockets = open_socket_count;

  return SL_STATUS_OK;
}

/**
 * A static utility function to get either peer address or socket address given the socket ID.
 * @param socket_id 			ID of socket.
 * @param name 					structure to hold the IP address, port, family type of IP address
 * @param name_len 				size of of buffer sent by caller.
 * @param required_address_type Whether peer data or local data to be copied to buffer.
 * @return						0 when successes, otherwise -1.
 */
static int sli_si91x_get_sock_address(int socket_id,
                                      struct sockaddr *name,
                                      socklen_t *name_len,
                                      uint8_t required_address_type)
{
  // Retrieve the SI91X socket associated with the given socket ID
  const sli_si91x_socket_t *si91x_socket = sli_get_si91x_socket(socket_id);
  // Determine the number of bytes to copy
  uint8_t bytes_to_be_copied = (*name_len > sizeof(struct sockaddr_in6)) ? sizeof(struct sockaddr_in6)
                                                                         : (uint8_t)*name_len;

  // Validate socket
  SLI_SET_ERRNO_AND_RETURN_IF_TRUE(si91x_socket == NULL, EBADF);
  SLI_SET_ERRNO_AND_RETURN_IF_TRUE(name == NULL || name_len == NULL, EFAULT);
  SLI_SET_ERRNO_AND_RETURN_IF_TRUE(*name_len <= 0, EINVAL);

  // Ensure the required conditions for copying the address are met
  SLI_SET_ERRNO_AND_RETURN_IF_TRUE(
    required_address_type == SLI_SI91X_BSD_SOCKET_PEER_ADDRESS && si91x_socket->state != CONNECTED,
    ENOTCONN);
  SLI_SET_ERRNO_AND_RETURN_IF_TRUE(required_address_type == SLI_SI91X_BSD_SOCKET_LOCAL_ADDRESS
                                     && (si91x_socket->state < BOUND || si91x_socket->state == DISCONNECTED),
                                   EBADF);

  // Copy the address based on the required type (local or peer).
  if (required_address_type == SLI_SI91X_BSD_SOCKET_LOCAL_ADDRESS) {
    memcpy(name, &si91x_socket->local_address, bytes_to_be_copied);
  } else {
    memcpy(name, &si91x_socket->remote_address, bytes_to_be_copied);
  }

  // Set the actual length of the copied address structure
  *name_len = si91x_socket->local_address.sin6_family == AF_INET ? sizeof(struct sockaddr_in)
                                                                 : sizeof(struct sockaddr_in6);
  return SLI_SI91X_NO_ERROR;
}

int socket(int family, int type, int protocol)
{
  return sli_si91x_socket(family, type, protocol, NULL);
}

int bind(int socket_id, const struct sockaddr *addr, socklen_t addr_len)
{
  return sli_si91x_bind(socket_id, addr, addr_len);
}

int listen(int socket_id, int backlog)
{
  errno = 0;

  sl_status_t status;
  // Retrieve the SI91X socket associated with the given socket ID.
  sli_si91x_socket_t *si91x_socket = sli_get_si91x_socket(socket_id);

  // Validate socket
  SLI_SET_ERRNO_AND_RETURN_IF_TRUE(si91x_socket == NULL, EBADF);
  SLI_SET_ERRNO_AND_RETURN_IF_TRUE(si91x_socket->state != INITIALIZED && si91x_socket->state != BOUND, EBADF);
  SLI_SET_ERRNO_AND_RETURN_IF_TRUE(si91x_socket->type != SOCK_STREAM, EOPNOTSUPP);

  status = sli_create_and_send_socket_request(socket_id, SLI_SI91X_SOCKET_TCP_SERVER, &backlog);

  SLI_SOCKET_VERIFY_STATUS_AND_RETURN(status, SL_STATUS_OK, SLI_SI91X_UNDEFINED_ERROR);

  si91x_socket->state = LISTEN;

  return SLI_SI91X_NO_ERROR;
}

int accept(int socket_id, struct sockaddr *addr, socklen_t *addr_len)
{
  return sli_si91x_accept(socket_id, addr, addr_len, NULL);
}

int connect(int socket_id, const struct sockaddr *addr, socklen_t addr_len)
{
  return sli_si91x_connect(socket_id, addr, addr_len);
}

ssize_t send(int socket_id, const void *data, size_t data_len, int flags)
{
  // Sending data to a socket without specifying a destination address
  return sendto(socket_id, data, data_len, flags, NULL, 0);
}

ssize_t recv(int socket_id, void *buf, size_t buf_len, int flags)
{
  // Receiving data from a socket without specifying the source address
  return recvfrom(socket_id, buf, buf_len, flags, NULL, NULL);
}

ssize_t sendto(int socket_id,
               const void *data,
               size_t data_len,
               int flags,
               const struct sockaddr *to_addr,
               socklen_t to_addr_len)
{
  // Initialize variables and error handling
  UNUSED_PARAMETER(flags);
  errno = 0;

  sl_status_t status                      = SL_STATUS_OK;
  sli_si91x_socket_t *si91x_socket        = sli_get_si91x_socket(socket_id);
  sli_si91x_socket_send_request_t request = { 0 };

  // Check for various error conditions
  SLI_SET_ERRNO_AND_RETURN_IF_TRUE(si91x_socket == NULL, EBADF);
  SLI_SET_ERRNO_AND_RETURN_IF_TRUE(si91x_socket->type == SOCK_STREAM && si91x_socket->state != CONNECTED, ENOTCONN);
  SLI_SET_ERRNO_AND_RETURN_IF_TRUE(!(si91x_socket->ssl_bitmap & SLI_SI91X_WEBSOCKET_FEAT) && data == NULL, EFAULT);

  SLI_SET_ERRNO_AND_RETURN_IF_TRUE(data_len > (size_t)sl_si91x_get_socket_mss(socket_id), EMSGSIZE);

  // // If it's a UDP socket in an unconnected state, establish a connection
  status = sli_si91x_udp_connect_if_unconnected(si91x_socket, to_addr, to_addr_len, socket_id);
  SLI_SET_ERRNO_AND_RETURN_IF_TRUE(status != SL_STATUS_OK, SLI_SI91X_UNDEFINED_ERROR);

  // Possible state of socket can be reset or disconnect only.
  SLI_SET_ERRNO_AND_RETURN_IF_TRUE(si91x_socket->type == SOCK_DGRAM && si91x_socket->state != CONNECTED
                                     && si91x_socket->state != UDP_UNCONNECTED_READY,
                                   EBADF);

  SLI_SET_ERRNO_AND_RETURN_IF_TRUE(
    si91x_socket->state == UDP_UNCONNECTED_READY
      && ((si91x_socket->local_address.sin6_family == AF_INET && to_addr_len < sizeof(struct sockaddr_in))
          || (si91x_socket->local_address.sin6_family == AF_INET6 && to_addr_len < sizeof(struct sockaddr_in6))),
    EINVAL);

  // create a socket send request
  if (si91x_socket->local_address.sin6_family == AF_INET6) {
    const struct sockaddr_in6 *socket_address = (const struct sockaddr_in6 *)to_addr;
    request.ip_version                        = SL_IPV6_VERSION;
    request.data_offset = (si91x_socket->type == SOCK_STREAM) ? SLI_TCP_V6_HEADER_LENGTH : SLI_UDP_V6_HEADER_LENGTH;
    const uint8_t *destination_ip =
      (si91x_socket->state == UDP_UNCONNECTED_READY || to_addr_len >= sizeof(struct sockaddr_in6))
#ifndef __ZEPHYR__
        ? socket_address->sin6_addr.__u6_addr.__u6_addr8
        : si91x_socket->remote_address.sin6_addr.__u6_addr.__u6_addr8;
#else
        ? socket_address->sin6_addr.s6_addr
        : si91x_socket->remote_address.sin6_addr.s6_addr;
#endif

    memcpy(request.dest_ip_addr.ipv6_address, destination_ip, SL_IPV6_ADDRESS_LENGTH);
  } else {
    const struct sockaddr_in *socket_address = (const struct sockaddr_in *)to_addr;
    request.ip_version                       = SL_IPV4_VERSION;
    request.data_offset = (si91x_socket->type == SOCK_STREAM) ? SLI_TCP_HEADER_LENGTH : SLI_UDP_HEADER_LENGTH;
    const uint32_t *destination_ip =
      (si91x_socket->state == UDP_UNCONNECTED_READY || to_addr_len >= sizeof(struct sockaddr_in))
        ? &socket_address->sin_addr.s_addr
        : &((struct sockaddr_in *)&si91x_socket->remote_address)->sin_addr.s_addr;

#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Warray-bounds"
#endif // __GNUC__
    memcpy(request.dest_ip_addr.ipv4_address, destination_ip, SL_IPV4_ADDRESS_LENGTH);
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif // __GNUC__
  }

  // Set other parameters in the request structure
  // Check for Websocket feature bit
  if (si91x_socket->ssl_bitmap & SLI_SI91X_WEBSOCKET_FEAT) {
    sli_si91x_set_websocket_offset(&request, data_len);
  }
  // Set the socket_id with both the socket ID and the opcode of WebSocket in one line
  request.socket_id = (uint16_t)((si91x_socket->id & 0x00FF) | ((uint16_t)(si91x_socket->opcode & 0xFF) << 8));

  request.dest_port = (si91x_socket->state == UDP_UNCONNECTED_READY || to_addr_len > 0)
                        ? ((const struct sockaddr_in *)to_addr)->sin_port
                        : si91x_socket->remote_address.sin6_port;
  request.length    = data_len;

  // Send the socket data request
  status = sli_si91x_send_socket_data(si91x_socket, &request, data);
  SLI_SOCKET_VERIFY_STATUS_AND_RETURN(status, SL_STATUS_OK, ENOBUFS);

  return data_len;
}

// Validate socket and input parameters, initialize UDP socket if necessary, and adjust buffer length
static sl_status_t sli_prepare_socket(sli_si91x_socket_t *si91x_socket, int socket_id, const void *buf, size_t *buf_len)
{
  // Validate input parameters
  SLI_SET_ERRNO_AND_RETURN_IF_TRUE(si91x_socket == NULL, EBADF);
  SLI_SET_ERRNO_AND_RETURN_IF_TRUE(si91x_socket->type == SOCK_STREAM && si91x_socket->state != CONNECTED, ENOTCONN);
  SLI_SET_ERRNO_AND_RETURN_IF_TRUE(buf == NULL, EFAULT);
  SLI_SET_ERRNO_AND_RETURN_IF_TRUE(*buf_len <= 0, EINVAL);

  // Initialize UDP socket if necessary
  if (si91x_socket->type == SOCK_DGRAM && (si91x_socket->state == BOUND || si91x_socket->state == INITIALIZED)) {
    sl_status_t bsd_status = sli_create_and_send_socket_request(socket_id, SLI_SI91X_SOCKET_UDP_CLIENT, NULL);
    SLI_SOCKET_VERIFY_STATUS_AND_RETURN(bsd_status, SL_STATUS_OK, SLI_SI91X_UNDEFINED_ERROR);
    si91x_socket->state = UDP_UNCONNECTED_READY;
  }

  // Adjust buffer length based on socket type
  if (si91x_socket->type == SOCK_STREAM) {
    if (*buf_len > SLI_DEFAULT_STREAM_MSS_SIZE_IPV4 || *buf_len > SLI_DEFAULT_STREAM_MSS_SIZE_IPV6) {
      *buf_len = (si91x_socket->local_address.sin6_family == AF_INET) ? SLI_DEFAULT_DATAGRAM_MSS_SIZE_IPV4
                                                                      : SLI_DEFAULT_DATAGRAM_MSS_SIZE_IPV6;
    }
  } else if (si91x_socket->type == SOCK_DGRAM
             && (*buf_len > SLI_DEFAULT_STREAM_MSS_SIZE_IPV4 || *buf_len > SLI_DEFAULT_STREAM_MSS_SIZE_IPV6)) {
    *buf_len = (si91x_socket->local_address.sin6_family == AF_INET) ? SLI_DEFAULT_DATAGRAM_MSS_SIZE_IPV4
                                                                    : SLI_DEFAULT_DATAGRAM_MSS_SIZE_IPV6;
  }

  return SL_STATUS_OK;
}

// Populate the source address structure and process the response
static void sli_populate_source_address(struct sockaddr *addr,
                                        socklen_t *addr_len,
                                        const sl_si91x_socket_metadata_t *response,
                                        void *buf,
                                        size_t buf_len,
                                        ssize_t *bytes_read)
{
  // Process the response
  *bytes_read = (ssize_t)((response->length <= buf_len) ? response->length : buf_len);
  memcpy(buf, ((const uint8_t *)response + response->offset), (size_t)*bytes_read);

  if (addr != NULL) {
    if (response->ip_version == SL_IPV4_VERSION && *addr_len >= sizeof(struct sockaddr_in)) {
      struct sockaddr_in *socket_address = (struct sockaddr_in *)addr;

      socket_address->sin_port   = response->dest_port;
      socket_address->sin_family = AF_INET;
      memcpy(&socket_address->sin_addr.s_addr, response->dest_ip_addr.ipv4_address, SL_IPV4_ADDRESS_LENGTH);

      *addr_len = sizeof(struct sockaddr_in);
    } else if (response->ip_version == SL_IPV6_VERSION && *addr_len >= sizeof(struct sockaddr_in6)) {
      struct sockaddr_in6 *ipv6_socket_address = (struct sockaddr_in6 *)addr;

      ipv6_socket_address->sin6_port   = response->dest_port;
      ipv6_socket_address->sin6_family = AF_INET6;
#ifndef __ZEPHYR__
      memcpy(&ipv6_socket_address->sin6_addr.__u6_addr.__u6_addr8,
             response->dest_ip_addr.ipv6_address,
             SL_IPV6_ADDRESS_LENGTH);
#else
      memcpy(&ipv6_socket_address->sin6_addr.s6_addr, response->dest_ip_addr.ipv6_address, SL_IPV6_ADDRESS_LENGTH);
#endif

      *addr_len = sizeof(struct sockaddr_in6);
    } else {
      *addr_len = 0; // Not BSD compliant
    }
  }
}

ssize_t recvfrom(int socket_id, void *buf, size_t buf_len, int flags, struct sockaddr *addr, socklen_t *addr_len)
{
  UNUSED_PARAMETER(flags); // Ignoring the 'flags' parameter
  sli_wifi_wait_period_t wait_time = 0;
  errno                            = 0;

  sli_si91x_req_socket_read_t request        = { 0 }; // Initialize a request structure
  sl_status_t status                         = SL_STATUS_OK;
  ssize_t bytes_read                         = 0; // Number of bytes read
  sl_wifi_system_packet_t *packet            = NULL;
  const sl_si91x_socket_metadata_t *response = NULL;                            // Response structure
  sli_si91x_socket_t *si91x_socket           = sli_get_si91x_socket(socket_id); // Get socket information

  sl_wifi_buffer_t *buffer = NULL;

  // Validate input parameters, initialize UDP socket if necessary, and adjust buffer length
  status = sli_prepare_socket(si91x_socket, socket_id, buf, &buf_len);
  if (status != SL_STATUS_OK) {
    return -1;
  }

  // Ensure the socket is in the correct state
  SLI_SET_ERRNO_AND_RETURN_IF_TRUE(si91x_socket->state != CONNECTED && si91x_socket->state != UDP_UNCONNECTED_READY,
                                   EBADF);

  // Prepare the request
  request.socket_id = (uint8_t)si91x_socket->id;
  memcpy(request.requested_bytes, &buf_len, sizeof(buf_len));
  memcpy(request.read_timeout, &si91x_socket->read_timeout, sizeof(request.read_timeout));

  // Configure wait time and send the command
  wait_time = (SLI_WIFI_WAIT_FOR_EVER | SLI_WIFI_WAIT_FOR_RESPONSE_BIT);
  status    = sli_si91x_send_socket_command(si91x_socket,
                                         SLI_WLAN_REQ_SOCKET_READ_DATA,
                                         &request,
                                         sizeof(request),
                                         wait_time,
                                         &buffer);
  if (status == SL_STATUS_SI91X_SOCKET_CLOSED) {
    sli_si91x_host_free_buffer(buffer);
    errno = ENOTCONN;
    return -1;
  }

  if ((status != SL_STATUS_OK) && (buffer != NULL)) {
    sli_si91x_host_free_buffer(buffer);
  }
  SLI_SOCKET_VERIFY_STATUS_AND_RETURN(status, SL_STATUS_OK, SLI_SI91X_UNDEFINED_ERROR);

  // Process the response
  packet   = (sl_wifi_system_packet_t *)sli_wifi_host_get_buffer_data(buffer, 0, NULL);
  response = (sl_si91x_socket_metadata_t *)packet->data;
  sli_populate_source_address(addr, addr_len, response, buf, buf_len, &bytes_read);

  // Free the buffer
  sli_si91x_host_free_buffer(buffer);

  return bytes_read;
}

int getsockname(int socket_id, struct sockaddr *name, socklen_t *name_len)
{
  return sli_si91x_get_sock_address(socket_id, name, name_len, SLI_SI91X_BSD_SOCKET_LOCAL_ADDRESS);
}

int getpeername(int socket_id, struct sockaddr *name, socklen_t *name_len)
{
  return sli_si91x_get_sock_address(socket_id, name, name_len, SLI_SI91X_BSD_SOCKET_PEER_ADDRESS);
}

// Validate input parameters
static int sli_validate_setsockopt_params(const sli_si91x_socket_t *si91x_socket,
                                          const void *option_value,
                                          int option_level)
{
  // Check if the socket is valid
  SLI_SET_ERRNO_AND_RETURN_IF_TRUE(si91x_socket == NULL, EBADF);

  // Check if the option value is valid
  SLI_SET_ERRNO_AND_RETURN_IF_TRUE(option_value == NULL, EFAULT);

  // Check if the option_level is valid
  SLI_SET_ERRNO_AND_RETURN_IF_TRUE(
    ((option_level != SOL_SOCKET) && (option_level != SOL_TCP) && (option_level != IPPROTO_IP)),
    EINVAL);

  return SLI_SI91X_NO_ERROR;
}

static int sli_handle_so_rcvtimeo(sli_si91x_socket_t *si91x_socket, const void *option_value, socklen_t option_length)
{
  sl_si91x_time_value *timeout = NULL;
  timeout                      = (sl_si91x_time_value *)option_value;
  uint16_t timeout_val;

  // Configure receive timeout
  if ((timeout->tv_sec == 0) && (timeout->tv_usec != 0) && (timeout->tv_usec < 1000)) {
    timeout->tv_usec = 1000;
  }
  timeout_val = (uint16_t)((timeout->tv_usec / 1000) + (timeout->tv_sec * 1000));

  // Need to add check here if Synchronous bit map is set (after async socket_id implementation)
  memcpy(&si91x_socket->read_timeout,
         &timeout_val,
         SLI_GET_SAFE_MEMCPY_LENGTH(sizeof(si91x_socket->read_timeout), option_length));
  return SLI_SI91X_NO_ERROR;
}

static int sli_handle_so_keepalive(sli_si91x_socket_t *si91x_socket, const void *option_value, socklen_t option_length)
{
  // Set TCP keep-alive initial time
  memcpy(&si91x_socket->tcp_keepalive_initial_time,
         (const uint16_t *)option_value,
         SLI_GET_SAFE_MEMCPY_LENGTH(sizeof(si91x_socket->tcp_keepalive_initial_time), option_length));
  return SLI_SI91X_NO_ERROR;
}

static int sli_handle_tcp_ulp(sli_si91x_socket_t *si91x_socket, const void *option_value, socklen_t option_length)
{
  // Set TLS version based on the provided option_value
  if ((sizeof(TLS_1_2) != option_length) && (sizeof(TLS) != option_length)) {
    SLI_SET_ERROR_AND_RETURN(EINVAL);
  }
  if (strncmp(option_value, TLS, option_length) == 0) {
    si91x_socket->ssl_bitmap = SL_SI91X_ENABLE_TLS;
  } else if (strncmp(option_value, TLS_1_2, option_length) == 0) {
    si91x_socket->ssl_bitmap = SL_SI91X_ENABLE_TLS | SL_SI91X_TLS_V_1_2;
  } else if (strncmp(option_value, TLS_1_1, option_length) == 0) {
    si91x_socket->ssl_bitmap = SL_SI91X_ENABLE_TLS | SL_SI91X_TLS_V_1_1;
  } else if (strncmp(option_value, TLS_1_0, option_length) == 0) {
    si91x_socket->ssl_bitmap = SL_SI91X_ENABLE_TLS | SL_SI91X_TLS_V_1_0;
  }
#if defined(SLI_SI917)
  else if (strncmp(option_value, TLS_1_3, option_length) == 0) {
    si91x_socket->ssl_bitmap = SL_SI91X_ENABLE_TLS | SL_SI91X_TLS_V_1_3;
  }
#endif
  return SLI_SI91X_NO_ERROR;
}

static int sli_handle_so_max_retransmission_timeout_value(sli_si91x_socket_t *si91x_socket,
                                                          const void *option_value,
                                                          socklen_t option_length)
{
  // Set max retransmission timeout value with bounds check
  if (IS_POWER_OF_TWO(*(const uint8_t *)option_value)
      && ((*(const uint8_t *)option_value) < SLI_MAX_RETRANSMISSION_TIME_VALUE)) {
    memcpy(&si91x_socket->max_retransmission_timeout_value,
           (const uint8_t *)option_value,
           SLI_GET_SAFE_MEMCPY_LENGTH(sizeof(si91x_socket->max_retransmission_timeout_value), option_length));
  } else {
    SL_DEBUG_LOG("\n Max retransmission timeout value in between 1 - 128 and "
                 "should be power of two. ex:1,2,4,8,16,32,64,128 \n");
    SLI_SET_ERROR_AND_RETURN(EINVAL);
  }
  return SLI_SI91X_NO_ERROR;
}

static int sli_handle_ip_tos(sli_si91x_socket_t *si91x_socket, const void *option_value)
{
  int32_t converted_tos_value = sli_map_tos_to_nwp(option_value);
  if (converted_tos_value < 0) {
    SLI_SET_ERROR_AND_RETURN(EINVAL);
  }

  si91x_socket->tos = (uint32_t)converted_tos_value;
  return SLI_SI91X_NO_ERROR;
}

static int sli_handle_sl_so_cert_index(sli_si91x_socket_t *si91x_socket, const void *option_value)
{
  // Check if the provided certificate index is within a valid range
  SLI_SET_ERRNO_AND_RETURN_IF_TRUE(((*(const uint8_t *)option_value < SLI_SI91X_CERT_INDEX_0)
                                    || (*(const uint8_t *)option_value > SLI_SI91X_CERT_INDEX_2)),
                                   EINVAL);

  // Set the certificate index
  si91x_socket->certificate_index = *(const uint8_t *)option_value;
  return SLI_SI91X_NO_ERROR;
}

static int sli_handle_sl_so_high_performance_socket(sli_si91x_socket_t *si91x_socket, const void *option_value)
{
  // Check if the provided value is SLI_SI91X_HIGH_PERFORMANCE_SOCKET
  SLI_SET_ERRNO_AND_RETURN_IF_TRUE(*(const uint8_t *)option_value != SLI_SI91X_HIGH_PERFORMANCE_SOCKET, EINVAL);

  // Set the SLI_SI91X_HIGH_PERFORMANCE_SOCKET flag in the ssl_bitmap of si91x_socket
  si91x_socket->ssl_bitmap |= SLI_SI91X_HIGH_PERFORMANCE_SOCKET;
  return SLI_SI91X_NO_ERROR;
}

static int sli_handle_sl_so_tls_sni_alpn(sli_si91x_socket_t *si91x_socket, const void *option_value)
{
  // Call a function to add a TLS extension to si91x_socket
  sl_status_t status = sli_si91x_add_tls_extension(&si91x_socket->tls_extensions,
                                                   (const sl_si91x_socket_type_length_value_t *)option_value);
  // Check if the operation was successful
  if (status != SL_STATUS_OK) {
    SLI_SET_ERROR_AND_RETURN(ENOMEM);
  }
  return SLI_SI91X_NO_ERROR;
}

static int sli_handle_sl_so_mss(sli_si91x_socket_t *si91x_socket, const void *option_value, socklen_t option_length)
{
  // Set the Maximum Segment Size (MSS) for the socket
  if (option_length == sizeof(uint16_t)) {
    const uint16_t mss = *(const uint16_t *)option_value;
    si91x_socket->mss  = mss;
  } else {
    errno = EINVAL;
    return -1;
  }
  return SLI_SI91X_NO_ERROR;
}

static int sli_handle_sl_so_sock_vap_id(sli_si91x_socket_t *si91x_socket,
                                        const void *option_value,
                                        socklen_t option_length)
{
  // Set the VAP ID for the socket
  if (option_length == sizeof(uint8_t)) {
    const uint8_t vap_id = *(const uint8_t *)option_value;
    si91x_socket->vap_id = vap_id;
  } else {
    errno = EINVAL;
    return -1;
  }
  return SLI_SI91X_NO_ERROR;
}

static int sli_handle_sl_so_maxretry(sli_si91x_socket_t *si91x_socket,
                                     const void *option_value,
                                     socklen_t option_length)
{
  // Set the maximum number of retries for the socket
  if (option_length == sizeof(uint8_t)) {
    const uint8_t max_tcp_retries = *(const uint8_t *)option_value;
    si91x_socket->max_tcp_retries = max_tcp_retries;
  } else {
    errno = EINVAL;
    return -1;
  }
  return SLI_SI91X_NO_ERROR;
}

static int sli_handle_sl_so_verify_domain_name(sli_si91x_socket_t *si91x_socket,
                                               const void *option_value,
                                               socklen_t option_length)
{
  // Set the domain name for verification
  if ((option_length + 1) > SLI_SI91X_MAX_DOMAIN_NAME_LENGTH) {
    errno = EINVAL;
    return -1;
  }

  // Free any existing domain name
  if (si91x_socket->domain_name != NULL) {
    free(si91x_socket->domain_name);
    si91x_socket->domain_name = NULL;
  }

  // Allocate memory for the new domain name
  si91x_socket->domain_name = (uint8_t *)malloc(option_length + 1);
  if (si91x_socket->domain_name == NULL) {
    errno = ENOMEM;
    return -1;
  }

  // Copy the domain name and null-terminate
  memcpy(si91x_socket->domain_name, option_value, option_length);
  si91x_socket->domain_name[option_length] = '\0';

  return SLI_SI91X_NO_ERROR;
}

int setsockopt(int socket_id, int option_level, int option_name, const void *option_value, socklen_t option_length)
{
  sli_si91x_socket_t *si91x_socket = sli_get_si91x_socket(socket_id);
  int status                       = sli_validate_setsockopt_params(si91x_socket, option_value, option_level);
  if (status != SLI_SI91X_NO_ERROR) {
    return status;
  }

  switch (option_name) {
    case SO_RCVTIMEO:
      return sli_handle_so_rcvtimeo(si91x_socket, option_value, option_length);

    case SO_KEEPALIVE:
      return sli_handle_so_keepalive(si91x_socket, option_value, option_length);

    case TCP_ULP:
      return sli_handle_tcp_ulp(si91x_socket, option_value, option_length);

#if defined(SLI_SI917)
    case SO_MAX_RETRANSMISSION_TIMEOUT_VALUE:
      return sli_handle_so_max_retransmission_timeout_value(si91x_socket, option_value, option_length);

    case IP_TOS:
      return sli_handle_ip_tos(si91x_socket, option_value);
#endif

    case SL_SO_CERT_INDEX:
      return sli_handle_sl_so_cert_index(si91x_socket, option_value);

    case SL_SO_HIGH_PERFORMANCE_SOCKET:
      return sli_handle_sl_so_high_performance_socket(si91x_socket, option_value);

    case SL_SO_TLS_SNI:
    case SL_SO_TLS_ALPN:
      return sli_handle_sl_so_tls_sni_alpn(si91x_socket, option_value);

    case SL_SO_MSS:
      return sli_handle_sl_so_mss(si91x_socket, option_value, option_length);

    case SL_SO_SOCK_VAP_ID:
      return sli_handle_sl_so_sock_vap_id(si91x_socket, option_value, option_length);

    case SL_SO_MAXRETRY:
      return sli_handle_sl_so_maxretry(si91x_socket, option_value, option_length);

    case SL_SO_VERIFY_DOMAIN_NAME:
      return sli_handle_sl_so_verify_domain_name(si91x_socket, option_value, option_length);

    default: {
      // Unsupported option
      SLI_SET_ERROR_AND_RETURN(ENOPROTOOPT);
    }
  }

  return SLI_SI91X_NO_ERROR;
}

int getsockopt(int socket_id, int option_level, int option_name, void *option_value, socklen_t *option_length)
{
  int16_t mss                            = 0;
  const sli_si91x_socket_t *si91x_socket = sli_get_si91x_socket(socket_id);

  // Check if the socket is valid
  SLI_SET_ERRNO_AND_RETURN_IF_TRUE(si91x_socket == NULL, EBADF);
  // Check if option_value is valid
  SLI_SET_ERRNO_AND_RETURN_IF_TRUE(option_value == NULL, EFAULT);
  // Check if option_level is SOL_SOCKET
  SLI_SET_ERRNO_AND_RETURN_IF_TRUE((option_level != SOL_SOCKET && option_level != SOL_TCP) || option_length == NULL,
                                   EINVAL);

  // Determine the requested socket option and handle it accordingly
  switch (option_name) {
    case SO_SNDBUF: {
      mss            = sl_si91x_get_socket_mss(socket_id);
      *option_length = SLI_GET_SAFE_MEMCPY_LENGTH(*option_length, sizeof(mss));
      memcpy(option_value, &mss, *option_length);
      break;
    }

    case SO_RCVTIMEO: {
      // Get receive timeout
      *option_length = SLI_GET_SAFE_MEMCPY_LENGTH(*option_length, sizeof(si91x_socket->read_timeout));
      memcpy(option_value, &si91x_socket->read_timeout, *option_length);
      break;
    }

    case SO_KEEPALIVE: {
      // Get TCP keep-alive initial time
      *option_length = SLI_GET_SAFE_MEMCPY_LENGTH(*option_length, sizeof(si91x_socket->tcp_keepalive_initial_time));
      memcpy(option_value, &si91x_socket->tcp_keepalive_initial_time, *option_length);
      break;
    }

    case SO_TYPE: {
      // Retrieve and copy the socket type
      *option_length = SLI_GET_SAFE_MEMCPY_LENGTH(*option_length, sizeof(si91x_socket->type));
      memcpy(option_value, &si91x_socket->type, *option_length);
      break;
    }

    case SO_DOMAIN: {
      // Retrieve and copy the socket domain
      *option_length = SLI_GET_SAFE_MEMCPY_LENGTH(*option_length, sizeof(si91x_socket->local_address.sin6_family));
      memcpy(option_value, &si91x_socket->local_address.sin6_family, *option_length);
      break;
    }

    case SO_PROTOCOL: {
      // Retrieve and copy the socket protocol
      *option_length = SLI_GET_SAFE_MEMCPY_LENGTH(*option_length, sizeof(si91x_socket->protocol));
      memcpy(option_value, &si91x_socket->protocol, *option_length);
      break;
    }

    case SO_ERROR: {
      // Retrieve and copy the socket error (errno) and reset errno to 0
      *option_length = SLI_GET_SAFE_MEMCPY_LENGTH(*option_length, sizeof(errno));
      memcpy(option_value, &errno, *option_length);
      errno = 0;
      break;
    }

    case TCP_ULP: {
      // Retrieve and copy the socket ssl_bitmap
      *option_length = SLI_GET_SAFE_MEMCPY_LENGTH(*option_length, sizeof(si91x_socket->ssl_bitmap));
      memcpy(option_value, &si91x_socket->ssl_bitmap, *option_length);
      break;
    }

    case SL_SO_CERT_INDEX: {
      // Retrieve and copy the socket certificate index
      *option_length = SLI_GET_SAFE_MEMCPY_LENGTH(*option_length, sizeof(si91x_socket->certificate_index));
      memcpy(option_value, &si91x_socket->certificate_index, *option_length);
      break;
    }

    default: {
      SLI_SET_ERROR_AND_RETURN(ENOPROTOOPT);
    }
  }

  return SLI_SI91X_NO_ERROR;
}

int close(int socket_id)
{
  errno = 0; // Clear any existing error
  return sli_si91x_shutdown(socket_id, SHUTDOWN_BY_ID);
}

struct hostent *gethostbyname(const char *name)
{
  sl_status_t status = SL_STATUS_OK;

  if ((name == NULL)
      || (sl_strnlen((char *)(name), SLI_SI91X_DNS_REQUEST_MAX_URL_LEN + 1) > SLI_SI91X_DNS_REQUEST_MAX_URL_LEN)) {
    herrno = TRY_AGAIN;
    return NULL;
  }
// Retrieve host information based on address type
#ifdef SLI_SI91X_ENABLE_IPV6
  status = sl_net_dns_resolve_hostname(name, SLI_WIFI_WAIT_FOR_DNS_RESOLUTION, SL_NET_DNS_TYPE_IPV6, &host_ip_address);
#else
  status = sl_net_dns_resolve_hostname(name, SLI_WIFI_WAIT_FOR_DNS_RESOLUTION, SL_NET_DNS_TYPE_IPV4, &host_ip_address);
#endif

  // Handle the DNS resolution result
  if (status == SL_STATUS_TIMEOUT) {
    herrno = TRY_AGAIN;
    return NULL;
  } else if (status != SL_STATUS_OK) {
    herrno = HOST_NOT_FOUND;
    return NULL;
  }

// Set host information
#ifdef SLI_SI91X_ENABLE_IPV6
  host_ent.h_length   = (host_ip_address.type == SL_IPV4 ? SL_IPV4_ADDRESS_LENGTH : SL_IPV6_ADDRESS_LENGTH);
  host_ent.h_addrtype = (host_ip_address.type == SL_IPV4 ? AF_INET : AF_INET6);
#else
  host_ent.h_length = SL_IPV4_ADDRESS_LENGTH;
  host_ent.h_addrtype = AF_INET;
#endif

  host_ent.h_name = (char *)name;
  host_ent.h_addr = (char *)&host_ip_address;

  herrno = NETDB_SUCCESS;
  return &host_ent;
}

struct hostent *gethostbyname2(const char *name, int af)
{
  sl_status_t status = SL_STATUS_OK;

  // Validate the 'name' argument
  if ((name == NULL)
      || (sl_strnlen((char *)(name), SLI_SI91X_DNS_REQUEST_MAX_URL_LEN + 1) > SLI_SI91X_DNS_REQUEST_MAX_URL_LEN)) {
    herrno = TRY_AGAIN;
    return NULL;
  }

  // Retrieve host information based on address type
  if (af == AF_INET6) {
    status =
      sl_net_dns_resolve_hostname(name, SLI_WIFI_WAIT_FOR_DNS_RESOLUTION, SL_NET_DNS_TYPE_IPV6, &host_ip_address);
  } else {
    status =
      sl_net_dns_resolve_hostname(name, SLI_WIFI_WAIT_FOR_DNS_RESOLUTION, SL_NET_DNS_TYPE_IPV4, &host_ip_address);
  }

  // Handle the DNS resolution result
  if (status == SL_STATUS_TIMEOUT) {
    herrno = TRY_AGAIN;
    return NULL;
  } else if (status != SL_STATUS_OK) {
    herrno = HOST_NOT_FOUND;
    return NULL;
  }

  // Set host information based on the address family
  if (af == AF_INET6) {
    host_ent.h_length   = SL_IPV6_ADDRESS_LENGTH;
    host_ent.h_addrtype = AF_INET6;
  } else {
    host_ent.h_length   = SL_IPV4_ADDRESS_LENGTH;
    host_ent.h_addrtype = AF_INET;
  }

  host_ent.h_name = (char *)name;
  host_ent.h_addr = (char *)&host_ip_address;

  herrno = NETDB_SUCCESS; // Set success status
  return &host_ent;
}

#ifndef __ZEPHYR__
int select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, const struct timeval *timeout)
#else
int select(int nfds,
           sl_si91x_fdset_t *readfds,
           sl_si91x_fdset_t *writefds,
           sl_si91x_fdset_t *exceptfds,
           const struct timeval *timeout)
#endif
{
  return sli_si91x_select(nfds, readfds, writefds, exceptfds, timeout, NULL);
}
