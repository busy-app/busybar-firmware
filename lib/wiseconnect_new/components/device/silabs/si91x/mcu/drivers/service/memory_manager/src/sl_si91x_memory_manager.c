/***************************************************************************/ /**
 * @file sl_si91x_memory_manager.c
 * @brief Si91x Memory Manager Wrapper Implementation
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

#include "sl_si91x_memory_manager.h"
#include "sl_status.h"
#include "stddef.h"
#include "sl_component_catalog.h"

#if defined(SL_CATALOG_FREERTOS_KERNEL_PRESENT)
#include "FreeRTOS.h"
#include "portable.h"

// Provide a weak stub for xPortResetHeapMinimumEverFreeHeapSize
// This function is only available in heap_4 and heap_5 implementations
// If the real implementation exists, it will override this weak stub
#if defined(__GNUC__) || defined(__clang__)
__attribute__((weak)) void xPortResetHeapMinimumEverFreeHeapSize(void)
{
  // Weak stub - does nothing if heap_4/heap_5 is not used
  // The real implementation in heap_4.c or heap_5.c will override this
}
#else
// For other compilers, declare as extern - user must ensure it exists if using heap_4/heap_5
extern void xPortResetHeapMinimumEverFreeHeapSize(void);
#endif
#endif

/***************************************************************************/ /**
  * @addtogroup si91x_memory_manager HAL Layer
  * @{
  ******************************************************************************/

/***************************************************************************/ /**
  * Si91x wrapper for getting current heap used size.
  *
  * @return Current amount of memory used in the heap (in bytes)
  ******************************************************************************/
size_t sl_si91x_memory_get_used_heap_size(void)
{
#if defined(SL_CATALOG_FREERTOS_KERNEL_PRESENT)
  const size_t total_heap = configTOTAL_HEAP_SIZE;
  const size_t free_heap  = xPortGetFreeHeapSize();

  if (total_heap >= free_heap) {
    return total_heap - free_heap;
  } else {
    return 0;
  }
#else
  return 0;
#endif
}

/***************************************************************************/ /**
  * Si91x wrapper for getting heap high watermark.
  *
  * @return Maximum amount of memory that has been used in the heap (in bytes)
  ******************************************************************************/
size_t sl_si91x_memory_get_heap_high_watermark(void)
{
#if defined(SL_CATALOG_FREERTOS_KERNEL_PRESENT)
  const size_t total_heap    = configTOTAL_HEAP_SIZE;
  const size_t min_ever_free = xPortGetMinimumEverFreeHeapSize();

  if (total_heap >= min_ever_free) {
    return total_heap - min_ever_free;
  } else {
    return 0;
  }
#else
  return 0;
#endif
}

/***************************************************************************/ /**
  * Si91x wrapper for getting current free heap size.
  *
  * @return Current amount of free memory in the heap (in bytes)
  ******************************************************************************/
size_t sl_si91x_memory_get_free_heap_size(void)
{
#if defined(SL_CATALOG_FREERTOS_KERNEL_PRESENT)
  return xPortGetFreeHeapSize();
#else
  return 0;
#endif
}

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
 ******************************************************************************/
void sl_si91x_memory_reset_heap_high_watermark(void)
{
#if defined(SL_CATALOG_FREERTOS_KERNEL_PRESENT)
  // FreeRTOS heap_4 and heap_5 implementations provide this function
  // It resets the minimum ever free heap size, which effectively
  // resets the high watermark (since watermark = total - min_free)

  // Call the FreeRTOS reset function
  // Note: This function is available in heap_4 and heap_5 implementations.
  // For heap_1, heap_2, heap_3, this will cause a linker error if those
  // implementations are used. Applications using those heap implementations
  // should not call this function or provide their own implementation.
  xPortResetHeapMinimumEverFreeHeapSize();
#else
  // No-op for non-FreeRTOS builds
  (void)0;
#endif
}
