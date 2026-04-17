/***************************************************************************//**
 * @file
 * @brief Memory Manager Driver Implementation.
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

#include "sl_memory_manager_region_config.h"
#include "sl_memory_manager_config.h"
#include "sl_memory_manager.h"
#include "sli_memory_manager.h"
#include "sli_code_classification.h"
#include "sl_assert.h"
#include "sl_bit.h"
#include "sl_common.h"

#if defined(SL_COMPONENT_CATALOG_PRESENT)
#include "sl_component_catalog.h"
#endif

#if defined(SL_CATALOG_BANK_RETENTION_CONTROL_PRESENT) ||  \
    defined(SL_CATALOG_BANK_RETENTION_CONTROL_STUBBED_PRESENT)
#include "sli_memory_manager_retention_control.h"
#endif

/*******************************************************************************
 **********************************   EXTERN   *********************************
 ******************************************************************************/

extern __INLINE void sli_block_len_dword_encode(sli_block_metadata_t *meta,
                                                uint32_t len);
extern __INLINE void sli_block_offset_prev_dword_encode(sli_block_metadata_t *meta,
                                                        uint32_t len);
extern __INLINE void sli_block_offset_next_dword_encode(sli_block_metadata_t *meta,
                                                        uint32_t len);
extern __INLINE uint32_t sli_block_offset_prev_dword_decode(const sli_block_metadata_t *meta);
extern __INLINE uint32_t sli_block_len_dword_decode(const sli_block_metadata_t *meta);
extern __INLINE uint32_t sli_block_offset_next_dword_decode(const sli_block_metadata_t *meta);

/*******************************************************************************
 ***************************  LOCAL VARIABLES   ********************************
 ******************************************************************************/

#if defined(SLI_MEMORY_MANAGER_ENABLE_TEST_UTILITIES)
// Dynamic reservation bookkeeping.
sl_memory_reservation_t *sli_reservation_handle_ptr_table[SLI_MAX_RESERVATION_COUNT];

// Reservation no retention bookkeeping.
sl_memory_reservation_t sli_reservation_no_retention_table[SLI_MAX_RESERVATION_COUNT] = { 0 };
#endif

/*******************************************************************************
 ***************************   LOCAL FUNCTIONS   *******************************
 ******************************************************************************/

#if defined(SLI_MEMORY_MANAGER_ENABLE_TEST_UTILITIES)
/***************************************************************************//**
 * Gets the index in sli_reservation_handle_ptr_table[] by block address.
 *
 * @param[in]  addr  Pointer to block reservation.
 *
 * @return    Corresponding index in sli_reservation_handle_ptr_table.
 ******************************************************************************/
static uint32_t get_reservation_ix_by_addr(void *addr)
{
  for (uint32_t ix = 0; ix < SLI_MAX_RESERVATION_COUNT; ix++) {
    if (sli_reservation_handle_ptr_table[ix] == NULL) {
      continue;
    }
    if (sli_reservation_handle_ptr_table[ix]->block_address == addr) {
      return ix;
    }
  }
  return -1;
}

/***************************************************************************//**
 * Gets the index in sli_reservation_handle_ptr_table[]
 * by reservation handle pointer.
 *
 * @param[in]  reservation_handle_ptr  Pointer to reservation handle.
 *
 * @return    Corresponding index in sli_reservation_handle_ptr_table.
 ******************************************************************************/
static uint32_t get_reservation_ix_by_handle(sl_memory_reservation_t *reservation_handle_ptr)
{
  for (uint32_t ix = 0; ix < SLI_MAX_RESERVATION_COUNT; ix++) {
    if (sli_reservation_handle_ptr_table[ix] == NULL) {
      continue;
    }
    if (sli_reservation_handle_ptr_table[ix] == reservation_handle_ptr) {
      return ix;
    }
  }
  return -1;
}

/***************************************************************************//**
 * Get an index of sli_reservation_handle_ptr_table that is free.
 *
 * @return    Index of an empty entry in sli_reservation_handle_ptr_table.
 ******************************************************************************/
uint32_t sli_get_available_reservation_handle_ix(void)
{
  for (uint32_t ix = 0; ix < SLI_MAX_RESERVATION_COUNT; ix++) {
    if (sli_reservation_handle_ptr_table[ix] == NULL) {
      return ix;
    }
  }
  return -1;
}

/***************************************************************************//**
 * Gets the index in sli_reservation_no_retention_table[] by block address.
 *
 * @param[in]  addr  Pointer to block reservation.
 *
 * @return    Corresponding index in sli_reservation_no_retention_table[].
 ******************************************************************************/
static uint32_t get_reservation_no_retention_ix(void *addr)
{
  for (uint32_t ix = 0; ix < SLI_MAX_RESERVATION_COUNT; ix++) {
    if (sli_reservation_no_retention_table[ix].block_address == NULL) {
      continue;
    }
    if (sli_reservation_no_retention_table[ix].block_address == addr) {
      return ix;
    }
  }
  return -1;
}

/***************************************************************************//**
 * Gets an index of sli_reservation_no_retention_table[] that is free.
 *
 * @return    Index of an empty entry in sli_reservation_no_retention_table[].
 ******************************************************************************/
static uint32_t get_available_reservation_no_retention_ix(void)
{
  for (uint32_t ix = 0; ix < SLI_MAX_RESERVATION_COUNT; ix++) {
    if (sli_reservation_no_retention_table[ix].block_address == NULL) {
      return ix;
    }
  }
  return -1;
}
#endif

/***************************************************************************//**
 * Initializes a memory block metadata to some reset values.
 ******************************************************************************/
void sli_memory_metadata_init(sli_block_metadata_t *block_metadata)
{
  memset(block_metadata, 0, SLI_BLOCK_METADATA_SIZE_BYTE);
}

/***************************************************************************//**
 * Gets pointer pointing to the first free block of adequate size.
 *
 * @note (1) For a block reservation, there's no metadata next to the
 *           reserved block. For this reason, when looking for a free block
 *           large enough to fit a new reserved block, the size of the metadata
 *           is counted in the available size of the free blocks.
 *
 * @note (2) For a short-term block, if the required data alignment is greater
 *           than 8 bytes, the found block size must account for the correct
 *           alignment of the block data payload. A series of computations is
 *           done starting from the end of the found block to determine the
 *           best data offset needed to align the data payload. The worst
 *           alignment (size_real + block_align) cannot be taken by default
 *           as it may imply loosing too many bytes in internal fragmentation
 *           due to the alignment requirement.
 ******************************************************************************/
size_t sli_memory_find_free_block(sl_memory_heap_t *heap,
                                  size_t size,
                                  size_t align,
                                  sl_memory_block_type_t type,
                                  bool block_reservation,
                                  sli_block_metadata_t **block)
{
  sli_block_metadata_t *current_block_metadata = NULL;
  sli_block_metadata_t *free_lt_list_head = (sli_block_metadata_t *)heap->free_lt_list_head;
  sli_block_metadata_t *free_st_list_head = (sli_block_metadata_t *)heap->free_st_list_head;
  void *data_payload = NULL;
  size_t size_adjusted = 0;
  size_t current_block_len;
  size_t block_align = (align == SL_MEMORY_BLOCK_ALIGN_DEFAULT) ? SLI_BLOCK_ALLOC_MIN_ALIGN : align;
  size_t data_payload_offset;
  bool is_aligned = false;

  *block = NULL;

  current_block_metadata = (type == BLOCK_TYPE_LONG_TERM) ? free_lt_list_head : free_st_list_head;
  if (current_block_metadata == NULL) {
    return 0;
  }

  current_block_len = SLI_BLOCK_LEN_DWORD_TO_BYTE(sli_block_len_dword_decode(current_block_metadata));

  // For a block reservation, add the metadata's size to the free blocks' available memory space. See Note #2.
  current_block_len += block_reservation ? SLI_BLOCK_METADATA_SIZE_BYTE : 0;

  // Try to find a block to allocate (first-fit).
  while (current_block_metadata != NULL) {
    if ((!current_block_metadata->block_in_use) && (current_block_len >= size)) {
      if (type == BLOCK_TYPE_LONG_TERM) {
        // Check alignment requested and ensure size of found block can accommodate worst case alignment.
        // For LT, alignment requirement can be verified here whether the block is split or not.
        data_payload = (void *)((uint8_t *)current_block_metadata + SLI_BLOCK_METADATA_SIZE_BYTE);
        is_aligned = SLI_ADDR_IS_ALIGNED(data_payload, block_align);
        data_payload_offset = (uintptr_t)data_payload % block_align;

        if (is_aligned || (current_block_len >= (size + data_payload_offset))) {
          // Compute remaining block size given an alignment handling or not.
          size_adjusted = is_aligned ? size : (size + data_payload_offset);
          break;
        }
      } else {
        if (block_align == SLI_BLOCK_ALLOC_MIN_ALIGN) {
          // If alignment is 8 bytes (default min alignment), take the requested adjusted size.
          size_adjusted = size;
        } else {
          // If non 8-byte alignment, search the more optimized size accounting for the required alignment. See Note #3.
          uint8_t *block_end = (uint8_t *)((uint64_t *)current_block_metadata + SLI_BLOCK_METADATA_SIZE_DWORD + sli_block_len_dword_decode(current_block_metadata));

          data_payload = (void *)(block_end - size);
          data_payload = (void *)SLI_ALIGN_ROUND_DOWN(((uintptr_t)data_payload), block_align);
          size_adjusted = (size_t)(block_end - (uint8_t *)data_payload);
        }

        if (current_block_len >= size_adjusted) {
          break;
        }
      }
    }

    // Get next block.
    if (type == BLOCK_TYPE_LONG_TERM) {
      if (sli_block_offset_next_dword_decode(current_block_metadata) == 0) {
        return 0;  // End of heap. No block found.
      }
      // Long-term browsing direction goes from start to end of heap.
      current_block_metadata = (sli_block_metadata_t *)((uint64_t *)current_block_metadata + sli_block_offset_next_dword_decode(current_block_metadata));
    } else {
      if (sli_block_offset_prev_dword_decode(current_block_metadata) == 0) {
        return 0;  // Start of heap. No block found.
      }
      // Short-term browsing direction goes from end to start of heap.
      current_block_metadata = (sli_block_metadata_t *)((uint64_t *)current_block_metadata - sli_block_offset_prev_dword_decode(current_block_metadata));
    }

    current_block_len = SLI_BLOCK_LEN_DWORD_TO_BYTE(sli_block_len_dword_decode(current_block_metadata));
    current_block_len += block_reservation ? SLI_BLOCK_METADATA_SIZE_BYTE : 0;
  }

  *block = current_block_metadata;
  return size_adjusted;
}

/***************************************************************************//**
 * Finds the next free block that will become the long-term or short-term head
 * pointer.
 ******************************************************************************/
sli_block_metadata_t *sli_memory_find_head_free_block(sl_memory_heap_t *heap,
                                                      sl_memory_block_type_t type,
                                                      sli_block_metadata_t *block_start_from)
{
  sli_block_metadata_t *current_block_metadata = NULL;
  sli_block_metadata_t *free_block_metadata = NULL;
  bool search = true;

  if (heap->free_blocks_number == 0) {
    // No more free blocks.
    return NULL;
  }

  if (block_start_from != NULL) {
    // Start searching from the given block.
    current_block_metadata = block_start_from;
  } else {
    // Start searching from heap start (long-term [LT]) or near heap end (short-term [ST]).
    // For ST, searching cannot start at the absolute heap end. So the ST head pointer is used as it points
    // to the last free block closest to the heap end.
    current_block_metadata = (type == BLOCK_TYPE_LONG_TERM) ? (sli_block_metadata_t *)heap->base_addr : (sli_block_metadata_t *)heap->free_st_list_head;
  }
  // Make sure the block isn't NULL to prevent dereferencing a NULL pointer.
  EFM_ASSERT(current_block_metadata != NULL);

  // Long-term block: find the first free block closest to the heap start.
  // Short-term block: find the first free block closest to the heap end.
  do {
    if (current_block_metadata->block_in_use == 0) {
      free_block_metadata = current_block_metadata;
      search = false;
    } else if ((type == BLOCK_TYPE_LONG_TERM) && (sli_block_offset_next_dword_decode(current_block_metadata) != 0)) {
      current_block_metadata = (sli_block_metadata_t *)((uint64_t *)current_block_metadata + sli_block_offset_next_dword_decode(current_block_metadata));
    } else if ((type == BLOCK_TYPE_SHORT_TERM) && (sli_block_offset_prev_dword_decode(current_block_metadata) != 0)) {
      current_block_metadata = (sli_block_metadata_t *)((uint64_t *)current_block_metadata - sli_block_offset_prev_dword_decode(current_block_metadata));
    } else {
      free_block_metadata = NULL;
      break;
    }
  } while (search);

  return free_block_metadata;
}

/***************************************************************************//**
 * Gets long-term head pointer pointing to the first free block.
 ******************************************************************************/
void *sli_memory_get_longterm_head_ptr(void)
{
  return sli_general_purpose_heap.free_lt_list_head;
}

/***************************************************************************//**
 * Gets short-term head pointer pointing to the first free block.
 ******************************************************************************/
void *sli_memory_get_shortterm_head_ptr(void)
{
  return sli_general_purpose_heap.free_st_list_head;
}

/***************************************************************************//**
 * Update free lists heads (short and long terms).
 ******************************************************************************/
void sli_update_free_list_heads(sl_memory_heap_t *heap,
                                sli_block_metadata_t *free_head,
                                const sli_block_metadata_t *condition_block,
                                bool search)
{
  sli_block_metadata_t *free_lt_list_head = (sli_block_metadata_t *)heap->free_lt_list_head;
  sli_block_metadata_t *free_st_list_head = (sli_block_metadata_t *)heap->free_st_list_head;

  if (search) {
    if ((free_lt_list_head == condition_block) || (condition_block == NULL)) {
      free_lt_list_head = sli_memory_find_head_free_block(heap, BLOCK_TYPE_LONG_TERM, free_head);
    }
    if ((free_st_list_head == condition_block) || (condition_block == NULL)) {
      free_st_list_head = sli_memory_find_head_free_block(heap, BLOCK_TYPE_SHORT_TERM, free_head);
    }
  } else {
    if ((free_lt_list_head == condition_block) || (free_head < free_lt_list_head)) {
      free_lt_list_head = free_head;
    }
    if ((free_st_list_head == condition_block) || (free_head > free_st_list_head)) {
      free_st_list_head = free_head;
    }
  }

  heap->free_lt_list_head = (void *)free_lt_list_head;
  heap->free_st_list_head = (void *)free_st_list_head;
}

/***************************************************************************//**
 * Creates a new heap instance.
 *
 * @note The heap is linked to the general-purpose heap if it has been initialized.
 *       If the general-purpose heap is not initialized, the new heap stands alone.
 *       The general-purpose heap should always be initialized first.
 ******************************************************************************/
sl_status_t sli_memory_create_heap(void *base_addr,
                                   size_t size,
                                   sl_memory_block_attrib_t attrib,
                                   sl_memory_heap_t *heap)
{
  // Verify parameters.
  EFM_ASSERT((base_addr != NULL)
             && (size > 0)
             && (attrib <= SL_MEMORY_HEAP_ALLOC_EXTERNAL_RAM)
             && (heap != NULL));

#if defined(SLI_MEMORY_MANAGER_SUPPORT_ALLOCATION_FALLBACK)
  // Make sure either the general purpose heap is initialized or that it's
  // the heap being initialized.
  EFM_ASSERT((sli_general_purpose_heap.base_addr != NULL)
             || (attrib == SL_MEMORY_HEAP_ALLOC_GENERAL_RAM));

  // Only link to general purpose heap if it has been initialized (base_addr != NULL).
  if (sli_general_purpose_heap.base_addr != NULL) {
    // Find the last initialized heap.
    sl_memory_heap_t *neighbor_heap = &sli_general_purpose_heap;

    while ((neighbor_heap->next_handle != NULL) && (neighbor_heap != heap)) {
      neighbor_heap = neighbor_heap->next_handle;
    }

    // Link the new heap to the end of the heap list.
    // Do not link if the new heap is itself.
    if (neighbor_heap != heap) {
      neighbor_heap->next_handle = heap;
    }
  }
#endif

  // Initialize the heap structure.
  heap->base_addr = base_addr;
  heap->size = size;
  heap->used_size = 0;
  heap->high_watermark = 0;
  heap->free_blocks_number = 0;
  heap->attrib = attrib;
  heap->next_handle = NULL;

  // At first, all the heap is available to long-term/short-term blocks.
  heap->free_lt_list_head = base_addr;
  heap->free_st_list_head = base_addr;

  // First free block is entire heap size, minus one metadata block area.
  // Long-term and short-term list initialized with the same free block.
  sli_block_metadata_t *free_lt_list_head = (sli_block_metadata_t *)heap->free_lt_list_head;
  sli_memory_metadata_init(free_lt_list_head);
  sli_block_len_dword_encode(free_lt_list_head, (SLI_BLOCK_LEN_BYTE_TO_DWORD(size - SLI_BLOCK_METADATA_SIZE_BYTE)));
  heap->free_blocks_number++;

#if defined(SL_CATALOG_BANK_RETENTION_CONTROL_PRESENT) ||  \
    defined(SL_CATALOG_BANK_RETENTION_CONTROL_STUBBED_PRESENT)
  sli_memory_manager_hal_init(heap);
#endif

  INCREMENT_BANK_COUNTER(heap, base_addr, (void *)((uint8_t *)base_addr + SLI_BLOCK_METADATA_SIZE_BYTE - 1));

#if defined(SL_MEMORY_MANAGER_STATISTICS_API_ENABLE) && (SL_MEMORY_MANAGER_STATISTICS_API_ENABLE == 1)
  // Add first free block metadata to heap usage.
  SLI_MEMORY_STAT_HEAP_INCREASE(heap, SLI_BLOCK_METADATA_SIZE_BYTE);
#endif

  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Get the heap in which a block is allocated.
 ******************************************************************************/
SL_CODE_CLASSIFY(SL_CODE_COMPONENT_MEMORY_MANAGER, SL_CODE_CLASS_TIME_CRITICAL)
sl_memory_heap_t *sli_memory_get_heap_handle(const void *block)
{
  (void)block;

  if ((block >= sli_general_purpose_heap.base_addr) && (block < (void *)((uint8_t *)sli_general_purpose_heap.base_addr + sli_general_purpose_heap.size))) {
    return &sli_general_purpose_heap;
  }
#if defined(SL_CATALOG_MEMORY_MANAGER_PSRAM_PRESENT)
  if ((block >= sli_psram_heap.base_addr) && (block < (void *)((uint8_t *)sli_psram_heap.base_addr + sli_psram_heap.size))) {
    return &sli_psram_heap;
  }
#endif
#if defined(SL_CATALOG_MEMORY_MANAGER_DTCM_PRESENT)
  if ((block >= sli_dtcm_heap.base_addr) && (block < (void *)((uint8_t *)sli_dtcm_heap.base_addr + sli_dtcm_heap.size))) {
    return &sli_dtcm_heap;
  }
#endif

  return NULL;
}

/***************************************************************************//**
 * Creates the Stack at the end of the Heap.
 ******************************************************************************/
void sli_memory_create_stack(void)
{
  void *stack;

  // Reserve the Stack at the end of the general-purpose heap.
  EFM_ASSERT(sl_memory_reserve_no_retention(SL_STACK_SIZE, SL_MEMORY_BLOCK_ALIGN_DEFAULT, &stack) == SL_STATUS_OK);

#if defined(DEBUG_EFM) || defined(DEBUG_EFM_USER)
  // Verify that the reserved block matches the Stack region.
  sl_memory_region_t region = sl_memory_get_stack_region();
  EFM_ASSERT((uint32_t)stack == (uint32_t)region.addr);
#else
  (void)stack;
#endif
}

#if defined(SLI_MEMORY_MANAGER_ENABLE_TEST_UTILITIES)
/***************************************************************************//**
 * Gets the pointer to sl_memory_reservation_t{} by block address.
 ******************************************************************************/
sl_memory_reservation_t *sli_memory_get_reservation_handle_by_addr(void *addr)
{
  uint32_t reservation_ix = get_reservation_ix_by_addr(addr);

  if (reservation_ix != (uint32_t)-1) {
    return sli_reservation_handle_ptr_table[reservation_ix];
  }

  return NULL;
}

/***************************************************************************//**
 * Gets the size of a reservation by block address.
 ******************************************************************************/
uint32_t sli_memory_get_reservation_size_by_addr(void *addr)
{
  sl_memory_reservation_t *reservation_handle_ptr = sli_memory_get_reservation_handle_by_addr(addr);

  if (reservation_handle_ptr != NULL) {
    return reservation_handle_ptr->block_size;
  }
  // Not a reservation, return 0 size.
  return 0;
}

/***************************************************************************//**
 * Bookkeeps a reservation for profiling purposes.
 ******************************************************************************/
sl_status_t sli_memory_save_reservation_handle(sl_memory_reservation_t *reservation_handle_ptr)
{
  uint32_t reservation_ix = sli_get_available_reservation_handle_ix();

  if (reservation_ix != (uint32_t)-1) {
    sli_reservation_handle_ptr_table[reservation_ix] = reservation_handle_ptr;
    return SL_STATUS_OK;
  } else {
    return SL_STATUS_FULL;
  }
}

/***************************************************************************//**
 * Removes a reservation from records.
 ******************************************************************************/
sl_status_t sli_memory_remove_reservation_handle(sl_memory_reservation_t *reservation_handle_ptr)
{
  uint32_t reservation_ix = get_reservation_ix_by_handle(reservation_handle_ptr);

  if (reservation_ix != (uint32_t)-1) {
    sli_reservation_handle_ptr_table[reservation_ix] = NULL;
    return SL_STATUS_OK;
  } else {
    return SL_STATUS_NOT_FOUND;
  }
}

/***************************************************************************//**
 * Bookkeeps a reservation (no retention) for profiling purposes.
 ******************************************************************************/
sl_status_t sli_memory_save_reservation_no_retention(void *block_address,
                                                     uint32_t block_size)
{
  uint32_t reservation_ix = get_available_reservation_no_retention_ix();

  if (reservation_ix != (uint32_t)-1) {
    sli_reservation_no_retention_table[reservation_ix].block_address = block_address;
    sli_reservation_no_retention_table[reservation_ix].block_size = block_size;

    return SL_STATUS_OK;
  } else {
    return SL_STATUS_FULL;
  }
}

/***************************************************************************//**
 * Gets the size of a reservation (no retention) by block address.
 ******************************************************************************/
uint32_t sli_memory_get_reservation_no_retention_size(void *addr)
{
  uint32_t reservation_ix = get_reservation_no_retention_ix(addr);

  if (reservation_ix != (uint32_t)-1) {
    return sli_reservation_no_retention_table[reservation_ix].block_size;
  }
  // Not a reservation (no retention), return 0 size.
  return 0;
}

/***************************************************************************//**
 * Does a heap integrity check forwards from the general-purpose heap free_lt_list_head
 * and return the pointer to the corrupted sli_block_metadata_t{} (if applicable).
 * This could go past reservations so there are checks.
 *
 * @return    Pointer to the corrupted sli_block_metadata_t{}.
 ******************************************************************************/
sli_block_metadata_t *sli_memory_check_general_purpose_heap_integrity_forwards(void)
{
  return sli_memory_check_heap_integrity_forwards(&sli_general_purpose_heap);
}

/***************************************************************************//**
 * Does a heap integrity check forwards from the general-purpose heap and return
 * the pointer to the corrupted sli_block_metadata_t{} (if applicable).
 * This could go past reservations so there are checks.
 *
 * @note: Here are a few typical heap layout patterns that can be checked:
 *
 * Meta(LT1)|Payload(LT1)|Meta(LT2)|Payload(LT2)|Meta(free1)|Payload(free1)||
 * Meta(LT1)|Payload(LT1)|Meta(free1)|Payload(free1)|Meta(ST1)|Payload(ST1)||
 * Meta(RH1)|Payload(RH1)|Meta(free1)|Payload(free1)|Payload(R1)||
 * Meta(RH1)|Payload(RH1)|Meta(free1)|Payload(free1)|Payload(R1)|Payload(RNR1)||
 * Meta(RH1)|Payload(RH1)|Meta(LT1)|Payload(LT1)|Meta(free1)|Payload(free1)|Payload(R1)|Payload(RNR1)||
 * Meta(RH1)|Payload(RH1)|Meta(LT1)|Payload(LT1)|Meta(free1)|Payload(free1)|Meta(ST1)|Payload(ST1)|Payload(R1)|Payload(RNR1)||
 *
 * Legend of heap layouts:
 *    LT1 = allocated long-term block, ST1 = allocated short-term block
 *    RH1 = reservation handle, R1 = reserved block
 *    RNR1 = reserved block with no retention
 *    free1 = free block
 *    || = real end of heap
 ******************************************************************************/
sli_block_metadata_t *sli_memory_check_heap_integrity_forwards(sl_memory_heap_t *heap)
{
  uint64_t *heap_end_by_metadata = NULL;
  uint32_t is_corrupted = 0;
  sli_block_metadata_t *current = (sli_block_metadata_t *)heap->free_lt_list_head;
  sl_memory_region_t heap_region = sl_memory_get_heap_region();
  uint32_t reservation_size = 0;

  while (current != NULL) {
    // Reached last block in heap.
    if (sli_block_offset_next_dword_decode(current) == 0) {
      heap_end_by_metadata = (uint64_t *)current + sli_block_len_dword_decode(current) + SLI_BLOCK_METADATA_SIZE_DWORD;

      // Check if reservation (one or more).
      while ((reservation_size = sli_memory_get_reservation_size_by_addr((void *)heap_end_by_metadata)) != 0) {
        heap_end_by_metadata = (uint64_t *)((uint8_t *)heap_end_by_metadata + reservation_size);
      }

      // Check if reservation no retention (one or more) if heap end is at the end of the DMEM.
      if ((uint64_t *)heap->base_addr + heap->size == (uint64_t *)heap_region.addr + heap_region.size) {
        while ((reservation_size = sli_memory_get_reservation_no_retention_size((void *)heap_end_by_metadata)) != 0) {
          heap_end_by_metadata = (uint64_t *)((uint8_t *)heap_end_by_metadata + reservation_size);
        }
      }

      // Check the computed heap_end_by_metadata against real heap end, after accounting for any reserved blocks
      // and reserved blocks no retention.
      if (heap_end_by_metadata != (void *)((uintptr_t)heap->base_addr + heap->size)) {
        is_corrupted = 1;
      }
      break;
    }

    // Calculate the address of the next block using offset and length.
    sli_block_metadata_t *next_blk_by_offset = (sli_block_metadata_t *)((uint64_t *)current + sli_block_offset_next_dword_decode(current));
    sli_block_metadata_t *next_blk_by_len = (sli_block_metadata_t *)((uint64_t *)current + sli_block_len_dword_decode(current) + SLI_BLOCK_METADATA_SIZE_DWORD);

    // Check if reservation (one or more).
    while ((reservation_size = sli_memory_get_reservation_size_by_addr((void *)next_blk_by_len)) != 0) {
      next_blk_by_len = (sli_block_metadata_t *)((uint8_t *)next_blk_by_len + reservation_size);
    }

    // Check the computed next_blk_by_len against the next block, after accounting for any reserved blocks.
    if (next_blk_by_offset != next_blk_by_len) {
      is_corrupted = 1;
      break;
    } else {
      current = next_blk_by_offset;
    }
  }

  return is_corrupted ? current : NULL;
}

/***************************************************************************//**
 * Does a heap integrity check backwards from the general-purpose heap free_st_list_head
 * and return the pointer to the corrupted sli_block_metadata_t{} (if applicable).
 * This should not go past any reservations, hence there are no checks.
 ******************************************************************************/
sli_block_metadata_t *sli_memory_check_general_purpose_heap_integrity_backwards(void)
{
  return sli_memory_check_heap_integrity_backwards(&sli_general_purpose_heap);
}

/***************************************************************************//**
 * Does a heap integrity check backwards from free_st_list_head and return
 * the pointer to the corrupted sli_block_metadata_t{} (if applicable).
 * This should not go past any reservations, hence there are no checks.
 ******************************************************************************/
sli_block_metadata_t *sli_memory_check_heap_integrity_backwards(sl_memory_heap_t *heap)
{
  uint64_t *heap_base_by_metadata = 0;
  uint32_t is_corrupted = 0;
  uint32_t reservation_size;
  sli_block_metadata_t *current = (sli_block_metadata_t *)heap->free_st_list_head;

  while (current != NULL) {
    // Reached first block in heap.
    if (sli_block_offset_prev_dword_decode(current) == 0) {
      heap_base_by_metadata = ((uint64_t *)current);
      if (heap_base_by_metadata != (void *)heap->base_addr) {
        is_corrupted = 1;
      }
      break;
    }

    // Calculate the address of the current block using offset and length of the previous block.
    sli_block_metadata_t *prev_blk_by_offset = (sli_block_metadata_t *)((uint64_t *)current - sli_block_offset_prev_dword_decode(current));
    sli_block_metadata_t *current_by_prev_offset = (sli_block_metadata_t *)((uint64_t *)prev_blk_by_offset + sli_block_offset_next_dword_decode(prev_blk_by_offset));
    sli_block_metadata_t *current_by_prev_len = (sli_block_metadata_t *)((uint64_t *)prev_blk_by_offset + (sli_block_len_dword_decode(prev_blk_by_offset) + SLI_BLOCK_METADATA_SIZE_DWORD));

    // Check if reservation (one or more).
    // This is required when sli_general_purpose_heap.free_st_list_head has reservations before it.
    reservation_size = sli_memory_get_reservation_size_by_addr((void *)current_by_prev_len);

    while (reservation_size != 0) {
      current_by_prev_len = (sli_block_metadata_t *)((uint8_t*)current_by_prev_len + reservation_size);
      reservation_size = sli_memory_get_reservation_size_by_addr((void *)current_by_prev_len);
    }

    // Check the computed current_by_prev_len against the previous block, after accounting for any reserved blocks.
    // This doesn't apply if this is the first block at the heap start that has undergone a data payload adjustment.
    if ((current_by_prev_len != current_by_prev_offset) && !(current->heap_start_align)) {
      is_corrupted = 1;
      break;
    } else {
      current = prev_blk_by_offset;
    }
  }

  if (is_corrupted) {
    return (sli_block_metadata_t *)current;
  }

  return NULL;
}
#endif /* SLI_MEMORY_MANAGER_ENABLE_TEST_UTILITIES */
