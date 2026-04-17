/***************************************************************************/ /**
 * @file  sl_si91x_watchdog_timer_config.h
 * @brief Watchdog-Timer configuration file.
 *******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
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
#ifndef SL_SI91X_WATCHDOG_TIMER_CONFIG_H
#define SL_SI91X_WATCHDOG_TIMER_CONFIG_H

#include "sl_si91x_watchdog_timer.h"

// <<< Use Configuration Wizard in Context Menu >>>

// <e>WatchDog Timer UC Configuration
//  <i> Enable: Peripheral configuration is taken straight from the
//  configuration set in the universal configuration (UC).
//  <i> Disable: If the application demands it to be modified during runtime, use the
//  sl_si91x_watchdog_set_configuration API to modify the peripheral
//  configuration.
//  <i> Default: 1
#define WDT_TIMER_UC 1

// <h>Watchdog Timer Configuration

// <o SL_WDT_SYSTEM_RESET_TIME> System-reset time
//   <TIME_DELAY_6=> 1.9 ms
//   <TIME_DELAY_7=> 3.9 ms
//   <TIME_DELAY_8=> 7.8 ms
//   <TIME_DELAY_9=> 15.6 ms
//   <TIME_DELAY_10=> 31.2 ms
//   <TIME_DELAY_11=> 62.5 ms
//   <TIME_DELAY_12=> 125 ms
//   <TIME_DELAY_13=> 250 ms
//   <TIME_DELAY_14=> 500 ms
//   <TIME_DELAY_15=> 1000 ms
//   <TIME_DELAY_16=> 2000 ms
//   <TIME_DELAY_17=> 4000 ms
//   <TIME_DELAY_18=> 8000 ms
//   <TIME_DELAY_19=> 16000 ms
//   <TIME_DELAY_20=> 32000 ms
//   <TIME_DELAY_21=> 64000 ms
//   <TIME_DELAY_22=> 128000 ms
//   <TIME_DELAY_23=> 256000 ms
//   <TIME_DELAY_24=> 512000 ms
//   <TIME_DELAY_25=> 1024000 ms
//   <TIME_DELAY_26=> 2048000 ms
//   <TIME_DELAY_27=> 4096000 ms
//   <TIME_DELAY_28=> 8192000 ms
//   <TIME_DELAY_29=> 16384000 ms
//   <TIME_DELAY_30=> 32768000 ms
//   <TIME_DELAY_31=> 65536000 ms
// <i> Selection of System-reset time
#define SL_WDT_SYSTEM_RESET_TIME TIME_DELAY_17

// <o SL_WDT_INTERRUPT_TIME> Interrupt Time
//   <TIME_DELAY_6=> 1.9 ms
//   <TIME_DELAY_7=> 3.9 ms
//   <TIME_DELAY_8=> 7.8 ms
//   <TIME_DELAY_9=> 15.6 ms
//   <TIME_DELAY_10=> 31.2 ms
//   <TIME_DELAY_11=> 62.5 ms
//   <TIME_DELAY_12=> 125 ms
//   <TIME_DELAY_13=> 250 ms
//   <TIME_DELAY_14=> 500 ms
//   <TIME_DELAY_15=> 1000 ms
//   <TIME_DELAY_16=> 2000 ms
//   <TIME_DELAY_17=> 4000 ms
//   <TIME_DELAY_18=> 8000 ms
//   <TIME_DELAY_19=> 16000 ms
//   <TIME_DELAY_20=> 32000 ms
//   <TIME_DELAY_21=> 64000 ms
//   <TIME_DELAY_22=> 128000 ms
//   <TIME_DELAY_23=> 256000 ms
//   <TIME_DELAY_24=> 512000 ms
//   <TIME_DELAY_25=> 1024000 ms
//   <TIME_DELAY_26=> 2048000 ms
//   <TIME_DELAY_27=> 4096000 ms
//   <TIME_DELAY_28=> 8192000 ms
//   <TIME_DELAY_29=> 16384000 ms
//   <TIME_DELAY_30=> 32768000 ms
//   <TIME_DELAY_31=> 65536000 ms
// <i> Selection of Interrupt Time
#define SL_WDT_INTERRUPT_TIME TIME_DELAY_15

// <o SL_WDT_WINDOW_TIME> Window Time
//   <TIME_DELAY_6=> 1.9 ms
//   <TIME_DELAY_7=> 3.9 ms
//   <TIME_DELAY_8=> 7.8 ms
//   <TIME_DELAY_9=> 15.6 ms
//   <TIME_DELAY_10=> 31.2 ms
//   <TIME_DELAY_11=> 62.5 ms
//   <TIME_DELAY_12=> 125 ms
//   <TIME_DELAY_13=> 250 ms
//   <TIME_DELAY_14=> 500 ms
//   <TIME_DELAY_15=> 1000 ms
// <i> Selection of Window Time
#define SL_WDT_WINDOW_TIME TIME_DELAY_6

// </h>
// </e>
// <<< end of configuration section >>>

#if defined(SL_WDT_INTERRUPT_TIME)
watchdog_timer_config_t sl_watchdog_timer_config_handle = {
  .interrupt_time    = SL_WDT_INTERRUPT_TIME,
  .system_reset_time = SL_WDT_SYSTEM_RESET_TIME,
  .window_time       = SL_WDT_WINDOW_TIME,
};
#endif // SL_WDT_INTERRUPT_TIME

#endif // SL_SI91X_WATCHDOG_TIMER_CONFIG_H
