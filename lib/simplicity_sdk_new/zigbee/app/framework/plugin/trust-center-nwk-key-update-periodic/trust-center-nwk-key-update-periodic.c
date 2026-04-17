/***************************************************************************//**
 * @file
 * @brief Functionality for periodically updating the NWK key.
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
#include "app/framework/util/common.h"
#include "app/framework/util/util.h"

// *****************************************************************************
// Globals

#ifdef SL_COMPONENT_CATALOG_PRESENT
#include "sl_component_catalog.h"
#endif
#ifdef SL_CATALOG_ZIGBEE_PRO_COMPLIANCE_PRESENT
#include "app/test/pro-compliance.h"
#endif
#include "trust-center-nwk-key-update-periodic-config.h"
sl_zigbee_af_event_t sl_zigbee_af_trust_center_nwk_key_update_periodic_my_event;
#define myEvent (&sl_zigbee_af_trust_center_nwk_key_update_periodic_my_event)
// Use the event data member to keep track of the remaing time
#define remainingTimeInMinutes (sl_zigbee_af_trust_center_nwk_key_update_periodic_my_event.data)
void sl_zigbee_af_trust_center_nwk_key_update_periodic_my_event_handler(sl_zigbee_af_event_t * event);

// All delays are converted into milliseconds, the max delay in minutes converted to millisecond
#define SL_ZIGBEE_MAX_EVENT_DELAY_MINUTES ((SL_ZIGBEE_MAX_EVENT_DELAY_MS / 60000))

// *****************************************************************************

extern sl_status_t sl_zigbee_af_trust_center_start_network_key_update(void);

static void scheduleNextUpdate(void)
{
  if (remainingTimeInMinutes == 0) {
    remainingTimeInMinutes = KEY_UPDATE_PERIOD_MINUTES;
    sl_zigbee_af_app_println("Next NWK key update in %d minutes",
                             KEY_UPDATE_PERIOD_MINUTES);
  }
  if (remainingTimeInMinutes > SL_ZIGBEE_MAX_EVENT_DELAY_MINUTES) {
    remainingTimeInMinutes -= SL_ZIGBEE_MAX_EVENT_DELAY_MINUTES;
    sl_zigbee_af_event_set_delay_minutes(myEvent, SL_ZIGBEE_MAX_EVENT_DELAY_MINUTES);
  } else if (remainingTimeInMinutes > 0) {
    sl_zigbee_af_event_set_delay_minutes(myEvent, remainingTimeInMinutes);
    remainingTimeInMinutes = 0;
  }
}

void sl_zigbee_af_trust_center_nwk_key_update_periodic_my_event_handler(sl_zigbee_af_event_t * event)
{
  UNUSED_VAR(event);
  // A very simple approach is to always kick off the NWK key update
  // assuming it will take much longer to complete than the period
  // at which we initiate.  If something went wrong and the NWK key update
  // didn't complete, a subsequent key update attempt may help recover this and
  // finish the previous one.  This also handles the reboot case where
  // the TC may have rebooted while in the middle of the key update.
  if (remainingTimeInMinutes == 0) {
    sl_status_t status = sl_zigbee_af_trust_center_start_network_key_update();
    sl_zigbee_app_debug_println("Starting NWK Key update, status: 0x%02X", status);
  }
  scheduleNextUpdate();
}

void sl_zigbee_af_trust_center_nwk_key_update_periodic_init_cb(uint8_t init_level)
{
#ifdef SL_CATALOG_ZIGBEE_PRO_COMPLIANCE_PRESENT
  if (isZCPContext() == true) {
    return;
  }
#endif // SL_CATALOG_ZIGBEE_PRO_COMPLIANCE_PRESENT
  switch (init_level) {
    case SL_ZIGBEE_INIT_LEVEL_EVENT:
    {
      sl_zigbee_af_event_init(myEvent,
                              sl_zigbee_af_trust_center_nwk_key_update_periodic_my_event_handler);
      break;
    }

    case SL_ZIGBEE_INIT_LEVEL_DONE:
    {
      // Even though we might not be on a network at this point,
      // we want to schedule the process for sending key updates from
      // now on.

      // However for the test-harness, we manually manipulate the process
      // so we don't want it to automatically kick off.
#if !defined(SL_CATALOG_ZIGBEE_TEST_HARNESS_PRESENT)
      scheduleNextUpdate();
#endif
    }
  }
}
