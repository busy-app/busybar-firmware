/***************************************************************************//**
 * @file
 * @brief Provides API to create and configure HCI event filter for filtering
 * Extended Advertisment Reports using RSSI and UUID.
 *
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

#ifndef SL_BT_HCI_EXT_ADV_FILTER_CONFIG_H
#define SL_BT_HCI_EXT_ADV_FILTER_CONFIG_H

// <<< Use Configuration Wizard in Context Menu >>>

// <h> Filter Configuration
// <o SL_BT_HCI_CONFIGURE_FILTERING_OPCODE> <f.h>
// <d> 0xff11
// <i> Defines opcode that is used to identify Filter Configuration command
// <i> This must be chosen that it doesn't overlap with any other vendor
// specific command
#define SL_BT_HCI_CONFIGURE_FILTERING_OPCODE 0xff11
// <o.0> Enable RSSI filtering
// Defines filter configuration bit SL_BT_HCI_FILTER_CONFIG_RSSI_ENABLE 0
// Default: on
// <o.1> Enable filtering of 16-bit Service Data UUIDs
// Defines filter configuraion bit
// SL_BT_HCI_FILTER_CONFIG_SERVICE_DATA_UUID_16_BIT_ENABLE 1
// Default: off
// <o.2> Enable filtering of Incomplete Lists of 16-bit Service Class UUIDs
// Defines filter configuration bit
// SL_BT_HCI_FILTER_CONFIG_INCOMPLETE_SERVICE_CLASS_UUID_16_BIT_ENABLE 2
// Default: off
// <o.3> Enable filtering of Complete Lists of 16-bit Service Class UUIDs
// Defines filter configuration bit
// SL_BT_HCI_FILTER_CONFIG_CNOMPLETE_SERVICE_CLASS_UUID_16_BIT_ENABLE 3
// Default: off
// <o.4> Enable 32-bit Service Data UUID filtering
// Defines filter configuration bit
// SL_BT_HCI_FILTER_CONFIG_SERVICE_DATA_UUID_32_BIT_ENABLE 4
// Default: off
// <o.5> Enable filtering of Incomplete Lists of 32-bit Service Class UUIDs
// Defines filter configuration bit
// SL_BT_HCI_FILTER_CONFIG_INCOMPLETE_SERVICE_CLASS_UUID_32_BIT_ENABLE 5
// Default: off
// <o.6> Enable filtering of Complete Lists of 32-bit Service Class UUIDs
// Defines filter configuration bit
// SL_BT_HCI_FILTER_CONFIG_COMPLETE_SERVICE_CLASS_UUID_32_BIT_ENABLE 6
// Default: off
#define SL_BT_HCI_FILTER_CONFIG 1
// <o> RSSI filtering threshold value [dBm] <-120..-30>
// <i> Defines RSSI threshold value for RSSI filtering
// <i> If event's RSSI value is lower than the threshold, the event will be
// discarded
// <i> If event's RSSI value is equals or is higher than threshold, the event
// will be processed/send to the host
// <i> Default: -80
#define SL_BT_HCI_FILTER_RSSI_THRESHOLD -80
// <o> Number of 16-bit UUIDs to be included in the Advertisment Report <#*2>
// <i> Default: 0
#define SL_BT_HCI_FILTER_16BIT_UUID_ARRAY_LEN 0
// <o> Number of 32-bit UUIDs to be included in the Advertisment Reports <#*4>
// <i> Default: 0
#define SL_BT_HCI_FILTER_32BIT_UUID_ARRAY_LEN 0
// </h>

// <<< end of configuration section >>>

#endif // SL_BT_HCI_EXT_ADV_FILTER_CONFIG_H
