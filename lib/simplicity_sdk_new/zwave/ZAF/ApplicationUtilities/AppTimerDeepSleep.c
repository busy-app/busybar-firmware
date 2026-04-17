/**
 * @file
 * @brief Application Timer module
 *
 * @copyright 2018 Silicon Laboratories Inc.
 */

/* Z-Wave includes */
#include <AppTimer.h>
#include <SwTimerLiaison.h>
#include <ZW_system_startup_api.h>
#include <zpal_retention_register.h>
#include "zpal_log.h"

#include <assert.h>
#include <FreeRTOS.h>
#include <task.h>

/**
 * First (zero based) retention register to use for persisting application
 * timers during Deep Sleep. Other retention registers used for Deep Sleep persistent
 * app timers are defined as offsets from this value.
 */
#define FIRST_APP_TIMER_RETENTION_REGISTER        (ZPAL_RETENTION_REGISTER_PROTOCOL_RESERVED_COUNT + 0)

/**
 * First retention register to use for persisting the Deep Sleep persistent application
 * timers during Deep Sleep. (actual number of registers used is determined by
 * how many times AppTimerDeepSleepPersistentRegister() is called).
 */
#define TIMER_VALUES_BEGIN_RETENTION_REGISTER     (FIRST_APP_TIMER_RETENTION_REGISTER)

/**
 * We have 8 retention registers allocated for ZAF/App timers.
 * 6 for persistent timers.
 */
_Static_assert(APP_TIMER_RETENTION_REGISTER_RESERVED_COUNT == 6, "STATIC_ASSERT_FAILED_retention_register_count");

/**
 * On wakeup from Deep Sleep, if the difference between expected timeout of an
 * Deep Sleep persistent application timer and the elapsed time at wake up, is
 * smaller than this value then the timer callback will be activated.
 */
#define APP_TIMER_TRIGGER_DELTA_MS 10

// Using AppTimer singleton
extern SAppTimer g_AppTimer;
// Using state variable from AppTimer
extern bool g_deepSleepTimersLoaded;

/* This function will be called in the correct task context */
void AppTimerDeepSleepCallbackWrapper(SSwTimer* pTimer)
{
  ZPAL_LOG_DEBUG(ZPAL_LOG_ZAF_APP_TIMER, "AppTimerDeepSleepCallbackWrapper timerId=%d\n", pTimer->Id);
  assert(g_AppTimer.DeepSleepPersistent[pTimer->Id] && g_AppTimer.pDeepSleepCallback[pTimer->Id]);

  if (g_AppTimer.pDeepSleepCallback[pTimer->Id]) {
    ZPAL_LOG_DEBUG(ZPAL_LOG_ZAF_APP_TIMER, "Calling g_AppTimer.pDeepSleepCallback[%d] = %p\n", pTimer->Id, g_AppTimer.pDeepSleepCallback[pTimer->Id]);
    (g_AppTimer.pDeepSleepCallback[pTimer->Id])(pTimer);
  }
}

bool AppTimerDeepSleepPersistentRegister(SSwTimer* pTimer,
                                         bool bAutoReload,
                                         void(*pCallback)(SSwTimer* pTimer))
{
  /* We don't support auto reload of Deep Sleep persistent timers (at least it has
   * not been tested - it might actually work now) */
  assert(false == bAutoReload);

  /* Check that we have a retention register available for this new persistent timer */
  uint32_t count = 0;
  for (uint32_t timerId = 0; timerId < MAX_NUM_APP_TIMERS; timerId++) {
    if (true == g_AppTimer.DeepSleepPersistent[timerId]) {
      count++;
    }
  }
  if (count >= MAX_NUM_PERSISTENT_APP_TIMERS) {
    /* All timer retention registers are taken */
    ZPAL_LOG_DEBUG(ZPAL_LOG_ZAF_APP_TIMER, "AppTimerDeepSleepPersistentRegister: Max number of registrations exceeded (%d)\n", MAX_NUM_PERSISTENT_APP_TIMERS);
    return false;
  }

  /* We register AppTimerDeepSleepCallbackWrapper() as the call back in order to
   * update the timer status in retention registers when the timer expires.
   * The actual callback is saved to g_AppTimer.pDeepSleepCallback and will be
   * called by AppTimerDeepSleepCallbackWrapper() */
  ESwTimerLiaisonStatus status = TimerLiaisonRegister(&g_AppTimer.TimerLiaison,
                                                      pTimer,
                                                      bAutoReload,
                                                      AppTimerDeepSleepCallbackWrapper);

  ZPAL_LOG_DEBUG(ZPAL_LOG_ZAF_APP_TIMER, "AppTimerDeepSleepPersistentRegister() id=%d pCallback=%p\n", pTimer->Id, pCallback);
  if (status == ESWTIMERLIAISON_STATUS_SUCCESS) {
    g_AppTimer.DeepSleepPersistent[pTimer->Id] = true;
    g_AppTimer.pDeepSleepCallback[pTimer->Id]  = pCallback;
    return true;
  }

  return false;
}

ESwTimerStatus AppTimerDeepSleepPersistentStart(SSwTimer* pTimer, uint32_t iTimeout)
{
  ZPAL_LOG_DEBUG(ZPAL_LOG_ZAF_APP_TIMER, "AppTimerDeepSleepPersistentStart() id=%d, timeout=%u\n", pTimer->Id, iTimeout);
  ESwTimerStatus status = TimerStart(pTimer, iTimeout);
  return status;
}

ESwTimerStatus AppTimerDeepSleepPersistentRestart(SSwTimer* pTimer)
{
  ZPAL_LOG_DEBUG(ZPAL_LOG_ZAF_APP_TIMER, "AppTimerDeepSleepPersistentRestart() id=%d\n", pTimer->Id);
  ESwTimerStatus status = TimerRestart(pTimer);
  return status;
}

ESwTimerStatus AppTimerDeepSleepPersistentStop(SSwTimer* pTimer)
{
  ZPAL_LOG_DEBUG(ZPAL_LOG_ZAF_APP_TIMER, "AppTimerDeepSleepPersistentStop() id=%d\n", pTimer->Id);
  ESwTimerStatus status = TimerStop(pTimer);
  return status;
}

/*
 * How the Deep Sleep persistent application timers are saved to RTCC retention registers
 *
 * For example the following list of APPLICATION TIMERS (contained in struct
 * g_AppTimer) is assumed (ordered by timer id)
 *
 * 0 (DeepSleepPersistent = false)
 * 1 (DeepSleepPersistent = false)
 * 2 (DeepSleepPersistent = true) MsUntilTimeout=30000
 * 3 (DeepSleepPersistent = true) MsUntilTimeout=0xFFFFFFFF (not active when saved)
 * 4 (DeepSleepPersistent = false)
 * 5 (DeepSleepPersistent = true) MsUntilTimeout=20000
 * 6 (DeepSleepPersistent = false)
 * 7 (DeepSleepPersistent = false)
 *
 * - DeepSleepPersistent is the flag in g_AppTimer ("true" implies that the
 *   timer should be saved, and it also implies that the timer exist)
 *
 * - MsUntilTimeout is the calculated number of milliseconds remaining
 *   before the timer times out. If equal to 0xFFFFFFFF then the timer is
 *   not active.
 *
 * The RETENTION REGISTERS will only contain the timer values for the Deep Sleep
 * persistent timers plus the task tick values when the timer values were
 * saved and when the device is going to sleep in Deep Sleep:
 *
 * 0 TaskTick at power-down
 * 1 TaskTick at save timers
 * 2 timerValue_ms=30000
 * 3 timerValue_ms=0xFFFFFFFF
 * 4 timerValue_ms=20000
 *
 * When the device wakes up from Deep Sleep the values in the retention registers
 * together with the time spent in Deep Sleep hibernate are used to determine if a
 * timer has expired or what value should be used to start it again to have
 * it time out at the right moment relative to its original start time.
 */
void AppTimerDeepSleepPersistentSaveAll(void)
{
  uint32_t reg = TIMER_VALUES_BEGIN_RETENTION_REGISTER;

  // Don't touch the retention registers until they are loaded
  if (false == g_deepSleepTimersLoaded) {
    return;
  }
  uint32_t taskTickCount = xTaskGetTickCount();

  ZPAL_LOG_DEBUG(ZPAL_LOG_ZAF_APP_TIMER, "AppTimerDeepSleepPersistentSaveAll tick: %u\n", taskTickCount);

  for (uint32_t timerId = 0; timerId < MAX_NUM_APP_TIMERS; timerId++) {
    if (true == g_AppTimer.DeepSleepPersistent[timerId]) {
      SSwTimer *pTimer        = g_AppTimer.aTimerPointerArray[timerId];
      uint32_t  timerValue_ms = UINT32_MAX;

      TimerGetMsUntilTimeout(pTimer, taskTickCount, &timerValue_ms);

      ZPAL_LOG_DEBUG(ZPAL_LOG_ZAF_APP_TIMER, "Saving value for timer %d: %u (0x%x) ms\n", timerId, timerValue_ms, timerValue_ms);

      zpal_retention_register_write(reg, timerValue_ms);
      reg++;
    }
  }
}

void AppTimerDeepSleepPersistentLoadAll(zpal_reset_reason_t resetReason)
{
  g_deepSleepTimersLoaded = true;

  /* Do nothing if we did not wake up from Deep Sleep */
  if (ZPAL_RESET_REASON_DEEP_SLEEP_EXT_INT != resetReason
      && ZPAL_RESET_REASON_DEEP_SLEEP_WUT != resetReason) {
    return;
  }
  uint32_t saved_timer_value = 0;

  /* Read saved timer values from retention registers into array
   * while looking for smallest value larger than savedBeforePowerdownMs */
  uint8_t  valIdx = 0;
  uint32_t deep_sleep_duration_ms = GetCompletedSleepDurationMs();
  for (uint8_t timerId = 0; timerId < MAX_NUM_APP_TIMERS; timerId++) {
    if (false == g_AppTimer.DeepSleepPersistent[timerId]) {
      continue;
    }
    if (ZPAL_STATUS_OK != zpal_retention_register_read(TIMER_VALUES_BEGIN_RETENTION_REGISTER + valIdx, &saved_timer_value)
        || UINT32_MAX == saved_timer_value) {
      valIdx++;
      continue;
    }
    ZPAL_LOG_INFO(ZPAL_LOG_ZAF_APP_TIMER, "Loaded value for timer %d: %u ms\n", timerId, saved_timer_value);

    // If corrected timer value is not at least APP_TIMER_TRIGGER_DELTA_MS, use 0 to trigger callback immediately
    uint32_t updated_timeout = saved_timer_value > deep_sleep_duration_ms + APP_TIMER_TRIGGER_DELTA_MS ? saved_timer_value - deep_sleep_duration_ms : 0;

    SSwTimer * timer = g_AppTimer.aTimerPointerArray[timerId];
    if (0 == updated_timeout) {
      TimerStop(timer);
      TimerLiaisonExpiredTimerCallback(timer);
    } else {
      TimerStart(g_AppTimer.aTimerPointerArray[timerId], updated_timeout);
    }
    valIdx++;
  }
  AppTimerDeepSleepPersistentResetStorage();
}

uint32_t AppTimerDeepSleepGetFirstRetentionRegister(void)
{
  return FIRST_APP_TIMER_RETENTION_REGISTER;
}

uint32_t AppTimerDeepSleepGetLastRetentionRegister(void)
{
  uint32_t count = 0;
  for (uint32_t timerId = 0; timerId < MAX_NUM_APP_TIMERS; timerId++) {
    if (true == g_AppTimer.DeepSleepPersistent[timerId]) {
      count++;
    }
  }
  return TIMER_VALUES_BEGIN_RETENTION_REGISTER + count - 1;
}

void AppTimerDeepSleepPersistentResetStorage(void)
{
  uint32_t first       = AppTimerDeepSleepGetFirstRetentionRegister();
  uint32_t last        = AppTimerDeepSleepGetLastRetentionRegister();

  ZPAL_LOG_DEBUG(ZPAL_LOG_ZAF_APP_TIMER, "\nResetDeepSleepPersistentAppTimerStorage first=%u, last=%ux\n", first, last);

  assert(first <= last);
  for (uint32_t reg = first; reg <= last; reg++) {
    zpal_retention_register_write(reg, UINT32_MAX);
  }
}

/**
 * @brief Checks if any Deep Sleep persistent timer is about to expire (less than 1 second remaining).
 * @return true if at least one timer has less than 1000 ms remaining, false otherwise.
 */
bool AppTimerDeepSleepPersistentIsAnyTimerExpiringSoon(void)
{
  TickType_t task_tick_count = xTaskGetTickCount();

  for (uint8_t timer_id = 0; timer_id < MAX_NUM_APP_TIMERS; timer_id++) {
    if (g_AppTimer.DeepSleepPersistent[timer_id]) {
      uint32_t timer_value_ms = UINT32_MAX;
      TimerGetMsUntilTimeout(g_AppTimer.aTimerPointerArray[timer_id], task_tick_count, &timer_value_ms);
      if (timer_value_ms < 1000) {
        ZPAL_LOG_INFO(ZPAL_LOG_ZAF_APP_TIMER, "\nTimer %d is about to expire (%u ms)\n", timer_id, timer_value_ms);
        return true;
      }
    }
  }
  return false;
}
