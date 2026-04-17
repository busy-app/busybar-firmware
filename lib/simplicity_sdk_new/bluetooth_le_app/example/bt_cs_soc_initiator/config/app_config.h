/***************************************************************************//**
 * @file
 * @brief CS Initiator example configuration
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

#ifndef APP_CONFIG_H
#define APP_CONFIG_H

// <<< Use Configuration Wizard in Context Menu >>>

// <o SYSTEM_MIN_TX_POWER_DBM> Connection minimum TX Power <-127..20>
// <i> Connection minimum TX Power in dBm
// <i> Default: -3
#define SYSTEM_MIN_TX_POWER_DBM               -3

// <o SYSTEM_MAX_TX_POWER_DBM> Connection maximum TX Power <-127..20>
// <i> Connection minimum TX Power in dBm. Must be greater than the minimum value.
// <i> Default: 20
#define SYSTEM_MAX_TX_POWER_DBM               20

// <q ALWAYS_INIT_TRACE> Enable trace on init
// <i> If enabled, this option bypasses trace initialization with push button.
// <i> Default: 0
#define ALWAYS_INIT_TRACE                     0

// <q CS_INITIATOR_UART_LOG> Enable initiator log on UART
// <i> Default: 1
// <i> Enable or disable VCOM logging alongside RTT logging to save power.
#ifndef CS_INITIATOR_UART_LOG
#define CS_INITIATOR_UART_LOG                 1
#endif

// <s CS_INITIATOR_DEVICE_NAME> Device name
// <i> Default: "CS RFLCT"
#ifndef REFLECTOR_DEVICE_NAME
#define REFLECTOR_DEVICE_NAME "CS RFLCT"
#endif // REFLECTOR_DEVICE_NAME

// <q PREFER_AUTENTICATED_PAIRING> Prefer authenticated pairing
// <i> Setting this flag causes the device to prefer authenticated
// <i> pairing if both authenticated pairing and "Just Works" are available.
// <i> Default: 0
#ifndef PREFER_AUTENTICATED_PAIRING
#define PREFER_AUTENTICATED_PAIRING          0
#endif // PREFER_AUTENTICATED_PAIRING

// <q NEW_BOND_REQUIRES_PASSKEY> Require authentication for new bonds
// <i> Allow bonding without authentication or require authentication
// <i> (passkey) for bonding (Man-in-the-Middle protection)
// <i> Default: 0
#ifndef NEW_BOND_REQUIRES_PASSKEY
#define NEW_BOND_REQUIRES_PASSKEY            0
#endif // NEW_BOND_REQUIRES_PASSKEY

// <q ALLOW_BONDING> Allow bonding
// <i> Default: 0
#ifndef ALLOW_BONDING
#define ALLOW_BONDING                        0
#endif // ALLOW_BONDING

// <q DELETE_BONDINGS_ON_STARTUP> Deletes all bondings on startup
// <i> Default: 0
#ifndef DELETE_BONDINGS_ON_STARTUP
#define DELETE_BONDINGS_ON_STARTUP           0
#endif // DELETE_BONDINGS_ON_STARTUP

// <q ALLOW_DEBUG_KEYS> Allow debug keys
// <i> Allow debug keys from remote device (1) or
// <i> reject pairing if remote device uses debug keys (0).
// <i> Default: 0
#ifndef ALLOW_DEBUG_KEYS
#define ALLOW_DEBUG_KEYS                     0
#endif // ALLOW_DEBUG_KEYS

// <q SET_DEBUG_MODE> Set the device in debug mode
// <i> Set Security Manager in debug mode. In this mode, the secure connections
// <i> bonding uses known debug keys, so that the encrypted packet can be opened by
// <i> Bluetooth protocol analyzer. To disable the debug mode, restart the device.
// <i> Default: 0
#ifndef SET_DEBUG_MODE
#define SET_DEBUG_MODE                       0
#endif // SET_DEBUG_MODE

// <o CS_APP_PASSKEY> CS Initiator passkey <-1..999999>
// <i> Passkey used for setting own passkey for Passkey Entry method.
// <i> Depending on the capabilities this value will be set
// <i> using the sl_bt_sm_set_passkey() command and displayed,
// <i> or used as an input for sl_bt_sm_enter_passkey().
// <i> Default: 1234
#ifndef CS_APP_PASSKEY
#define CS_APP_PASSKEY                       1234
#endif // CS_APP_PASSKEY

// <o CS_APP_CAPABILITY> Capabilities
// <i> These values define the security management related I/O capabilities
// <i> supported by the device.
// <sl_bt_sm_io_capability_noinputnooutput=> No input no output
// <sl_bt_sm_io_capability_displayonly=> Display only
// <sl_bt_sm_io_capability_displayyesno=> Display with yes/no buttons
// <sl_bt_sm_io_capability_keyboardonly=> Keyboard only
// <sl_bt_sm_io_capability_keyboarddisplay=> Display with keyboard
// <i> Default: sl_bt_sm_io_capability_noinputnooutput
#ifndef CS_APP_CAPABILITY
#define CS_APP_CAPABILITY              sl_bt_sm_io_capability_noinputnooutput
#endif // CS_APP_CAPABILITY

// <o CS_INITIATOR_HEADER_LOG> Header log of measurements results <1..20>
// <i> Sets how many measurements are written between header logs
// <i> Default: 5
#define CS_INITIATOR_HEADER_LOG                5

// <<< end of configuration section >>>

#endif // APP_CONFIG_H
