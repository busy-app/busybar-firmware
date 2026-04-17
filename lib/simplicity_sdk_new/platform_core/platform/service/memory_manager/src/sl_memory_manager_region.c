/***************************************************************************//**
 * @file
 * @brief Getters for Heap and stack
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
#include <stdint.h>
#include "em_device.h"
#include "sl_memory_manager_region.h"
#include "sl_memory_manager_region_config.h"
#include "sl_component_catalog.h"

// Prevent's compilation errors when building in simulation.
#ifndef   __USED
  #define __USED
#endif

#if defined(__GNUC__)
// Declare stack object used with GCC.
static char sl_stack[SL_STACK_SIZE] __attribute__ ((aligned(8), used, section(".stack")));

/*
 * Declare the base and limit of the full heap region used with GCC to make
 * use of otherwise unused memory.
 */
extern char __HeapBase[];
extern char __HeapLimit[];

#elif defined(__ICCARM__)
#define IAR_HEAP_BLOCK_NAME      "MEMORY_MANAGER_HEAP"

// Declare stack object used with IAR.
__root char sl_stack[SL_STACK_SIZE] @ ".stack";

  #pragma section=IAR_HEAP_BLOCK_NAME

#endif

/***************************************************************************//**
 * Gets size and location of the stack.
 ******************************************************************************/
sl_memory_region_t sl_memory_get_stack_region(void)
{
  sl_memory_region_t region;

  region.addr = &sl_stack;
  region.size = SL_STACK_SIZE;
  return region;
}

/***************************************************************************//**
 * Gets size and location of the heap.
 ******************************************************************************/
sl_memory_region_t sl_memory_get_heap_region(void)
{
  sl_memory_region_t region;

  // Report the actual heap region.
#if defined(__GNUC__)
  region.addr = __HeapBase;
  region.size = (uintptr_t) __HeapLimit - (uintptr_t) __HeapBase;

#elif defined(__ICCARM__)
  region.addr = __section_begin(IAR_HEAP_BLOCK_NAME);
  region.size = __section_size(IAR_HEAP_BLOCK_NAME);
#endif

  return region;
}

#if defined(SL_CATALOG_MEMORY_MANAGER_PSRAM_PRESENT)

#if defined(__ICCARM__)
#define IAR_PSRAM_BLOCK_NAME "PSRAM_HEAP"
#pragma section=IAR_PSRAM_BLOCK_NAME
#endif

/***************************************************************************//**
 * Gets size and location of the PSRAM heap.
 ******************************************************************************/
sl_memory_region_t sl_memory_get_psram_heap_region(void)
{
  sl_memory_region_t region;

  // Report the actual PSRAM heap region.
#if defined(__GNUC__)
  extern char __HeapPsramBase[];
  extern char __HeapPsramLimit[];

  region.addr = __HeapPsramBase;
  region.size = (uintptr_t) __HeapPsramLimit - (uintptr_t) __HeapPsramBase;

#elif defined(__ICCARM__)
  region.addr = __section_begin(IAR_PSRAM_BLOCK_NAME);
  region.size = __section_size(IAR_PSRAM_BLOCK_NAME);
#endif

  return region;
}
#endif

#if defined(SL_CATALOG_MEMORY_MANAGER_DTCM_PRESENT)

#if defined(__ICCARM__)
#define IAR_DTCM_BLOCK_NAME "DTCM_HEAP"
#pragma section=IAR_DTCM_BLOCK_NAME
#endif

/***************************************************************************//**
 * Gets size and location of the DTCM heap.
 ******************************************************************************/
sl_memory_region_t sl_memory_get_dtcm_heap_region(void)
{
  sl_memory_region_t region;

  // Report the actual DTCM heap region.
#if defined(__GNUC__)
  extern char __HeapDtcmBase[];
  extern char __HeapDtcmLimit[];

  region.addr = __HeapDtcmBase;
  region.size = (uintptr_t) __HeapDtcmLimit - (uintptr_t) __HeapDtcmBase;

#elif defined(__ICCARM__)
  region.addr = __section_begin(IAR_DTCM_BLOCK_NAME);
  region.size = __section_size(IAR_DTCM_BLOCK_NAME);
#endif

  return region;
}
#endif

#if defined(__GNUC__)
/***************************************************************************//**
 * Extends the process data space.
 *
 * @param[in]  incr   Number of bytes to increment/decrement
 *
 * @return Start of the new space allocated if successful. -1 if error.
 *
 * @note (1) This is a helper function called by the standard C library
 *           function malloc(). _sbrk() is used to dynamically change the
 *           amount of space allocated for the calling process data segment.
 *           The change is made by allocating the appropriate amount of space
 *           from the end of heap.
 *           _sbrk() adds 'incr' bytes to the end of heap and changes
 *           the allocated space accordingly. 'incr' can be negative, in which
 *           case the amount of allocated space is decreased.
 *
 * @note (2) When the Memory Manager (MM) is used, there is no need for
 *           _sbrk() as there is no possible extension with the MM controlling
 *           the entire heap size.
 *           If _sbrk() is called by the standard C library, then the project
 *           may have used the standard C malloc() function implementation.
 *           In that case, the MM retarget, wrapping the GCC malloc() to the
 *           MM sl_malloc(), may have not worked. You may want to double-check
 *           your project settings.
 ******************************************************************************/
__USED __WEAK void * _sbrk(int incr)
{
#if defined(SL_CATALOG_MEMORY_MANAGER_PRESENT)
  (void)incr;

  // This means there is an issue with the setup of C standard library. See Note #2.
  while (1) {
    // infinite loop
  }
#else
  static char *heap_end = __HeapBase;
  char *prev_heap_end;

  if ((heap_end + incr) > __HeapLimit) {
    // Not enough heap
    return (void *) -1;
  }

  prev_heap_end = heap_end;
  heap_end += incr;

  return prev_heap_end;
#endif
}
#endif
