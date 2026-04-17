/***************************************************************************/ /**
 * @file sli_routing_utility_types.h
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

#ifndef SLI_ROUTING_UTILITY_TYPES_H
#define SLI_ROUTING_UTILITY_TYPES_H

#include "sli_queue_manager.h"
#include "cmsis_os2.h"
#include <stdint.h>

typedef void (*sli_routing_utility_packet_status_handler_t)(uint16_t packet_type, sl_status_t status, void *context);

// Structure representing a routing entry
typedef struct {
  sl_status_t (*destination_packet_handler)(void *packet,
                                            uint32_t packet_size,
                                            sli_routing_utility_packet_status_handler_t packet_status_handler,
                                            void *context);          // Function pointer for sending a packet
  sli_routing_utility_packet_status_handler_t packet_status_handler; // Function pointer for handling packet status
  uint16_t packet_type;
  sli_queue_t *queue_handle;    // Pointer to the queue handle that is being used
  osEventFlagsId_t event_group; // Event flags identifier for the queue
  uint32_t event_flag;          // Event associated with the queue (could represent a specific type of event)
} sli_routing_entry_t;

// Structure representing the routing configuration
typedef struct {
  sli_routing_entry_t *routing_table;
  uint16_t routing_table_size;
} sli_routing_table_t;

#endif