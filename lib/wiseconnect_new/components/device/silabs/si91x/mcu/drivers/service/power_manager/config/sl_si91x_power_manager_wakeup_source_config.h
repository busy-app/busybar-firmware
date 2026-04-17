/***************************************************************************/ /**
 * @file sl_si91x_power_manager_wakeup_source_config.h
 * @brief Power Manager Wakeup Source Configuration
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

#ifndef SL_SI91X_POWER_MANAGER_WAKEUP_SOURCE_CONFIG_H
#define SL_SI91X_POWER_MANAGER_WAKEUP_SOURCE_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

// <<< Use Configuration Wizard in Context Menu >>>

// <e>Calendar Wakeup
#define SL_ENABLE_CALENDAR_WAKEUP_SOURCE 0

// <q ENABLE_SECOND> Enable Second Wakeup Source
// <i> Default: 0
#define ENABLE_SECOND 0

// <q ENABLE_ALARM> Enable Alarm Wakeup Source
// <i> Default: 1
#define ENABLE_ALARM 0

// <o ALARM_TIME_MSEC> Alarm Time (in milliseconds)
// <i> Default: 5000
#define ALARM_TIME_MSEC 5000
// </e>

// <e>GPIO Wakeup
#define SL_ENABLE_GPIO_WAKEUP_SOURCE 0

// <e ENABLE_NPSS_GPIO_0> Enable NPSS GPIO 0
// <i> Default: 0
#define ENABLE_NPSS_GPIO_0 0

// <o NPSS_GPIO_0_POLARITY> Select NPSS GPIO_0 Polarity
//   <POLARITY_LOW=> Polarity Low
//   <POLARITY_HIGH=> Polarity High
// <i> Selection of NPSS polarity.
#define NPSS_GPIO_0_POLARITY POLARITY_LOW
//</e>

// <e ENABLE_NPSS_GPIO_1> Enable NPSS GPIO 1
// <i> Default: 0
#define ENABLE_NPSS_GPIO_1 0

// <o NPSS_GPIO_1_POLARITY> Select NPSS GPIO_1 Polarity
//   <POLARITY_LOW=> Polarity Low
//   <POLARITY_HIGH=> Polarity High
// <i> Selection of NPSS polarity.
#define NPSS_GPIO_1_POLARITY POLARITY_LOW
//</e>

// <e ENABLE_NPSS_GPIO_2> Enable NPSS GPIO 2
// <i> Default: 0
#define ENABLE_NPSS_GPIO_2 0

// <o NPSS_GPIO_2_POLARITY> Select NPSS GPIO_2 Polarity
//   <POLARITY_LOW=> Polarity Low
//   <POLARITY_HIGH=> Polarity High
// <i> Selection of NPSS polarity.
#define NPSS_GPIO_2_POLARITY POLARITY_LOW
// </e>

// <e ENABLE_NPSS_GPIO_3> Enable NPSS GPIO 3
// <i> Default: 0
#define ENABLE_NPSS_GPIO_3 0

// <o NPSS_GPIO_3_POLARITY> Select NPSS GPIO_3 Polarity
//   <POLARITY_LOW=> Polarity Low
//   <POLARITY_HIGH=> Polarity High
// <i> Selection of NPSS polarity.
#define NPSS_GPIO_3_POLARITY POLARITY_LOW
// </e>
// </e>

// <e>Deep Sleep Timer Wakeup
#define SL_ENABLE_DST_WAKEUP_SOURCE 0

// <o DST_WAKEUP_TIME> Sleep Time (in microseconds) <10000-4294967295>
// <i> Default: 10000
#define DST_WAKEUP_TIME 10000

// </e>

// <e> Wireless Wakeup
#define SL_ENABLE_WIRELESS_WAKEUP_SOURCE 1
// </e>

// <<< end of configuration section >>>

#ifdef __cplusplus
}
#endif

#endif // SL_SI91X_POWER_MANAGER_WAKEUP_SOURCE_CONFIG_H