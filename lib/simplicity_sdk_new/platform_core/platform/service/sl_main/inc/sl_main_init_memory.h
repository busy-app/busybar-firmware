/***************************************************************************//**
 * @file
 * @brief Main Kernel Initialization.
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
#ifndef _SL_MAIN_INIT_MEMORY_H
#define _SL_MAIN_INIT_MEMORY_H

/***************************************************************************//**
 * @addtogroup sl_main System Setup (sl_main)
 * @brief System Setup (sl_main)
 * @details
 * ### Main Init memory
 *
 * Main memory init provides a function to do permanent memory allocations:
 *
 *   - sli_allocate_permanent_memory()
 *
 * Handlers can be registered
 * for the following events using the Event Handler API provided by the
 * Event Handler component:
 *
 *   - driver_permanent_allocation   -> sli_driver_permanent_allocation()
 *   - service_permanent_allocation  -> sli_service_permanent_allocation()
 *   - stack_permanent_allocation    -> sli_stack_permanent_allocation()
 *   - internal_permanent_allocation -> sli_internal_permanent_allocation()
 *
 * After the handlers are called, the function app_permanent_memory_alloc()
 * is called to allocate permanent memory blocks for the application.
 *
 * This ensures an optimal RAM placement for efficient memory usage.
 * @{
 *
 ******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************//**
 * @brief User-defined function for initializing application memory allocations.
 *
 * @note Will be automatically called after silicon labs platform
 *       and stacks components are initialized.
 ******************************************************************************/
void app_permanent_memory_alloc(void);

#ifdef __cplusplus
}
#endif

/** @} (end addtogroup sl_main) */

#endif // _SL_MAIN_INIT_MEMORY_H
