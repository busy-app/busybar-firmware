/***************************************************************************/ /**
 * @file sli_buffer_manager.c
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
#include "sli_buffer_manager.h"
#include "sl_status.h"
#include "sl_constants.h"
#include "stdlib.h"
#include "string.h"
#include "cmsis_os2.h"
#include "sl_core.h"
#include "sl_slist.h"

#define SLI_MEM_POOL_BLOCK_SIZE(x)               \
  (x                                             \
   + sizeof(sli_buffer_manager_mempool_handler_t \
              *)) ///< Calculate the block size based on metadata present in the internal buffer.

#define SLI_MAX_MEMPOOL_HANDLERS_COUNT SLI_BUFFER_MANAGER_MAX_POOL ///< Maximum number of memory pools.

#define SLI_MINIUM_ELEMENTS_IN_COMMON_MEMPOOL_QUEUE \
  1 ///< This macro determines minimum number of common mempools present in the common mempool queue.

#define SLI_ZERO_TIMEOUT 0

/***************************************************************************************************************** 
 * @brief Internal structures
*********************************************************************************************************************/
typedef struct {
  sl_slist_node_t next;          //< Next node(used only in case of common mempool).
  void *mempool_memory;          ///< Memory pool memory.
  sli_mem_pool_handle_t mempool; ///< Memory pool handler.

  uint16_t max_buffer_count;       ///< Maximum buffer count.
  uint16_t allocated_buffer_count; ///< Allocated buffer count.
  bool is_common_pool;             ///< Whether the buffer has been allocated from common mempool.
} sli_buffer_manager_mempool_handler_t;

#pragma pack(1)
typedef struct {
  sli_buffer_manager_mempool_handler_t
    *buffer_manager_mempool_handler; ///< pointer of the mempool from which the data has been allocated.
  uint8_t data[];                    ///< Data.
} sli_internal_buffer_t;

typedef struct {
  sli_buffer_manager_mempool_handler_t *head; ///< Head of the queue.
  sli_buffer_manager_mempool_handler_t *tail; ///< Tail of the queue.

  sli_buffer_manager_mempool_handler_t *last_used_handler; ///< Pointer to the last used common mempool.
  uint8_t size;                                            ///< Number of common mempools in the queue.
} sli_buffer_manager_mempool_queue_t;

/***************************************************************************************************************** 
 * Static variables
 * ****************************************************************************************************************/
static sli_buffer_manager_mempool_handler_t dedicated_mempool_handlers[SLI_MAX_MEMPOOL_HANDLERS_COUNT] = { 0 };
static sli_buffer_manager_mempool_queue_t common_mempool_queue                                         = { 0 };
static sli_buffer_manager_pool_info_t common_mempool_configuration                                     = { 0 };
/***************************************************************************************************************** 
 * Static functions
 * ****************************************************************************************************************/

/**
 * @brief Function to check if the timeout has expired.
 *
 * @param start_time Start time.
 * @param wait_time Wait time.
 * @return true if the timeout has expired.
 */
inline static bool sli_buffer_manager_has_timeout_expired(uint32_t start_time, uint32_t wait_time)
{
  return ((osKernelGetTickCount() - start_time) > wait_time);
}

/**
 * @brief Function to create and assign a SiSDK's mempool to sli_buffer_manager_mempool_handler_t.
 *
 * @param configuration Memory pool configuration.
 * @param mempool_handler Memory pool handler.
 * @return SL_STATUS_OK if the operation is successful.
 */
static sl_status_t sli_buffer_manager_create_and_assign_mempool(sli_buffer_manager_pool_info_t *configuration,
                                                                sli_buffer_manager_mempool_handler_t *mempool_handler,
                                                                bool is_common_pool)
{
  CORE_irqState_t state           = CORE_EnterAtomic();
  size_t buffer_size              = (configuration->block_count * SLI_MEM_POOL_BLOCK_SIZE(configuration->block_size));
  mempool_handler->mempool_memory = malloc(buffer_size);
  if (mempool_handler->mempool_memory == NULL) {
    CORE_ExitAtomic(state);
    return SL_STATUS_ALLOCATION_FAILED;
  }

  sli_mem_pool_create(&mempool_handler->mempool,
                      SLI_MEM_POOL_BLOCK_SIZE(configuration->block_size),
                      configuration->block_count,
                      mempool_handler->mempool_memory,
                      buffer_size);

  mempool_handler->max_buffer_count       = configuration->block_count;
  mempool_handler->allocated_buffer_count = 0;
  mempool_handler->is_common_pool         = is_common_pool;

  CORE_ExitAtomic(state);

  return SL_STATUS_OK;
}

/**
 * @brief Function to allocate a buffer from the dedicated pool.
 *
 * @param buffer Buffer.
 * @param pool_type Pool type.
 * @param start_time Start time.
 * @param wait_duration_ms Wait duration.
 * @return SL_STATUS_OK if the operation is successful.
 */
static sl_status_t sli_buffer_manager_allocate_buffer_from_dedicated_pool(
  sli_internal_buffer_t **buffer,
  const sli_buffer_manager_pool_types_t pool_type,
  uint32_t start_time,
  uint32_t wait_duration_ms)
{
  sli_buffer_manager_mempool_handler_t *mempool_handler = &dedicated_mempool_handlers[pool_type];
  if (mempool_handler->mempool_memory == NULL) {
    return SL_STATUS_NOT_INITIALIZED;
  }
  *buffer = NULL;

  do {
    CORE_irqState_t state = CORE_EnterAtomic();

    if (mempool_handler->allocated_buffer_count >= mempool_handler->max_buffer_count) {
      CORE_ExitAtomic(state);
      continue;
    }

    *buffer = (sli_internal_buffer_t *)sli_mem_pool_alloc(&mempool_handler->mempool);
    if (*buffer != NULL) {
      (*buffer)->buffer_manager_mempool_handler = mempool_handler;
      mempool_handler->allocated_buffer_count++;
    }

    CORE_ExitAtomic(state);
  } while (((osKernelGetTickCount() - start_time) < wait_duration_ms) && (*buffer == NULL));

  return (*buffer == NULL) ? SL_STATUS_ALLOCATION_FAILED : SL_STATUS_OK;
}

/**
 * @brief Function to allocate a buffer from the common pool.
 *
 * @param buffer Buffer.
 * @param start_time Start time.
 * @param wait_duration_ms Wait duration.
 * @return SL_STATUS_OK if the operation is successful.
 */
static sl_status_t sli_buffer_manager_allocate_buffer_from_common_pool(sli_internal_buffer_t **buffer,
                                                                       uint32_t start_time,
                                                                       const uint32_t wait_duration_ms)
{
  CORE_irqState_t state = CORE_EnterAtomic();

  // If there are no common mempools in the queue, return.
  if (common_mempool_queue.size == 0) {
    CORE_ExitAtomic(state);
    return SL_STATUS_FAIL;
  }

  sli_buffer_manager_mempool_handler_t *mempool_handler = common_mempool_queue.last_used_handler;
  *buffer                                               = NULL;

  do {

    if (mempool_handler->allocated_buffer_count >= mempool_handler->max_buffer_count) {
      mempool_handler = (sli_buffer_manager_mempool_handler_t *)mempool_handler->next.node;
      continue;
    }

    *buffer = (sli_internal_buffer_t *)sli_mem_pool_alloc(&mempool_handler->mempool);
    if ((*buffer) != NULL) {
      (*buffer)->buffer_manager_mempool_handler = mempool_handler;
      mempool_handler->allocated_buffer_count++;
      common_mempool_queue.last_used_handler = mempool_handler;
      CORE_ExitAtomic(state);
      return SL_STATUS_OK;
    }

    // If the buffer is not allocated, move to the next mempool handler.
    mempool_handler = (sli_buffer_manager_mempool_handler_t *)mempool_handler->next.node;

  } while (((osKernelGetTickCount() - start_time) < wait_duration_ms)
           && (mempool_handler != common_mempool_queue.last_used_handler));

  CORE_ExitAtomic(state);
  return SL_STATUS_ALLOCATION_FAILED;
}

/**
 * @brief Function to create a new common memory pool and append it to the common mempool queue.
 *
 * @return SL_STATUS_OK if the operation is successful.
 */
static sl_status_t sli_buffer_manager_create_new_common_mempool(void)
{

  CORE_irqState_t state                                 = CORE_EnterAtomic();
  sli_buffer_manager_mempool_handler_t *mempool_handler = malloc(sizeof(sli_buffer_manager_mempool_handler_t));

  if (mempool_handler == NULL) {
    CORE_ExitAtomic(state);
    return SL_STATUS_ALLOCATION_FAILED;
  }

  memset(mempool_handler, 0, sizeof(sli_buffer_manager_mempool_handler_t));

  sl_status_t status =
    sli_buffer_manager_create_and_assign_mempool(&common_mempool_configuration, mempool_handler, true);

  if (status != SL_STATUS_OK) {
    free(mempool_handler);

    CORE_ExitAtomic(state);
    return status;
  }

  if (common_mempool_queue.head == NULL && common_mempool_queue.tail == NULL) {
    common_mempool_queue.head = mempool_handler;
    common_mempool_queue.tail = mempool_handler;

  } else {
    common_mempool_queue.tail->next.node = (sl_slist_node_t *)mempool_handler;
    common_mempool_queue.tail            = mempool_handler;
  }

  // Assign head as the next node of the tail.
  mempool_handler->next.node = (sl_slist_node_t *)common_mempool_queue.head;

  // Assign last_used_handler pointer to the newly created mempool.
  common_mempool_queue.last_used_handler = mempool_handler;

  common_mempool_queue.size++;
  CORE_ExitAtomic(state);
  return SL_STATUS_OK;
}

/**
 * @brief Function to remove a mempool from the common mempool queue.
 *
 * @param common_pool_handler Common  mempool handler.
 * @return SL_STATUS_OK if the operation is successful.
 */
static sl_status_t sli_buffer_manager_free_a_common_mempool_from_queue(
  sli_buffer_manager_mempool_handler_t *common_pool_handler)
{
  CORE_irqState_t state = CORE_EnterAtomic();

  // If the node to be removed is the head node, update the head pointer.
  if (common_pool_handler == common_mempool_queue.head) {
    common_mempool_queue.head            = (sli_buffer_manager_mempool_handler_t *)common_pool_handler->next.node;
    common_mempool_queue.tail->next.node = (sl_slist_node_t *)common_mempool_queue.head;

    if (common_mempool_queue.last_used_handler == common_pool_handler) {
      common_mempool_queue.last_used_handler = common_mempool_queue.tail;
    }

    free(common_pool_handler->mempool_memory);
    free(common_pool_handler);

    common_mempool_queue.size--;
    CORE_ExitAtomic(state);
    return SL_STATUS_OK;
  }

  sli_buffer_manager_mempool_handler_t *current_node =
    (sli_buffer_manager_mempool_handler_t *)common_mempool_queue.head->next.node;
  sli_buffer_manager_mempool_handler_t *previous_node = common_mempool_queue.head;

  // Loop until we find the required node or we reached the head again.
  while (current_node != common_pool_handler && current_node != common_mempool_queue.head) {
    previous_node = current_node;
    current_node  = (sli_buffer_manager_mempool_handler_t *)current_node->next.node;
  }

  if (current_node != common_pool_handler) {
    CORE_ExitAtomic(state);
    return SL_STATUS_NOT_FOUND;
  }

  // Update the next pointer of the previous node to the next pointer of the current node.
  previous_node->next.node = current_node->next.node;

  // If node that is to be removed is tail node, update the tail pointer.
  if (current_node == common_mempool_queue.tail) {
    common_mempool_queue.tail = (sli_buffer_manager_mempool_handler_t *)current_node->next.node;
  }

  // Update the last_used_handler pointer if the node that is to be removed is the last_used_handler node.
  if (common_mempool_queue.last_used_handler == common_pool_handler) {
    common_mempool_queue.last_used_handler = common_mempool_queue.tail;
  }

  free(current_node->mempool_memory);
  free(current_node);

  common_mempool_queue.size--;

  CORE_ExitAtomic(state);
  return SL_STATUS_OK;
}

/**
 * @brief Function to free all the common mempools.
 *
 * @return SL_STATUS_OK if the operation is successful.
 */
static sl_status_t sli_buffer_manager_free_all_common_mempools(void)
{
  CORE_irqState_t state = CORE_EnterAtomic();

  ///< If there are no common mempools, return.
  if (common_mempool_queue.size <= 0) {
    CORE_ExitAtomic(state);
    return SL_STATUS_OK;
  }

  sli_buffer_manager_mempool_handler_t *head                = common_mempool_queue.head;
  sli_buffer_manager_mempool_handler_t *mempool_to_be_freed = NULL;

  // Note: we are intentionally not updating the tail reference
  do {
    mempool_to_be_freed = head;
    head                = (sli_buffer_manager_mempool_handler_t *)mempool_to_be_freed->next.node;

    free(mempool_to_be_freed->mempool_memory);
    free(mempool_to_be_freed);
  } while (mempool_to_be_freed != common_mempool_queue.tail);

  memset(&common_mempool_queue, 0, sizeof(sli_buffer_manager_mempool_queue_t));
  memset(&common_mempool_configuration, 0, sizeof(sli_buffer_manager_pool_info_t));

  CORE_ExitAtomic(state);

  return SL_STATUS_OK;
}

static sl_status_t sli_buffer_manager_free_all_mempools(void)
{
  CORE_irqState_t state = CORE_EnterAtomic();

  // Free the dedicated mempools.
  for (uint8_t index = 0; index < SLI_MAX_MEMPOOL_HANDLERS_COUNT; index++) {
    sli_buffer_manager_mempool_handler_t *mempool_handler = &dedicated_mempool_handlers[index];

    if (mempool_handler->mempool_memory != NULL) {
      free(mempool_handler->mempool_memory);
      memset(mempool_handler, 0, sizeof(sli_buffer_manager_mempool_handler_t));
    }
  }

  // Free the common mempool.
  sli_buffer_manager_free_all_common_mempools();

  CORE_ExitAtomic(state);

  return SL_STATUS_OK;
}

/***************************************************************************************************************** 
 * Public functions
 * ****************************************************************************************************************/
sl_status_t sli_buffer_manager_init(sli_buffer_manager_configuration_t *configuration)
{
  SL_VERIFY_POINTER_OR_RETURN(configuration, SL_STATUS_NULL_POINTER);
  sl_status_t status = SL_STATUS_ALLOCATION_FAILED;
  if (configuration->common_pool_info.block_count == 0 || configuration->common_pool_info.block_size == 0) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  for (uint8_t index = 0; index < SLI_BUFFER_MANAGER_MAX_POOL; index++) {
    if (configuration->pool_info[index] == NULL) {
      continue;
    }
    if (configuration->pool_info[index]->block_count == 0 || configuration->pool_info[index]->block_size == 0) {
      sli_buffer_manager_free_all_mempools();
      return SL_STATUS_INVALID_PARAMETER;
    }
    status = sli_buffer_manager_create_and_assign_mempool(configuration->pool_info[index],
                                                          &dedicated_mempool_handlers[index],
                                                          false);
    if (status != SL_STATUS_OK) {
      sli_buffer_manager_free_all_mempools();
      return SL_STATUS_NO_MORE_RESOURCE;
    }
  }

  memcpy(&common_mempool_configuration, &configuration->common_pool_info, sizeof(sli_buffer_manager_pool_info_t));
  status = sli_buffer_manager_create_new_common_mempool();

  if (status != SL_STATUS_OK) {
    sli_buffer_manager_free_all_mempools();
    return SL_STATUS_NO_MORE_RESOURCE;
  }

  return SL_STATUS_OK;
}

sl_status_t sli_buffer_manager_deinit(void)
{
  sli_buffer_manager_free_all_mempools();
  return SL_STATUS_OK;
}

sl_status_t sli_buffer_manager_allocate_buffer(const sli_buffer_manager_pool_types_t pool_type,
                                               const sli_buffer_manager_allocation_types_t allocation_type,
                                               const uint32_t wait_duration_ms,
                                               sli_buffer_t *buffer)
{
  sli_internal_buffer_t *internal_buffer = NULL;
  uint32_t start                         = osKernelGetTickCount();
  // Allocate buffer from the dedicated pool incase of SLI_BUFFER_MANAGER_ALLOCATION_TYPE_DEDICATED.
  if (allocation_type == SLI_BUFFER_MANAGER_ALLOCATION_TYPE_DEDICATED) {
    sl_status_t status =
      sli_buffer_manager_allocate_buffer_from_dedicated_pool(&internal_buffer, pool_type, start, wait_duration_ms);
    VERIFY_STATUS_AND_RETURN(status);

    *buffer = internal_buffer->data;
    return SL_STATUS_OK;
  }

  sl_status_t status = sli_buffer_manager_allocate_buffer_from_common_pool(&internal_buffer, start, wait_duration_ms);
  // If buffer is to be allocated from uninitialized common pool, return error.
  if (status == SL_STATUS_FAIL) {
    return SL_STATUS_NOT_INITIALIZED;
  }
  // If the buffer is not allocated from the common pool, allocate from the dedicated pool.
  if (status != SL_STATUS_OK && !sli_buffer_manager_has_timeout_expired(start, wait_duration_ms)) {
    status =
      sli_buffer_manager_allocate_buffer_from_dedicated_pool(&internal_buffer, pool_type, start, SLI_ZERO_TIMEOUT);
  }

  // If the buffer is not allocated from the dedicated pool, create a new common pool and allocate from it.
  if (status != SL_STATUS_OK && !sli_buffer_manager_has_timeout_expired(start, wait_duration_ms)) {
    status = sli_buffer_manager_create_new_common_mempool();
    VERIFY_STATUS_AND_RETURN(status);
    sli_buffer_manager_allocate_buffer_from_common_pool(&internal_buffer, start, wait_duration_ms);
  }

  // If the buffer is still not allocated, return error.
  if (internal_buffer == NULL) {
    return SL_STATUS_ALLOCATION_FAILED;
  }

  *buffer = internal_buffer->data;
  return SL_STATUS_OK;
}

sl_status_t sli_buffer_manager_free_buffer(sli_buffer_t buffer)
{
  SL_VERIFY_POINTER_OR_RETURN(buffer, SL_STATUS_NULL_POINTER);

  CORE_irqState_t state = CORE_EnterAtomic();

  sli_internal_buffer_t *internal_buffer = NULL;
  uint8_t *temp                          = NULL;

  // Decrement the pointer to get the reference to the internal buffer.
  temp            = (uint8_t *)buffer;
  internal_buffer = (sli_internal_buffer_t *)(temp - sizeof(sli_buffer_manager_mempool_handler_t *));

  sli_buffer_manager_mempool_handler_t *mempool_handler =
    (sli_buffer_manager_mempool_handler_t *)internal_buffer->buffer_manager_mempool_handler;
  sli_mem_pool_free(&mempool_handler->mempool, internal_buffer);
  mempool_handler->allocated_buffer_count--;

  if ((mempool_handler->is_common_pool) && (mempool_handler->allocated_buffer_count == 0)
      && (common_mempool_queue.size > SLI_MINIUM_ELEMENTS_IN_COMMON_MEMPOOL_QUEUE)) {
    sli_buffer_manager_free_a_common_mempool_from_queue(mempool_handler);
  }

  CORE_ExitAtomic(state);
  return SL_STATUS_OK;
}
