/***************************************************************************/ /**
 * @file sli_wifi_command_engine.h
 * @brief
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

#ifndef SLI_WIFI_COMMAND_ENGINE_H
#define SLI_WIFI_COMMAND_ENGINE_H
#include "sl_status.h"
#include <stdint.h>

/**
 * @brief Defines the thread priority for the WLAN command engine.
 *
 * This macro sets the priority level for the WLAN command engine thread.
 * The default value is set to `osPriorityRealtime`, which is a real-time priority level.
 *
 * @note
 * - The priority level of this thread should be second highest after @ref SL_WLAN_EVENT_THREAD_PRIORITY among all the threads in the system.
 */
#ifndef SL_WLAN_COMMAND_ENGINE_THREAD_PRIORITY
#define SL_WLAN_COMMAND_ENGINE_THREAD_PRIORITY osPriorityRealtime
#endif

/**
 * @brief
 *  This function initializes the command engine and creates a thread to receive the TX and RX events.
 */
void sli_wifi_command_engine_init(void);

/**
 * @brief
 *   This function deinitialize the command engine and terminates the thread.
 */
void sli_wifi_command_engine_deinit(void);

#endif
