/***************************************************************************//**
 * @file
 * @brief Bluetooth CS configuration
 *******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * In no event will the authors be held liable for any damages arising from the
 * use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software in a
 *    product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 ******************************************************************************/

#ifndef SL_BT_CS_CONFIG_H
#define SL_BT_CS_CONFIG_H

// <<< Use Configuration Wizard in Context Menu >>>
// <o SL_BT_CONFIG_MAX_CS_CONFIGS_PER_CONNECTION> Maximum number of CS configurations per Bluetooth connection <1-4>
// <i> Default: 4
// <i> Define the number of CS configurations the application needs per Bluetooth connection.
#ifndef SL_BT_CONFIG_MAX_CS_CONFIGS_PER_CONNECTION
#define SL_BT_CONFIG_MAX_CS_CONFIGS_PER_CONNECTION     (4)
#endif

// <o SL_BT_CONFIG_MAX_CS_PROCEDURES> Maximum number of procedures per CS configuration <1-32>
// <i> Default: 2
// <i> Define the maximum number of simultaneous procedures the CS device supports.
#ifndef SL_BT_CONFIG_MAX_CS_PROCEDURES
#define SL_BT_CONFIG_MAX_CS_PROCEDURES     (2)
#endif

// <o SL_BT_CONFIG_CS_SYNC_MAX_ANTENNAS> Max number of antennas used for CS sync packets <1-4>.
// <i> Default: 0xFF
// <i> Define the maximum number of antennas used for CS sync packets.
// <i> When set to 0xFF, the controller will automatically determine the maximum from the ACI.
// <i> This configuration is relevant if the application is configuring CS Sync Antenna Selection
// <i> to non-static pattern.
#ifndef SL_BT_CONFIG_CS_SYNC_MAX_ANTENNAS
#define SL_BT_CONFIG_CS_SYNC_MAX_ANTENNAS  (0xFF)
#endif

// <<< end of configuration section >>>
#endif
