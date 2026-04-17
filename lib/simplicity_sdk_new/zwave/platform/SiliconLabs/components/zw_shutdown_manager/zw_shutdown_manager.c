/*******************************************************************************
 * @file zw_shutdown_manager.c
 * @brief This file contains application specific power manager IDs and abstraction
 *******************************************************************************
 * # License
 * <b> Copyright 2025 Silicon Laboratories Inc. www.silabs.com </b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of the Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * https://www.silabs.com/about-us/legal/master-software-license-agreement
 * By installing, copying or otherwise using this software, you agree to the
 * terms of the MSLA.
 *
 ******************************************************************************/

#include <stdint.h>
#include "sl_component_catalog.h"

#include "sl_power_manager.h"
#include "sl_sleeptimer.h"
#include "sl_dcdc.h"
#include "sl_status.h"

#include "zpal_retention_register_private.h"
#include "zpal_misc.h"
#include "zpal_radio.h"
#include "zpal_log.h"

#include "zw_shutdown_manager.h"
#include "ZW_basis_api.h"

#include "em_burtc.h"

#ifdef SL_CATALOG_ZW_APP_TIMER_DEEP_SLEEP_PRESENT
#include "AppTimer.h"
#endif

/** The same handle is used for :
 * - Managing the specific case of SAPI command that needs to enable a lock for a specific time
 * - Managing the grace period (the short amount of time between the request to got to EM4 and the EMU_EnterEM4)
 *
 * It's made possible because there is no overlapping period for those timers.
 */
static sl_sleeptimer_timer_handle_t em4_sleeptimer_handle;
static volatile uint8_t em4_locks_counter = 0;
static bool temporary_lock_active = false; // Track if a temporary lock is currently active

void zw_shutdown_manager_callback(sl_power_manager_em_t from, sl_power_manager_em_t to)
{
  /*
     For never listening device the maximum available power mode is EM4.

     Condition to go to EM4:
     - The sl_power_manager is transitioning to EM2 or above
     - Z-Wave did not set an em4_lock

     note: the sleeptimer component provided by platform add an EM2 requirement.
     Because the sleeptimer is always running the power manager will never transition to EM3.
     In this case we therefore highjack power manager to go to EM4.
   */

  if ((SL_POWER_MANAGER_EM2 == from) && (SL_POWER_MANAGER_EM2 > to)) {
    //Wake up from EM2 or EM3 sleep
    sl_dcdc_exit_em2();
  }

  if (SL_POWER_MANAGER_EM2 == to) {
    if (0 != em4_locks_counter || ZPAL_RADIO_STATUS_OFF != zpal_radio_get_wakeup_status()) {
      sl_dcdc_setup_em2();
      return;
    }

    uint32_t tick_remaining = UINT32_MAX;
    if (SL_STATUS_OK != sl_sleeptimer_get_remaining_time_of_first_timer(SL_SLEEPTIMER_ANY_FLAG, &tick_remaining)) {
      sl_dcdc_setup_em2();
      return;
    }
    if (sl_sleeptimer_ms_to_tick(1000) > tick_remaining) {
      ZPAL_LOG_DEBUG(ZPAL_LOG_SHUTDOWN_MANAGER, "Timer expiring soon: %u ticks remaining\n", tick_remaining);
      sl_dcdc_setup_em2();
      return;
    }

#ifndef NDEBUG
    //highjack EM2 state to EM4 if no shutdown lock active & radio is off
    ZPAL_LOG_DEBUG(ZPAL_LOG_SHUTDOWN_MANAGER, "Going to EM4 (not effective in debug)\n");
    return;
#endif

    ZW_stack_shutdown();

    #ifdef SL_CATALOG_ZW_APP_TIMER_DEEP_SLEEP_PRESENT
    AppTimerDeepSleepPersistentSaveAll();
#endif

    // OS tick nor platform tick are preserved in EM4, store platform tick (more precise) to non-volatile memory for restoring deep sleep timer count at wakeup
    zpal_retention_register_write_private(ZPAL_RETENTION_REGISTER_PRIVATE_DEEP_SLEEP_TICK, sl_sleeptimer_get_tick_count());
    __DSB(); // Ensure BURAM write completion before WFI execution inside EM4 entry function

    BURTC_CompareSet(0U, 0); // Reset BURTC Compare register
    BURTC_IntClear(BURTC_IF_COMP); // Clear BURTC Compare interrupt register

    // Reprogram BURTC compare with next sleeptimer value
    uint32_t burtc_counter = BURTC_CounterGet();
    (void) sl_sleeptimer_get_remaining_time_of_first_timer(SL_SLEEPTIMER_ANY_FLAG, &tick_remaining);
    uint32_t compare_value = burtc_counter + tick_remaining;
    BURTC_CompareSet(0U, compare_value); // Set expected value to BURTC Compare register
    ZPAL_LOG_DEBUG(ZPAL_LOG_SHUTDOWN_MANAGER, "Reprogrammed BURTC compare to %lu (delta=%lu ticks)\n", compare_value, tick_remaining);

    sl_dcdc_setup_em4h();
    //No return from here until next wake up.
    sl_power_manager_enter_em4();
  }
}

static sl_power_manager_em_transition_event_handle_t pm_event_handle = { 0 };
static const sl_power_manager_em_transition_event_info_t pm_event_info =
{
  .event_mask = SL_POWER_MANAGER_EVENT_TRANSITION_ENTERING_EM0
                | SL_POWER_MANAGER_EVENT_TRANSITION_ENTERING_EM1
                | SL_POWER_MANAGER_EVENT_TRANSITION_ENTERING_EM2,
  .on_event = zw_shutdown_manager_callback
};

static void zpal_radio_status_callback(const zpal_radio_status_t state)
{
  static bool zpal_radio_shutdown_lock_state = false;
  switch (state) {
    case ZPAL_RADIO_STATUS_OFF:
      if (zpal_radio_shutdown_lock_state) {
        zw_shutdown_manager_release_lock();
        zpal_radio_shutdown_lock_state = false;
      }
      break;
    case ZPAL_RADIO_STATUS_ON:
    case ZPAL_RADIO_STATUS_FLIRS:
      if (!zpal_radio_shutdown_lock_state) {
        zw_shutdown_manager_add_lock();
        zpal_radio_shutdown_lock_state = true;
      }
      break;
    default:
      break;
  }
}
void zw_shutdown_manager_init(void)
{
  sl_power_manager_subscribe_em_transition_event(&pm_event_handle, &pm_event_info);
  zpal_radio_set_status_callback(zpal_radio_status_callback);
}

void zw_shutdown_manager_add_lock(void)
{
  em4_locks_counter++;
}

static void temporary_lock_revoke_callback(__attribute__((unused)) sl_sleeptimer_timer_handle_t *handle, __attribute__((unused)) void *contextData)
{
  // Only release the lock if it's actually active to prevent double-release
  if (temporary_lock_active) {
    zw_shutdown_manager_release_lock();
    temporary_lock_active = false;
  }
}

void zw_shutdown_manager_take_temporary_lock(uint32_t duration)
{
  // If a temporary lock is already active, just restart the timer and return early
  if (temporary_lock_active) {
    sl_sleeptimer_restart_timer_ms(&em4_sleeptimer_handle, duration, temporary_lock_revoke_callback, NULL, 0, 0);
    return;
  }

  // First time: acquire the lock
  zw_shutdown_manager_add_lock();
  temporary_lock_active = true;
  __attribute__((unused)) sl_status_t status = sl_sleeptimer_start_timer_ms(&em4_sleeptimer_handle, duration, temporary_lock_revoke_callback, NULL, 0, 0);
  assert(status == SL_STATUS_OK); // Verify timer started successfully
}

void zw_shutdown_manager_release_lock(void)
{
  if (em4_locks_counter > 0) {
    em4_locks_counter--;
  }
}

void zw_shutdown_manager_reset(void)
{
  em4_locks_counter = 0;
  temporary_lock_active = false;
}
