/***************************************************************************//**
 * @file
 * @brief Code common to SOC and host to handle managing polling
 *******************************************************************************
 * # License
 * <b>Copyright 2018 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

#include "app/framework/include/af.h"
#include "app/framework/util/util.h"
#include "app/framework/util/common.h"
#include "app/framework/util/af-event.h"

#include "app/framework/plugin/end-device-support/end-device-support.h"

#if defined(SL_ZIGBEE_SCRIPTED_TEST)
  #include "app/framework/util/af-event-test.h"

  #define SL_ZIGBEE_AF_PLUGIN_END_DEVICE_SUPPORT_LONG_POLL_INTERVAL_SECONDS 15
  #define SL_ZIGBEE_AF_PLUGIN_END_DEVICE_SUPPORT_SHORT_POLL_INTERVAL_SECONDS 1
#endif

// *****************************************************************************
// Globals

typedef struct {
  sl_zigbee_af_application_task_t currentAppTasks;
  sl_zigbee_af_application_task_t wakeTimeoutBitmask;
  uint32_t longPollIntervalMs;
  bool longPollIntervalAlreadySet;
  uint16_t shortPollIntervalMs;
  bool shortPollIntervalAlreadySet;
  uint16_t wakeTimeoutMs;
  uint16_t lastAppTaskScheduleTime;
  sl_zigbee_af_event_poll_control_t pollControl;
} State;
static State states[SL_ZIGBEE_SUPPORTED_NETWORKS] = { { 0 } };

#if defined(ENABLE_POLL_COMPLETED_CALLBACK)
  #define ENABLE_POLL_COMPLETED_CALLBACK_DEFAULT true
#else
  #define ENABLE_POLL_COMPLETED_CALLBACK_DEFAULT false
#endif
bool sli_zigbee_af_enable_poll_completed_callback = ENABLE_POLL_COMPLETED_CALLBACK_DEFAULT;

#ifndef SL_ZIGBEE_AF_HAS_END_DEVICE_NETWORK
#ifndef SL_CATALOG_ZIGBEE_TEST_HARNESS_Z3_PRESENT
  #error "End device support only allowed on end devices."
#endif
#endif

// *****************************************************************************
// Functions

void sl_zigbee_af_end_device_support_init_cb(uint8_t init_level)
{
  (void)init_level;

  uint8_t i;
  for (i = 0; i < SL_ZIGBEE_SUPPORTED_NETWORKS; i++) {
    states[i].currentAppTasks = 0;
    states[i].wakeTimeoutBitmask = SL_ZIGBEE_AF_PLUGIN_END_DEVICE_SUPPORT_WAKE_TIMEOUT_BITMASK;
    if (false == states[i].longPollIntervalAlreadySet) {
      states[i].longPollIntervalMs =
        (SL_ZIGBEE_AF_PLUGIN_END_DEVICE_SUPPORT_LONG_POLL_INTERVAL_SECONDS
         * MILLISECOND_TICKS_PER_SECOND);
    }
    if (false == states[i].shortPollIntervalAlreadySet) {
      states[i].shortPollIntervalMs =
        (SL_ZIGBEE_AF_PLUGIN_END_DEVICE_SUPPORT_SHORT_POLL_INTERVAL_SECONDS
         * MILLISECOND_TICKS_PER_SECOND);
    }
    states[i].wakeTimeoutMs = (SL_ZIGBEE_AF_PLUGIN_END_DEVICE_SUPPORT_WAKE_TIMEOUT_SECONDS
                               * MILLISECOND_TICKS_PER_SECOND);
    states[i].lastAppTaskScheduleTime = 0;
    states[i].pollControl = SL_ZIGBEE_AF_LONG_POLL;
  }
}

void sl_zigbee_af_add_to_current_app_tasks_cb(sl_zigbee_af_application_task_t tasks)
{
  if (sli_zigbee_af_pro_is_current_network()
      && (SL_ZIGBEE_SLEEPY_END_DEVICE <= sli_zigbee_af_current_zigbee_pro_network->nodeType
          || tasks == SL_ZIGBEE_AF_FORCE_SHORT_POLL_FOR_PARENT_CONNECTIVITY)) {
    // Allow short poll on non-sleepy/SL_ZIGBEE_END_DEVICE as well only if device is
    // in SL_ZIGBEE_AF_FORCE_SHORT_POLL_FOR_PARENT_CONNECTIVITY task.
    State *state = &states[sl_zigbee_get_current_network()];
    state->currentAppTasks |= tasks;
    if (tasks & state->wakeTimeoutBitmask) {
      state->lastAppTaskScheduleTime = halCommonGetInt16uMillisecondTick();
    }
  }
}

void sl_zigbee_af_remove_from_current_app_tasks_cb(sl_zigbee_af_application_task_t tasks)
{
  State *state = &states[sl_zigbee_get_current_network()];
  state->currentAppTasks &= (~tasks);
}

uint32_t sl_zigbee_af_get_current_app_tasks_cb(void)
{
  State *state = &states[sl_zigbee_get_current_network()];
  return state->currentAppTasks;
}

uint32_t sl_zigbee_af_get_long_poll_interval_ms_cb(void)
{
  State *state = &states[sl_zigbee_get_current_network()];
  return state->longPollIntervalMs;
}

uint32_t sl_zigbee_af_get_long_poll_interval_qs_cb(void)
{
  return (sl_zigbee_af_get_long_poll_interval_ms_cb()
          / MILLISECOND_TICKS_PER_QUARTERSECOND);
}

void sl_zigbee_af_set_long_poll_interval_ms_cb(uint32_t longPollIntervalMs)
{
  State *state = &states[sl_zigbee_get_current_network()];
  state->longPollIntervalMs = longPollIntervalMs;
  state->longPollIntervalAlreadySet = true;
}

void sl_zigbee_af_set_long_poll_interval_qs_cb(uint32_t longPollIntervalQs)
{
  sl_zigbee_af_set_long_poll_interval_ms_cb(longPollIntervalQs
                                            * MILLISECOND_TICKS_PER_QUARTERSECOND);
}

uint16_t sl_zigbee_af_get_short_poll_interval_ms_cb(void)
{
  State *state = &states[sl_zigbee_get_current_network()];
  return state->shortPollIntervalMs;
}

uint16_t sl_zigbee_af_get_short_poll_interval_qs_cb(void)
{
  return (sl_zigbee_af_get_short_poll_interval_ms_cb()
          / MILLISECOND_TICKS_PER_QUARTERSECOND);
}

void sl_zigbee_af_set_short_poll_interval_ms_cb(uint16_t shortPollIntervalMs)
{
  State *state = &states[sl_zigbee_get_current_network()];
  state->shortPollIntervalMs = shortPollIntervalMs;
  state->shortPollIntervalAlreadySet = true;
}

void sl_zigbee_af_set_short_poll_interval_qs_cb(uint16_t shortPollIntervalQs)
{
  sl_zigbee_af_set_short_poll_interval_ms_cb(shortPollIntervalQs
                                             * MILLISECOND_TICKS_PER_QUARTERSECOND);
}

#ifdef EZSP_HOST
  #define sl_zigbee_ok_to_long_poll() true
#endif

uint32_t sl_zigbee_af_get_current_poll_interval_ms_cb(void)
{
  if (sli_zigbee_af_pro_is_current_network()
      && (SL_ZIGBEE_SLEEPY_END_DEVICE <= sli_zigbee_af_current_zigbee_pro_network->nodeType
          || (sl_zigbee_af_get_current_app_tasks_cb() & SL_ZIGBEE_AF_FORCE_SHORT_POLL_FOR_PARENT_CONNECTIVITY))) {
    // Allow short poll on non-sleepy/SL_ZIGBEE_END_DEVICE as well only if device is
    // in SL_ZIGBEE_AF_FORCE_SHORT_POLL_FOR_PARENT_CONNECTIVITY task.
    State *state = &states[sl_zigbee_get_current_network()];
    if (elapsedTimeInt16u(state->lastAppTaskScheduleTime,
                          halCommonGetInt16uMillisecondTick())
        > state->wakeTimeoutMs) {
      state->currentAppTasks &= ~state->wakeTimeoutBitmask;
    }
    if (!sl_zigbee_ok_to_long_poll()
        || state->currentAppTasks != 0
        || sl_zigbee_af_get_current_poll_control_cb() == SL_ZIGBEE_AF_SHORT_POLL) {
      return sl_zigbee_af_get_short_poll_interval_ms_cb();
    }
  }
  return sl_zigbee_af_get_long_poll_interval_ms_cb();
}

uint32_t sl_zigbee_af_get_current_poll_interval_qs_cb(void)
{
  return (sl_zigbee_af_get_current_poll_interval_ms_cb()
          / MILLISECOND_TICKS_PER_QUARTERSECOND);
}

uint16_t sl_zigbee_af_get_wake_timeout_ms_cb(void)
{
  State *state = &states[sl_zigbee_get_current_network()];
  return state->wakeTimeoutMs;
}

uint16_t sl_zigbee_af_get_wake_timeout_qs_cb(void)
{
  return (sl_zigbee_af_get_wake_timeout_ms_cb()
          / MILLISECOND_TICKS_PER_QUARTERSECOND);
}

void sl_zigbee_af_set_wake_timeout_ms_cb(uint16_t wakeTimeoutMs)
{
  State *state = &states[sl_zigbee_get_current_network()];
  state->wakeTimeoutMs = wakeTimeoutMs;
}

void sl_zigbee_af_set_wake_timeout_qs_cb(uint16_t wakeTimeoutQs)
{
  sl_zigbee_af_set_wake_timeout_ms_cb(wakeTimeoutQs
                                      * MILLISECOND_TICKS_PER_QUARTERSECOND);
}

sl_zigbee_af_application_task_t sl_zigbee_af_get_wake_timeout_bitmask_cb(void)
{
  State *state = &states[sl_zigbee_get_current_network()];
  return state->wakeTimeoutBitmask;
}

void sl_zigbee_af_set_wake_timeout_bitmask_cb(sl_zigbee_af_application_task_t tasks)
{
  State *state = &states[sl_zigbee_get_current_network()];
  state->wakeTimeoutBitmask = tasks;
}

#include "zap-event.h"

sl_zigbee_af_event_poll_control_t sl_zigbee_af_get_current_poll_control_cb(void)
{
  uint8_t networkIndex = sl_zigbee_get_current_network();
  sl_zigbee_af_event_poll_control_t pollControl = states[networkIndex].pollControl;
#if SL_ZIGBEE_AF_GENERATED_UC_EVENT_CONTEXT_COUNT > 0
  uint8_t i;
  for (i = 0; i < sli_zigbee_app_event_context_length; i++) {
    sl_zigbee_event_context_t *context = &sli_zigbee_app_event_context[i];
    if (networkIndex == sl_zigbee_af_network_index_from_endpoint(context->endpoint)
        && sl_zigbee_af_event_is_scheduled(context->event)
        && pollControl < context->pollControl) {
      pollControl = context->pollControl;
    }
  }
#endif
  return pollControl;
}

sl_zigbee_af_event_poll_control_t sl_zigbee_af_get_default_poll_control_cb(void)
{
  State *state = &states[sl_zigbee_get_current_network()];
  return state->pollControl;
}

void sl_zigbee_af_set_default_poll_control_cb(sl_zigbee_af_event_poll_control_t pollControl)
{
  State *state = &states[sl_zigbee_get_current_network()];
  state->pollControl = pollControl;
}
