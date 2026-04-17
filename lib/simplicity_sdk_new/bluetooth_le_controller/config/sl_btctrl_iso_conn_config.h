/***************************************************************************//**
 * @file
 * @brief Bluetooth Connected Isochronous Channel configuration
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

#ifndef SL_BT_ISO_CONN_CONFIG_H
#define SL_BT_ISO_CONN_CONFIG_H

// <<< Use Configuration Wizard in Context Menu >>>

// <h> Bluetooth Connected Isochronous Channel Configuration

// <o SL_BT_CONFIG_MAX_CISES> Max number of connected isochronous streams <0-31>
// <i> Default: 2
// <i> Define the number of connected isochronous streams that the application needs to use concurrently.
#define SL_BT_CONFIG_MAX_CISES    (2)

// <o SL_BT_CONFIG_MAX_CIGS> Max number of connected isochronous groups <0-1>
// <i> Default: 1
// <i> Define the number of connected isochronous groups that the application needs to use concurrently.
#define SL_BT_CONFIG_MAX_CIGS     (1)

// </h> End Bluetooth Connected Isochronous Stream Configuration

// <<< end of configuration section >>>

#endif // SL_BT_ISO_CONN_CONFIG_H
