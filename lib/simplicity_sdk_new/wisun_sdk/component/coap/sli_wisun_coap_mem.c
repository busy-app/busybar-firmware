/***************************************************************************//**
 * @file sli_wisun_coap_mem.c
 * @brief Wi-SUN CoAP memory handler module
 *******************************************************************************
 * # License
 * <b>Copyright 2019 Silicon Laboratories Inc. www.silabs.com</b>
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

// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------
#include "sl_assert.h"
#include "sl_memory_manager.h"
#include "sli_wisun_coap_mem.h"
#include "sl_wisun_types.h"
#include "sl_wisun_trace_util.h"
// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

/* Using static memory pool */
#if SL_WISUN_COAP_MEM_USE_STATIC_MEMORY_POOL

#include "sl_mempool.h"

/// Internal CoAP memory pool handler structure
typedef struct _coap_mem {
  /// ID
  uint32_t id;
  /// Memory Pool object
  sl_mempool_t mempool;
  /// Size of the blocks
  uint16_t block_size;
  /// Count of the allocated blocks
  uint16_t block_count;
  /// buffer pointer
  uint8_t *buff;
} _coap_mem_t;

// -----------------------------------------------------------------------------
//                          Static Function Declarations
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                                Static Variables
// -----------------------------------------------------------------------------

/// Very Low memory buffer
static uint8_t _mem0_very_low
[(WISUN_COAP_MEMORY_VERY_LOW_SIZE * WISUN_COAP_MEMORY_VERY_LOW_COUNT)] = { 0 };

/// Low memory buffer
static uint8_t _mem1_low
[(WISUN_COAP_MEMORY_LOW_SIZE      * WISUN_COAP_MEMORY_LOW_COUNT)] = { 0 };

/// Medium memory buffer
static uint8_t _mem2_medium
[(WISUN_COAP_MEMORY_MEDIUM_SIZE   * WISUN_COAP_MEMORY_MEDIUM_COUNT)] = { 0 };

/// High memory buffer
static uint8_t _mem3_high
[(WISUN_COAP_MEMORY_HIGH_SIZE     * WISUN_COAP_MEMORY_HIGH_COUNT)] = { 0 };

/// Memory handler array
static _coap_mem_t _mem[WISUN_COAP_MEMORY_OPTION_COUNT] = {
  {
    .id          = WISUN_COAP_MEMORY_VERY_LOW_ID,
    .block_size  = WISUN_COAP_MEMORY_VERY_LOW_SIZE,
    .block_count = WISUN_COAP_MEMORY_VERY_LOW_COUNT,
    .buff        = _mem0_very_low
  },
  {
    .id          = WISUN_COAP_MEMORY_LOW_ID,
    .block_size  = WISUN_COAP_MEMORY_LOW_SIZE,
    .block_count = WISUN_COAP_MEMORY_LOW_COUNT,
    .buff        = _mem1_low
  },
  {
    .id          = WISUN_COAP_MEMORY_MEDIUM_ID,
    .block_size  = WISUN_COAP_MEMORY_MEDIUM_SIZE,
    .block_count = WISUN_COAP_MEMORY_MEDIUM_COUNT,
    .buff        = _mem2_medium
  },
  {
    .id          = WISUN_COAP_MEMORY_HIGH_ID,
    .block_size  = WISUN_COAP_MEMORY_HIGH_SIZE,
    .block_count = WISUN_COAP_MEMORY_HIGH_COUNT,
    .buff        = _mem3_high,
  }
};

#endif

// -----------------------------------------------------------------------------
//                          Static Function Declarations
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                          Static Function Definitions
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------

/* Init Static memory pool */
void sli_wisun_coap_mem_init(void)
{
#if SL_WISUN_COAP_MEM_USE_STATIC_MEMORY_POOL
  for (uint8_t i = 0; i < WISUN_COAP_MEMORY_OPTION_COUNT; ++i) {
    // create memory pool
    EFM_ASSERT(sl_mempool_create(&_mem[i].mempool,
                                 _mem[i].block_count,
                                 _mem[i].block_size,
                                 _mem[i].buff,
                                 _mem[i].block_count * _mem[i].block_size) == SL_STATUS_OK);
  }
#else
  (void) 0;
#endif
}

/* Wi-SUN Coap malloc */
void *sli_wisun_coap_mem_malloc(size_t size)
{
  void *p = NULL;
  if (!size) {
    return NULL;
  }
// STATIC MEMORY POOL
#if SL_WISUN_COAP_MEM_USE_STATIC_MEMORY_POOL

  for (uint8_t i = 0; i < WISUN_COAP_MEMORY_OPTION_COUNT; ++i) {
    if (size <= _mem[i].block_size) {   // check size
      p = sl_mempool_alloc(&_mem[i].mempool);
      if (p != NULL) {   // if allocated, return, otherwise continue to try a bigger memory chunk
        return p;
      }
    }
  }

// HEAP MALLOC
#else
  p = sl_malloc(size);
#endif

  return p;
}

/* Wi-SUN Coap free */
void sli_wisun_coap_mem_free(void *addr)
{
  if (addr == NULL) {
    return;
  }

// STATIC MEMORY POOL
#if SL_WISUN_COAP_MEM_USE_STATIC_MEMORY_POOL

  for (uint8_t i = 0; i < WISUN_COAP_MEMORY_OPTION_COUNT; ++i) {
    if (sl_mempool_is_addr_in_buff(&_mem[i].mempool, addr)) {
      sl_mempool_free(&_mem[i].mempool, addr);
    }
  }

// HEAP FREE
#else
  sl_free(addr);
#endif
}
