/***************************************************************************//**
 * @file
 * @brief Implementation of Call stack usage statistics
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

#include "em_device.h"
#include "sl_memory_manager.h"
#include "slx_memory_manager_call_stack_usage.h"
#include "sl_component_catalog.h"

#if defined(SL_CATALOG_KERNEL_PRESENT)
#error "memory_manager_call_stack_usage component is not compatible with RTOS environments."
#endif

/*******************************************************************************
 *********************************   DEFINES   *********************************
 ******************************************************************************/
#define WATERMARK_VALUE 0xbeefbeef

/*******************************************************************************
 **************************   GLOBAL FUNCTIONS   *******************************
 ******************************************************************************/

/**************************************************************************//**
 * Calculate the call stack usage since the last watermark.
 *****************************************************************************/
size_t slx_memory_manager_get_call_stack_usage(void)
{
  // Get the current stack pointer and stack region boundaries.
  uint32_t *stack_pointer = (uint32_t *)__get_MSP();
  sl_memory_region_t stack_region = sl_memory_get_stack_region();
  uint32_t *stack_bottom = stack_region.addr;
  uint32_t *stack_top = (void *)((uint8_t *)stack_region.addr + stack_region.size);

  // Find the first non-watermarked value in the call stack.
  uint32_t *ptr;
  for (ptr = stack_bottom; ptr < stack_pointer; ptr++) {
    if (*ptr != WATERMARK_VALUE) {
      break;
    }
  }

  // Calculate and return the call stack usage.
  return (uint32_t)(stack_top - ptr) * sizeof(uint32_t);
}

/**************************************************************************//**
 * Watermark the unused portion of the call stack.
 *****************************************************************************/
void slx_memory_manager_watermark_call_stack(void)
{
  // Get the current stack pointer and stack region boundaries.
  uint32_t *stack_pointer = (uint32_t *)__get_MSP();
  sl_memory_region_t stack_region = sl_memory_get_stack_region();
  uint32_t *stack_bottom  = stack_region.addr;

  // Watermark the unused portion of the call stack.
  for (uint32_t *ptr = stack_pointer - sizeof(uint32_t); ptr >= stack_bottom; ptr--) {
    *ptr = WATERMARK_VALUE;
  }
}
