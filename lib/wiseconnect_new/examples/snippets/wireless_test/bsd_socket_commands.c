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
#include "console.h"
#include "sl_utility.h"
#ifdef SLI_SI91X_OFFLOAD_NETWORK_STACK
#include "socket.h"
#include "errno.h"
#include "select.h"
#include "netinet_in.h"
#endif
#include "sl_ip_types.h"
#include "sl_constants.h"
#include "sl_net.h"
#include "netdb.h"
#include "sl_utility.h"
#include <string.h>
#ifdef SLI_SI91X_LWIP_HOSTED_NETWORK_STACK
#include "lwip/sockets.h"
int errno;
#endif

/******************************************************
 *                    Macros
 ******************************************************/

#define VERIFY_BSD_STATUS(status) \
  do {                            \
    if (status < 0) {             \
      print_errno();              \
      return SL_STATUS_FAIL;      \
    }                             \
  } while (0)

/******************************************************
 *               Static Function Declarations
 ******************************************************/

static inline void print_errno(void);

/******************************************************
 *               Extern Functions
 ******************************************************/

static uint8_t buffer[500] = { 0 };

/******************************************************
 *               Function Definitions
 ******************************************************/

sl_status_t wifi_bsd_socket_create_handler(console_args_t *arguments)
{
  const int32_t domain   = (int32_t)arguments->arg[0];
  const int32_t type     = (int32_t)arguments->arg[1];
  const int32_t protocol = (int32_t)arguments->arg[2];

  int32_t socket_fd = socket(domain, type, protocol);
  VERIFY_BSD_STATUS(socket_fd);

  printf("%ld", socket_fd);
  return SL_STATUS_OK;
}

sl_status_t wifi_bsd_socket_bind_handler(console_args_t *arguments)
{
  sl_ipv4_address_t ip              = { 0 };
  struct sockaddr_in socket_address = { 0 };
  const int32_t socket_fd           = (int32_t)arguments->arg[0];
#ifdef SLI_SI91X_LWIP_HOSTED_NETWORK_STACK
  const uint16_t port = htons((uint16_t)arguments->arg[3]);
#else
  const uint16_t port = (uint16_t)arguments->arg[3];
#endif
  sl_net_inet_addr((char *)arguments->arg[1], (uint32_t *)&ip);

  //Note: Hardcoding with ipv4, Need to rework once si91x got ipv6 support.
  socket_address.sin_family = AF_INET;
  socket_address.sin_port   = port;
  memcpy(&socket_address.sin_addr.s_addr, &ip.value, SL_IPV4_ADDRESS_LENGTH);

  int32_t status = bind(socket_fd, (struct sockaddr *)&socket_address, sizeof(struct sockaddr_in));
  VERIFY_BSD_STATUS(status);

  return SL_STATUS_OK;
}

sl_status_t wifi_bsd_socket_listen_handler(console_args_t *arguments)
{
  const int32_t socket_fd = (int32_t)arguments->arg[0];
  const int32_t backlog   = (int32_t)arguments->arg[1];

  int32_t status = listen(socket_fd, backlog);
  VERIFY_BSD_STATUS(status);

  return SL_STATUS_OK;
}

sl_status_t wifi_bsd_socket_accept_handler(console_args_t *arguments)
{
  const int32_t socket_fd                  = (int32_t)arguments->arg[0];
  struct sockaddr_in remote_socket_address = { 0 };
  socklen_t socket_length                  = sizeof(struct sockaddr_in);

  int32_t status = accept(socket_fd, ((struct sockaddr *)&remote_socket_address), &socket_length);
  VERIFY_BSD_STATUS(status);

  printf("\r\n Socket ID: %ld\n", status);
  if (socket_length == sizeof(struct sockaddr_in)) {
    const uint8_t *ip_address = (const uint8_t *)&remote_socket_address.sin_addr.s_addr;
    printf("%u.%u.%u.%u:%u",
           ip_address[0],
           ip_address[1],
           ip_address[2],
           ip_address[3],
           remote_socket_address.sin_port);
  }

  return SL_STATUS_OK;
}

sl_status_t wifi_bsd_socket_receive_from_handler(console_args_t *arguments)
{
  int32_t sock_fd            = (int32_t)arguments->arg[0];
  struct sockaddr_in address = { 0 };
  socklen_t address_length   = sizeof(address);
  sl_ipv4_address_t ip       = { 0 };

  int32_t status = recvfrom(sock_fd, buffer, sizeof(buffer), 0, (struct sockaddr *)&address, &address_length);
  VERIFY_BSD_STATUS(status);

  memcpy(&ip.value, &address.sin_addr.s_addr, SL_IPV4_ADDRESS_LENGTH);
  printf("%u.%u.%u.%u:%u\r\n", ip.bytes[0], ip.bytes[1], ip.bytes[2], ip.bytes[3], address.sin_port);

  printf("Received:%s", buffer);
  return SL_STATUS_OK;
}

sl_status_t wifi_bsd_socket_receive_handler(console_args_t *arguments)
{
  int32_t sock_fd = (int32_t)arguments->arg[0];

  int32_t status = recv(sock_fd, buffer, sizeof(buffer), 0);
  VERIFY_BSD_STATUS(status);

  printf("Received:%s", buffer);
  return SL_STATUS_OK;
}

sl_status_t wifi_bsd_socket_send_handler(console_args_t *arguments)
{
  int32_t sock_fd = (int32_t)arguments->arg[0];
  uint8_t *buffer = GET_OPTIONAL_COMMAND_ARG(arguments, 1, "HI", uint8_t *);
  int flags       = 0;

  int32_t status = send(sock_fd, buffer, strlen((char *)buffer), flags);

  VERIFY_BSD_STATUS(status);

  printf("%ld bytes sent", status);

  return SL_STATUS_OK;
}

sl_status_t wifi_bsd_socket_send_to_handler(console_args_t *arguments)
{
  struct sockaddr_in address = { 0 };
  sl_ipv4_address_t ip       = { 0 };
  int32_t sock_fd            = (int32_t)arguments->arg[0];
  char *buffer               = GET_OPTIONAL_COMMAND_ARG(arguments, 1, "HI", char *);
  socklen_t ip_length        = (socklen_t)arguments->arg[3];

  if (ip_length == SL_IPV4_ADDRESS_LENGTH) {
    sl_net_inet_addr((char *)arguments->arg[2], (uint32_t *)&ip);
    memcpy(&address.sin_addr.s_addr, &ip.value, SL_IPV4_ADDRESS_LENGTH);
  }

#ifdef SLI_SI91X_LWIP_HOSTED_NETWORK_STACK
  address.sin_port = htons((in_port_t)arguments->arg[4]);
#else
  address.sin_port    = (in_port_t)arguments->arg[4];
#endif
  address.sin_family = AF_INET;

  int32_t status = sendto(sock_fd, buffer, strlen(buffer), 0, (const struct sockaddr *)&address, sizeof(address));
  VERIFY_BSD_STATUS(status);

  printf("%ld bytes sent", status);

  return SL_STATUS_OK;
}

sl_status_t wifi_bsd_socket_connect_handler(console_args_t *arguments)
{
  struct sockaddr_in address = { 0 };
  sl_ipv4_address_t ip       = { 0 };

  int32_t sock_fd     = (int32_t)arguments->arg[0];
  socklen_t ip_length = (socklen_t)arguments->arg[2];
#ifdef SLI_SI91X_LWIP_HOSTED_NETWORK_STACK
  address.sin_port = htons((in_port_t)arguments->arg[3]);
#else
  address.sin_port    = (in_port_t)arguments->arg[3];
#endif
  if (ip_length == SL_IPV4_ADDRESS_LENGTH) {
    address.sin_family = AF_INET;
    sl_net_inet_addr((char *)arguments->arg[1], (uint32_t *)&ip);
    memcpy(&address.sin_addr.s_addr, &ip.value, SL_IPV4_ADDRESS_LENGTH);
  }

  int32_t status = connect(sock_fd, (struct sockaddr *)&address, sizeof(address));
  VERIFY_BSD_STATUS(status);

  return SL_STATUS_OK;
}

sl_status_t wifi_bsd_socket_close_handler(console_args_t *arguments)
{
  int32_t sock_fd = (int32_t)arguments->arg[0];

  int32_t status = close(sock_fd);
  VERIFY_BSD_STATUS(status);

  return SL_STATUS_OK;
}

sl_status_t wifi_bsd_get_host_by_name_handler(console_args_t *arguments)
{
#ifndef SLI_SI91X_LWIP_HOSTED_NETWORK_STACK
  char *host_name = (char *)arguments->arg[0];

  struct hostent *host_ent = gethostbyname(host_name);
  if (host_ent == NULL) {
    return SL_STATUS_FAIL;
  }

  print_sl_ip_address((sl_ip_address_t *)host_ent->h_addr);

  printf("host_name: %s, h_errno:%d, type:%d, length:%d", host_name, h_errno, host_ent->h_addrtype, host_ent->h_length);

  return SL_STATUS_OK;
#else
  //! Not supported in LWIP(external stack)..
  return SL_STATUS_NOT_SUPPORTED;
#endif
}

sl_status_t wifi_bsd_get_sock_name(console_args_t *arguments)
{
  int status = 0;

  int socket_id                     = (int32_t)arguments->arg[0];
  struct sockaddr_in socket_address = { 0 };
  socklen_t socket_length           = sizeof(struct sockaddr_in);

  status = getsockname(socket_id, (struct sockaddr *)&socket_address, &socket_length);

  VERIFY_BSD_STATUS(status);

  if (socket_length == sizeof(struct sockaddr_in)) {
    uint8_t *ip_address = (uint8_t *)&socket_address.sin_addr.s_addr;

    printf("\n\rIP address %u:%u:%u:%u", ip_address[0], ip_address[1], ip_address[2], ip_address[3]);
    printf("\n\rPort %d", socket_address.sin_port);
  }

  return SL_STATUS_OK;
}
sl_status_t wifi_bsd_select_handler(console_args_t *arguments)
{
  fd_set readfds;
  int nfds               = GET_OPTIONAL_COMMAND_ARG(arguments, 0, 5, uint32_t);
  struct timeval timeout = { .tv_sec = GET_OPTIONAL_COMMAND_ARG(arguments, 1, 5, uint32_t), .tv_usec = 0 };

  FD_ZERO(&readfds);
  for (int i = 0; i < nfds; i++) {
    FD_SET(i, &readfds);
  }

  int32_t retval = select(nfds + 1, &readfds, NULL, NULL, &timeout);
  VERIFY_BSD_STATUS(retval);

  if (retval == 0) {
    printf("Timeout");
  } else {
    printf("Data is available on sockets: ");
    for (int i = 0; i < nfds; i++) {
      if (FD_ISSET(i, &readfds)) {
        printf("%d, ", i);
      }
      FD_CLR(i, &readfds);
    }
  }
  return SL_STATUS_OK;
}

sl_status_t wifi_bsd_socket_get_opt_handler(console_args_t *arguments)
{
  int32_t socket       = (int32_t)arguments->arg[0];
  int32_t option_name  = (int32_t)arguments->arg[1];
  int32_t option_value = 0;
  socklen_t length     = sizeof(option_value);

  int32_t socket_status = getsockopt(socket, SOL_SOCKET, option_name, &option_value, &length);
  VERIFY_BSD_STATUS(socket_status);

  printf("%lu", option_value);

  return SL_STATUS_OK;
}

sl_status_t wifi_bsd_get_peer_name(console_args_t *arguments)
{
  int status = 0;

  int socket_id                            = (int32_t)arguments->arg[0];
  struct sockaddr_in remote_socket_address = { 0 };
  socklen_t socket_length                  = sizeof(struct sockaddr_in);

  status = getpeername(socket_id, (struct sockaddr *)&remote_socket_address, &socket_length);

  VERIFY_BSD_STATUS(status);

  if (socket_length == sizeof(struct sockaddr_in)) {
    uint8_t *ip_address = (uint8_t *)&remote_socket_address.sin_addr.s_addr;

    printf("\n\rIP address %d:%d:%d:%d", ip_address[0], ip_address[1], ip_address[2], ip_address[3]);
    printf("\n\rPort %d", remote_socket_address.sin_port);
  }

  return SL_STATUS_OK;
}
sl_status_t wifi_bsd_socket_set_opt_handler(console_args_t *arguments)
{
  int32_t socket         = (int32_t)arguments->arg[0];
  int32_t option_name    = (int32_t)arguments->arg[1];
  uint32_t option_value  = (uint32_t)arguments->arg[2];
  uint32_t option_length = (uint32_t)arguments->arg[3];

  int32_t socket_status = setsockopt(socket, SOL_SOCKET, option_name, &option_value, option_length);
  VERIFY_BSD_STATUS(socket_status);

  return SL_STATUS_OK;
}

static inline void print_errno(void)
{
  printf("errno: %d", errno);
}
