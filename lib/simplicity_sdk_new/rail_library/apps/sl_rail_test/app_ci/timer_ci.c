/***************************************************************************//**
 * @file
 * @brief This file is for testing the RAIL timer interface.
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

#include <stdio.h>
#include <string.h>

#include "app_common.h"

// Hardware-based, single timer functionality
// When the multiTimer is enabled, this API uses one instance of the
// software-based, multiTimer.
void printTimerStats(sl_cli_command_arg_t *args)
{
  bool enabled = sl_rail_is_timer_running(railHandle);
  bool expired = sl_rail_is_timer_expired(railHandle);
  sl_rail_time_t expirationTime = sl_rail_get_timer(railHandle);
  sl_rail_time_t currentTime = sl_rail_get_time(railHandle);

  responsePrint(sl_cli_get_command_string(args, 0),
                "timeUs:%u,ExpirationTime:%u,"
                "IsRunning:%s,IsExpired:%s",
                currentTime,
                expirationTime,
                (enabled ? "True" : "False"),
                (expired ? "True" : "False"));
}

void setTimer(sl_cli_command_arg_t *args)
{
  if (!inAppMode(NONE, sl_cli_get_command_string(args, 0))) {
    return;
  }

  uint32_t timeOut = sl_cli_get_argument_uint32(args, 0);
  sl_rail_time_mode_t mode = SL_RAIL_TIME_DELAY;

  // If this is absolute mode then switch the delay mode
  if (strcmp(sl_cli_get_argument_string(args, 1), "abs") == 0) {
    mode = SL_RAIL_TIME_ABSOLUTE;
  }

  sl_rail_cancel_timer(railHandle);
  if (sl_rail_set_timer(railHandle, timeOut, mode, &railtest_SwTimerExpired)
      != SL_RAIL_STATUS_NO_ERROR) {
    responsePrintError(sl_cli_get_command_string(args, 0), 0x40, "set_timer failed");
    return;
  }

  args->argc = sl_cli_get_command_count(args); /* only reference cmd str */
  printTimerStats(args);
}

void timerCancel(sl_cli_command_arg_t *args)
{
  if (inAppMode(NONE, sl_cli_get_command_string(args, 0))) {
    sl_rail_cancel_timer(railHandle);
    args->argc = sl_cli_get_command_count(args); /* only reference cmd str */
    printTimerStats(args);
  }
}

// Software-based, multi-timer functionality
#define NUM_MULTI_TIMERS      3
static sl_rail_multi_timer_t multiTimer[NUM_MULTI_TIMERS];

void printMultiTimerStats(sl_cli_command_arg_t *args)
{
  uint8_t index = sl_cli_get_argument_uint8(args, 0);
  if (index >= NUM_MULTI_TIMERS) {
    responsePrintError(sl_cli_get_command_string(args, 0), 0x10,
                       "Invalid multiTimer index - start with 0. Number of multiTimers is %d",
                       NUM_MULTI_TIMERS);
    return;
  }

  bool enabled = sl_rail_is_multi_timer_running(railHandle, &multiTimer[index]);
  bool expired = sl_rail_is_multi_timer_expired(railHandle, &multiTimer[index]);
  sl_rail_time_t expirationTime = sl_rail_get_multi_timer(railHandle, &multiTimer[index],
                                                          SL_RAIL_TIME_ABSOLUTE);
  sl_rail_time_t currentTime = sl_rail_get_time(railHandle);

  responsePrint(sl_cli_get_command_string(args, 0),
                "timeUs:%u,"
                "ExpirationTime:%u,"
                "IsRunning:%s,"
                "IsExpired:%s,"
                "MultiTimerIndex:%d",
                currentTime,
                expirationTime,
                (enabled ? "True" : "False"),
                (expired ? "True" : "False"),
                index);
}

void enableMultiTimer(sl_cli_command_arg_t *args)
{
  if (!inAppMode(NONE, sl_cli_get_command_string(args, 0))) {
    return;
  }

  bool enable = !!sl_cli_get_argument_uint8(args, 0);
  sl_rail_status_t status = sl_rail_config_multi_timer(railHandle, enable);
  if (status == SL_RAIL_STATUS_NO_ERROR) {
    responsePrint(sl_cli_get_command_string(args, 0),
                  "status:%s,numMultiTimers:%d",
                  (enable ? "enabled" : "disabled"),
                  NUM_MULTI_TIMERS);
  } else {
    responsePrintError(sl_cli_get_command_string(args, 0), 0x10, "Unable to configure the multiTimer.");
  }
}

static void railtest_MultiTimerExpired(sl_rail_multi_timer_t *tmr,
                                       sl_rail_time_t expectedTimeOfEvent,
                                       void *cbArg)
{
  (void)expectedTimeOfEvent;
  // Get pre-allocated RailAppEvent_t, which may be NULL if it failed
  // to be allocated or was cancelled.
  RailAppEvent_t *multitimer = (RailAppEvent_t *)cbArg;
  if (multitimer == NULL) {
    eventsMissed++;
    return;
  }
  tmr->cb_arg = NULL; // Prevent possible double-free

  // Other multitimer fields were filled in at time of timer setup
  // Note that 'expectedTimeOfEvent' parameter may not be 'now'
  // so we don't trust it.
  multitimer->multitimer.currentTime = sl_rail_get_time(railHandle);

  queueAdd(&railAppEventQueue, (void *)multitimer);
}

void setMultiTimer(sl_cli_command_arg_t *args)
{
  if (!inAppMode(NONE, sl_cli_get_command_string(args, 0))) {
    return;
  }

  uint8_t index = sl_cli_get_argument_uint8(args, 0);
  if (index >= NUM_MULTI_TIMERS) {
    responsePrintError(sl_cli_get_command_string(args, 0), 0x10,
                       "Invalid multiTimer index - start with 0. Number of multiTimers is %d",
                       NUM_MULTI_TIMERS);
    return;
  }

  uint32_t timeOut = sl_cli_get_argument_uint32(args, 1);
  sl_rail_time_mode_t mode = SL_RAIL_TIME_DELAY;

  // If this is absolute mode then switch the delay mode
  if (strcmp(sl_cli_get_argument_string(args, 2), "abs") == 0) {
    mode = SL_RAIL_TIME_ABSOLUTE;
  }

  // Is this timer still pending so can reuse its pRailAppEvent?
  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_CRITICAL();
  RailAppEvent_t *multitimer = (RailAppEvent_t *)(multiTimer[index].cb_arg);
  (void) sl_rail_cancel_multi_timer(railHandle, &multiTimer[index]);
  CORE_EXIT_CRITICAL();
  if (multitimer == NULL) {
    // Pre-allocate the RailAppEvent_t due to latency of allocating at expiration
    multitimer = (RailAppEvent_t *)sl_malloc(sizeof(RailAppEvent_t));
    if (multitimer != NULL) {
      multitimer->type = MULTITIMER;
      multitimer->multitimer.index = index;
      // multitimer->multitimer.expirationTime filled in below
      // and .currentTime filled in at actual expiration.
    }
  }
  if (sl_rail_set_multi_timer(railHandle,
                              &multiTimer[index],
                              timeOut,
                              mode,
                              &railtest_MultiTimerExpired,
                              (void *)multitimer)
      != SL_RAIL_STATUS_NO_ERROR) {
    if (multitimer != NULL) {
      multiTimer[index].cb_arg = NULL;
      sl_free((void *)multitimer);
    }
    responsePrintError(sl_cli_get_command_string(args, 0), 0x40, "set_multi_timer failed");
    return;
  }
  if (multitimer != NULL) {
    multitimer->multitimer.expirationTime
      = sl_rail_get_multi_timer(railHandle, &multiTimer[index],
                                SL_RAIL_TIME_ABSOLUTE);
  }

  args->argc = sl_cli_get_command_count(args); /* only reference cmd str */
  printMultiTimerStats(args);
}

void multiTimerCancel(sl_cli_command_arg_t *args)
{
  if (!inAppMode(NONE, sl_cli_get_command_string(args, 0))) {
    return;
  }

  uint8_t index = sl_cli_get_argument_uint8(args, 0);
  if (index >= NUM_MULTI_TIMERS) {
    responsePrintError(sl_cli_get_command_string(args, 0), 0x10,
                       "Invalid multiTimer index - start with 0. Number of multiTimers is %d",
                       NUM_MULTI_TIMERS);
    return;
  }

  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_CRITICAL();
  void *pRailAppEvent = multiTimer[index].cb_arg;
  if (pRailAppEvent != NULL) {
    multiTimer[index].cb_arg = NULL;
    sl_free(pRailAppEvent);
  }
  CORE_EXIT_CRITICAL();
  if (sl_rail_cancel_multi_timer(railHandle, &multiTimer[index]) == SL_RAIL_STATUS_NO_ERROR) {
    args->argc = sl_cli_get_command_count(args); /* only reference cmd str */
    printMultiTimerStats(args);
  } else {
    responsePrintError(sl_cli_get_command_string(args, 0), 0x10, "MultiTimer unable to cancel.");
  }
}
