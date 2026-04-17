/***************************************************************************//**
 * @file
 * @brief Common code for internal apps running in simulation.  Can be used by
 * either Host or Onboard applications.
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

#include PLATFORM_HEADER
#ifdef EZSP_HOST
// Includes needed for ember related functions for the EZSP host
  #include "stack/include/sl_zigbee_types.h"
  #include "app/util/ezsp/ezsp-protocol.h"
  #include "app/util/ezsp/ezsp.h"
  #include "app/util/ezsp/ezsp-utils.h"
  #include "app/util/ezsp/serial-interface.h"
#else
// Includes needed for ember related functions for the EM250
  #include "stack/include/sl_zigbee.h"
  #include "stack/core/sl_zigbee_stack.h"     // we use the events
#endif // EZSP_HOST

#include "hal/hal.h"
#include "plugin/serial/serial.h"

#include "app/util/common/common.h"
#include "app/util/common/internal-common.h"
#include "event_queue/event-queue.h"

static bool suspended = false;

// The run(...) function for the application is renamed
//  runSimulated(...) automatically by the Makefile.

void runSimulated(sli_zigbee_event_queue_t *queue,
                  void (* heartbeat)(void))
{
  uint32_t msToNextEvent = 0;
  msToNextEvent += 0;  // no-op to silence an IAR warning
  while (true) {
    halResetWatchdog();

    if (!suspended) {
      sl_zigbee_tick();
    } else {
      simulatedTimePasses();
    }

    if (heartbeat != NULL) {
      (heartbeat)();
    }
    if (queue != NULL) {
      sli_zigbee_run_event_queue(queue);
      msToNextEvent = sli_zigbee_ms_to_next_queue_event(queue);
    } else {
      // If there are no application events, set the time to
      // the maximum value (as would happen in emberMsToNextEvent()
      // when all the events are inactive)
      msToNextEvent = 0xFFFFFF;
    }
    simulatedTimePassesMs(msToNextEvent);
  }
}

void simulateTaskSuspend(void)
{
  suspended = true;
}

void simulateTaskResume(void)
{
  suspended = false;
}
