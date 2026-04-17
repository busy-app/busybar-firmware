/***************************************************************************//**
 * @file
 * @brief Memory Manager Driver's Retarget Implementation.
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

#include "sl_memory_manager.h"

#if defined(SL_COMPONENT_CATALOG_PRESENT)
#include "sl_component_catalog.h"
#endif

#if defined(SL_CATALOG_MEMORY_PROFILER_PRESENT)
#include "sli_memory_profiler.h"
#endif

#if defined(__GNUC__)
#include <sys/reent.h>

// Wrapping a system function with GCC works by using the linker option '--wrap=symbol'.
// Any undefined reference to "symbol" will be resolved to "__wrap_symbol".
// Any undefined reference to "__real_symbol" will be resolved to "symbol".
// In our case, "__real_symbol" is not really required as the retargeted standard
// C memory functions will call the corresponding Memory Manager (MM)
// native function and not the standard functions again. So it should be seen:
// standard _malloc_r()  -> MM sl_malloc()
// standard _free_r()    -> MM sl_free()
// standard _calloc_r()  -> MM sl_calloc()
// standard _realloc_r() -> MM sl_realloc()
#define STD_LIB_WRAPPER_MALLOC  __wrap__malloc_r
#define STD_LIB_WRAPPER_FREE    __wrap__free_r
#define STD_LIB_WRAPPER_CALLOC  __wrap__calloc_r
#define STD_LIB_WRAPPER_REALLOC __wrap__realloc_r

// The GNU gold linker has an issue with LTO and wrapping, where the symbol is
// stripped even if it is used in the source code, which leads to link-time errors.
// https://sourceware.org/bugzilla/show_bug.cgi?id=24415
// By marking the wrapper as externally_visible, the symbol will not be stripped
// from the final binary, regardless if it is referenced or not in the source code.
#if !defined(__clang__)
#define ATTR_EXT_VIS    __attribute__((externally_visible))
#else
// The 'externally_visible' attribute is not supported by clang, but the 'used'
// attribute has a similar effect in preventing the compiler from stripping symbols.
// However, for this wrapper, we can leave it empty for clang.
#define ATTR_EXT_VIS
#endif

// Reentrant parameter.
#define RARG            const struct _reent *reent,
#define VOID_RARG       (void) reent

#elif defined(__IAR_SYSTEMS_ICC__)
// Wrapping a system function works with IAR by patching symbol definitions using $Super$$ and $Sub$$
// The $Super$$ special pattern identifies the original unpatched function used for calling
// the original function directly.
// The $Sub$$ special pattern identifies the new function that is called instead of the
// original function.
// In our case, $Super$$ is not really required as the retargeted standard
// C memory functions will call the corresponding Memory Manager (MM)
// native function and not the standard functions again.
//
// NOTE: IAR supports three separate heap memory handlers: the basic, the advanced, and the no-free
//       heap handlers.
//       - If there are calls to heap memory allocation routines in your application, but no calls
//         to heap deallocation routines, the linker automatically chooses the no-free heap.
//       - If there are calls to heap memory allocation routines in your application, the linker
//         automatically chooses the advanced heap.
//       - If there are calls to heap memory allocation routines in a library for example, the linker
//         automatically chooses the basic heap.
//
//       Depending on the heap handler type, IAR will select a different malloc/free/calloc/realloc
//       implementation provided by the IAR system library. That's why, there are different sets of
//       macros below below to wrap the right IAR standard memory functions with $Sub$$.
//       - Basic heap:    IAR memory functions are prefixed with "basic_"
//       - Advanced heap: IAR memory functions are prefixed with "dl"
//       - No Free heap:  IAR memory functions are prefixed with "no_free"
//
//       For No Free heap, IAR does not provide a free and realloc implementation.
#if (__VER__ == 8050009)
#define STD_LIB_WRAPPER_MALLOC  $Sub$$__iar_dlmalloc
#define STD_LIB_WRAPPER_FREE    $Sub$$__iar_dlfree
#define STD_LIB_WRAPPER_CALLOC  $Sub$$__iar_dlcalloc
#define STD_LIB_WRAPPER_REALLOC $Sub$$__iar_dlrealloc
#elif (__VER__ == 9040001) || (__VER__ == 9060004)
#define STD_LIB_WRAPPER_MALLOC  $Sub$$__basic_malloc
#define STD_LIB_WRAPPER_FREE    $Sub$$__basic_free
#define STD_LIB_WRAPPER_CALLOC  $Sub$$__basic_calloc
#define STD_LIB_WRAPPER_REALLOC $Sub$$__basic_realloc

#define STD_LIB_WRAPPER_MALLOC_ADVANCED  $Sub$$__iar_dlmalloc
#define STD_LIB_WRAPPER_FREE_ADVANCED    $Sub$$__iar_dlfree
#define STD_LIB_WRAPPER_CALLOC_ADVANCED  $Sub$$__iar_dlcalloc
#define STD_LIB_WRAPPER_REALLOC_ADVANCED $Sub$$__iar_dlrealloc

#define STD_LIB_WRAPPER_MALLOC_NO_FREE  $Sub$$__no_free_malloc
#define STD_LIB_WRAPPER_CALLOC_NO_FREE  $Sub$$__no_free_calloc
#else
#error Unsupported IAR compiler version for standard C memory functions retarget
#endif

// Since IAR does not use LTO, resolve the attribute as nothing.
#define ATTR_EXT_VIS

// Since IAR does not use reentrant functions, resolve reentrant parameter to nothing.
#define RARG
#define VOID_RARG

#else
#error Unsupported compiler for standard C memory functions retarget
#endif

#if defined(TEST_MEMORY_MANAGER_RETARGET_PRESENT)
volatile uint32_t retarget_malloc_counter = 0;
volatile uint32_t retarget_free_counter = 0;
volatile uint32_t retarget_calloc_counter = 0;
volatile uint32_t retarget_realloc_counter = 0;
#endif

/*******************************************************************************
 **************************   GLOBAL FUNCTIONS   *******************************
 ******************************************************************************/

/***************************************************************************//**
 * malloc() wrapper. Allocates a memory block of at least requested size from
 * the heap.
 *
 * @param[in] size  Size of the block, in bytes.
 *
 * @return    Pointer to allocated block if successful. Null pointer if
 *            allocation failed.
 *
 * @note Requesting a block of 0 byte will return a null pointer.
 *
 * @note All allocated blocks using this function will be considered long-term
 *       allocations.
 ******************************************************************************/
ATTR_EXT_VIS void *STD_LIB_WRAPPER_MALLOC(RARG
                                          size_t size)
{
#if defined(SL_CATALOG_MEMORY_PROFILER_PRESENT)
  void * volatile return_address = sli_memory_profiler_get_return_address();
#endif
  VOID_RARG;
  void *ptr;

  ptr = sl_malloc(size);
#if defined(TEST_MEMORY_MANAGER_RETARGET_PRESENT)
  retarget_malloc_counter++;
#endif

#if defined(SL_CATALOG_MEMORY_PROFILER_PRESENT)
  sli_memory_profiler_track_ownership(SLI_INVALID_MEMORY_TRACKER_HANDLE,
                                      ptr,
                                      return_address);
#endif

  return ptr;
}

#if defined(__IAR_SYSTEMS_ICC__) && (__VER__ == 9040001)
void *STD_LIB_WRAPPER_MALLOC_ADVANCED(size_t size)
{
#if defined(SL_CATALOG_MEMORY_PROFILER_PRESENT)
  void * volatile return_address = sli_memory_profiler_get_return_address();
#endif
  void *ptr;

  ptr = sl_malloc(size);

#if defined(SL_CATALOG_MEMORY_PROFILER_PRESENT)
  sli_memory_profiler_track_ownership(SLI_INVALID_MEMORY_TRACKER_HANDLE,
                                      ptr,
                                      return_address);
#endif

  return ptr;
}

void *STD_LIB_WRAPPER_MALLOC_NO_FREE(size_t size)
{
#if defined(SL_CATALOG_MEMORY_PROFILER_PRESENT)
  void * volatile return_address = sli_memory_profiler_get_return_address();
#endif
  void *ptr;

  ptr = sl_malloc(size);

#if defined(SL_CATALOG_MEMORY_PROFILER_PRESENT)
  sli_memory_profiler_track_ownership(SLI_INVALID_MEMORY_TRACKER_HANDLE,
                                      ptr,
                                      return_address);
#endif

  return ptr;
}
#endif

/***************************************************************************//**
 * free() wrapper. Frees a previously allocated block back into the heap.
 *
 * @param[in] ptr   Pointer to memory block to be freed.
 *
 * @note Passing a null pointer does nothing.
 ******************************************************************************/
ATTR_EXT_VIS void STD_LIB_WRAPPER_FREE(RARG
                                       void *ptr)
{
  VOID_RARG;
  sl_free(ptr);
#if defined(TEST_MEMORY_MANAGER_RETARGET_PRESENT)
  retarget_free_counter++;
#endif
}

#if defined(__IAR_SYSTEMS_ICC__) && (__VER__ == 9040001)
void STD_LIB_WRAPPER_FREE_ADVANCED(void *ptr)
{
  sl_free(ptr);
}
#endif

/***************************************************************************//**
 * calloc() wrapper. Dynamically allocates a block of memory cleared to 0.
 *
 * @param[in] item_count  Number of elements to be allocated.
 * @param[in] size        Size of each elements, in bytes.
 *
 * @return Pointer to allocated block if successful. Null pointer if
 *         allocation failed.
 *
 * @note All allocated blocks using this function will be considered long-term
 *       allocations.
 ******************************************************************************/
ATTR_EXT_VIS void *STD_LIB_WRAPPER_CALLOC(RARG
                                          size_t item_count,
                                          size_t size)
{
#if defined(SL_CATALOG_MEMORY_PROFILER_PRESENT)
  void * volatile return_address = sli_memory_profiler_get_return_address();
#endif
  VOID_RARG;
  void *ptr;

  ptr = sl_calloc(item_count, size);
#if defined(TEST_MEMORY_MANAGER_RETARGET_PRESENT)
  retarget_calloc_counter++;
#endif

#if defined(SL_CATALOG_MEMORY_PROFILER_PRESENT)
  sli_memory_profiler_track_ownership(SLI_INVALID_MEMORY_TRACKER_HANDLE,
                                      ptr,
                                      return_address);
#endif

  return ptr;
}

#if defined(__IAR_SYSTEMS_ICC__) && (__VER__ == 9040001)
void *STD_LIB_WRAPPER_CALLOC_ADVANCED(size_t item_count,
                                      size_t size)
{
#if defined(SL_CATALOG_MEMORY_PROFILER_PRESENT)
  void * volatile return_address = sli_memory_profiler_get_return_address();
#endif
  void *ptr;

  ptr = sl_calloc(item_count, size);

#if defined(SL_CATALOG_MEMORY_PROFILER_PRESENT)
  sli_memory_profiler_track_ownership(SLI_INVALID_MEMORY_TRACKER_HANDLE,
                                      ptr,
                                      return_address);
#endif

  return ptr;
}

void *STD_LIB_WRAPPER_CALLOC_NO_FREE(size_t item_count,
                                     size_t size)
{
#if defined(SL_CATALOG_MEMORY_PROFILER_PRESENT)
  void * volatile return_address = sli_memory_profiler_get_return_address();
#endif
  void *ptr;

  ptr = sl_calloc(item_count, size);

#if defined(SL_CATALOG_MEMORY_PROFILER_PRESENT)
  sli_memory_profiler_track_ownership(SLI_INVALID_MEMORY_TRACKER_HANDLE,
                                      ptr,
                                      return_address);
#endif

  return ptr;
}
#endif

/***************************************************************************//**
 * realloc() wrapper. Resizes a previously allocated memory block.
 *
 * @param[in] ptr   Pointer to the allocation to resize. If NULL, behavior
 *                  is same as sl_malloc(), sl_memory_alloc().
 * @param[in] size  New size of the block, in bytes. If 0, behavior is same as
 *                  sl_free(), sl_memory_free().
 *
 * @return Pointer to newly allocated block, if successful. Null pointer if
 *         re-allocation failed.
 *
 * @note All re-allocated blocks using this function will be considered
 *       long-term allocations.
 *
 * @note 'ptr' NULL and 'size' of 0 bytes is an incorrect parameters
 *       combination. No reallocation will be done by the function as it is
 *       an error condition.
 *
 * @note If the new 'size' is the same as the old, the function changes nothing
 *       and returns the same provided address 'ptr'.
 ******************************************************************************/
ATTR_EXT_VIS void *STD_LIB_WRAPPER_REALLOC(RARG
                                           void *ptr,
                                           size_t size)
{
#if defined(SL_CATALOG_MEMORY_PROFILER_PRESENT)
  void * volatile return_address = sli_memory_profiler_get_return_address();
#endif
  VOID_RARG;
  void *r_ptr;

  r_ptr = sl_realloc(ptr, size);
#if defined(TEST_MEMORY_MANAGER_RETARGET_PRESENT)
  retarget_realloc_counter++;
#endif

#if defined(SL_CATALOG_MEMORY_PROFILER_PRESENT)
  sli_memory_profiler_track_ownership(SLI_INVALID_MEMORY_TRACKER_HANDLE,
                                      r_ptr,
                                      return_address);
#endif

  return r_ptr;
}

#if defined(__IAR_SYSTEMS_ICC__) && (__VER__ == 9040001)
void *STD_LIB_WRAPPER_REALLOC_ADVANCED(void *ptr,
                                       size_t size)
{
#if defined(SL_CATALOG_MEMORY_PROFILER_PRESENT)
  void * volatile return_address = sli_memory_profiler_get_return_address();
#endif
  void *r_ptr;

  r_ptr = sl_realloc(ptr, size);

#if defined(SL_CATALOG_MEMORY_PROFILER_PRESENT)
  sli_memory_profiler_track_ownership(SLI_INVALID_MEMORY_TRACKER_HANDLE,
                                      r_ptr,
                                      return_address);
#endif

  return r_ptr;
}
#endif
