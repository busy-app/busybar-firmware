/***************************************************************************/ /**
 * @file  sl_si91x_socket.c
 *******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
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
#include "sl_status.h"
#include "sl_constants.h"
#include "sl_si91x_socket.h"
#include "sl_si91x_socket_callback_framework.h"
#include "sl_si91x_socket_types.h"
#include "sl_si91x_socket_constants.h"
#include "sl_si91x_protocol_types.h"
#include "sl_rsi_utility.h"
#include "sl_si91x_driver.h"
#include <stdint.h>
#include <string.h>
#include "sli_wifi_utility.h"
/******************************************************
 *                      Macros
 ******************************************************/
#define SLI_TCP_HEADER_LENGTH          56
#define SLI_UDP_HEADER_LENGTH          44
#define SLI_TCP_V6_HEADER_LENGTH       76
#define SLI_UDP_V6_HEADER_LENGTH       64
#define SLI_SI91X_SSL_HEADER_SIZE_IPV4 90
#define SLI_SI91X_SSL_HEADER_SIZE_IPV6 110

void sl_si91x_set_remote_termination_callback(sl_si91x_socket_remote_termination_callback_t callback)
{
  sli_si91x_set_remote_socket_termination_callback(callback);
}

// Create a new socket
int sl_si91x_socket(int family, int type, int protocol)
{
  return sli_si91x_socket(family, type, protocol, NULL);
}

int sl_si91x_socket_async(int family, int type, int protocol, sl_si91x_socket_receive_data_callback_t callback)
{
  SLI_SET_ERRNO_AND_RETURN_IF_TRUE(NULL == callback, 0);

  return sli_si91x_socket(family, type, protocol, callback);
}

int sl_si91x_bind(int socket, const struct sockaddr *addr, socklen_t addr_len)
{
  return sli_si91x_bind(socket, addr, addr_len);
}

int sl_si91x_connect(int socket, const struct sockaddr *addr, socklen_t addr_len)
{
  return sli_si91x_connect(socket, addr, addr_len);
}

int sl_si91x_listen(int socket, int max_number_of_clients)
{
  int32_t status;
  sli_si91x_socket_t *si91x_socket = sli_get_si91x_socket(socket);

  // Check if the socket is valid
  SLI_SET_ERRNO_AND_RETURN_IF_TRUE(si91x_socket == NULL, EBADF);
  SLI_SET_ERRNO_AND_RETURN_IF_TRUE(si91x_socket->state != INITIALIZED && si91x_socket->state != BOUND, EBADF);
  SLI_SET_ERRNO_AND_RETURN_IF_TRUE(si91x_socket->type != SOCK_STREAM, EOPNOTSUPP);

  // Create and send a socket request to make it a TCP server with the specified maximum number of clients
  status = sli_create_and_send_socket_request(socket, SLI_SI91X_SOCKET_TCP_SERVER, &max_number_of_clients);
  SLI_SOCKET_VERIFY_STATUS_AND_RETURN(status, SLI_SI91X_NO_ERROR, SLI_SI91X_UNDEFINED_ERROR);

  si91x_socket->state = LISTEN;

  return SLI_SI91X_NO_ERROR;
}

int sl_si91x_accept(int socket, const struct sockaddr *addr, socklen_t addr_len)
{
  return sli_si91x_accept(socket, (struct sockaddr *)addr, &addr_len, NULL);
}

int sl_si91x_shutdown(int socket, int how)
{
  return sli_si91x_shutdown(socket, how);
}

int sl_si91x_accept_async(int socket, sl_si91x_socket_accept_callback_t callback)
{
  return sli_si91x_accept(socket, NULL, 0, callback);
}

int sl_si91x_setsockopt(int32_t sockID, int level, int option_name, const void *option_value, socklen_t option_len)
{
  UNUSED_PARAMETER(level);

  // Retrieve the socket using the socket index
  sli_si91x_socket_t *si91x_socket = sli_get_si91x_socket(sockID);
  uint16_t timeout_val;

  // Check if the socket is valid
  SLI_SET_ERRNO_AND_RETURN_IF_TRUE(si91x_socket == NULL, EBADF);

  // Check if the option value is not NULL
  SLI_SET_ERRNO_AND_RETURN_IF_TRUE(option_value == NULL, EFAULT);

  switch (option_name) {
    case SL_SI91X_SO_RCVTIME: {
      // Configure receive timeout
      const sl_si91x_time_value *timeout_const = (const sl_si91x_time_value *)option_value;
      sl_si91x_time_value timeout              = *timeout_const;

      // Ensure that the timeout value is at least 1 millisecond
      if ((timeout.tv_sec == 0) && (timeout.tv_usec != 0) && (timeout.tv_usec < 1000)) {
        timeout.tv_usec = 1000;
      }
      // Calculate the timeout value in milliseconds
      timeout_val = (uint16_t)((timeout.tv_usec / 1000) + (timeout.tv_sec * 1000));

      // Need to add check here if Synchronous bit map is set (after async socket_id implementation)
      memcpy(&si91x_socket->read_timeout,
             &timeout_val,
             SLI_GET_SAFE_MEMCPY_LENGTH(sizeof(si91x_socket->read_timeout), option_len));
      break;
    }

    case SL_SI91X_SO_MAXRETRY: {
      // Set the maximum number of TCP retries
      memcpy(&si91x_socket->max_tcp_retries,
             (const uint16_t *)option_value,
             SLI_GET_SAFE_MEMCPY_LENGTH(sizeof(si91x_socket->max_tcp_retries), option_len));
      break;
    }

    case SL_SI91X_SO_MSS: {
      memcpy(&si91x_socket->mss,
             (const uint16_t *)option_value,
             SLI_GET_SAFE_MEMCPY_LENGTH(sizeof(si91x_socket->mss), option_len));
      break;
    }

    case SL_SI91X_SO_TCP_KEEPALIVE: {
      // Set the TCP keep-alive initial time
      memcpy(&si91x_socket->tcp_keepalive_initial_time,
             (const uint16_t *)option_value,
             SLI_GET_SAFE_MEMCPY_LENGTH(sizeof(si91x_socket->tcp_keepalive_initial_time), option_len));
      break;
    }
    case SL_SI91X_SO_HIGH_PERFORMANCE_SOCKET: {
      // Enable high-performance socket mode
      SLI_SET_ERRNO_AND_RETURN_IF_TRUE(*(uint32_t *)option_value != SLI_SI91X_HIGH_PERFORMANCE_SOCKET, EINVAL);
      si91x_socket->ssl_bitmap |= SLI_SI91X_HIGH_PERFORMANCE_SOCKET;
      break;
    }

    case SL_SI91X_SO_SSL_ENABLE: {
      // Enable SSL for the socket
      SLI_SET_ERRNO_AND_RETURN_IF_TRUE((*(uint32_t *)option_value) != SL_SI91X_ENABLE_TLS, EINVAL);
      si91x_socket->ssl_bitmap |= SL_SI91X_ENABLE_TLS;
      break;
    }
    case SL_SI91X_SO_SSL_V_1_0_ENABLE: {
      // Enable SSL version 1.0 for the socket
      SLI_SET_ERRNO_AND_RETURN_IF_TRUE(((*(uint32_t *)option_value) != (SL_SI91X_ENABLE_TLS | SL_SI91X_TLS_V_1_0)),
                                       EINVAL);
      si91x_socket->ssl_bitmap |= SL_SI91X_ENABLE_TLS | SL_SI91X_TLS_V_1_0;
      break;
    }
    case SL_SI91X_SO_SSL_V_1_1_ENABLE: {
      // Enable SSL version 1.1 for the socket
      SLI_SET_ERRNO_AND_RETURN_IF_TRUE(((*(uint32_t *)option_value) != (SL_SI91X_ENABLE_TLS | SL_SI91X_TLS_V_1_1)),
                                       EINVAL);
      si91x_socket->ssl_bitmap |= SL_SI91X_ENABLE_TLS | SL_SI91X_TLS_V_1_1;
      break;
    }
    case SL_SI91X_SO_SSL_V_1_2_ENABLE: {
      // Enable SSL version 1.2 for the socket
      SLI_SET_ERRNO_AND_RETURN_IF_TRUE(((*(uint32_t *)option_value) != (SL_SI91X_ENABLE_TLS | SL_SI91X_TLS_V_1_2)),
                                       EINVAL);
      si91x_socket->ssl_bitmap |= SL_SI91X_ENABLE_TLS | SL_SI91X_TLS_V_1_2;
      break;
    }

    case SL_SI91X_SO_SOCK_VAP_ID: {
      // Set the VAP ID for the socket
      si91x_socket->vap_id = *((const uint8_t *)option_value);
      break;
    }

    case SL_SI91x_SO_TCP_ACK_INDICATION: {
      // Enable TCP_ACK_INDICATION
      SLI_SET_ERRNO_AND_RETURN_IF_TRUE((*(uint8_t *)option_value) != SLI_SI91X_SOCKET_FEAT_TCP_ACK_INDICATION, EINVAL);
      si91x_socket->socket_bitmap |= SLI_SI91X_SOCKET_FEAT_TCP_ACK_INDICATION;
      break;
    }
#if defined(SLI_SI917)
    case SL_SI91X_SO_SSL_V_1_3_ENABLE: {
      // Enable SSL version 1.3 for the socket.
      SLI_SET_ERRNO_AND_RETURN_IF_TRUE(((*(uint32_t *)option_value) != (SL_SI91X_ENABLE_TLS | SL_SI91X_TLS_V_1_3)),
                                       EINVAL);
      si91x_socket->ssl_bitmap |= SL_SI91X_ENABLE_TLS | SL_SI91X_TLS_V_1_3;
      break;
    }
#endif

    case SL_SI91X_SO_CERT_INDEX: {
      SLI_SET_ERRNO_AND_RETURN_IF_TRUE(
        ((*(uint8_t *)option_value < SLI_SI91X_CERT_INDEX_0) || (*(uint8_t *)option_value > SLI_SI91X_CERT_INDEX_2)),
        EINVAL);

      si91x_socket->certificate_index = *(const uint8_t *)option_value;
      break;
    }

    case SL_SI91X_SO_TLS_SNI:
    case SL_SI91X_SO_TLS_ALPN: {
      sl_status_t status = sli_si91x_add_tls_extension(&si91x_socket->tls_extensions,
                                                       (const sl_si91x_socket_type_length_value_t *)option_value);

      if (status != SL_STATUS_OK) {
        SLI_SET_ERROR_AND_RETURN(ENOMEM);
      }
      break;
    }

#if defined(SLI_SI917)
    case SL_SI91X_SO_MAX_RETRANSMISSION_TIMEOUT_VALUE: {
      if (IS_POWER_OF_TWO((*(uint32_t *)option_value))
          && ((*(const uint32_t *)option_value) < SLI_MAX_RETRANSMISSION_TIME_VALUE)) {
        memcpy(&si91x_socket->max_retransmission_timeout_value,
               (const uint32_t *)option_value,
               SLI_GET_SAFE_MEMCPY_LENGTH(sizeof(si91x_socket->max_retransmission_timeout_value), option_len));
      } else {
        SL_DEBUG_LOG("\n Max retransmission timeout value in between 1 - 128 and "
                     "should be power of two. ex:1,2,4,8,16,32,64,128 \n");
        SLI_SET_ERROR_AND_RETURN(EINVAL);
      }
      break;
    }
#endif

    case SL_SI91X_SO_DTLS_ENABLE: {
      // Enable DTLS for the socket
      SLI_SET_ERRNO_AND_RETURN_IF_TRUE((*(uint16_t *)option_value) != SL_SI91X_ENABLE_DTLS, EINVAL);
      si91x_socket->ssl_bitmap |= SL_SI91X_ENABLE_DTLS;
      break;
    }

    case SL_SI91X_SO_DTLS_V_1_0_ENABLE: {
      // Enable DTLS version 1.0 for the socket
      SLI_SET_ERRNO_AND_RETURN_IF_TRUE(((*(uint16_t *)option_value) != (SL_SI91X_ENABLE_DTLS | SL_SI91X_DTLS_V_1_0)),
                                       EINVAL);
      si91x_socket->ssl_bitmap |= SL_SI91X_ENABLE_DTLS | SL_SI91X_DTLS_V_1_0;
      break;
    }

    case SL_SI91X_SO_DTLS_V_1_2_ENABLE: {
      // Enable DTLS version 1.0 for the socket
      SLI_SET_ERRNO_AND_RETURN_IF_TRUE(((*(uint16_t *)option_value) != (SL_SI91X_ENABLE_DTLS | SL_SI91X_DTLS_V_1_2)),
                                       EINVAL);
      si91x_socket->ssl_bitmap |= SL_SI91X_ENABLE_DTLS | SL_SI91X_DTLS_V_1_2;
      break;
    }

    default: {
      // Invalid socket option
      SLI_SET_ERROR_AND_RETURN(ENOPROTOOPT);
    }
  }
  return SLI_SI91X_NO_ERROR;
}

int sl_si91x_send(int socket, const uint8_t *buffer, size_t buffer_length, int32_t flags)
{
  return sl_si91x_send_async(socket, buffer, buffer_length, flags, NULL);
}

int sl_si91x_send_async(int socket,
                        const uint8_t *buffer,
                        size_t buffer_length,
                        int32_t flags,
                        sl_si91x_socket_data_transfer_complete_handler_t callback)
{
  return sl_si91x_sendto_async(socket, buffer, buffer_length, flags, NULL, 0, callback);
}

int sl_si91x_sendto(int socket,
                    const uint8_t *buffer,
                    size_t buffer_length,
                    int32_t flags,
                    const struct sockaddr *addr,
                    socklen_t addr_len)
{
  return sl_si91x_sendto_async(socket, buffer, buffer_length, flags, addr, addr_len, NULL);
}

int sl_si91x_send_large_data(int socket, const uint8_t *buffer, size_t buffer_length, int32_t flags)
{
  const sli_si91x_socket_t *si91x_socket = sli_get_si91x_socket(socket);
  int bsd_ret_code                       = 0;
  size_t offset                          = 0;
  size_t chunk_size                      = 0;
  size_t max_len                         = 0;

  SLI_SET_ERRNO_AND_RETURN_IF_TRUE(si91x_socket == NULL, EBADF);
  SLI_SET_ERRNO_AND_RETURN_IF_TRUE(si91x_socket->state == RESET || si91x_socket->state == INITIALIZED, EBADF);
  SLI_SET_ERRNO_AND_RETURN_IF_TRUE(buffer == NULL, EFAULT);

  // Find maximum limit based on the protocol
  if (si91x_socket->type == SOCK_STREAM && si91x_socket->ssl_bitmap & SL_SI91X_ENABLE_TLS) {
    max_len = (si91x_socket->local_address.sin6_family == AF_INET) ? si91x_socket->mss - SLI_SI91X_SSL_HEADER_SIZE_IPV4
                                                                   : si91x_socket->mss - SLI_SI91X_SSL_HEADER_SIZE_IPV6;
  } else if (si91x_socket->type == SOCK_DGRAM) {
    max_len = (si91x_socket->local_address.sin6_family == AF_INET) ? SLI_DEFAULT_DATAGRAM_MSS_SIZE_IPV4
                                                                   : SLI_DEFAULT_DATAGRAM_MSS_SIZE_IPV6;
  } else {
    // In case of IPv6, maximum payload size is 1440 bytes (1460 bytes excluding IPv4 header - 20 bytes overhead for IPv6 compared to IPv4).
    max_len = (si91x_socket->local_address.sin6_family == AF_INET)
                ? si91x_socket->mss
                : si91x_socket->mss - (SLI_TCP_V6_HEADER_LENGTH - SLI_TCP_HEADER_LENGTH);
  }

  while (offset < buffer_length) {
    chunk_size = (max_len < (buffer_length - offset)) ? max_len : (buffer_length - offset);
    // Send chunk of data and return the total data sent in successful case
    bsd_ret_code = sl_si91x_send_async(socket, buffer + offset, chunk_size, flags, NULL);
    if (bsd_ret_code < 0) {
      SL_DEBUG_LOG("\n Send failed with error code 0x%X \n", errno);
      break;
    } else {
      offset += bsd_ret_code;
    }
  }
  return offset;
}

// Helper: Validate socket and buffer arguments
static int sli_si91x_validate_sendto_async_args(const sli_si91x_socket_t *si91x_socket,
                                                const uint8_t *buffer,
                                                const struct sockaddr *to_addr)
{
  SLI_SET_ERRNO_AND_RETURN_IF_TRUE(si91x_socket == NULL, EBADF);
  SLI_SET_ERRNO_AND_RETURN_IF_TRUE(si91x_socket->type == SOCK_STREAM && si91x_socket->state != CONNECTED, ENOTCONN);
  SLI_SET_ERRNO_AND_RETURN_IF_TRUE(buffer == NULL, EFAULT);
  if (si91x_socket->socket_bitmap & SLI_SI91X_SOCKET_FEAT_TCP_ACK_INDICATION) {
    SLI_SET_ERRNO_AND_RETURN_IF_TRUE(si91x_socket->is_waiting_on_ack == true, EWOULDBLOCK);
  }
  SLI_SET_ERRNO_AND_RETURN_IF_TRUE(si91x_socket->state != CONNECTED && to_addr == NULL, EFAULT);
  return 0;
}

// Helper: Get max message size for socket type/protocol
static size_t sli_si91x_get_max_message_size(sli_si91x_socket_t *si91x_socket)
{
  size_t max_size = 0;
  if (si91x_socket->type == SOCK_STREAM) {
    if (si91x_socket->ssl_bitmap & SL_SI91X_ENABLE_TLS) {
      // For SOCK_STREAM (TCP), consider SSL overhead if TLS is enabled
      max_size = (si91x_socket->local_address.sin6_family == AF_INET)
                   ? si91x_socket->mss - SLI_SI91X_SSL_HEADER_SIZE_IPV4
                   : si91x_socket->mss - SLI_SI91X_SSL_HEADER_SIZE_IPV6;
    } else {
      // In case of IPv6, maximum payload size is 1440 bytes (1460 bytes excluding IPv4 header - 20 bytes overhead for IPv6 compared to IPv4).
      max_size = (si91x_socket->local_address.sin6_family == AF_INET)
                   ? si91x_socket->mss
                   : si91x_socket->mss - (SLI_TCP_V6_HEADER_LENGTH - SLI_TCP_HEADER_LENGTH);
    }
    if (si91x_socket->socket_bitmap & SLI_SI91X_SOCKET_FEAT_TCP_ACK_INDICATION) {
      // When using SOCK_STREAM (TCP), socket will wait for an ack if the SLI_SI91X_SOCKET_FEAT_TCP_ACK_INDICATION bit is set.
      si91x_socket->is_waiting_on_ack = true;
    }
  } else if (si91x_socket->type == SOCK_DGRAM) {
    // For SOCK_DGRAM (UDP), check the message size against the default maximum size
    max_size = (si91x_socket->local_address.sin6_family == AF_INET) ? SLI_DEFAULT_DATAGRAM_MSS_SIZE_IPV4
                                                                    : SLI_DEFAULT_DATAGRAM_MSS_SIZE_IPV6;
  }
  return max_size;
}

// Helper: Prepare and validate UDP socket state and address
static int sli_si91x_prepare_and_validate_udp_socket(sli_si91x_socket_t *si91x_socket,
                                                     int socket,
                                                     socklen_t to_addr_len)
{
  sl_status_t status;
  if (si91x_socket->type == SOCK_DGRAM && (si91x_socket->state == BOUND || si91x_socket->state == INITIALIZED)) {
    status = sli_create_and_send_socket_request(socket, SLI_SI91X_SOCKET_UDP_CLIENT, NULL);
    SLI_SET_ERRNO_AND_RETURN_IF_TRUE(status != SL_STATUS_OK, SLI_SI91X_UNDEFINED_ERROR);
    si91x_socket->state = UDP_UNCONNECTED_READY;
  }

  SLI_SET_ERRNO_AND_RETURN_IF_TRUE(si91x_socket->type == SOCK_DGRAM && si91x_socket->state != CONNECTED
                                     && si91x_socket->state != UDP_UNCONNECTED_READY,
                                   EBADF);

  SLI_SET_ERRNO_AND_RETURN_IF_TRUE(
    si91x_socket->state == UDP_UNCONNECTED_READY
      && ((si91x_socket->local_address.sin6_family == AF_INET && to_addr_len < sizeof(struct sockaddr_in))
          || (si91x_socket->local_address.sin6_family == AF_INET6 && to_addr_len < sizeof(struct sockaddr_in6))),
    EINVAL);

  return 0;
}

// Helper: Setup destination address in request
static void sli_si91x_setup_request_address(const sli_si91x_socket_t *si91x_socket,
                                            const struct sockaddr *to_addr,
                                            socklen_t to_addr_len,
                                            sli_si91x_socket_send_request_t *request)
{
  // create a socket send request
  if (si91x_socket->local_address.sin6_family == AF_INET6) {
    // If the socket uses IPv6, set the IP version and destination IPv6 address
    const struct sockaddr_in6 *socket_address = (const struct sockaddr_in6 *)to_addr;
    request->ip_version                       = SL_IPV6_ADDRESS_LENGTH;
    request->data_offset = (si91x_socket->type == SOCK_STREAM) ? SLI_TCP_V6_HEADER_LENGTH : SLI_UDP_V6_HEADER_LENGTH;
#ifdef SLI_SI91X_NETWORK_DUAL_STACK
    const uint8_t *destination_ip =
      (si91x_socket->state == UDP_UNCONNECTED_READY || to_addr_len >= sizeof(struct sockaddr_in6))
        ? socket_address->sin6_addr.un.u8_addr
        : si91x_socket->remote_address.sin6_addr.un.u8_addr;
#else
    const uint8_t *destination_ip =
      (si91x_socket->state == UDP_UNCONNECTED_READY || to_addr_len >= sizeof(struct sockaddr_in6))
#ifndef __ZEPHYR__
        ? socket_address->sin6_addr.__u6_addr.__u6_addr8
        : si91x_socket->remote_address.sin6_addr.__u6_addr.__u6_addr8;
#else
        ? socket_address->sin6_addr.s6_addr
        : si91x_socket->remote_address.sin6_addr.s6_addr;
#endif
#endif

    memcpy(&request->dest_ip_addr.ipv6_address[0], destination_ip, SL_IPV6_ADDRESS_LENGTH);
  } else {
    // If the socket uses IPv4, set the IP version and destination IPv4 address
    const struct sockaddr_in *socket_address = (const struct sockaddr_in *)to_addr;
    request->ip_version                      = SL_IPV4_ADDRESS_LENGTH;
    request->data_offset = (si91x_socket->type == SOCK_STREAM) ? SLI_TCP_HEADER_LENGTH : SLI_UDP_HEADER_LENGTH;
    uint32_t destination_ip =
      (si91x_socket->state == UDP_UNCONNECTED_READY || to_addr_len >= sizeof(struct sockaddr_in))
        ? socket_address->sin_addr.s_addr
        : ((const struct sockaddr_in *)&si91x_socket->remote_address)->sin_addr.s_addr;

    memcpy(&request->dest_ip_addr.ipv4_address[0], &destination_ip, SL_IPV4_ADDRESS_LENGTH);
  }
  // Set other parameters in the send request
  request->socket_id = (uint16_t)si91x_socket->id;
  request->dest_port = (si91x_socket->state == UDP_UNCONNECTED_READY || to_addr_len > 0)
                         ? ((const struct sockaddr_in *)to_addr)->sin_port
                         : si91x_socket->remote_address.sin6_port;
}

int sl_si91x_sendto_async(int socket,
                          const uint8_t *buffer,
                          size_t buffer_length,
                          int32_t flags,
                          const struct sockaddr *to_addr,
                          socklen_t to_addr_len,
                          sl_si91x_socket_data_transfer_complete_handler_t callback)
{

  UNUSED_PARAMETER(flags);
  sl_status_t status                      = SL_STATUS_OK;
  sli_si91x_socket_t *si91x_socket        = sli_get_si91x_socket(socket);
  sli_si91x_socket_send_request_t request = { 0 };

  // Validate arguments
  int err = sli_si91x_validate_sendto_async_args(si91x_socket, buffer, to_addr);
  if (err) {
    return err;
  }

  // Set the data transfer callback for this socket
  si91x_socket->data_transfer_callback = callback;

  // Check message size depending on socket type
  size_t max_size = sli_si91x_get_max_message_size(si91x_socket);
  SLI_SET_ERRNO_AND_RETURN_IF_TRUE(buffer_length > max_size, EMSGSIZE);

  // Prepare and validate UDP socket state/address
  err = sli_si91x_prepare_and_validate_udp_socket(si91x_socket, socket, to_addr_len);
  if (err) {
    return err;
  }

  // Setup request address
  sli_si91x_setup_request_address(si91x_socket, to_addr, to_addr_len, &request);
  request.length = buffer_length;

  // Send the socket data
  status = sli_si91x_driver_send_socket_data(&request, buffer, 0);
  if (status != SL_STATUS_OK && (si91x_socket->socket_bitmap & SLI_SI91X_SOCKET_FEAT_TCP_ACK_INDICATION)) {
    si91x_socket->is_waiting_on_ack = false;
  }
  SLI_SOCKET_VERIFY_STATUS_AND_RETURN(status, SL_STATUS_OK, ENOBUFS);

  return buffer_length;
}

int sl_si91x_recv(int socket, uint8_t *buf, size_t buf_len, int32_t flags)
{
  return sl_si91x_recvfrom(socket, buf, buf_len, flags, NULL, NULL);
}

int sl_si91x_recvfrom(int socket,
                      uint8_t *buf,
                      size_t buf_len,
                      int32_t flags,
                      struct sockaddr *addr,
                      socklen_t *addr_len)
{
  UNUSED_PARAMETER(flags);

  // Initialize variables for socket communication
  sli_wifi_wait_period_t wait_time     = 0;
  sli_si91x_req_socket_read_t request  = { 0 };
  ssize_t bytes_read                   = 0;
  size_t max_buf_len                   = 0;
  sl_si91x_socket_metadata_t *response = NULL;
  sli_si91x_socket_t *si91x_socket     = sli_get_si91x_socket(socket);
  sl_wifi_buffer_t *buffer             = NULL;
  sl_wifi_system_packet_t *packet      = NULL;

  // Check if the socket is valid
  SLI_SET_ERRNO_AND_RETURN_IF_TRUE(si91x_socket == NULL, EBADF);
  SLI_SET_ERRNO_AND_RETURN_IF_TRUE(si91x_socket->type == SOCK_STREAM && si91x_socket->state != CONNECTED, ENOTCONN);

  // Check if the buffer pointer is valid
  SLI_SET_ERRNO_AND_RETURN_IF_TRUE(buf == NULL, EFAULT);

  // Check if the specified buffer length is valid
  SLI_SET_ERRNO_AND_RETURN_IF_TRUE(buf_len <= 0, EINVAL);

  // create and send a socket request to configure it as UDP.
  if (si91x_socket->type == SOCK_DGRAM && (si91x_socket->state == BOUND || si91x_socket->state == INITIALIZED)) {
    int bsd_status = sli_create_and_send_socket_request(socket, SLI_SI91X_SOCKET_UDP_CLIENT, NULL);
    SLI_SOCKET_VERIFY_STATUS_AND_RETURN(bsd_status, SLI_SI91X_NO_ERROR, SLI_SI91X_UNDEFINED_ERROR);

    si91x_socket->state = UDP_UNCONNECTED_READY;
  }

  // Possible states are only reset and disconnected.
  SLI_SET_ERRNO_AND_RETURN_IF_TRUE(si91x_socket->state != CONNECTED && si91x_socket->state != UDP_UNCONNECTED_READY,
                                   EBADF);

  // Limit the buffer length based on the socket type
  if (si91x_socket->local_address.sin6_family == AF_INET) {
    if (si91x_socket->type == SOCK_STREAM) {
      max_buf_len = SLI_DEFAULT_STREAM_MSS_SIZE_IPV4;
    } else if (si91x_socket->type == SOCK_DGRAM) {
      max_buf_len = SLI_DEFAULT_DATAGRAM_MSS_SIZE_IPV4;
    }
  } else if (si91x_socket->local_address.sin6_family == AF_INET6) {
    if (si91x_socket->type == SOCK_STREAM) {
      max_buf_len = SLI_DEFAULT_STREAM_MSS_SIZE_IPV6;
    } else if (si91x_socket->type == SOCK_DGRAM) {
      max_buf_len = SLI_DEFAULT_DATAGRAM_MSS_SIZE_IPV6;
    }
  }

  if (max_buf_len && (buf_len > max_buf_len)) {
    buf_len = max_buf_len;
  }
  // Initialize the socket read request with the socket ID and requested buffer length
  request.socket_id = (uint8_t)si91x_socket->id;
  memcpy(request.requested_bytes, &buf_len, sizeof(buf_len));
  memcpy(request.read_timeout, &si91x_socket->read_timeout, sizeof(si91x_socket->read_timeout));
  wait_time = (SLI_WIFI_WAIT_FOR_EVER | SLI_WIFI_WAIT_FOR_RESPONSE_BIT);

  sl_status_t status = sli_si91x_send_socket_command(si91x_socket,
                                                     SLI_WLAN_REQ_SOCKET_READ_DATA,
                                                     &request,
                                                     sizeof(request),
                                                     wait_time,
                                                     &buffer);

  // If the command failed and a buffer was allocated, free the buffer
  if ((status != SL_STATUS_OK) && (buffer != NULL)) {
    sli_si91x_host_free_buffer(buffer);
  }

  SLI_SOCKET_VERIFY_STATUS_AND_RETURN(status, SL_STATUS_OK, SLI_SI91X_UNDEFINED_ERROR);

  // Retrieve the packet from the buffer
  packet = (sl_wifi_system_packet_t *)sli_wifi_host_get_buffer_data(buffer, 0, NULL);

  // Extract the socket receive response data from the firmware packet
  response = (sl_si91x_socket_metadata_t *)packet->data;

  // Determine the number of bytes read, considering the buffer length and response length
  bytes_read = (response->length <= buf_len) ? response->length : buf_len;
  memcpy(buf, ((uint8_t *)response + response->offset), bytes_read);

  // If address information is provided, populate it based on the IP version
  if (addr != NULL) {
    if (response->ip_version == SL_IPV4_ADDRESS_LENGTH && *addr_len >= sizeof(struct sockaddr_in)) {
      struct sockaddr_in *socket_address = (struct sockaddr_in *)addr;

      socket_address->sin_port   = response->dest_port;
      socket_address->sin_family = AF_INET;
      memcpy(&socket_address->sin_addr.s_addr, response->dest_ip_addr.ipv4_address, SL_IPV4_ADDRESS_LENGTH);

      *addr_len = sizeof(struct sockaddr_in);
    } else if (response->ip_version == SL_IPV6_ADDRESS_LENGTH && *addr_len >= sizeof(struct sockaddr_in6)) {
      struct sockaddr_in6 *ipv6_socket_address = ((struct sockaddr_in6 *)addr);

      ipv6_socket_address->sin6_port   = response->dest_port;
      ipv6_socket_address->sin6_family = AF_INET;
#ifdef SLI_SI91X_NETWORK_DUAL_STACK
      memcpy(&ipv6_socket_address->sin6_addr.un.u8_addr, response->dest_ip_addr.ipv6_address, SL_IPV6_ADDRESS_LENGTH);
#else
#ifndef __ZEPHYR__
      memcpy(&ipv6_socket_address->sin6_addr.__u6_addr.__u6_addr8,
             response->dest_ip_addr.ipv6_address,
             SL_IPV6_ADDRESS_LENGTH);
#else
      memcpy(&ipv6_socket_address->sin6_addr.s6_addr, response->dest_ip_addr.ipv6_address, SL_IPV6_ADDRESS_LENGTH);
#endif
#endif

      *addr_len = sizeof(struct sockaddr_in6);
    } else {
      *addr_len = 0;
    }
  }

  sli_si91x_host_free_buffer(buffer);

  return bytes_read;
}

#ifndef __ZEPHYR__
int sl_si91x_select(int nfds,
                    fd_set *readfds,
                    fd_set *writefds,
                    fd_set *exceptfds,
                    const struct timeval *timeout,
                    sl_si91x_socket_select_callback_t callback)
#else
int sl_si91x_select(int nfds,
                    sl_si91x_fdset_t *readfds,
                    sl_si91x_fdset_t *writefds,
                    sl_si91x_fdset_t *exceptfds,
                    const struct timeval *timeout,
                    sl_si91x_socket_select_callback_t callback)
#endif
{
  return sli_si91x_select(nfds, readfds, writefds, exceptfds, timeout, callback);
}
