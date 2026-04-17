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
#include <string.h>
#include <stdalign.h>

#include "sl_memory_manager_config.h"
#include "sl_memory_manager.h"
#include "sli_memory_manager.h"
#include "sli_code_classification.h"
#include "sl_assert.h"
#include "sl_bit.h"
#include "sl_common.h"
#include "sl_core.h"

#if defined(SL_COMPONENT_CATALOG_PRESENT)
#include "sl_component_catalog.h"
#endif

#if (defined(DEBUG_EFM) || defined(DEBUG_EFM_USER))
#if defined(SL_CATALOG_ERRNO_PRESENT)
#include "errno.h"
#else
#include <errno.h>
#endif
#include "errno_error_codes.h"
#endif

#if defined(SL_CATALOG_BANK_RETENTION_CONTROL_PRESENT)
#include "sli_memory_manager_retention_control.h"
#endif

#if defined(SL_CATALOG_MEMORY_PROFILER_PRESENT)
#include "em_device.h" // For SRAM_BASE and SRAM_SIZE
#include "sli_memory_profiler.h"

// Names for the top-level Memory Profiler trackers provided by the Common
// Memory Manager. The name string pointers are also used as the tracker
// handles. Heap and reservation names are shared with other files.
static const char sli_mm_ram_name[] = "Physical RAM";
static const char sli_mm_stack_name[] = "C stack";
const char sli_mm_heap_name[] = "MM Heap";
const char sli_mm_heap_reservation_name[] = "MM reservation";
static const char sli_mm_heap_malloc_lt_name[] = "MM malloc LT";
static const char sli_mm_heap_malloc_st_name[] = "MM malloc ST";
#endif

#if defined(SLI_MEMORY_MANAGER_ENABLE_SYSTEMVIEW)
#include "SEGGER_SYSVIEW.h"
extern char __HeapBase[];
extern char __HeapLimit[];

#define HEAP_SIZE (__HeapLimit - __HeapBase)

// Heap ID for SystemView heap definitions.
// These values are chosen to be bigger than SEGGER_SYSVIEW_ID_BASE.
#define HEAP_LT_ID 0xFFFFFFFF
#define HEAP_ST_ID 0xFFFFFFFE
#endif

/*******************************************************************************
 ***************************  LOCAL VARIABLES   ********************************
 ******************************************************************************/

sl_memory_heap_t sli_general_purpose_heap SL_FAST_DATA;
#if defined(SL_CATALOG_MEMORY_MANAGER_PSRAM_PRESENT)
sl_memory_heap_t sli_psram_heap SL_FAST_DATA;
#endif
#if defined(SL_CATALOG_MEMORY_MANAGER_DTCM_PRESENT)
sl_memory_heap_t sli_dtcm_heap SL_FAST_DATA;
#endif

#if defined(DEBUG_EFM) || defined(DEBUG_EFM_USER)
bool reserve_no_retention_first = true;
#endif

bool sli_mm_initialized = false;

/*******************************************************************************
 ***************************   LOCAL FUNCTIONS   *******************************
 ******************************************************************************/

static sli_block_metadata_t *memory_manage_data_alignment(sl_memory_heap_t *heap,
                                                          sli_block_metadata_t *current_block_metadata,
                                                          size_t block_align);

#if defined(SLI_MEMORY_MANAGER_SUPPORT_ALLOCATION_FALLBACK)
static sl_status_t memory_manage_allocation_fallback(size_t size,
                                                     size_t align,
                                                     uint8_t type,
                                                     void **block);
#endif

/*******************************************************************************
 **************************   GLOBAL FUNCTIONS   *******************************
 ******************************************************************************/

/***************************************************************************//**
 * Initializes the memory manager.
 *
 * @note (1) Calling this function multiple times will result in an unexpected
 *           behavior as the heap layout will be corrupted.
 ******************************************************************************/
sl_status_t sl_memory_init(void)
{
  sl_status_t status = SL_STATUS_OK;
  sl_memory_region_t heap_region = sl_memory_get_heap_region();

  // Check for double initialization
  if (sli_mm_initialized) {
    status = SL_STATUS_ALREADY_INITIALIZED;
    return status;
  }

  // Create the general-purpose heap.
  status = sli_memory_create_heap(heap_region.addr,
                                  heap_region.size,
                                  SL_MEMORY_HEAP_ALLOC_GENERAL_RAM,
                                  &sli_general_purpose_heap);

#if defined(DEBUG_EFM) || defined(DEBUG_EFM_USER)
  reserve_no_retention_first = true;
#endif

#if defined(SLI_MEMORY_MANAGER_ENABLE_TEST_UTILITIES)
  // Initialize reservations tables.
  for (uint32_t ix = 0; ix < SLI_MAX_RESERVATION_COUNT; ix++) {
    sli_reservation_handle_ptr_table[ix] = NULL;
    sli_reservation_no_retention_table[ix].block_address = NULL;
    sli_reservation_no_retention_table[ix].block_size = 0;
  }
#endif

#if defined(SLI_MEMORY_MANAGER_STACK_IN_HEAP)
  sli_memory_create_stack();
#endif

#if defined(SL_CATALOG_MEMORY_PROFILER_PRESENT)
  // Create the pool tracker for the physical RAM
  sli_memory_profiler_create_pool_tracker(sli_mm_ram_name,
                                          sli_mm_ram_name,
                                          (void*) (uintptr_t) SRAM_BASE,
                                          SRAM_SIZE);

  // Record the allocation of the stack from the physical RAM
  sl_memory_region_t stack_region = sl_memory_get_stack_region();
  sli_memory_profiler_track_alloc(sli_mm_ram_name,
                                  stack_region.addr,
                                  stack_region.size);

  // Record the creation of the pool tracker for the stack
  sli_memory_profiler_create_pool_tracker(sli_mm_stack_name,
                                          sli_mm_stack_name,
                                          stack_region.addr,
                                          stack_region.size);

  // Record the allocation of the heap from the physical RAM
  sli_memory_profiler_track_alloc(sli_mm_ram_name,
                                  heap_region.addr,
                                  heap_region.size);

  // Record the creation of the pool tracker for the heap
  sli_memory_profiler_create_pool_tracker(sli_mm_heap_name,
                                          sli_mm_heap_name,
                                          heap_region.addr,
                                          heap_region.size);

  // Create the malloc family of trackers
  sli_memory_profiler_create_tracker(sli_mm_heap_malloc_lt_name,
                                     sli_mm_heap_malloc_lt_name);
  sli_memory_profiler_create_tracker(sli_mm_heap_malloc_st_name,
                                     sli_mm_heap_malloc_st_name);

  // Create the reservation tracker
  sli_memory_profiler_create_tracker(sli_mm_heap_reservation_name,
                                     sli_mm_heap_reservation_name);
#endif

#if defined(SLI_MEMORY_MANAGER_ENABLE_SYSTEMVIEW)
  SEGGER_SYSVIEW_HeapDefine((void*)HEAP_LT_ID, (void*)__HeapBase, HEAP_SIZE, SLI_BLOCK_METADATA_SIZE_BYTE);
  SEGGER_SYSVIEW_HeapDefine((void*)HEAP_ST_ID, (void*)__HeapBase, HEAP_SIZE, SLI_BLOCK_METADATA_SIZE_BYTE);
  SEGGER_SYSVIEW_NameResource((uint32_t) HEAP_LT_ID, "HEAP LONG TERM");
  SEGGER_SYSVIEW_NameResource((uint32_t) HEAP_ST_ID, "HEAP SHORT TERM");
#endif

  if (status == SL_STATUS_OK) {
    sli_mm_initialized = true;
  }
  return status;
}

#if defined(SL_CATALOG_MEMORY_MANAGER_PSRAM_PRESENT)
/***************************************************************************//**
 * Initializes the PSRAM heap for memory allocations.
 ******************************************************************************/
sl_status_t sl_memory_init_psram(void)
{
  sl_status_t status = SL_STATUS_OK;

  sl_memory_region_t psram_heap_region = sl_memory_get_psram_heap_region();

  EFM_ASSERT(psram_heap_region.addr != NULL);
  EFM_ASSERT(psram_heap_region.size > 0);

  // Create PSRAM heap.
  status = sli_memory_create_heap(psram_heap_region.addr,
                                  psram_heap_region.size,
                                  SL_MEMORY_HEAP_ALLOC_EXTERNAL_RAM,
                                  &sli_psram_heap);

  return status;
}
#endif

#if defined(SL_CATALOG_MEMORY_MANAGER_DTCM_PRESENT)
/***************************************************************************//**
 * Initializes the DTCM heap for memory allocations.
 ******************************************************************************/
sl_status_t sl_memory_init_dtcm(void)
{
  sl_status_t status = SL_STATUS_OK;

  sl_memory_region_t dtcm_heap_region = sl_memory_get_dtcm_heap_region();

  EFM_ASSERT(dtcm_heap_region.addr != NULL);
  EFM_ASSERT(dtcm_heap_region.size > 0);

  // Create DTCM heap.
  status = sli_memory_create_heap(dtcm_heap_region.addr,
                                  dtcm_heap_region.size,
                                  SL_MEMORY_HEAP_ALLOC_CPU_RAM,
                                  &sli_dtcm_heap);

  return status;
}
#endif

/***************************************************************************//**
 * Reserves a memory block that will never need retention in EM2.
 ******************************************************************************/
sl_status_t sl_memory_reserve_no_retention(size_t size,
                                           size_t align,
                                           void **block)
{
  // Call the heap-specific function with the general-purpose heap.
  return sl_memory_heap_reserve_no_retention(&sli_general_purpose_heap, size, align, block);
}

/***************************************************************************//**
 * Allocates a memory block of at least requested size from the heap. Simple
 * version.
 ******************************************************************************/
void *sl_malloc(size_t size)
{
  uint8_t type;
#if defined(SL_CATALOG_MEMORY_PROFILER_PRESENT)
  void * volatile return_address = sli_memory_profiler_get_return_address();
#endif
  void *block_avail = NULL;

#if defined(SLI_MEMORY_MANAGER_SUPPORT_ALLOCATION_FALLBACK)
  type = (uint8_t)BLOCK_TYPE_LONG_TERM | (uint8_t)SL_MEMORY_HEAP_ALLOC_EXTERNAL_RAM;
#else
  type = BLOCK_TYPE_LONG_TERM;
#endif

#if (defined(DEBUG_EFM) || defined(DEBUG_EFM_USER))
  sl_status_t status = sl_memory_alloc_advanced(size, SL_MEMORY_BLOCK_ALIGN_DEFAULT, type, &block_avail);
  switch (status) {
    case SL_STATUS_NULL_POINTER:
      errno = ENOTRECOVERABLE;
      break;

    case SL_STATUS_INVALID_PARAMETER:
      errno = EINVAL;
      break;

    case SL_STATUS_ALLOCATION_FAILED:
      errno = ENOMEM;
      break;

    default:
      break;
  }
#else
  (void)sl_memory_alloc_advanced(size, SL_MEMORY_BLOCK_ALIGN_DEFAULT, type, &block_avail);
#endif

#if defined(SL_CATALOG_MEMORY_PROFILER_PRESENT)
  sli_memory_profiler_track_ownership(SLI_INVALID_MEMORY_TRACKER_HANDLE, block_avail, return_address);
#endif

  return block_avail;
}

/***************************************************************************//**
 * Dynamically allocates a block of memory from the general-purpose heap.
 ******************************************************************************/
sl_status_t sl_memory_alloc(size_t size,
                            uint8_t type,
                            void **block)
{
#if defined(SL_CATALOG_MEMORY_PROFILER_PRESENT)
  void * volatile return_address = sli_memory_profiler_get_return_address();
#endif
  sl_status_t status;

  status = sl_memory_heap_alloc(&sli_general_purpose_heap, size, type, block);

#if defined(SL_CATALOG_MEMORY_PROFILER_PRESENT)
  sli_memory_profiler_track_ownership(SLI_INVALID_MEMORY_TRACKER_HANDLE, *block, return_address);
#endif

  return status;
}

/***************************************************************************//**
 * Dynamically allocates a block of memory from the general-purpose heap.
 * Advanced version that allows to specify alignment.
 ******************************************************************************/
sl_status_t sl_memory_alloc_advanced(size_t size,
                                     size_t align,
                                     uint8_t type,
                                     void **block)
{
#if defined(SL_CATALOG_MEMORY_PROFILER_PRESENT)
  void * volatile return_address = sli_memory_profiler_get_return_address();
#endif
  sl_status_t status;

  status = sl_memory_heap_alloc_advanced(&sli_general_purpose_heap, size, align, type, block);

#if defined(SL_CATALOG_MEMORY_PROFILER_PRESENT)
  sli_memory_profiler_track_ownership(SLI_INVALID_MEMORY_TRACKER_HANDLE, *block, return_address);
#endif

  return status;
}

/***************************************************************************//**
 * Frees a previously allocated block back into the heap. Simple version.
 ******************************************************************************/
void sl_free(void *ptr)
{
#if (defined(DEBUG_EFM) || defined(DEBUG_EFM_USER))
  sl_status_t status = sl_memory_free(ptr);
  switch (status) {
    case SL_STATUS_NULL_POINTER:
      errno = EFAULT;
      break;

    case SL_STATUS_FAIL:
      errno = EINVAL;
      break;

    default:
      break;
  }
#else
  (void)sl_memory_free(ptr);
#endif
}

/***************************************************************************//**
 * Frees a dynamically allocated block of memory from the general-purpose heap.
 *
 * @note (1) According to IEEE Std 1003.1-2017 (Open Group Base Specifications
 *           Issue 7, 2018 edition), if ptr is a null pointer, no action shall
 *           occur.
 *
 * @note (2) There is no distinction between long-term (LT) and short-term (ST)
 *           blocks when freeing. The free operation is the same for both block
 *           types. Merging adjacent blocks is the same also. The only exception
 *           is the LT and ST head pointers. They must be updated accordingly
 *           with  the freed block. The freed block can be a LT or ST block at
 *           the next allocation.
 ******************************************************************************/
sl_status_t sl_memory_free(void *block)
{
  return sl_memory_heap_free(NULL, block);
}

/***************************************************************************//**
 * Dynamically allocates a memory block cleared to 0 from the general
 * purpose heap. Simple version.
 ******************************************************************************/
void *sl_calloc(size_t item_count,
                size_t size)
{
  uint8_t type;
#if defined(SL_CATALOG_MEMORY_PROFILER_PRESENT)
  void * volatile return_address = sli_memory_profiler_get_return_address();
#endif
  void *block_avail = NULL;

#if defined(SLI_MEMORY_MANAGER_SUPPORT_ALLOCATION_FALLBACK)
  type = (uint8_t)BLOCK_TYPE_LONG_TERM | (uint8_t)SL_MEMORY_HEAP_ALLOC_EXTERNAL_RAM;
#else
  type = BLOCK_TYPE_LONG_TERM;
#endif

#if (defined(DEBUG_EFM) || defined(DEBUG_EFM_USER))
  sl_status_t status = sl_memory_calloc(item_count, size, type, &block_avail);
  switch (status) {
    case SL_STATUS_NULL_POINTER:
      errno = ENOTRECOVERABLE;
      break;

    case SL_STATUS_INVALID_PARAMETER:
      errno = EINVAL;
      break;

    case SL_STATUS_ALLOCATION_FAILED:
      errno = ENOMEM;
      break;

    default:
      break;
  }
#else
  (void)sl_memory_calloc(item_count, size, type, &block_avail);
#endif

#if defined(SL_CATALOG_MEMORY_PROFILER_PRESENT)
  sli_memory_profiler_track_ownership(SLI_INVALID_MEMORY_TRACKER_HANDLE, block_avail, return_address);
#endif

  return block_avail;
}

/***************************************************************************//**
 * Dynamically allocates a memory block cleared to 0 from the general
 * purpose heap.
 ******************************************************************************/
sl_status_t sl_memory_calloc(size_t item_count,
                             size_t size,
                             uint8_t type,
                             void **block)
{
#if defined(SL_CATALOG_MEMORY_PROFILER_PRESENT)
  void * volatile return_address = sli_memory_profiler_get_return_address();
#endif
  sl_status_t status;

  status = sl_memory_heap_calloc(&sli_general_purpose_heap, item_count, size, type, block);

#if defined(SL_CATALOG_MEMORY_PROFILER_PRESENT)
  sli_memory_profiler_track_ownership(SLI_INVALID_MEMORY_TRACKER_HANDLE, *block, return_address);
#endif

  return status;
}

/***************************************************************************//**
 * Resizes a previously allocated memory block. Simple version.
 ******************************************************************************/
void *sl_realloc(void *ptr,
                 size_t size)
{
#if defined(SL_CATALOG_MEMORY_PROFILER_PRESENT)
  void * volatile return_address = sli_memory_profiler_get_return_address();
#endif
  void *block_avail = NULL;

#if (defined(DEBUG_EFM) || defined(DEBUG_EFM_USER))
  sl_status_t status = sl_memory_realloc(ptr, size, &block_avail);
  switch (status) {
    case SL_STATUS_NULL_POINTER:
      errno = ENOTRECOVERABLE;
      break;

    case SL_STATUS_INVALID_PARAMETER:
      errno = EINVAL;
      break;

    case SL_STATUS_ALLOCATION_FAILED:
      errno = ENOMEM;
      break;

    case SL_STATUS_FAIL:
      errno = EINVAL;
      break;

    default:
      break;
  }
#else
  (void)sl_memory_realloc(ptr, size, &block_avail);
#endif

#if defined(SL_CATALOG_MEMORY_PROFILER_PRESENT)
  // Realloc to 0 bytes is equivalent to free, so only track ownership when size
  // is other than 0
  if (size != 0) {
    sli_memory_profiler_track_ownership(SLI_INVALID_MEMORY_TRACKER_HANDLE, block_avail, return_address);
  }
#endif

  return block_avail;
}

/***************************************************************************//**
 * Resizes a previously allocated memory block.
 *
 * @note (1) If 'ptr' is a null pointer, sl_memory_realloc() is equivalent to
 *           sl_memory_malloc() for the specified 'size'.
 *           If 'size' is 0 and 'ptr' points to an existing block of memory,
 *           sl_memory_realloc() is equivalent to sl_memory_free() and the
 *           memory block is deallocated.
 *
 * @note (2) The content of the reallocated memory block is preserved up to
 *           the lesser of the new and old sizes, even if the block is moved
 *           to a new location. If the new size is larger, the value of the
 *           newly allocated portion is indeterminate.
 ******************************************************************************/
sl_status_t sl_memory_realloc(void *ptr,
                              size_t size,
                              void **block)
{
#if defined(SL_CATALOG_MEMORY_PROFILER_PRESENT)
  void * volatile return_address = sli_memory_profiler_get_return_address();
#endif
  sl_status_t status;

#if defined(SL_CATALOG_MEMORY_MANAGER_PSRAM_PRESENT) || defined (SL_CATALOG_MEMORY_MANAGER_DTCM_PRESENT)
  sl_memory_heap_t *heap;

  // Retrieve the heap where the block was previously allocated.
  heap = sli_memory_get_heap_handle(ptr);

  // If heap is NULL, use the general purpose heap.
  // This allows realloc to do allocation if ptr is NULL.
  heap = (heap == NULL) ? &sli_general_purpose_heap : heap;

  status = sl_memory_heap_realloc(heap, ptr, size, block);
#else
  // Assume general_purpose_heap on boards that don't have a PSRAM or a DTCM
  status = sl_memory_heap_realloc(&sli_general_purpose_heap, ptr, size, block);
#endif

#if defined(SL_CATALOG_MEMORY_PROFILER_PRESENT)
  // Realloc to 0 bytes is equivalent to free, so only track ownership when size
  // is other than 0
  if (size != 0) {
    sli_memory_profiler_track_ownership(SLI_INVALID_MEMORY_TRACKER_HANDLE, *block, return_address);
  }
#endif

  return status;
}

/***************************************************************************//**
 * Populates an sl_memory_heap_info_t{} structure with the current status of
 * the general-purpose heap.
 ******************************************************************************/
sl_status_t sl_memory_get_heap_info(sl_memory_heap_info_t *heap_info)
{
  return sl_memory_heap_get_info(&sli_general_purpose_heap, heap_info);
}

/***************************************************************************//**
 * Retrieves the total size of the general-purpose heap.
 ******************************************************************************/
size_t sl_memory_get_total_heap_size(void)
{
  return sl_memory_heap_get_total_size(&sli_general_purpose_heap);
}

/***************************************************************************//**
 * Retrieves the current amount of free memory in the general-purpose heap.
 ******************************************************************************/
size_t sl_memory_get_free_heap_size(void)
{
  return sl_memory_heap_get_free_size(&sli_general_purpose_heap);
}

/***************************************************************************//**
 * Retrieves the current amount of memory used in the general-purpose heap.
 ******************************************************************************/
size_t sl_memory_get_used_heap_size(void)
{
  return sl_memory_heap_get_used_size(&sli_general_purpose_heap);
}

/***************************************************************************//**
 * Retrieves the general-purpose heap's high watermark.
 ******************************************************************************/
size_t sl_memory_get_heap_high_watermark(void)
{
  return sl_memory_heap_get_high_watermark(&sli_general_purpose_heap);
}

/***************************************************************************//**
 * Resets the general-purpose heap's high watermark to the current heap used.
 ******************************************************************************/
void sl_memory_reset_heap_high_watermark(void)
{
  sl_memory_heap_reset_high_watermark(&sli_general_purpose_heap);
}

/***************************************************************************//**
 * Reserves a memory block that will never need retention in EM2 from a specific
 * heap instance.
 *
 * @note (1) A first check is done to verify the heap has enough space for the
 *           requested size. This first check is not enough in case there is an
 *           alignment adjustment done thereafter. A second check is done to
 *           ensure the remaining size is enough to accommodate the requested
 *           size in case of alignment adjustment.
 ******************************************************************************/
sl_status_t sl_memory_heap_reserve_no_retention(sl_memory_heap_t *heap,
                                                size_t size,
                                                size_t align,
                                                void **block)
{
  // Check proper alignment characteristics.
  EFM_ASSERT((align == SL_MEMORY_BLOCK_ALIGN_DEFAULT)
             || (SL_MATH_IS_PWR2(align)
                 && (align <= SL_MEMORY_BLOCK_ALIGN_512_BYTES)));

#if defined(DEBUG_EFM) || defined(DEBUG_EFM_USER)
  // Assert block reservation with retention is done prior to any other allocations types.
  EFM_ASSERT(reserve_no_retention_first == true);
#endif

  // Make sure the heap handle isn't NULL.
  EFM_ASSERT(heap != NULL);

  size_t block_align = align;
  void *data_payload_start = NULL;
  size_t block_size_remaining;
  size_t block_len_dw;
  size_t size_real;
  sl_status_t status;
  sli_block_metadata_t *free_st_list_head = (sli_block_metadata_t *)heap->free_st_list_head;

  // Verify that the block pointer isn't NULL.
  if (block == NULL) {
    return SL_STATUS_NULL_POINTER;
  }

  *block = NULL; // No block reserved yet.

  if ((size == 0) || (size >= heap->size)) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  // Adjust size to match the minimum alignment to maximize CPU access performance.
  size_real = SLI_ALIGN_ROUND_UP(size, SLI_BLOCK_ALLOC_MIN_ALIGN);

  // Adjust alignment.
  if ((block_align == SL_MEMORY_BLOCK_ALIGN_DEFAULT) || (block_align <= 4U)) {
    block_align = SLI_BLOCK_ALLOC_MIN_ALIGN;
  }

  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_ATOMIC();

  block_len_dw = sli_block_len_dword_decode(free_st_list_head);
  block_size_remaining = SLI_BLOCK_LEN_DWORD_TO_BYTE(block_len_dw);
  // Verify there is enough space in heap.
  if (block_size_remaining >= size_real) {
    // Get aligned block: get address from end of available heap minus the requested size. Round down this address.
    *block = (void *)(((uint64_t *)free_st_list_head + (block_len_dw + SLI_BLOCK_METADATA_SIZE_DWORD)) - SLI_BLOCK_LEN_BYTE_TO_DWORD(size_real));
    *block = (void *)SLI_ALIGN_ROUND_DOWN(((uintptr_t)*block), block_align);

    // Update heap start metadata. Available heap size reduced from reserved block size aligned.
    data_payload_start = (void *)((uint8_t *)free_st_list_head + SLI_BLOCK_METADATA_SIZE_BYTE);
    sli_block_len_dword_encode(free_st_list_head, ((uint64_t *)*block - (uint64_t *)data_payload_start));

    // Ensure there is still enough space after alignment. See Note #1.
    block_len_dw = sli_block_len_dword_decode(free_st_list_head);
    if (block_size_remaining < SLI_BLOCK_LEN_DWORD_TO_BYTE(block_len_dw)) {
      CORE_EXIT_ATOMIC();
      return SL_STATUS_ALLOCATION_FAILED;
    }

    status = SL_STATUS_OK;
  } else {
    status = SL_STATUS_ALLOCATION_FAILED;
  }

#if defined(SL_MEMORY_MANAGER_STATISTICS_API_ENABLE) && (SL_MEMORY_MANAGER_STATISTICS_API_ENABLE == 1)
  if (status == SL_STATUS_OK) {
    // Account for the real heap consumption, including any extra bytes lost due to alignment.
    size_t remaining = SLI_BLOCK_LEN_DWORD_TO_BYTE(sli_block_len_dword_decode(free_st_list_head));
    size_t consumed = block_size_remaining - remaining;
    SLI_MEMORY_STAT_HEAP_INCREASE(heap, consumed);
  }
#endif

  CORE_EXIT_ATOMIC();

#if defined(SLI_MEMORY_MANAGER_ENABLE_TEST_UTILITIES)
  sli_memory_save_reservation_no_retention(*block, size_real);
#endif

  return status;
}

/***************************************************************************//**
 * Dynamically allocates a block of memory from a specific heap instance.
 ******************************************************************************/
sl_status_t sl_memory_heap_alloc(sl_memory_heap_t *heap,
                                 size_t size,
                                 uint8_t type,
                                 void **block)
{
#if defined(SL_CATALOG_MEMORY_PROFILER_PRESENT)
  void * volatile return_address = sli_memory_profiler_get_return_address();
#endif
  sl_status_t status;

  status = sl_memory_heap_alloc_advanced(heap, size, SL_MEMORY_BLOCK_ALIGN_DEFAULT, type, block);

#if defined(SL_CATALOG_MEMORY_PROFILER_PRESENT)
  sli_memory_profiler_track_ownership(SLI_INVALID_MEMORY_TRACKER_HANDLE, *block, return_address);
#endif

  return status;
}

/***************************************************************************//**
 * Dynamically allocates a block of memory from a specific heap instance.
 * Advanced version that allows to specify alignment.
 *
 * @note (1) When splitting a long-term (LT) block, the block part not returned
 *           to the requester becomes a free block. The new block considered as
 *           a free block can be either a LT or short-term (ST) block at the
 *           next allocation. Consequently, the LT and ST head pointers must
 *           be updated to this new free block start address if it is needed.
 ******************************************************************************/
sl_status_t sl_memory_heap_alloc_advanced(sl_memory_heap_t *heap,
                                          size_t size,
                                          size_t align,
                                          uint8_t type,
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

  sli_block_metadata_t *current_block_metadata = NULL;
  sli_block_metadata_t *allocated_blk = NULL;
  const sli_block_metadata_t *old_block_metadata = NULL;
  size_t block_len_dw;
  size_t current_block_len;
  size_t size_real;
  size_t size_adjusted;
  size_t block_size_remaining;
  size_t block_align = (align == SL_MEMORY_BLOCK_ALIGN_DEFAULT) ? SLI_BLOCK_ALLOC_MIN_ALIGN : align;
  size_t other_offset = 0;
  bool is_aligned = false;
  sl_memory_block_type_t block_type = (sl_memory_block_type_t) (type & SLI_MEMORY_BLOCK_TYPE_MASK);
#if defined(DEBUG_EFM) || defined(DEBUG_EFM_USER)
  reserve_no_retention_first = false;
#endif

  // Verify that the block pointer isn't NULL.
  if (block == NULL) {
    return SL_STATUS_NULL_POINTER;
  }

  *block = NULL; // No block allocated yet.

  if ((size == 0) || (size >= heap->size)) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  // Adjust size to match the minimum alignment to maximize CPU access performance.
  size_real = SLI_ALIGN_ROUND_UP(size, SLI_BLOCK_ALLOC_MIN_ALIGN);

  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_ATOMIC();
  size_adjusted = sli_memory_find_free_block(heap,
                                             size_real,
                                             align,
                                             block_type,
                                             false,
                                             &current_block_metadata);

  if ((current_block_metadata == NULL) || (size_adjusted == 0)) {
    CORE_EXIT_ATOMIC();
#if defined(SLI_MEMORY_MANAGER_SUPPORT_ALLOCATION_FALLBACK)
    sl_memory_block_attrib_t blocks_heap_attribute = (sl_memory_block_attrib_t) (type & SLI_MEMORY_BLOCK_ATTRIBUTE_MASK);
    if (blocks_heap_attribute != 0) {
      sl_status_t status;
      status = memory_manage_allocation_fallback(size,
                                                 align,
                                                 type,
                                                 block);
      if (status == SL_STATUS_OK) {
        return SL_STATUS_OK;
      }
    }
#endif

#if defined(SL_CATALOG_MEMORY_PROFILER_PRESENT)
    sli_memory_profiler_track_alloc_with_ownership(sli_mm_heap_name, NULL, size, return_address);
#endif  // defined(SL_CATALOG_MEMORY_PROFILER_PRESENT)
    return SL_STATUS_ALLOCATION_FAILED;
  }

  // The adjusted size changes only when the free block isn't aligned.
  is_aligned = (size_adjusted == size_real) ? true : false;

  block_len_dw = sli_block_len_dword_decode(current_block_metadata);
  current_block_len = SLI_BLOCK_LEN_DWORD_TO_BYTE(block_len_dw);
  block_size_remaining = current_block_len - size_adjusted;

  // Prepare found block.
  allocated_blk = current_block_metadata;

  // Update counter of free blocks.
  heap->free_blocks_number--;

  // Split allocated block if possible.
  if (block_size_remaining >= SLI_BLOCK_ALLOCATION_MIN_SIZE) {
    sli_block_metadata_t *new_free_blk;

    if (block_type == BLOCK_TYPE_LONG_TERM) {
      size_t new_free_blk_offset = size_real + SLI_BLOCK_METADATA_SIZE_BYTE;

      // Verify if alignment adjustment is required.
      old_block_metadata = current_block_metadata;
      if (!is_aligned) {
        current_block_metadata = memory_manage_data_alignment(heap, current_block_metadata, block_align);
        allocated_blk = current_block_metadata;
      }

      // Create a new free block with the adjusted size.
      new_free_blk = (sli_block_metadata_t *)((uint8_t *)current_block_metadata + new_free_blk_offset);
      sli_memory_metadata_init(new_free_blk);
      block_len_dw = sli_block_len_dword_decode(current_block_metadata);
      sli_block_len_dword_encode(new_free_blk, (block_len_dw - SLI_BLOCK_LEN_BYTE_TO_DWORD(size_real + SLI_BLOCK_METADATA_SIZE_BYTE)));

      sli_block_offset_prev_dword_encode(new_free_blk, SLI_BLOCK_LEN_BYTE_TO_DWORD(new_free_blk_offset));

      if (sli_block_offset_next_dword_decode(current_block_metadata) != 0) {
        sli_block_metadata_t *old_next_block = (sli_block_metadata_t *)((uint64_t *)current_block_metadata + sli_block_offset_next_dword_decode(current_block_metadata));

        // Split block had a next block before the split. Update metadata of new free block
        // and the metadata of the next block after the new free block.
        sli_block_offset_next_dword_encode(new_free_blk, (sli_block_offset_next_dword_decode(current_block_metadata) - SLI_BLOCK_LEN_BYTE_TO_DWORD(new_free_blk_offset)));
        sli_block_offset_prev_dword_encode(old_next_block, sli_block_offset_next_dword_decode(new_free_blk));
      } else {
        sli_block_offset_next_dword_encode(new_free_blk, 0); // end of heap.
      }

      // Initialize final metadata of found block.
      sli_block_len_dword_encode(allocated_blk, SLI_BLOCK_LEN_BYTE_TO_DWORD(size_real));
      sli_block_offset_prev_dword_encode(allocated_blk, sli_block_offset_prev_dword_decode(current_block_metadata));
      sli_block_offset_next_dword_encode(allocated_blk, sli_block_offset_prev_dword_decode(new_free_blk));

      // Update head pointers. See Note #1.
      sli_update_free_list_heads(heap, new_free_blk, old_block_metadata, false);

      // Decrement bank counter for previous free block metadata. Will be accounted in allocation.
      DECREMENT_BANK_COUNTER(heap, (uint8_t *) allocated_blk, (uint8_t *)allocated_blk + SLI_BLOCK_METADATA_SIZE_BYTE - 1);

      // Increment bank counter for new free block metadata.
      INCREMENT_BANK_COUNTER(heap, (uint8_t *)new_free_blk, (uint8_t *)new_free_blk + SLI_BLOCK_METADATA_SIZE_BYTE - 1);
    } else {
      // Create a new block = allocated block returned to requester. This new block is the nearest to the heap end.
      allocated_blk = (sli_block_metadata_t *)((uint8_t *)current_block_metadata + block_size_remaining);

      sli_memory_metadata_init(allocated_blk);
      sli_block_len_dword_encode(allocated_blk, SLI_BLOCK_LEN_BYTE_TO_DWORD(size_adjusted));
      sli_block_offset_prev_dword_encode(allocated_blk, SLI_BLOCK_LEN_BYTE_TO_DWORD(block_size_remaining));

      if (sli_block_offset_next_dword_decode(current_block_metadata) != 0) {
        sli_block_metadata_t *next_blk = (sli_block_metadata_t *)((uint64_t *)current_block_metadata + sli_block_offset_next_dword_decode(current_block_metadata));
        // Add other offset (in case reservation is used).
        other_offset = SLI_BLOCK_LEN_DWORD_TO_BYTE(sli_block_offset_next_dword_decode(current_block_metadata)) - current_block_len;
        sli_block_offset_next_dword_encode(allocated_blk, SLI_BLOCK_LEN_BYTE_TO_DWORD(size_adjusted + other_offset));
        sli_block_offset_prev_dword_encode(next_blk, sli_block_offset_next_dword_decode(allocated_blk));
      } else {
        sli_block_offset_next_dword_encode(allocated_blk, 0); // end of heap.
      }

      // Update original found block which becomes a free block.
      new_free_blk = current_block_metadata;
      sli_block_len_dword_encode(new_free_blk, SLI_BLOCK_LEN_BYTE_TO_DWORD(block_size_remaining - SLI_BLOCK_METADATA_SIZE_BYTE));

      sli_block_offset_next_dword_encode(new_free_blk, sli_block_offset_prev_dword_decode(allocated_blk));

      // Data payload alignment for short-term is managed during the first-fit algorithm loop
      // at the beginning of this function.

      // When the found block is split for ST, the newly created block becomes the allocated block
      // returned to the requester. In that case, the remaining part is already pointed by
      // the ST header pointer and is the first free block to consider for the next allocation.
      // Thus no need to update the ST header pointer. ST head pointer is always the same when the
      // block is split. LT head pointer is left untouched for ST block allocation with split.
    }

    // New block is created so there is a new metadata metadata.
    SLI_MEMORY_STAT_HEAP_INCREASE(heap, SLI_BLOCK_METADATA_SIZE_BYTE);
    allocated_blk->block_in_use = true;
    // Account for the split block that is free.
    heap->free_blocks_number++;
  } else {
    // Verify if alignment adjustment is required.
    old_block_metadata = allocated_blk;
    if (block_type == BLOCK_TYPE_SHORT_TERM) {
      const void *data_payload = (void *)((uint8_t *)allocated_blk + SLI_BLOCK_METADATA_SIZE_BYTE);
      is_aligned = SLI_ADDR_IS_ALIGNED(data_payload, block_align);
    }
    if (!is_aligned) {
      allocated_blk = memory_manage_data_alignment(heap, allocated_blk, block_align);
    }

    // Initialize final metadata of found block that was not split.
    // Other metadata members (length and offsets to previous/next) are already correctly set.
    allocated_blk->block_in_use = true; // This setting must be done prior to calling sli_memory_find_head_free_block().

    // Update head pointers accordingly.
    sli_update_free_list_heads(heap, allocated_blk, old_block_metadata, true);

    // Decrement bank counter for previous free block metadata. Will be accounted in allocation.
    DECREMENT_BANK_COUNTER(heap, (uint8_t *)allocated_blk, (uint8_t *)allocated_blk + SLI_BLOCK_METADATA_SIZE_BYTE - 1);
  }

  block_len_dw = sli_block_len_dword_decode(allocated_blk);
  SLI_MEMORY_STAT_HEAP_INCREASE(heap, SLI_BLOCK_LEN_DWORD_TO_BYTE(block_len_dw));

  CORE_EXIT_ATOMIC();

  *block = (void *)((uint8_t *)allocated_blk + SLI_BLOCK_METADATA_SIZE_BYTE);

  // Increment bank counters for banks spanning the new allocation.
  // Include metadata as it was removed or is new.
  INCREMENT_BANK_COUNTER(heap, (uint8_t *)allocated_blk, (uint8_t *)*block + SLI_BLOCK_LEN_DWORD_TO_BYTE(allocated_blk->length) - 1);

#if defined(SL_CATALOG_MEMORY_PROFILER_PRESENT)
  sli_memory_profiler_track_alloc(sli_mm_heap_name, allocated_blk, size_real + SLI_BLOCK_METADATA_SIZE_BYTE);
  if (block_type == BLOCK_TYPE_LONG_TERM) {
    sli_memory_profiler_track_alloc_with_ownership(sli_mm_heap_malloc_lt_name, *block, size, return_address);
  } else if (block_type == BLOCK_TYPE_SHORT_TERM) {
    sli_memory_profiler_track_alloc_with_ownership(sli_mm_heap_malloc_st_name, *block, size, return_address);
  }
#endif

#if defined(SLI_MEMORY_MANAGER_ENABLE_SYSTEMVIEW)
  allocated_blk->block_type = type;
  uint32_t tag = (uint32_t)__builtin_extract_return_addr(__builtin_return_address(0));
  tag |= type << 31U;
  if (block_type == BLOCK_TYPE_LONG_TERM) {
    SEGGER_SYSVIEW_HeapAllocEx((void*)HEAP_LT_ID, *block, size, tag);
  } else if (block_type == BLOCK_TYPE_SHORT_TERM) {
    SEGGER_SYSVIEW_HeapAllocEx((void*)HEAP_ST_ID, *block, size, tag);
  }
#endif

  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Frees a previously allocated block back into a specific heap instance.
 *
 * @note (1) According to IEEE Std 1003.1-2017 (Open Group Base Specifications
 *           Issue 7, 2018 edition), if ptr is a null pointer, no action shall
 *           occur.
 *
 * @note (2) There is no distinction between long-term (LT) and short-term (ST)
 *           blocks when freeing. The free operation is the same for both block
 *           types. Merging adjacent blocks is the same also. The only exception
 *           is the LT and ST head pointers. They must be updated accordingly
 *           with  the freed block. The freed block can be a LT or ST block at
 *           the next allocation.
 *
 * @note (3) The heap parameter is not used in this function.
 *           This parameter is present to preserve the API consistency with the
 *           other sl_memory_heap_xxx() functions.
 *           The heap handle is retrieved from the block pointer passed as
 *           a parameter.
 ******************************************************************************/
sl_status_t sl_memory_heap_free(sl_memory_heap_t *heap,
                                void *block)
{
  (void)heap; // Unused parameter, see Note #3.
  sl_memory_heap_t *block_heap;

  sli_block_metadata_t *free_lt_list_head;
  sli_block_metadata_t *free_st_list_head;
  size_t block_len_dw;
  size_t total_size_free_block_dw;

  if (block == NULL) {
    return SL_STATUS_NULL_POINTER;  // See Note #1.
  }

  block_heap = sli_memory_get_heap_handle(block);
  free_lt_list_head = (sli_block_metadata_t *)block_heap->free_lt_list_head;
  free_st_list_head = (sli_block_metadata_t *)block_heap->free_st_list_head;

#if defined(SL_CATALOG_MEMORY_PROFILER_PRESENT)
  sli_memory_profiler_track_free(sli_mm_heap_name, ((uint8_t *)block - SLI_BLOCK_METADATA_SIZE_BYTE));
#endif

  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_ATOMIC();

  sli_block_metadata_t *current_metadata = (sli_block_metadata_t *)((uint8_t *)block - SLI_BLOCK_METADATA_SIZE_BYTE);
  // Ensure the block being freed was in use with a valid length.
  block_len_dw = sli_block_len_dword_decode(current_metadata);
  if (current_metadata->block_in_use == 0 || block_len_dw == 0) {
    CORE_EXIT_ATOMIC();
    return SL_STATUS_FAIL;
  }

  total_size_free_block_dw = block_len_dw + SLI_BLOCK_METADATA_SIZE_DWORD;
  sli_block_metadata_t *free_block = current_metadata;
  sli_block_metadata_t *next_block = NULL;
  SLI_MEMORY_STAT_HEAP_DECREASE(block_heap, SLI_BLOCK_LEN_DWORD_TO_BYTE(block_len_dw));

  // Decrement bank counters for banks spanning the freed allocation.
  // Include metadata as it is part of the allocation for the bank counters.
  DECREMENT_BANK_COUNTER(block_heap, (uint8_t *)current_metadata, (uint8_t *)block + SLI_BLOCK_LEN_DWORD_TO_BYTE(current_metadata->length) - 1);

  // Update counter with block being freed.
  block_heap->free_blocks_number++;

  // Check if previous block exists and is free.
  if (sli_block_offset_prev_dword_decode(current_metadata) > 0) {
    sli_block_metadata_t *metadata_prev_blk = (sli_block_metadata_t *)((uint64_t *)current_metadata - sli_block_offset_prev_dword_decode(current_metadata));
    size_t prev_blk_len_dw = sli_block_len_dword_decode(metadata_prev_blk);

    // Check that there is no reservation between current block and previous block.
    size_t reservations_size_prev = sli_block_offset_next_dword_decode(metadata_prev_blk) - prev_blk_len_dw - SLI_BLOCK_METADATA_SIZE_DWORD;

    if ((!metadata_prev_blk->block_in_use && !current_metadata->heap_start_align)
        && (reservations_size_prev == 0)) {
      // Merge current block to free with previous adjacent block.
      free_block = metadata_prev_blk;
      total_size_free_block_dw += prev_blk_len_dw + SLI_BLOCK_METADATA_SIZE_DWORD;

      // 2 free blocks have been merged, account for 1 free block only.
      block_heap->free_blocks_number--;
      // To account for one less metadata in heap.
      SLI_MEMORY_STAT_HEAP_DECREASE(block_heap, SLI_BLOCK_METADATA_SIZE_BYTE);
    } else if (current_metadata->heap_start_align) {
      // Special block whose data payload was aligned near heap start. Merge process is special as between
      // the heap start and the block metadata, there is a lost zone to be merged. But at heap start, there is
      // no valid metadata. A new valid metadata will exist after this special merge.
      free_block = metadata_prev_blk;
      total_size_free_block_dw += sli_block_offset_prev_dword_decode(current_metadata);
      // Release the heap-start alignment padding that was previously counted as used.
      SLI_MEMORY_STAT_HEAP_DECREASE(block_heap, SLI_BLOCK_LEN_DWORD_TO_BYTE(sli_block_offset_prev_dword_decode(current_metadata)));

      current_metadata->heap_start_align = false;
      sli_block_offset_prev_dword_encode(free_block, 0);   // heap start.

      // Increment counter for new free metadata
      INCREMENT_BANK_COUNTER(block_heap, (uint8_t *)free_block, (uint8_t *)free_block + SLI_BLOCK_METADATA_SIZE_BYTE - 1);
    } else {
      // Previous block is used, nothing to merge but bank counters need to be incremented to preserve free block metadata.
      INCREMENT_BANK_COUNTER(block_heap, (uint8_t *)free_block, (uint8_t *)free_block + SLI_BLOCK_METADATA_SIZE_BYTE - 1);
    }
  } else {
    // Previous block is the heap start. Nothing to merge but bank counters need to be incremented to preserve free block metadata.
    INCREMENT_BANK_COUNTER(block_heap, (uint8_t *)free_block, (uint8_t *)free_block + SLI_BLOCK_METADATA_SIZE_BYTE - 1);
  }

  // Check if next block exists and is free.
  if ((sli_block_offset_next_dword_decode(current_metadata) > 0)
      && (((size_t)current_metadata + SLI_BLOCK_LEN_DWORD_TO_BYTE(sli_block_offset_next_dword_decode(current_metadata))) < ((size_t)block_heap->base_addr + block_heap->size))) {
    next_block = (sli_block_metadata_t *)((uint64_t *)current_metadata + sli_block_offset_next_dword_decode(current_metadata));

    // Check that there is no reservation between current block and next block.
    size_t reservations_size_next = sli_block_offset_next_dword_decode(current_metadata) - sli_block_len_dword_decode(current_metadata) - SLI_BLOCK_METADATA_SIZE_DWORD;

    if ((!next_block->block_in_use) && (reservations_size_next == 0)) {
      // Remove metadata of next block from bank counter as free block will be merged with adjacent block.
      DECREMENT_BANK_COUNTER(block_heap, (uint8_t*)next_block, (uint8_t*)next_block + SLI_BLOCK_METADATA_SIZE_BYTE - 1);

      // Merge block with next adjacent block.
      block_len_dw = sli_block_len_dword_decode(next_block);
      total_size_free_block_dw += block_len_dw + SLI_BLOCK_METADATA_SIZE_DWORD;
      // Invalidate the next block metadata.
      sli_block_len_dword_encode(next_block, 0);
      // Get the "next" block adjacent to the invalidated next block.
      next_block = (sli_block_offset_next_dword_decode(next_block) == 0) ? NULL : ((sli_block_metadata_t *)((uint64_t *)next_block + sli_block_offset_next_dword_decode(next_block)));

      // 2 free blocks have been merged, account for 1 free block only.
      block_heap->free_blocks_number--;

      // To account for one less metadata in heap.
      SLI_MEMORY_STAT_HEAP_DECREASE(block_heap, SLI_BLOCK_METADATA_SIZE_BYTE);
    }
  }

  // Invalidate the block being freed.
  current_metadata->block_in_use = 0;
  sli_block_len_dword_encode(current_metadata, 0);

  // Update accordingly the metadata block considered as free.
  sli_block_len_dword_encode(free_block, (total_size_free_block_dw - SLI_BLOCK_METADATA_SIZE_DWORD));
  free_block->block_in_use = 0;
  if (next_block != NULL) {
    // Update implicit double linked-list.
    sli_block_offset_next_dword_encode(free_block, SLI_BLOCK_LEN_BYTE_TO_DWORD((size_t)next_block - (size_t)free_block));
    sli_block_offset_prev_dword_encode(next_block, sli_block_offset_next_dword_decode(free_block));
  } else {
    sli_block_offset_next_dword_encode(free_block, 0);  // Next block is the heap end.
  } // free_block->offset_neighbour_prev does not change.

  // Update free list heads. See Note #2.
  if (free_lt_list_head == NULL             // LT list is empty. Freed block becomes the new 1st element.
      || free_lt_list_head > free_block     // LT list not empty. Verify if freed block becomes the head.
      || sli_block_len_dword_decode(free_lt_list_head) == 0) {
    free_lt_list_head = free_block;
  }

  if (free_st_list_head == NULL             // ST list is empty. Freed block becomes the new 1st element.
      || free_st_list_head < free_block     // ST list not empty. Verify if freed block becomes the head.
      || sli_block_len_dword_decode(free_st_list_head) == 0) {
    free_st_list_head = free_block;
  }
  // Update the heap's head pointers.
  block_heap->free_lt_list_head = (void *)free_lt_list_head;
  block_heap->free_st_list_head = (void *)free_st_list_head;

  CORE_EXIT_ATOMIC();

#if defined(SLI_MEMORY_MANAGER_ENABLE_SYSTEMVIEW)
  if ((current_metadata->block_type & SLI_MEMORY_BLOCK_TYPE_MASK) == BLOCK_TYPE_LONG_TERM) {
    SEGGER_SYSVIEW_HeapFree((void*)HEAP_LT_ID, block);
  } else if ((current_metadata->block_type & SLI_MEMORY_BLOCK_TYPE_MASK) == BLOCK_TYPE_SHORT_TERM) {
    SEGGER_SYSVIEW_HeapFree((void*)HEAP_ST_ID, block);
  }
#endif

  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Dynamically allocates a memory block cleared to 0 from a specific
 * heap instance.
 ******************************************************************************/
sl_status_t sl_memory_heap_calloc(sl_memory_heap_t *heap,
                                  size_t item_count,
                                  size_t size,
                                  uint8_t type,
                                  void **block)
{
#if defined(SL_CATALOG_MEMORY_PROFILER_PRESENT)
  void * volatile return_address = sli_memory_profiler_get_return_address();
#endif
  size_t block_size;
  sl_status_t status = SL_STATUS_OK;

  // Verify that the block pointer isn't NULL.
  if (block == NULL) {
    return SL_STATUS_NULL_POINTER;
  }

  *block = NULL; // No block allocated yet.

  if ((size == 0) || (item_count == 0)) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  // Calculate block size to allocate.
  block_size = item_count * size;

  // Allocate block.
  status = sl_memory_heap_alloc(heap, block_size, type, block);

  if ((status == SL_STATUS_OK) && (*block != NULL)) {
    // Clear block to zeros.
    memset(*block, 0, block_size);
  }

#if defined(SL_CATALOG_MEMORY_PROFILER_PRESENT)
  sli_memory_profiler_track_ownership(SLI_INVALID_MEMORY_TRACKER_HANDLE, *block, return_address);
#endif

  return status;
}

/***************************************************************************//**
 * Resizes a previously allocated memory block from a specific heap instance.
 *
 * @param[in]  heap   Handle to the heap instance.
 * @param[in]  ptr    Pointer to the allocation to resize. If NULL, behavior
 *                    is same as sl_memory_heap_alloc().
 * @param[in]  size   New size of the block, in bytes. If 0, behavior is same as
 *                    sl_memory_heap_free().
 * @param[out] block  Pointer to variable that will receive the start address
 *                    of the new allocated memory. NULL in case of error condition.
 *
 * @return  SL_STATUS_OK if successful. Error code otherwise.
 *
 * @note (1) If 'ptr' is a null pointer, sl_memory_realloc() is equivalent to
 *           sl_memory_malloc() for the specified 'size'.
 *           If 'size' is 0 and 'ptr' points to an existing block of memory,
 *           sl_memory_realloc() is equivalent to sl_memory_free() and the
 *           memory block is deallocated.
 *
 * @note (2) The content of the reallocated memory block is preserved up to
 *           the lesser of the new and old sizes, even if the block is moved
 *           to a new location. If the new size is larger, the value of the
 *           newly allocated portion is indeterminate.
 ******************************************************************************/
sl_status_t sl_memory_heap_realloc(sl_memory_heap_t *heap,
                                   void *ptr,
                                   size_t size,
                                   void **block)
{
  // Make sure the heap handle isn't NULL.
  EFM_ASSERT(heap != NULL);

#if defined(SL_CATALOG_MEMORY_PROFILER_PRESENT)
  void * volatile return_address = sli_memory_profiler_get_return_address();
#endif
  sl_status_t status = SL_STATUS_OK;
  sli_block_metadata_t *current_block = NULL;
  sli_block_metadata_t *next_block = NULL;
  size_t current_block_len;
  size_t size_real;
  uint32_t reservation_offset;

  // Verify that the block pointer isn't NULL.
  if (block == NULL) {
    return SL_STATUS_NULL_POINTER;
  }

  *block = NULL; // No block allocated yet.

  if (size >= heap->size) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  if ((ptr == NULL) && (size == 0)) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  // Manage special parameters values (see Note #1).
  if (ptr == NULL) {
    status = sl_memory_alloc(size, BLOCK_TYPE_LONG_TERM, block);
#if defined(SL_CATALOG_MEMORY_PROFILER_PRESENT)
    sli_memory_profiler_track_ownership(SLI_INVALID_MEMORY_TRACKER_HANDLE, *block, return_address);
#endif
    return status;
  } else if (size == 0) {
    status = sl_memory_free(ptr);
    return status;
  }

  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_ATOMIC();

  // Get metadata of current block.
  current_block = (sli_block_metadata_t *)((uint8_t *)ptr - SLI_BLOCK_METADATA_SIZE_BYTE);
  current_block_len = SLI_BLOCK_LEN_DWORD_TO_BYTE(sli_block_len_dword_decode(current_block));

  // Adjust size to match the minimum alignment to maximize CPU access performance.
  size_real = SLI_ALIGN_ROUND_UP(size, SLI_BLOCK_ALLOC_MIN_ALIGN);

  // BLOCK EXTENSION.
  if (size_real > current_block_len) {
    bool find_new_block = false;
#if defined(SL_MEMORY_MANAGER_STATISTICS_API_ENABLE) && (SL_MEMORY_MANAGER_STATISTICS_API_ENABLE == 1)
    bool removed_next_metadata = false;
#endif

    reservation_offset = sli_block_offset_next_dword_decode(current_block) - (sli_block_len_dword_decode(current_block) + SLI_BLOCK_METADATA_SIZE_DWORD);

    if ((sli_block_offset_next_dword_decode(current_block) != 0) && (reservation_offset == 0)) {
      next_block = (sli_block_metadata_t *)((uint64_t *)current_block + sli_block_offset_next_dword_decode(current_block));
      int32_t next_block_len_remaining = SLI_BLOCK_LEN_DWORD_TO_BYTE(sli_block_len_dword_decode(next_block)) - (size_real - current_block_len);

      // Verify if next block is free & has room to extend the current block.
      if ((next_block->block_in_use == 0) && (next_block_len_remaining >= 0)) {
        // Decrement bank counters for banks spanning the original allocation.
        // This need to be done because the extension and original size need to count as 1 in the bank counter.
        DECREMENT_BANK_COUNTER(heap, (uint8_t *)current_block, (uint8_t *)ptr + current_block_len - 1);

        // Increment bank counters for banks spanning the new allocation size.
        INCREMENT_BANK_COUNTER(heap, (uint8_t *)current_block, (uint8_t *)ptr + size_real - 1);

        // Remove free block metadata from bank counter as free block will be merged with adjacent block or removed.
        DECREMENT_BANK_COUNTER(heap, (uint8_t*)next_block, (uint8_t*)next_block + SLI_BLOCK_METADATA_SIZE_BYTE - 1);

        if (next_block_len_remaining >= SL_MEMORY_MANAGER_BLOCK_ALLOCATION_MIN_SIZE) {
          // Enough space left in next block to leave a smaller free block.

          // Compute adjusted adjacent free block location.
          sli_block_metadata_t *adjusted_next_block = (sli_block_metadata_t *)((uint8_t *)current_block + SLI_BLOCK_METADATA_SIZE_BYTE + size_real);

          // Update all relevant metadata fields of current block, next block, next next block (if applicable).
          sli_block_len_dword_encode(current_block, SLI_BLOCK_LEN_BYTE_TO_DWORD(size_real));
          sli_block_offset_next_dword_encode(current_block, (sli_block_len_dword_decode(current_block) + SLI_BLOCK_METADATA_SIZE_DWORD));
          sli_memory_metadata_init(adjusted_next_block);
          sli_block_len_dword_encode(adjusted_next_block, SLI_BLOCK_LEN_BYTE_TO_DWORD(next_block_len_remaining));
          sli_block_offset_prev_dword_encode(adjusted_next_block, sli_block_offset_next_dword_decode(current_block));

          // Increment bank counter for new free block metadata.
          INCREMENT_BANK_COUNTER(heap, (uint8_t *)adjusted_next_block, (uint8_t *)adjusted_next_block + SLI_BLOCK_METADATA_SIZE_BYTE - 1);

          if (sli_block_offset_next_dword_decode(next_block) != 0) {
            sli_block_metadata_t *next_next_block = (sli_block_metadata_t *)((uint64_t *)next_block + sli_block_offset_next_dword_decode(next_block));

            // Add reservations offset.
            reservation_offset = sli_block_offset_next_dword_decode(next_block) - sli_block_len_dword_decode(next_block);

            sli_block_offset_next_dword_encode(adjusted_next_block, (sli_block_len_dword_decode(adjusted_next_block) + reservation_offset));
            sli_block_offset_prev_dword_encode(next_next_block, sli_block_offset_next_dword_decode(adjusted_next_block));
          } else {
            sli_block_offset_next_dword_encode(adjusted_next_block, 0); // End of heap
          }

          // Update head pointers accordingly.
          sli_update_free_list_heads(heap, adjusted_next_block, next_block, false);
          // Ensure old next block metadata is invalid.
          sli_memory_metadata_init(next_block);
        } else {
          // Not enough space in next block, simply append all next block to current one
          // by updating all required blocks' metadata.
          heap->free_blocks_number--;
#if defined(SL_MEMORY_MANAGER_STATISTICS_API_ENABLE) && (SL_MEMORY_MANAGER_STATISTICS_API_ENABLE == 1)
          removed_next_metadata = true;
#endif
          sli_block_len_dword_encode(current_block, (sli_block_len_dword_decode(current_block)
                                                     + SLI_BLOCK_METADATA_SIZE_DWORD
                                                     + sli_block_len_dword_decode(next_block)));
          if (sli_block_offset_next_dword_decode(next_block) != 0) {
            sli_block_metadata_t *next_next_block = (sli_block_metadata_t *)((uint64_t *)next_block + sli_block_offset_next_dword_decode(next_block));

            // Add reservations offset.
            reservation_offset = sli_block_offset_next_dword_decode(next_block) - sli_block_len_dword_decode(next_block);

            sli_block_offset_next_dword_encode(current_block, (sli_block_len_dword_decode(current_block) + reservation_offset));
            sli_block_offset_prev_dword_encode(next_next_block, sli_block_offset_next_dword_decode(current_block));
          } else {
            sli_block_offset_next_dword_encode(current_block, 0);   // End of heap
          }

          // Update head pointers accordingly.
          sli_update_free_list_heads(heap, current_block, next_block, true);

          // Ensure old next block metadata is invalid.
          sli_memory_metadata_init(next_block);
        }

        // At this point, current block data payload do not need to be copied. See Note #2.

        // Current block has been extended. Its payload must be returned to the caller.
        *block = ptr;
#if defined(SL_CATALOG_MEMORY_PROFILER_PRESENT)
        sli_memory_profiler_track_realloc(sli_mm_heap_name,
                                          (uint8_t *)ptr - SLI_BLOCK_METADATA_SIZE_BYTE,
                                          (uint8_t *)ptr - SLI_BLOCK_METADATA_SIZE_BYTE,
                                          size_real + SLI_BLOCK_METADATA_SIZE_BYTE);
#endif
      } else {
        // Next block cannot fulfill the extension. Get a new one from the heap.
        find_new_block = true;
      }
    } else {
      // Current block is the last heap's block. No possible adjacent extension.
      // Get a new one from the heap.
      find_new_block = true;
    }

    if (find_new_block == true) {
      // Allocate a new block.
      status = sl_memory_alloc(size_real, BLOCK_TYPE_LONG_TERM, block);
      if (status != SL_STATUS_OK) {
        CORE_EXIT_ATOMIC();
        return status;
      }

      // Copy data from current block to new block. See Note #2.
      memcpy(*block, ptr, current_block_len);

#if defined(SL_CATALOG_MEMORY_PROFILER_PRESENT)
      sli_memory_profiler_track_realloc(sli_mm_heap_name,
                                        (uint8_t *)ptr - SLI_BLOCK_METADATA_SIZE_BYTE,
                                        (uint8_t *)*block - SLI_BLOCK_METADATA_SIZE_BYTE,
                                        size_real + SLI_BLOCK_METADATA_SIZE_BYTE);
#endif

      // Free current block. Reallocated block is different from the current one.
      status = sl_memory_free(ptr);
      if (status != SL_STATUS_OK) {
        CORE_EXIT_ATOMIC();
        return status;
      }
    }
#if defined(SL_MEMORY_MANAGER_STATISTICS_API_ENABLE) && (SL_MEMORY_MANAGER_STATISTICS_API_ENABLE == 1)
    if (find_new_block == false) {
      size_t new_len = SLI_BLOCK_LEN_DWORD_TO_BYTE(sli_block_len_dword_decode(current_block));
      SLI_MEMORY_STAT_HEAP_INCREASE(heap, (new_len - current_block_len));
      if (removed_next_metadata) {
        // The metadata of the adjacent free block was absorbed into the allocation.
        // It no longer exists as overhead, so remove it from used_size.
        SLI_MEMORY_STAT_HEAP_DECREASE(heap, SLI_BLOCK_METADATA_SIZE_BYTE);     
      }
    }
#endif

    // BLOCK REDUCTION.
  } else if (size_real < current_block_len) {
    size_t current_block_remaining_len = current_block_len - size_real;
    bool create_new_block = false;
#if defined(SL_MEMORY_MANAGER_STATISTICS_API_ENABLE) && (SL_MEMORY_MANAGER_STATISTICS_API_ENABLE == 1)
    bool reduced_block = false;
    bool added_free_metadata = false;
#endif
    // Calculate reservation offset.
    reservation_offset = current_block->offset_neighbour_next - (current_block->length + SLI_BLOCK_METADATA_SIZE_DWORD);

    // Decrement bank counters for banks spanning the original allocation.
    // This need to be done because we need to remove any increments from the bank counters
    // to accurately represent the new counter.
    DECREMENT_BANK_COUNTER(heap, (uint8_t *)current_block, (uint8_t *)ptr + current_block_len - 1);

    // Re-increment bank counters based on the new allocation size.
    INCREMENT_BANK_COUNTER(heap, (uint8_t *)current_block, (uint8_t *)ptr + size_real - 1);

    if (sli_block_offset_next_dword_decode(current_block) != 0) {
      next_block = (sli_block_metadata_t *)((uint64_t *)current_block + sli_block_offset_next_dword_decode(current_block));

      // Verify if next block is free to merge the newly unallocated portion of the current block.
      if (next_block->block_in_use == 0 && reservation_offset == 0) {
        // Compute adjusted adjacent free block location.
        sli_block_metadata_t *adjusted_next_block = (sli_block_metadata_t *)((uint8_t *)current_block + SLI_BLOCK_METADATA_SIZE_BYTE + size_real);

        // Update all relevant metadata fields of current block, next block, next next block (if applicable).
        sli_block_len_dword_encode(current_block, SLI_BLOCK_LEN_BYTE_TO_DWORD(size_real));
#if defined(SL_MEMORY_MANAGER_STATISTICS_API_ENABLE) && (SL_MEMORY_MANAGER_STATISTICS_API_ENABLE == 1)
        reduced_block = true;
#endif
        sli_block_offset_next_dword_encode(current_block, (sli_block_len_dword_decode(current_block) + SLI_BLOCK_METADATA_SIZE_DWORD));
        sli_memory_metadata_init(adjusted_next_block);
        sli_block_len_dword_encode(adjusted_next_block, (SLI_BLOCK_LEN_BYTE_TO_DWORD(current_block_remaining_len)
                                                         + sli_block_len_dword_decode(next_block)));
        sli_block_offset_prev_dword_encode(adjusted_next_block, sli_block_offset_next_dword_decode(current_block));

        // Remove free block metadata from bank counter as free block is merged with previous block.
        DECREMENT_BANK_COUNTER(heap, (uint8_t*)next_block, (uint8_t*)next_block + SLI_BLOCK_METADATA_SIZE_BYTE - 1);

        // Increment bank counter for new free block metadata.
        INCREMENT_BANK_COUNTER(heap, (uint8_t *)adjusted_next_block, (uint8_t *)adjusted_next_block + SLI_BLOCK_METADATA_SIZE_BYTE - 1);

        if (sli_block_offset_next_dword_decode(next_block) != 0) {
          sli_block_metadata_t *next_next_block = (sli_block_metadata_t *)((uint64_t *)next_block + sli_block_offset_next_dword_decode(next_block));

          // Next block reservation offset.
          uint32_t next_reservation_offset = sli_block_offset_next_dword_decode(next_block) - (sli_block_len_dword_decode(next_block) + SLI_BLOCK_METADATA_SIZE_DWORD);

          sli_block_offset_next_dword_encode(adjusted_next_block, (sli_block_len_dword_decode(adjusted_next_block) + SLI_BLOCK_METADATA_SIZE_DWORD + next_reservation_offset));
          sli_block_offset_prev_dword_encode(next_next_block, sli_block_offset_next_dword_decode(adjusted_next_block));
        } else {
          sli_block_offset_next_dword_encode(adjusted_next_block, 0);   // End of heap
        }

        // Update head pointers accordingly.
        sli_update_free_list_heads(heap, adjusted_next_block, next_block, false);

        // Ensure old next block metadata is invalid.
        sli_memory_metadata_init(next_block);
      } else {
        // Next block is in use and cannot be merged with the newly unallocated portion.
        create_new_block = true;
      }
    } else {
      // Current block is the last heap's block. No possible adjacent block to merge with the newly unallocated portion.
      create_new_block = true;
    }

    // Try to create a new free block in the unallocated portion of the current block.
    if (create_new_block == true) {
      if (current_block_remaining_len >= SLI_BLOCK_ALLOCATION_MIN_SIZE) {
        // Compute adjusted adjacent free block location.
        sli_block_metadata_t *adjusted_next_block = (sli_block_metadata_t *)((uint8_t *)current_block + SLI_BLOCK_METADATA_SIZE_BYTE + size_real);

        // Update all relevant metadata fields of current block, next block, next next block (if applicable).
        sli_block_len_dword_encode(current_block, SLI_BLOCK_LEN_BYTE_TO_DWORD(size_real));
#if defined(SL_MEMORY_MANAGER_STATISTICS_API_ENABLE) && (SL_MEMORY_MANAGER_STATISTICS_API_ENABLE == 1)        
        reduced_block = true;
#endif
        sli_block_offset_next_dword_encode(current_block, (sli_block_len_dword_decode(current_block) + SLI_BLOCK_METADATA_SIZE_DWORD));
        sli_memory_metadata_init(adjusted_next_block);
        sli_block_len_dword_encode(adjusted_next_block, SLI_BLOCK_LEN_BYTE_TO_DWORD(current_block_remaining_len - SLI_BLOCK_METADATA_SIZE_BYTE));
        sli_block_offset_prev_dword_encode(adjusted_next_block, sli_block_offset_next_dword_decode(current_block));

        // Increment bank counter for new free block metadata.
        INCREMENT_BANK_COUNTER(heap, (uint8_t *)adjusted_next_block, (uint8_t *)adjusted_next_block + SLI_BLOCK_METADATA_SIZE_BYTE - 1);

        if (next_block != NULL) {
          sli_block_offset_next_dword_encode(adjusted_next_block, (sli_block_len_dword_decode(adjusted_next_block) + SLI_BLOCK_METADATA_SIZE_DWORD + reservation_offset));
          sli_block_offset_prev_dword_encode(next_block, sli_block_offset_next_dword_decode(adjusted_next_block));
        } else {
          sli_block_offset_next_dword_encode(adjusted_next_block, 0);   // End of heap
        }
#if defined(SL_MEMORY_MANAGER_STATISTICS_API_ENABLE) && (SL_MEMORY_MANAGER_STATISTICS_API_ENABLE == 1)
        added_free_metadata = true;
#endif
        heap->free_blocks_number++;
        // Update head pointers accordingly.
        sli_update_free_list_heads(heap, adjusted_next_block, NULL, false);
      } else {
        // Not enough space in current block remaining area to create a new free block.
        // consider the current block unallocated portion as lost for now until the current block is freed.
        // In that case, no need to update the current block metadata. All remains the same.
      }
    }

    // Current block has been reduced. Its payload must be returned to the caller.
    *block = ptr;
#if defined(SL_CATALOG_MEMORY_PROFILER_PRESENT)
    sli_memory_profiler_track_realloc(sli_mm_heap_name,
                                      (uint8_t *)ptr - SLI_BLOCK_METADATA_SIZE_BYTE,
                                      (uint8_t *)ptr - SLI_BLOCK_METADATA_SIZE_BYTE,
                                      size_real + SLI_BLOCK_METADATA_SIZE_BYTE);
#endif
#if defined(SL_MEMORY_MANAGER_STATISTICS_API_ENABLE) && (SL_MEMORY_MANAGER_STATISTICS_API_ENABLE == 1)
if (reduced_block) {
  SLI_MEMORY_STAT_HEAP_DECREASE(heap, (current_block_len - size_real));
  if (added_free_metadata) {
    // A new free block metadata was created when splitting the reduced block.
    SLI_MEMORY_STAT_HEAP_INCREASE(heap, SLI_BLOCK_METADATA_SIZE_BYTE);
  }
}
#endif
  } else {
    // If the size requested does not provoke a block extension or reduction, consider no error.
    // And return the same given address. We still track it to show that resize was requested.
    *block = ptr;
#if defined(SL_CATALOG_MEMORY_PROFILER_PRESENT)
    sli_memory_profiler_track_realloc(sli_mm_heap_name,
                                      (uint8_t *)ptr - SLI_BLOCK_METADATA_SIZE_BYTE,
                                      (uint8_t *)ptr - SLI_BLOCK_METADATA_SIZE_BYTE,
                                      size_real + SLI_BLOCK_METADATA_SIZE_BYTE);
#endif
  }

  CORE_EXIT_ATOMIC();

#if defined(SL_CATALOG_MEMORY_PROFILER_PRESENT)
  sli_memory_profiler_track_ownership(SLI_INVALID_MEMORY_TRACKER_HANDLE, *block, return_address);
#endif

  return status;
}

/***************************************************************************//**
 * Populates an sl_memory_heap_info_t{} structure with the current status of
 * a specified heap instance.
 ******************************************************************************/
sl_status_t sl_memory_heap_get_info(const sl_memory_heap_t *heap,
                                    sl_memory_heap_info_t *heap_info)
{
#if defined(SL_MEMORY_MANAGER_STATISTICS_API_ENABLE) && (SL_MEMORY_MANAGER_STATISTICS_API_ENABLE == 1)
  sli_block_metadata_t *block_metadata = (sli_block_metadata_t *)heap->base_addr;
  bool compute = true;
  size_t block_len_dw = 0u;
  size_t remaining_size = 0u;
  size_t free_block_count = 0u;
  size_t used_block_count = 0u;
  size_t largest_free_size = 0u;
  size_t smallest_free_size = SIZE_MAX;
  size_t largest_used_size = 0u;
  size_t smallest_used_size = SIZE_MAX;
  size_t used_bank_count = 0u;
  size_t total_bank = 0u;

  if (heap_info == NULL) {
    return SL_STATUS_NULL_POINTER;
  }

  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_ATOMIC();

  do {
    block_len_dw = sli_block_len_dword_decode(block_metadata);
    // Calculate the smallest and largest used size and the remaining heap sizes.
    if (block_metadata->block_in_use == 0) {
      remaining_size += SLI_BLOCK_LEN_DWORD_TO_BYTE(block_len_dw);
      largest_free_size = SL_MAX(SLI_BLOCK_LEN_DWORD_TO_BYTE(block_len_dw), largest_free_size);
      smallest_free_size = SL_MIN(SLI_BLOCK_LEN_DWORD_TO_BYTE(block_len_dw), smallest_free_size);
      free_block_count++;
    } else {
      largest_used_size = SL_MAX(SLI_BLOCK_LEN_DWORD_TO_BYTE(block_len_dw), largest_used_size);
      smallest_used_size = SL_MIN(SLI_BLOCK_LEN_DWORD_TO_BYTE(block_len_dw), smallest_used_size);
      used_block_count++;
    }

    // If no used block, set the smallest used size to 0.
    if (smallest_used_size == SIZE_MAX) {
      smallest_used_size = 0u;
    }

    // If no free block, set the smallest free size to 0.
    if (smallest_free_size == SIZE_MAX) {
      smallest_free_size = 0u;
    }

    // Get the next block.
    if (sli_block_offset_next_dword_decode(block_metadata) == 0) {
      compute = false;
    } else {
      block_metadata = (sli_block_metadata_t *)((uint64_t *)block_metadata + sli_block_offset_next_dword_decode(block_metadata));
    }
  } while (compute);

#if defined(SL_CATALOG_BANK_RETENTION_CONTROL_PRESENT)
  sli_retention_control_t *retention_control = (sli_retention_control_t *)heap->retention_control;

  // Get ID of banks limit.
  uint32_t heap_start_bank_id = sli_memory_manager_get_bank_id_by_addr(heap, heap->base_addr);
  uint32_t heap_end_bank_id = sli_memory_manager_get_bank_id_by_addr(heap, (void *)((uint8_t *)heap->base_addr + heap->size - 1));

  for (uint32_t id = heap_start_bank_id; id <= heap_end_bank_id; id++) {
    if (retention_control->banks_counter[id] > 0) {
      used_bank_count++;
      __NOP();
    }
    total_bank++;
  }
#endif

  CORE_EXIT_ATOMIC();

  heap_info->base_addr = (uint32_t)heap->base_addr;
  heap_info->total_size = heap->size;
  heap_info->used_size = heap->used_size;
  heap_info->free_size = remaining_size;
  heap_info->free_block_count = free_block_count;
  heap_info->free_block_largest_size = largest_free_size;
  heap_info->free_block_smallest_size = smallest_free_size;
  heap_info->used_block_count = used_block_count;
  heap_info->used_block_largest_size = largest_used_size;
  heap_info->used_block_smallest_size = smallest_used_size;
  heap_info->used_bank_count = used_bank_count;
  heap_info->total_bank = total_bank;

#else
  (void) heap;
  (void) heap_info;
#endif

  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Retrieves the total size of a specified heap instance.
 ******************************************************************************/
size_t sl_memory_heap_get_total_size(const sl_memory_heap_t *heap)
{
#if defined(SL_MEMORY_MANAGER_STATISTICS_API_ENABLE) && (SL_MEMORY_MANAGER_STATISTICS_API_ENABLE == 1)
  return heap->size;
#else
  (void) heap;
  return 0;
#endif
}

/***************************************************************************//**
 * Retrieves the current amount of free memory in a specified heap instance.
 ******************************************************************************/
size_t sl_memory_heap_get_free_size(const sl_memory_heap_t *heap)
{
#if defined(SL_MEMORY_MANAGER_STATISTICS_API_ENABLE) && (SL_MEMORY_MANAGER_STATISTICS_API_ENABLE == 1)
  sl_memory_heap_info_t heap_info;

  sl_memory_heap_get_info(heap, &heap_info);

  return heap_info.free_size;
#else
  (void) heap;
  return 0;
#endif
}

/***************************************************************************//**
 * Retrieves the current amount of memory used in a specified heap instance.
 ******************************************************************************/
size_t sl_memory_heap_get_used_size(const sl_memory_heap_t *heap)
{
  size_t heap_used_size_value = 0;

#if defined(SL_MEMORY_MANAGER_STATISTICS_API_ENABLE) && (SL_MEMORY_MANAGER_STATISTICS_API_ENABLE == 1)
  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_ATOMIC();
  heap_used_size_value = heap->used_size;
  CORE_EXIT_ATOMIC();
#else
  (void) heap;
#endif

  return heap_used_size_value;
}

/***************************************************************************//**
 * Retrieves a specified heap instance's high watermark.
 ******************************************************************************/
size_t sl_memory_heap_get_high_watermark(const sl_memory_heap_t *heap)
{
  size_t heap_high_watermark_value = 0;

#if defined(SL_MEMORY_MANAGER_STATISTICS_API_ENABLE) && (SL_MEMORY_MANAGER_STATISTICS_API_ENABLE == 1)
  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_ATOMIC();
  heap_high_watermark_value = heap->high_watermark;
  CORE_EXIT_ATOMIC();
#else
  (void) heap;
#endif

  return heap_high_watermark_value;
}

/***************************************************************************//**
 * Resets a specified heap instance's high watermark to the current heap used.
 ******************************************************************************/
void sl_memory_heap_reset_high_watermark(sl_memory_heap_t *heap)
{
#if defined(SL_MEMORY_MANAGER_STATISTICS_API_ENABLE) && (SL_MEMORY_MANAGER_STATISTICS_API_ENABLE == 1)
  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_ATOMIC();
  heap->high_watermark = heap->used_size;
  CORE_EXIT_ATOMIC();
#else
  (void) heap;
#endif
}

/*******************************************************************************
 ***************************   LOCAL FUNCTIONS   *******************************
 ******************************************************************************/

/***************************************************************************//**
 * Manages the required data alignment by moving the non-aligned block payload
 * to the closest aligned location.
 *
 * @param[in]  current_block_metadata   Pointer to block whose payload start
 *                                      address must be moved to be aligned.
 *
 * @param[in]  block_align              Alignment required, in bytes.
 *
 * @return     Pointer to the new block with the correct alignment.
 ******************************************************************************/
static sli_block_metadata_t *memory_manage_data_alignment(sl_memory_heap_t *heap,
                                                          sli_block_metadata_t *current_block_metadata,
                                                          size_t block_align)
{
  void *data_payload = (void *)((uint8_t *)current_block_metadata + SLI_BLOCK_METADATA_SIZE_BYTE);
  sli_block_metadata_t *old_block_metadata = current_block_metadata;
  size_t align_offset;

  // Align payload to the correct alignment.
  data_payload = (void *)SLI_ALIGN_ROUND_UP(((uintptr_t)data_payload), block_align);

  // Get the new metadata location and update all relevant fields.
  current_block_metadata = (sli_block_metadata_t *)((uint8_t *)data_payload - SLI_BLOCK_METADATA_SIZE_BYTE);
  sli_memory_metadata_init(current_block_metadata);
  align_offset = (size_t)((uint64_t *)current_block_metadata - (uint64_t *)old_block_metadata);
  sli_block_len_dword_encode(current_block_metadata, (sli_block_len_dword_decode(old_block_metadata) - align_offset));

  size_t block_offset_prev_dw = sli_block_offset_prev_dword_decode(old_block_metadata) + align_offset;
  sli_block_offset_prev_dword_encode(current_block_metadata, block_offset_prev_dw);
  if (sli_block_offset_prev_dword_decode(old_block_metadata) != 0) {
    sli_block_metadata_t *prev_block = (sli_block_metadata_t *)((uint64_t *)old_block_metadata - sli_block_offset_prev_dword_decode(old_block_metadata));
    size_t block_len_dw = sli_block_len_dword_decode(prev_block);

    sli_block_offset_next_dword_encode(prev_block, sli_block_offset_prev_dword_decode(current_block_metadata));

    // Merge lost space because of the alignment into the previous block. It helps to keep
    // all computations in malloc()/free() valid. For ST split block, the lost space is back into
    // a free block space.
    sli_block_len_dword_encode(prev_block, (block_len_dw + align_offset));
  } else {
    // Special case where the block data payload being aligned is at the heap start. A special flag in the block metadata
    // is used to identify this special block in sl_memory_free() and accordingly perform the merge with previous adjacent block.
    current_block_metadata->heap_start_align = true;
  }

  if (sli_block_offset_next_dword_decode(old_block_metadata) != 0) {
    sli_block_metadata_t *next_block = (sli_block_metadata_t *)((uint64_t *)old_block_metadata + sli_block_offset_next_dword_decode(old_block_metadata));

    sli_block_offset_next_dword_encode(current_block_metadata, (sli_block_offset_next_dword_decode(old_block_metadata) - align_offset));
    sli_block_offset_prev_dword_encode(next_block, sli_block_offset_next_dword_decode(current_block_metadata));
  } else {
    sli_block_offset_next_dword_encode(current_block_metadata, 0);
  }

#if defined(SL_MEMORY_MANAGER_STATISTICS_API_ENABLE) && (SL_MEMORY_MANAGER_STATISTICS_API_ENABLE == 1)
  size_t align_offset_bytes = SLI_BLOCK_LEN_DWORD_TO_BYTE(align_offset);
  if (sli_block_offset_prev_dword_decode(old_block_metadata) == 0) {
    // Heap-start alignment consumes bytes that are unavailable until the aligned block is freed.
    SLI_MEMORY_STAT_HEAP_INCREASE(heap, align_offset_bytes);
  } else {
    sli_block_metadata_t *prev_block = (sli_block_metadata_t *)((uint64_t *)old_block_metadata
                                                              - sli_block_offset_prev_dword_decode(old_block_metadata));
    // If the previous block is already in use, the alignment padding becomes internal fragmentation
    // and must be counted as used. If the previous block is free, the padding is merged back into
    // the free space and should not affect used_size.
    if (prev_block->block_in_use) {
      SLI_MEMORY_STAT_HEAP_INCREASE(heap, align_offset_bytes);
    }
  }
#else
  (void) heap;
#endif

  return current_block_metadata;
}

#if defined(SLI_MEMORY_MANAGER_SUPPORT_ALLOCATION_FALLBACK)
/***************************************************************************//**
 * Manages allocation fallback into different heap types.
 *
 * @param[in]  size                     Size of the block to allocate in bytes.
 * @param[in]  align                    Alignment required, in bytes.
 * @param[in]  type                     Block type and attribute mask.
 * @param[out] block                    Pointer to variable that will receive
 *                                      the start address of the allocated memory.
 *
 * @return     SL_STATUS_OK if success.
 *             SL_STATUS_NO_MORE_RESOURCE if no fallbacks are available.
 *             Allocation error code otherwise.
 *
 * @note (1) This function is used to handle allocation fallbacks when the
 *           requested allocation cannot be satisfied by the current heap.
 *           It tries to allocate from different heaps based on the specified
 *           block attributes.
 *
 * @note (2) The fallback order follows the heaps initialization order which is:
 *             1. General Purpose Heap
 *             2. PSRAM
 *             3. DTCM
 *
 *           This  makes it so that:
 *             Failed allocations from the general purpose heap always goes first to the
 *             PSRAM and then to the DTCM.
 ******************************************************************************/
static sl_status_t memory_manage_allocation_fallback(size_t size,
                                                     size_t align,
                                                     uint8_t type,
                                                     void **block)
{
  sl_memory_heap_t *fallback_heap = sli_general_purpose_heap.next_handle;
  sl_memory_block_attrib_t blocks_heap_attribute = (sl_memory_block_attrib_t) (type & SLI_MEMORY_BLOCK_ATTRIBUTE_MASK);
  size_t allocation_size = SLI_ALIGN_ROUND_UP(size, SLI_BLOCK_ALLOC_MIN_ALIGN);

  while (fallback_heap != NULL) {
    // If the fallback heap attribute matches one of the blocks heap attributes
    // and there is enough space in heap, try the allocation.
    if (((fallback_heap->attrib & blocks_heap_attribute) == fallback_heap->attrib)
        && (allocation_size <= (fallback_heap->size - fallback_heap->used_size))) {
      sl_status_t status = sl_memory_heap_alloc_advanced(fallback_heap,
                                                         size,
                                                         align,
                                                         type & ~SLI_MEMORY_BLOCK_ATTRIBUTE_MASK,  // No fallback on this allocation
                                                         block);
      if (status == SL_STATUS_OK) {
        return SL_STATUS_OK;
      }
    }

    fallback_heap = fallback_heap->next_handle;
  }

  return SL_STATUS_NO_MORE_RESOURCE;
}
#endif  // SLI_MEMORY_MANAGER_SUPPORT_ALLOCATION_FALLBACK

#if defined(SL_CATALOG_MEMORY_MANAGER_DTCM_PRESENT)
/***************************************************************************//**
 * Gets the DTCM heap handle.
 ******************************************************************************/
SL_CODE_CLASSIFY(SL_CODE_COMPONENT_MEMORY_MANAGER, SL_CODE_CLASS_TIME_CRITICAL)
sl_memory_heap_t *sl_memory_manager_get_dtcm_heap(void)
{
  return &sli_dtcm_heap;
}
#endif

#if defined(SL_CATALOG_MEMORY_MANAGER_PSRAM_PRESENT)
/***************************************************************************//**
 * Gets the PSRAM heap handle.
 ******************************************************************************/
SL_CODE_CLASSIFY(SL_CODE_COMPONENT_MEMORY_MANAGER, SL_CODE_CLASS_TIME_CRITICAL)
sl_memory_heap_t *sl_memory_manager_get_psram_heap(void)
{
  return &sli_psram_heap;
}
#endif
