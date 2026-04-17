/***************************************************************************//**
 * @brief Zigbee Application Framework sleep code.
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
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

#define CURRENT_MODULE_NAME "zigbee_app_framework_sleep"

#ifdef SL_COMPONENT_CATALOG_PRESENT
#include "sl_component_catalog.h"
#endif
#include PLATFORM_HEADER

#if defined(SL_CATALOG_POWER_MANAGER_PRESENT)
#include "hal.h"
#include "sl_sleeptimer.h"
#include "sl_zigbee_system_common.h"
#include "zigbee_sleep_config.h"
#include "sl_power_manager.h"
#ifdef SL_CATALOG_ZIGBEE_ZCL_FRAMEWORK_CORE_PRESENT
#include "app/framework/include/af.h"
#include "app/framework/util/af-main.h"
#else // !SL_CATALOG_ZIGBEE_ZCL_FRAMEWORK_CORE_PRESENT

#include "stack/core/sl_zigbee_multi_network.h"
#endif //SL_CATALOG_ZIGBEE_ZCL_FRAMEWORK_CORE_PRESENT

// This next line is a workaround for RAIL_LIB-6303 and can go away once that
// ticket is resolved
#include "sl_rail.h" //For sl_rail_initPowerManager

#define POWER_MANAGER_EVENTS_OF_INTEREST            \
  (SL_POWER_MANAGER_EVENT_TRANSITION_ENTERING_EM0   \
   | SL_POWER_MANAGER_EVENT_TRANSITION_LEAVING_EM0  \
   | SL_POWER_MANAGER_EVENT_TRANSITION_ENTERING_EM2 \
   | SL_POWER_MANAGER_EVENT_TRANSITION_LEAVING_EM2)

//------------------------------------------------------------------------------
// Forward declarations
bool sl_zigbee_af_idle_sleep_ok_to_idle_cb(void);
bool sl_zigbee_af_idle_sleep_ok_to_sleep_cb(uint32_t durationMs);

bool sli_zigbee_af_ok_to_idle_or_sleep(void);
bool sli_zigbee_af_check_stay_awake_when_not_joined(void);

//------------------------------------------------------------------------------
// Static and externs

// Get length of data in a callback queue.
extern uint16_t sl_zigbee_get_callback_queue_length(void);
// CAUTION : This variable may appear useless but it serves a purpose
// Power manager does not care where EM rewuirements are added or removed from
// Therefore it is mandatory that we check that we added the requirement before removing it
// Also note that indiscriminately adding EM1 requirement will cause overflow and make the
// microcontroller go into EM2 if EFM_ASSERT is not enabled. This results in a multitude of
// problems that may manifest as "BLE connection dies, Rx turns off etc"!!!
#if (!defined(_SILICON_LABS_32B_SERIES_3))
static bool em1_requirement_set = false;
#endif //!defined(_SILICON_LABS_32B_SERIES_3)
static sl_power_manager_em_transition_event_handle_t pm_handle;
static void energy_mode_transition_callback(sl_power_manager_em_t from, sl_power_manager_em_t to);
static sl_power_manager_em_transition_event_info_t pm_event_info =
{ POWER_MANAGER_EVENTS_OF_INTEREST,
  energy_mode_transition_callback };

bool sli_zigbee_af_stay_awake_when_not_joined = (SL_ZIGBEE_APP_FRAMEWORK_STAY_AWAKE_WHEN_NOT_JOINED ? true : false);
bool sli_zigbee_af_force_end_device_to_stay_awake = false;
uint32_t lastWakeupMs = 0;

#if !defined(SL_CATALOG_KERNEL_PRESENT)
static void wakeup_timer_callback(sl_sleeptimer_timer_handle_t* timer_id, void *user);
static sl_sleeptimer_timer_handle_t wakeup_timer_id;
#endif //!defined(SL_CATALOG_KERNEL_PRESENT)

#ifdef SL_CATALOG_ZIGBEE_FORCE_SLEEP_AND_WAKEUP_PRESENT
#include "force-sleep-wakeup.h"
#endif // SL_CATALOG_ZIGBEE_FORCE_SLEEP_AND_WAKEUP_PRESENT

void sli_zigbee_app_framework_sleep_init(void)
{
  sl_rail_init_power_manager();
#if (!defined(_SILICON_LABS_32B_SERIES_3))
  em1_requirement_set = true;
  sl_power_manager_add_em_requirement(SL_POWER_MANAGER_EM1);
#endif //!defined(_SILICON_LABS_32B_SERIES_3)
  sl_power_manager_subscribe_em_transition_event(&pm_handle, &pm_event_info);
}

#ifdef SL_CATALOG_ZIGBEE_FORCE_SLEEP_AND_WAKEUP_PRESENT
bool sli_zigbee_app_framework_is_force_sleep(void)
{
  if (sli_zigbee_app_framework_get_force_sleep_flag()) {
#if (!defined(_SILICON_LABS_32B_SERIES_3))
    // We NEED to go to EM2. Remove EM1 requirement if set and allow power manager to sleep
    if (em1_requirement_set) {
      sl_power_manager_remove_em_requirement(SL_POWER_MANAGER_EM1);
      em1_requirement_set = false;
    }
#endif //!defined(_SILICON_LABS_32B_SERIES_3)
    return true;
  }
  return false;
}
#endif //#ifdef SL_CATALOG_ZIGBEE_FORCE_SLEEP_AND_WAKEUP_PRESENT

#if !defined(SL_CATALOG_KERNEL_PRESENT)
// Only for bare-metal
// This function is called in the sl_power_manager_is_ok_to_sleep autogenerated code
// This function is the zigbee check to ensure it is okay to idle or sleep
// The return value of this function indicates that it is okay to go to EM1
// If this function removes all EM1 requirements, then power manager will allow going into EM2
bool sli_zigbee_app_framework_is_ok_to_sleep(void)
{
  uint32_t duration_ms = 0;

#ifdef SL_CATALOG_ZIGBEE_FORCE_SLEEP_AND_WAKEUP_PRESENT
  if (sli_zigbee_app_framework_is_force_sleep() == true) {
    return true;
  }
#endif //#ifdef SL_CATALOG_ZIGBEE_FORCE_SLEEP_AND_WAKEUP_PRESENT

  duration_ms = sli_zigbee_app_framework_set_pm_requirements_and_get_ms_to_next_wakeup();

  // Limit the value of sleep duration to what the sleep timer allows
  uint32_t max_millisecond_allowed_by_sleeptimer = sl_sleeptimer_get_max_ms32_conversion();
  duration_ms = SL_MIN(duration_ms, max_millisecond_allowed_by_sleeptimer);

  // Set up sleep timer to wake us up in time for the next event
  // Note that this duration is always over SL_ZIGBEE_APP_FRAMEWORK_MINIMUM_SLEEP_DURATION_MS
  // for EM2, but may not adhere to this minimum for EM1
  if ( duration_ms > 0 ) {
    assert(sl_sleeptimer_restart_timer_ms(&wakeup_timer_id,
                                          duration_ms,
                                          wakeup_timer_callback,
                                          NULL,
                                          0u,
                                          0u) == SL_STATUS_OK);
    return true;
  }

  // Not ok to go to EM1 or EM2
  return false;
}
static void wakeup_timer_callback(sl_sleeptimer_timer_handle_t* timer_id,
                                  void *user)
{
  (void)timer_id;
  (void)user;
}
sl_power_manager_on_isr_exit_t sli_zigbee_af_sleep_on_isr_exit(void)
{
  if (!sli_zigbee_app_framework_is_ok_to_sleep()) {
    return SL_POWER_MANAGER_WAKEUP;
  }
  return SL_POWER_MANAGER_IGNORE;
}
#endif //!DSL_CATALOG_KERNEL_PRESENT

static void energy_mode_transition_callback(sl_power_manager_em_t from,
                                            sl_power_manager_em_t to)
{
  (void)to;
  if (from == SL_POWER_MANAGER_EM2) { // Leaving EM2
    lastWakeupMs = halCommonGetInt32uMillisecondTick();
  }
}

// This function is called from both baremetal and RTOS applications
// Its primary purpose is to check all application idle/sleep override flags and
// the time-remaining to the next stack and application events to answer two questions:
// a) Is it okay to sleep ? If so, it removes the EM1 requirement
// b) How long can we idle/sleep for ? the function returns the duration in ms that sleep / idle is allowed for
// The baremetal and RTOS applications then use the returned duration to either start a sleep-timer OR
// yield the zigbee task. CAUTION: THIS FUNCTION DISABLES AND RE_ENABLES INTERRUPTS !!!

uint32_t sli_zigbee_app_framework_set_pm_requirements_and_get_ms_to_next_wakeup(void)
{
  uint32_t duration_ms = 0;

  // Check if sleep is permitted
  bool in_sleep_backoff;
  #if (SL_ZIGBEE_APP_FRAMEWORK_BACKOFF_SLEEP_MS > 0)
  in_sleep_backoff = (halCommonGetInt32uMillisecondTick() - lastWakeupMs) < SL_ZIGBEE_APP_FRAMEWORK_BACKOFF_SLEEP_MS;
  #else
  in_sleep_backoff = false;
  #endif
  bool ok_to_idle_or_sleep = sli_zigbee_af_ok_to_idle_or_sleep();

  // Check if the micro can go into EM2
  bool sleep_allowed = (ok_to_idle_or_sleep && !in_sleep_backoff);

  if (sleep_allowed) {
    duration_ms = sli_zigbee_af_ms_to_next_event();

    // If the sleep duration is below our minimum threshold, we don't bother
    // sleeping.  It takes time to shut everything down and bring everything
    // back up and, at some point, it becomes useless to sleep. If the
    // duration is long enough we will sleep. Otherwise, we will try to idle.

    // Check with application using sl_zigbee_af_idle_sleep_ok_to_sleep_cb
    // on whether it is ok to go to EM2
    if (duration_ms < SL_ZIGBEE_APP_FRAMEWORK_MINIMUM_SLEEP_DURATION_MS
        || !sl_zigbee_af_idle_sleep_ok_to_sleep_cb(duration_ms)) {
      duration_ms = 0;
#if (!defined(_SILICON_LABS_32B_SERIES_3))
      if (!em1_requirement_set) {
        // Prevent the system from entering em2
        sl_power_manager_add_em_requirement(SL_POWER_MANAGER_EM1);
        em1_requirement_set = true;
      }
#endif //!defined(_SILICON_LABS_32B_SERIES_3)
    }
#if (!defined(_SILICON_LABS_32B_SERIES_3))
    else {
      if (em1_requirement_set) {
        // Allow the system to enter em2
        sl_power_manager_remove_em_requirement(SL_POWER_MANAGER_EM1);
        em1_requirement_set = false;
      }
    }
#endif //!defined(_SILICON_LABS_32B_SERIES_3)
  }
#if (!defined(_SILICON_LABS_32B_SERIES_3))
  else { // Micro cannot go into EM2. So add EM1 requirement if it isnt already set
    if (!em1_requirement_set) {
      // Prevent the system from entering em2
      sl_power_manager_add_em_requirement(SL_POWER_MANAGER_EM1);
      em1_requirement_set = true;
    }
  }
#endif //!defined(_SILICON_LABS_32B_SERIES_3)

  // If durations_ms is 0 it means we could not enter em2, so we see how long we
  // can enter em1 instead. Ensure application permits EM1
  if (duration_ms == 0 && sl_zigbee_af_idle_sleep_ok_to_idle_cb()) {
#if (!defined(_SILICON_LABS_32B_SERIES_3))
    assert(em1_requirement_set);
#endif //!defined(_SILICON_LABS_32B_SERIES_3)
    duration_ms = sli_zigbee_af_ms_to_next_event();
  }

  return (duration_ms);
}

void sl_zigbee_af_force_end_device_to_stay_awake(bool stayAwake)
{
  sli_zigbee_af_force_end_device_to_stay_awake = stayAwake;
}

#if defined(SL_CATALOG_SIMPLE_BUTTON_PRESENT) && (SL_ZIGBEE_APP_FRAMEWORK_USE_BUTTON_TO_STAY_AWAKE == 1)
#include "sl_simple_button.h"
#include "sl_simple_button_instances.h"
void sl_button_on_change(const sl_button_t *handle)
{
  // If button 0 is pressed, stay awake
  if ( sl_button_get_state(handle) == SL_SIMPLE_BUTTON_PRESSED ) {
    sl_zigbee_af_force_end_device_to_stay_awake(SL_SIMPLE_BUTTON_INSTANCE(0) == handle);
    if (sli_zigbee_af_force_end_device_to_stay_awake) {
      sl_zigbee_wakeup_common_task();
    }
  }
}
#endif //defined(SL_CATALOG_SIMPLE_BUTTON_PRESENT) && (SL_ZIGBEE_APP_FRAMEWORK_USE_BUTTON_TO_STAY_AWAKE == 1)

bool sli_zigbee_af_ok_to_idle_or_sleep(void)
{
  if (sli_zigbee_af_force_end_device_to_stay_awake) {
    return false;
  }

  if (sli_zigbee_af_check_stay_awake_when_not_joined()) {
    return false;
  }

 #ifdef SL_CATALOG_ZIGBEE_ZCL_FRAMEWORK_CORE_PRESENT
  return (sl_zigbee_af_get_current_sleep_control_cb() != SL_ZIGBEE_AF_STAY_AWAKE);
 #else // SL_CATALOG_ZIGBEE_ZCL_FRAMEWORK_CORE_PRESENT
  return true;   // There is only idling on NCP, hence return true always.
 #endif  // SL_CATALOG_ZIGBEE_ZCL_FRAMEWORK_CORE_PRESENT
}

bool sli_zigbee_af_check_stay_awake_when_not_joined(void)
{
 #ifdef SL_CATALOG_ZIGBEE_ZCL_FRAMEWORK_CORE_PRESENT
  if (sli_zigbee_af_stay_awake_when_not_joined) {
    return sli_zigbee_af_stay_awake_network_state();
  }
  #endif // SL_CATALOG_ZIGBEE_ZCL_FRAMEWORK_CORE_PRESENT
  return false;
}

WEAK(bool sl_zigbee_af_idle_sleep_ok_to_idle_cb(void))
{
  return true;
}
WEAK(bool sl_zigbee_af_idle_sleep_ok_to_sleep_cb(uint32_t durationMs))
{
  (void)durationMs;
  return true;
}
#else //!SL_CATALOG_POWER_MANAGER_PRESENT - NO POWER MANAGER -
void sli_zigbee_app_framework_sleep_init(void)
{
}
bool sli_zigbee_app_framework_is_ok_to_sleep(void)
{
  return false;
}
#endif //#if defined(SL_CATALOG_POWER_MANAGER_PRESENT)
