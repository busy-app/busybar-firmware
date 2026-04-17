/***************************************************************************/ /**
 * @file sl_si91x_memory_manager.h
 * @brief Si91x Memory Manager Wrapper API
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

#ifndef SL_SI91X_MEMORY_MANAGER_H
#define SL_SI91X_MEMORY_MANAGER_H

#include "sl_status.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************/ /**
 * @addtogroup sl_si91x_memory_manager Si91x Memory Manager
 * @{
 ******************************************************************************/

/***************************************************************************/ /**
 * Si91x wrapper for getting current heap used size.
 *
 * @details Retrieves the current amount of memory used in the heap.
 *          This function provides heap statistics for memory monitoring
 *          and debugging purposes.
 *
 * @return Current amount of memory used in the heap (in bytes)
 ******************************************************************************/
size_t sl_si91x_memory_get_used_heap_size(void);

/***************************************************************************/ /**
 * Si91x wrapper for getting heap high watermark.
 *
 * @details Retrieves the maximum amount of memory that has been used
 *          in the heap since system initialization or since the last
 *          watermark reset. This is useful for determining peak memory
 *          usage and optimizing memory allocation.
 *
 * @return Maximum amount of memory that has been used in the heap (in bytes)
 ******************************************************************************/
size_t sl_si91x_memory_get_heap_high_watermark(void);

/***************************************************************************/ /**
 * Si91x wrapper for getting current free heap size.
 *
 * @details Retrieves the current amount of free memory available in
 *          the heap. This can be used to check available memory before
 *          performing memory-intensive operations.
 *
 * @return Current amount of free memory in the heap (in bytes)
 ******************************************************************************/
size_t sl_si91x_memory_get_free_heap_size(void);

/***************************************************************************/ /**
 * Si91x wrapper for resetting heap high watermark.
 *
 * @details Resets the high watermark to the current heap usage, allowing
 *          new watermark measurements to start from the current state.
 *          This is useful when you want to measure memory usage for a
 *          specific operation or time period.
 *
 * @note For FreeRTOS heap implementations (heap_4, heap_5), this function
 *       uses xPortResetHeapMinimumEverFreeHeapSize() which resets the
 *       minimum ever free heap size. This effectively resets the high
 *       watermark since high watermark = total_heap - minimum_ever_free.
 *
 *       For other heap implementations, this function will be a no-op.
 *
 * @return None
 ******************************************************************************/
void sl_si91x_memory_reset_heap_high_watermark(void);

/** @} (end addtogroup sl_si91x_memory_manager) */

#ifdef __cplusplus
}
#endif

#endif // SL_SI91X_MEMORY_MANAGER_H
