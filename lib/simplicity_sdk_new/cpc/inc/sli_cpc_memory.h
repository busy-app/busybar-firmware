/***************************************************************************/ /**
 * @file
 * @brief CPC Internal Memory Functions
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

#ifndef SLI_CPC_MEMORY_H
#define SLI_CPC_MEMORY_H

#include "sl_status.h"

#include "sli_cpc_instance.h"

#if defined(SL_CATALOG_CPC_PRIMARY_PRESENT)
#include "sli_cpc_system_primary.h"
#endif

#if defined(SL_CATALOG_CPC_SECONDARY_PRESENT)
#include "sli_cpc_system_secondary.h"
#endif

#if defined(SL_CATALOG_MEMORY_MANAGER_PRESENT)
#include "sl_memory_manager.h"
#else
#include "sli_mem_pool.h"
#endif

/** @brief Struct for rx_buffer allocated in memory pool. */
typedef struct {
  sli_cpc_instance_t *instance; ///< Pointer to instance that allocated the RX buffer
  uint8_t            data[];    ///< The data itself
} sli_cpc_rx_buffer_t;

/***************************************************************************//**
 * Initialize CPC buffers' handling module.
 ******************************************************************************/
sl_status_t sli_cpc_memory_init(sli_cpc_instance_t *inst);

/***************************************************************************//**
 * @brief Get a handle to a CPC write buffer.
 *
 * Retrieves a handle for write operations. In environments with RTOS,
 * it handles semaphore acquisition based on the given timeout.
 *
 * @param[in] instance Pointer to instance to operate on.
 * @param[out] handle  Pointer to store the retrieved buffer handle.
 * @param[in] block    If true, the function blocks until the buffer is available or
 *                     timeout occurs; non-blocking otherwise.
 * @param[in] timeout  Timeout for buffer availability in milliseconds used only when
 *                     RTOS is used.
 *
 * @return             Status code indicating operation result
 ******************************************************************************/
sl_status_t sli_cpc_get_write_buffer_handle(sli_cpc_instance_t *inst,
                                            sl_cpc_buffer_handle_t **handle,
                                            bool block,
                                            uint32_t timeout);

/***************************************************************************//**
 * Get a CPC buffer handle.
 *
 * @param[in] instance Pointer to instance to operate on.
 * @param[out] handle  Address of the variable that will receive the handle
 *                     pointer.
 * @param[in] type     Type of buffer handle to allocate.
 *
 * @return Status code.
 ******************************************************************************/
sl_status_t sli_cpc_get_buffer_handle(sli_cpc_instance_t *inst,
                                      sl_cpc_buffer_handle_t **handle,
                                      sl_cpc_buffer_handle_type_t type);

/***************************************************************************//**
 * Get a CPC buffer for reception. This also allocates a buffer for the HDLC
 * header and a RX buffer if necessary.
 *
 * @param[in] instance Pointer to instance to operate on.
 * @param[out] handle  Address of the variable that will receive the buffer
 *                     pointer.
 * @param[in] alloc_rx_buffer  True to allocate handle's RX buffer.
 *
 * @return Status code.
 ******************************************************************************/
sl_status_t sli_cpc_get_buffer_handle_for_rx(sli_cpc_instance_t *inst,
                                             sl_cpc_buffer_handle_t **handle,
                                             bool alloc_rx_buffer);

/***************************************************************************//**
 * Get a CPC header buffer.
 *
 * @param[in] instance Pointer to instance to operate on.
 * @param[out] buffer  Address of the variable that will receive the buffer
 *                     pointer.
 *
 * @return Status code.
 ******************************************************************************/
sl_status_t sli_cpc_get_hdlc_header_buffer(sli_cpc_instance_t *inst, uint8_t **buffer);

/***************************************************************************//**
 * Get a CPC header and buffer for transmitting a reject packet.
 *
 * @param[in] instance Pointer to instance to operate on.
 * @param[out] handle  Address of the variable that will receive the buffer
 *                     pointer.
 *
 * @return Status code.
 ******************************************************************************/
sl_status_t sli_cpc_get_reject_buffer(sli_cpc_instance_t *inst, sl_cpc_buffer_handle_t **handle);

/***************************************************************************//**
 * Get a CPC RAW buffer for reception.
 *
 * @param[in] instance Pointer to instance to operate on.
 * @param[out] handle  Address of the variable that will receive the RAW buffer
 *                     pointer.
 *
 * @return Status code.
 ******************************************************************************/
sl_status_t sli_cpc_get_raw_rx_buffer(sli_cpc_instance_t *inst, uint8_t **raw_rx_buffer);

/***************************************************************************//**
 * Free a CPC RAW rx buffer.
 *
 * @param[in] instance Pointer to instance to operate on.
 * @param[out] handle  Address of the variable that will receive the RAW buffer.
 *
 * @return Status code.
 ******************************************************************************/
sl_status_t sli_cpc_free_raw_rx_buffer(sli_cpc_instance_t *inst, uint8_t *raw_rx_buffer);

/***************************************************************************//**
 * Free header, buffer and handle.
 *
 * @param[in] instance Pointer to instance to operate on.
 * @param[in] handle   Handle to free.
 *
 * @return Status code.
 ******************************************************************************/
sl_status_t sli_cpc_free_buffer_handle(sl_cpc_buffer_handle_t *handle);

/***************************************************************************//**
 * Allocate queue item and push data buffer in receive queue then free
 * header and buffer handle.
 *
 * @param[in] instance     Pointer to instance to operate on.
 * @param[in] handle       Handle to free.
 * @param[out] head        Head pointer to where the newly allocated queue
 *                         item will be pushed.
 * @param[in] data_length  Length of the data in the handle.
 *
 * @return Status code.
 ******************************************************************************/
sl_status_t sli_cpc_push_back_rx_data_in_receive_queue(sli_cpc_instance_t *inst,
                                                       sl_cpc_buffer_handle_t *handle,
                                                       sl_slist_node_t **head,
                                                       uint16_t data_length);

/***************************************************************************//**
 * Free internal rx buffer; Not pushed in RX Queue .
 *
 * @param[in] instance  Pointer to instance to operate on.
 * @param[in] data      Pointer to data buffer to free.
 *
 * @return Status code .
 ******************************************************************************/
sl_status_t sli_cpc_free_rx_buffer(sli_cpc_instance_t *inst, void *data);

/***************************************************************************//**
 * Free CPC HDLC header.
 *
 * @param[in] instance  Pointer to instance to operate on.
 * @param[in] buffer    Pointer to hdlc header to free.
 *
 * @return Status code.
 ******************************************************************************/
sl_status_t sli_cpc_free_hdlc_header(sli_cpc_instance_t *inst, void *header);

/***************************************************************************//**
 * Get a receive queue item.
 *
 * @param[in] instance  Pointer to instance to operate on.
 * @param[out] item     Address of the variable that will receive the item pointer.
 *
 * @return Status code.
 ******************************************************************************/
sl_status_t sli_cpc_get_receive_queue_item(sli_cpc_instance_t *inst, sl_cpc_receive_queue_item_t **item);

/***************************************************************************//**
 * Free receive queue item and data buffer.
 *
 * @param[in] instance  Pointer to instance to operate on.
 * @param[in] item      Pointer to item to free.
 ******************************************************************************/
void sli_cpc_free_receive_queue_item(sli_cpc_instance_t *inst, sl_cpc_receive_queue_item_t *item);

/***************************************************************************//**
 * Push receive queue item queued to the postponed free list
 *
 * @param[in] instance      Pointer to instance to operate on.
 * @param[in] item          Pointer to item to free.
 * @param[out] data         Pointer to variable that will receive the rx buffer pointer.
 * @param[out] data_length  Pointer to variable that will receive the data length.
 *
 * @return Status code.
 ******************************************************************************/
sl_status_t sli_cpc_push_receive_queue_item_to_postponed_list(sli_cpc_instance_t *inst,
                                                              sl_cpc_receive_queue_item_t *rx_queue_item,
                                                              void **data,
                                                              uint16_t *data_length);

/***************************************************************************//**
 * Get an endoint.
 *
 * @param[in] instance   Pointer to instance to operate on.
 * @param[out] endpoint  Address of the variable that will receive the item pointer.
 *
 * @return Status code.
 ******************************************************************************/
sl_status_t sli_cpc_get_endpoint(sli_cpc_instance_t *inst, sl_cpc_endpoint_t **endpoint);

/***************************************************************************//**
 * Free endpoint.
 *
 * @param[in] instance  Pointer to instance to operate on.
 * @param[in] endpoint  Pointer to endpoint to free.
 ******************************************************************************/
void sli_cpc_free_endpoint(sli_cpc_instance_t *inst, sl_cpc_endpoint_t *endpoint);

#if defined(SL_CATALOG_CPC_PRIMARY_PRESENT)
/***************************************************************************//**
 * Get a a system command handle.
 *
 * @param[in] instance  Pointer to instance to operate on.
 * @param[out] item     Address of the variable that will receive the item pointer.
 *
 * @return Status code.
 ******************************************************************************/
sl_status_t sli_cpc_get_system_command_handle(sli_cpc_instance_t *inst, sli_cpc_system_command_handle_t **item);

/***************************************************************************//**
 * Free CPC system command handle.
 *
 * @param[in] instance  Pointer to instance to operate on.
 * @param[in] item      Pointer to system command handle to free.
 *
 * @return Status code.
 ******************************************************************************/
sl_status_t sli_cpc_free_system_command_handle(sli_cpc_instance_t *inst, sli_cpc_system_command_handle_t *item);
#endif

#if defined(SL_CATALOG_CPC_SECONDARY_PRESENT)
/***************************************************************************//**
 * Get a a system command context.
 *
 * @param[in] instance  Pointer to instance to operate on.
 * @param[out] item     Address of the variable that will receive the item pointer.
 *
 * @return Status code.
 ******************************************************************************/
sl_status_t sli_cpc_get_system_command_context(sli_cpc_instance_t *inst, sli_cpc_system_cmd_context_t **item);

/***************************************************************************//**
 * Free CPC system command buffer.
 *
 * @param[in] instance  Pointer to instance to operate on.
 * @param[in] item      Pointer to system command buffer to free.
 *
 * @return Status code.
 ******************************************************************************/
sl_status_t sli_cpc_free_system_command_context(sli_cpc_instance_t *inst, sli_cpc_system_cmd_context_t *item);
#endif

/***************************************************************************//**
 * Get a buffer to store a security tag.
 *
 * @param[in] instance     Pointer to instance to operate on.
 * @param[out] tag_buffer  Address of the variable that will receive the argument
 *                         tag_buffer pointer.
 *
 * @return Status code.
 ******************************************************************************/
sl_status_t sli_cpc_get_security_tag_buffer(sli_cpc_instance_t *inst, void **tag_buffer);

/***************************************************************************//**
 * Free a security tag buffer.
 *
 * @param[in] instance    Pointer to instance to operate on.
 * @param[in] tag_buffer  Pointer to the buffer to free.
 *
 * @return Status code.
 ******************************************************************************/
sl_status_t sli_cpc_free_security_tag_buffer(sli_cpc_instance_t *inst, void *tag_buffer);

/***************************************************************************//**
 * Push back a list item containing an allocated buffer handle for the core
 ******************************************************************************/
void sli_cpc_push_back_core_buffer_handle(sl_slist_node_t **head, sl_cpc_buffer_handle_t *buf_handle);

/***************************************************************************//**
 * Pop a buffer handle from a list owned by the CPC core
 ******************************************************************************/
sl_cpc_buffer_handle_t* sli_cpc_pop_core_buffer_handle(sl_slist_node_t **head);

/***************************************************************************//**
 * Remove a list item containing an allocated buffer handle for the core
 ******************************************************************************/
void sli_cpc_remove_core_buffer_handle(sl_slist_node_t **head, sl_cpc_buffer_handle_t *buffer_handle);

/***************************************************************************//**
 * Push a list item containing an allocated buffer handle for the core
 ******************************************************************************/
void sli_cpc_push_core_buffer_handle(sl_slist_node_t **head, sl_cpc_buffer_handle_t *buf_handle);

/***************************************************************************//**
 * Pop a buffer handle from a list owned by the CPC driver
 ******************************************************************************/
sl_cpc_buffer_handle_t* sli_cpc_pop_driver_buffer_handle(sl_slist_node_t **head);

/***************************************************************************//**
 * Remove a buffer handle from a list owned by the CPC driver
 ******************************************************************************/
void sli_cpc_remove_driver_buffer_handle(sl_slist_node_t **head, sl_cpc_buffer_handle_t *buffer_handle);

/***************************************************************************//**
 * Push a buffer handle to a list.
 ******************************************************************************/
void sli_cpc_push_back_driver_buffer_handle(sl_slist_node_t **head, sl_cpc_buffer_handle_t *buf_handle);

/***************************************************************************//**
 * Push a list item containing an allocated buffer handle for the driver
 ******************************************************************************/
void sli_cpc_push_driver_buffer_handle(sl_slist_node_t **head, sl_cpc_buffer_handle_t *buf_handle);

#endif // SLI_CPC_MEMORY_H
