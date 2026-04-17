/***************************************************************************/ /**
 * @file
 * @brief CPC API implementation.
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

#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>

#include "sl_status.h"

#include "sl_cpc_config.h"
#include "sli_cpc_system_common.h"
#include "sli_cpc.h"
#include "sli_cpc_assert.h"
#include "sli_cpc_hdlc.h"
#include "sli_cpc_debug.h"
#include "sli_cpc_drv.h"
#include "sli_cpc_memory.h"
#include "sl_common.h"
#include "sl_slist.h"

#if defined(SL_COMPONENT_CATALOG_PRESENT)
#include "sl_component_catalog.h"
#endif

#if defined(SL_CATALOG_MEMORY_MANAGER_PRESENT)
#include "sl_memory_manager.h"
#else
#include "sli_mem_pool.h"
#endif

#if defined(SL_CATALOG_CPC_PRIMARY_PRESENT)
// needed for definition of sli_cpc_system_command_handle_t;
#include "sli_cpc_system_primary.h"
#endif

#if defined(SL_CATALOG_CPC_SECONDARY_PRESENT)
// needed for SLI_CPC_SYSTEM_COMMAND_BUFFER_SIZE
#include "sli_cpc_system_secondary.h"
#endif

#include "sl_cpc_instance_handles.h"

#if defined(SL_CATALOG_CPC_DRIVER_SDIO_PRESENT)
#include "sl_cpc_drv_secondary_sdio_config.h"
#endif

/*******************************************************************************
 ***************************  LOCAL VARIABLES   ********************************
 ******************************************************************************/

#if !defined(SL_CATALOG_MEMORY_MANAGER_PRESENT)
SLI_MEM_POOL_DECLARE_BUFFER_WITH_TYPE(mempool_buffer_handle,
                                      sl_cpc_buffer_handle_t,
                                      SLI_CPC_BUFFER_HANDLE_MAX_COUNT);
SLI_MEM_POOL_DECLARE_BUFFER(mempool_hdlc_header,
                            SLI_CPC_HDLC_HEADER_RAW_SIZE,
                            SLI_CPC_HDLC_HEADER_MAX_COUNT);
SLI_MEM_POOL_DECLARE_BUFFER(mempool_hdlc_reject,
                            SLI_CPC_HDLC_REJECT_PAYLOAD_SIZE,
                            SLI_CPC_HDLC_REJECT_MAX_COUNT);
SLI_MEM_POOL_DECLARE_BUFFER(mempool_rx_buffer,
                            SLI_CPC_RX_BUFFER_MAX_LENGTH,
                            SL_CPC_RX_BUFFER_MAX_COUNT);
SLI_MEM_POOL_DECLARE_BUFFER_WITH_TYPE(mempool_rx_queue_item,
                                      sl_cpc_receive_queue_item_t,
                                      SLI_CPC_RX_QUEUE_ITEM_MAX_COUNT);
#if defined(SL_CATALOG_CPC_PRIMARY_PRESENT)
SLI_MEM_POOL_DECLARE_BUFFER_WITH_TYPE(mempool_command_handle,
                                      sli_cpc_system_command_handle_t,
                                      SLI_CPC_SYSTEM_COMMAND_HANDLE_COUNT);
#endif
#if defined(SL_CATALOG_CPC_SECONDARY_PRESENT)
SLI_MEM_POOL_DECLARE_BUFFER(mempool_system_command,
                            SLI_CPC_SYSTEM_COMMAND_BUFFER_SIZE,
                            SLI_CPC_SYSTEM_COMMAND_BUFFER_COUNT);
#endif

#if (SL_CPC_ENDPOINT_SECURITY_ENABLED >= 1)
SLI_MEM_POOL_DECLARE_BUFFER(mempool_tx_security_tag,
                            SLI_SECURITY_TAG_LENGTH_BYTES,
                            SL_CPC_TX_QUEUE_ITEM_MAX_COUNT);
#endif
#endif // !defined(SL_CATALOG_MEMORY_MANAGER_PRESENT)

/*******************************************************************************
 **************************   LOCAL FUNCTIONS   ********************************
 ******************************************************************************/

static void* alloc_object(sl_cpc_mem_pool_handle_t *pool);

static void free_object(sl_cpc_mem_pool_handle_t *pool,
                        void *block);

static void notify_rx_buffer_free(sli_cpc_instance_t *inst);

static sl_status_t free_receive_queue_item(sli_cpc_instance_t *inst,
                                           sl_cpc_receive_queue_item_t *item);

static sl_status_t free_receive_queue_item_from_postponed_list(sli_cpc_instance_t *inst);

/*******************************************************************************
 **************************   GLOBAL FUNCTIONS   *******************************
 ******************************************************************************/

/***************************************************************************//**
 * Initialize CPC buffers' handling module.
 ******************************************************************************/
sl_status_t sli_cpc_memory_init(sli_cpc_instance_t *inst)
{
#if defined(SL_CATALOG_KERNEL_PRESENT)
  osSemaphoreAttr_t tx_item_availability_signal_semaphore_attr = {
#if defined(SL_CATALOG_CMSIS_OS_COMMON_PRESENT)
    .cb_mem = inst->tx_item_availability_semaphore_cb,
    .cb_size = sizeof(inst->tx_item_availability_semaphore_cb),
#else
    .cb_mem = NULL,
    .cb_size = 0U,
#endif // SL_CATALOG_CMSIS_OS_COMMON_PRESENT
  };

  inst->tx_item_availability_signal = osSemaphoreNew(inst->tx_queue_item_max_count,
                                                     inst->tx_queue_item_max_count,
                                                     &tx_item_availability_signal_semaphore_attr);
  if (inst->tx_item_availability_signal == 0) {
    SLI_CPC_ASSERT(0);
    return SL_STATUS_ALLOCATION_FAILED;
  }
#endif

#if (SL_CPC_DEBUG_MEMORY_ALLOCATOR_COUNTERS == 1)
  inst->mempool_buffer_handle.used_block_cnt = 0;
  inst->mempool_hdlc_header.used_block_cnt = 0;
  inst->mempool_hdlc_reject.used_block_cnt = 0;
  inst->mempool_rx_buffer.used_block_cnt = 0;
  inst->mempool_rx_queue_item.used_block_cnt = 0;

#if defined(SL_CATALOG_CPC_PRIMARY_PRESENT)
  inst->mempool_command_handle.used_block_cnt = 0;
#endif

#if defined(SL_CATALOG_CPC_SECONDARY_PRESENT)
  inst->mempool_system_command.used_block_cnt = 0;
#endif

#if (SL_CPC_ENDPOINT_SECURITY_ENABLED >= 1)
  inst->mempool_tx_security_tag.used_block_cnt = 0;
#endif

  inst->endpoints_used_block_cnt = 0;
#endif // SL_CPC_DEBUG_MEMORY_ALLOCATOR_COUNTERS >= 1

#if defined(SL_CATALOG_MEMORY_MANAGER_PRESENT)
  sl_status_t status;
  status = sl_memory_pool_handle_alloc(&inst->mempool_buffer_handle.handle);
  SLI_CPC_ASSERT(status == SL_STATUS_OK);
  status = sl_memory_create_pool(sizeof(sl_cpc_buffer_handle_t),
                                 SLI_CPC_BUFFER_HANDLE_MAX_COUNT_INST(inst),
                                 inst->mempool_buffer_handle.handle);
  SLI_CPC_ASSERT(status == SL_STATUS_OK);

  status = sl_memory_pool_handle_alloc(&inst->mempool_hdlc_header.handle);
  SLI_CPC_ASSERT(status == SL_STATUS_OK);
  status = sl_memory_create_pool(SLI_CPC_HDLC_HEADER_RAW_SIZE,
                                 SLI_CPC_HDLC_HEADER_MAX_COUNT_INST(inst),
                                 inst->mempool_hdlc_header.handle);
  SLI_CPC_ASSERT(status == SL_STATUS_OK);

  status = sl_memory_pool_handle_alloc(&inst->mempool_hdlc_reject.handle);
  SLI_CPC_ASSERT(status == SL_STATUS_OK);
  status = sl_memory_create_pool(SLI_CPC_HDLC_REJECT_PAYLOAD_SIZE,
                                 SLI_CPC_HDLC_REJECT_MAX_COUNT_INST(inst),
                                 inst->mempool_hdlc_reject.handle);
  SLI_CPC_ASSERT(status == SL_STATUS_OK);

  status = sl_memory_pool_handle_alloc(&inst->mempool_rx_buffer.handle);
  SLI_CPC_ASSERT(status == SL_STATUS_OK);
  status = sl_memory_create_pool(SLI_CPC_RX_BUFFER_MAX_LENGTH_INST(inst),
                                 inst->rx_buffer_max_count,
                                 inst->mempool_rx_buffer.handle);
  SLI_CPC_ASSERT(status == SL_STATUS_OK);

  status = sl_memory_pool_handle_alloc(&inst->mempool_rx_queue_item.handle);
  SLI_CPC_ASSERT(status == SL_STATUS_OK);
  status = sl_memory_create_pool(sizeof(sl_cpc_receive_queue_item_t),
                                 SLI_CPC_RX_QUEUE_ITEM_MAX_COUNT_INST(inst),
                                 inst->mempool_rx_queue_item.handle);
  SLI_CPC_ASSERT(status == SL_STATUS_OK);

#if defined(SL_CATALOG_CPC_PRIMARY_PRESENT)
  status = sl_memory_pool_handle_alloc(&inst->mempool_command_handle.handle);
  SLI_CPC_ASSERT(status == SL_STATUS_OK);
  status = sl_memory_create_pool(sizeof(sli_cpc_system_command_handle_t),
                                 SLI_CPC_SYSTEM_COMMAND_HANDLE_COUNT,
                                 inst->mempool_command_handle.handle);
  SLI_CPC_ASSERT(status == SL_STATUS_OK);
#endif

#if defined(SL_CATALOG_CPC_SECONDARY_PRESENT)
  status = sl_memory_pool_handle_alloc(&inst->mempool_system_command.handle);
  SLI_CPC_ASSERT(status == SL_STATUS_OK);
  status = sl_memory_create_pool(SLI_CPC_SYSTEM_COMMAND_BUFFER_SIZE,
                                 SLI_CPC_SYSTEM_COMMAND_BUFFER_COUNT,
                                 inst->mempool_system_command.handle);
  SLI_CPC_ASSERT(status == SL_STATUS_OK);
#endif

#if (SL_CPC_ENDPOINT_SECURITY_ENABLED >= 1)
  status = sl_memory_pool_handle_alloc(&inst->mempool_tx_security_tag.handle);
  SLI_CPC_ASSERT(status == SL_STATUS_OK);
  status = sl_memory_create_pool(SLI_SECURITY_TAG_LENGTH_BYTES,
                                 inst->tx_queue_item_max_count,
                                 inst->mempool_tx_security_tag.handle);
  SLI_CPC_ASSERT(status == SL_STATUS_OK);
#endif
#else
  SLI_CPC_ASSERT(inst == &g_instance);

  // global defines such as SLI_CPC_BUFFER_HANDLE_MAX_COUNT were used to define the size
  // of the buffer to allocate to act as storage for the mempool, but we prefer to use
  // the instance macro to fetch the configured size, to make the code more similar with
  // and without the memory manager. But we don't want to introduce any mismatch between
  // size stored in the instance and the actual if a macro doesn't behave as expected, so
  // assert them to make sure they are alright.
  SLI_CPC_ASSERT(SLI_CPC_BUFFER_HANDLE_MAX_COUNT_INST(inst) == SLI_CPC_BUFFER_HANDLE_MAX_COUNT);
  SLI_CPC_ASSERT(SLI_CPC_HDLC_HEADER_MAX_COUNT_INST(inst) == SLI_CPC_HDLC_HEADER_MAX_COUNT);
  SLI_CPC_ASSERT(SLI_CPC_HDLC_REJECT_MAX_COUNT_INST(inst) == SLI_CPC_HDLC_REJECT_MAX_COUNT);
  SLI_CPC_ASSERT(SLI_CPC_RX_BUFFER_MAX_LENGTH_INST(inst) == SLI_CPC_RX_BUFFER_MAX_LENGTH);
  SLI_CPC_ASSERT(inst->rx_buffer_max_count == SL_CPC_RX_BUFFER_MAX_COUNT);
  SLI_CPC_ASSERT(SLI_CPC_RX_QUEUE_ITEM_MAX_COUNT_INST(inst) == SLI_CPC_RX_QUEUE_ITEM_MAX_COUNT);
  SLI_CPC_ASSERT(inst->tx_queue_item_max_count == SL_CPC_TX_QUEUE_ITEM_MAX_COUNT);

  sli_mem_pool_create(&inst->mempool_buffer_handle.handle,
                      (uint16_t)sizeof(sl_cpc_buffer_handle_t),
                      SLI_CPC_BUFFER_HANDLE_MAX_COUNT_INST(inst),
                      mempool_buffer_handle_buffer,
                      sizeof(mempool_buffer_handle_buffer));

  sli_mem_pool_create(&inst->mempool_hdlc_header.handle,
                      SLI_CPC_HDLC_HEADER_RAW_SIZE,
                      SLI_CPC_HDLC_HEADER_MAX_COUNT_INST(inst),
                      mempool_hdlc_header_buffer,
                      sizeof(mempool_hdlc_header_buffer));

  sli_mem_pool_create(&inst->mempool_hdlc_reject.handle,
                      SLI_CPC_HDLC_REJECT_PAYLOAD_SIZE,
                      SLI_CPC_HDLC_REJECT_MAX_COUNT_INST(inst),
                      mempool_hdlc_reject_buffer,
                      sizeof(mempool_hdlc_reject_buffer));

  sli_mem_pool_create(&inst->mempool_rx_buffer.handle,
                      SLI_CPC_RX_BUFFER_MAX_LENGTH_INST(inst),
                      inst->rx_buffer_max_count,
                      mempool_rx_buffer_buffer,
                      sizeof(mempool_rx_buffer_buffer));

  sli_mem_pool_create(&inst->mempool_rx_queue_item.handle,
                      (uint16_t)sizeof(sl_cpc_receive_queue_item_t),
                      SLI_CPC_RX_QUEUE_ITEM_MAX_COUNT_INST(inst),
                      mempool_rx_queue_item_buffer,
                      sizeof(mempool_rx_queue_item_buffer));

#if defined(SL_CATALOG_CPC_PRIMARY_PRESENT)
  sli_mem_pool_create(&inst->mempool_command_handle.handle,
                      (uint16_t)sizeof(sli_cpc_system_command_handle_t),
                      SLI_CPC_SYSTEM_COMMAND_HANDLE_COUNT,
                      mempool_command_handle_buffer,
                      sizeof(mempool_command_handle_buffer));
#endif

#if defined(SL_CATALOG_CPC_SECONDARY_PRESENT)
  sli_mem_pool_create(&inst->mempool_system_command.handle,
                      SLI_CPC_SYSTEM_COMMAND_BUFFER_SIZE,
                      SLI_CPC_SYSTEM_COMMAND_BUFFER_COUNT,
                      mempool_system_command_buffer,
                      sizeof(mempool_system_command_buffer));
#endif

#if (SL_CPC_ENDPOINT_SECURITY_ENABLED >= 1)
  sli_mem_pool_create(&inst->mempool_tx_security_tag.handle,
                      SLI_SECURITY_TAG_LENGTH_BYTES,
                      inst->tx_queue_item_max_count,
                      mempool_tx_security_tag_buffer,
                      sizeof(mempool_tx_security_tag_buffer));
#endif
#endif // SL_CATALOG_MEMORY_MANAGER_PRESENT

  sl_slist_init(&inst->postponed_free_rx_queue_item);
#if !defined(SL_CATALOG_KERNEL_PRESENT)
  inst->tx_item_used_count = 0;
#endif
  inst->sframe_tx_item_used_count = 0;

  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Get a CPC handle buffer from write operations
 ******************************************************************************/
sl_status_t sli_cpc_get_write_buffer_handle(sli_cpc_instance_t *inst,
                                            sl_cpc_buffer_handle_t **handle,
                                            bool block,
                                            uint32_t timeout)
{
#if defined(SL_CATALOG_KERNEL_PRESENT)
  sl_status_t status;
  osStatus_t sem_status = osSemaphoreAcquire(inst->tx_item_availability_signal, block ? timeout : 0);
  if (sem_status == osErrorResource ||  sem_status == osErrorTimeout) {
    return SL_STATUS_NO_MORE_RESOURCE;
  }
  SLI_CPC_ASSERT(sem_status == osOK);

  status = sli_cpc_get_buffer_handle(inst, handle, SL_CPC_TX_DATA_BUFFER_HANDLE);
  SLI_CPC_ASSERT(status != SL_STATUS_NO_MORE_RESOURCE); // On a kernel, this should not happen

  return status;
#else
  (void)block;
  (void)timeout;

  return sli_cpc_get_buffer_handle(inst, handle, SL_CPC_TX_DATA_BUFFER_HANDLE);
#endif
}

/***************************************************************************//**
 * Get a CPC handle buffer
 ******************************************************************************/
sl_status_t sli_cpc_get_buffer_handle(sli_cpc_instance_t *inst,
                                      sl_cpc_buffer_handle_t **handle,
                                      sl_cpc_buffer_handle_type_t type)
{
  MCU_DECLARE_IRQ_STATE;
  sl_cpc_buffer_handle_t *buf_handle;

  if (handle == NULL) {
    return SL_STATUS_NULL_POINTER;
  }

  buf_handle = alloc_object(&inst->mempool_buffer_handle);
  if (buf_handle == NULL) {
    return SL_STATUS_NO_MORE_RESOURCE;
  }

  // Clear the buffer handle
  memset(buf_handle, 0, sizeof(sl_cpc_buffer_handle_t));

  // Update counters
  MCU_ENTER_ATOMIC();
  switch (type) {
    case SL_CPC_RX_INTERNAL_BUFFER_HANDLE:
      break;
    case SL_CPC_TX_REJECT_BUFFER_HANDLE:
    case SL_CPC_TX_SFRAME_BUFFER_HANDLE:
      if (inst->sframe_tx_item_used_count >= SLI_CPC_TX_QUEUE_ITEM_SFRAME_MAX_COUNT_INST(inst)) {
        free_object(&inst->mempool_buffer_handle, buf_handle);
        MCU_EXIT_ATOMIC();
        return SL_STATUS_NO_MORE_RESOURCE;
      }
      ++inst->sframe_tx_item_used_count;
      break;
    case SL_CPC_TX_DATA_BUFFER_HANDLE:
#if !defined(SL_CATALOG_KERNEL_PRESENT)
      if (inst->tx_item_used_count >= inst->tx_queue_item_max_count) {
        free_object(&inst->mempool_buffer_handle, buf_handle);
        MCU_EXIT_ATOMIC();
        return SL_STATUS_NO_MORE_RESOURCE;
      }
      ++inst->tx_item_used_count;
#endif
      break;
    case SL_CPC_UNKNOWN_BUFFER_HANDLE:
    default:
      return SL_STATUS_INVALID_PARAMETER;
      break;
  }
  MCU_EXIT_ATOMIC();

  // Set buffer handle type
  buf_handle->type = type;
  buf_handle->instance = inst;

  *handle = buf_handle;

  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Get a CPC header buffer
 ******************************************************************************/
sl_status_t sli_cpc_get_hdlc_header_buffer(sli_cpc_instance_t *inst, uint8_t **header)
{
  if (header == NULL) {
    return SL_STATUS_NULL_POINTER;
  }

  *header = (uint8_t *) alloc_object(&inst->mempool_hdlc_header);
  if (*header == NULL) {
    return SL_STATUS_NO_MORE_RESOURCE;
  }

  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Get a CPC header buffer for transmitting a reject
 ******************************************************************************/
sl_status_t sli_cpc_get_reject_buffer(sli_cpc_instance_t *inst, sl_cpc_buffer_handle_t **handle)
{
  sl_status_t status;
  sl_cpc_buffer_handle_t *buf_handle;

  status = sli_cpc_get_buffer_handle(inst, &buf_handle, SL_CPC_TX_REJECT_BUFFER_HANDLE);
  if (status != SL_STATUS_OK) {
    return status;
  }

  buf_handle->data = alloc_object(&inst->mempool_hdlc_reject);

  if (buf_handle->data == NULL) {
    sli_cpc_free_buffer_handle(buf_handle);
    return SL_STATUS_NO_MORE_RESOURCE;
  }

  buf_handle->data_length = 1;
  *handle = buf_handle;

  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Allocate a CPC RX buffer. The instance pointer will be populated and the
 * data pointer will be returned to the caller.
 ******************************************************************************/
static uint8_t* alloc_cpc_rx_buffer(sli_cpc_instance_t *inst)
{
  sli_cpc_rx_buffer_t *rx_buffer = alloc_object(&inst->mempool_rx_buffer);

  if (rx_buffer == NULL) {
    return NULL;
  }

  rx_buffer->instance = inst;

  return rx_buffer->data;
}

/***************************************************************************//**
 * Free a CPC RX buffer from its data pointer. Using a container_of operation,
 * the RX buffer can be reconstructed and we get back the pointer to the instance
 * that allocated the RX buffer.
 ******************************************************************************/
static sli_cpc_instance_t* free_cpc_rx_buffer(uint8_t *buffer)
{
  sli_cpc_rx_buffer_t *rx_buffer = container_of(buffer, sli_cpc_rx_buffer_t, data);
  sli_cpc_instance_t *inst;

  inst = rx_buffer->instance;

  free_object(&rx_buffer->instance->mempool_rx_buffer, rx_buffer);

  return inst;
}

/***************************************************************************//**
 * Get a CPC buffer for reception.
 ******************************************************************************/
sl_status_t sli_cpc_get_buffer_handle_for_rx(sli_cpc_instance_t *inst,
                                             sl_cpc_buffer_handle_t **handle,
                                             bool alloc_rx_buffer)
{
  sl_cpc_buffer_handle_t *buf_handle;
  sl_status_t status;
  uint8_t *data = NULL;
  uint8_t *hdlc_header = NULL;

  if (handle == NULL) {
    return SL_STATUS_NULL_POINTER;
  }

  if (alloc_rx_buffer) {
    data = alloc_cpc_rx_buffer(inst);
    if (data == NULL) {
      return SL_STATUS_NO_MORE_RESOURCE;
    }
  }

  status = sli_cpc_get_hdlc_header_buffer(inst, &hdlc_header);
  if (hdlc_header == NULL) {
    goto rx_buffer_allocated;
  }

  status = sli_cpc_get_buffer_handle(inst, &buf_handle, SL_CPC_RX_INTERNAL_BUFFER_HANDLE);
  if (status != SL_STATUS_OK) {
    goto hdlc_header_allocated;
  }

  buf_handle->data = data;
  buf_handle->hdlc_header = hdlc_header;
  buf_handle->data_length = 0u;
  buf_handle->reason = SL_CPC_REJECT_NO_ERROR;
  *handle = buf_handle;

  return SL_STATUS_OK;

  // Error path:
  hdlc_header_allocated:
  sli_cpc_free_hdlc_header(inst, hdlc_header);

  rx_buffer_allocated:
  if (alloc_rx_buffer) {
    free_cpc_rx_buffer(data);
  }
  return status;
}

/***************************************************************************//**
 * Get a CPC RAW buffer for reception.
 ******************************************************************************/
sl_status_t sli_cpc_get_raw_rx_buffer(sli_cpc_instance_t *inst, uint8_t **raw_rx_buffer)
{
  *raw_rx_buffer = alloc_cpc_rx_buffer(inst);
  if (*raw_rx_buffer == NULL) {
    return SL_STATUS_NO_MORE_RESOURCE;
  }

  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Get a CPC RAW buffer for reception.
 ******************************************************************************/
sl_status_t sli_cpc_free_raw_rx_buffer(sli_cpc_instance_t *inst, uint8_t *raw_rx_buffer)
{
  (void)inst;

  if (raw_rx_buffer != NULL) {
    free_cpc_rx_buffer(raw_rx_buffer);
  } else {
    return SL_STATUS_NULL_POINTER;
  }

  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Free rx handle and all associate buffers
 ******************************************************************************/
sl_status_t sli_cpc_free_buffer_handle(sl_cpc_buffer_handle_t *handle)
{
  bool rx_buffer_handle_freed = false;
  sli_cpc_instance_t *inst;
  MCU_DECLARE_IRQ_STATE;

  if (handle == NULL) {
    return SL_STATUS_NULL_POINTER;
  }

  if (handle->ref_count > 0) {
    // Can't free the buffer_handle, it is being used elsewhere
    return SL_STATUS_BUSY;
  }

  inst = handle->instance;
  SLI_CPC_ASSERT(inst != NULL);

  // Check if the buffer handle was still pointing to another buffer handle
  // If so, it could cause a dangling buffer_handle not referenced in any list
  SLI_CPC_ASSERT(handle->core_node.node == NULL);
  SLI_CPC_ASSERT(handle->driver_node.node == NULL);

  if (handle->hdlc_header != NULL) {
    sli_cpc_free_hdlc_header(inst, handle->hdlc_header);
  }

#if (SL_CPC_ENDPOINT_SECURITY_ENABLED >= 1)
  sli_cpc_free_security_tag_buffer(inst, handle->security_tag);
#endif

  switch (handle->type) {
    case SL_CPC_RX_USER_BUFFER_HANDLE:
      rx_buffer_handle_freed = true;
      break;
    case SL_CPC_RX_INTERNAL_BUFFER_HANDLE:
      // Buffer was allocated but handle is freed before being passed to user
      // This means that we must free the rx buffer or it will be leaked

      if (handle->data != NULL) {
        sli_cpc_free_rx_buffer(inst, handle->data);
      }
      rx_buffer_handle_freed = true;
#ifdef CPC_DEBUG_TRACE
      if (handle->endpoint != NULL) {
        SLI_CPC_DEBUG_TRACE_ENDPOINT_RXD_DATA_FRAME_DROPPED(handle->endpoint);
      } else {
        SLI_CPC_DEBUG_TRACE_CORE_RXD_DATA_FRAME_DROPPED(inst);
      }
#endif
      break;
    case SL_CPC_TX_REJECT_BUFFER_HANDLE:
      if (handle->data) {
        free_object(&inst->mempool_hdlc_reject, handle->data);
      }
      SL_FALLTHROUGH
    case SL_CPC_TX_SFRAME_BUFFER_HANDLE:
    case SL_CPC_TX_DATA_BUFFER_HANDLE:
    case SL_CPC_UNKNOWN_BUFFER_HANDLE:
      break;
    default:
      SLI_CPC_ASSERT(0);
      break;
  }

  // Update counters
  MCU_ENTER_ATOMIC();
  free_object(&inst->mempool_buffer_handle, handle);

  if (handle->type == SL_CPC_TX_DATA_BUFFER_HANDLE) {
#if defined(SL_CATALOG_KERNEL_PRESENT)
    osStatus_t sem_status = osSemaphoreRelease(inst->tx_item_availability_signal);
    SLI_CPC_ASSERT(sem_status == osOK);
#else
    SLI_CPC_ASSERT(inst->tx_item_used_count > 0);
    --inst->tx_item_used_count;
#endif
  } else if (handle->type == SL_CPC_TX_SFRAME_BUFFER_HANDLE || handle->type == SL_CPC_TX_REJECT_BUFFER_HANDLE) {
    SLI_CPC_ASSERT(inst->sframe_tx_item_used_count > 0);
    --inst->sframe_tx_item_used_count;
  }
  MCU_EXIT_ATOMIC();

  if (rx_buffer_handle_freed) {
    // Notify the driver
    if (inst->driver->ops.on_rx_buffer_handle_free) {
      inst->driver->ops.on_rx_buffer_handle_free(inst->driver);
    }
  }

  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Convert rx buffer handle to a receive queue item;
 *  Free hdlc header and handle
 *  Alloc queue item and set fields
 ******************************************************************************/
sl_status_t sli_cpc_push_back_rx_data_in_receive_queue(sli_cpc_instance_t *inst,
                                                       sl_cpc_buffer_handle_t *handle,
                                                       sl_slist_node_t **head,
                                                       uint16_t data_length)
{
  sl_cpc_receive_queue_item_t *item;
  sl_status_t status;

  if (handle == NULL || head == NULL) {
    return SL_STATUS_NULL_POINTER;
  }

  status = sli_cpc_get_receive_queue_item(inst, &item);
  if (status != SL_STATUS_OK) {
    return status;
  }

  item->node.node = NULL;
  item->data = handle->data;
  SLI_CPC_ASSERT(handle->type == SL_CPC_RX_INTERNAL_BUFFER_HANDLE);
  item->buffer_type = SL_CPC_RX_USER_BUFFER_HANDLE;
  item->data_length = data_length;

  sl_slist_push_back(head, &item->node);

  // Buffer is now owned by the user
  handle->type = SL_CPC_RX_USER_BUFFER_HANDLE;

  sli_cpc_free_buffer_handle(handle);

  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Free rx buffer returned by sl_cpc_read().
 ******************************************************************************/
sl_status_t sl_cpc_free_rx_buffer(void *data)
{
  sli_cpc_instance_t *inst;

  if (data == NULL) {
    return SL_STATUS_NULL_POINTER;
  }

  inst = free_cpc_rx_buffer(data);

  free_receive_queue_item_from_postponed_list(inst);

  // Notify that at least one RX buffer is available
  notify_rx_buffer_free(inst);

  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Free internal rx buffer; Not pushed in RX Queue.
 ******************************************************************************/
sl_status_t sli_cpc_free_rx_buffer(sli_cpc_instance_t *inst, void *data)
{
  sli_cpc_instance_t *data_instance;

  if (data == NULL) {
    return SL_STATUS_NULL_POINTER;
  }

  data_instance = free_cpc_rx_buffer(data);
  SLI_CPC_ASSERT(data_instance == inst);

  // Notify that at least one RX buffer is available
  notify_rx_buffer_free(inst);

  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Free CPC hdlc header
 ******************************************************************************/
sl_status_t sli_cpc_free_hdlc_header(sli_cpc_instance_t *inst, void *data)
{
  if (data == NULL) {
    return SL_STATUS_NULL_POINTER;
  }

  free_object(&inst->mempool_hdlc_header, data);

  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Get receive queue item
 ******************************************************************************/
sl_status_t sli_cpc_get_receive_queue_item(sli_cpc_instance_t *inst, sl_cpc_receive_queue_item_t **item)
{
  if (item == NULL) {
    return SL_STATUS_NULL_POINTER;
  }

  *item = alloc_object(&inst->mempool_rx_queue_item);
  if (*item == NULL) {
    return SL_STATUS_NO_MORE_RESOURCE;
  }

  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Free receive queue item
 ******************************************************************************/
static sl_status_t free_receive_queue_item(sli_cpc_instance_t *inst,
                                           sl_cpc_receive_queue_item_t *item)
{
  if (item == NULL) {
    return SL_STATUS_NULL_POINTER;
  }

  free_object(&inst->mempool_rx_queue_item, item);

  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Free receive queue item and the data buffer
 ******************************************************************************/
void sli_cpc_free_receive_queue_item(sli_cpc_instance_t *inst, sl_cpc_receive_queue_item_t *item)
{
  if (item == NULL) {
    return;
  }

  if (item->data != NULL) {
    sl_status_t status;

    SLI_CPC_ASSERT(item->buffer_type == SL_CPC_RX_USER_BUFFER_HANDLE);

    status = sli_cpc_free_rx_buffer(inst, item->data);
    SLI_CPC_ASSERT(status == SL_STATUS_OK);
  }

  free_object(&inst->mempool_rx_queue_item, item);
}

/***************************************************************************//**
 * Push receive queue item queued to the postponed free list
 ******************************************************************************/
sl_status_t sli_cpc_push_receive_queue_item_to_postponed_list(sli_cpc_instance_t *inst,
                                                              sl_cpc_receive_queue_item_t *rx_queue_item,
                                                              void **data,
                                                              uint16_t * data_length)
{
  *data = rx_queue_item->data;
  *data_length = rx_queue_item->data_length;

  sl_slist_push(&inst->postponed_free_rx_queue_item, &rx_queue_item->node);

  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Free receive queue item queued in the postponed free list
 ******************************************************************************/
static sl_status_t free_receive_queue_item_from_postponed_list(sli_cpc_instance_t *inst)
{
  sl_slist_node_t *node = sl_slist_pop(&inst->postponed_free_rx_queue_item);
  sl_cpc_receive_queue_item_t *item = SL_SLIST_ENTRY(node, sl_cpc_receive_queue_item_t, node);
  SLI_CPC_ASSERT(item != NULL);

  free_receive_queue_item(inst, item);

  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Get endpoint
 ******************************************************************************/
sl_status_t sli_cpc_get_endpoint(sli_cpc_instance_t *inst, sl_cpc_endpoint_t **endpoint)
{
  sl_status_t status = SL_STATUS_OK;
  #if (SL_CPC_DEBUG_MEMORY_ALLOCATOR_COUNTERS == 1)
  MCU_DECLARE_IRQ_STATE;
  #endif

  if (endpoint == NULL) {
    return SL_STATUS_NULL_POINTER;
  }

  #if (SL_CPC_DEBUG_MEMORY_ALLOCATOR_COUNTERS == 1)
  MCU_ENTER_ATOMIC();
  #endif

  // Dynamically allocate the endpoint
  *endpoint = calloc(1, sizeof(sl_cpc_endpoint_t));
  if (*endpoint == NULL) {
    status = SL_STATUS_NO_MORE_RESOURCE;
  }

  #if (SL_CPC_DEBUG_MEMORY_ALLOCATOR_COUNTERS == 1)
  // Increment debug counter on allocation success
  if (status == SL_STATUS_OK) {
    inst->endpoints_used_block_cnt++;
  }
  MCU_EXIT_ATOMIC();
  #else
  (void)inst;
  #endif

  return status;
}

/***************************************************************************//**
 * Free endpoint
 ******************************************************************************/
void sli_cpc_free_endpoint(sli_cpc_instance_t *inst, sl_cpc_endpoint_t *endpoint)
{
  SLI_CPC_ASSERT(endpoint != NULL);
  #if (SL_CPC_DEBUG_MEMORY_ALLOCATOR_COUNTERS == 1)
  MCU_DECLARE_IRQ_STATE;
  MCU_ENTER_ATOMIC();
  #endif

  #if defined(SL_CATALOG_MEMORY_MANAGER_PRESENT)
  sl_free(endpoint);
  #else
  free(endpoint);
  #endif

  #if (SL_CPC_DEBUG_MEMORY_ALLOCATOR_COUNTERS == 1)
  // Decrement debug counter on free
  inst->endpoints_used_block_cnt--;
  MCU_EXIT_ATOMIC();
  #else
  (void)inst;
  #endif
}

/***************************************************************************//**
 * Pop a buffer handle from a list owned by the CPC core
 ******************************************************************************/
sl_cpc_buffer_handle_t* sli_cpc_pop_core_buffer_handle(sl_slist_node_t **head)
{
  sl_slist_node_t *item_node = sl_slist_pop(head);
  if (item_node == NULL) {
    return NULL;
  }
  sl_cpc_buffer_handle_t *item = SL_SLIST_ENTRY(item_node, sl_cpc_buffer_handle_t, core_node);
  SLI_CPC_ASSERT(item != NULL);
  SLI_CPC_ASSERT(item->ref_count > 0);
  --item->ref_count;
  return item;
}

/***************************************************************************//**
 * Pop a buffer handle from a list owned by the CPC driver
 ******************************************************************************/
sl_cpc_buffer_handle_t* sli_cpc_pop_driver_buffer_handle(sl_slist_node_t **head)
{
  sl_slist_node_t *item_node = sl_slist_pop(head);
  if (item_node == NULL) {
    return NULL;
  }
  sl_cpc_buffer_handle_t *item = SL_SLIST_ENTRY(item_node, sl_cpc_buffer_handle_t, driver_node);
  SLI_CPC_ASSERT(item != NULL);
  SLI_CPC_ASSERT(item->ref_count > 0);
  --item->ref_count;
  return item;
}

/***************************************************************************//**
 * Remove a buffer handle from a list owned by the CPC core
 ******************************************************************************/
void sli_cpc_remove_core_buffer_handle(sl_slist_node_t **head, sl_cpc_buffer_handle_t *buffer_handle)
{
  SLI_CPC_ASSERT(buffer_handle != NULL);
  SLI_CPC_ASSERT(buffer_handle->ref_count > 0);
  buffer_handle->ref_count--;
  sl_slist_remove(head, &buffer_handle->core_node);
}

/***************************************************************************//**
 * Remove a buffer handle from a list owned by the CPC driver
 ******************************************************************************/
void sli_cpc_remove_driver_buffer_handle(sl_slist_node_t **head, sl_cpc_buffer_handle_t *buffer_handle)
{
  SLI_CPC_ASSERT(buffer_handle != NULL);
  SLI_CPC_ASSERT(buffer_handle->ref_count > 0);
  buffer_handle->ref_count--;
  sl_slist_remove(head, &buffer_handle->driver_node);
}

/***************************************************************************//**
 * Push back a list item containing an allocated buffer handle for the core
 ******************************************************************************/
void sli_cpc_push_back_core_buffer_handle(sl_slist_node_t **head, sl_cpc_buffer_handle_t *buf_handle)
{
  SLI_CPC_ASSERT(buf_handle != NULL);
  buf_handle->ref_count++;
  SLI_CPC_ASSERT(buf_handle->ref_count <= 2);
  sl_slist_push_back(head, &buf_handle->core_node);
}

/***************************************************************************//**
 * Push back a list item containing an allocated buffer handle for the driver
 ******************************************************************************/
void sli_cpc_push_back_driver_buffer_handle(sl_slist_node_t **head, sl_cpc_buffer_handle_t *buf_handle)
{
  SLI_CPC_ASSERT(buf_handle != NULL);
  buf_handle->ref_count++;
  SLI_CPC_ASSERT(buf_handle->ref_count <= 2);
  sl_slist_push_back(head, &buf_handle->driver_node);
}

/***************************************************************************//**
 * Push a list item containing an allocated buffer handle for the core
 ******************************************************************************/
void sli_cpc_push_core_buffer_handle(sl_slist_node_t **head, sl_cpc_buffer_handle_t *buf_handle)
{
  SLI_CPC_ASSERT(buf_handle != NULL);
  buf_handle->ref_count++;
  SLI_CPC_ASSERT(buf_handle->ref_count <= 2);
  sl_slist_push(head, &buf_handle->core_node);
}

/***************************************************************************//**
 * Push a list item containing an allocated buffer handle for the driver
 ******************************************************************************/
void sli_cpc_push_driver_buffer_handle(sl_slist_node_t **head, sl_cpc_buffer_handle_t *buf_handle)
{
  SLI_CPC_ASSERT(buf_handle != NULL);
  buf_handle->ref_count++;
  SLI_CPC_ASSERT(buf_handle->ref_count <= 2);
  sl_slist_push(head, &buf_handle->driver_node);
}

#if defined(SL_CATALOG_CPC_PRIMARY_PRESENT)
/***************************************************************************//**
 * Get a system command handle
 ******************************************************************************/
sl_status_t sli_cpc_get_system_command_handle(sli_cpc_instance_t *inst, sli_cpc_system_command_handle_t **item)
{
  if (item == NULL) {
    return SL_STATUS_NULL_POINTER;
  }

  *item = alloc_object(&inst->mempool_command_handle);
  if (*item == NULL) {
    return SL_STATUS_NO_MORE_RESOURCE;
  }

  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Free a command buffer
 ******************************************************************************/
sl_status_t sli_cpc_free_system_command_handle(sli_cpc_instance_t *inst, sli_cpc_system_command_handle_t *item)
{
  if (item == NULL) {
    return SL_STATUS_NULL_POINTER;
  }

  free_object(&inst->mempool_command_handle, item);

  return SL_STATUS_OK;
}
#endif

#if defined(SL_CATALOG_CPC_SECONDARY_PRESENT)
/***************************************************************************//**
 * Get a command buffer
 ******************************************************************************/
sl_status_t sli_cpc_get_system_command_context(sli_cpc_instance_t *inst, sli_cpc_system_cmd_context_t **item)
{
  if (item == NULL) {
    return SL_STATUS_NULL_POINTER;
  }

  *item = alloc_object(&inst->mempool_system_command);
  if (*item == NULL) {
    return SL_STATUS_NO_MORE_RESOURCE;
  }

  (*item)->ep = NULL;
  (*item)->ep_id = 0;
  (*item)->on_complete_arg = NULL;

  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Free a command context
 ******************************************************************************/
sl_status_t sli_cpc_free_system_command_context(sli_cpc_instance_t *inst, sli_cpc_system_cmd_context_t *item)
{
  if (item == NULL) {
    return SL_STATUS_NULL_POINTER;
  }

  free_object(&inst->mempool_system_command, item);

  return SL_STATUS_OK;
}
#endif

/***************************************************************************//**
 * Allocate a security tag buffer
 ******************************************************************************/
sl_status_t sli_cpc_get_security_tag_buffer(sli_cpc_instance_t *inst, void **tag_buffer)
{
#if (SL_CPC_ENDPOINT_SECURITY_ENABLED >= 1)
  if (tag_buffer == NULL) {
    return SL_STATUS_NULL_POINTER;
  }

  *tag_buffer = alloc_object(&inst->mempool_tx_security_tag);
  if (*tag_buffer == NULL) {
    return SL_STATUS_NO_MORE_RESOURCE;
  }

  return SL_STATUS_OK;
#else
  (void)inst;
  (void)tag_buffer;

  return SL_STATUS_NOT_AVAILABLE;
#endif
}

/***************************************************************************//**
 * Free a security tag buffer
 ******************************************************************************/
sl_status_t sli_cpc_free_security_tag_buffer(sli_cpc_instance_t *inst, void *tag_buffer)
{
#if (SL_CPC_ENDPOINT_SECURITY_ENABLED >= 1)
  if (tag_buffer == NULL) {
    return SL_STATUS_NULL_POINTER;
  }

  free_object(&inst->mempool_tx_security_tag, tag_buffer);

  return SL_STATUS_OK;
#else
  (void)inst;
  (void)tag_buffer;

  return SL_STATUS_NOT_AVAILABLE;
#endif
}

/***************************************************************************//**
 * Alloc object from a specified pool; Manage stat as well
 ******************************************************************************/
static void* alloc_object(sl_cpc_mem_pool_handle_t *pool)
{
  void *block;

  #if (SL_CPC_DEBUG_MEMORY_ALLOCATOR_COUNTERS == 1)
  MCU_DECLARE_IRQ_STATE;
  MCU_ENTER_ATOMIC();
  #endif

  #if defined(SL_CATALOG_MEMORY_MANAGER_PRESENT)
  sl_status_t status = sl_memory_pool_alloc(pool->handle,
                                            &block);
  if (status != SL_STATUS_OK) {
    // If allocation failed, null the pointer to keep the same error handling
    // with sli_mem_pool and memory manager.
    block = NULL;
  }
  #else
  block = sli_mem_pool_alloc(&pool->handle);
  #endif
  #if (SL_CPC_DEBUG_MEMORY_ALLOCATOR_COUNTERS == 1)
  if (block != NULL) {
    pool->used_block_cnt++;
  }
  MCU_EXIT_ATOMIC();
  #endif

  return block;
}

/***************************************************************************//**
 * Free object from a specified pool; Manage stat as well
 ******************************************************************************/
static void free_object(sl_cpc_mem_pool_handle_t *pool,
                        void *block)
{
  #if (SL_CPC_DEBUG_MEMORY_ALLOCATOR_COUNTERS == 1)
  MCU_DECLARE_IRQ_STATE;
  MCU_ENTER_ATOMIC();
  #endif

  #if defined(SL_CATALOG_MEMORY_MANAGER_PRESENT)
  sl_status_t status = sl_memory_pool_free(pool->handle,
                                           block);
  SLI_CPC_ASSERT(status == SL_STATUS_OK);
  #else
  sli_mem_pool_free(&pool->handle, block);
  #endif

  #if (SL_CPC_DEBUG_MEMORY_ALLOCATOR_COUNTERS == 1)
  pool->used_block_cnt--;
  MCU_EXIT_ATOMIC();
  #endif
}

/***************************************************************************//**
 * Notify driver that an RX buffer has been freed
 ******************************************************************************/
static void notify_rx_buffer_free(sli_cpc_instance_t *inst)
{
  if (inst->driver->ops.on_rx_buffer_free) {
    inst->driver->ops.on_rx_buffer_free(inst->driver);
  }
}
