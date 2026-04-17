/***************************************************************************/ /**
 * @file
 * @brief Networking Types
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
#pragma once
#include "sl_net_constants.h"
#include "sli_net_constants.h"

/**
 * @brief Network Manager Message structure.
 * 
 * @details
 * This structure is used for communication with the network manager thread.
 * It encapsulates details about the network interface, event flags.
 * 
 */
typedef struct {
  sl_net_interface_t interface;      ///< Identifier for the network interface
  sli_net_queue_event_t event_flags; ///< Event flags for the network manager
  sl_net_profile_id_t profile_id;    ///< Profile ID associated with the event
} sli_network_manager_message_t;

/**
 * @struct sli_net_nat_config_t
 * @brief NAT configuration structure.
 *
 * @details
 * This structure contains configuration parameters for Network Address Translation (NAT)
 * on a network interface. It includes an enable flag, session timeout values for TCP and
 * non-TCP connections, and the associated network interface.
 */
typedef struct {
  uint8_t enable;                   ///< Enable or disable NAT (1 = enable, 0 = disable)
  uint8_t reserved[3];              ///< Reserved for alignment/future use
  uint32_t tcp_session_timeout;     ///< NAT TCP session timeout in seconds
  uint32_t non_tcp_session_timeout; ///< NAT non-TCP session timeout in seconds
  sl_net_interface_t interface;     ///< Network interface to apply NAT settings
} sli_net_nat_config_t;

/**
 * @enum sli_net_async_state_t
 * @brief Internal async operation states for network interfaces
 * 
 * @details
 *   This enum tracks the current state of an asynchronous network bring-up operation
 *   initiated by @ref sl_net_up_async(). The state transitions are:
 *   IDLE -> START -> CONNECTING -> IP_CONFIG -> CONNECTED (success path)
 *   Any state can transition to IDLE on error.
 * 
 * @note Append new enum values; do not reorder existing ones for binary compatibility.
 */
typedef enum {
  SLI_NET_ASYNC_IDLE = 0,   ///< No async operation in progress
  SLI_NET_ASYNC_START,      ///< Async operation requested, not yet started
  SLI_NET_ASYNC_IP_CONFIG,  ///< IP configuration in progress (DHCP or static IP setup)
  SLI_NET_ASYNC_CONNECTED,  ///< Async bring-up completed successfully
  SLI_NET_ASYNC_CONNECTING, ///< WiFi connection in progress (after sl_wifi_connect returns IN_PROGRESS)
} sli_net_async_state_t;

/**
 * @enum sli_net_state_t
 * @brief Network connection state.
 *
 * Represents the current lifecycle stage of a network interface or connection.
 *
 * States:
 * - SLI_NET_STATE_DISCONNECTED: No active connection; interface is idle.
 * - SLI_NET_STATE_CONNECTING: Connection attempt in progress (e.g., association, authentication, DHCP).
 * - SLI_NET_STATE_CONNECTED: Connection established; network services available.
 */
typedef enum {
  SLI_NET_STATE_DISCONNECTED = 0,
  SLI_NET_STATE_CONNECTING,
  SLI_NET_STATE_CONNECTED,
} sli_net_state_t;

/**
 * @struct sli_net_async_if_state_t
 * @brief Per-interface asynchronous network state container.
 *
 * Holds the current asynchronous state machine value for a network interface
 * along with the associated profile identifier that the state refers to.
 *
 */

typedef struct {
  sli_net_async_state_t state; ///< Current asynchronous state for the interface.
  sl_net_profile_id_t profile; ///< Profile identifier associated with the ongoing / last async operation.
} sli_net_async_if_state_t;

/**
 * @struct sli_net_sync_client_if_state_t
 * @brief synchronous client network state container.
 *
 * Maintains the synchronous connection or operational state for a client-side
 * network interface together with the profile identifier used for connection
 * parameters.
 *
 */
typedef struct {
  sli_net_state_t state;       ///< Current synchronous client state (DISCONNECTED, CONNECTING, CONNECTED).
  sl_net_profile_id_t profile; ///< Profile identifier used for the synchronous client operation.
} sli_net_sync_client_if_state_t;
