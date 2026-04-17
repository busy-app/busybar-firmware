/***************************************************************************/ /**
 * @file  sli_wifi_callback_framework.h
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

#pragma once

#include "sl_wifi_device.h"

/**
 * @brief Forward declaration for legacy Wi-Fi event callback function pointer type of @ref sl_wifi_callback_function_t.
 */
typedef sl_status_t (*sl_wifi_callback_function_t)(sl_wifi_event_t event,
                                                   void *data,
                                                   uint32_t data_length,
                                                   void *optional_arg);

/**
 * @brief Forward declaration for enhanced Wi-Fi event callback function pointer type (with status code) of @ref sl_wifi_callback_function_v2_t.
 *
 */
typedef sl_status_t (*sl_wifi_callback_function_v2_t)(sl_wifi_event_t event,
                                                      sl_status_t status_code,
                                                      void *data,
                                                      uint32_t data_length,
                                                      void *optional_arg);

/**
 * @struct sli_wifi_callback_entry_t
 * @brief Structure representing an internal Wi-Fi callback entry.
 */
typedef struct {
  sl_wifi_callback_function_t function;       ///< Legacy callback function pointer (deprecated)
  sl_wifi_callback_function_v2_t function_v2; ///< Enhanced callback function pointer with status code support
  void *arg;                                  ///< User-provided argument passed to the callback function
} sli_wifi_callback_entry_t;

/***************************************************************************/
/**
 * @brief
 *   Retrieve the callback entry for a specified Wi-Fi event group.
 *
 * @details
 *   This function returns a pointer to the callback entry associated with the given
 *   Wi-Fi event group.
 *
 * @param[in] group
 *   Wi-Fi event group of type @ref sl_wifi_event_group_t for which to retrieve the callback entry.
 *   If the group value exceeds SL_WIFI_EVENT_GROUP_COUNT, the function will attempt to
 *   derive the event group from the event value. The SL_WIFI_EVENT_FAIL_INDICATION bit
 *   is automatically masked out during processing.
 *
 * @return
 *   Pointer to the callback entry of type @ref sli_wifi_callback_entry_t if the group is valid,
 *   NULL if the group index is out of bounds or invalid.
 *
 * @note
 *   The returned pointer points to an entry in the global registered_callbacks array
 *   and should not be freed by the caller.
 ******************************************************************************/
sli_wifi_callback_entry_t *sli_get_callback_entry(sl_wifi_event_group_t group);

/***************************************************************************/
/**
 * @brief
 *   Internal function to register a callback for a selected event group. All the individual Wi-Fi events related to specific group would be triggered via this group callback.
 *
 *   This function allows the user to register a callback function for a specific group of Wi-Fi events @ref sl_wifi_event_group_t.
 *   When any event within the specified group occurs, the registered callback function will be invoked, providing the event details and any associated data.
 * 
 * @param[in] group
 *   Group ID of the event for which the callback is registered. See @ref sl_wifi_event_group_t for possible values.
 * @param[in] function
 *    Function pointer to callback of type @ref sl_wifi_callback_function_t that would be invoked when an event in the specified group occurs.
 * @param[in] function_v2
 *    Function pointer to callback of type @ref sl_wifi_callback_function_v2_t that would be invoked when an event in the specified group occurs.
 * @param[in] optional_arg
 *   Optional user provided argument to pass additional context or information to the callback function. This would be passed back to callback handler of type @ref sl_wifi_callback_function_t.
 * @return
 *   sl_status_t. See [Status Codes](https://docs.silabs.com/gecko-platform/latest/platform-common/status)
 *   and [WiSeConnect Status Codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) for details.
 * @note
 *   Callbacks can be set only for event groups defined in @ref sl_wifi_event_group_t, not for individual events defined in @ref sl_wifi_event_t.
 ******************************************************************************/
sl_status_t sli_wifi_set_callback(sl_wifi_event_group_t group,
                                  sl_wifi_callback_function_t function,
                                  sl_wifi_callback_function_v2_t function_v2,
                                  void *optional_arg);
