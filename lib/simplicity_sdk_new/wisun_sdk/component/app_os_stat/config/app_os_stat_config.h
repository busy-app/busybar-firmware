/***************************************************************************//**
 * @file app_os_stat_config.h
 * @brief Application OS statistic configuration
 *******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
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
#ifndef APP_OS_STAT_CONFIG_H
#define APP_OS_STAT_CONFIG_H

#include "cmsis_os2.h"

/**************************************************************************//**
 * @defgroup APP_OS_STAT_CONFIG Configurations
 * @ingroup APP_OS_STAT
 * @{
 *****************************************************************************/

// <<< Use Configuration Wizard in Context Menu >>>

// // <h> App OS Statistic configuration

// <q APP_OS_STAT_VERBOSE_MODE_ENABLED> Enable verbose mode
// <i> Default value: 0
#define APP_OS_STAT_VERBOSE_MODE_ENABLED        0U ///< Enable verbose mode

// <q APP_OS_STAT_ASYNC_MODE_ENABLED> Enable async mode
// <i> Default value: 0
// <i> If enabled, max stack/heap usage statistics are printed asynchronously, when the values are changed
// <i> If disabled, stack/heap usage statistics are printed periodically
// <i> Hint: In a long test run without issues, checking the last entry includes everything and can be used to adjust the memory settings for the SDK threads'
#define APP_OS_STAT_ASYNC_MODE_ENABLED          0U ///< Enable async mode

// <o APP_OS_STAT_THREAD_PRIO> App OS statistic thread priority
// <i> Default: osPriorityHigh
#define APP_OS_STAT_THREAD_PRIO                 osPriorityHigh ///< App OS statistic thread priority

// <o APP_OS_STAT_STACK_SIZE_WORD> App OS statistic thread stack size in word
// <i> Default: 256
#define APP_OS_STAT_STACK_SIZE_WORD             256U ///< App OS statistic thread stack size in word

// <o APP_OS_STAT_UPDATE_PERIOD_TIME_MS> App OS statistic update period time
// <i> Default: 1000
#define APP_OS_STAT_UPDATE_PERIOD_TIME_MS       1000U ///< App OS statistic update period time

// <o APP_OS_STAT_MAX_THREAD_CNT> Maximum registerable thread count
// <i> Default: 20
#define APP_OS_STAT_MAX_THREAD_CNT              20U ///< Maximum registerable thread count

// <o APP_OS_STAT_RTT_LOG_CHANNEL> RTT Log channel for app os statistics
// <i> Default: 0
#define APP_OS_STAT_RTT_LOG_CHANNEL             0U ///< RTT Log channel for app os statistics

// <q APP_OS_STAT_THREAD_STACK_ENABLED> Enable thread stack usage statistic
// <i> default 1
#define APP_OS_STAT_THREAD_STACK_ENABLED        1U ///< Enable thread stack usage statistic

// <q APP_OS_STAT_HEAP_ENABLED> Enable heap usage statistic
// <i> default 1
#define APP_OS_STAT_HEAP_ENABLED                1U ///< Enable heap usage statistic

// <q APP_OS_STAT_MANUAL_REGISTER_THREADS> Enable manual thread registering
// <i> default 0
#define APP_OS_STAT_MANUAL_REGISTER_THREADS     0U ///< Enable manual thread registering

// </h>

// <<< end of configuration section >>>

/** @}*/

#endif // APP_OS_STAT_CONFIG_H
