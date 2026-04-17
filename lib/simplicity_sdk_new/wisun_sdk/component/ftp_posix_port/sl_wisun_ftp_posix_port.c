/***************************************************************************//**
 * @file sl_wisun_ftp_posix_port.c
 * @brief Wi-SUN POSIX File Transfer Protocol portable implementation
 *******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
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

// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------
#include "sl_ftp.h"
#include "socket/socket.h"
#include "cmsis_os2.h"
#include "sl_wisun_app_core_util.h"
#include "sl_wisun_types.h"
#include "sl_wisun_api.h"
#include "sl_wisun_trace_util.h"
#include "sl_memory_manager.h"
#include "sl_string.h"

#if SL_FTP_ENABLE_TFTP_PROTOCOL
#include "sl_tftp_clnt.h"
#endif
// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------
#define IPV6_STR_BUF_SIZE     (40U)
// -----------------------------------------------------------------------------
//                          Static Function Declarations
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                                Static Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------

/***************************************************************************//**
 * @brief Delay ms
 * @details Portable function
 * @param delay_ms Milliseconds
 ******************************************************************************/
void sl_ftp_delay_ms(const uint32_t delay_ms)
{
  (void) osDelay(delay_ms);
}

/***************************************************************************//**
 * @brief FTP network is connected
 * @details Portable function
 * @return true if network is connected, otherwise false
 ******************************************************************************/
bool sl_ftp_is_network_connected(void)
{
  return sl_wisun_app_core_util_network_is_connected();
}

/***************************************************************************//**
 * @brief FTP wait for network connection
 * @details Portable function
 ******************************************************************************/
void sl_ftp_wait_for_connection(void)
{
  sl_wisun_app_core_util_wait_for_connection();
}

/***************************************************************************//**
 * @brief FTP get global IP address
 * @details Portable function
 * @return Global address string representation
 ******************************************************************************/
const char * sl_ftp_get_global_addr(void)
{
  static char str[IPV6_STR_BUF_SIZE] = { 0U };
  in6_addr_t addr = { 0U };

  // get current address
  if (sl_wisun_get_ip_address(SL_WISUN_IP_ADDRESS_TYPE_GLOBAL, &addr) != SL_STATUS_OK) {
    sl_strcpy_s(str, IPV6_STR_BUF_SIZE, SL_FTP_LOCAL_HOST_STR);
    return (const char *) str;
  }

  // get string
  if (inet_ntop(AF_INET6, (const void *)&addr, str, IPV6_STR_BUF_SIZE) == NULL) {
    sl_strcpy_s(str, IPV6_STR_BUF_SIZE, SL_FTP_LOCAL_HOST_STR);
  }

  return (const char *) str;
}

/***************************************************************************//**
 * @brief FTP Close
 * @details Portable function
 * @param sockid Socket ID
 * @return int32_t Closed socket ID on success, -1 on error
 ******************************************************************************/
int32_t sl_ftp_socket_close(int32_t sockid)
{
  return close(sockid);
}

#if SL_FTP_ENABLE_FTP_PROTOCOL

/***************************************************************************//**
 * @brief FTP TCP socket create
 * @details Portable function
 * @return int32_t Socket ID
 ******************************************************************************/
int32_t sl_ftp_tcp_socket_create(void)
{
  return socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
}

/***************************************************************************//**
 * @brief FTP TCP socket connect
 * @details Portable function
 * @param[in] sockid Socket ID
 * @param[in] host Host address string
 * @param[in] port Host port
 * @return int32_t 0 on success, -1 on error
 ******************************************************************************/
int32_t sl_ftp_tcp_socket_connect(int32_t sockid, const char *host, uint16_t port)
{
  sockaddr_in6_t waddr = { 0U };

  if (inet_pton(AF_INET6, host,
                &waddr.sin6_addr) != 1) {
    return SOCKET_RETVAL_ERROR;
  }

  waddr.sin6_family = AF_INET6;
  waddr.sin6_port = htons(port);

  return connect(sockid, (const struct sockaddr *)&waddr, sizeof(sockaddr_in6_t));
}

/***************************************************************************//**
 * @brief FTP TCP send
 * @details Portable function
 * @param sockid Socket ID
 * @param buff Buffer to send
 * @param len Length of buffer
 * @return int32_t Count of sent bytes on success, -1 on error
 ******************************************************************************/
int32_t sl_ftp_tcp_socket_send(int32_t sockid, const void *buff, uint32_t len)
{
  return send(sockid, buff, len, 0L);
}

/***************************************************************************//**
 * @brief FTP TCP receive
 * @details Portable function
 * @param sockid Socket ID
 * @param buff Buffer storage for receiving
 * @param len Length of buffer
 * @return int32_t Count of received bytes on success, -1 on error
 ******************************************************************************/
int32_t sl_ftp_tcp_socket_recv(int32_t sockid, void *buff, uint32_t len)
{
  return recv(sockid, buff, len, 0L);
}
#endif // SL_FTP_ENABLE_FTP_PROTOCOL

#if SL_FTP_ENABLE_TFTP_PROTOCOL

/***************************************************************************//**
 * @brief TFTP udp socket create
 * @details Portable function
 * @return int32_t Socket ID
 ******************************************************************************/
int32_t sl_tftp_udp_socket_create(void)
{
  return socket(AF_INET6, SOCK_DGRAM | SOCK_NONBLOCK, IPPROTO_UDP);
}

/**************************************************************************//**
 * @brief TFTP Send a message on a UDP socket.
 * @details Preferred in datagram mode (UDP).
 * @param[in] sockid socket ID
 * @param[in] buff buffer pointer to send
 * @param[in] len length of data to send
 * @param[in] dest_addr destination address ptr, the structure must be prepared for UDP sockets
 * @return On success, these calls return the number of bytes sent. On error, -1 is returned
 *****************************************************************************/
int32_t sl_tftp_udp_sendto(int32_t sockid, const void *buff, uint32_t len, const void *dest_addr)
{
  return sendto(sockid, buff, len, 0L, (const struct sockaddr *)dest_addr, sizeof(sockaddr_in6_t));
}

/**************************************************************************//**
 * @brief TFTP Receive messages on a UDP socket
 * @details Receives data on a socket whether or not it is connection-oriented.
 * @param[in] sockid socket id
 * @param[out] buff destination buffer ptr
 * @param[in] len length of data to read
 * @param[out] src_addr Source address
 * @return return the number of bytes received, or -1 if an error occurred
 *****************************************************************************/
int32_t sl_tftp_udp_recvfrom(int32_t sockid, void *buff, uint32_t len, void *src_addr)
{
  socklen_t addr_len = sizeof(sockaddr_in6_t);
  return recvfrom(sockid, buff, len, 0L, (struct sockaddr *)src_addr, &addr_len);
}

void * sl_tftp_udp_get_addr(const char *host,
                            uint16_t port)
{
  sockaddr_in6_t *waddr = NULL;

  waddr = (sockaddr_in6_t *)sl_malloc(sizeof(sockaddr_in6_t));
  if (waddr == NULL) {
    return NULL;
  }

  if (inet_pton(AF_INET6, host, &waddr->sin6_addr) != 1) {
    sl_free(waddr);
    return NULL;
  }

  waddr->sin6_family = AF_INET6;
  waddr->sin6_port = htons(port);

  return (void *)waddr;
}

void sl_tftp_udp_free_addr(void *addr)
{
  sl_free(addr);
}

#endif // SL_FTP_ENABLE_TFTP_PROTOCOL
