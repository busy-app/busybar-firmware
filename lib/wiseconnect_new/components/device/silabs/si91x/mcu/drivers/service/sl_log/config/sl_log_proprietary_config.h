/***************************************************************************/ /**
* @file sl_log_proprietary_config.h
* @brief SL DEBUG LOGGER Proprietary Config.
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

#ifndef SL_LOG_PROPRIETARY_CONFIG_H
#define SL_LOG_PROPRIETARY_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup sl_log_proprietary_config SL Log Proprietary Configuration
 * @{
 */

/**
 * @defgroup sl_log_uc_proprietary_config Proprietary Log UC Configuration Settings
 * @brief Silicon Labs Universal Configurator settings for the logging system
 * @{
 */

// <<< Use Configuration Wizard in Context Menu >>>
//  <e>DEBUG LOGGER Proprietary UC Configuration
/**
 * @brief Enable Universal Configurator support for SL Log
 *
 * When enabled, allows configuration of the logging system through
 * Silicon Labs' Universal Configurator interface.
 *
 * @note Default value is 1 (enabled)
 */
#ifndef SL_LOG_ENABLE_UC_PROPRIETARY_CONFIG
#define SL_LOG_ENABLE_UC_PROPRIETARY_CONFIG 1
#endif

/**
 * @defgroup sl_log_modes Proprietary Backend Modes
 * @brief Configuration modes for the proprietary log backend
 * @{
 */

/** @brief Buffer mode - log messages are stored in internal buffer */
#define SL_LOG_CONFIG_MODE_BUFFER 0
/** @brief Console mode - log messages are sent directly to console/UART */
#define SL_LOG_CONFIG_MODE_CONSOLE 1
/** @brief Host mode - log messages are sent to host interface */
#define SL_LOG_CONFIG_MODE_HOST 2

/** @} (end addtogroup sl_log_modes) */

// <o SL_LOG_CONFIG_MODE> PROPRIETARY_CONFIG_MODE
// <SL_LOG_CONFIG_MODE_BUFFER => Buffer Mode
// <SL_LOG_CONFIG_MODE_CONSOLE => Console Mode
// <SL_LOG_CONFIG_MODE_HOST => Host Mode
/**
 * @brief Proprietary backend configuration mode
 *
 *
 * Valid values:
 * - SL_LOG_CONFIG_MODE_BUFFER: Store messages in internal buffer
 * - SL_LOG_CONFIG_MODE_CONSOLE: Send messages directly to console/UART
 * - SL_LOG_CONFIG_MODE_HOST: Send messages to host interface
 *
 * @note Default: SL_LOG_CONFIG_MODE_HOST
 */
#define SL_LOG_CONFIG_MODE SL_LOG_CONFIG_MODE_HOST

/** @} (end addtogroup sl_log_uc_proprietary_config) */

/** @} (end addtogroup sl_log_proprietary_config) */

#ifdef __cplusplus
}
#endif

#endif
//  </e>
