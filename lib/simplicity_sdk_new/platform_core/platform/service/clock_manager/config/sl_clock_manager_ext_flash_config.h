/***************************************************************************//**
 * @file
 * @brief Clock Manager - External Flash configuration file.
 *******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
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

// <<< Use Configuration Wizard in Context Menu >>>

#ifndef SL_CLOCK_MANAGER_EXT_FLASH_CONFIG_H
#define SL_CLOCK_MANAGER_EXT_FLASH_CONFIG_H

// <h> External Flash Settings

// <o SL_CLOCK_MANAGER_EXT_FLASH_MAX_FREQ> Max External Flash Frequency in Hz <90000000-160000000>
// <i> When external flash is present, the maximum flash frequency must be specified.
// <i> The flash will operate at the maximum frequency set.
// <i> Maximum frequency cannot be 0 Hz. This configuration is only
// <i> applicable when HFXO is enabled.
// <d> 0
#ifndef SL_CLOCK_MANAGER_EXT_FLASH_MAX_FREQ
#define SL_CLOCK_MANAGER_EXT_FLASH_MAX_FREQ  0
#endif

// </h>

// <<< end of configuration section >>>

#endif /* SL_CLOCK_MANAGER_EXTERNAL_FLASH_CONFIG_H */
