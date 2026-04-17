/***************************************************************************//**
 * @file
 * @brief System Kernel Initialization.
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
#ifndef SL_SYSTEM_KERNEL_H
#define SL_SYSTEM_KERNEL_H

#include "sl_common.h"

/***************************************************************************//**
 * @addtogroup system System Setup (sl_system) (deprecated)
 * @{
 ******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Start the kernel
 *
 * API was deprecated in version 2025.6. Please migrate to the new sl_main API.
 * See the sl_main migration guide: https://docs.silabs.com/gecko-platform/latest/platform-service-sl-system-to-sl-main-migration-guide/
 */
SL_DEPRECATED_API_SDK_2025_6
void sl_system_kernel_start(void);

#ifdef __cplusplus
}
#endif

/** @} (end addtogroup system) */

#endif // SL_SYSTEM_KERNEL_H
