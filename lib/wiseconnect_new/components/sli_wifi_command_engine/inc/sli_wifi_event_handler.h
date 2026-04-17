/***************************************************************************/ /**
 * @file sli_wifi_event_handler.h
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

#ifndef SLI_WIFI_EVENT_HANDLER_H
#define SLI_WIFI_EVENT_HANDLER_H

/**
 * @brief
 *  Initialize the event handler.
 * @note
 * The definition of this function is provided by the specific implementation of the event handler.
 */
void sli_wifi_event_handler_init(void);

/**
 * @brief
 *  Deinitialize the event handler.
 * @note
 *  The definition of this function is provided by the specific implementation of the event handler.
 */
void sli_wifi_event_handler_deinit(void);

/**
 * @brief
 *  Command Engine calls this function to handle the event received.
 */
void sli_wifi_handle_event(uint32_t *event);

/**
 * @brief
 *  Command Engine calls this function to get the flags to wait on.
 */
uint32_t sli_wifi_event_handler_get_flags_to_wait_on(void);

/**
 * @brief
 *  Command Engine calls this function to wait on events.
 */
uint32_t sli_wifi_command_engine_wait_for_event(uint32_t event_mask, uint32_t timeout);

/**
 * @brief
 *  Command Engine calls this function to get the wait time.
 */
uint32_t sli_wifi_event_handler_get_wait_time(uint32_t *event);

/**
 * @brief
 *  Command Engine calls this function to set events.
 */
uint32_t sli_wifi_command_engine_set_event(uint32_t event_mask);

#endif
