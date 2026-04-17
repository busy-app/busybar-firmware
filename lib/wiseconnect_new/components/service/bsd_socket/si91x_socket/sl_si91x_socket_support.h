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
#ifndef SL_SI91X_SOCKET_SUPPORT_INCLUDED_H
#define SL_SI91X_SOCKET_SUPPORT_INCLUDED_H

#include <string.h>
#include <stdint.h>

#ifndef __ZEPHYR__
#include "socket.h"
#else
#include <sys/socket.h>
#endif
#include "sl_status.h"
#include "sl_si91x_protocol_types.h"

/**
 * @addtogroup SI91X_SOCKET_FUNCTIONS
 * @{ 
 */

///< A macro to define the size of array in sl_si91x_socket_info_response_t to hold socket data.
#define SL_SI91X_SOCKET_INFO_RESPONSE_SOCKETS_COUNT 20

/**
 * @brief Structure for query SiWx91x socket information.
 *
 * @details
 * The structure queries the information about a specific socket in the SiWx91x series.
 * It includes details such as: socket identifier, type, source and destination ports,
 * and the IP address of the remote host. The IP address can be either IPv4 or IPv6,
 * determined by the context structure which is used.
 */
typedef struct {
  uint8_t sock_id[2]; ///< Socket Identifier.

  uint8_t sock_type[2]; ///< Socket type (for example,TCP, and UDP).

  uint8_t source_port[2]; ///< Source port number used by the socket.

  uint8_t dest_port[2]; ///< Destination port number used by the socket.

  union {
    uint8_t ipv4_address[4]; ///< IPv4 address of the remote host.

    uint8_t ipv6_address[16]; ///< IPv6 address of the remote host.
  } dest_ip_address;          ///< IP address of the destination host.
} sl_si91x_sock_info_query_t;

/**
 * @brief Structure for SiWx91x socket information response.
 *
 * @details
 * The structure holds the response information for a query about the currently opened sockets
 * in the SiWx91x series. It includes the total number of opened sockets and an array containing
 * detailed information about each socket.
 */
typedef struct {
  uint16_t number_of_opened_sockets; ///< Total number of currently opened sockets.

  sl_si91x_sock_info_query_t
    socket_info[SL_SI91X_SOCKET_INFO_RESPONSE_SOCKETS_COUNT]; ///< Array contains information about each open socket.
} sl_si91x_socket_info_response_t;

/**
 * @brief
 * Retrieve information about currently opened sockets from the socket management system.
 * @param[out] socket_info_response
 *    Pointer to a sl_si91x_socket_info_response_t structure that will hold the response from the socket management system.
 * @return 
 * sl_status_t
*/
sl_status_t sl_si91x_get_socket_info(sl_si91x_socket_info_response_t *socket_info_response);
/** @} */

#endif //SL_SI91X_SOCKET_SUPPORT_INCLUDED_H
