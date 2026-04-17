/***************************************************************************//**
 * @file
 * @brief Routines for the Simple Clock plugin.
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
#include "simple-clock.h"

// The variable "timeS" represents seconds since the ZigBee epoch, which was 0
// hours, 0 minutes, 0 seconds, on the 1st of January, 2000 UTC.  The variable
// "tickMs" is the millsecond tick at which that time was set.  The variable
// "remainderMs" is used to track sub-second chunks of time when converting
// from ticks to seconds.
static uint32_t timeS = 0;
static uint32_t tickMs = 0;
static uint16_t remainderMs = 0;

sl_zigbee_af_plugin_simple_clock_time_sync_status_t syncStatus = SL_ZIGBEE_AF_SIMPLE_CLOCK_NEVER_UTC_SYNC;

// This event is used to periodically force an update to the internal time in
// order to avoid potential rollover problems with the system ticks.  A call to
// GetCurrentTime or SetTime will reschedule the event.
sl_zigbee_af_event_t sl_zigbee_af_simple_clock_update_event;
#define updateEvent (&sl_zigbee_af_simple_clock_update_event)
void sl_zigbee_af_simple_clock_update_event_handler(sl_zigbee_af_event_t * event);

void sl_zigbee_af_simple_clock_init_cb(uint8_t init_level)
{
  switch (init_level) {
    case SL_ZIGBEE_INIT_LEVEL_EVENT:
    {
      sl_zigbee_af_event_init(updateEvent,
                              sl_zigbee_af_simple_clock_update_event_handler);
      break;
    }

    case SL_ZIGBEE_INIT_LEVEL_DONE:
    {
      sl_zigbee_af_set_time_cb(0);
      break;
    }
  }
}

uint32_t sl_zigbee_af_get_current_time_cb(void)
{
  // Using system ticks, calculate how many seconds have elapsed since we last
  // got the time.  That amount plus the old time is our new time.  Remember
  // the tick time right now too so we can do the same calculations again when
  // we are next asked for the time.  Also, keep track of the sub-second chunks
  // of time during the conversion from ticks to seconds so the clock does not
  // drift due to rounding.
  uint32_t elapsedMs, lastTickMs = tickMs;
  tickMs = halCommonGetInt32uMillisecondTick();
  elapsedMs = elapsedTimeInt32u(lastTickMs, tickMs);
  timeS += elapsedMs / MILLISECOND_TICKS_PER_SECOND;
  remainderMs += elapsedMs % MILLISECOND_TICKS_PER_SECOND;
  if (MILLISECOND_TICKS_PER_SECOND <= remainderMs) {
    timeS++;
    remainderMs -= MILLISECOND_TICKS_PER_SECOND;
  }

  // Schedule an event to recalculate time to help avoid rollover problems.
  sl_zigbee_af_event_set_delay_ms(updateEvent,
                                  MILLISECOND_TICKS_PER_DAY);
  return timeS;
}

uint32_t sl_zigbee_af_get_current_time_seconds_with_ms_precision(uint16_t* millisecondsRemainderReturn)
{
  uint32_t value = sl_zigbee_af_get_current_time_cb();
  *millisecondsRemainderReturn = remainderMs;
  return value;
}

void sl_zigbee_af_set_time_cb(uint32_t utcTime)
{
  tickMs = halCommonGetInt32uMillisecondTick();
  timeS = utcTime;
  remainderMs = 0;

  // Immediately get the new time in order to reschedule the event.
  sl_zigbee_af_get_current_time_cb();
}

void sl_zigbee_af_simple_clock_set_utc_synced_time(uint32_t utcTime)
{
  sl_zigbee_af_set_time_cb(utcTime);
  syncStatus = SL_ZIGBEE_AF_SIMPLE_CLOCK_UTC_SYNCED;
}

sl_zigbee_af_plugin_simple_clock_time_sync_status_t sl_zigbee_af_simple_clock_get_time_sync_status(void)
{
  return syncStatus;
}

void sl_zigbee_af_simple_clock_update_event_handler(sl_zigbee_af_event_t * event)
{
  UNUSED_VAR(event);
  // Get the time, which will reschedule the event.

  // TODO: If time has not been synced for a while, set the status to
  // SL_ZIGBEE_AF_SIMPLE_CLOCK_STALE_UTC_SYNC

  sl_zigbee_af_get_current_time_cb();
}
