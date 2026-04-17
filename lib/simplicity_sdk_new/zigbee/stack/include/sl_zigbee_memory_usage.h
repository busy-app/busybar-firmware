/***************************************************************************//**
 * @file
 * @brief API declarations for memory usage statistics from sl_memory_manager.
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

#ifndef SL_ZIGBEE_MEMORY_USAGE_H
#define SL_ZIGBEE_MEMORY_USAGE_H

// NOTE for uint32_t
#include <stdint.h>

#include "sl_enum.h"
#include "sl_status.h"

/**
 * @brief defines the kind of memory usage statistic being acquired
 */
SL_ENUM(sl_zigbee_memory_usage_data_t) {
  TOTAL_HEAP_SIZE             = 0x01,  // gets the total available heap size in bytes
  CURRENT_USED_HEAP_SIZE      = 0x02,  // gets the used heap size in bytes at the time requested
  CURRENT_HEAP_HIGH_WATERMARK = 0x03,  // gets the "high watermark" of the heap (the highest the heap has been) in bytes at the time requested
  INIT_USED_HEAP_SIZE         = 0x04,  // gets the used heap size in bytes at the time after sl_system_init
  INIT_HEAP_HIGH_WATERMARK    = 0x05,  // gets the "high watermark" of the heap (the highest the heap has been) in bytes at the time after sl_system_init
};

/**
 * @brief Stores the memory usage data during "boot" (after app init in main.c)
 */
void sl_zigbee_memory_usage_init(void);

/**
 * @brief Gets memory usage data from sl_memory_manager APIs
 * @param type Type of memory usage data to be acquired
 * @return Data (in bytes) reflecting current or "boot" memory usage
 */
uint32_t sl_zigbee_get_memory_usage_data(sl_zigbee_memory_usage_data_t type);

#endif // SL_ZIGBEE_MEMORY_USAGE_H
