/***************************************************************************//**
 * @file
 * @brief This is a simple wrapper for the stdlib memory management functions
 *   like malloc, calloc, realloc and free in order to redirect memory_manager API
 *   to stdlib implementation.
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

#include <stdlib.h>
#if defined(SL_COMPONENT_CATALOG_PRESENT)
#include "sl_component_catalog.h"
#endif

#if defined(SL_CATALOG_SI91X_SOC_MODE_PRESENT)
#include "sl_si91x_memory_manager.h"
#endif

#include "sl_memory_manager.h"

#include "sl_core.h"
#include "sl_status.h"

/*******************************************************************************
 ****************************   GLOBAL VARIABLES   *****************************
 ******************************************************************************/

sl_memory_heap_t sli_general_purpose_heap;

/***************************************************************************//**
 * Initializes the memory manager.
 ******************************************************************************/
sl_status_t sl_memory_init(void)
{
  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Allocates a memory block of at least requested size from the heap. Simple
 * version.
 *
 * This function will be redirected to the 'malloc()' function.
 ******************************************************************************/
void *sl_malloc(size_t size)
{
  void *block_avail;
  CORE_DECLARE_IRQ_STATE;

  CORE_ENTER_ATOMIC();

  block_avail = malloc(size);

  CORE_EXIT_ATOMIC();

  return block_avail;
}

/***************************************************************************//**
 * Dynamically allocates a block of memory. Advanced version that allows to
 * specify alignment.
 *
 * This function will be redirected to the standard 'malloc()' function.
 ******************************************************************************/
sl_status_t sl_memory_alloc_advanced(size_t size,
                                     size_t align,
                                     uint8_t type,
                                     void **block)
{
  (void)type;

  if (block == NULL) {
    return SL_STATUS_NULL_POINTER;
  }

  // No block allocated yet.
  *block = NULL;

  if ((size == 0) || (align == 0)) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_ATOMIC();

  *block = malloc(size);

  CORE_EXIT_ATOMIC();

  if (*block == NULL) {
    return SL_STATUS_ALLOCATION_FAILED;
  }

  if ((align != SL_MEMORY_BLOCK_ALIGN_DEFAULT) && (((uintptr_t)*block % (uintptr_t)align) != 0)) {
    // Deallocate *block.
    sl_free(*block);
    *block = NULL;

    return SL_STATUS_INVALID_PARAMETER;
  }

  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Allocates a memory block from a specific heap instance.
 * Advanced version that allows to specify alignment.
 ******************************************************************************/
sl_status_t sl_memory_heap_alloc_advanced(sl_memory_heap_t *heap,
                                          size_t size,
                                          size_t align,
                                          uint8_t type,
                                          void **block)
{
  (void) heap;
  return sl_memory_alloc_advanced(size, align, type, block);
}

/***************************************************************************//**
 * Dynamically allocates a block of memory.
 *
 * This function simplifies memory allocation by calling 'sl_memory_alloc_advanced()'
 * with alignment set to 8 bytes.
 ******************************************************************************/
sl_status_t sl_memory_alloc(size_t size,
                            uint8_t type,
                            void **block)
{
  return sl_memory_alloc_advanced(size, SL_MEMORY_BLOCK_ALIGN_DEFAULT, type, block);
}

/***************************************************************************//**
 * Allocates a memory block from a specific heap instance.
 ******************************************************************************/
sl_status_t sl_memory_heap_alloc(sl_memory_heap_t *heap,
                                 size_t size,
                                 uint8_t type,
                                 void **block)
{
  (void) heap;
  return sl_memory_alloc(size, type, block);
}

/***************************************************************************//**
 * Dynamically allocates a block of memory cleared to 0. Simple version.
 *
 * This function will be redirected to the standard 'calloc()' function.
 ******************************************************************************/
void *sl_calloc(size_t item_count,
                size_t size)
{
  void *block_avail = NULL;

  (void)sl_memory_calloc(item_count, size, BLOCK_TYPE_LONG_TERM, &block_avail);

  return block_avail;
}

/***************************************************************************//**
 * Dynamically allocates a block of memory cleared to 0.
 *
 * This function will be redirected to the standard 'calloc()' function.
 ******************************************************************************/
sl_status_t sl_memory_calloc(size_t item_count,
                             size_t size,
                             uint8_t type,
                             void **block)
{
  (void)type;
  CORE_DECLARE_IRQ_STATE;

  // Verify that the block pointer isn't NULL.
  if (block == NULL) {
    return SL_STATUS_NULL_POINTER;
  }

  *block = NULL; // No block allocated yet.

  if ((size == 0) || (item_count == 0)) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  CORE_ENTER_ATOMIC();

  *block = calloc(item_count, size);

  CORE_EXIT_ATOMIC();

  if (*block == NULL) {
    return SL_STATUS_ALLOCATION_FAILED;
  }

  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Allocates a memory block cleared to 0 from a specific heap instance.
 ******************************************************************************/
sl_status_t sl_memory_heap_calloc(sl_memory_heap_t *heap,
                                  size_t item_count,
                                  size_t size,
                                  uint8_t type,
                                  void **block)
{
  (void) heap;
  return sl_memory_calloc(item_count, size, type, block);
}

/***************************************************************************//**
 * Resizes a previously allocated memory block. Simple version.
 *
 * This function will be redirected to the standard 'realloc()' function.
 ******************************************************************************/
void *sl_realloc(void *ptr, size_t size)
{
  void *block_avail = NULL;

  (void)sl_memory_realloc(ptr, size, &block_avail);

  return block_avail;
}

/***************************************************************************//**
 * Resizes a previously allocated memory block.
 *
 * This function will be redirected to the standard 'realloc()' function.
 ******************************************************************************/
sl_status_t sl_memory_realloc(void *ptr,
                              size_t size,
                              void **block)
{
  (void)block;
  CORE_DECLARE_IRQ_STATE;

  // Verify that the block pointer isn't NULL.
  if (block == NULL) {
    return SL_STATUS_NULL_POINTER;
  }

  *block = NULL; // No block allocated yet.

  if ((ptr == NULL) && (size == 0)) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  CORE_ENTER_ATOMIC();

  *block = realloc(ptr, size);

  CORE_EXIT_ATOMIC();

  if (*block == NULL) {
    return SL_STATUS_ALLOCATION_FAILED;
  }

  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Resizes a previously allocated memory block from a specific heap instance.
 ******************************************************************************/
sl_status_t sl_memory_heap_realloc(sl_memory_heap_t *heap,
                                   void *ptr,
                                   size_t size,
                                   void **block)
{
  (void) heap;
  return sl_memory_realloc(ptr, size, block);
}

/***************************************************************************//**
 * Frees a dynamically allocated block of memory.
 *
 * This function will be redirected to the standard 'free()' function.
 ******************************************************************************/
sl_status_t sl_memory_free(void *ptr)
{
  CORE_DECLARE_IRQ_STATE;

  if (ptr == NULL) {
    return SL_STATUS_NULL_POINTER;
  }

  CORE_ENTER_ATOMIC();

  free(ptr);

  CORE_EXIT_ATOMIC();

  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Frees a previously allocated block from a specific heap instance.
 ******************************************************************************/
sl_status_t sl_memory_heap_free(sl_memory_heap_t *heap,
                                void *block)
{
  (void) heap;
  return sl_memory_free(block);
}

/***************************************************************************//**
 * Frees a previously allocated block back into the heap. Simple version.
 *
 * This function will be redirected to the 'free()' function.
 ******************************************************************************/
void sl_free(void *ptr)
{
  (void)sl_memory_free(ptr);
}

/***************************************************************************//**
 * Populates an sl_memory_heap_info_t{} structure with the current status of
 * the heap.
 *
 * This function is a stub for the memory manager redirect component.
 ******************************************************************************/
sl_status_t sl_memory_get_heap_info(sl_memory_heap_info_t *heap_info)
{
  if (heap_info == NULL) {
    return SL_STATUS_NULL_POINTER;
  }

  // Stub implementation: Set all fields to zero.
  heap_info->base_addr = 0;
  heap_info->used_size = 0;
  heap_info->free_size = 0;
  heap_info->total_size = 0;
  heap_info->free_block_count = 0;
  heap_info->free_block_largest_size = 0;
  heap_info->free_block_smallest_size = 0;
  heap_info->used_block_count = 0;
  heap_info->used_block_largest_size = 0;
  heap_info->used_block_smallest_size = 0;

  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Populates an sl_memory_heap_info_t{} structure with the current status of
 * the heap.
 *
 * This function is a stub for the memory manager redirect component.
 ******************************************************************************/
sl_status_t sl_memory_heap_get_info(const sl_memory_heap_t *heap,
                                    sl_memory_heap_info_t *heap_info)
{
  (void) heap;
  return sl_memory_get_heap_info(heap_info);
}

/***************************************************************************//**
 * Gets the total size of the heap.
 *
 * This function is a stub for the memory manager redirect component.
 ******************************************************************************/
size_t sl_memory_get_total_heap_size(void)
{
  // Stub implementation: Return zero.
  return 0;
}

/***************************************************************************//**
 * Gets the total size of the heap.
 *
 * This function is a stub for the memory manager redirect component.
 ******************************************************************************/
size_t sl_memory_heap_get_total_size(const sl_memory_heap_t *heap)
{
  (void) heap;
  return sl_memory_get_total_heap_size();
}

/***************************************************************************//**
 * Gets the current free heap size.
 *
 * This function is a stub for the memory manager redirect component.
 ******************************************************************************/
size_t sl_memory_get_free_heap_size(void)
{
#if defined(SL_CATALOG_SI91X_SOC_MODE_PRESENT)
  return sl_si91x_memory_get_free_heap_size();
#else
  // Stub implementation: Return zero.
  return 0;
#endif
}

/***************************************************************************//**
 * Gets the current free heap size.
 *
 * This function is a stub for the memory manager redirect component.
 ******************************************************************************/
size_t sl_memory_heap_get_free_size(const sl_memory_heap_t *heap)
{
  (void) heap;
  return sl_memory_get_free_heap_size();
}

/***************************************************************************//**
 * Gets the current used heap size.
 *
 * This function is a stub for the memory manager redirect component.
 ******************************************************************************/
size_t sl_memory_get_used_heap_size(void)
{
#if defined(SL_CATALOG_SI91X_SOC_MODE_PRESENT)
  return sl_si91x_memory_get_used_heap_size();
#else
  // Stub implementation: Return zero.
  return 0;
#endif
}

/***************************************************************************//**
 * Gets the current used heap size.
 *
 * This function is a stub for the memory manager redirect component.
 ******************************************************************************/
size_t sl_memory_heap_get_used_size(const sl_memory_heap_t *heap)
{
  (void) heap;
  return sl_memory_get_used_heap_size();
}

/***************************************************************************//**
 * Gets heap high watermark.
 *
 * This function is a stub for the memory manager redirect component.
 ******************************************************************************/
size_t sl_memory_get_heap_high_watermark(void)
{
#if defined(SL_CATALOG_SI91X_SOC_MODE_PRESENT)
  return sl_si91x_memory_get_heap_high_watermark();
#else
  // Stub implementation: Return zero.
  return 0;
#endif
}

/***************************************************************************//**
 * Gets heap high watermark.
 *
 * This function is a stub for the memory manager redirect component.
 ******************************************************************************/
size_t sl_memory_heap_get_high_watermark(const sl_memory_heap_t *heap)
{
  (void) heap;
  return sl_memory_get_heap_high_watermark();
}

/***************************************************************************//**
 * Reset heap high watermark to the current heap used.
 *
 * This function is a stub for the memory manager redirect component.
 ******************************************************************************/
void sl_memory_reset_heap_high_watermark(void)
{
#if defined(SL_CATALOG_SI91X_SOC_MODE_PRESENT)
  sl_si91x_memory_reset_heap_high_watermark();
#else
  // Stub implementation: No operation performed.
  (void)0;
#endif
}

/***************************************************************************//**
 * Reset heap high watermark to the current heap used.
 *
 * This function is a stub for the memory manager redirect component.
 ******************************************************************************/
void sl_memory_heap_reset_high_watermark(sl_memory_heap_t *heap)
{
  (void) heap;
  sl_memory_reset_heap_high_watermark();
}
