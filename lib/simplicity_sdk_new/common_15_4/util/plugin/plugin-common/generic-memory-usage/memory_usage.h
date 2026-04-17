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
#ifndef MEMORY_USAGE_H
#define MEMORY_USAGE_H

#include <stdint.h>
#include "sl_enum.h"
#include "sl_status.h"

#define SL_OPENTHREAD_MEMORY_USAGE_CLI_ENABLE 1
/**
 * @brief Enum for memory usage data types
 */
typedef enum {
  SL_MEMORY_USAGE_TOTAL_HEAP_SIZE = 0x01,               // Total available heap size in bytes
  SL_MEMORY_USAGE_CURRENT_USED_HEAP_SIZE = 0x02,        // Used heap size in bytes at the time requested
  SL_MEMORY_USAGE_CURRENT_HEAP_HIGH_WATERMARK = 0x03,   // High watermark of the heap in bytes at the time requested
  SL_MEMORY_USAGE_INIT_USED_HEAP_SIZE = 0x04,           // Used heap size in bytes after system init
  SL_MEMORY_USAGE_INIT_HEAP_HIGH_WATERMARK = 0x05       // High watermark after system init
} sl_memory_usage_data_t;

/**
 * @brief Initializes the memory usage tracking during boot (after app init in main.c)
 */
void sl_memory_usage_init(void);

/**
 * @brief Gets memory usage data from sl_memory_manager APIs
 * @param type Type of memory usage data to be acquired
 * @return Data (in bytes) reflecting current or "boot" memory usage
 */
uint32_t sl_get_memory_usage_data(sl_memory_usage_data_t type);

#endif // MEMORY_USAGE_H
