/***************************************************************************/ /**
 * @file sl_si91x_power_manager_debug_config.h
 * @brief Power Manager Debug Configuration
 *******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifndef SL_SI91X_POWER_MANAGER_DEBUG_CONFIG_H
#define SL_SI91X_POWER_MANAGER_DEBUG_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "sl_si91x_power_manager.h"

// <<< Use Configuration Wizard in Context Menu >>>

// <h>Power Manager Debug Configuration

// <e SL_SI91X_POWER_MANAGER_DEBUG> Enable Power Manager debugging feature
// <i> Enable or disable debugging features (trace the different modules that have requirements).
// <i> Default: 0
#define SL_SI91X_POWER_MANAGER_DEBUG 0

// <o SL_SI91X_POWER_MANAGER_DEBUG_POOL_SIZE> Maximum numbers of requirements that can be logged
// <i> Default: 10
#define SL_SI91X_POWER_MANAGER_DEBUG_POOL_SIZE 10

// </e>

// </h>
// <<< end of configuration section >>>

#ifdef __cplusplus
}
#endif

#endif // SL_SI91X_POWER_MANAGER_DEBUG_CONFIG_H