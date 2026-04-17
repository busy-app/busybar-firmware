/***************************************************************************/ /**
 * @file
 * @brief NCP UART configuration file.
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
#ifndef SL_NCP_UART_CONFIGURATION_H
#define SL_NCP_UART_CONFIGURATION_H

// <<< Use Configuration Wizard in Context Menu >>>

// <<< end of configuration section >>>

// <<< sl:start pin_tool >>>
// <gpio> SLEEP_CONFIRM
// $[GPIO_SLEEP_CONFIRM]
#define SI91X_NCP_SLEEP_CONFIRM_PORT gpioPortA
#define SI91X_NCP_SLEEP_CONFIRM_PIN  5

// [GPIO_SLEEP_CONFIRM]$

// <gpio> WAKE_INDICATOR
// $[GPIO_WAKE_INDICATOR]
#define SI91X_NCP_WAKE_INDICATOR_PORT gpioPortD
#define SI91X_NCP_WAKE_INDICATOR_PIN  2

// [GPIO_WAKE_INDICATOR]$

// <gpio> RESET
// $[GPIO_RESET]
#define SI91X_NCP_RESET_PORT gpioPortA
#define SI91X_NCP_RESET_PIN  6

// [GPIO_RESET]$

// <<< sl:end pin_tool >>>

#endif // SL_NCP_UART_CONFIGURATION_H