/***************************************************************************/ /**
 * @file  sl_si91x_socket.h
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
#include "sl_si91x_socket_types.h"

#ifdef __cplusplus
extern "C" {
#endif
/**
 * @addtogroup SI91X_SOCKET_FUNCTIONS
 * @{ 
 */
/**
 * @brief Creates a new socket.
 *
 * @details
 * The function creates a new socket and returns a file descriptor for the respective socket.
 * The socket is used for communication within the specified protocol family,
 * type, and protocol. The created socket is used for various network operations
 * such as, connecting to a remote host, sending and receiving data, and so on.
 *
 * @param[in] family 
 *   Specifies the communication domain for the socket. This selects the protocol family to be used. 
 *   Accepts values from @ref BSD_SOCKET_FAMILIY. Currently, only @ref AF_INET and @ref AF_INET6 are supported.
 *
 * @param[in] type 
 *   Specifies the type of the socket, which determines the semantics of communication. 
 *   Accepts values from @ref BSD_SOCKET_TYPES. Currently, only @ref SOCK_STREAM and @ref SOCK_DGRAM are supported.
 *
 * @param[in] protocol 
 *   Specifies a particular protocol to be used with the socket. 
 *   Accepts values from @ref BSD_SOCKET_PROTOCOL. Currently, only @ref IPPROTO_TCP, @ref IPPROTO_UDP, and @ref IPPROTO_IP are supported.
 *
 * @return 
 *   Returns the socket ID or file descriptor for the newly created socket on success, or -1 on failure.
 */
int sl_si91x_socket(int family, int type, int protocol);

/**
 * @brief Creates an asynchronous socket and registers the provided callback.
 * 
 * @details
 *    This function creates a new asynchronous socket and registers a callback function
 *    that is called whenever data is received on the socket. The socket can be used
 *    for communication within the specified protocol family, type, and protocol.
 *
 * @param[in] family Specifies the communication domain for the socket. This selects the protocol family to be used. 
 *                   Accepts values from @ref BSD_SOCKET_FAMILIY. Currently, only @ref AF_INET and @ref AF_INET6 are supported.
 *
 * @param[in] type Specifies the type of the socket, which determines the semantics of communication. 
 *                 Accepts values from @ref BSD_SOCKET_TYPES. Currently, only @ref SOCK_STREAM and @ref SOCK_DGRAM are supported.
 *
 * @param[in] protocol Specifies a particular protocol to be used with the socket. 
 *                     Accepts values from @ref BSD_SOCKET_PROTOCOL. Currently, only @ref IPPROTO_TCP, @ref IPPROTO_UDP, and @ref IPPROTO_IP are supported.
 *
 *  @param[in] callback A function pointer of type @ref sl_si91x_socket_receive_data_callback_t. This function is called when the socket receives data.
 *
 * @return Returns the socket ID or file descriptor for the newly created socket on success, or -1 on failure.
 */
int sl_si91x_socket_async(int family, int type, int protocol, sl_si91x_socket_receive_data_callback_t callback);

/**
 * @brief Sets a specified socket option on the identified socket asynchronously.
 *
 * @details
 * This function sets a specified option for a given socket asynchronously. The options
 * can be set at various levels and include parameters such as receive timeout, maximum
 * retries, maximum segment size, TCP keepalive, SSL options, and so on.
 *
 * @param[in] socket 
 *   The socket ID or file descriptor for the specified socket.
 *
 * @param[in] level 
 *   The option level. Accepts values from @ref BSD_SOCKET_OPTION_LEVEL.
 *
 * @param[in] option_name 
 *   The option to be configured. Accepts values from @ref SI91X_SOCKET_OPTION_NAME. 
 *   Currently, following options are supported:
 *   - @ref SL_SI91X_SO_RCVTIME
 *   - @ref SL_SI91X_SO_MAXRETRY
 *   - @ref SL_SI91X_SO_MSS
 *   - @ref SL_SI91X_SO_TCP_KEEPALIVE
 *   - @ref SL_SI91X_SO_HIGH_PERFORMANCE_SOCKET
 *   - @ref SL_SI91X_SO_SSL_ENABLE
 *   - @ref SL_SI91X_SO_SSL_V_1_0_ENABLE 
 *   - @ref SL_SI91X_SO_SSL_V_1_1_ENABLE 
 *   - @ref SL_SI91X_SO_SSL_V_1_2_ENABLE
 *   - @ref SL_SI91X_SO_SOCK_VAP_ID
 *   - @ref SL_SI91X_SO_SSL_V_1_3_ENABLE
 *   - @ref SL_SI91X_SO_CERT_INDEX
 *   - @ref SL_SI91X_SO_TLS_SNI
 *   - @ref SL_SI91X_SO_TLS_ALPN
 *   - @ref SL_SI91X_SO_MAX_RETRANSMISSION_TIMEOUT_VALUE
 *
 * @param[in] option_value 
 *   The value of the parameter.
 *   | option_name                                       | option_value                              |  description                                                                                                               |
 *   |---------------------------------------------------|-------------------------------------------|----------------------------------------------------------------------------------------------------------------------------|
 *   | @ref SL_SI91X_SO_RCVTIME                          | sl_si91x_time_value                       | Socket Receive timeout. sl_si91x_time_value structure is used to represent time in two parts: seconds and microseconds.    |
 *   | @ref SL_SI91X_SO_MAXRETRY                         | uint16_t                                  | Maximum number of TCP retries                                                                                              |
 *   | @ref SL_SI91X_SO_MSS                              | uint16_t                                  | Maximum Segment Size (MSS) for the TCP connection                                                                          |
 *   | @ref SL_SI91X_SO_TCP_KEEPALIVE                    | uint16_t                                  | Set TCP keepalive in seconds                                                                                               |
 *   | @ref SL_SI91X_SO_HIGH_PERFORMANCE_SOCKET          | BIT(7)                                    | Set high performance socket                                                                                                |
 *   | @ref SL_SI91X_SO_SSL_ENABLE                       | SL_SI91X_ENABLE_TLS                       | Enable TLS/SSL                                                                                                             |
 *   | @ref SL_SI91X_SO_SSL_V_1_0_ENABLE                 | SL_SI91X_ENABLE_TLS \| SL_SI91X_TLS_V_1_0 | Enable TLS v1.0                                                                                                            |
 *   | @ref SL_SI91X_SO_SSL_V_1_1_ENABLE                 | SL_SI91X_ENABLE_TLS \| SL_SI91X_TLS_V_1_1 | Enable TLS v1.1                                                                                                            |
 *   | @ref SL_SI91X_SO_SSL_V_1_2_ENABLE                 | SL_SI91X_ENABLE_TLS \| SL_SI91X_TLS_V_1_2 | Enable TLS v1.2                                                                                                            |
 *   | @ref SL_SI91X_SO_SSL_V_1_3_ENABLE                 | SL_SI91X_ENABLE_TLS \| SL_SI91X_TLS_V_1_3 | Enable TLS v1.3                                                                                                            |
 *   | @ref SL_SI91X_SO_SOCK_VAP_ID                      | uint8_t                                   | Specifies the interface on which the socket will operate                                                                   |
 *   | @ref SL_SI91X_SO_CERT_INDEX                       | uint8_t                                   | Certificate index                                                                                                          |
 *   | @ref SL_SI91X_SO_TLS_SNI                          | sl_si91x_socket_type_length_value_t       | Server Name Indication (SNI)                                                                                               |
 *   | @ref SL_SI91X_SO_TLS_ALPN                         | sl_si91x_socket_type_length_value_t       | Application-Layer Protocol Negotiation (ALPN)                                                                              |
 *   | @ref SL_SI91X_SO_MAX_RETRANSMISSION_TIMEOUT_VALUE | uint8_t                                   | Maximum retransmission timeout value for TCP                                                                               |
 *
 * @param[in] option_len 
 *   The length of the parameter of type @ref socklen_t.
 *
 * @return 
 *   Returns 0 on success, or -1 on failure.
 * 
 * @note
 * This function is used only for the SiWx91x socket API.
 * The options set in this function will not be effective if called after `sl_si91x_connect()` or `sl_si91x_listen()` for TCP, or after `sl_si91x_sendto()`, `sl_si91x_recvfrom()`, or `sl_si91x_connect()` for UDP.
 * The value of the option SL_SI91X_SO_MAX_RETRANSMISSION_TIMEOUT_VALUE should be a power of 2 between 1 and 128.
 */
int sl_si91x_setsockopt(int32_t socket, int level, int option_name, const void *option_value, socklen_t option_len);

/**
 * @brief Assigns a local protocol address to a socket.
 *
 * @details
 * The function binds a socket to a specific local address and port number.
 * It is typically used on the server side to specify the port on which the server
 * will listen for incoming connections.
 *
 * @param[in] socket 
 *   The socket ID or file descriptor for the specified socket.
 *
 * @param[in] addr 
 *   Pointer to a `struct sockaddr` contains the address to which the socket is bound.
 *   This address specifies the local IP address and port number.
 *
 * @param[in] addr_len 
 *   The length of the socket address, in bytes, of type `socklen_t`.
 *
 * @return 
 *   Returns 0 on success, or -1 on failure.
 */
int sl_si91x_bind(int socket, const struct sockaddr *addr, socklen_t addr_len);

/**
 * @brief Enables a socket to listen for remote connection requests in passive mode.
 *
 * @details
 * The function configures a socket to listen for incoming connection requests.
 * It is typically used on the server side after the socket has been bound to a local
 * address using the `sl_si91x_bind` function. The socket enters passive mode,
 * where it waits for remote clients to connect.
 *
 * @param[in] socket 
 *   The socket ID or file descriptor for the specified socket.
 *
 * @param[in] max_number_of_clients 
 *   The maximum number of pending connections which the socket can queue.
 *
 * @return 
 *   Returns 0 on success, or -1 on failure.
 */
int sl_si91x_listen(int socket, int max_number_of_clients);

/**
 * @brief Accepts a connection request from a remote peer. 
 *
 * @details 
 * The function blocks until a client attempts to connect to the server socket. After receiving a connection request, it proceeds.
 *
 * @param[in] socket The socket ID or file descriptor for the specified socket.
 * @param[in] addr The address of type @ref sockaddr to which datagrams are to be sent.
 * @param[in] addr_len The length of the socket address of type @ref socklen_t in bytes.
 * @return int 
 */
int sl_si91x_accept(int socket, const struct sockaddr *addr, socklen_t addr_len);

/**
 * @brief 
 *  Accepts a connection request from the remote peer and registers a callback.
 * 
 * @details
 *  The function sets up the server socket to listen for incoming connections,
 *  and immediately returns without blocking the main program's execution. 
 * 
 * @param[in] socket 
 *  The socket ID or file descriptor for the specified socket.
 * @param[in] callback 
 *  A function pointer of type @ref sl_si91x_socket_accept_callback_t that is called when a new client is connected to the server.
 * @return int 
 */
int sl_si91x_accept_async(int socket, sl_si91x_socket_accept_callback_t callback);

/**
 * @brief 
 * Initiates a connection to a remote socket specified by the addr parameter.
 * 
 * @details
 * The function initiates a connection to a remote socket specified by the `addr` parameter.
 * It is typically used on the client side to establish a connection to a server.
 * 
 * @param[in] socket 
 *  The socket ID or file descriptor for the specified socket.
 * @param[in] addr 
 *  Address of type @ref sockaddr to which datagrams are to be sent.
 * @param[in] addr_len
 *  Length of the socket address of type @ref socklen_t in bytes.
 * @return int 
 */
int sl_si91x_connect(int socket, const struct sockaddr *addr, socklen_t addr_len);

/**
 * @brief 
 * Sends the data to the remote peer on the given socket. 
 * 
 * @details
 * This should be used only when the socket is in a connected state.
 * 
 * @param[in] socket 
 * The socket ID or file descriptor for the specified socket.
 * @param[in] buffer 
 * Pointer to the buffer containing data to send to the remote peer.
 * @param[in] buffer_length 
 *  Length of the buffer pointed to by the buffer parameter.
 * @param[in] flags 
 *  Controls the transmission of the data.
 * @return int 
 * @note The flags parameter is not currently supported.
 * @note For TCP, the maximum buffer length should not exceed the MSS.
 * @note The following table lists the maximum buffer length which could be sent over each supported protocol.
 *  
 *  Protocol | Maximum data chunk (bytes)
 *  ---------|----------------------
 *  UDP      | 1472 bytes
 *  TCP      | 1460 bytes
 *  TLS      | 1370 bytes
 */
int sl_si91x_send(int socket, const uint8_t *buffer, size_t buffer_length, int32_t flags);

/**
 * @brief 
 * Transmits one or more messages to a socket asynchronously. 
 * 
 * @details
 * This should be used only when the socket is in a connected state.
 * 
 * @param[in] socket 
 * The socket ID or file descriptor for the specified socket.
 * @param[in] buffer 
 * Pointer to the buffer containing data to send to the remote peer
 * @param[in] buffer_length 
 *  Length of the buffer pointed to by the buffer parameter.
 * @param[in] flags 
 *  Controls the transmission of the data.
 * @param[in] callback 
 *  A function pointer of type @ref sl_si91x_socket_data_transfer_complete_handler_t that is called after complete data transfer.
 * @return int 
 * @note The flags parameter is not currently supported.
 * @note For TCP, the maximum buffer length should not exceed the MSS.
 * @note The following table lists the maximum buffer length which could be sent over each supported protocol.
 *  
 *  Protocol | Maximum data chunk (bytes)
 *  ---------|----------------------
 *  UDP      | 1472 bytes
 *  TCP      | 1460 bytes
 *  TLS      | 1370 bytes
 */
int sl_si91x_send_async(int socket,
                        const uint8_t *buffer,
                        size_t buffer_length,
                        int32_t flags,
                        sl_si91x_socket_data_transfer_complete_handler_t callback);

/**
 * @brief 
 * Transmits one or more messages to another socket. 
 * 
 * @details
 * The function is called from an unconnected socket, typically like a UDP socket.
 * 
 * @param[in] socket 
 * The socket ID or file descriptor for the specified socket.
 * @param[in] buffer 
 *  Pointer to data buffer contains data to send to remote peer.
 * @param[in] buffer_length 
 *  Length of the buffer pointed to by the buffer parameter.
 * @param[in] flags 
 *  Controls the transmission of the data.
 * @param[in] addr 
 *  Address of type @ref sockaddr to which datagrams are to be sent.
 * @param[in] addr_len
 *  Length of the socket address of type @ref socklen_t in bytes.
 * @return int 
 * @note The flags parameter is not currently supported.
 * @note For TCP, the maximum buffer length should not exceed the MSS.
 * @note The following table lists the maximum buffer length which could be sent over each supported protocol.
 *  
 *  Protocol | Maximum data chunk (bytes)
 *  ---------|----------------------
 *  UDP      | 1472 bytes
 *  TCP      | 1460 bytes
 *  TLS      | 1370 bytes
 */
int sl_si91x_sendto(int socket,
                    const uint8_t *buffer,
                    size_t buffer_length,
                    int32_t flags,
                    const struct sockaddr *addr,
                    socklen_t addr_len);

/**
 * @brief 
 * Transmits one or more messages to another socket asynchronously, and receives acknowledgement through the registered callback.
 * 
 * @details
 * The function can also be called from an unconnected socket, typically like a UDP socket.
 * 
 * @param[in] socket 
 * The socket ID or file descriptor for the specified socket.
 * @param[in] buffer 
 *  Pointer to data buffer contains data to send to remote peer.
 * @param[in] buffer_length 
 *  Length of the buffer pointed to by the buffer parameter. 
 * @param[in] flags 
 *  Controls the transmission of the data.
 * @param[in] to_addr 
 *  Address of type @ref sockaddr to which datagrams are to be sent.
 * @param[in] to_addr_len
 *  Length of the socket address of type @ref socklen_t in bytes.
 * @param[in] callback 
 *  A function pointer of type @ref sl_si91x_socket_data_transfer_complete_handler_t that is called after complete data transfer.
 * @return int 
 * @note The flags parameter is not currently supported.
 * @note For TCP, the maximum buffer length should not exceed the MSS.
 * @note The following table lists the maximum buffer length which could be sent over each supported protocol.
 *  
 *  Protocol | Maximum data chunk (bytes)
 *  ---------|----------------------
 *  UDP      | 1472 bytes
 *  TCP      | 1460 bytes
 *  TLS      | 1370 bytes
 */
int sl_si91x_sendto_async(int socket,
                          const uint8_t *buffer,
                          size_t buffer_length,
                          int32_t flags,
                          const struct sockaddr *to_addr,
                          socklen_t to_addr_len,
                          sl_si91x_socket_data_transfer_complete_handler_t callback);

/**
 * @brief Sends data that is larger than the Maximum Segment Size (MSS).
 *
 * @details
 * This function sends data that exceeds the MSS size to a remote peer. It handles
 * the segmentation of the data into smaller chunks that fit within the MSS limit.
 * This API can even be used when the buffer length is less than the MSS.
 *
 * @param[in] socket 
 *   The socket ID or file descriptor for the specified socket.
 *
 * @param[in] buffer 
 *   Pointer to the data buffer contains the data to be sent to the remote peer.
 *
 * @param[in] buffer_length 
 *   The length of the buffer pointed to by the buffer parameter.
 *
 * @param[in] flags 
 *   Controls the transmission of the data. Note that the flags parameter is not currently supported.
 *
 * @return 
 *   Returns the number of bytes sent on success, or -1 on failure.
 */
int sl_si91x_send_large_data(int socket, const uint8_t *buffer, size_t buffer_length, int32_t flags);

/**
 * @brief Receives data from a connected socket.
 *
 * @details
 * This function receives data from a connected socket and stores it in the specified buffer.
 * It is typically used on the client or server side to read incoming data from a remote peer.
 *
 * If the incoming TCP payload exceeds the provided buffer, only the first bufferLength bytes are returned.
 * Any remaining bytes will be delivered by subsequent calls to `sl_si91x_recv()`.
 * The max buffer length for each supported protocol is as follows:
 *   - IPv4: 1460 bytes
 *   - IPv6: 1440 bytes
 *
 * @param[in] socket
 *   The socket ID or file descriptor for the specified socket.
 *
 * @param[out] buffer 
 *   Pointer to the buffer holds the data received from the remote peer.
 *
 * @param[in] bufferLength 
 *   The length of the buffer pointed to by the buffer parameter.
 *
 * @param[in] flags 
 *   Controls the reception of the data. Note that the flags parameter are not currently supported.
 *
 * @return 
 *   Returns the number of bytes received on success, or -1 on failure.
 */
int sl_si91x_recv(int socket, uint8_t *buffer, size_t bufferLength, int32_t flags);

/**
 * @brief Receives data from an unconnected socket, typically a UDP socket.
 *
 * @details
 * This function receives data from an unconnected socket and stores it in the specified buffer.
 * It is typically used to receive data from a remote peer without establishing a connection.
 *
 * If the payload of an incoming UDP datagram exceeds the size of the user-provided buffer, the sl_si91x_recvfrom() function returns only the first buffersize bytes.
 * The remaining portion of the same datagram is delivered in subsequent calls, effectively splitting the datagram into chunks.
 * To ensure the entire datagram is received in a single call, the buffer should be sized as follows:
 *   - IPv4: 1472 bytes
 *   - IPv6: 1452 bytes
 *
 * @param[in] socket 
 *   The socket ID or file descriptor for the specified socket.
 *
 * @param[out] buffer 
 *   Pointer to the buffer that will hold the data received from the remote peer.
 *
 * @param[in] buffersize 
 *   The size of the buffer pointed to by the buffer parameter.
 *
 * @param[in] flags 
 *   Controls the reception of the data. Note that the flags parameter is not currently supported.
 *
 * @param[out] fromAddr 
 *   Pointer to a @ref sockaddr that will hold the address of the remote peer from which the current packet was received.
 *
 * @param[in, out] fromAddrLen 
 *   Pointer to a @ref socklen_t that contains the length of the remote peer address (fromAddr). 
 *   On return, it will contain the actual length of the address.
 *
 * @return 
 *   Returns the number of bytes received on success, or -1 on failure.
 */
int sl_si91x_recvfrom(int socket,
                      uint8_t *buffer,
                      size_t buffersize,
                      int32_t flags,
                      struct sockaddr *fromAddr,
                      socklen_t *fromAddrLen);

/**
 * @brief Disables send or receive operations on a socket.
 *
 * @details
 * This function disables further send or receive operations on a specified socket.
 * It can either close a specific socket or all sockets associated with a given port number.
 *
 * @param[in] socket
 *   The socket ID or file descriptor for the specified socket that is to be closed.
 *
 * @param[in] how
 *   Determines the scope of the shutdown operation: (@ref SI91X_SOCKET_SHUTDOWN_OPTION)
 *   - 0: Close the specified socket.
 *   - 1: Close all sockets open on the specified socket's source port number.
 *
 * @return 
 *   Returns 0 on success, or -1 on failure.
 *
 * @note
 *   If the socket is a server socket, the `how` parameter is ignored, and the socket is always closed based on the port number.
 */
int sl_si91x_shutdown(int socket, int how);

/**
 * @brief 
 * The sl_si91x_select() function is used to monitor multiple file descriptors for readiness to
 * perform I/O operations.  The file descriptors in the sets are monitored to
 * see if they are ready for reading, ready for writing, or have an error
 * condition pending.  
 * @details 
 * sl_si91x_select() allows a program to monitor multiple file descriptors, 
 * waiting until one or more of the file descriptors become "ready" 
 * for some class of I/O operation (e.g., input possible).  A file 
 * descriptor is considered ready if it is possible to perform a 
 * corresponding I/O operation without blocking. 
 * 
 * @param[in] nfds 
 *  The first nfds descriptors are checked in each set; that is, the descriptors from 0 through nfds-1.
 * @param[in,out] readfds 
 *  A pointer to a fd_set object that specifies the descriptors to check for files that are ready for reading.
 * @param[in,out] writefds 
 *  A pointer to a fd_set object that specifies the descriptors to check for files that are ready for writing.
 * @param[in,out] exceptfds 
 *  A pointer to a fd_set object that will be watched for exceptions.
 * @param[in] timeout 
 *  If timeout is provided, the device shall wait for timeout duration for the file descriptors to become ready.
 *  If timeout is NULL, the device shall wait indefinitely for the file descriptors to become ready.
 * @param[in] callback 
 *  A function pointer of type @ref sl_si91x_socket_select_callback_t that will be called when an asynchronous response  is received for a select request.
 * @return 
 *  If callback is provided, the function will immediately return zero for success, and -1 for failure.
 *  If callback is NULL, returns:
 *  - total number of file descriptors set on success.
 *  - 0 when no file descriptors are ready within the specified timeout.
 *  - -1 on failure.
 * 
 * @note 
 * The select function modifies the sets passed to it, so if the function
 * is to be called again, the sets must be reinitialized.
 * The exceptfds parameter is not currently supported.
 * @note 
 * If the number of select requests is not configured, the sl_si91x_select() API will fail and return -1, with the errno being set to EPERM (Operation not permitted).
 * @note 
 * The number of select operations the device can handle can be configured using the [SL_SI91X_EXT_TCP_IP_TOTAL_SELECTS](../wiseconnect-api-reference-guide-si91x-driver/si91-x-extended-tcp-ip-feature-bitmap#sl-si91-x-ext-tcp-ip-total-selects).
 * @note
 * The sl_si91x_select() function has the following limitations:
 *   - Supports a maximum of 10 sockets per call.
 *   - Supports read and write fd_sets only. It does not monitor the exceptfds set.
 *   - Operates only with connected sockets. Listening sockets are not supported.
 */
#ifndef __ZEPHYR__
int sl_si91x_select(int nfds,
                    fd_set *readfds,
                    fd_set *writefds,
                    fd_set *exceptfds,
                    const struct timeval *timeout,
                    sl_si91x_socket_select_callback_t callback);
#else
int sl_si91x_select(int nfds,
                    sl_si91x_fdset_t *readfds,
                    sl_si91x_fdset_t *writefds,
                    sl_si91x_fdset_t *exceptfds,
                    const struct timeval *timeout,
                    sl_si91x_socket_select_callback_t callback);
#endif

/**
 * @brief Registers a callback for remote socket termination events.
 *
 * @details
 * This function registers a callback function is called when a remote socket is terminated.
 * The callback function should be of type @ref sl_si91x_socket_remote_termination_callback_t.
 *
 * @param[in] callback
 *   A valid function pointer of type @ref sl_si91x_socket_remote_termination_callback_t that is called when the remote socket is terminated.
 */
void sl_si91x_set_remote_termination_callback(sl_si91x_socket_remote_termination_callback_t callback);
/** @} */
#ifdef __cplusplus
}
#endif
