/***************************************************************************//**
 * @file
 * @brief Bluetooth external bonding database configuration
 *******************************************************************************
 * # License
 * <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifndef SL_BT_EXTERNAL_BONDINGDB_CONFIG_H
#define SL_BT_EXTERNAL_BONDINGDB_CONFIG_H

// <<< Use Configuration Wizard in Context Menu >>>
// <q SL_BT_CONFIG_EXTERNAL_LOCAL_IRK> Store local IRK externally
// <i> Default: 0
// <i> Configures the Bluetooth stack to store the local IRK externally.
// <i> In this configuration the user application takes care of storing the
// <i> local IRK persistently. The Bluetooth stack sends events to the
// <i> application to store the local IRK  and to request the application
// <i> to pass the local IRK to the Bluetooth stack when the stack is stared.
// <i> When local IRK is stored externally, NVM is not required by the Bluetooth
// <i> stack to store bonding related data.
#define SL_BT_CONFIG_EXTERNAL_LOCAL_IRK     (0)
// <<< end of configuration section >>>

#endif
