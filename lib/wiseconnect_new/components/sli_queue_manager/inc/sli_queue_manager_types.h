/***************************************************************************/ /**
 * @file sli_queue_manager_types.h
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
#ifndef SLI_QUEUE_MANAGER_TYPES_H
#define SLI_QUEUE_MANAGER_TYPES_H

#include "sl_status.h"
#include "sl_slist.h"
#include "sli_buffer_manager.h"

/**
 * @brief Represents a node in a queue.
 *
 * This structure is used to define a single node in a queue, where each node
 * contains a pointer to the next node in the queue and a pointer to the data
 * it holds.
 *
 * @struct sli_queue_node_s
 * @var sli_queue_node_s::next
 * Pointer to the next node in the queue. If this is the last node, the pointer
 * will be NULL.
 * @var sli_queue_node_s::data
 * Pointer to the data stored in this node. The data can be of any type.
 */
typedef struct sli_queue_node_s {
  struct sli_queue_node_s *next;
  void *data;
} sli_queue_node_t;

/**
 * @struct sli_queue_t
 * @brief Structure representing a Queue handle.
 */
typedef struct {
  sli_queue_node_t *head;
  sli_queue_node_t *tail;
  sli_buffer_manager_pool_types_t queue_node_pool;
  void *lock;
} sli_queue_t;

/**
 * @brief Macro to check the status of a queue.
 * This macro evaluates whether the specified queue is empty by checking if the head of the queue is NULL.
 *
 * @param queue The queue to check the status of.
 *
 * @return True if the queue is empty (i.e., the head is NULL), otherwise false.
 */
#define SLI_QUEUE_MANAGER_IS_QUEUE_EMPTY(queue) (NULL == (queue)->head)

/**
 * @typedef sli_queue_manager_node_match_handler_t
 * @brief Callback function to identify the node to be removed from the queue.
 *
 * @details
 * The callback function is invoked from with in the sli_remove_node_from_queue() API
 * while iterating through the nodes in the queue and this function should return TRUE
 * if the current node on which the callback is invoked should be removed.
 *
 * @param queue
 *   Queue Handle.
 *
 * @param node
 *   Current Packet in the queue.
 *
 * @param context
 *   User specific context pointer which is being passed in sli_remove_node_from_queue() API.
 *
 * @return
 *   Should return TRUE if the target node is found else it should return FALSE
 */
typedef bool (*sli_queue_manager_node_match_handler_t)(sli_queue_t *handle, void *data, void *node_match_data);

/**
 * @typedef sli_queue_manager_flush_handler_t
 * @brief Callback function to free the nodes in the queue when sli_flush_queue() API or
 * @brief sli_queue_instance_deinit() API is called.
 *
 * @details
 * The callback function is invoked from with in the sli_flush_queue() API
 * or sli_queue_instance_deinit() API to free up any packets in the queue.
 *
 * @param queue
 *   Queue Handle.
 *
 * @param node
 *   Packet which is present in the queue that has to be freed.
 *
 * @return
 *   N/A
 */
typedef void (*sli_queue_manager_flush_handler_t)(sli_queue_t *handle, void *data, void *context);

#endif // SLI_QUEUE_MANAGER_TYPES_H