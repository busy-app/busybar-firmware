/*******************************************************************************
* @file  iot_socket_commands.c
* @brief 
*******************************************************************************
* # License
* <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
*******************************************************************************
*
* The licensor of this software is Silicon Laboratories Inc. Your use of this
* software is governed by the terms of Silicon Labs Master Software License
* Agreement (MSLA) available at
* www.silabs.com/about-us/legal/master-software-license-agreement. This
* software is distributed to you in Source Code format and is governed by the
* sections of the MSLA applicable to Source Code.
*
******************************************************************************/

/** @file
 *
 * File Description
 *
 */
#include "console.h"
#include "iot_socket.h"
#include "sl_ip_types.h"
#include "sl_constants.h"
#include "sl_utility.h"
#include "sl_wifi.h"
#include <string.h>
#include <stdio.h>

/******************************************************
 *                    Macros
 ******************************************************/

#define VERIFY_IOT_STATUS(status) \
  do {                            \
    if (status < 0) {             \
      print_iot_status(status);   \
      return SL_STATUS_FAIL;      \
    }                             \
  } while (0)

/******************************************************
 *                    Constants
 ******************************************************/

#define MAX_SOCK_SEND_BUF_LENGTH 1460

/******************************************************
 *               Static Function Declarations
 ******************************************************/

static inline void print_ipv6(const sl_ipv6_address_t *ip);
static inline void print_iot_status(const int32_t status);

/******************************************************
 *               Static Variable Declarations
 ******************************************************/

static uint8_t socket_buffer[500] = { 0 };

/******************************************************
 *               Extern Functions
 ******************************************************/

/******************************************************
 *               Function Definitions
 ******************************************************/

sl_status_t wifi_iot_socket_create_handler(console_args_t *arguments)
{
  int32_t type     = (int32_t)arguments->arg[0];
  int32_t protocol = (int32_t)arguments->arg[1];
  int32_t domain   = GET_OPTIONAL_COMMAND_ARG(arguments, 2, IOT_SOCKET_AF_INET, int32_t);

  if (!sl_wifi_is_interface_up(SL_WIFI_ALL_INTERFACES)) {
    return SL_STATUS_WIFI_INTERFACE_NOT_UP;
  }

  int32_t sock_fd = iotSocketCreate(domain, type, protocol);
  VERIFY_IOT_STATUS(sock_fd);

  printf("%ld", sock_fd);

  return SL_STATUS_OK;
}

sl_status_t wifi_iot_socket_close_handler(console_args_t *arguments)
{
  int32_t sock_fd = (int32_t)arguments->arg[0];

  int32_t iot_socket_status = iotSocketClose(sock_fd);
  VERIFY_IOT_STATUS(iot_socket_status);

  return SL_STATUS_OK;
}

sl_status_t wifi_iot_socket_bind_handler(console_args_t *arguments)
{
  int32_t sock_fd       = (int32_t)arguments->arg[0];
  sl_ipv4_address_t *ip = (sl_ipv4_address_t *)&arguments->arg[1];
  uint16_t port         = (uint16_t)arguments->arg[2];
  uint32_t ip_length    = GET_OPTIONAL_COMMAND_ARG(arguments, 3, sizeof(sl_ipv4_address_t), uint32_t);

  int32_t iot_socket_status = iotSocketBind(sock_fd, ip->bytes, ip_length, port);
  VERIFY_IOT_STATUS(iot_socket_status);

  return SL_STATUS_OK;
}

sl_status_t wifi_iot_socket_listen_handler(console_args_t *arguments)
{
  int32_t sock_fd = (int32_t)arguments->arg[0];
  int32_t backlog = (uint32_t)arguments->arg[1];

  int32_t iot_socket_status = iotSocketListen(sock_fd, backlog);
  VERIFY_IOT_STATUS(iot_socket_status);

  return SL_STATUS_OK;
}

sl_status_t wifi_iot_socket_accept_handler(console_args_t *arguments)
{
  int32_t sock_fd      = (int32_t)arguments->arg[0];
  sl_ipv4_address_t ip = { 0 };
  uint32_t ip_length   = SL_IPV4_ADDRESS_LENGTH;
  uint16_t port        = 0;

  int32_t iot_socket_status = iotSocketAccept(sock_fd, ip.bytes, &ip_length, &port);
  VERIFY_IOT_STATUS(iot_socket_status);

  // print client socket fd
  printf("%ld\n", iot_socket_status);

  return SL_STATUS_OK;
}

sl_status_t wifi_iot_socket_connect_handler(console_args_t *arguments)
{
  int32_t sock_fd       = (int32_t)arguments->arg[0];
  sl_ipv4_address_t *ip = (sl_ipv4_address_t *)&arguments->arg[1];
  uint16_t port         = (uint16_t)arguments->arg[2];
  uint32_t ip_length    = GET_OPTIONAL_COMMAND_ARG(arguments, 3, sizeof(sl_ipv4_address_t), uint32_t);

  int32_t iot_socket_status = iotSocketConnect(sock_fd, ip->bytes, ip_length, port);
  VERIFY_IOT_STATUS(iot_socket_status);

  return SL_STATUS_OK;
}

sl_status_t wifi_iot_socket_send_handler(console_args_t *arguments)
{
  int32_t sock_fd = (int32_t)arguments->arg[0];
  uint8_t *buffer = (uint8_t *)arguments->arg[1];

  int32_t iot_socket_status = iotSocketSend(sock_fd, buffer, strlen((char *)buffer));
  VERIFY_IOT_STATUS(iot_socket_status);

  printf("%ld bytes sent", iot_socket_status);
  return SL_STATUS_OK;
}

sl_status_t wifi_iot_socket_get_socket_name_handler(console_args_t *arguments)
{
  int32_t sock_fd      = (int32_t)arguments->arg[0];
  sl_ipv4_address_t ip = { 0 };
  uint32_t ip_length   = SL_IPV4_ADDRESS_LENGTH;
  uint16_t port        = 0;

  int32_t iot_socket_status = iotSocketGetSockName(sock_fd, ip.bytes, &ip_length, &port);
  VERIFY_IOT_STATUS(iot_socket_status);

  printf("%u.%u.%u.%u:%u", ip.bytes[0], ip.bytes[1], ip.bytes[2], ip.bytes[3], port);

  return SL_STATUS_OK;
}

sl_status_t wifi_iot_socket_get_peer_name_handler(console_args_t *arguments)
{
  int32_t sock_fd      = (int32_t)arguments->arg[0];
  sl_ipv4_address_t ip = { 0 };
  uint32_t ip_length   = SL_IPV4_ADDRESS_LENGTH;
  uint16_t port        = 0;

  int32_t iot_socket_status = iotSocketGetPeerName(sock_fd, ip.bytes, &ip_length, &port);
  VERIFY_IOT_STATUS(iot_socket_status);

  printf("%u.%u.%u.%u:%u", ip.bytes[0], ip.bytes[1], ip.bytes[2], ip.bytes[3], port);

  return SL_STATUS_OK;
}

sl_status_t wifi_iot_socket_set_opt_handler(console_args_t *arguments)
{
  int32_t socket      = (int32_t)arguments->arg[0];
  int32_t option_id   = (int32_t)arguments->arg[1];
  uint32_t option_val = (uint32_t)arguments->arg[2];
  uint32_t length     = sizeof(option_val);

  int32_t iot_socket_status = iotSocketSetOpt(socket, option_id, &option_val, length);
  VERIFY_IOT_STATUS(iot_socket_status);

  return SL_STATUS_OK;
}

sl_status_t wifi_iot_socket_get_opt_handler(console_args_t *arguments)
{
  int32_t socket      = (int32_t)arguments->arg[0];
  int32_t option_id   = (int32_t)arguments->arg[1];
  uint32_t option_val = 0;
  uint32_t length     = 0;

  int32_t iot_socket_status = iotSocketGetOpt(socket, option_id, &option_val, &length);
  VERIFY_IOT_STATUS(iot_socket_status);

  printf("%lu", option_val);

  return SL_STATUS_OK;
}

sl_status_t wifi_iot_socket_receive_handler(console_args_t *arguments)
{
  int32_t sock_fd = (int32_t)arguments->arg[0];

  int32_t iot_socket_status = iotSocketRecv(sock_fd, socket_buffer, sizeof(socket_buffer));
  VERIFY_IOT_STATUS(iot_socket_status);

  printf("Received:%s", socket_buffer);
  memset(socket_buffer, 0, sizeof(socket_buffer));

  return SL_STATUS_OK;
}

sl_status_t wifi_iot_socket_receive_from_handler(console_args_t *arguments)
{
  int32_t sock_fd      = (int32_t)arguments->arg[0];
  uint16_t port        = 0;
  uint32_t ip_length   = SL_IPV4_ADDRESS_LENGTH;
  sl_ipv4_address_t ip = { 0 };

  int32_t iot_socket_status =
    iotSocketRecvFrom(sock_fd, socket_buffer, sizeof(socket_buffer), ip.bytes, &ip_length, &port);
  VERIFY_IOT_STATUS(iot_socket_status);

  if (ip_length == SL_IPV4_ADDRESS_LENGTH) {
    printf("remote:%u.%u.%u.%u:%u\r\n", ip.bytes[0], ip.bytes[1], ip.bytes[2], ip.bytes[3], port);
  }
  printf("Received:%s", socket_buffer);
  memset(socket_buffer, 0, sizeof(socket_buffer));

  return SL_STATUS_OK;
}

sl_status_t wifi_iot_get_host_by_name_handler(console_args_t *arguments)
{
  uint8_t ip_address[16] = { 0 };
  char *name             = (char *)arguments->arg[0];
  int32_t af             = (int32_t)arguments->arg[1];
  uint32_t ip_length     = GET_OPTIONAL_COMMAND_ARG(arguments, 2, sizeof(sl_ipv4_address_t), uint32_t);

  int32_t iot_socket_status = iotSocketGetHostByName(name, af, ip_address, &ip_length);
  VERIFY_IOT_STATUS(iot_socket_status);

  print_sl_ip_address((sl_ip_address_t *)&ip_address);

  return SL_STATUS_OK;
}

sl_status_t wifi_iot_socket_send_to_handler(console_args_t *arguments)
{
  int32_t sock_fd       = (int32_t)arguments->arg[0];
  uint8_t *buffer       = (uint8_t *)arguments->arg[1];
  sl_ipv4_address_t *ip = (sl_ipv4_address_t *)&arguments->arg[2];
  uint16_t port         = (uint16_t)arguments->arg[3];
  uint32_t ip_length    = GET_OPTIONAL_COMMAND_ARG(arguments, 4, sizeof(sl_ipv4_address_t), uint32_t);
  uint32_t length       = strlen((char *)buffer);

  int32_t iot_socket_status = iotSocketSendTo(sock_fd, buffer, length, ip->bytes, ip_length, port);
  VERIFY_IOT_STATUS(iot_socket_status);

  printf("%ld bytes sent", iot_socket_status);

  return SL_STATUS_OK;
}

static inline void print_ipv6(const sl_ipv6_address_t *ip)
{
  printf("%lx:%lx:%lx:%lx", ip->value[0], ip->value[1], ip->value[2], ip->value[3]);
}

static inline void print_iot_status(const int32_t status)
{
  printf("Error:%ld", status);
}
