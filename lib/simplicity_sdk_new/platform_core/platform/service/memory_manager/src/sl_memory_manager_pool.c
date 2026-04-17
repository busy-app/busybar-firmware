/***************************************************************************//**
 * @file
 * @brief Memory Manager Driver's Memory Pool Lightweight Feature Implementation.
 *******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
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

#include "sl_memory_manager.h"
#include "sli_memory_manager.h"
#include "sl_memory_manager_config.h"

#include "sl_assert.h"
#include "sl_core.h"

#if defined(SL_COMPONENT_CATALOG_PRESENT)
#include "sl_component_catalog.h"
#endif

#if defined(SL_CATALOG_MEMORY_PROFILER_PRESENT)
#include "sli_memory_profiler.h"
#endif

/*******************************************************************************
 *********************************   DEFINES   *********************************
 ******************************************************************************/

#define SLI_MEM_POOL_OUT_OF_MEMORY     UINTPTR_MAX
#define SLI_MEM_POOL_REQUIRED_PADDING(obj_size) (((sizeof(size_t) - ((obj_size) % sizeof(size_t))) % sizeof(size_t)))

#if (defined(SL_MEMORY_MANAGER_POOL_DOUBLE_FREE_PROTECTION_ENABLE) && (SL_MEMORY_MANAGER_POOL_DOUBLE_FREE_PROTECTION_ENABLE == 1))
/***************************************************************************//**
 * Checks if a value looks like a valid free list pointer.
 * Used as a quick O(1) pre-check for double-free detection.
 *
 * A value looks like a free list pointer if it's either:
 * - The end-of-list marker (SLI_MEM_POOL_OUT_OF_MEMORY), OR
 * - A block-aligned address within the pool's memory range
 *
 * @param[in] pool_handle  Pointer to the memory pool handle.
 * @param[in] value        The value to check.
 *
 * @return true if value looks like it could be a free list pointer.
 ******************************************************************************/
static inline bool sli_looks_like_free_list_ptr(const sl_memory_pool_t *pool_handle,
                                                size_t value)
{
  // Check for end-of-list marker.
  if (value == SLI_MEM_POOL_OUT_OF_MEMORY) {
    return true;
  }

  // Check if it points within the pool's block range.
  size_t pool_start = (size_t)pool_handle->block_address;
  size_t pool_end = pool_start + (pool_handle->block_size * pool_handle->block_count);

  if ((value >= pool_start) && (value < pool_end)) {
    // Must be aligned to a block boundary.
    size_t offset = value - pool_start;
    return (offset % pool_handle->block_size) == 0;
  }

  return false;
}

/***************************************************************************//**
 * Checks if a block is in the free list by walking the list.
 * Used as O(n) verification when the quick check is suspicious.
 *
 * @param[in] pool_handle  Pointer to the memory pool handle.
 * @param[in] block        The block address to search for.
 *
 * @return true if block is found in the free list (already freed).
 *
 * @note Must be called within an atomic section.
 ******************************************************************************/
static bool sli_block_is_in_free_list(const sl_memory_pool_t *pool_handle,
                                      const void *block)
{
  void *current = pool_handle->block_free;

  while ((size_t)current != SLI_MEM_POOL_OUT_OF_MEMORY) {
    if (current == block) {
      return true;
    }
    // Follow the free list to the next block.
    current = (void *)*(size_t *)current;
  }

  return false;
}
#endif // (defined(SL_MEMORY_MANAGER_POOL_DOUBLE_FREE_PROTECTION_ENABLE) && (SL_MEMORY_MANAGER_POOL_DOUBLE_FREE_PROTECTION_ENABLE == 1))
/***************************************************************************//**
 * Creates a memory pool.
 ******************************************************************************/
sl_status_t sl_memory_create_pool(size_t block_size,
                                  uint32_t block_count,
                                  sl_memory_pool_t *pool_handle)
{
  return sl_memory_heap_create_pool(&sli_general_purpose_heap, block_size, block_count, pool_handle);
}

/***************************************************************************//**
 * Deletes a memory pool.
 *
 * @note The pool_handle provided is neither freed or invalidated. It can be
 *       reused in a new call to sl_memory_create_pool() to create another pool.
 ******************************************************************************/
sl_status_t sl_memory_delete_pool(sl_memory_pool_t *pool_handle)
{
  uint32_t free_block_count;
  sl_status_t status;

  if (pool_handle == NULL) {
    return SL_STATUS_NULL_POINTER;
  }

  // Verify that the pool was properly initialized.
  if (pool_handle->block_address == NULL) {
    return SL_STATUS_NULL_POINTER;
  }

  // Verify that no blocks are allocated.
  free_block_count = sl_memory_pool_get_free_block_count(pool_handle);
  if (free_block_count != pool_handle->block_count) {
    return SL_STATUS_INVALID_STATE;
  }

#if defined(SL_CATALOG_MEMORY_PROFILER_PRESENT)
  // Delete the memory tracker
  sli_memory_profiler_delete_tracker(pool_handle);
#endif

  // Free block.
  status = sl_memory_free(pool_handle->block_address);

  return status;
}

/***************************************************************************//**
 * Allocates a block from a memory pool.
 ******************************************************************************/
sl_status_t sl_memory_pool_alloc(sl_memory_pool_t *pool_handle,
                                 void **block)
{
#if defined(SL_CATALOG_MEMORY_PROFILER_PRESENT)
  void * volatile return_address = sli_memory_profiler_get_return_address();
#endif
  CORE_DECLARE_IRQ_STATE;

  if ((pool_handle == NULL) || (block == NULL)) {
    return SL_STATUS_NULL_POINTER;
  }

  // No block allocated yet.
  *block = NULL;

  CORE_ENTER_ATOMIC();

  if ((size_t)pool_handle->block_free == SLI_MEM_POOL_OUT_OF_MEMORY) {
    CORE_EXIT_ATOMIC();
#if defined(SL_CATALOG_MEMORY_PROFILER_PRESENT)
    sli_memory_profiler_track_alloc_with_ownership(pool_handle, NULL, pool_handle->block_size, return_address);
#endif
    return SL_STATUS_EMPTY;
  }

  // Get the next free block.
  void *block_addr = pool_handle->block_free;

  // Update the next free block using the address saved in that block.
  pool_handle->block_free = (void *)*(size_t *)block_addr;

#if defined(MEMORY_MANAGER_TEST_CONDITIONS)
  // Clear the first word of the allocated block so it doesn't look like
  // a free list pointer. This enables O(1) double-free detection in most cases.
  // Without this, unwritten blocks would trigger the O(n) verification path
  // because they still contain the old "next" pointer from the free list.
  // This is only done in test conditions to avoid performance impact.
  *(size_t *)block_addr = 0;
#endif

  CORE_EXIT_ATOMIC();

#if defined(SL_CATALOG_MEMORY_PROFILER_PRESENT)
  sli_memory_profiler_track_alloc_with_ownership(pool_handle, block_addr, pool_handle->block_size, return_address);
#endif

  *block = block_addr;

  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Frees a block from a memory pool.
 ******************************************************************************/
sl_status_t sl_memory_pool_free(sl_memory_pool_t *pool_handle,
                                void *block)
{
  CORE_DECLARE_IRQ_STATE;

  if ((pool_handle == NULL) || (block == NULL)) {
    return SL_STATUS_NULL_POINTER;
  }

  // Validate that the provided address is in the pool payload range.
  if ((block < pool_handle->block_address)
      || ((size_t)block >= ((size_t)pool_handle->block_address + (pool_handle->block_size * pool_handle->block_count)))) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  CORE_ENTER_ATOMIC();

#if (defined(SL_MEMORY_MANAGER_POOL_DOUBLE_FREE_PROTECTION_ENABLE) && (SL_MEMORY_MANAGER_POOL_DOUBLE_FREE_PROTECTION_ENABLE == 1))
  // Double-free detection using hybrid approach:
  //
  // O(1) check: does block content look like a free list pointer?
  // After allocation, sl_memory_pool_alloc() clears the first word to 0.
  // After a legitimate free, the block contains a pointer to the next free block
  // (a valid block address) or the end marker. If the block was allocated and the
  // user didn't overwrite the first word with a valid block address (rare), the
  // content will be 0 and this check will pass quickly.
  size_t stored_value = *(size_t *)block;
  if (sli_looks_like_free_list_ptr(pool_handle, stored_value)) {
    // Content looks suspicious - verify by walking the free list.
    if (sli_block_is_in_free_list(pool_handle, block)) {
      // Block is actually in the free list - this is a double-free.
      CORE_EXIT_ATOMIC();
      return SL_STATUS_INVALID_PARAMETER;
    }
    // False positive: user data happened to look like a free list pointer.
    // Proceed with normal free.
  }
#endif // (defined(SL_MEMORY_MANAGER_POOL_DOUBLE_FREE_PROTECTION_ENABLE) && (SL_MEMORY_MANAGER_POOL_DOUBLE_FREE_PROTECTION_ENABLE == 1))

#if defined(SL_CATALOG_MEMORY_PROFILER_PRESENT)
  sli_memory_profiler_track_free(pool_handle, block);
#endif

  // Save the current free block address in this block and update free list head.
  *(size_t *)block = (size_t)pool_handle->block_free;
  pool_handle->block_free = block;

  CORE_EXIT_ATOMIC();

  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Gets the count of free blocks in a memory pool.
 ******************************************************************************/
uint32_t sl_memory_pool_get_free_block_count(const sl_memory_pool_t *pool_handle)
{
  uint32_t free_block_count = 0;
  void *free_block;

  if (pool_handle == NULL) {
    return 0;
  }

  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_ATOMIC();

  free_block = pool_handle->block_free;

  // Go through the free block list and count the number of free blocks remaining.
  while ((size_t)free_block != SLI_MEM_POOL_OUT_OF_MEMORY) {
    free_block = (void *)*(size_t *)free_block;
    free_block_count++;
  }

  CORE_EXIT_ATOMIC();

  return free_block_count;
}

/***************************************************************************//**
 * Creates a memory pool from a specific heap instance.
 ******************************************************************************/
sl_status_t sl_memory_heap_create_pool(sl_memory_heap_t *heap,
                                       size_t block_size,
                                       uint32_t block_count,
                                       sl_memory_pool_t *pool_handle)
{
#if defined(SL_CATALOG_MEMORY_PROFILER_PRESENT)
  void * volatile return_address = sli_memory_profiler_get_return_address();
#endif
  sl_status_t status = SL_STATUS_OK;
  uint8_t *block = NULL;
  size_t block_addr;
  size_t pool_size;

  // Make sure the heap handle isn't NULL.
  EFM_ASSERT(heap != NULL);

  EFM_ASSERT(block_count > 0);
  EFM_ASSERT(block_size > 0);

  if (pool_handle == NULL) {
    return SL_STATUS_NULL_POINTER;
  }

  // SLI_MEM_POOL_REQUIRED_PADDING Rounds up to the nearest platform-dependant size. On a 32-bit processor,
  // it will be rounded-up to 4 bytes. E.g. 101 bytes will be rounded up to 104 bytes.
  pool_handle->block_size = block_size + (uint16_t)SLI_MEM_POOL_REQUIRED_PADDING(block_size);
  pool_handle->block_count = block_count;

  // Reserve a block in which the entire pool will reside. Uses a long term allocation to keep
  // behavior similar to dynamic reservation.
  pool_size = pool_handle->block_size * pool_handle->block_count;
  status = sl_memory_heap_alloc(heap, pool_size, BLOCK_TYPE_LONG_TERM, (void **)&block);

#if defined(SL_CATALOG_MEMORY_PROFILER_PRESENT)
  sli_memory_profiler_track_ownership(SLI_INVALID_MEMORY_TRACKER_HANDLE, block, return_address);
#endif

  if (status != SL_STATUS_OK) {
    return status;
  }

#if defined(SL_CATALOG_MEMORY_PROFILER_PRESENT)
  // Create the tracker for the pool with no description. The code that created
  // the pool can add the tracker description if relevant.
  sli_memory_profiler_create_pool_tracker(pool_handle, NULL, block, pool_size);
#endif

  pool_handle->block_address = (void *)block;

  // Returned block pointer not used because its reference is already stored in block_address.
  (void)&block;

  pool_handle->block_free = (uint32_t *)pool_handle->block_address;

  block_addr = (size_t)pool_handle->block_address;

  // Populate the list of free blocks except the last block.
  for (uint16_t i = 0; i < (block_count - 1); i++) {
    *(size_t *)block_addr = block_addr + pool_handle->block_size;
    block_addr += pool_handle->block_size;
  }

  // Last element will indicate out of memory.
  *(size_t *)block_addr = SLI_MEM_POOL_OUT_OF_MEMORY;

  return status;
}
