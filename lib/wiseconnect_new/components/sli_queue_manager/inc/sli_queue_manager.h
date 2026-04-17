/***************************************************************************/ /**
 * @file sli_queue_manager.h
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
#ifndef SLI_QUEUE_MANAGER_H
#define SLI_QUEUE_MANAGER_H

#include "sli_queue_manager_types.h"

/**
 * @brief Initializes a queue manager with the specified queue node pool.
 *
 * This function sets up a queue manager instance, associating it with a 
 * specific pool type for managing queue nodes. It prepares the queue 
 * manager for subsequent operations such as enqueueing and dequeueing.
 *
 * @param[in,out] handle Pointer to the queue manager instance to initialize.
 *                       This must be a valid pointer to an uninitialized 
 *                       `sli_queue_t` structure.
 * @param[in] queue_node_pool The type of pool to use for managing queue nodes.
 *                            This specifies the memory pool from which 
 *                            queue nodes will be allocated.
 *
 * @return SL_STATUS_OK if the initialization was successful.
 *         Appropriate error code otherwise.
 */
sl_status_t sli_queue_manager_init(sli_queue_t *handle, sli_buffer_manager_pool_types_t queue_node_pool);

/**
 * @brief Enqueue a packet to the queue.
 *
 * @param handle Queue Handle.
 * @param node Packet which is to be added in to the queue. 
 * @return sl_status_t
 */
sl_status_t sli_queue_manager_enqueue(sli_queue_t *handle, void *data);

/**
 * @brief Enqueues a node into the specified queue.
 *
 * This function adds the given node to the end of the queue managed by the provided handle.
 *
 * @param[in,out] handle Pointer to the queue handle where the node will be enqueued.
 * @param[in] node Pointer to the queue node to be enqueued.
 * @return sl_status_t Status code indicating the result of the operation.
 *         - SL_STATUS_OK on success.
 *         - Appropriate error code on failure (e.g., invalid parameters, queue full).
 */
sl_status_t sli_queue_manager_enqueue_node(sli_queue_t *handle, sli_queue_node_t *node);

/**
 * @brief Add a packet to the head of the queue identified by queue_id in the given instance.
 * 
 * @param handle Queue Handle.
 * @param node Packet which is to be added in to the queue.
 * @return sl_status_t
 */
sl_status_t sli_queue_manager_add_to_queue_head(sli_queue_t *handle, void *data);

/**
 * @brief Dequeue a packet from the queue identified by queue_id in the given instance.
 * 
 * @param handle Queue Handle.
 * @param node Pointer to hold packet which is to be removed from the queue.
 * @return sl_status_t
 */
sl_status_t sli_queue_manager_dequeue(sli_queue_t *handle, void **data);

/**
 * @brief Dequeues a node from the specified queue.
 *
 * Removes the front node from the queue referenced by the given handle and returns
 * a pointer to the dequeued node through the data parameter.
 *
 * @param[in]  handle Pointer to the queue handle from which to dequeue a node.
 * @param[out] node  Pointer to a pointer that will receive the dequeued node.
 *
 * @return sl_status_t
 *         - SL_STATUS_OK if the operation was successful.
 *         - SL_STATUS_EMPTY if the queue is empty.
 *         - SL_STATUS_NULL_POINTER if handle or data is NULL.
 *
 * @note The caller is responsible for managing the memory of the dequeued node.
 */
sl_status_t sli_queue_manager_dequeue_node(sli_queue_t *handle, sli_queue_node_t **node);

/**
 * @brief Remove a packet from the queue identified by id_handler.
 * 
 * @param handle Queue Handle.
 * @param id_handler Pointer to function which can identify the target node in the queue.
 * @param node_match_data User specific pointer which can be passed to id_handler.
 * @param node Pointer to hold packet which is to be removed from the queue. 
 * @return sl_status_t
 */
sl_status_t sli_queue_manager_remove_node_from_queue(sli_queue_t *handle,
                                                     sli_queue_manager_node_match_handler_t id_handler,
                                                     void *node_match_data,
                                                     void **data);

/**
 * @brief Remove all packets from the queue identified by id_handler.
 * 
 * @param handle Queue Handle.
 * @param id_handler Pointer to function which can identify the target node in the queue.
 * @param node_match_data User specific pointer which can be passed to id_handler and flush_handler. 
 * @param flush_handler Pointer to function which can free the target node in the queue.   
 * @return sl_status_t
 */
sl_status_t sli_queue_manager_flush_nodes_from_queue(sli_queue_t *handle,
                                                     sli_queue_manager_node_match_handler_t id_handler,
                                                     void *node_match_data,
                                                     sli_queue_manager_flush_handler_t flush_handler);

/**
 * @brief Flush all the nodes present in the queue.
 * 
 * @param handle Queue Handle.
 * @param flush_handler Pointer to function which can free the all nodes in the queue.   
 * @return sl_status_t
 */
sl_status_t sli_queue_manager_flush_queue(sli_queue_t *handle, sli_queue_manager_flush_handler_t flush_handler);

/**
 * @brief De-Initialize the queue manager.
 * 
 * @param handle Queue Handle.
 * @param flush_handler Pointer to function which can free the all nodes in the queue. 
 * @return sl_status_t
 */
sl_status_t sli_queue_manager_deinit(sli_queue_t *handle, sli_queue_manager_flush_handler_t flush_handler);

#endif // SLI_QUEUE_MANAGER_H