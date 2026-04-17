/***************************************************************************/ /**
 * @file
 * @brief Networking constants
 *******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
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

/// Enumerate IP configuration modes
typedef enum {
  SLI_NET_STATIC_IP = 0, ///< Static
  SLI_NET_DHCP,          ///< DHCP
  SLI_NET_DHCP_RESERVED, ///< DHCP Reserved
  SLI_NET_DHCP_HOSTNAME, ///< DHCP HostName
  SLI_NET_DHCP_OPTION81, ///< DHCP option 81
  SLI_NET_DHCP_OPTION77  ///< DHCP option 77
} sli_net_ip_config_mode_t;

/// Enumeration representing network events.
typedef enum {
  SLI_NET_CONNECT_Q_EVENT = 0,
  SLI_NET_DISCONNECT_Q_EVENT,
  SLI_NET_CONNECT_FAILURE_Q_EVENT,
  SLI_NET_ASYNC_UP_Q_EVENT,

  SLI_NET_AUTO_JOIN_Q_EVENT,
  SLI_NET_AUTO_JOIN_RETRY_Q_EVENT,
  SLI_NET_THREAD_TERMINATE_Q_EVENT,
} sli_net_queue_event_t;

/// Enumeration representing response flags.
typedef enum {
  SLI_NET_RSP_FLAG_AUTO_JOIN_SUCCESS    = (1u << 0), ///< Auto-join completed successfully
  SLI_NET_RSP_FLAG_AUTO_JOIN_FAILURE    = (1u << 1), ///< Auto-join failed
  SLI_NET_RSP_FLAG_THREAD_TERMINATE_ACK = (1u << 2), ///< Network manager thread acknowledged terminate
} sli_net_response_flags_t;

/// @brief Delay in milliseconds before auto-join retry attempt.
#ifndef SLI_NET_AUTO_JOIN_RETRY_DELAY_MS
#define SLI_NET_AUTO_JOIN_RETRY_DELAY_MS 30000u
#endif

/// @brief Message priority levels for the network manager queues.
typedef enum { SLI_NET_MSG_PRIO_NORMAL = 0u, SLI_NET_MSG_PRIO_HIGH = 1u } sli_net_msg_prio_t;

/// @brief Maximum number of messages in network manager queues (request and response).
/// @note Can be overridden by defining this macro before including the header.
#ifndef SLI_NETWORK_MANAGER_QUEUE_DEPTH
#define SLI_NETWORK_MANAGER_QUEUE_DEPTH 10
#endif