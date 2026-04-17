/*******************************************************************************
 * @brief This file enables support for all Silabs specific features available
 * only through the Simplicity SDK
 *******************************************************************************
 * # License
 * <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: LicenseRef-MSLA
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of the Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement
 * By installing, copying or otherwise using this software, you agree to the
 * terms of the MSLA.
 *
 ******************************************************************************/
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>

#ifdef SL_CATALOG_MEMORY_MANAGER_PRESENT
#include "sl_memory_manager.h"
#else
// Stubs for host/NCP builds
static inline size_t sl_memory_get_total_heap_size(void)
{
  return 0;
}
static inline size_t sl_memory_get_used_heap_size(void)
{
  return 0;
}
static inline size_t sl_memory_get_heap_high_watermark(void)
{
  return 0;
}
#endif
#include "sl_status.h"
#include "memory_usage.h"

static size_t init_used_heap = 0;
static size_t init_heap_high_watermark = 0;

void sl_memory_usage_init(void)
{
  init_used_heap = sl_memory_get_used_heap_size();
  init_heap_high_watermark = sl_memory_get_heap_high_watermark();
}

uint32_t sl_get_memory_usage_data(sl_memory_usage_data_t memory_usage_data_type)
{
  size_t memory_value = 0;
  switch (memory_usage_data_type) {
    case SL_MEMORY_USAGE_TOTAL_HEAP_SIZE:
      memory_value = sl_memory_get_total_heap_size();
      break;
    case SL_MEMORY_USAGE_CURRENT_USED_HEAP_SIZE:
      memory_value = sl_memory_get_used_heap_size();
      break;
    case SL_MEMORY_USAGE_CURRENT_HEAP_HIGH_WATERMARK:
      memory_value = sl_memory_get_heap_high_watermark();
      break;
    case SL_MEMORY_USAGE_INIT_USED_HEAP_SIZE:
      memory_value = init_used_heap;
      break;
    case SL_MEMORY_USAGE_INIT_HEAP_HIGH_WATERMARK:
      memory_value = init_heap_high_watermark;
      break;
    default:
      // Optionally handle error
      break;
  }
  return memory_value;
}
