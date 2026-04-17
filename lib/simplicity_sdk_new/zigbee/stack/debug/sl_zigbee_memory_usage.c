/***************************************************************************//**
 * @file
 * @brief API definitions for memory usage statistics from sl_memory_manager.
 *******************************************************************************
 * # License
 * <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

#include <stddef.h>
#include <stdint.h>
#include <assert.h>
#include <stdbool.h>
#include "sl_zigbee_memory_usage.h"
#include "memory_usage.h"

#ifdef SL_CATALOG_MEMORY_MANAGER_PRESENT
#include "sl_memory_manager.h"
#endif

uint32_t sli_zigbee_stack_get_memory_usage_data(sl_zigbee_memory_usage_data_t memory_usage_data_type);

// IPC redirection function
uint32_t sli_zigbee_stack_get_memory_usage_data(sl_zigbee_memory_usage_data_t memory_usage_data_type)
{
  // Map Zigbee enum to generic enum
  sl_memory_usage_data_t generic_type;
  switch (memory_usage_data_type) {
    case TOTAL_HEAP_SIZE:
      generic_type = SL_MEMORY_USAGE_TOTAL_HEAP_SIZE;
      break;
    case CURRENT_USED_HEAP_SIZE:
      generic_type = SL_MEMORY_USAGE_CURRENT_USED_HEAP_SIZE;
      break;
    case CURRENT_HEAP_HIGH_WATERMARK:
      generic_type = SL_MEMORY_USAGE_CURRENT_HEAP_HIGH_WATERMARK;
      break;
    case INIT_USED_HEAP_SIZE:
      generic_type = SL_MEMORY_USAGE_INIT_USED_HEAP_SIZE;
      break;
    case INIT_HEAP_HIGH_WATERMARK:
      generic_type = SL_MEMORY_USAGE_INIT_HEAP_HIGH_WATERMARK;
      break;
    default:
      assert(false);
      generic_type = SL_MEMORY_USAGE_TOTAL_HEAP_SIZE;
      break;
  }
  return sl_get_memory_usage_data(generic_type);
}
