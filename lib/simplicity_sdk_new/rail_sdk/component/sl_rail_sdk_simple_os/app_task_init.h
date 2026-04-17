/***************************************************************************//**
 * @file app_task_init.h
 * @brief RAIL SDK - Simple OS Init Component
 *******************************************************************************
 * # License
 * <b>Copyright 2018 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifndef APP_TASK_INIT_H
#define APP_TASK_INIT_H

// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------
/**
 * \addtogroup rail_sdk_extension
 * @{
 */
/**
 * \addtogroup rail_sdk_simple_os
 * @{
 */
// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                          Public Function Declarations
// -----------------------------------------------------------------------------
/**
 * @brief Initialize the task that will run the proprietary task than includes,
 * the proprietary routines including app_init and app_process functions.
 */
void app_task_init(void);

/**
 * @brief Notify the kernel to allow the proprietary task to run, as it is
 * designed to wait for notifications or flags.
 */
void app_task_notify(void);

/**
 * @brief Print the sample app name with the OS which it is running on.
 *
 * @param[in] app_name Sample app name to be printed.
 */
void print_sample_app_name(const char* app_name);

#endif // APP_TASK_INIT_H
/** @} */ // end of rail_sdk_simple_os group
/** @} */ // end of extension group
