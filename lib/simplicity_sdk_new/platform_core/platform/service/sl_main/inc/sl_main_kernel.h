/***************************************************************************//**
 * @file
 * @brief Main - Kernel Start Task Functions
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
#ifndef _SL_MAIN_KERNEL_H
#define _SL_MAIN_KERNEL_H

#include <stdbool.h>

/***************************************************************************//**
 * @addtogroup sl_main System Setup (sl_main)
 * @{
 ******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************//**
 * @brief Start the kernel.
 ******************************************************************************/
void sl_main_kernel_start(void);

/***************************************************************************//**
 * @brief User-defined function to determine if the start task should continue.
 *
 * @return  true if the start task should continue, false otherwise.
 *
 * @note By default the start task should not continue, but this function can be
 *       re-implemented to force the start task to continue running.
 ******************************************************************************/
bool sl_main_start_task_should_continue(void);

#ifdef __cplusplus
}
#endif

/** @} (end addtogroup sl_main) */

#endif // _SL_MAIN_KERNEL_H
