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
#pragma once

#include "sl_status.h"
#include "sl_net_types.h"

/** \addtogroup SL_PING_FUNCTIONS Functions
 * \ingroup SL_PING_APIS
 * @{ */

/***************************************************************************/ /**
 * @brief
 *   Send an ICMP ping request to a specific IP address.
 * @details
 *   This function sends an ICMP ping request to a specific IP address (IPv4 or IPv6) with a user defined ping packet size.
 *   It verifies the device’s initialization status and validates the IP address type before sending the request to the specific IP address.
 * @pre Pre-condition:
 *  - [sl_net_up](../wiseconnect-api-reference-guide-nwk-mgmt/net-interface-functions#sl-net-up) should be called before this API.
 * @param[in] ip_address
 *   The destination IP address should be of type [sl_ip_address_type_t](../wiseconnect-api-reference-guide-common/sl-ip-address-t) either IPv4 or IPv6.
 * @param[in] ping_packet_size
 *   The size of the ping packet must fall within the valid range of [0, 300].
 * @return
 *   [sl_status_t](https://docs.silabs.com/gecko-platform/latest/platform-common/status) - Status of the operation.
 *   - SL_STATUS_OK: Operation successful.
 *   - SL_STATUS_NOT_INITIALIZED: Device is not initialized.
 *   - SL_STATUS_INVALID_PARAMETER: Invalid IP address type.
 * 
 * @note 
 * 1. This API is asynchronous. The ping response is delivered through the registered [sl_net_event_handler_t](../wiseconnect-api-reference-guide-nwk-mgmt/sl-net-types#sl-net-event-handler-t) callback, 
 * using the event type [SL_NET_PING_RESPONSE_EVENT](../wiseconnect-api-reference-guide-nwk-mgmt/sl-net-constants#sl-net-event-t). 
 * To receive this event, the sl_net_event_handler_t must be registered via the [sl_net_init()](../wiseconnect-api-reference-guide-nwk-mgmt/net-interface-functions#sl-net-init) API. 
 ******************************************************************************/
sl_status_t sl_si91x_send_ping(sl_ip_address_t ip_address, uint16_t ping_packet_size);

/** @} */
