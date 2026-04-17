/***************************************************************************/ /**
 * @file  bsd_socket_commands.c
 * @brief BSD Socket AT Commands
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

#include "console.h"
#include "sl_utility.h"
#ifdef SLI_SI91X_OFFLOAD_NETWORK_STACK
#include "socket.h"
#include "errno.h"
#include "select.h"
#endif
#ifdef SLI_SI91X_LWIP_HOSTED_NETWORK_STACK
#include "lwip/sockets.h"
int errno;
#endif
#include "sl_constants.h"
#include "netdb.h"
#include "sl_net.h"
#include "sl_wifi.h"
#include "sl_si91x_socket.h"
#include "sl_si91x_socket_utility.h"
#include "at_utility.h"
#include "at_command_data_mode.h"
#include <string.h>
#include <inttypes.h>

/******************************************************
 *                    Macros
 ******************************************************/
#define UDP_MAX_RECEIVE_DATA_LENGTH 1470
#define TCP_MAX_RECEIVE_DATA_LENGTH 1460

#define INVALID_SOCK          (-1)
#define SOCK_DOMAIN_DEFAULT   AF_INET
#define SOCK_TYPE_DEFAULT     SOCK_STREAM
#define SOCK_PROTOCOL_DEFAULT IPPROTO_TCP

#define VERIFY_BSD_STATUS(status) \
  do {                            \
    if (status < 0) {             \
      return SL_STATUS_FAIL;      \
    }                             \
  } while (0)

/******************************************************
 *               Static Function Declarations
 ******************************************************/

/******************************************************
 *               Extern Functions
 ******************************************************/

/******************************************************
 *               Function Declarations
 ******************************************************/
uint8_t nfds = 0;

typedef struct {
  int fd;
  char *address;
  uint16_t port;
} server_info_t;

static server_info_t sv_info = {
  .fd      = -1,
  .address = NULL,
  .port    = 0,
};

typedef struct socket_node {
  int fd;
  int domain;
  struct socket_node *next;
} socket_node_t;

static socket_node_t *socket_list_head = NULL;

/******************************************************
 *               Socket List Management Functions
 ******************************************************/

// Add a new socket to the list
static sl_status_t socket_list_add(int fd, int domain)
{
  if (fd < 0) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  socket_node_t *current = socket_list_head;
  while (current != NULL) {
    if (current->fd == fd) {
      // Socket already exists in the list
      return SL_STATUS_OK;
    }
    current = current->next;
  }

  socket_node_t *new_node = calloc(1, sizeof(socket_node_t));
  SL_VERIFY_POINTER_OR_RETURN(new_node, SL_STATUS_ALLOCATION_FAILED);

  new_node->fd     = fd;
  new_node->domain = domain;
  new_node->next   = socket_list_head;
  socket_list_head = new_node;

  return SL_STATUS_OK;
}

// Find socket info by fd
static socket_node_t *socket_list_find(int fd)
{
  socket_node_t *current = socket_list_head;
  while (current != NULL) {
    if (current->fd == fd) {
      return current;
    }
    current = current->next;
  }
  return NULL;
}

// Get socket domain
static int socket_list_get_domain(int fd)
{
  socket_node_t *node = socket_list_find(fd);
  return (node != NULL) ? node->domain : AF_UNSPEC;
}

// Check if socket exists
static bool socket_list_exists(int fd)
{
  return (socket_list_find(fd) != NULL);
}

// Remove socket from list
static sl_status_t socket_list_remove(int fd)
{
  socket_node_t *current = socket_list_head;
  socket_node_t *prev    = NULL;

  while (current != NULL) {
    if (current->fd == fd) {
      if (prev == NULL) {
        socket_list_head = current->next;
      } else {
        prev->next = current->next;
      }
      free(current);
      return SL_STATUS_OK;
    }
    prev    = current;
    current = current->next;
  }

  return SL_STATUS_NOT_FOUND;
}

/******************************************************
 *               Function Definitions
 ******************************************************/

// at+socket=<domain>,<type>,<protocol>
sl_status_t bsd_socket_create_handler(console_args_t *arguments)
{
  CHECK_ARGUMENT_BITMAP(arguments, 0x07);

  int domain   = GET_OPTIONAL_COMMAND_ARG(arguments, 0, SOCK_DOMAIN_DEFAULT, int);
  int type     = GET_OPTIONAL_COMMAND_ARG(arguments, 1, SOCK_TYPE_DEFAULT, int);
  int protocol = GET_OPTIONAL_COMMAND_ARG(arguments, 2, SOCK_PROTOCOL_DEFAULT, int);

  int socket_fd = socket(domain, type, protocol);
  VERIFY_BSD_STATUS(socket_fd);

  sl_status_t status = socket_list_add(socket_fd, domain);
  if (status != SL_STATUS_OK) {
    close(socket_fd);
    return status;
  }

  PRINT_AT_CMD_SUCCESS;
  AT_PRINTF("%d", socket_fd);

  return SL_STATUS_OK;
}

// ?
sl_status_t bsd_socket_bind_handler(console_args_t *arguments)
{
  sl_ipv4_address_t ip = { 0 };
  char *ip_address;
  const int32_t socket_fd = (int32_t)arguments->arg[0];
#ifdef SLI_SI91X_LWIP_HOSTED_NETWORK_STACK
  const uint16_t port = htons((uint16_t)arguments->arg[2]);
#else
  const uint16_t port = (uint16_t)arguments->arg[2];
#endif
  ip_address         = (char *)arguments->arg[1];
  uint8_t ip_version = GET_OPTIONAL_COMMAND_ARG(arguments, 3, SL_IPV4_VERSION, uint8_t);
  int32_t status     = -1;

  if (ip_version == SL_IPV4_VERSION) {
    struct sockaddr_in socket_address = { 0 };
    convert_string_to_sl_ipv4_address(ip_address, &ip);
    socket_address.sin_family = AF_INET;
    socket_address.sin_port   = port;
    memcpy(&socket_address.sin_addr.s_addr, &ip.value, SL_IPV4_ADDRESS_LENGTH);

    status = bind(socket_fd, (struct sockaddr *)&socket_address, sizeof(struct sockaddr_in));
    VERIFY_BSD_STATUS(status);

  } else {
    struct sockaddr_in6 server_address6 = { 0 };
    server_address6.sin6_family         = AF_INET6;
    server_address6.sin6_port           = port;
    status = bind(socket_fd, (struct sockaddr *)&server_address6, sizeof(struct sockaddr_in6));
    VERIFY_BSD_STATUS(status);
  }

  PRINT_AT_CMD_SUCCESS;
  return SL_STATUS_OK;
}

// ?
sl_status_t bsd_socket_listen_handler(console_args_t *arguments)
{
  const int32_t socket_fd = (int32_t)arguments->arg[0];
  const int32_t backlog   = (int32_t)arguments->arg[1];
  int32_t status          = -1;

  status = listen(socket_fd, backlog);
  VERIFY_BSD_STATUS(status);

  PRINT_AT_CMD_SUCCESS;
  return SL_STATUS_OK;
}

// ?
sl_status_t bsd_socket_accept_handler(console_args_t *arguments)
{
  const int32_t socket_fd                  = (int32_t)arguments->arg[0];
  struct sockaddr_in remote_socket_address = { 0 };
  socklen_t socket_length                  = sizeof(struct sockaddr_in);
  uint8_t ip_version                       = GET_OPTIONAL_COMMAND_ARG(arguments, 1, SL_IPV4_VERSION, uint8_t);
  int32_t status                           = -1;

  if (ip_version == SL_IPV4_VERSION) {
    status = accept(socket_fd, ((struct sockaddr *)&remote_socket_address), &socket_length);
  } else {
    status = accept(socket_fd, NULL, NULL);
  }

  VERIFY_BSD_STATUS(status);

  PRINT_AT_CMD_SUCCESS;
  printf("Socket ID: %lu\n", status);
  if ((ip_version == SL_IPV4_VERSION) && (socket_length == sizeof(struct sockaddr_in))) {
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

// at+recv=<socket-id>,<remote-ip-address>,<remote-port>,<bytes-to-read>,<flags>
sl_status_t bsd_socket_receive_from_handler(console_args_t *arguments)
{
  CHECK_ARGUMENT_BITMAP(arguments, 0x19);

  int sock_fd          = GET_OPTIONAL_COMMAND_ARG(arguments, 0, INVALID_SOCK, int);
  const char *address  = GET_OPTIONAL_COMMAND_ARG(arguments, 1, NULL, const char *);
  uint16_t port        = GET_OPTIONAL_COMMAND_ARG(arguments, 2, 0, uint16_t);
  size_t bytes_to_read = GET_OPTIONAL_COMMAND_ARG(arguments, 3, 0, size_t);
  int flags            = GET_OPTIONAL_COMMAND_ARG(arguments, 4, 0, int);

  if ((sock_fd == INVALID_SOCK) || (bytes_to_read == 0)) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  // Check if socket exists in list
  int domain = socket_list_get_domain(sock_fd);
  if (domain == AF_UNSPEC) {
    return SL_STATUS_NOT_INITIALIZED;
  }

  uint8_t *buffer = calloc(1, bytes_to_read);
  if (buffer == NULL) {
    return SL_STATUS_ALLOCATION_FAILED;
  }

  ssize_t size = -1;
  sl_status_t status;
  sl_ip_address_t ip_address;

  if (domain == AF_INET) {
    status = sl_net_inet_addr(address, &ip_address.ip.v4.value);
    VERIFY_STATUS_AND_RETURN(status);

    struct sockaddr_in server_address = { 0 };
    socklen_t address_length          = sizeof(server_address);
    server_address.sin_addr.s_addr    = ip_address.ip.v4.value;
    server_address.sin_port           = (in_port_t)port;
    server_address.sin_family         = AF_INET;

    size = recvfrom(sock_fd, buffer, bytes_to_read, flags, (struct sockaddr *)&server_address, &address_length);
  } else if (domain == AF_INET6) {
    unsigned char hex_addr[SL_IPV6_ADDRESS_LENGTH] = { 0 };
    int ret = sl_inet_pton6(address, address + strlen(address), hex_addr, (unsigned int *)ip_address.ip.v6.value);
    if (ret != 0x1) {
      return SL_STATUS_INVALID_PARAMETER;
    }

    struct sockaddr_in6 server_address6 = { 0 };
    socklen_t address_length            = sizeof(server_address6);

    memcpy(&server_address6.sin6_addr.__u6_addr.__u6_addr32,
           &(ip_address.ip.v6.value),
           sizeof(server_address6.sin6_addr.__u6_addr.__u6_addr32));
    server_address6.sin6_family = AF_INET6;
    server_address6.sin6_port   = (in_port_t)port;

    size = recvfrom(sock_fd, buffer, bytes_to_read, flags, (struct sockaddr *)&server_address6, &address_length);
  } else {
    SL_CLEANUP_MALLOC(buffer);
    return SL_STATUS_NOT_SUPPORTED;
  }

  if (size < 0) {
    SL_CLEANUP_MALLOC(buffer);
    return size;
  }

  PRINT_AT_CMD_SUCCESS;
  printf("%d ", size);
  at_print_char_buffer((char *)buffer, size);

  SL_CLEANUP_MALLOC(buffer);

  return SL_STATUS_OK;
}

static sl_status_t setup_server_info(server_info_t *info, int fd, const char *address, uint16_t port)
{
  if (info == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  info->fd   = fd;
  info->port = port;

  // Clear before new allocate
  SL_CLEANUP_MALLOC(info->address);

  if (address != NULL) {
    size_t addr_len = strlen(address);
    info->address   = calloc(1, addr_len + 1);
    if (info->address == NULL) {
      return SL_STATUS_ALLOCATION_FAILED;
    }

    strncpy(info->address, address, addr_len);
    info->address[addr_len] = '\0';
  }

  return SL_STATUS_OK;
}

static sl_status_t bsd_socket_sendto(server_info_t *info, uint8_t *buffer, size_t data_len)
{
  if ((info == NULL) || (buffer == NULL)) {
    return SL_STATUS_NULL_POINTER;
  }

  ssize_t size;
  if (info->address != NULL) {
    sl_ip_address_t ip_address = { 0 };
    sl_status_t status         = sl_net_inet_addr_auto(info->address, &ip_address);
    VERIFY_STATUS_AND_RETURN(status);

    // Get domain from linked list
    int domain = socket_list_get_domain(info->fd);
    if (domain == AF_UNSPEC) {
      return SL_STATUS_NOT_INITIALIZED;
    }

    // Verify with sock_domain
    if ((ip_address.type == SL_IPV4 && domain != AF_INET) || (ip_address.type == SL_IPV6 && domain != AF_INET6)) {
      return SL_STATUS_INVALID_PARAMETER;
    }

    if (ip_address.type == SL_IPV4) {
      struct sockaddr_in server_address = { 0 };

      server_address.sin_addr.s_addr = ip_address.ip.v4.value;
      server_address.sin_port        = (in_port_t)info->port;
      server_address.sin_family      = AF_INET;

      size = sendto(info->fd, buffer, data_len, 0, (const struct sockaddr *)&server_address, sizeof(server_address));
      VERIFY_BSD_STATUS(size);
    } else if (ip_address.type == SL_IPV6) {
      struct sockaddr_in6 server_address6 = { 0 };

      memcpy(&server_address6.sin6_addr.__u6_addr.__u6_addr32,
             &(ip_address.ip.v6.value),
             sizeof(server_address6.sin6_addr.__u6_addr.__u6_addr32));
      server_address6.sin6_family = AF_INET6;
      server_address6.sin6_port   = (in_port_t)info->port;

      size = sendto(info->fd, buffer, data_len, 0, (const struct sockaddr *)&server_address6, sizeof(server_address6));
      VERIFY_BSD_STATUS(size);
    } else {
      return SL_STATUS_NOT_SUPPORTED;
    }
  } else {
    // Get domain from linked list
    int domain = socket_list_get_domain(info->fd);
    if (domain == AF_UNSPEC) {
      return SL_STATUS_NOT_INITIALIZED;
    }

    size = sendto(info->fd, buffer, data_len, 0, NULL, 0);
    VERIFY_BSD_STATUS(size);
  }

  return SL_STATUS_OK;
}

static sl_status_t bsd_socket_send_buffer_handler(uint8_t *buffer, uint32_t length, void *user_data)
{
  server_info_t *info = (server_info_t *)user_data;
  if (info == NULL) {
    return SL_STATUS_NULL_POINTER;
  }

  sl_status_t status = bsd_socket_sendto(info, buffer, (size_t)length);
  SL_CLEANUP_MALLOC(info->address);
  VERIFY_STATUS_AND_RETURN(status);

  PRINT_AT_CMD_SUCCESS;
  return SL_STATUS_OK;
}

// Old: at+send=<socket-id>,<remote-ip-address>,<remote-port>,<data-length>,<data>
// New: at+send=<socket-id>,<remote-ip-address>,<remote-port>,<data-length>
sl_status_t bsd_socket_send_to_handler(console_args_t *arguments)
{
  CHECK_ARGUMENT_BITMAP(arguments, 0x09);

  sl_status_t status  = SL_STATUS_OK;
  int sock_fd         = GET_OPTIONAL_COMMAND_ARG(arguments, 0, INVALID_SOCK, int);
  const char *address = GET_OPTIONAL_COMMAND_ARG(arguments, 1, NULL, const char *);
  uint16_t port       = GET_OPTIONAL_COMMAND_ARG(arguments, 2, 0, uint16_t);
  size_t data_len     = GET_OPTIONAL_COMMAND_ARG(arguments, 3, 0, size_t);
  const char *buffer  = GET_OPTIONAL_COMMAND_ARG(arguments, 4, NULL, const char *);

  if ((sock_fd == INVALID_SOCK) || (data_len == 0)) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  if (!socket_list_exists(sock_fd)) {
    return SL_STATUS_NOT_INITIALIZED;
  }

  if (buffer == NULL) { // Data Mode
    status = setup_server_info(&sv_info, sock_fd, address, port);
    VERIFY_STATUS_AND_RETURN(status);
    status = at_command_goto_data_mode(bsd_socket_send_buffer_handler, data_len, &sv_info);
  } else { // Old version
    server_info_t info = { 0 };
    status             = setup_server_info(&info, sock_fd, address, port);
    VERIFY_STATUS_AND_RETURN(status);
    data_len = data_len > strlen(buffer) ? strlen(buffer) : data_len;
    status   = bsd_socket_sendto(&info, (uint8_t *)buffer, data_len);
    SL_CLEANUP_MALLOC(info.address);
  }

  VERIFY_STATUS_AND_RETURN(status);

  PRINT_AT_CMD_SUCCESS;
  return SL_STATUS_OK;
}

// at+connect=<socket-id>,<remote-ip-address>,<remote-port>
sl_status_t bsd_socket_connect_handler(console_args_t *arguments)
{
  CHECK_ARGUMENT_BITMAP(arguments, 0x07);

  int sock_fd         = GET_OPTIONAL_COMMAND_ARG(arguments, 0, INVALID_SOCK, int);
  const char *address = GET_OPTIONAL_COMMAND_ARG(arguments, 1, NULL, const char *);
  uint16_t port       = GET_OPTIONAL_COMMAND_ARG(arguments, 2, 0, uint16_t);

  if ((sock_fd == INVALID_SOCK) || (address == NULL)) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  // Get domain from linked list
  int domain = socket_list_get_domain(sock_fd);
  if (domain == AF_UNSPEC) {
    return SL_STATUS_NOT_INITIALIZED;
  }

  sl_status_t status;
  sl_ip_address_t ip_address;

  if (domain == AF_INET) {
    status = sl_net_inet_addr(address, &ip_address.ip.v4.value);
    VERIFY_STATUS_AND_RETURN(status);

    struct sockaddr_in server_address = { 0 };
    server_address.sin_addr.s_addr    = ip_address.ip.v4.value;
    server_address.sin_port           = (in_port_t)port;
    server_address.sin_family         = AF_INET;

    int ret = connect(sock_fd, (const struct sockaddr *)&server_address, sizeof(server_address));
    VERIFY_BSD_STATUS(ret);
  } else if (domain == AF_INET6) {
    unsigned char hex_addr[SL_IPV6_ADDRESS_LENGTH] = { 0 };
    int ret = sl_inet_pton6(address, address + strlen(address), hex_addr, (unsigned int *)ip_address.ip.v6.value);
    if (ret != 0x1) {
      return SL_STATUS_INVALID_PARAMETER;
    }

    struct sockaddr_in6 server_address6 = { 0 };

    memcpy(&server_address6.sin6_addr.__u6_addr.__u6_addr32,
           &(ip_address.ip.v6.value),
           sizeof(server_address6.sin6_addr.__u6_addr.__u6_addr32));
    server_address6.sin6_family = AF_INET6;
    server_address6.sin6_port   = (in_port_t)port;

    ret = connect(sock_fd, (const struct sockaddr *)&server_address6, sizeof(server_address6));
    VERIFY_BSD_STATUS(ret);
  } else {
    return SL_STATUS_NOT_SUPPORTED;
  }

  PRINT_AT_CMD_SUCCESS;
  return SL_STATUS_OK;
}

// at+close=<socket-id>
sl_status_t bsd_socket_close_handler(console_args_t *arguments)
{
  CHECK_ARGUMENT_BITMAP(arguments, 0x01);

  int sock_fd = GET_OPTIONAL_COMMAND_ARG(arguments, 0, INVALID_SOCK, int);

  if (sock_fd == INVALID_SOCK) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  // Check if socket exists
  if (!socket_list_exists(sock_fd)) {
    return SL_STATUS_NOT_FOUND;
  }

  int status = close(sock_fd);
  VERIFY_BSD_STATUS(status);

  // Remove socket from list
  socket_list_remove(sock_fd);

  PRINT_AT_CMD_SUCCESS;
  return SL_STATUS_OK;
}

// ?
sl_status_t bsd_socket_get_host_by_name_handler(console_args_t *arguments)
{
#ifndef SLI_SI91X_LWIP_HOSTED_NETWORK_STACK
  char *host_name = (char *)arguments->arg[0];

  struct hostent *host_ent = gethostbyname(host_name);
  if (host_ent == NULL) {
    return SL_STATUS_FAIL;
  }

  PRINT_AT_CMD_SUCCESS;
  print_sl_ip_address((sl_ip_address_t *)host_ent->h_addr);

  printf("\r\nhost_name: %s, h_errno:%d, type:%d, length:%d",
         host_name,
         h_errno,
         host_ent->h_addrtype,
         host_ent->h_length);

  return SL_STATUS_OK;
#else
  //! Not supported in LWIP(external stack)..
  return SL_STATUS_NOT_SUPPORTED;
#endif
}

// ?
sl_status_t bsd_socket_get_sock_name(console_args_t *arguments)
{
  int status = 0;

  int socket_id                     = (int32_t)arguments->arg[0];
  struct sockaddr_in socket_address = { 0 };
  socklen_t socket_length           = sizeof(struct sockaddr_in);

  status = getsockname(socket_id, (struct sockaddr *)&socket_address, &socket_length);

  VERIFY_BSD_STATUS(status);

  PRINT_AT_CMD_SUCCESS;
  if (socket_length == sizeof(struct sockaddr_in)) {
    const uint8_t *ip_address = (uint8_t *)&socket_address.sin_addr.s_addr;

    printf("%u.%u.%u.%u", ip_address[0], ip_address[1], ip_address[2], ip_address[3]);
    printf(":%d\r\n", socket_address.sin_port);
  }

  return SL_STATUS_OK;
}

// ?
sl_status_t bsd_socket_select_handler(console_args_t *arguments)
{
  fd_set readfds;
  nfds                    = (uint8_t)GET_OPTIONAL_COMMAND_ARG(arguments, 0, 5, uint32_t);
  struct timeval timeout  = { .tv_sec = GET_OPTIONAL_COMMAND_ARG(arguments, 1, 5, uint32_t), .tv_usec = 0 };
  uint8_t timeout_disable = (uint8_t)GET_OPTIONAL_COMMAND_ARG(arguments, 2, 0, uint8_t);
  int32_t retval          = SL_STATUS_OK;

  FD_ZERO(&readfds);
  for (int i = 0; i < nfds; i++) {
    FD_SET(i, &readfds);
  }

  if (timeout_disable) {
    retval = select(nfds + 1, &readfds, NULL, NULL, NULL);
  } else {
    retval = select(nfds + 1, &readfds, NULL, NULL, &timeout);
  }
  VERIFY_BSD_STATUS(retval);

  PRINT_AT_CMD_SUCCESS;
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

// at+setsockopt=<socket-id>,<option-level>,<option-name>,<option-value-1>,<option-value-2>,...
sl_status_t bsd_socket_setsockopt_handler(console_args_t *arguments)
{
  CHECK_ARGUMENT_BITMAP(arguments, 0x0F);

  int sock_fd              = GET_OPTIONAL_COMMAND_ARG(arguments, 0, INVALID_SOCK, int);
  int option_level         = GET_OPTIONAL_COMMAND_ARG(arguments, 1, SOL_SOCKET, int);
  int option_name          = GET_OPTIONAL_COMMAND_ARG(arguments, 2, 0, int);
  const char *option_value = (char *)GET_OPTIONAL_COMMAND_ARG(arguments, 3, NULL, char *);
  uint16_t option_val      = 0;
  int socket_status        = 0;

  if ((sock_fd == INVALID_SOCK) || (option_value == NULL)) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  // Check if socket exists
  if (!socket_list_exists(sock_fd)) {
    return SL_STATUS_NOT_FOUND;
  }

  if (option_name == TCP_ULP) {
    socket_status = setsockopt(sock_fd, option_level, option_name, option_value, (strlen(option_value) + 1));
  } else if (option_name == SO_RCVTIMEO) {
    sl_si91x_time_value timeout = { 0 };
    timeout.tv_sec              = (uint32_t)atoi(option_value);
    timeout.tv_usec             = GET_OPTIONAL_COMMAND_ARG(arguments, 4, 0, uint32_t);
    socket_status               = setsockopt(sock_fd, option_level, option_name, &timeout, sizeof(timeout));
  } else {
    option_val    = (uint16_t)atoi(option_value);
    socket_status = setsockopt(sock_fd, option_level, option_name, &option_val, sizeof(option_val));
  }

  VERIFY_BSD_STATUS(socket_status);

  PRINT_AT_CMD_SUCCESS;
  return SL_STATUS_OK;
}
