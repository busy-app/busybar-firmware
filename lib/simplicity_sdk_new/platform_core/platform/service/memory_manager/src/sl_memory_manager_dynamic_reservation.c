/***************************************************************************//**
 * @file
 * @brief Memory Manager Driver's Block Reservation Feature Implementation.
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

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdalign.h>

#include "sl_memory_manager_config.h"
#include "sl_memory_manager.h"
#include "sli_memory_manager.h"

#include "sl_assert.h"
#include "sl_bit.h"
#include "sl_common.h"
#include "sl_core.h"

#if defined(SL_COMPONENT_CATALOG_PRESENT)
#include "sl_component_catalog.h"
#endif

#if defined(SL_CATALOG_MEMORY_PROFILER_PRESENT)
#include "sli_memory_profiler.h"
#endif

/*******************************************************************************
 **************************   GLOBAL FUNCTIONS   *******************************
 ******************************************************************************/

/***************************************************************************//**
 * Dynamically reserves a block of memory.
 *
 * @note (1) This function assumes the 'handle' is provided by the caller:
 *           - either statically (e.g. as a global variable)
 *           - or dynamically by calling sl_memory_reservation_handle_alloc().
 ******************************************************************************/
sl_status_t sl_memory_reserve_block(size_t size,
                                    size_t align,
                                    sl_memory_reservation_t *handle,
                                    void **block)
{
  return sl_memory_heap_reserve_block(&sli_general_purpose_heap, size, align, handle, block);
}

/***************************************************************************//**
 * Frees a dynamically reserved block of memory.
 *
 * @note (1) Block reservation are not part of the double linked-list as they
 *           don't have metadata next to them compared to a LT/ST block that
 *           has a metadata with offset. When releasing a reserved block,
 *           the previous and next blocks that are part of the double linked-list
 *           need to be found to insert this new free block. The neighbours are
 *           found by browsing through the linked-list from the heap start until
 *           a block metadata's address is higher than the block being
 *           released's address. The previous and next blocks are then saved to
 *           properly update the double linked-list.
 ******************************************************************************/
sl_status_t sl_memory_release_block(sl_memory_reservation_t *handle)
{
  sl_memory_heap_t *heap;
  size_t new_free_block_length;
  sli_block_metadata_t *new_free_block = NULL;
  sli_block_metadata_t *prev_block = NULL;
  sli_block_metadata_t *next_block = NULL;
  sli_block_metadata_t *free_lt_list_head;
  sli_block_metadata_t *free_st_list_head;
  sli_block_metadata_t *current_metadata;
  uint16_t reserved_block_offset;

  // Verify that the handle isn't NULL.
  if (handle == NULL) {
    return SL_STATUS_NULL_POINTER;
  }

  // Retrieve the heap where the block was allocated.
  heap = sli_memory_get_heap_handle(handle->block_address);

  free_lt_list_head = (sli_block_metadata_t *)heap->free_lt_list_head;
  free_st_list_head = (sli_block_metadata_t *)heap->free_st_list_head;
  current_metadata = (sli_block_metadata_t *)heap->base_addr;

#if defined(SL_CATALOG_MEMORY_PROFILER_PRESENT)
  sli_memory_profiler_track_free(sli_mm_heap_name, handle->block_address);
#endif

  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_ATOMIC();

  // Find neighbours by searching from the heap start. See Note #1.
  while ((uintptr_t)current_metadata < (uintptr_t)handle->block_address) {
    prev_block = current_metadata;

    if (sli_block_offset_next_dword_decode(current_metadata) == 0) {
      break;
    }

    current_metadata = (sli_block_metadata_t *)((uint64_t *)current_metadata + sli_block_offset_next_dword_decode(current_metadata));
  }
  next_block = ((uintptr_t)current_metadata >= (uintptr_t)handle->block_address) ? current_metadata : NULL;

  new_free_block = (sli_block_metadata_t *)handle->block_address;
  new_free_block_length = (uint16_t)SLI_BLOCK_LEN_BYTE_TO_DWORD(handle->block_size) - SLI_BLOCK_METADATA_SIZE_DWORD;

  // Create a new free block while trying to merge it with the previous and next free blocks if possible.
  if (prev_block != NULL) {
    // Calculate offset between the reserved block and the previous block's payload address.
    reserved_block_offset = (uint16_t)((uint64_t *)handle->block_address - (uint64_t *)prev_block - SLI_BLOCK_METADATA_SIZE_DWORD);
    // Then calculate the difference between the above offset and the length of the previous block.
    reserved_block_offset -= sli_block_len_dword_decode(prev_block);

    // Make sure there's no reserved block between the freed block and the previous block.
    // Layout around the reserved block to free (aka R1) will be:
    // |...|Metadata Free block|Data Free block|R1||
    if ((prev_block->block_in_use == 0) && (reserved_block_offset < SLI_BLOCK_RESERVATION_MIN_SIZE_DWORD)) {
      // New freed block's previous block is free, so merge both free blocks.
      new_free_block = prev_block;
      prev_block = (prev_block == heap->base_addr)
                   ? NULL
                   : (sli_block_metadata_t *)((uint64_t *)prev_block - sli_block_offset_prev_dword_decode(prev_block));

      new_free_block_length += sli_block_len_dword_decode(new_free_block) + SLI_BLOCK_METADATA_SIZE_DWORD;
    } else {
      // Create a new free block, because previous block is a dynamic allocation, a reserved block or the start of the heap.
      // Layout around the reserved block to free (aka R1) will be:
      // |...|Metadata Free block|Data Free block|R2|R1|| or |...|Metadata ST1|Data ST1|R1|| or |...|Metadata LT|Data LT|R1||
      heap->free_blocks_number++;
      SLI_MEMORY_STAT_HEAP_INCREASE(heap, SLI_BLOCK_METADATA_SIZE_BYTE);
    }
  }

  if (next_block != NULL) {
    // Calculate offset between the reserved block and the next block.
    reserved_block_offset = (uint16_t)((uint64_t *)next_block - (uint64_t *)handle->block_address);
    // Then calculate the difference between the above offset and the size of the block being released.
    reserved_block_offset -= SLI_BLOCK_LEN_BYTE_TO_DWORD(handle->block_size);

    // Make sure there's no reserved block between the freed block and the next block.
    if ((next_block->block_in_use == 0) && (reserved_block_offset < SLI_BLOCK_RESERVATION_MIN_SIZE_DWORD)) {
      // New freed block's following block is free, so merge both free blocks.
      new_free_block_length += sli_block_len_dword_decode(next_block) + reserved_block_offset + SLI_BLOCK_METADATA_SIZE_DWORD;
      // Invalidate the next block metadata.
      sli_block_len_dword_encode(next_block, 0);
      // 2 free blocks have been merged, account for 1 free block only.
      heap->free_blocks_number--;
      SLI_MEMORY_STAT_HEAP_DECREASE(heap, SLI_BLOCK_METADATA_SIZE_BYTE);
      
      if (sli_block_offset_next_dword_decode(next_block) != 0) {
        // Get next block following current next block.
        next_block = (sli_block_metadata_t *)((uint64_t *)next_block + sli_block_offset_next_dword_decode(next_block));
      } else {
        next_block = NULL;
      }
    }
  }

  // Update the new free metadata block accordingly.
  sli_memory_metadata_init(new_free_block);
  sli_block_len_dword_encode(new_free_block, new_free_block_length);

  if (next_block != NULL) {
    sli_block_offset_next_dword_encode(new_free_block, ((uint64_t *)next_block - (uint64_t *)new_free_block));
    sli_block_offset_prev_dword_encode(next_block, sli_block_offset_next_dword_decode(new_free_block));
  } else {
    // Heap end.
    sli_block_offset_next_dword_encode(new_free_block, 0);
  }

  if (prev_block != NULL) {
    sli_block_offset_prev_dword_encode(new_free_block, ((uint64_t *)new_free_block - (uint64_t *)prev_block));
    sli_block_offset_next_dword_encode(prev_block, sli_block_offset_prev_dword_decode(new_free_block));
  } else {
    // Heap start.
    sli_block_offset_prev_dword_encode(new_free_block, 0);
  }

  if (free_lt_list_head == NULL             // LT list is empty. Freed block becomes the new 1st element.
      || free_lt_list_head > new_free_block // LT list not empty. Verify if freed block becomes the head.
      || sli_block_len_dword_decode(free_lt_list_head) == 0) {
    free_lt_list_head = new_free_block;
  }

  if (free_st_list_head == NULL             // ST list is empty. Freed block becomes the new 1st element.
      || free_st_list_head < new_free_block // ST list not empty. Verify if freed block becomes the head.
      || sli_block_len_dword_decode(free_st_list_head) == 0) {
    free_st_list_head = new_free_block;
  }
  // Update the heap's head pointers.
  heap->free_lt_list_head = (void *)free_lt_list_head;
  heap->free_st_list_head = (void *)free_st_list_head;

  // Decrease heap usage statistic.
  SLI_MEMORY_STAT_HEAP_DECREASE(heap, SLI_ALIGN_ROUND_UP(handle->block_size, SLI_BLOCK_ALLOC_MIN_ALIGN));

  // Invalidate handle.
  handle->block_address = NULL;
  handle->block_size = 0;

#if defined(SLI_MEMORY_MANAGER_ENABLE_TEST_UTILITIES)
  sli_memory_remove_reservation_handle(handle);
#endif

  CORE_EXIT_ATOMIC();

  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Dynamically allocates a block reservation handle.
 ******************************************************************************/
sl_status_t sl_memory_reservation_handle_alloc(sl_memory_reservation_t **handle)
{
#if defined(SL_CATALOG_MEMORY_PROFILER_PRESENT)
  void * volatile return_address = sli_memory_profiler_get_return_address();
#endif
  sl_status_t status;

  status = sl_memory_alloc(sizeof(sl_memory_reservation_t), BLOCK_TYPE_LONG_TERM, (void**)handle);
#if defined(SL_CATALOG_MEMORY_PROFILER_PRESENT)
  sli_memory_profiler_track_ownership(SLI_INVALID_MEMORY_TRACKER_HANDLE, *handle, return_address);
#endif
  if (status != SL_STATUS_OK) {
    return status;
  }

  // Initialize handle data.
  (*handle)->block_address = NULL;
  (*handle)->block_size = 0;

  return status;
}

/***************************************************************************//**
 * Frees a dynamically allocated block reservation handle.
 ******************************************************************************/
sl_status_t sl_memory_reservation_handle_free(sl_memory_reservation_t *handle)
{
  // Check that block has been released before freeing handle.
  if ((handle->block_size != 0) || (handle->block_address != NULL)) {
    return SL_STATUS_FAIL;
  }

  return sl_memory_free((void *)handle);
}

/***************************************************************************//**
 * Gets the size of the memory reservation handle structure.
 ******************************************************************************/
uint32_t sl_memory_reservation_handle_get_size(void)
{
  return sizeof(sl_memory_reservation_t);
}

/***************************************************************************//**
 * Dynamically reserves a block of memory from a specific heap instance.
 *
 * @note (1) This function assumes the 'handle' is provided by the caller:
 *           - either statically (e.g. as a global variable)
 *           - or dynamically by calling sl_memory_reservation_handle_alloc().
 ******************************************************************************/
sl_status_t sl_memory_heap_reserve_block(sl_memory_heap_t *heap,
                                         size_t size,
                                         size_t align,
                                         sl_memory_reservation_t *handle,
                                         void **block)
{
#if defined(SL_CATALOG_MEMORY_PROFILER_PRESENT)
  void * volatile return_address = sli_memory_profiler_get_return_address();
#endif

  // Check proper alignment characteristics.
  EFM_ASSERT((align == SL_MEMORY_BLOCK_ALIGN_DEFAULT)
             || (SL_MATH_IS_PWR2(align)
                 && (align <= SL_MEMORY_BLOCK_ALIGN_512_BYTES)));

  // Make sure the heap handle isn't NULL.
  EFM_ASSERT(heap != NULL);

  sli_block_metadata_t *free_block_metadata = NULL;
  void *reserved_blk = NULL;
  size_t current_block_len;
  size_t size_real;
  size_t size_adjusted;
  size_t block_size_remaining;
  size_t block_align = (align == SL_MEMORY_BLOCK_ALIGN_DEFAULT) ? SLI_BLOCK_ALLOC_MIN_ALIGN : align;
#if defined(DEBUG_EFM) || defined(DEBUG_EFM_USER)
  reserve_no_retention_first = false;
#endif

  // Verify that the handle pointer isn't NULL. See Note #1.
  if ((handle == NULL) || (block == NULL)) {
    return SL_STATUS_NULL_POINTER;
  }

  // Check that the block does not exist yet.
  if ((handle->block_size != 0) || (handle->block_address != NULL)) {
    return SL_STATUS_FAIL;
  }

  *block = NULL; // No block reserved yet.

  if ((size == 0) || (size >= heap->size)) {
    return SL_STATUS_INVALID_PARAMETER;
  }

#if defined(SLI_MEMORY_MANAGER_ENABLE_TEST_UTILITIES)
  // Check if the reservation counter has reached its maximum limit.
  if ((int)sli_get_available_reservation_handle_ix() == -1) {
    return SL_STATUS_ALLOCATION_FAILED;
  }
#endif

  size_real = SLI_ALIGN_ROUND_UP(size, block_align);
  // Round size to minimum reservation size if requested size is too small.
  size_real = size_real < SLI_BLOCK_RESERVATION_MIN_SIZE_BYTE ? SLI_BLOCK_RESERVATION_MIN_SIZE_BYTE : size_real;

  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_ATOMIC();

// For block reservations, the size_adjusted contains the metadata.
  size_adjusted = sli_memory_find_free_block(heap, size_real, block_align, BLOCK_TYPE_SHORT_TERM, true, &free_block_metadata);

  if ((free_block_metadata == NULL) || (size_adjusted == 0)) {
    CORE_EXIT_ATOMIC();
#if defined(SL_CATALOG_MEMORY_PROFILER_PRESENT)
    sli_memory_profiler_track_alloc_with_ownership(sli_mm_heap_name, NULL, size, return_address);
#endif
    return SL_STATUS_ALLOCATION_FAILED;
  }

  current_block_len = SLI_BLOCK_LEN_DWORD_TO_BYTE(sli_block_len_dword_decode(free_block_metadata));
  // SLI_BLOCK_METADATA_SIZE_BYTE is added to the free block length to get the real remaining size as size_adjusted contains the metadata size.
  block_size_remaining = (current_block_len + SLI_BLOCK_METADATA_SIZE_BYTE) - size_adjusted;

  heap->free_blocks_number--;

  // Split free and reserved blocks if possible.
  if (block_size_remaining >= SLI_BLOCK_RESERVATION_MIN_SIZE_BYTE) {
    size_t block_len_dw = sli_block_len_dword_decode(free_block_metadata);

    // Changes size of free block.
    sli_block_len_dword_encode(free_block_metadata, (block_len_dw - SLI_BLOCK_LEN_BYTE_TO_DWORD(size_adjusted)));

    // Create a new block = reserved block returned to requester. This new block is the nearest to the heap end.
    reserved_blk = (sli_block_metadata_t *)((uint8_t *)free_block_metadata + block_size_remaining);

    // Update block size.
    handle->block_size = size_adjusted;

    // Account for the split block that is free.
    heap->free_blocks_number++;
  } else {
    sli_block_metadata_t *neighbour_block = NULL;

    // Create a new block with size of the free block.
    reserved_blk = (sli_block_metadata_t *)((uint8_t *)free_block_metadata);
    // Update block size with the remaining size of the free block.
    handle->block_size = size_real + block_size_remaining;

    // Update next neighbour.
    if (sli_block_offset_next_dword_decode(free_block_metadata) != 0) {
      neighbour_block = (sli_block_metadata_t *)((uint64_t *)free_block_metadata + sli_block_offset_next_dword_decode(free_block_metadata));

      if (sli_block_offset_prev_dword_decode(free_block_metadata) != 0) {
        size_t block_offset_prev_dw = sli_block_offset_prev_dword_decode(neighbour_block) + sli_block_offset_prev_dword_decode(free_block_metadata);

        sli_block_offset_prev_dword_encode(neighbour_block, block_offset_prev_dw);
      } else {
        // Heap start.
        sli_block_offset_next_dword_encode(neighbour_block, 0);
      }
    }

    // Update previous neighbour.
    if (sli_block_offset_prev_dword_decode(free_block_metadata) != 0) {
      neighbour_block = (sli_block_metadata_t *)((uint64_t *)free_block_metadata - sli_block_offset_prev_dword_decode(free_block_metadata));

      if (sli_block_offset_next_dword_decode(free_block_metadata) != 0) {
        size_t block_offset_next_dw = sli_block_offset_next_dword_decode(neighbour_block) + sli_block_offset_next_dword_decode(free_block_metadata);
        sli_block_offset_next_dword_encode(neighbour_block, block_offset_next_dw);
      } else {
        // Heap end.
        sli_block_offset_next_dword_encode(neighbour_block, 0);
      }
    }

    SLI_MEMORY_STAT_HEAP_DECREASE(heap, SLI_BLOCK_METADATA_SIZE_BYTE);

    // Update head pointers accordingly.
    sli_update_free_list_heads(heap, neighbour_block, free_block_metadata, true);
  }

  handle->block_address = reserved_blk;
  *block = reserved_blk;

  // Heap usage size statistic.
  SLI_MEMORY_STAT_HEAP_INCREASE(heap, size_real);

#if defined(SLI_MEMORY_MANAGER_ENABLE_TEST_UTILITIES)
  // Save the reservation for heap integrity check purposes.
  sli_memory_save_reservation_handle(handle);
#endif

  CORE_EXIT_ATOMIC();

#if defined(SL_CATALOG_MEMORY_PROFILER_PRESENT)
  sli_memory_profiler_track_alloc(sli_mm_heap_name, handle->block_address, size_real);
  sli_memory_profiler_track_alloc_with_ownership(sli_mm_heap_reservation_name,
                                                 handle->block_address,
                                                 handle->block_size,
                                                 return_address);
#endif

  return SL_STATUS_OK;
}
