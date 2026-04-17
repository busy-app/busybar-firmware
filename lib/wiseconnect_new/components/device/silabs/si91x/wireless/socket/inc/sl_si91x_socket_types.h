/********************************************************************************
 * @file  sl_si91x_socket_types.h
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

#pragma once

#include <stdint.h>
#include "sli_wifi_types.h"
#include "sl_si91x_types.h"
#include "cmsis_os2.h" // CMSIS RTOS2
#ifdef SLI_SI91X_NETWORK_DUAL_STACK
#include "lwip/sockets.h"
#else
#ifndef __ZEPHYR__
#include "socket.h"
#include "select.h"
#else
#include <sys/socket.h>
#endif
#endif
#include "sl_si91x_protocol_types.h"

/**
 * @addtogroup SI91X_SOCKET_FUNCTIONS
 * @{ 
 */
/**
 * @brief Structure for socket metadata associated with read event.
 *
 * @details
 * The structure holds the metadata information for the socket's read event.
 * It includes the following details such as: IP version, socket ID, length of data received,
 * offset within the buffer, destination port, and the IP address of the sender device. 
 * The IP address can be either IPv4 or IPv6, determined by the
 * `ip_version` field.
 */
typedef struct {
  uint16_t
    ip_version; ///< Two bytes for the IP version of the IP address, four bytes for the IPv4, and six bytes for the IPv6.

  uint16_t socket_id; ///< The socket number associated with the read event is two bytes.

  uint32_t length; ///< Four bytes. Length of received data.

  uint16_t offset; ///< Two bytes. Offset data from the start of the buffer.

  uint16_t dest_port; ///< Two bytes. Port number of the device which sends data to the destination.

  union {
    uint8_t ipv4_address[4]; ///< Four bytes. IPv4 address of the device which sends data. Used if ip_version is four.

    uint8_t ipv6_address[16]; ///< 16 bytes. IPv6 address of the device which sends data. Used if ip_version is six.
  } dest_ip_addr;             ///< Union for IPv4 or IPv6 address, depending on ip_version.
} sl_si91x_socket_metadata_t;

/**
 * @typedef sl_si91x_socket_receive_data_callback_t
 * @brief Callback function reads asynchronous data from the socket.
 *
 * @details
 * The callback function reads asynchronous data from the socket when the sl_si91x_socket_async
 * API is registered and called. The callback provides the following details:
 * socket ID, pointer to the buffer which contains receiver data, size of the buffer, 
 * and metadata of the receiver packet (such as IP address, and port number).
 * 
 *
 * @param socket
 *   Socket ID.
 *
 * @param buffer
 *   Pointer to the buffer which stores the receiver data.
 *
 * @param length
 *   Buffer size.
 *
 * @param firmware_socket_response
 * Pointer to sl_si91x_socket_metadata_t structure contains receiver packet metadata information. 
 * The metadata information consists of IP address (either, Ipv4 or IPV6), and port number.
 *
 * @return
 *   N/A
 */
typedef void (*sl_si91x_socket_receive_data_callback_t)(uint32_t socket,
                                                        uint8_t *buffer,
                                                        uint32_t length,
                                                        const sl_si91x_socket_metadata_t *firmware_socket_response);

/**
 * @typedef sl_si91x_socket_accept_callback_t
 * @brief Callback functions for new asynchronous accepted connection.
 *
 * @details
 * The callback provides paramenters for new accepted connection when the sl_si91x_accept_async API is registered and called. 
 * The callback provides the following details: socket ID of the accepted connection, address of remoter peer, 
 * and IP version of connection.
 *
 * @param socket
 *   Socket ID of the accepted connection.
 *
 * @param addr
 *   Pointer to `struct sockaddr` contains remote peer address.
 *
 * @param ip_version
 *   IP version of the connection (for example, four bytes for IPv4, and six bytes for IPv6).
 *
 * @return
 *   N/A
 */
typedef void (*sl_si91x_socket_accept_callback_t)(int32_t socket, struct sockaddr *addr, uint8_t ip_version);

/**
 * @typedef sl_si91x_socket_data_transfer_complete_handler_t
 * @brief Callback function indicates data transfer status.
 *
 * @details
 * The callback indicates the data transfer completion status when either of the sl_si91x_send_async or sl_si91x_sendto_async API is registered and called.
 * The callback provides the socket ID, and the number of bytes that are successfully transfer.
 *
 * @param socket
 *   Socket ID.
 *
 * @param length
 *   Number of bytes transferred.
 *
 * @return
 *   N/A
 */
typedef void (*sl_si91x_socket_data_transfer_complete_handler_t)(int32_t socket, uint16_t length);

/**
 * @typedef sl_si91x_socket_select_callback_t
 * @brief Callback function indicates asynchronous select request result.
 *
 * @details
 * The callback indicates asynchronous response reaches the select request when the sl_si91x_select API is registered and called. 
 * The callback provides the following details: file descriptor sets for read, write, and exception conditions, and status of the selected request.
 *
 * @param fd_read
 *   File descriptor pointer sets for read operations.
 *
 * @param fd_write
 *   File descriptor pointer sets for write operations.
 *
 * @param fd_except
 *   File descriptor pointer sets for exception condition.
 *
 * @param status
 *   Select request status.
 *
 * @return
 *   N/A
 */
#ifndef __ZEPHYR__
typedef void (*sl_si91x_socket_select_callback_t)(fd_set *fd_read, fd_set *fd_write, fd_set *fd_except, int32_t status);
#else
typedef void (*sl_si91x_socket_select_callback_t)(sl_si91x_fdset_t *fd_read,
                                                  sl_si91x_fdset_t *fd_write,
                                                  sl_si91x_fdset_t *fd_except,
                                                  int32_t status);
#endif

/**
 * @typedef sl_si91x_socket_remote_termination_callback_t
 * @brief Callback function indicates termination of the remote socket.
 *
 * @details
 * The callback function notifies on the termination of the remote socket when the sl_si91x_set_remote_termination_callback API is registered and called. 
 * The callback provides the following details: socket ID, remote socket port number, and number of bytes sent before termination of the remote socket.
 *
 * @param socket
 *   Socket ID.
 *
 * @param port
 *   Remote socket port number.
 *
 * @param bytes_sent
 *   Number of bytes sent before termination.
 *
 * @return
 *  The callback does not returns value.
 */
typedef void (*sl_si91x_socket_remote_termination_callback_t)(int socket, uint16_t port, uint32_t bytes_sent);

/** @} */

/// Internal  si91x BSD socket status
typedef enum {
  RESET = 0,   // State of unallocated socket.
  INITIALIZED, // Socket attains this state when socket() has been executed successfully.
  BOUND,       // Socket attains this state when bind() has been executed successfully.
  LISTEN,      // (TCP ONLY STATE) Socket attains this state when listen() has been executed successfully.
  UDP_UNCONNECTED_READY, // (UDP ONLY STATE) Socket attains this state when sendto() or recvfrom() has been executed successfully prior connect.
  CONNECTED,   // Socket attains this state when connect() has been executed successfully.
  DISCONNECTED // Socket attains this state when underlying connection is lost
} sli_si91x_bsd_socket_state_t;

/// Internal reasons for SI91x BSD socket disconnection, applicable only when the socket state is DISCONNECTED.
typedef enum {
  SLI_SI91X_BSD_DISCONNECT_REASON_INTERFACE_DOWN, // Indicates the connection was lost due to the network interface going down.
  SLI_SI91X_BSD_DISCONNECT_REASON_REMOTE_CLOSED // Indicates the connection was terminated by the remote endpoint.
} sli_si91x_bsd_disconnect_reason_t;

#define SLI_SI91X_MAX_SIZE_OF_EXTENSION_DATA 256

#ifdef SLI_SI91X_NETWORK_DUAL_STACK
#if !LWIP_IPV6
struct sockaddr_in6 {
  u8_t sin6_len;             /* length of this structure    */
  sa_family_t sin6_family;   /* AF_INET6                    */
  in_port_t sin6_port;       /* Transport layer port #      */
  u32_t sin6_flowinfo;       /* IPv6 flow information       */
  struct in6_addr sin6_addr; /* IPv6 address                */
  u32_t sin6_scope_id;       /* Set of interfaces for scope */
};
#endif
#endif

#pragma pack()
/// Internal  si91x TLS extensions
typedef struct {
  uint8_t buffer[SLI_SI91X_MAX_SIZE_OF_EXTENSION_DATA]; ///< Buffer
  uint16_t total_extensions;                            ///< Total extensions
  uint16_t current_size_of_extensions;                  ///< Current size of extensions
} sli_si91x_tls_extensions_t;

/// Structure to hold WebSocket host, resource and subprotocol information
typedef struct {
  uint8_t host_length;        ///< Length of WebSocket host name
  uint8_t resource_length;    ///< Length of WebSocket resource name
  uint8_t subprotocol_length; ///< Length of WebSocket subprotocol name
  uint8_t websocket_data[];   ///< WebSocket host name, resource name and subprotocol name
} sli_si91x_websocket_info_t;

#pragma pack()

/// Internal si91x socket handle
typedef struct {
  int32_t id;                          ///< Socket ID
  int32_t type;                        ///< Socket type
  int32_t index;                       ///< Socket index
  int role;                            ///< Socket role
  int32_t protocol;                    ///< Protocol
  uint16_t tcp_keepalive_initial_time; ///< TCP keepalive intial time
  uint8_t max_tcp_retries;             ///< MAX TCOP retries
  uint16_t read_timeout;               ///< Read timeout
  uint8_t certificate_index;           ///< Certificate Index
  uint8_t vap_id;                      ///< Virtual AP ID
  uint16_t mss;                        ///< Maximum segment size (MSS) value
  struct sockaddr_in6 local_address;   ///< Using sockaddr_in6 to hold either IPV4 or IPV6.
  struct sockaddr_in6 remote_address;  ///< Using sockaddr_in6 to hold either IPV4 or IPV6.
  sli_si91x_bsd_socket_state_t state;  ///< BSD socket state (used for internal tracking)
  sli_si91x_bsd_disconnect_reason_t
    disconnect_reason; ///< BSD socket disconnection reasons, applicable only when the socket state is DISCONNECTED.
  sli_si91x_tls_extensions_t tls_extensions; ///< TLS Extension
  bool is_waiting_on_ack;                    ///< Boolean flag to check if socket is waiting for an ack.
#if defined(SLI_SI917)
  uint32_t ssl_bitmap;                       ///< SSL bitmap
  uint32_t max_retransmission_timeout_value; ///< Max retransmission timeout value
  uint32_t tos;                              ///< TOS
#else
  uint8_t ssl_bitmap; ///< SSL Bitmap
#endif
  uint8_t opcode;                                                          ///< Opcode used in websocket
  sli_si91x_websocket_info_t *websocket_info;                              ///< Pointer to WebSocket info
  sl_si91x_socket_receive_data_callback_t recv_data_callback;              ///< Receive data callback
  sl_si91x_socket_data_transfer_complete_handler_t data_transfer_callback; ///< Data transfer callback
  sl_si91x_socket_accept_callback_t user_accept_callback;                  ///< Async Accept callback
  osEventFlagsId_t socket_events;                                          ///< Event Flags for sockets
  int32_t client_id;                                                       ///< Client Socket Id for accept
  uint8_t socket_bitmap;                                                   ///< Socket Bitmap
  uint8_t data_buffer_count;              ///< Number of queued data buffers allocated by this socket
  uint8_t data_buffer_limit;              ///< Maximum number of queued data buffers permitted for this socket
  sli_wifi_command_queue_t command_queue; ///< Command queue
  sli_wifi_buffer_queue_t tx_data_queue;  ///< Transmit data queue
  sli_wifi_buffer_queue_t rx_data_queue;  ///< Receive data queue
  uint8_t *domain_name;                   ///< Expected domain name for TLS certificate verification
} sli_si91x_socket_t;
