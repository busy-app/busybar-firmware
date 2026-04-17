/***************************************************************************//**
 * @file
 * @brief Memory Manager Driver's Memory Pool Common Implementation.
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

/*******************************************************************************
 **************************   GLOBAL FUNCTIONS   *******************************
 ******************************************************************************/

/***************************************************************************//**
 * Dynamically allocates a memory pool handle.
 ******************************************************************************/
sl_status_t sl_memory_pool_handle_alloc(sl_memory_pool_t **pool_handle)
{
#if defined(SL_CATALOG_MEMORY_PROFILER_PRESENT)
  void * volatile return_address = sli_memory_profiler_get_return_address();
#endif
  sl_status_t status;

  // Allocate pool_handle as a long-term block.
  status = sl_memory_alloc(sizeof(sl_memory_pool_t), BLOCK_TYPE_LONG_TERM, (void **)pool_handle);

#if defined(SL_CATALOG_MEMORY_PROFILER_PRESENT)
  sli_memory_profiler_track_ownership(SLI_INVALID_MEMORY_TRACKER_HANDLE, *pool_handle, return_address);
#endif

  return status;
}

/***************************************************************************//**
 * Frees a dynamically allocated memory pool handle.
 ******************************************************************************/
sl_status_t sl_memory_pool_handle_free(sl_memory_pool_t *pool_handle)
{
  sl_status_t status;

  // Free memory pool_handle.
  status = sl_memory_free((void *)pool_handle);

  return status;
}

/***************************************************************************//**
 * Gets the size of the memory pool handle structure.
 ******************************************************************************/
uint32_t sl_memory_pool_handle_get_size(void)
{
  return sizeof(sl_memory_pool_t);
}

/***************************************************************************//**
 * Gets the total count of blocks in a memory pool.
 ******************************************************************************/
uint32_t sl_memory_pool_get_total_block_count(const sl_memory_pool_t *pool_handle)
{
  if (pool_handle == NULL) {
    return 0;
  }

  return pool_handle->block_count;
}

/***************************************************************************//**
 * Gets the count of used blocks in a memory pool.
 ******************************************************************************/
uint32_t sl_memory_pool_get_used_block_count(const sl_memory_pool_t *pool_handle)
{
  uint32_t used_block_count = 0;

  if (pool_handle == NULL) {
    return 0;
  }

  used_block_count = pool_handle->block_count - sl_memory_pool_get_free_block_count(pool_handle);

  return used_block_count;
}
