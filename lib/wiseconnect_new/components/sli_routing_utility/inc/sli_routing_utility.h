/***************************************************************************/ /**
 * @file sli_routing_utility.h
 * @brief 
 *******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifndef SLI_ROUTING_UTILITY_H
#define SLI_ROUTING_UTILITY_H

#include "sli_routing_utility_types.h"
#include <stdint.h>
/**
 * @brief Routes a packet encapsulated in a queue node through the routing table.
 *
 * @param routing_table Pointer to the routing table to be used for routing.
 * @param packet_type Type of the packet to be routed.
 * @param queue_node Pointer to the queue node containing the packet to be routed.
 * @param queue_node_size Size of the queue node.
 * @param context Pointer to the context to be passed to the packet handler.
 *
 * @return Status of the routing operation.
 */
sl_status_t sli_routing_utility_route_queue_node(sli_routing_table_t *routing_table,
                                                 uint16_t packet_type,
                                                 sli_queue_node_t *queue_node,
                                                 uint16_t queue_node_size,
                                                 void *context);
/**
 * @brief Routes a packet through the routing table.
 *
 * @param routing_table Pointer to the routing table to be used for routing.
 * @param packet_type Type of the packet to be routed.
 * @param packet Pointer to the packet to be routed.
 * @param packet_size Size of the packet .
 * @param context Pointer to the context to be passed to the packet handler.
 *
 * @return Status of the routing operation.
 */
sl_status_t sli_routing_utility_route_packet(sli_routing_table_t *routing_table,
                                             uint16_t packet_type,
                                             void *packet,
                                             uint16_t packet_size,
                                             void *context);

#endif