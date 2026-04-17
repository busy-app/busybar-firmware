/***************************************************************************//**
 * @file
 * @brief Memory Manager Driver's C++ Implementation.
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

#include <cstddef>

#include "sl_memory_manager.h"
#include "sl_assert.h"

//--------------------------------------------------------------------------
// MEMORY MANAGER PRE-INITIALIZATION

#if defined (__GNUC__)
/***************************************************************************//**
 * Initializes the memory manager.
 *
 * @note With a C++ application, the memory manager requires to be initialized
 * prior to any malloc() done in any static global objects' constructors.
 *
 * This function is called after copying in RAM the .data section
 * (initialized global variables) and zeroing the .bss section (non-initialized
 * global variables) and before the main() entry point. The function is placed
 * in a special section called .preinit_array. preinit_array hold pointers
 * to functions that should be executed before any other initialization
 * functions, including C++ static constructors. It allows very early
 * initialization tasks that need to be completed before any dynamic linking
 * or library initialization occurs.
 *
 * When sl_memory_init() is called early during the GCC/G++ startup code,
 * sl_memory_init() is not called by the component sl_system.
 ******************************************************************************/
static void sl_memory_preinit(void) {
  sl_memory_init();
}

__attribute__((used, section(".preinit_array")))
static void (*preinit_array)(void) = sl_memory_preinit;
#endif

#if defined(__IAR_SYSTEMS_ICC__)
/***************************************************************************//**
 * Initializes the memory manager.
 *
 * @note This special C++ class and its associated global object allows the
 * memory manager sl_memory_init() to be initialized prior to any malloc() done
 * in any static global objects' constructors.
 * It serves the same purpose as the C function sl_memory_preinit() used for
 * GCC/G++ early initialization above. It will use the special section
 * .preinit_array.
 *
 * The IAR "#pragma early_dynamic_initialization" marks certain global objects
 * for earlier initialization by registering their constructor to the
 * .preinit_array section. When sl_memory_init() is called early during the
 * IAR startup code (i.e. after copying in RAM the .data section
 * (initialized global variables) and zeroing the .bss section (non-initialized
 * global variables) and before the main() entry point), sl_memory_init()
 * is not called by the component sl_system.
 ******************************************************************************/
#pragma early_dynamic_initialization
class sl_memory_preinit
{
public:
  sl_memory_preinit()
  {
    sl_memory_init();
  }

  ~sl_memory_preinit()
  {
  }
};

sl_memory_preinit sl_memory_preinit_obj;
#endif

//--------------------------------------------------------------------------
// GLOBAL C++ NEW/DELETE OVERLOAD

/***************************************************************************//**
 * Overloaded new operator.
 * Allocates a memory block of at least requested size from the heap.
 *
 * @param[in]  size   Size of the block, in bytes.
 *
 * @return    Pointer to allocated block if successful. Null pointer if
 *            allocation failed.
 ******************************************************************************/
void *operator new(size_t size)
{
  void *block = NULL;
  sl_status_t status;

  status = sl_memory_alloc(size, BLOCK_TYPE_LONG_TERM, (void **)&block);
  if (status != SL_STATUS_OK) {
    // Convert C NULL pointer to C++ dedicated type.
    block = nullptr;
  }

  return block;
}

/***************************************************************************//**
 * Overloaded delete operator used for single object allocations.
 * Frees a previously allocated block back into the heap.
 *
 * @param[in] ptr   Pointer to memory block to be freed.
 ******************************************************************************/
void operator delete(void *ptr)
{
  sl_status_t status;

  status = sl_memory_free(ptr);
  if (status != SL_STATUS_OK) {
    EFM_ASSERT(false);
  }
}

/***************************************************************************//**
 * Overloaded delete operator used for array of objects allocations.
 * Frees a previously allocated block back into the heap.
 *
 * @param[in] ptr   Pointer to memory block to be freed.
 ******************************************************************************/
void operator delete[](void *ptr)
{
  sl_status_t status;

  status = sl_memory_free(ptr);
  if (status != SL_STATUS_OK) {
    EFM_ASSERT(false);
  }
}

/***************************************************************************//**
 * Overloaded delete operator for single object allocations.
 * Frees a previously allocated block back into the heap.
 *
 * @param[in] ptr   Pointer to memory block to be freed.
 * @param[in] size  Size of block to be freed, in bytes.
 *
 * @note The -Wsized-deallocation option in G++ is a warning option related
 *       to C++17 and later versions. This other overloaded version of delete
 *       is recommended by -Wsized-deallocation option for C++17. Indeed,
 *       certain standard containers (e.g. vector, list) with C++17 use a delete
 *       that needs to provide a 'size' parameter. The Memory Manager
 *       does not provide a free() function taking a 'size' parameter. Hence,
 *       the 'size' parameter is simply ignored. When compiling with C++11,
 *       the same containers such as vector, list will use an overloaded delete
 *       without a 'size' parameter.
 *       The -Wsized-deallocation option is used to catch situations where
 *       you might be using the delete operator incorrectly with pointers
 *       that were not allocated with new or that were allocated with a type
 *       that doesn't have a suitable operator delete.
 ******************************************************************************/
void operator delete(void *ptr,
                     size_t size)
{
  sl_status_t status;

  (void)size;

  status = sl_memory_free(ptr);
  if (status != SL_STATUS_OK) {
    EFM_ASSERT(false);
  }
}

/***************************************************************************//**
 * Overloaded delete operator used for array of objects allocations.
 * Frees a previously allocated block back into the heap.
 *
 * @param[in] ptr   Pointer to memory block to be freed.
 * @param[in] size  Size of block to be freed, in bytes.
 ******************************************************************************/
void operator delete[](void *ptr,
                       size_t size)
{
  sl_status_t status;

  (void)size;

  status = sl_memory_free(ptr);
  if (status != SL_STATUS_OK) {
    EFM_ASSERT(false);
  }
}
