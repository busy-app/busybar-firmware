/**
 * @file
 * Offers complete Power Management commands for Silabs targets.
 * Strong implementations using Silabs-specific APIs (Power Manager, Shutdown Manager, App Timers).
 * @attention Must be linked for Silabs build targets only.
 * @copyright 2022 Silicon Laboratories Inc.
 */
#include <stdint.h>
#include "cmd_handlers.h"
#include "SerialAPI.h"
#include "app.h"
#include "utils.h"
#include "sl_power_manager.h"
#ifdef SL_CATALOG_ZW_SHUTDOWN_MANAGER_PRESENT
#include "zw_shutdown_manager.h"
#endif
#include "SwTimer.h"
#include "AppTimer.h"
#include "zpal_radio.h"
#include "zpal_log.h"

static SSwTimer mWakeupTimer = { 0 }; // Timer for wakeup after sleep timeout
static zpal_radio_stay_awake_id_t sapi_stay_awake_id = 0;
static bool sapi_shutdown_manager_lock_acquired = false; // Track if permanent IO lock has been acquired

/**
 * @brief wakeup after sleep timeout event
 *
 * @param pTimer Timer connected to this method
 */
static void ZCB_WakeupTimeout(__attribute__((unused)) SSwTimer *pTimer)
{
  ZPAL_LOG_DEBUG(ZPAL_LOG_APP, "ZCB_WakeupTimeout\n");
}

void cmds_power_management_init(void)
{
  AppTimerDeepSleepPersistentRegister(&mWakeupTimer, false, ZCB_WakeupTimeout);   // register for event jobs timeout event
}

// Implementations for power management commands
ZW_ADD_CMD(FUNC_ID_POWER_MANAGEMENT_STAY_AWAKE)
{
  uint8_t power_lock_type = frame->payload[0];
#if defined(SL_CATALOG_POWER_MANAGER_NO_DEEPSLEEP_PRESENT)
  if (POWER_LOCK_TYPE_IO == power_lock_type) {
    return; // Ignore Power management requests for Controller firmwares
  }
#endif
  /* HOST->ZW: Power Lock type | Power Lock timeout (4 bytes MSB first) | Wake Up Timer timeout (4 bytes MSB first) */
  /* According to spec: Power Lock type 0 for Radio and 1 for IO */
#define POWER_MANAGEMENT_STAY_AWAKE_MIN_FRAME_LENGTH 9

  if (frame_payload_len(frame) < POWER_MANAGEMENT_STAY_AWAKE_MIN_FRAME_LENGTH) {
    return; // Invalid frame length
  }

  uint32_t power_lock_timeout = (uint32_t)(frame->payload[1] << 24);  // **Power Lock timeout (32 bits MSB first)**
  power_lock_timeout |= (uint32_t)(frame->payload[2] << 16);
  power_lock_timeout |= (uint32_t)(frame->payload[3] << 8);
  power_lock_timeout |= (uint32_t)(frame->payload[4]);

  if (POWER_LOCK_TYPE_RADIO == power_lock_type) {
    // Radio power lock: use ZPAL radio API
    // use update_stay_awake to force acquisition of the lock
    zpal_radio_update_stay_awake(&sapi_stay_awake_id, power_lock_timeout);
  }
#ifdef SL_CATALOG_ZW_SHUTDOWN_MANAGER_PRESENT
  else if (POWER_LOCK_TYPE_IO == power_lock_type) {
    // IO power lock: use Silabs shutdown manager
    if (0 == power_lock_timeout) {
      // Permanent lock - acquire only once
      if (!sapi_shutdown_manager_lock_acquired) {
        zw_shutdown_manager_add_lock();
        sapi_shutdown_manager_lock_acquired = true;
      }
    } else {
      // Temporary lock
      zw_shutdown_manager_take_temporary_lock(power_lock_timeout);
    }
  }
#endif

  // **Wake Up Timer timeout (32 bits MSB first)** - Silabs-specific feature
  uint32_t wake_up_timer_timeout = (uint32_t)(frame->payload[5] << 24);
  wake_up_timer_timeout |= (uint32_t)(frame->payload[6] << 16);
  wake_up_timer_timeout |= (uint32_t)(frame->payload[7] << 8);
  wake_up_timer_timeout |= (uint32_t)(frame->payload[8]);

  if (power_lock_timeout && wake_up_timer_timeout) {
    AppTimerDeepSleepPersistentStart(&mWakeupTimer, wake_up_timer_timeout);
  }
  set_state_and_notify(stateIdle);
}

ZW_ADD_CMD(FUNC_ID_POWER_MANAGEMENT_CANCEL)
{
  /* HOST->ZW: Power Lock type */
  /* According to spec: Power Lock type 0 for Radio and 1 for IO */

  if (frame_payload_len(frame) < 1) {
    return; // Invalid frame length
  }

  uint8_t power_lock_type = frame->payload[0];  // **Power Lock type** - 0=Radio, 1=IO

  if (POWER_LOCK_TYPE_RADIO == power_lock_type) {
    // Cancel radio power lock
    zpal_radio_revoke_stay_awake(&sapi_stay_awake_id);
  }
#if defined(SL_CATALOG_POWER_MANAGER_NO_DEEPSLEEP_PRESENT)
  if (POWER_LOCK_TYPE_IO == power_lock_type) {
    set_state_and_notify(stateIdle);
    return;
  }
#endif

#ifdef SL_CATALOG_ZW_SHUTDOWN_MANAGER_PRESENT
  if (POWER_LOCK_TYPE_IO == power_lock_type) {
    // Cancel IO power lock only if we acquired it
    if (sapi_shutdown_manager_lock_acquired) {
      zw_shutdown_manager_release_lock();
      sapi_shutdown_manager_lock_acquired = false;
    }
  }
#endif
  set_state_and_notify(stateIdle);
}
