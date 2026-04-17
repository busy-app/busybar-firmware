/***************************************************************************/ /**
* @file sl_log_common_config.h
* @brief SL DEBUG LOGGER Config.
*******************************************************************************
* # License
* <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifndef SL_LOG_COMMON_CONFIG_H
#define SL_LOG_COMMON_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup sl_log_config SL Log Configuration
 * @{
 */

/**
 * @defgroup sl_log_levels Log Levels
 * @brief Log level definitions for filtering log messages
 * @{
 */

/** @brief Debug level logging */
#define SL_LOG_CONFIG_LEVEL_DEBUG 1
/** @brief Information level logging */
#define SL_LOG_CONFIG_LEVEL_INFO 2
/** @brief Warning level logging */
#define SL_LOG_CONFIG_LEVEL_WARN 3
/** @brief Error level logging */
#define SL_LOG_CONFIG_LEVEL_ERROR 4
/** @brief Crash level logging - most critical messages */
#define SL_LOG_CONFIG_LEVEL_CRASH 5
/** @brief No logging - all log messages are disabled */
#define SL_LOG_CONFIG_LEVEL_NONE 6

/** @} (end addtogroup sl_log_levels) */


/**
 * @defgroup sl_log_args Log Arguments
 * @brief Maximum number of arguments that can be passed to log functions
 * @{
 */

/** @brief No arguments supported */
#define SL_LOG_CONFIG_ARG0 0
/** @brief Up to 1 argument supported */
#define SL_LOG_CONFIG_ARG1 1
/** @brief Up to 2 arguments supported */
#define SL_LOG_CONFIG_ARG2 2
/** @brief Up to 3 arguments supported */
#define SL_LOG_CONFIG_ARG3 3

/** @} (end addtogroup sl_log_args) */

/**
 * @defgroup sl_log_uc_config UC Configuration Settings
 * @brief Silicon Labs Universal Configurator settings for the logging system
 * @{
 */

// <<< Use Configuration Wizard in Context Menu >>>
//  <e>DEBUG LOGGER UC Configuration
/**
 * @brief Enable Universal Configurator support for SL Log
 *
 * When enabled, allows configuration of the logging system through
 * Silicon Labs' Universal Configurator interface.
 *
 * @note Default value is 1 (enabled)
 */
#ifndef SL_LOG_ENABLE_UC_CONFIG
#define SL_LOG_ENABLE_UC_CONFIG 1
#endif

// <o SL_LOG_CONFIG_LEVEL_COMPILE_TIME> LOG_LEVEL
// <SL_LOG_CONFIG_LEVEL_NONE => NONE
// <SL_LOG_CONFIG_LEVEL_DEBUG => DEBUG
// <SL_LOG_CONFIG_LEVEL_INFO => INFO
// <SL_LOG_CONFIG_LEVEL_WARN => WARN
// <SL_LOG_CONFIG_LEVEL_ERROR => ERROR
// <i> Default: SL_LOG_CONFIG_LEVEL_INFO
/**
 * @brief Compile-time log level filter
 *
 * Sets the minimum log level that will be compiled into the application.
 * Log messages below this level will be completely removed at compile time,
 * reducing code size and runtime overhead.
 *
 * Valid values:
 * - SL_LOG_CONFIG_LEVEL_NONE: No logging
 * - SL_LOG_CONFIG_LEVEL_DEBUG: Debug level and above
 * - SL_LOG_CONFIG_LEVEL_INFO: Info level and above
 * - SL_LOG_CONFIG_LEVEL_WARN: Warning level and above
 * - SL_LOG_CONFIG_LEVEL_ERROR: Error level and above
 * - SL_LOG_CONFIG_LEVEL_CRASH: Only crash level messages
 *
 * @note Default: SL_LOG_CONFIG_LEVEL_INFO
 */
#define SL_LOG_CONFIG_LEVEL_COMPILE_TIME SL_LOG_CONFIG_LEVEL_INFO

// <o SL_LOG_CONFIG_ARG> CONFIG_MAX_ARGS
// <SL_LOG_CONFIG_ARG0 => 0
// <SL_LOG_CONFIG_ARG1 => 1
// <SL_LOG_CONFIG_ARG2 => 2
// <SL_LOG_CONFIG_ARG3 => 3
// <i> Default: 3
/**
 * @brief Maximum number of arguments supported in log messages
 *
 * Configures the maximum number of variable arguments that can be
 * passed to log functions. Higher values increase flexibility but
 * may consume more memory and processing time.
 *
 * Valid values:
 * - SL_LOG_CONFIG_ARG0: No arguments (string-only logging)
 * - SL_LOG_CONFIG_ARG1: Up to 1 argument
 * - SL_LOG_CONFIG_ARG2: Up to 2 arguments
 * - SL_LOG_CONFIG_ARG3: Up to 3 arguments
 *
 * @note Default: SL_LOG_CONFIG_ARG3 (3 arguments)
 */
#define SL_LOG_CONFIG_ARG SL_LOG_CONFIG_ARG3

// <o SL_LOG_NUMBER_OF_EVENTS> No of Logs <1-255>
// <i> Default: 128
/**
 * @brief Maximum number of log events that can be stored
 *
 * Defines the size of the internal log event buffer. This determines
 * how many log messages can be queued before older messages are
 * overwritten or transmission is required.
 *
 * @note Valid range: 1-255
 * @note Default: 128
 */
#define SL_LOG_NUMBER_OF_EVENTS 128 // Number of events

/**
 * @brief Timer instance used as timestamp counter
 *
 * Make sure TIMER instance size is 32bits.
 * Check datasheet for 32bits TIMERs.
 *
 * @note Valid range: 0 - 6
 * @note Default: 0
 */
// <o SL_LOG_CONFIG_TIMER_INSTANCE> TIMER Instance Used for timestamp counter
// <i> Default: 0
#define SL_LOG_CONFIG_TIMER_INSTANCE 0

/** @} (end addtogroup sl_log_uc_config) */

/** @} (end addtogroup sl_log_config) */

#ifdef __cplusplus
}
#endif

#endif
//  </e>
