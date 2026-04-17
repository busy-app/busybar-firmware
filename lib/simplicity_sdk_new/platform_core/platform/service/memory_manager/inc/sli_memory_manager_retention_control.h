/***************************************************************************//**
 * @file
 * @brief Memory Manager Retention Control.
 *******************************************************************************
 * # License
 * <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifndef SLI_MEMORY_MANAGER_RETENTION_CONTROL_H_
#define SLI_MEMORY_MANAGER_RETENTION_CONTROL_H_

#include <stddef.h>
#include <stdint.h>

#include "sl_memory_manager.h"

#ifdef __cplusplus
extern "C" {
#endif

// ----------------------------------------------------------------------------
// DATA TYPES

// Retention Control Structure.
typedef struct {
  uint32_t smallest_bank_size;
  uint16_t *banks_counter;
  uint32_t (*get_smallest_bank_id)(void *);
  uint32_t (*smallest_to_real_bank_id)(uint32_t);
  void (*enable_retention)(uint32_t);
  void (*disable_retention)(uint32_t);
  sli_bank_coverage_t (*get_block_bank_coverage)(void *, uint32_t);
} sli_retention_control_t;

// ----------------------------------------------------------------------------
// PROTOTYPES

/***************************************************************************//**
 * Initialize Memory Manager HAL for the given heap.
 *
 * @param[in]  heap  Heap Handle.
 ******************************************************************************/
void sli_memory_manager_hal_init(sl_memory_heap_t *heap);

/***************************************************************************//**
 * Gets RAM bank ID from address.
 *
 * @param[in]  heap  Heap Handle.
 * @param[in]  addr  Address.
 *
 * @return  RAM bank's ID.
 ******************************************************************************/
uint32_t sli_memory_manager_get_bank_id_by_addr(const sl_memory_heap_t *heap,
                                                void *addr);

/***************************************************************************//**
 * Increments Bank Counters between a start bank ID and an end bank ID.
 *
 * @param[in]  heap       Heap Handle.
 * @param[in]  start_id   Start bank ID.
 * @param[in]  end_id     End bank ID.
 ******************************************************************************/
void sli_memory_manager_increment_bank_counter(sl_memory_heap_t *heap,
                                               uint32_t start_id,
                                               uint32_t end_id);

/***************************************************************************//**
 * Decrements Bank Counters between a start bank ID and an end bank ID.
 *
 * @param[in]  heap       Heap Handle.
 * @param[in]  start_id   Start bank ID.
 * @param[in]  end_id     End bank ID.
 ******************************************************************************/
void sli_memory_manager_decrement_bank_counter(sl_memory_heap_t *heap,
                                               uint32_t start_id,
                                               uint32_t end_id);

#ifdef __cplusplus
}
#endif

#endif // SLI_MEMORY_MANAGER_RETENTION_CONTROL_H_
