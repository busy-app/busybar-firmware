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
#include "iot_socket.h"
#include <stdint.h>
#include <string.h>
#include "socket.h"
#include "errno.h"
#include "iot_socket_types.h"
#include "sl_constants.h"
#include "netdb.h"
#include "sl_string.h"

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/

#define DNS_REQUEST_MAX_URL_LEN 90

/******************************************************
 *                   Enumerations
 ******************************************************/

/******************************************************
 *                 Type Definitions
 ******************************************************/

/******************************************************
 *                    Structures
 ******************************************************/

/******************************************************
 *               Static Function Declarations
 ******************************************************/

/******************************************************
 *               Variable Definitions
 ******************************************************/

/******************************************************
 *               Function Definitions
 ******************************************************/
static int32_t sli_si91x_errno_to_rc(void)
{
  int32_t status;

  // Map errno values to IOT_SOCKET error codes
  switch (errno) {
    case EBADF: {
      status = IOT_SOCKET_ESOCK; // Bad file descriptor
      break;
    }
    case ENOBUFS: {
      status = IOT_SOCKET_ESOCK; // No buffer space available
      break;
    }
    case EIO:
    case EFAULT:
    case EINVAL: {
      status = IOT_SOCKET_EINVAL; // Invalid argument or I/O error
      break;
    }
    case ENOMEM: {
      status = IOT_SOCKET_ENOMEM; // Out of memory
      break;
    }
    case EWOULDBLOCK: {
      status = IOT_SOCKET_EAGAIN; // Operation would block
      break;
    }
    case EINPROGRESS: {
      status = IOT_SOCKET_EINPROGRESS; // Operation in progress
      break;
    }
    case ENOTCONN: {
      status = IOT_SOCKET_ENOTCONN; // Socket not connected
      break;
    }
    case EISCONN: {
      status = IOT_SOCKET_EISCONN; // Socket is already connected
      break;
    }
    case ECONNRESET: {
      status = IOT_SOCKET_ECONNRESET; // Connection reset by peer
      break;
    }
    case ECONNABORTED: {
      status = IOT_SOCKET_ECONNABORTED; // Connection aborted
      break;
    }
    case EALREADY: {
      status = IOT_SOCKET_EALREADY; // Operation already in progress
      break;
    }
    case EADDRINUSE: {
      status = IOT_SOCKET_EADDRINUSE; // Address already in use
      break;
    }
    case EHOSTUNREACH: {
      status = IOT_SOCKET_EHOSTNOTFOUND; // Host unreachable
      break;
    }
    case EOPNOTSUPP: {
      status = IOT_SOCKET_ENOTSUP; // Operation not supported
      break;
    }
    default: {
      status = IOT_SOCKET_ERROR; // Other errors
      break;
    }
  }

  return status;
}

static int32_t sli_si91x_construct_remote_host_address(sockaddr_storage_t *addr,
                                             const uint8_t *ip,
                                             uint32_t ip_len,
                                             uint16_t port)
{
  // Check parameters
  if ((addr == NULL) || (ip == NULL) || (port == 0)) {
    return IOT_SOCKET_EINVAL;
  }

  // Check ip length
  if ((ip_len != SL_IPV4_ADDRESS_LENGTH) && (ip_len != SL_IPV6_ADDRESS_LENGTH)) {
    return IOT_SOCKET_EINVAL;
  }

  // Initialize the sockaddr_storage structure
  memset(addr, 0, sizeof(struct sockaddr_in));

  if (ip_len == sizeof(struct in_addr)) {
    struct sockaddr_in *sa = (struct sockaddr_in *)addr;
    sa->sin_len            = sizeof(struct sockaddr_in);
    sa->sin_family         = AF_INET;
    sa->sin_port           = port;
    memcpy(&sa->sin_addr, ip, sizeof(struct in_addr));
    memset(sa->sin_zero, 0, SIN_ZERO_LEN);
    if (sa->sin_addr.s_addr == 0) {
      return IOT_SOCKET_EINVAL; // Invalid IP address
    }
  }
#if defined(SLI_SI91X_ENABLE_IPV6)
  else if (ip_len == sizeof(struct in6_addr)) {
    struct sockaddr_in6 *sa = (struct sockaddr_in6 *)addr;
    sa->sin6_len            = sizeof(struct sockaddr_in6);
    sa->sin6_family         = AF_INET6;
    sa->sin6_port           = port;
    memcpy(&sa->sin6_addr, ip, sizeof(struct in6_addr));
  }
#endif
  else {
    return IOT_SOCKET_EINVAL; // Invalid IP length
  }

  return IOT_SOCKET_NO_ERROR;
}

static int32_t sli_si91x_copy_ip_address_and_port(sockaddr_storage_t addr, uint8_t *ip, uint32_t *ip_len, uint16_t *port)
{
  if ((ip != NULL) && (ip_len != NULL) && (port != NULL)) {
    if (addr.socket_family == AF_INET) {
      const struct sockaddr_in *sa = (struct sockaddr_in *)&addr;
      if (*ip_len >= sizeof(sa->sin_addr)) {
        // Copy IPv4 address
        memcpy(ip, &sa->sin_addr, sizeof(sa->sin_addr));
        *ip_len = sizeof(sa->sin_addr);
      }
      *port = sa->sin_port;
    }
#if defined(SLI_SI91X_ENABLE_IPV6)
    else if (addr.socket_family == AF_INET6) {
      struct sockaddr_in6 *sa = (struct sockaddr_in6 *)&addr;
      if (*ip_len >= sizeof(sa->sin6_addr)) {
        // Copy IPv6 address
        memcpy(ip, &sa->sin6_addr, sizeof(sa->sin6_addr));
        *ip_len = sizeof(sa->sin6_addr);
      }
      if (port != NULL) {
        *port = sa->sin6_port;
      }
    }
#endif
  } else {
    return IOT_SOCKET_EINVAL; // Invalid argument
  }

  return IOT_SOCKET_NO_ERROR;
}

int32_t iotSocketCreate(int32_t af, int32_t type, int32_t protocol)
{
  int32_t status = 0;

  // Check supported protocol family, type of socket and protocol being specified
  if ((af != IOT_SOCKET_AF_INET && af != IOT_SOCKET_AF_INET6)
      || (type != IOT_SOCKET_SOCK_STREAM && type != IOT_SOCKET_SOCK_DGRAM)
      || (protocol != IOT_SOCKET_IPPROTO_TCP && protocol != IOT_SOCKET_IPPROTO_UDP)) {
    return IOT_SOCKET_EINVAL;
  }

  // Check protocol and type combination is supported
  if ((protocol == IOT_SOCKET_IPPROTO_TCP && type != IOT_SOCKET_SOCK_STREAM)
      || (protocol == IOT_SOCKET_IPPROTO_UDP && type != IOT_SOCKET_SOCK_DGRAM)) {
    return IOT_SOCKET_ENOTSUP;
  }

  // Map IOT_SOCKET constants to platform-specific values
  af       = (af == IOT_SOCKET_AF_INET) ? AF_INET : AF_INET6;
  type     = (type == IOT_SOCKET_SOCK_STREAM) ? SOCK_STREAM : SOCK_DGRAM;
  protocol = (protocol == IOT_SOCKET_IPPROTO_TCP) ? IPPROTO_TCP : IPPROTO_UDP;

  // Create the socket
  status = socket(af, type, protocol);

  return (status < 0) ? sli_si91x_errno_to_rc() : status;
}

int32_t iotSocketClose(int32_t socket)
{
  int32_t status = 0;
  status         = close(socket);

  return (status < 0) ? sli_si91x_errno_to_rc() : status;
}

int32_t iotSocketBind(int32_t socket, const uint8_t *ip, uint32_t ip_len, uint16_t port)
{
  int32_t status          = 0;
  sockaddr_storage_t addr = { 0 };

  // Construct remote host address
  status = sli_si91x_construct_remote_host_address(&addr, ip, ip_len, port);

  if (status < 0) {
    return status; // Return an error code if address construction fails
  }

  // Bind the socket to the constructed address
  status = bind(socket, (struct sockaddr *)&addr, addr.socket_length);

  return (status < 0) ? sli_si91x_errno_to_rc() : status; // Return an error code if bind fails
}

int32_t iotSocketListen(int32_t socket, int32_t backlog)
{
  int32_t status = 0;

  // Start listening for incoming connections on the socket with a specified backlog
  status = listen(socket, backlog);

  return (status < 0) ? sli_si91x_errno_to_rc() : status; // Return an error code if listen fails
}

int32_t iotSocketAccept(int32_t socket, uint8_t *ip, uint32_t *ip_len, uint16_t *port)
{
  int32_t status           = 0;
  int32_t client_socket_id = 0;
  sockaddr_storage_t addr  = { 0 };
  socklen_t addr_len       = sizeof(sockaddr_storage_t);

  // Check parameters
  if ((ip == NULL) || (ip_len == NULL) || (port == NULL)) {
    return IOT_SOCKET_EINVAL;
  }

  // Check ip length
  if ((*ip_len != SL_IPV4_ADDRESS_LENGTH) && (*ip_len != SL_IPV6_ADDRESS_LENGTH)) {
    return IOT_SOCKET_EINVAL;
  }

  // Accept an incoming connection and obtain the client socket ID
  client_socket_id = accept(socket, (struct sockaddr *)&addr, &addr_len);

  if (client_socket_id < 0) {
    return sli_si91x_errno_to_rc(); // Return an error code if accept fails
  }

  // Copy remote IP address and port
  status = sli_si91x_copy_ip_address_and_port(addr, ip, ip_len, port);

  if (status < 0) {
    return status; // Return an error if copying fails
  }

  return client_socket_id;
}

int32_t iotSocketConnect(int32_t socket, const uint8_t *ip, uint32_t ip_len, uint16_t port)
{
  int32_t status          = 0;
  sockaddr_storage_t addr = { 0 };

  // Construct remote host address
  status = sli_si91x_construct_remote_host_address(&addr, ip, ip_len, port);

  if (status < 0) {
    return status; // Return an error if address construction fails
  }

  // Connect to the remote host using the constructed address
  status = connect(socket, (struct sockaddr *)&addr, addr.socket_length);

  if (status < 0) {
    // Translate specific connection reset error to a more common one
    status = sli_si91x_errno_to_rc();
    if (status == IOT_SOCKET_ECONNRESET) {
      return IOT_SOCKET_ECONNREFUSED;
    }

    return status; // Return an error if connect fails
  }

  return status;
}

int32_t iotSocketSend(int32_t socket, const void *buf, uint32_t len)
{
  int32_t status = 0;

  if ((len == 0U) && (buf == NULL)) {
    return IOT_SOCKET_EINVAL; // Invalid argument
  }

  // Send data on the socket
  status = send(socket, buf, len, NO_FLAGS);

  if (status < 0) {
    status = sli_si91x_errno_to_rc();
    if (status == IOT_SOCKET_EINPROGRESS) {
      return IOT_SOCKET_EAGAIN; // Operation would block
    }
  }

  return status; // Return send status
}

int32_t iotSocketSendTo(int32_t socket,
                        const void *buf,
                        uint32_t len,
                        const uint8_t *ip,
                        uint32_t ip_len,
                        uint16_t port)
{
  int32_t status          = 0;
  sockaddr_storage_t addr = { 0 };
  socklen_t addr_len      = sizeof(sockaddr_storage_t);

  if (buf == NULL) {
    return IOT_SOCKET_EINVAL; // Invalid argument
  }

  // Construct remote host address
  status = sli_si91x_construct_remote_host_address(&addr, ip, ip_len, port);

  if (status < 0) {
    return status; // Return the error code if construction fails
  }

  status = sendto(socket, buf, len, NO_FLAGS, (struct sockaddr *)&addr, addr_len);

  return (status < 0) ? sli_si91x_errno_to_rc() : status; // Return the status or error code
}

int32_t iotSocketGetSockName(int32_t socket, uint8_t *ip, uint32_t *ip_len, uint16_t *port)
{
  int32_t status          = 0;
  sockaddr_storage_t addr = { 0 };
  socklen_t addr_len      = sizeof(sockaddr_storage_t);

  if ((ip == NULL) || (ip_len == NULL) || (port == NULL)) {
    return IOT_SOCKET_EINVAL; // Invalid argument
  }

  if ((*ip_len != SL_IPV4_ADDRESS_LENGTH) && (*ip_len != SL_IPV6_ADDRESS_LENGTH)) {
    return IOT_SOCKET_EINVAL; // Invalid IP length
  }

  status = getsockname(socket, (struct sockaddr *)&addr, &addr_len);

  if (status < 0) {
    return sli_si91x_errno_to_rc(); // Return the error code on failure
  }

  // Copy IP address and port and return status
  return sli_si91x_copy_ip_address_and_port(addr, ip, ip_len, port);
}

int32_t iotSocketGetPeerName(int32_t socket, uint8_t *ip, uint32_t *ip_len, uint16_t *port)
{
  int32_t status          = 0;
  sockaddr_storage_t addr = { 0 };
  socklen_t addr_len      = sizeof(sockaddr_storage_t);

  if ((ip == NULL) || (ip_len == NULL) || (port == NULL)) {
    return IOT_SOCKET_EINVAL; // Invalid argument
  }

  if ((*ip_len != SL_IPV4_ADDRESS_LENGTH) && (*ip_len != SL_IPV6_ADDRESS_LENGTH)) {
    return IOT_SOCKET_EINVAL; // Invalid IP length
  }

  status = getpeername(socket, (struct sockaddr *)&addr, &addr_len);

  if (status < 0) {
    return sli_si91x_errno_to_rc(); // Return the error code on failure
  }

  return sli_si91x_copy_ip_address_and_port(addr, ip, ip_len, port);
}

int32_t iotSocketSetOpt(int32_t socket, int32_t opt_id, const void *opt_val, uint32_t opt_len)
{
  int32_t status = 0;

  if ((opt_val == NULL) || (opt_len == 0)) {
    return IOT_SOCKET_EINVAL; // Invalid argument
  }

  switch (opt_id) {
    case IOT_SOCKET_IO_FIONBIO: {
      // Feature not suported currently
      return IOT_SOCKET_ENOTSUP;
    }
    case IOT_SOCKET_SO_RCVTIMEO: {
      status = setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, opt_val, opt_len);
      break;
    }
    case IOT_SOCKET_SO_SNDTIMEO: {
      // Feature not suported currently
      return IOT_SOCKET_ENOTSUP;
    }
    case IOT_SOCKET_SO_KEEPALIVE: {
      return IOT_SOCKET_ENOTSUP; // TBD
    }
    default: {
      return IOT_SOCKET_ENOTSUP; // Unsupported option
    }
  }

  if (status < 0) {
    return sli_si91x_errno_to_rc(); // failure
  }

  return status;
}

int32_t iotSocketGetOpt(int32_t socket, int32_t opt_id, void *opt_val, uint32_t *opt_len)
{
  int32_t status;

  if ((opt_val == NULL) || (opt_len == NULL)) {
    return IOT_SOCKET_EINVAL; // Invalid argument
  }

  switch (opt_id) {
    case IOT_SOCKET_SO_RCVTIMEO: {
      status = getsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, opt_val, opt_len);
      break;
    }
    case IOT_SOCKET_SO_SNDTIMEO: {
      // Feature not suported currently
      return IOT_SOCKET_ENOTSUP;
    }
    case IOT_SOCKET_SO_KEEPALIVE: {
      return IOT_SOCKET_ENOTSUP; // TBD
    }
    case IOT_SOCKET_SO_TYPE: {
      status               = getsockopt(socket, SOL_SOCKET, SO_TYPE, opt_val, opt_len);
      *(uint32_t *)opt_val = (*(uint32_t *)opt_val == SOCK_STREAM) ? IOT_SOCKET_SOCK_STREAM : IOT_SOCKET_SOCK_DGRAM;
      break;
    }
    default: {
      return IOT_SOCKET_ENOTSUP; // Unsupported option
    }
  }

  if (status < 0) {
    return sli_si91x_errno_to_rc(); // Failure
  }

  if (*opt_len > 4) {
    *opt_len = 4; // Ensure the option length is not greater than 4
  }

  return status;
}

int32_t iotSocketRecv(int32_t socket, void *buf, uint32_t len)
{
  int32_t bytes_received;

  if ((len == 0U) || (buf == NULL)) {
    return IOT_SOCKET_EINVAL;
  }

  bytes_received = recv(socket, buf, len, NO_FLAGS);

  return (bytes_received < 0) ? sli_si91x_errno_to_rc() : bytes_received; // Return received bytes or error code
}

int32_t iotSocketRecvFrom(int32_t socket, void *buf, uint32_t len, uint8_t *ip, uint32_t *ip_len, uint16_t *port)
{
  sockaddr_storage_t addr = { 0 };
  socklen_t addr_len      = sizeof(struct sockaddr_in);
  int32_t status          = 0;
  int32_t bytes_received  = 0;

  // Check for invalid arguments
  if ((buf == NULL) || (len == 0) || (ip == NULL) || (ip_len == NULL) || (port == NULL)) {
    return IOT_SOCKET_EINVAL;
  }

  // Check for valid IP address length
  if ((*ip_len != SL_IPV4_ADDRESS_LENGTH) && (*ip_len != SL_IPV6_ADDRESS_LENGTH)) {
    return IOT_SOCKET_EINVAL;
  }

  // Receive data from the socket into the buffer
  bytes_received = recvfrom(socket, buf, len, NO_FLAGS, (struct sockaddr *)&addr, &addr_len);

  if (bytes_received < 0) {
    return sli_si91x_errno_to_rc(); // Return the error code on failure
  }
  // Copy remote IP address and port
  status = sli_si91x_copy_ip_address_and_port(addr, ip, ip_len, port);

  if (status < 0) {
    return status; // Return any copy error
  }

  return bytes_received;
}

int32_t iotSocketGetHostByName(const char *name, int32_t af, uint8_t *ip, uint32_t *ip_length)
{
  struct hostent *host_ent = NULL;

  // Check if url length exceeds
  if (sl_strlen((char *)name) > DNS_REQUEST_MAX_URL_LEN) {
    return IOT_SOCKET_ENOTSUP; // URL too long, not supported
  }

  // Check for invalid arguments
  if ((name == NULL) || (ip == NULL) || (ip_length == NULL)) {
    return IOT_SOCKET_EINVAL;
  }

  // Check for invalid address family
  if ((af != IOT_SOCKET_AF_INET) && (af != IOT_SOCKET_AF_INET6)) {
    return IOT_SOCKET_ENOTSUP; // Unsupported address family
  }

  // Check if af and ip_length combination is valid
  if (((af == IOT_SOCKET_AF_INET6) && (*ip_length != SL_IPV6_ADDRESS_LENGTH))
      || ((af == IOT_SOCKET_AF_INET) && (*ip_length != SL_IPV4_ADDRESS_LENGTH))) {
    return IOT_SOCKET_EINVAL; // Incompatible IP address length
  }

  // Translate af from iot to bsd
  af = (af == IOT_SOCKET_AF_INET) ? AF_INET : AF_INET6;

  // Use gethostbyname2 to resolve the host name
  host_ent    = gethostbyname2(name, af);
  const int *herrno = __h_errno_location();

  if (host_ent == NULL) {
    switch (*herrno) {
      case TRY_AGAIN: {
        return IOT_SOCKET_ETIMEDOUT;
      }
      case HOST_NOT_FOUND: {
        return IOT_SOCKET_EHOSTNOTFOUND;
      }
      default: {
        return IOT_SOCKET_ERROR;
      }
    }
  }

  // Copy the resolved IP address into the provided buffer
  memcpy(ip, (uint8_t *)host_ent->h_addr, *ip_length);

  return IOT_SOCKET_NO_ERROR;
}
