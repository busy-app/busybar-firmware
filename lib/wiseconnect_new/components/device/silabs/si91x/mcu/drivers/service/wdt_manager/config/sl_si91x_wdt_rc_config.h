/***************************************************************************/ /**
 * @file  sl_si91x_wdt_rc_config.h
 * @brief Watchdog-Timer configuration file.
 *******************************************************************************
 * # License
 * <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licenser of this software is Silicon Laboratories Inc.
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
#ifndef SL_SI91X_WDT_RC_CONFIG_H
#define SL_SI91X_WDT_RC_CONFIG_H

#include "sl_si91x_watchdog_timer.h"
#include "sl_si91x_wdt_manager.h"
#include "sl_component_catalog.h"

// <<< Use Configuration Wizard in Context Menu >>>

// <h> Watchdog Timer UC Configuration

// <e> Watchdog Timer Configuration for M4
// <i> Enable: Peripheral configuration is taken straight from the
// configuration set in the Universal Configurator (UC).
// <i> Disable: If the application requires it to be modified during runtime, use the
// sl_si91x_watchdog_set_configuration API to modify the peripheral
// configuration.
// <i> Default: 1
// <i> Note: May also be defined by the WDT manager component based on device type; this define allows CMSIS Configuration Wizard to persist the option (v6).
#ifndef SL_WDT_MANAGER_PRESENT_M4_UC_RC
#define SL_WDT_MANAGER_PRESENT_M4_UC_RC 1
#endif

// <o SL_SI91X_WATCHDOG_MANAGER_TIMEOUT_PERIOD> WDT Timeout Interval in milliseconds
//   <SL_SI91X_WATCHDOG_MANAGER_TIMEOUT_INDEX_5 => 1
//   <SL_SI91X_WATCHDOG_MANAGER_TIMEOUT_INDEX_6 => 2
//   <SL_SI91X_WATCHDOG_MANAGER_TIMEOUT_INDEX_7 => 4
//   <SL_SI91X_WATCHDOG_MANAGER_TIMEOUT_INDEX_8 => 8
//   <SL_SI91X_WATCHDOG_MANAGER_TIMEOUT_INDEX_9 => 16
//   <SL_SI91X_WATCHDOG_MANAGER_TIMEOUT_INDEX_10 => 32
//   <SL_SI91X_WATCHDOG_MANAGER_TIMEOUT_INDEX_11 => 64
//   <SL_SI91X_WATCHDOG_MANAGER_TIMEOUT_INDEX_12 => 128
//   <SL_SI91X_WATCHDOG_MANAGER_TIMEOUT_INDEX_13 => 256
//   <SL_SI91X_WATCHDOG_MANAGER_TIMEOUT_INDEX_14 => 512
//   <SL_SI91X_WATCHDOG_MANAGER_TIMEOUT_INDEX_15 => 1024
//   <SL_SI91X_WATCHDOG_MANAGER_TIMEOUT_INDEX_16 => 2048
//   <SL_SI91X_WATCHDOG_MANAGER_TIMEOUT_INDEX_17 => 4096
//   <SL_SI91X_WATCHDOG_MANAGER_TIMEOUT_INDEX_18 => 8192
//   <SL_SI91X_WATCHDOG_MANAGER_TIMEOUT_INDEX_19 => 16384
//   <SL_SI91X_WATCHDOG_MANAGER_TIMEOUT_INDEX_20 => 32768
//   <SL_SI91X_WATCHDOG_MANAGER_TIMEOUT_INDEX_21 => 65536
//   <SL_SI91X_WATCHDOG_MANAGER_TIMEOUT_INDEX_22 => 131072
//   <SL_SI91X_WATCHDOG_MANAGER_TIMEOUT_INDEX_23 => 262144
//   <SL_SI91X_WATCHDOG_MANAGER_TIMEOUT_INDEX_24 => 524288
//   <SL_SI91X_WATCHDOG_MANAGER_TIMEOUT_INDEX_25 => 1048576
//   <SL_SI91X_WATCHDOG_MANAGER_TIMEOUT_INDEX_26 => 2097152
//   <SL_SI91X_WATCHDOG_MANAGER_TIMEOUT_INDEX_27 => 4194304
//   <SL_SI91X_WATCHDOG_MANAGER_TIMEOUT_INDEX_28 => 8388608
//   <SL_SI91X_WATCHDOG_MANAGER_TIMEOUT_INDEX_29 => 16777216
//   <SL_SI91X_WATCHDOG_MANAGER_TIMEOUT_INDEX_30 => 33554432
//   <SL_SI91X_WATCHDOG_MANAGER_TIMEOUT_INDEX_31 => 67108864
// <i> Selection of the time out interval. Please note that when the RC clock is used as the source for the LF-FSM, the timeout durations may vary due to the inherent frequency instability of the RC oscillator.
#define SL_SI91X_WATCHDOG_MANAGER_TIMEOUT_PERIOD SL_SI91X_WATCHDOG_MANAGER_TIMEOUT_INDEX_20

// </e>

// <e> Watchdog Timer Configuration for NWP
// <i> Default: 1
#define SL_WDT_MANAGER_PRESENT_NWP 1

// <o SL_SI91X_WATCHDOG_MANAGER_TIMEOUT_PERIOD_NWP> WDT Timeout Interval in seconds<20-255>
// <i> Specify the Timeout Interval for the watchdog timer in seconds.
// <i> Default: 32
#define SL_SI91X_WATCHDOG_MANAGER_TIMEOUT_PERIOD_NWP 32

// </e>

// </h>
// <<< end of configuration section >>>

#endif // SL_SI91X_WDT_RC_CONFIG_H
