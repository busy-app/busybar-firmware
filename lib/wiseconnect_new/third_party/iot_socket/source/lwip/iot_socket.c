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
 */

#include <string.h>
#include "iot_socket.h"
#include "lwip/netdb.h"
#include "lwip/sockets.h"
#include "RTE_Components.h"

#define NUM_SOCKS   MEMP_NUM_NETCONN

// Socket attributes
static struct {
  uint32_t ionbio  : 1;
  uint32_t bound   : 1;
  uint32_t tv_sec  : 20;
  uint32_t tv_msec : 10;
} sock_attr[NUM_SOCKS];

// Convert return codes from lwIP to IoT
static int32_t errno_to_rc (void) {
  int32_t rc;

  switch (errno) {
    case 0:
      rc = 0;
      break;
    case EBADF:
    case ENOBUFS:
      rc = IOT_SOCKET_ESOCK;
      break;
    case EIO:
    case EINVAL:
      rc = IOT_SOCKET_EINVAL;
      break;
    case ENOMEM:
      rc = IOT_SOCKET_ENOMEM;
      break;
    case EWOULDBLOCK:
      rc = IOT_SOCKET_EAGAIN;
      break;
    case EINPROGRESS:
      rc = IOT_SOCKET_EINPROGRESS;
      break;
    case ENOTCONN:
      rc = IOT_SOCKET_ENOTCONN;
      break;
    case EISCONN:
      rc = IOT_SOCKET_EISCONN;
      break;
    case ECONNRESET:
      rc = IOT_SOCKET_ECONNRESET;
      break;
    case ECONNABORTED:
      rc = IOT_SOCKET_ECONNABORTED;
      break;
    case EALREADY:
      rc = IOT_SOCKET_EALREADY;
      break;
    case EADDRINUSE:
      rc = IOT_SOCKET_EADDRINUSE;
      break;
    case EHOSTUNREACH:
      rc = IOT_SOCKET_EHOSTNOTFOUND;
      break;
    default:
      rc = IOT_SOCKET_ERROR;
      break;
  }

  return rc;
}

// Create a communication socket
int32_t iotSocketCreate (int32_t af, int32_t type, int32_t protocol) {
  int32_t rc;

  // Convert parameters
  switch (af) {
    case IOT_SOCKET_AF_INET:
      af = AF_INET;
      break;
    case IOT_SOCKET_AF_INET6:
      af = AF_INET6;
      break;
    default:
      return IOT_SOCKET_EINVAL;
  }

  switch (type) {
    case IOT_SOCKET_SOCK_STREAM:
      type = SOCK_STREAM;
      break;
    case IOT_SOCKET_SOCK_DGRAM:
      type = SOCK_DGRAM;
      break;
    default:
      return IOT_SOCKET_EINVAL;
  }

  switch (protocol) {
    case 0:
      break;
    case IOT_SOCKET_IPPROTO_TCP:
      if (type != SOCK_STREAM) {
        return IOT_SOCKET_EINVAL;
      }
      protocol = IPPROTO_TCP;
      break;
    case IOT_SOCKET_IPPROTO_UDP:
      if (type != SOCK_DGRAM) {
        return IOT_SOCKET_EINVAL;
      }
      protocol = IPPROTO_UDP;
      break;
    default:
      return IOT_SOCKET_EINVAL;
  }

  rc = socket(af, type, protocol);
  if (rc < 0) {
    return errno_to_rc ();
  }

  memset (&sock_attr[rc-LWIP_SOCKET_OFFSET], 0, sizeof(sock_attr[0]));
  return rc;
}

// Assign a local address to a socket
int32_t iotSocketBind (int32_t socket, const uint8_t *ip, uint32_t ip_len, uint16_t port) {
  struct sockaddr_storage addr;
  int32_t rc;

  // Check parameters
  if ((ip == NULL) || (port == 0)) {
    return IOT_SOCKET_EINVAL;
  }

  // Construct local address
  switch (ip_len) {
    case sizeof(struct in_addr): {
      struct sockaddr_in *sa = (struct sockaddr_in *)&addr;
      sa->sin_len    = sizeof(struct sockaddr_in);
      sa->sin_family = AF_INET;
      sa->sin_port   = lwip_htons((port));
      memcpy(&sa->sin_addr, ip, sizeof(struct in_addr));
      memset(sa->sin_zero, 0, SIN_ZERO_LEN);
    } break;
#if defined(RTE_Network_IPv6)
    case sizeof(struct in6_addr): {
      struct sockaddr_in6 *sa = (struct sockaddr_in6 *)&addr;
      sa->sin6_len   = sizeof(struct sockaddr_in6);
      sa->sin6_family= AF_INET6;
      sa->sin6_port  = htons(port);
      memcpy(&sa->sin6_addr, ip, sizeof(struct in6_addr));
    } break;
#endif
    default:
      return IOT_SOCKET_EINVAL;
  }

  rc = bind(socket, (struct sockaddr *)&addr, addr.s2_len);
  if (rc < 0) {
    rc = errno_to_rc ();
    if (rc == IOT_SOCKET_EADDRINUSE && sock_attr[socket-LWIP_SOCKET_OFFSET].bound) {
      return IOT_SOCKET_EINVAL;
    }
    return rc;
  }
  sock_attr[socket-LWIP_SOCKET_OFFSET].bound = 1;
  return rc;
}

// Listen for socket connections
int32_t iotSocketListen (int32_t socket, int32_t backlog) {
  int32_t rc;

  rc = listen(socket, backlog);
  if (rc == 0 && !sock_attr[socket-LWIP_SOCKET_OFFSET].bound) {
    return IOT_SOCKET_EINVAL;
  }
  if (rc < 0) {
    return errno_to_rc ();
  }
  return rc;
}

// Accept a new connection on a socket
int32_t iotSocketAccept (int32_t socket, uint8_t *ip, uint32_t *ip_len, uint16_t *port) {
  struct sockaddr_storage addr;
  socklen_t addr_len = sizeof(struct sockaddr_storage);
  int32_t rc;

  rc = accept (socket, (struct sockaddr *)&addr, &addr_len);
  if (rc < 0) {
    return errno_to_rc ();
  }

  // Copy remote IP address and port
  if ((ip != NULL) && (ip_len != NULL)) {
    if (addr.ss_family == AF_INET) {
      struct sockaddr_in *sa = (struct sockaddr_in *)&addr;
      if (*ip_len >= sizeof(sa->sin_addr)) {
        memcpy(ip, &sa->sin_addr, sizeof(sa->sin_addr));
        *ip_len = sizeof(sa->sin_addr);
      }
      if (port != NULL) {
        *port   = ntohs (sa->sin_port);
      }
    }
#if defined(RTE_Network_IPv6)
    else if (addr.ss_family == AF_INET6) {
      struct sockaddr_in6 *sa = (struct sockaddr_in6 *)&addr;
      if (*ip_len >= sizeof(sa->sin6_addr)) {
        memcpy(ip, &sa->sin6_addr, sizeof(sa->sin6_addr));
        *ip_len = sizeof(sa->sin6_addr);
      }
      if (port != NULL) {
        *port   = ntohs (sa->sin6_port);
      }
    }
#endif
  }

  return rc;
}

// Connect a socket to a remote host
int32_t iotSocketConnect (int32_t socket, const uint8_t *ip, uint32_t ip_len, uint16_t port) {
  struct sockaddr_storage addr;
  int32_t rc;

  // Check parameters
  if ((ip == NULL) || (port == 0)) {
    return IOT_SOCKET_EINVAL;
  }

  // Construct remote host address
  switch (ip_len) {
    case sizeof(struct in_addr): {
      struct sockaddr_in *sa = (struct sockaddr_in *)&addr;
      sa->sin_len    = sizeof(struct sockaddr_in);
      sa->sin_family = AF_INET;
      sa->sin_port   = lwip_htons((port));
      memcpy(&sa->sin_addr, ip, sizeof(struct in_addr));
      memset(sa->sin_zero, 0, SIN_ZERO_LEN);
      if (sa->sin_addr.s_addr == 0) return IOT_SOCKET_EINVAL;
    } break;
#if defined(RTE_Network_IPv6)
    case sizeof(struct in6_addr): {
      struct sockaddr_in6 *sa = (struct sockaddr_in6 *)&addr;
      sa->sin6_len   = sizeof(struct sockaddr_in6);
      sa->sin6_family= AF_INET6;
      sa->sin6_port  = htons(port);
      memcpy(&sa->sin6_addr, ip, sizeof(struct in6_addr));
    } break;
#endif
    default:
      return IOT_SOCKET_EINVAL;
  }

  rc = connect(socket, (struct sockaddr *)&addr, addr.s2_len);
  if (rc < 0) {
    rc = errno_to_rc ();
    if (rc == IOT_SOCKET_ECONNRESET) {
      return IOT_SOCKET_ECONNREFUSED;
    }
    return rc;
  }
  sock_attr[socket-LWIP_SOCKET_OFFSET].bound = 1;
  return rc;
}

// Check if socket is readable
static int32_t socket_check_read (int32_t socket) {
  struct timeval tv, *ptv;
  fd_set  fds;
  int32_t nr;

  if ((socket < LWIP_SOCKET_OFFSET) || (socket >= (LWIP_SOCKET_OFFSET + NUM_SOCKS))) {
    return IOT_SOCKET_ESOCK;
  }

  FD_ZERO(&fds);
  FD_SET(socket, &fds);
  ptv = &tv;
  memset (&tv, 0, sizeof(tv));
  if (!sock_attr[socket-LWIP_SOCKET_OFFSET].ionbio) {
    tv.tv_sec  = sock_attr[socket-LWIP_SOCKET_OFFSET].tv_sec;
    tv.tv_usec = sock_attr[socket-LWIP_SOCKET_OFFSET].tv_msec * 1000;
    if ((tv.tv_sec == 0U) && (tv.tv_usec == 0U)) {
      ptv = NULL;
    }
  }
  nr = select (socket+1, &fds, NULL, NULL, ptv);
  if (nr == 0) {
    return IOT_SOCKET_EAGAIN;
  }
  return 0;
}

// Receive data on a connected socket
int32_t iotSocketRecv (int32_t socket, void *buf, uint32_t len) {
  int32_t rc;

  if (len == 0U) {
    return socket_check_read (socket);
  }
  if (buf == NULL) {
    return IOT_SOCKET_EINVAL;
  }
  rc = recv(socket, buf, len, 0);
  if (rc < 0) {
    return errno_to_rc ();
  }

  return rc;
}

// Receive data on a socket
int32_t iotSocketRecvFrom (int32_t socket, void *buf, uint32_t len, uint8_t *ip, uint32_t *ip_len, uint16_t *port) {
  struct sockaddr_storage addr;
  socklen_t addr_len = sizeof(struct sockaddr_storage);
  int32_t rc;

  if (len == 0U) {
    return socket_check_read (socket);
  }
  if (buf == NULL) {
    return IOT_SOCKET_EINVAL;
  }
  rc = recvfrom(socket, buf, len, 0, (struct sockaddr *)&addr, &addr_len);
  if (rc < 0) {
    return errno_to_rc ();
  }

  // Copy remote IP address and port
  if ((ip != NULL) && (ip_len != NULL)) {
    if (addr.ss_family == AF_INET) {
      struct sockaddr_in *sa = (struct sockaddr_in *)&addr;
      if (*ip_len >= sizeof(sa->sin_addr)) {
        memcpy(ip, &sa->sin_addr, sizeof(sa->sin_addr));
        *ip_len = sizeof(sa->sin_addr);
      }
      if (port != NULL) {
        *port   = ntohs (sa->sin_port);
      }
    }
#if defined(RTE_Network_IPv6)
    else if (addr.ss_family == AF_INET6) {
      struct sockaddr_in6 *sa = (struct sockaddr_in6 *)&addr;
      if (*ip_len >= sizeof(sa->sin6_addr)) {
        memcpy(ip, &sa->sin6_addr, sizeof(sa->sin6_addr));
        *ip_len = sizeof(sa->sin6_addr);
      }
      if (port != NULL) {
        *port   = ntohs (sa->sin6_port);
      }
    }
#endif
  }

  return rc;
}

// Check if socket is writable
static int32_t socket_check_write (int32_t socket) {
#if 0
  // Reentrant call to select does not work
  // (while select on check_readable is in progress)
  struct timeval tv;
  fd_set fds;
  int32_t nr;

  if ((socket < LWIP_SOCKET_OFFSET) || (socket >= (LWIP_SOCKET_OFFSET + NUM_SOCKS))) {
    return IOT_SOCKET_ESOCK;
  }
  FD_ZERO(&fds);
  FD_SET(socket, &fds);
  memset (&tv, 0, sizeof(tv));
  nr = select (socket+1, NULL, &fds, NULL, &tv);
  if (nr == 0) {
    return IOT_SOCKET_ERROR;
  }
#else
  (void)socket;
#endif
  return 0;
}

// Send data on a connected socket
int32_t iotSocketSend (int32_t socket, const void *buf, uint32_t len) {
  int32_t rc;

  if (len == 0U) {
    return socket_check_write (socket);
  }
  if (buf == NULL) {
    return IOT_SOCKET_EINVAL;
  }
  rc = send(socket, buf, len, 0);
  if (rc < 0) {
    rc = errno_to_rc ();
    if (rc == IOT_SOCKET_EINPROGRESS && sock_attr[socket-LWIP_SOCKET_OFFSET].ionbio) {
      return IOT_SOCKET_EAGAIN;
    }
  }

  return rc;
}

// Send data on a socket
int32_t iotSocketSendTo (int32_t socket, const void *buf, uint32_t len, const uint8_t *ip, uint32_t ip_len, uint16_t port) {
  struct sockaddr_storage addr;
  socklen_t addr_len = sizeof(struct sockaddr_storage);
  int32_t rc;

  if (len == 0U) {
    return socket_check_write (socket);
  }
  if ((buf == NULL) || (ip == NULL)) {
    return IOT_SOCKET_EINVAL;
  }
  if (ip != NULL) {
    // Construct remote host address
    switch (ip_len) {
      case sizeof(struct in_addr): {
        struct sockaddr_in *sa = (struct sockaddr_in *)&addr;
        sa->sin_len    = sizeof(struct sockaddr_in);
        sa->sin_family = AF_INET;
        sa->sin_port   = lwip_htons((port));
        memcpy(&sa->sin_addr, ip, sizeof(struct in_addr));
        memset(sa->sin_zero, 0, SIN_ZERO_LEN);
      } break;
#if defined(RTE_Network_IPv6)
      case sizeof(struct in6_addr): {
        struct sockaddr_in6 *sa = (struct sockaddr_in6 *)&addr;
        sa->sin6_len   = sizeof(struct sockaddr_in6);
        sa->sin6_family= AF_INET6;
        sa->sin6_port  = htons(port);
        memcpy(&sa->sin6_addr, ip, sizeof(struct in6_addr));
      } break;
#endif
      default:
        return IOT_SOCKET_EINVAL;
    }
    rc = sendto(socket, buf, len, 0, (struct sockaddr *)&addr, addr_len);
  } else {
    rc = sendto(socket, buf, len, 0, NULL, 0);
  }

  if (rc < 0) {
    return errno_to_rc ();
  }

  return rc;
}

// Retrieve local IP address and port of a socket
int32_t iotSocketGetSockName (int32_t socket, uint8_t *ip, uint32_t *ip_len, uint16_t *port) {
  struct sockaddr_storage addr;
  socklen_t addr_len = sizeof(struct sockaddr_storage);
  int32_t rc;

  rc = getsockname(socket, (struct sockaddr *)&addr, &addr_len);
  if (rc < 0) {
    return errno_to_rc ();
  }
  if (!sock_attr[socket-LWIP_SOCKET_OFFSET].bound) {
    return IOT_SOCKET_EINVAL;
  }

  rc = IOT_SOCKET_EINVAL;

  // Copy local IP address and port
  if (addr.ss_family == AF_INET) {
    struct sockaddr_in *sa = (struct sockaddr_in *)&addr;
    if ((ip != NULL) && (ip_len != NULL) && (*ip_len >= sizeof(sa->sin_addr))) {
      memcpy(ip, &sa->sin_addr, sizeof(sa->sin_addr));
      *ip_len = sizeof(sa->sin_addr);
      rc = 0;
    }
    if (port != NULL) {
      *port   = ntohs (sa->sin_port);
      rc = 0;
    }
  }
#if defined(RTE_Network_IPv6)
  else if (addr.ss_family == AF_INET6) {
    struct sockaddr_in6 *sa = (struct sockaddr_in6 *)&addr;
    if ((ip != NULL) && (ip_len != NULL) && (*ip_len >= sizeof(sa->sin6_addr))) {
      memcpy(ip, &sa->sin6_addr, sizeof(sa->sin6_addr));
      *ip_len = sizeof(sa->sin6_addr);
      rc = 0;
    }
    if (port != NULL) {
      *port   = ntohs (sa->sin6_port);
      rc = 0;
    }
  }
#endif

  return rc;
}

// Retrieve remote IP address and port of a socket
int32_t iotSocketGetPeerName (int32_t socket, uint8_t *ip, uint32_t *ip_len, uint16_t *port) {
  struct sockaddr_storage addr;
  socklen_t addr_len = sizeof(struct sockaddr_storage);
  int32_t rc;

  rc = getpeername(socket, (struct sockaddr *)&addr, &addr_len);
  if (rc < 0) {
    return errno_to_rc ();
  }

  rc = IOT_SOCKET_EINVAL;

  // Copy remote IP address and port
  if (addr.ss_family == AF_INET) {
    struct sockaddr_in *sa = (struct sockaddr_in *)&addr;
    if ((ip != NULL) && (ip_len != NULL) && (*ip_len >= sizeof(sa->sin_addr))) {
      memcpy(ip, &sa->sin_addr, sizeof(sa->sin_addr));
      *ip_len = sizeof(sa->sin_addr);
      rc = 0;
    }
    if (port != NULL) {
      *port   = ntohs (sa->sin_port);
      rc = 0;
    }
  }
#if defined(RTE_Network_IPv6)
  else if (addr.ss_family == AF_INET6) {
    struct sockaddr_in6 *sa = (struct sockaddr_in6 *)&addr;
    if ((ip != NULL) && (ip_len != NULL) && (*ip_len >= sizeof(sa->sin6_addr))) {
      memcpy(ip, &sa->sin6_addr, sizeof(sa->sin6_addr));
      *ip_len = sizeof(sa->sin6_addr);
      rc = 0;
    }
    if (port != NULL) {
      *port   = ntohs (sa->sin6_port);
      rc = 0;
    }
  }
#endif

  return rc;
}

// Get socket option
int32_t iotSocketGetOpt (int32_t socket, int32_t opt_id, void *opt_val, uint32_t *opt_len) {
  int32_t rc;

  if ((opt_val == NULL) || (opt_len == NULL) || (*opt_len == 0)) {
    return IOT_SOCKET_EINVAL;
  }
  switch (opt_id) {
    case IOT_SOCKET_IO_FIONBIO:
      return IOT_SOCKET_EINVAL;
    case IOT_SOCKET_SO_RCVTIMEO: {
#if LWIP_SO_SNDRCVTIMEO_NONSTANDARD
      rc = getsockopt(socket, SOL_SOCKET, SO_RCVTIMEO,  (char *)opt_val, opt_len);
#else
      struct timeval tv;
      uint32_t tv_len = sizeof(tv);
      rc = getsockopt(socket, SOL_SOCKET, SO_RCVTIMEO,  (char *)&tv, &tv_len);
      if (rc == 0) {
        *((int32_t *)opt_val) = tv.tv_sec * 1000 + tv.tv_usec / 1000;
      }
#endif
    } break;
    case IOT_SOCKET_SO_SNDTIMEO: {
#if LWIP_SO_SNDRCVTIMEO_NONSTANDARD
      rc = getsockopt(socket, SOL_SOCKET, SO_SNDTIMEO,  (char *)opt_val, opt_len);
#else
      struct timeval tv;
      uint32_t tv_len = sizeof(tv);
      rc = getsockopt(socket, SOL_SOCKET, SO_SNDTIMEO,  (char *)&tv, &tv_len);
      if (rc == 0) {
        *((int32_t *)opt_val) = tv.tv_sec * 1000 + tv.tv_usec / 1000;
      }
#endif
    } break;
    case IOT_SOCKET_SO_KEEPALIVE:
      rc = getsockopt(socket, SOL_SOCKET, SO_KEEPALIVE, (char *)opt_val, opt_len);
      break;
    case IOT_SOCKET_SO_TYPE:
      rc = getsockopt(socket, SOL_SOCKET, SO_TYPE,      (char *)opt_val, opt_len);
      break;
    default:
      return IOT_SOCKET_EINVAL;
  }
  if (rc < 0) {
    return errno_to_rc ();
  }
  if (*opt_len > 4) *opt_len = 4;

  return rc;
}

// Set socket option
int32_t iotSocketSetOpt (int32_t socket, int32_t opt_id, const void *opt_val, uint32_t opt_len) {
  int32_t rc;

  if ((opt_val == NULL) || (opt_len == 0)) {
    return IOT_SOCKET_EINVAL;
  }
  switch (opt_id) {
    case IOT_SOCKET_IO_FIONBIO:
      if (opt_len != sizeof(unsigned long)) {
        return IOT_SOCKET_EINVAL;
      }
      rc = ioctlsocket(socket, FIONBIO, (void *)(uint32_t)opt_val);
      if (rc == 0) {
        sock_attr[socket-LWIP_SOCKET_OFFSET].ionbio = *(const uint32_t *)opt_val ? 1 : 0;
      }
      break;
    case IOT_SOCKET_SO_RCVTIMEO: {
#if LWIP_SO_SNDRCVTIMEO_NONSTANDARD
      rc = setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO,  (const char *)opt_val, opt_len);
#else
      struct timeval tv;
      tv.tv_sec  = (*(const int32_t *)opt_val / 1000);
      tv.tv_usec = (*(const int32_t *)opt_val % 1000) * 1000;
      rc = setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO,  (const char *)&tv, sizeof(tv));
#endif
      if (rc == 0) {
        sock_attr[socket-LWIP_SOCKET_OFFSET].tv_sec  = *(const uint32_t *)opt_val / 1000U;
        sock_attr[socket-LWIP_SOCKET_OFFSET].tv_msec = *(const uint32_t *)opt_val % 1000U;
      }
    } break;
    case IOT_SOCKET_SO_SNDTIMEO: {
#if LWIP_SO_SNDRCVTIMEO_NONSTANDARD
      rc = setsockopt(socket, SOL_SOCKET, SO_SNDTIMEO,  (const char *)opt_val, opt_len);
#else
      struct timeval tv;
      tv.tv_sec  = (*(const int32_t *)opt_val / 1000);
      tv.tv_usec = (*(const int32_t *)opt_val % 1000) * 1000;
      rc = setsockopt(socket, SOL_SOCKET, SO_SNDTIMEO,  (const char *)&tv, sizeof(tv));
#endif
    } break;
    case IOT_SOCKET_SO_KEEPALIVE:
      rc = setsockopt(socket, SOL_SOCKET, SO_KEEPALIVE, (const char *)opt_val, opt_len);
      break;
    case IOT_SOCKET_SO_TYPE:
      return IOT_SOCKET_EINVAL;
    default:
      return IOT_SOCKET_EINVAL;
  }
  if (rc < 0) {
    return errno_to_rc ();
  }

  return rc;
}

// Close and release a socket
int32_t iotSocketClose (int32_t socket) {
  int32_t rc;

  rc = closesocket(socket);
  if (rc == 0) {
    memset (&sock_attr[socket-LWIP_SOCKET_OFFSET], 0, sizeof(sock_attr[0]));
  }
  if (rc < 0) {
    return errno_to_rc ();
  }

  return rc;
}

// Retrieve host IP address from host name
int32_t iotSocketGetHostByName (const char *name, int32_t af, uint8_t *ip, uint32_t *ip_len) {
#ifndef SLI_SI91X_LWIP_HOSTED_NETWORK_STACK
	struct hostent *host;

  // Check parameters
  if ((name == NULL) || (ip == NULL) || (ip_len == NULL)) {
    return IOT_SOCKET_EINVAL;
  }
  switch (af) {
    case IOT_SOCKET_AF_INET:
      if (*ip_len < sizeof(sizeof(struct sockaddr_in))) {
        return IOT_SOCKET_EINVAL;
      }
      break;
#if defined(RTE_Network_IPv6)
    case IOT_SOCKET_AF_INET6:
      if (*ip_len < sizeof(sizeof(struct sockaddr_in6))) {
        return IOT_SOCKET_EINVAL;
      }
      break;
#endif
    default:
      return IOT_SOCKET_EINVAL;
  }

  // Resolve hostname
  host = gethostbyname (name);
  if (host == NULL) {
    switch (h_errno) {
      case HOST_NOT_FOUND:
        return IOT_SOCKET_EHOSTNOTFOUND;
      default:
        return IOT_SOCKET_ERROR;
    }
  }

  // Copy resolved IP address
  switch (host->h_addrtype) {
    case AF_INET:
      memcpy(ip, host->h_addr_list[0], sizeof(struct in_addr));
      *ip_len = sizeof(struct in_addr);
      break;
#if defined(RTE_Network_IPv6)
    case AF_INET6:
      memcpy(ip, host->h_addr_list[0], sizeof(struct in6_addr));
      *ip_len = sizeof(struct in6_addr);
      break;
#endif
    default:
      return IOT_SOCKET_ERROR;
  }

  return 0;
#else
  //! Not supported in LWIP(external stack).
return IOT_SOCKET_ENOTSUP;
#endif
}
