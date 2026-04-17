/***************************************************************************//**
 * @file
 * @brief  Memory Manager's call stack usage API
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

#ifndef SLX_MEMORY_MANAGER_CALL_STACK_USAGE_H
#define SLX_MEMORY_MANAGER_CALL_STACK_USAGE_H

#ifdef __cplusplus
extern "C" {
#endif

/** @cond DOXYGEN_INCLUDE_INTERNAL */

/***************************************************************************//**
 * @addtogroup memory_manager_call_stack_usage Memory Manager Call Stack Usage Statistics.
 * @{
 *
 * @details Silicon Labs experimental API to get call stack usage statistics.
 *
 * This utility provides a method to get the actual call stack usage of a use
 * case. Typically it should be used in the following sequence:
 *
 * 1. Right after the application is booted and before performing a use case,
 * watermark the call stack with @ref slx_memory_manager_watermark_call_stack().
 *
 * 2. Execute the use case for which you want to measure the call stack usage.
 *
 * 3. Use @ref slx_memory_manager_get_call_stack_usage() to get the usage number.
 ******************************************************************************/

/***************************************************************************//**
 * Watermark the call stack. Watermark the call stack by writing a known value
 * to the unused space.
 *
 * @note Does not work in RTOS environments as each task has its own stack.
 ******************************************************************************/
void slx_memory_manager_watermark_call_stack(void);

/***************************************************************************//**
 * Get the call stack usage since the last watermark.
 *
 * @note Does not work in RTOS environments as each task has its own stack.
 *
 * @return Call stack usage in bytes.
 ******************************************************************************/
size_t slx_memory_manager_get_call_stack_usage(void);

/** @endcond */ // DOXYGEN_INCLUDE_INTERNAL

#ifdef __cplusplus
}
#endif

#endif // SLX_MEMORY_MANAGER_CALL_STACK_USAGE_H
