/*
 * Copyright (c) 2018-2022 Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * $Date:        27. January 2020
 * $Revision:    V1.2.0
 *
 * Project:      IoT Socket API definitions
 *
 * Version 1.2.0
 *   Extended iotSocketRecv/RecvFrom/Send/SendTo (support for polling)
 * Version 1.1.0
 *   Added function iotSocketRecvFrom
 *   Added function iotSocketSendTo
 *   Added function iotSocketGetSockName
 *   Added function iotSocketGetPeerName
 *   Added function iotSocketGetOpt
 *   Removed function iotSocketGetStatus
 *   Changed IP address pointer type to uint8_t *
 *   Added socket option SO_KEEPALIVE and SO_TYPE
 *   Removed socket option SO_REUSEADDR
 * Version 1.0.0
 *   Initial Release
 */

#ifndef IOT_SOCKET_H
#define IOT_SOCKET_H

#ifdef  __cplusplus
extern "C"
{
#endif

#include <stdint.h>

/**
 * @addtogroup IOT_SOCKET_ADDRESS_FAMILY Address Family
 * @ingroup IOT_SOCKET_FUNCTIONS
 * @{ 
 */
/**** Address Family definitions ****/
#define IOT_SOCKET_AF_INET              1       ///< Internet Protocol version 4 (IPv4) address family.
#define IOT_SOCKET_AF_INET6             2       ///< Internet Protocol version 6 (IPv6) address family.
/** @} */

/**
 * @addtogroup IOT_SOCKET_TYPE Socket Type
 * @ingroup IOT_SOCKET_FUNCTIONS
 * @{ 
 */
/**** Socket Type definitions ****/
#define IOT_SOCKET_SOCK_STREAM          1       ///< Stream socket type. Provides sequenced, reliable, two-way, connection-based byte streams.
#define IOT_SOCKET_SOCK_DGRAM           2       ///< Datagram socket type. Supports datagrams, which are connectionless, unreliable messages of a fixed maximum length.
/** @} */

/**
 * @addtogroup IOT_SOCKET_PROTOCOL Socket Protocol
 * @ingroup IOT_SOCKET_FUNCTIONS
 * @{ 
 */
/**** Socket Protocol definitions ****/
#define IOT_SOCKET_IPPROTO_TCP          1       ///< Transmission Control Protocol (TCP). This is a reliable, stream-oriented protocol.
#define IOT_SOCKET_IPPROTO_UDP          2       ///< User Datagram Protocol (UDP). This is a connectionless, unreliable, datagram protocol.
/** @} */

/**
 * @addtogroup IOT_SOCKET_OPTION_ID Socket Option Id
 * @ingroup IOT_SOCKET_FUNCTIONS
 * @{ 
 */
/**** Socket Option definitions ****/
#define IOT_SOCKET_IO_FIONBIO           1       ///< Non-blocking I/O (Set only, default = 0); opt_val = &nbio, opt_len = sizeof(nbio), nbio (integer): 0=blocking, non-blocking otherwise
#define IOT_SOCKET_SO_RCVTIMEO          2       ///< Receive timeout in ms (default = 0); opt_val = &timeout, opt_len = sizeof(timeout)
#define IOT_SOCKET_SO_SNDTIMEO          3       ///< Send timeout in ms (default = 0); opt_val = &timeout, opt_len = sizeof(timeout)
#define IOT_SOCKET_SO_KEEPALIVE         4       ///< Keep-alive messages (default = 0); opt_val = &keepalive, opt_len = sizeof(keepalive), keepalive (integer): 0=disabled, enabled otherwise
#define IOT_SOCKET_SO_TYPE              5       ///< Socket Type (Get only); opt_val = &socket_type, opt_len = sizeof(socket_type), socket_type (integer): IOT_SOCKET_SOCK_xxx
/** @} */

/**
 * @addtogroup IOT_SOCKET_RETURN_CODES Socket Return Codes
 * @ingroup IOT_SOCKET_FUNCTIONS
 * @{ 
 */
/**** Socket Return Codes ****/
#define IOT_SOCKET_ERROR                (-1)    ///< General error code for unspecified errors.
#define IOT_SOCKET_ESOCK                (-2)    ///< Error code indicating an invalid socket.
#define IOT_SOCKET_EINVAL               (-3)    ///< Error code indicating an invalid argument.
#define IOT_SOCKET_ENOTSUP              (-4)    ///< Error code indicating that the requested operation is not supported.
#define IOT_SOCKET_ENOMEM               (-5)    ///< Error code indicating that there is not enough memory to perform the requested operation.
#define IOT_SOCKET_EAGAIN               (-6)    ///< Error code indicating that the operation would block or has timed out.
#define IOT_SOCKET_EINPROGRESS          (-7)    ///< Error code indicating that the operation is in progress.
#define IOT_SOCKET_ETIMEDOUT            (-8)    ///< Error code indicating that the operation has timed out.
#define IOT_SOCKET_EISCONN              (-9)    ///< Error code indicating that the socket is already connected.
#define IOT_SOCKET_ENOTCONN             (-10)   ///< Error code indicating that the socket is not connected.
#define IOT_SOCKET_ECONNREFUSED         (-11)   ///< Error code indicating that the connection was refused by the peer.
#define IOT_SOCKET_ECONNRESET           (-12)   ///< Error code indicating that the connection was reset by the peer.
#define IOT_SOCKET_ECONNABORTED         (-13)   ///< Error code indicating that the connection was aborted locally.
#define IOT_SOCKET_EALREADY             (-14)   ///< Error code indicating that a connection is already in progress.
#define IOT_SOCKET_EADDRINUSE           (-15)   ///< Error code indicating that the address is already in use.
#define IOT_SOCKET_EHOSTNOTFOUND        (-16)   ///< Error code indicating that the host was not found.
/** @} */

/**
 * @addtogroup IOT_SOCKET_FUNCTIONS
 * @{ 
 */

/***************************************************************************/ 
/**
 * @brief
 *   Create a communication socket.
 * 
 * @details
 *   The @ref iotSocketCreate() function creates a new socket that is used for communication. 
 *   The socket is created with the specified address family, socket type, and protocol.
 * 
 * @param[in] af
 *   Address family. One of the values from @ref IOT_SOCKET_ADDRESS_FAMILY.
 * 
 * @param[in] type
 *   Socket type. One of the values from @ref IOT_SOCKET_TYPE.
 * 
 * @param[in] protocol
 *   Socket protocol. One of the values from @ref IOT_SOCKET_PROTOCOL.
 * 
 * @return
 *   Returns the socket identification number (>=0) on success. On failure, returns one of the following error codes:
 *   - @ref IOT_SOCKET_EINVAL : Invalid argument.
 *   - @ref IOT_SOCKET_ENOTSUP : Operation not supported.
 *   - @ref IOT_SOCKET_ENOMEM : Not enough memory.
 *   - @ref IOT_SOCKET_ERROR : Unspecified error.
 ******************************************************************************/
extern int32_t iotSocketCreate(int32_t af, int32_t type, int32_t protocol);

/***************************************************************************/ 
/**
 * @brief
 *   Assign a local address to a socket.
 * 
 * @details
 *   The @ref iotSocketBind() function assigns a local IP address and port number to a socket identified by `socket`. 
 *   This function is typically used on the server side to bind a socket to a specific IP address and port number, 
 *   so that the socket can listen for incoming connections or datagrams.
 * 
 * @param[in] socket
 *   Socket identification number.
 * 
 * @param[in] ip
 *   Pointer to the local IP address. The address should be in network byte order.
 * 
 * @param[in] ip_len
 *   Length of the `ip` address in bytes.
 * 
 * @param[in] port
 *   Local port number. The port number should be in host byte order.
 * 
 * @return
 *   Returns 0 on success. On failure, returns one of the following error codes:
 *   - @ref IOT_SOCKET_ESOCK : Invalid socket.
 *   - @ref IOT_SOCKET_EINVAL : Invalid argument (address or socket already bound).
 *   - @ref IOT_SOCKET_EADDRINUSE : Address already in use.
 *   - @ref IOT_SOCKET_ERROR : Unspecified error.
 ******************************************************************************/
extern int32_t iotSocketBind(int32_t socket, const uint8_t *ip, uint32_t ip_len, uint16_t port);

/***************************************************************************/ 
/**
 * @brief
 *   Listen for socket connections.
 * 
 * @details
 *   The @ref iotSocketListen() function marks the socket referred to by `socket` as a passive socket, 
 *   that is, as a socket that will be used to accept incoming connection requests using @ref iotSocketAccept(). 
 *   The `backlog` parameter defines the maximum length to which the queue of pending connections for `socket` may grow.
 * 
 * @param[in] socket
 *   Socket identification number.
 * 
 * @param[in] backlog
 *   Maximum number of pending connections that can be queued.
 * 
 * @return
 *   Returns 0 on success. On failure, returns one of the following error codes:
 *   - @ref IOT_SOCKET_ESOCK : Invalid socket.
 *   - @ref IOT_SOCKET_EINVAL : Invalid argument (socket not bound).
 *   - @ref IOT_SOCKET_ENOTSUP : Operation not supported.
 *   - @ref IOT_SOCKET_EISCONN : Socket is already connected.
 *   - @ref IOT_SOCKET_ERROR : Unspecified error.
 ******************************************************************************/
extern int32_t iotSocketListen(int32_t socket, int32_t backlog);

/***************************************************************************/ 
/**
 * @brief
 *   Accept a new connection on a socket.
 * 
 * @details
 *   The @ref iotSocketAccept() function is used to accept a new incoming connection on a socket that is in listening mode. 
 *   It extracts the first connection request on the queue of pending connections for the listening socket, 
 *   `socket`, creates a new connected socket, and returns a new file descriptor referring to that socket. 
 *   The newly created socket is not in the listening state. The original socket remains open and can continue 
 *   to accept new connections.
 * 
 * @param[in] socket
 *   Socket identification number.
 * 
 * @param[out] ip
 *   Pointer to a buffer where the address of the connecting socket will be returned. If NULL, no address is returned.
 * 
 * @param[in, out] ip_len
 *   Pointer to the length of the `ip` buffer. On input, it should contain the size of the buffer. On output, 
 *   it will be modified to indicate the actual length of the address returned. If `ip` is NULL, `ip_len` should also be NULL.
 * 
 * @param[out] port
 *   Pointer to a buffer where the port of the connecting socket will be returned. If NULL, no port is returned.
 * 
 * @return
 *   Returns the socket identification number of the accepted socket (>=0) on success. On failure, returns one of the following error codes:
 *   - @ref IOT_SOCKET_ESOCK : Invalid socket.
 *   - @ref IOT_SOCKET_EINVAL : Invalid argument (socket not in listen mode).
 *   - @ref IOT_SOCKET_ENOTSUP : Operation not supported (socket type does not support accepting connections).
 *   - @ref IOT_SOCKET_ECONNRESET : Connection reset by the peer.
 *   - @ref IOT_SOCKET_ECONNABORTED : Connection aborted locally.
 *   - @ref IOT_SOCKET_EAGAIN : Operation would block or timed out (may be called again).
 *   - @ref IOT_SOCKET_ERROR : Unspecified error.
 ******************************************************************************/
extern int32_t iotSocketAccept(int32_t socket, uint8_t *ip, uint32_t *ip_len, uint16_t *port);

/***************************************************************************/ 
/**
 * @brief
 *   Connect a socket to a remote host.
 * 
 * @details
 *   The @ref iotSocketConnect() function establishes a connection to a remote host specified by the IP address and port number. 
 *   This function is used for connection-oriented protocols such as TCP. The socket identified by `socket` must be created 
 *   and optionally bound to a local address before calling this function.
 * 
 * @param[in] socket
 *   Socket identification number.
 * 
 * @param[in] ip
 *   Pointer to the remote IP address.
 * 
 * @param[in] ip_len
 *   Length of the `ip` address in bytes.
 * 
 * @param[in] port
 *   Remote port number.
 * 
 * @return
 *   Returns 0 on success. On failure, returns one of the following error codes:
 *   - @ref IOT_SOCKET_ESOCK : Invalid socket.
 *   - @ref IOT_SOCKET_EINVAL : Invalid argument.
 *   - @ref IOT_SOCKET_EALREADY : Connection already in progress.
 *   - @ref IOT_SOCKET_EINPROGRESS : Operation in progress.
 *   - @ref IOT_SOCKET_EISCONN : Socket is already connected.
 *   - @ref IOT_SOCKET_ECONNREFUSED : Connection rejected by the peer.
 *   - @ref IOT_SOCKET_ECONNABORTED : Connection aborted locally.
 *   - @ref IOT_SOCKET_EADDRINUSE : Address already in use.
 *   - @ref IOT_SOCKET_ETIMEDOUT : Operation timed out.
 *   - @ref IOT_SOCKET_ERROR : Unspecified error.
 ******************************************************************************/
extern int32_t iotSocketConnect(int32_t socket, const uint8_t *ip, uint32_t ip_len, uint16_t port);

/***************************************************************************/ 
/**
 * @brief
 *   Receive data from a socket.
 * 
 * @details
 *   The @ref iotSocketRecv() function receives data from a connected socket identified by `socket`. 
 *   The received data is stored in the buffer pointed to by `buf`, which has a length of `len` bytes.
 * 
 * @param[in] socket
 *   Socket identification number.
 * 
 * @param[out] buf
 *   Pointer to the buffer where the received data should be stored.
 * 
 * @param[in] len
 *   Length of the buffer in bytes.
 * 
 * @return
 *   Returns the number of bytes received (>=0) on success. On failure, returns one of the following error codes:
 *   - 0: Data is available (when `len` is 0).
 *   - @ref IOT_SOCKET_ESOCK : Invalid socket.
 *   - @ref IOT_SOCKET_EINVAL : Invalid argument (pointer to buffer or length).
 *   - @ref IOT_SOCKET_ENOTCONN : Socket is not connected.
 *   - @ref IOT_SOCKET_ECONNRESET : Connection reset by the peer.
 *   - @ref IOT_SOCKET_ECONNABORTED : Connection aborted locally.
 *   - @ref IOT_SOCKET_EAGAIN : Operation would block or timed out (may be called again).
 *   - @ref IOT_SOCKET_ERROR : Unspecified error.
 ******************************************************************************/
extern int32_t iotSocketRecv(int32_t socket, void *buf, uint32_t len);

/***************************************************************************/ 
/**
 * @brief
 *   Receive data from a socket.
 * 
 * @details
 *   The @ref iotSocketRecvFrom() function receives data from a socket identified by `socket`. 
 *   The received data is stored in the buffer pointed to by `buf`, which has a length of `len` bytes. 
 *   If the `ip` and `port` parameters are provided, the function also returns the source address and port 
 *   of the received data.
 * 
 * @param[in] socket
 *   Socket identification number.
 * 
 * @param[out] buf
 *   Pointer to the buffer where the received data should be stored.
 * 
 * @param[in] len
 *   Length of the buffer in bytes.
 * 
 * @param[out] ip
 *   Pointer to a buffer where the remote source address will be returned. If NULL, no address is returned.
 * 
 * @param[in, out] ip_len
 *   Pointer to the length of the `ip` buffer. On input, it should contain the size of the buffer. On output, 
 *   it will be modified to indicate the actual length of the address returned. If `ip` is NULL, `ip_len` should also be NULL.
 * 
 * @param[out] port
 *   Pointer to a buffer where the remote source port will be returned. If NULL, no port is returned.
 * 
 * @return
 *   Returns the number of bytes received (>=0) on success. On failure, returns one of the following error codes:
 *   - 0: Data is available (when `len` is 0).
 *   - @ref IOT_SOCKET_ESOCK : Invalid socket.
 *   - @ref IOT_SOCKET_EINVAL : Invalid argument (pointer to buffer or length).
 *   - @ref IOT_SOCKET_ENOTCONN : Socket is not connected.
 *   - @ref IOT_SOCKET_ECONNRESET : Connection reset by the peer.
 *   - @ref IOT_SOCKET_ECONNABORTED : Connection aborted locally.
 *   - @ref IOT_SOCKET_EAGAIN : Operation would block or timed out (may be called again).
 *   - @ref IOT_SOCKET_ERROR : Unspecified error.
 ******************************************************************************/
extern int32_t iotSocketRecvFrom(int32_t socket, void *buf, uint32_t len, uint8_t *ip, uint32_t *ip_len, uint16_t *port);

/***************************************************************************/ 
/**
 * @brief
 *   Send data or check if data can be sent on a connected socket.
 * 
 * @details
 *   The @ref iotSocketSend() function sends data from the buffer pointed to by `buf` through the socket identified by `socket`. 
 *   If `len` is 0, the function checks if data can be sent on the socket without blocking.
 * 
 * @param[in] socket
 *   Socket identification number.
 * 
 * @param[in] buf
 *   Pointer to the buffer containing the data to send.
 * 
 * @param[in] len
 *   Length of the data in bytes.
 * 
 * @return
 *   Returns the number of bytes sent (>=0) on success if `len` is not 0. If `len` is 0, returns 0 if data can be sent. 
 *   On failure, returns one of the following error codes:
 *   - @ref IOT_SOCKET_ESOCK : Invalid socket.
 *   - @ref IOT_SOCKET_EINVAL : Invalid argument (pointer to buffer or length).
 *   - @ref IOT_SOCKET_ENOTCONN : Socket is not connected.
 *   - @ref IOT_SOCKET_ECONNRESET : Connection reset by the peer.
 *   - @ref IOT_SOCKET_ECONNABORTED : Connection aborted locally.
 *   - @ref IOT_SOCKET_EAGAIN : Operation would block or timed out (may be called again).
 *   - @ref IOT_SOCKET_ERROR : Unspecified error.
 * 
 * @note
 *   - The function does not guarantee that the packets are transmitted to the remote node; they are enqueued in the queue.
 *   - The function can only send a maximum of 1460 bytes in the case of plain TCP and UDP. For TLS, the maximum buffer length is 1370 bytes.
 ******************************************************************************/
extern int32_t iotSocketSend(int32_t socket, const void *buf, uint32_t len);

/***************************************************************************/ 
/**
 * @brief
 *   Send data or check if data can be sent on a socket.
 * 
 * @details
 *   The @ref iotSocketSendTo() function sends data from the buffer pointed to by `buf` through the socket identified by `socket` 
 *   to the specified remote destination IP address and port. If `len` is 0, the function checks if data can be sent on the socket 
 *   without blocking.
 * 
 * @param[in] socket
 *   Socket identification number.
 * 
 * @param[in] buf
 *   Pointer to the buffer containing the data to send.
 * 
 * @param[in] len
 *   Length of the data in bytes.
 * 
 * @param[in] ip
 *   Pointer to the remote destination IP address.
 * 
 * @param[in] ip_len
 *   Length of the `ip` address in bytes.
 * 
 * @param[in] port
 *   Remote destination port number.
 * 
 * @return
 *   Returns the number of bytes sent (>=0) on success if `len` is not 0. If `len` is 0, returns 0 if data can be sent. 
 *   On failure, returns one of the following error codes:
 *   - @ref IOT_SOCKET_ESOCK : Invalid socket.
 *   - @ref IOT_SOCKET_EINVAL : Invalid argument (pointer to buffer or length).
 *   - @ref IOT_SOCKET_ENOTCONN : Socket is not connected.
 *   - @ref IOT_SOCKET_ECONNRESET : Connection reset by the peer.
 *   - @ref IOT_SOCKET_ECONNABORTED : Connection aborted locally.
 *   - @ref IOT_SOCKET_EAGAIN : Operation would block or timed out (may be called again).
 *   - @ref IOT_SOCKET_ERROR : Unspecified error.
 * 
 * @note
 *   - If the number of bytes to be sent exceeds the Maximum Segment Size (MSS) specified by the remote node, the API will return @ref IOT_SOCKET_ERROR.
 *     To know the MSS size for the socket, use the `sl_si91x_get_socket_mss()` utility. In the case of TCP, is should be called after @ref iotSocketConnect().
 *   - The function does not guarantee that the packets are transmitted to the remote node; they are enqueued in the queue.
 *   - The function can only send a maximum of 1460 bytes in the case of plain TCP or UDP. For TLS, the maximum buffer length is 1370 bytes.
 ******************************************************************************/
extern int32_t iotSocketSendTo(int32_t socket, const void *buf, uint32_t len, const uint8_t *ip, uint32_t ip_len, uint16_t port);

/***************************************************************************/ 
/**
 * @brief
 *   Retrieve local IP address and port of a socket.
 * 
 * @details
 *   The @ref iotSocketGetSockName() function retrieves the local IP address and port number associated with the socket 
 *   identified by `socket`. This function is typically used to obtain the local address and port assigned to a socket 
 *   after it has been bound or connected.
 * 
 * @param[in] socket
 *   Socket identification number.
 * 
 * @param[out] ip
 *   Pointer to a buffer where the local IP address will be returned. If NULL, no address is returned.
 * 
 * @param[in, out] ip_len
 *   Pointer to the length of the `ip` buffer. On input, it should contain the size of the buffer. On output, 
 *   it will be modified to indicate the actual length of the address returned. If `ip` is NULL, `ip_len` should also be NULL.
 * 
 * @param[out] port
 *   Pointer to a buffer where the local port number would be returned. If NULL, no port is returned.
 * 
 * @return
 *   Returns 0 on success. On failure, returns one of the following error codes:
 *   - @ref IOT_SOCKET_ESOCK : Invalid socket.
 *   - @ref IOT_SOCKET_EINVAL : Invalid argument (pointer to buffer or length).
 *   - @ref IOT_SOCKET_ERROR : Unspecified error.
 ******************************************************************************/
extern int32_t iotSocketGetSockName(int32_t socket, uint8_t *ip, uint32_t *ip_len, uint16_t *port);

/***************************************************************************/ 
/**
 * @brief
 *   Retrieve remote IP address and port of a socket.
 * 
 * @details
 *   The @ref iotSocketGetPeerName() function retrieves the remote IP address and port number associated with the socket 
 *   identified by `socket`. This function is typically used to obtain the address and port of the peer to which the socket 
 *   is connected.
 * 
 * @param[in] socket
 *   Socket identification number.
 * 
 * @param[out] ip
 *   Pointer to a buffer where the remote IP address would be returned. If NULL, no address is returned.
 * 
 * @param[in, out] ip_len
 *   Pointer to the length of the `ip` buffer. On input, it should contain the size of the buffer. On output, 
 *   it will be modified to indicate the actual length of the address returned. If `ip` is NULL, `ip_len` should also be NULL.
 * 
 * @param[out] port
 *   Pointer to a buffer where the remote port number would be returned. If NULL, no port is returned.
 * 
 * @return
 *   Returns 0 on success. On failure, returns one of the following error codes:
 *   - @ref IOT_SOCKET_ESOCK : Invalid socket.
 *   - @ref IOT_SOCKET_EINVAL : Invalid argument (pointer to buffer or length).
 *   - @ref IOT_SOCKET_ENOTCONN : Socket is not connected.
 *   - @ref IOT_SOCKET_ERROR : Unspecified error.
 ******************************************************************************/
extern int32_t iotSocketGetPeerName(int32_t socket, uint8_t *ip, uint32_t *ip_len, uint16_t *port);

/***************************************************************************/ 
/**
 * @brief
 *   Retrieve socket option.
 * 
 * @details
 *   The @ref iotSocketGetOpt() function retrieves the value of a socket option specified by `opt_id` for the socket 
 *   identified by `socket`. The option value is stored in the buffer pointed to by `opt_val`, and the length of the 
 *   option value is specified by `opt_len`.
 * 
 * @param[in] socket
 *   Socket identification number.
 * 
 * @param[in] opt_id
 *   Option identifier. One of the values from @ref IOT_SOCKET_OPTION_ID.
 * 
 * @param[out] opt_val
 *   Pointer to the buffer that would receive the option value.
 * 
 * @param[in,out] opt_len
 *   Pointer to the length of the option value:
 *   - On input, it specifies the length of the buffer.
 *   - On output, it specifies the length of the data returned.
 * 
 * @return
 *   Returns 0 on success. On failure, returns one of the following error codes:
 *   - @ref IOT_SOCKET_ESOCK : Invalid socket.
 *   - @ref IOT_SOCKET_EINVAL : Invalid argument.
 *   - @ref IOT_SOCKET_ENOTSUP : Operation not supported.
 *   - @ref IOT_SOCKET_ERROR : Unspecified error.
 * 
 * @note
 *   The following options are currently supported:
 *   - @ref IOT_SOCKET_IO_FIONBIO
 *   - @ref IOT_SOCKET_SO_RCVTIMEO
 *   - @ref IOT_SOCKET_SO_SNDTIMEO
 *   - @ref IOT_SOCKET_SO_KEEPALIVE
 *   - @ref IOT_SOCKET_SO_TYPE
 ******************************************************************************/
extern int32_t iotSocketGetOpt(int32_t socket, int32_t opt_id, void *opt_val, uint32_t *opt_len);

/***************************************************************************/ 
/**
 * @brief
 *   Set socket option.
 * 
 * @details
 *   The @ref iotSocketSetOpt() function sets the value of a socket option specified by `opt_id` for the socket 
 *   identified by `socket`. The option value is provided in the buffer pointed to by `opt_val`, and the length of the 
 *   option value is specified by `opt_len`.
 * 
 * @param[in] socket
 *   Socket identification number.
 * 
 * @param[in] opt_id
 *   Option identifier. One of the values from @ref IOT_SOCKET_OPTION_ID.
 * 
 * @param[in] opt_val
 *   Pointer to the buffer containing the option value.
 * 
 * @param[in] opt_len
 *   Length of the option value in bytes.
 * 
 * @return
 *   Returns 0 on success. On failure, returns one of the following error codes:
 *   - @ref IOT_SOCKET_ESOCK : Invalid socket.
 *   - @ref IOT_SOCKET_EINVAL : Invalid argument.
 *   - @ref IOT_SOCKET_ENOTSUP : Operation not supported.
 *   - @ref IOT_SOCKET_ERROR : Unspecified error.
 * 
 * @note
 *   The following options are currently supported:
 *   - @ref IOT_SOCKET_IO_FIONBIO
 *   - @ref IOT_SOCKET_SO_RCVTIMEO
 *   - @ref IOT_SOCKET_SO_SNDTIMEO
 *   - @ref IOT_SOCKET_SO_KEEPALIVE
 *   - @ref IOT_SOCKET_SO_TYPE
 ******************************************************************************/
extern int32_t iotSocketSetOpt(int32_t socket, int32_t opt_id, const void *opt_val, uint32_t opt_len);

/***************************************************************************/ 
/**
 * @brief
 *   Close and release a socket.
 * 
 * @details
 *   The @ref iotSocketClose() function closes the socket identified by `socket` and releases any resources associated with it. 
 *   If the socket is a server socket or the first client socket, closing it may also close other related sockets.
 * 
 * @param[in] socket
 *   Socket identification number.
 * 
 * @return
 *   Returns 0 on success. On failure, returns one of the following error codes:
 *   - @ref IOT_SOCKET_ESOCK : Invalid socket.
 *   - @ref IOT_SOCKET_EAGAIN : Operation would block (may be called again).
 *   - @ref IOT_SOCKET_ERROR : Unspecified error.
 * 
 * @note
 *   Calling close on a server or the first client socket may result in closing other related sockets as well.
 ******************************************************************************/
extern int32_t iotSocketClose(int32_t socket);

/***************************************************************************/ 
/**
 * @brief
 *   Retrieve host IP address from host name.
 * 
 * @details
 *   The @ref iotSocketGetHostByName() function resolves a host name to an IP address. The resolved IP address is stored 
 *   in the buffer pointed to by `ip`, and the length of the IP address is specified by `ip_len`. The address family is 
 *   specified by `af`.
 * 
 * @param[in] name
 *   Host name to be resolved.
 * 
 * @param[in] af
 *   Address family. One of the values from @ref IOT_SOCKET_ADDRESS_FAMILY.
 * 
 * @param[out] ip
 *   Pointer to the buffer where the resolved IP address will be returned.
 * 
 * @param[in,out] ip_len
 *   Pointer to the length of the `ip` buffer:
 *   - On input, it specifies the length of the supplied `ip` buffer.
 *   - On output, it specifies the length of the stored `ip` address.
 * 
 * @return
 *   Returns 0 on success. On failure, returns one of the following error codes:
 *   - @ref IOT_SOCKET_EINVAL : Invalid argument.
 *   - @ref IOT_SOCKET_ENOTSUP : Operation not supported.
 *   - @ref IOT_SOCKET_ETIMEDOUT : Operation timed out.
 *   - @ref IOT_SOCKET_EHOSTNOTFOUND : Host not found.
 *   - @ref IOT_SOCKET_ERROR : Unspecified error.
 ******************************************************************************/
extern int32_t iotSocketGetHostByName(const char *name, int32_t af, uint8_t *ip, uint32_t *ip_len);

/** @} */

#ifdef  __cplusplus
}
#endif

#endif /* IOT_SOCKET_H */
