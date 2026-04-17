/***************************************************************************//**
 * @file
 * @brief Memory Manager Retention Control Empty HAL.
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

#include <stddef.h>
#include <stdint.h>
#include "sli_memory_manager_retention_control.h"
#include "sli_code_classification.h"
#include "em_device.h"
#include "sl_assert.h"

#define DMEM_MEM_BASE  SRAM_BASE

#if !defined(DMEM_NUM_BANKS)
// On XG21 DMEM_NUM_BANKS is named DMEM_NUM_BANK
#if defined(DMEM_NUM_BANK)
#define DMEM_NUM_BANKS DMEM_NUM_BANK
// On XG26 DMEM_NUM_BANKS is named DMEM0_NUM_BANKS
#elif defined(DMEM0_NUM_BANKS)
#define DMEM_NUM_BANKS DMEM0_NUM_BANKS
#define DMEM_BANK0_SIZE DMEM0_BANK0_SIZE
#endif
#endif


static uint32_t memory_manager_dmem_get_smallest_bank_id(void *addr);
static uint32_t memory_manager_dmem_smallest_to_real_bank_id(uint32_t small_bank_id);
static void memory_manager_dmem_enable_retention(uint32_t bank_id);
static void memory_manager_dmem_disable_retention(uint32_t bank_id);
static sli_bank_coverage_t memory_manager_dmem_get_block_bank_coverage(void *start_addr,
                                                                       uint32_t block_size);

// Banks allocation counters for DMEM.
static uint16_t dmem_banks_counter[DMEM_NUM_BANKS] SL_FAST_DATA = { 0 };

// DMEM RAM type.
static sli_retention_control_t retention_control_dmem SL_FAST_DATA = {
  .smallest_bank_size = DMEM_BANK0_SIZE,
  .banks_counter = dmem_banks_counter,
  .get_smallest_bank_id = memory_manager_dmem_get_smallest_bank_id,
  .smallest_to_real_bank_id = memory_manager_dmem_smallest_to_real_bank_id,
  .enable_retention = memory_manager_dmem_enable_retention,
  .disable_retention = memory_manager_dmem_disable_retention,
  .get_block_bank_coverage = memory_manager_dmem_get_block_bank_coverage,
};

/*******************************************************************************
 **************************   GLOBAL FUNCTIONS   *******************************
 ******************************************************************************/

/***************************************************************************//**
 * Initialize Memory Manager HAL for the given heap.
 ******************************************************************************/
void sli_memory_manager_hal_init(sl_memory_heap_t *heap)
{
  heap->retention_control = &retention_control_dmem;
}

/***************************************************************************//**
 * Gets RAM bank ID from address.
 ******************************************************************************/
SL_CODE_CLASSIFY(SL_CODE_COMPONENT_MEMORY_MANAGER, SL_CODE_CLASS_TIME_CRITICAL)
uint32_t sli_memory_manager_get_bank_id_by_addr(const sl_memory_heap_t *heap,
                                                void *addr)
{
  // Validate given address.
  EFM_ASSERT((addr >= heap->base_addr) && (addr <= (void *)((uint8_t *)heap->base_addr + heap->size)));
  uint32_t bank_id;
  sli_retention_control_t *retention_control = (sli_retention_control_t *)heap->retention_control;

  // Get the bank ID as if all banks were of the same size being the smallest
  // bank size for the given heap.
  bank_id = retention_control->get_smallest_bank_id(addr);

  // Convert the bank ID into the real ID as exposed in the registers.
  bank_id = retention_control->smallest_to_real_bank_id(bank_id);

  return bank_id;
}

/***************************************************************************//**
 * Increments Bank Counters between a start bank ID and an end bank ID.
 ******************************************************************************/
SL_CODE_CLASSIFY(SL_CODE_COMPONENT_MEMORY_MANAGER, SL_CODE_CLASS_TIME_CRITICAL)
void sli_memory_manager_increment_bank_counter(sl_memory_heap_t *heap,
                                               uint32_t start_id,
                                               uint32_t end_id)
{
  EFM_ASSERT(start_id <= end_id);
  sli_retention_control_t *retention_control = (sli_retention_control_t *)heap->retention_control;

  // Update banks' counter and retention.
  for (uint32_t id = start_id; id <= end_id; id++) {
    retention_control->banks_counter[id]++;
    retention_control->enable_retention(id);
  }
}

/***************************************************************************//**
 * Decrements Bank Counters between a start bank ID and an end bank ID.
 ******************************************************************************/
SL_CODE_CLASSIFY(SL_CODE_COMPONENT_MEMORY_MANAGER, SL_CODE_CLASS_TIME_CRITICAL)
void sli_memory_manager_decrement_bank_counter(sl_memory_heap_t *heap,
                                               uint32_t start_id,
                                               uint32_t end_id)
{
  EFM_ASSERT(start_id <= end_id);
  sli_retention_control_t *retention_control = (sli_retention_control_t *)heap->retention_control;

  // Update banks' counter and retention.
  for (uint32_t id = start_id; id <= end_id; id++) {
    retention_control->banks_counter[id]--;
    if (!retention_control->banks_counter[id]) {
      retention_control->disable_retention(id);
    }
  }
}

/*******************************************************************************
 **************************   LOCAL FUNCTIONS   *******************************
 ******************************************************************************/

/***************************************************************************//**
 * Get DMEM bank ID as if all banks were of the same size (Using the smallest
 * bank size).
 *
 * @param[in]  addr  Address.
 *
 * @return  DMEM smallest bank ID.
 ******************************************************************************/
SL_CODE_CLASSIFY(SL_CODE_COMPONENT_MEMORY_MANAGER, SL_CODE_CLASS_TIME_CRITICAL)
static uint32_t memory_manager_dmem_get_smallest_bank_id(void *addr)
{
  return ((size_t)((uint8_t *)addr - DMEM_MEM_BASE) / DMEM_BANK0_SIZE);
}

/***************************************************************************//**
 * Convert a smallest bank ID (As if all banks had the smallest bank size)
 * to a real bank ID for DMEM.
 *
 * @param[in]  small_bank_id  Smallest bank ID.
 *
 * @return  DMEM real bank ID.
 ******************************************************************************/
SL_CODE_CLASSIFY(SL_CODE_COMPONENT_MEMORY_MANAGER, SL_CODE_CLASS_TIME_CRITICAL)
static uint32_t memory_manager_dmem_smallest_to_real_bank_id(uint32_t small_bank_id)
{
  return small_bank_id;
}

/***************************************************************************//**
 * Enable the retention for a given DMEM bank ID.
 *
 * @param[in]  bank_id  Bank ID.
 ******************************************************************************/
SL_CODE_CLASSIFY(SL_CODE_COMPONENT_MEMORY_MANAGER, SL_CODE_CLASS_TIME_CRITICAL)
static void memory_manager_dmem_enable_retention(uint32_t bank_id)
{
  (void)bank_id;
}

/***************************************************************************//**
 * Disable the retention for a given DMEM bank ID.
 *
 * @param[in]  bank_id  Bank ID.
 ******************************************************************************/
SL_CODE_CLASSIFY(SL_CODE_COMPONENT_MEMORY_MANAGER, SL_CODE_CLASS_TIME_CRITICAL)
static void memory_manager_dmem_disable_retention(uint32_t bank_id)
{
  (void)bank_id;
}

/***************************************************************************//**
 * Get the bank coverage of a pool block in DMEM.
 * The block may span multiple banks.
 *
 * @param[in]  start_addr   Pointer to the start address of the block.
 * @param[in]  pool_handle  Pointer to the pool handle.
 *
 * @return     The bank coverage of the block.
 ******************************************************************************/
SL_CODE_CLASSIFY(SL_CODE_COMPONENT_MEMORY_MANAGER, SL_CODE_CLASS_TIME_CRITICAL)
static sli_bank_coverage_t memory_manager_dmem_get_block_bank_coverage(void *start_addr,
                                                                       uint32_t block_size)
{
  sli_bank_coverage_t block_coverage;

  // Assumes consistent bank sizes.
  block_coverage.start = ((uintptr_t)start_addr - (uintptr_t)SRAM_BASE) / retention_control_dmem.smallest_bank_size;
  block_coverage.end = (((uintptr_t)start_addr + block_size - 1) - (uintptr_t)SRAM_BASE) / retention_control_dmem.smallest_bank_size;

  return block_coverage;
}