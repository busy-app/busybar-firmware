/***************************************************************************/ /**
 * @file sli_routing_utility.c
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

#ifndef SLI_ROUTING_UTILITY_C
#define SLI_ROUTING_UTILITY_C

#include "sli_routing_utility.h"
#include "cmsis_os2.h"
#include "sli_routing_utility_types.h"
#include "sl_status.h"
#include "sl_constants.h"

sl_status_t sli_routing_utility_route_queue_node(sli_routing_table_t *routing_table,
                                                 uint16_t packet_type,
                                                 sli_queue_node_t *queue_node,
                                                 uint16_t queue_node_size,
                                                 void *context)
{
  sl_status_t status = SL_STATUS_FAIL;

  // Check if packet_type is within the bounds of the routing table
  if (packet_type >= routing_table->routing_table_size) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  // Get the routing entry for the given packet type
  sli_routing_entry_t *entry = &routing_table->routing_table[packet_type];

  // Get the packet data from the queue node
  void *packet_data = queue_node->data;

  // Call the destination packet handler if it is not NULL
  if (entry->destination_packet_handler != NULL) {
    status = entry->destination_packet_handler(packet_data, queue_node_size, entry->packet_status_handler, context);
    VERIFY_STATUS_AND_RETURN(status);
  }

  // Enqueue the queue node if the queue handle is not NULL
  if (entry->queue_handle != NULL) {
    status = sli_queue_manager_enqueue_node(entry->queue_handle, queue_node);
    VERIFY_STATUS_AND_RETURN(status);

    // Set the event flags
    osEventFlagsSet(entry->event_group, entry->event_flag);
  }

  return SL_STATUS_OK;
}

sl_status_t sli_routing_utility_route_packet(sli_routing_table_t *routing_table,
                                             uint16_t packet_type,
                                             void *packet,
                                             uint16_t packet_size,
                                             void *context)
{
  sl_status_t status = SL_STATUS_FAIL;

  // Check if packet_type is within the bounds of the routing table
  if (packet_type >= routing_table->routing_table_size) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  sli_routing_entry_t *entry = &routing_table->routing_table[packet_type];

  // Call the destination packet handler if it is not NULL
  if (entry->destination_packet_handler != NULL) {
    status = entry->destination_packet_handler(packet, packet_size, entry->packet_status_handler, context);
    VERIFY_STATUS_AND_RETURN(status);
  }

  // Enqueue the packet if the queue handle is not NULL
  if (entry->queue_handle != NULL) {
    status = sli_queue_manager_enqueue(entry->queue_handle, packet);
    VERIFY_STATUS_AND_RETURN(status);

    // Set the event flags
    osEventFlagsSet(entry->event_group, entry->event_flag);
  }

  return SL_STATUS_OK;
}

#endif
