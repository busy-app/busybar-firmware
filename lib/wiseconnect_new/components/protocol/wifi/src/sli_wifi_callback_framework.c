/***************************************************************************/ /**
 * @file  sli_wifi_callback_framework.c
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

#include "sli_wifi_callback_framework.h"
#include "sl_wifi_callback_framework.h"

static sl_wifi_event_group_t sli_get_event_group_from_event(sl_wifi_event_t event);

sli_wifi_callback_entry_t sli_registered_callbacks[SL_WIFI_EVENT_GROUP_COUNT];

static sl_wifi_event_group_t sli_get_event_group_from_event(sl_wifi_event_t event)
{
  //For TWT Events
  if (event == SL_WIFI_TWT_UNSOLICITED_SESSION_SUCCESS_EVENT || event == SL_WIFI_TWT_AP_REJECTED_EVENT
      || event == SL_WIFI_TWT_OUT_OF_TOLERANCE_EVENT || event == SL_WIFI_TWT_RESPONSE_NOT_MATCHED_EVENT
      || event == SL_WIFI_TWT_UNSUPPORTED_RESPONSE_EVENT || event == SL_WIFI_TWT_TEARDOWN_SUCCESS_EVENT
      || event == SL_WIFI_TWT_AP_TEARDOWN_SUCCESS_EVENT || event == SL_WIFI_TWT_FAIL_MAX_RETRIES_REACHED_EVENT
      || event == SL_WIFI_TWT_INACTIVE_DUE_TO_ROAMING_EVENT || event == SL_WIFI_TWT_INACTIVE_DUE_TO_DISCONNECT_EVENT
      || event == SL_WIFI_RESCHEDULE_TWT_SUCCESS_EVENT || event == SL_WIFI_TWT_INFO_FRAME_EXCHANGE_FAILED_EVENT
      || event == SL_WIFI_TWT_INACTIVE_NO_AP_SUPPORT_EVENT) {
    return SL_WIFI_TWT_RESPONSE_EVENTS;
  }
  // for STATS Events
  else if (event == SL_WIFI_STATS_EVENT || event == SL_WIFI_STATS_ASYNC_EVENT || event == SL_WIFI_STATS_ADVANCE_EVENT
           || event == SL_WIFI_STATS_TEST_MODE_EVENT || event == SL_WIFI_STATS_MODULE_STATE_EVENT) {
    return SL_WIFI_STATS_RESPONSE_EVENTS;
  } else if (event == SL_WIFI_TRANSCEIVER_RX_DATA_RECEIVE_CB || event == SL_WIFI_TRANSCEIVER_TX_DATA_STATUS_CB) {
    return SL_WIFI_TRANSCEIVER_EVENTS;
  } else {
    event = SL_WIFI_INVALID_EVENT;
  }
  return (sl_wifi_event_group_t)event;
}

sli_wifi_callback_entry_t *sli_get_callback_entry(sl_wifi_event_group_t group)
{
  if (group > SL_WIFI_EVENT_GROUP_COUNT) {
    group = sli_get_event_group_from_event((sl_wifi_event_t)group);
  }
  group = (group & ~SL_WIFI_EVENT_FAIL_INDICATION);
  if (group < 0 || group >= SL_WIFI_EVENT_GROUP_COUNT) {
    return NULL; // Return NULL if the group index is out of bounds
  }
  return &sli_registered_callbacks[group];
}

sl_status_t sli_wifi_set_callback(sl_wifi_event_group_t group,
                                  sl_wifi_callback_function_t function,
                                  sl_wifi_callback_function_v2_t function_v2,
                                  void *optional_arg)
{
  sli_wifi_callback_entry_t *entry = sli_get_callback_entry(group);

  if (entry == NULL) {
    return SL_STATUS_FAIL;
  }

  if (function != NULL) {
    entry->function = function;
    entry->arg      = optional_arg;
    return SL_STATUS_OK;
  }

  if (function_v2 != NULL) {
    entry->function_v2 = function_v2;
    entry->arg         = optional_arg;
    return SL_STATUS_OK;
  }

  return SL_STATUS_FAIL;
}
