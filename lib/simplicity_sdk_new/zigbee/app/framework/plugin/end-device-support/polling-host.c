/***************************************************************************//**
 * @file
 * @brief Host routines for the Polling plugin, which controls an end device's
 *        polling behavior.
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
#include "app/framework/util/af-event.h"
#include "app/framework/plugin/end-device-support/end-device-support.h"

// *****************************************************************************
// Globals

static uint8_t numPollsFailingLimit;
static bool enablePollCompletedCallback;

// *****************************************************************************
// Functions

// This is called to scheduling polling events for the network(s).  We only
// care about end device networks.  For each of those, the NCP will be told to
// poll for joined networks or not to poll otherwise.
void sl_zigbee_af_end_device_support_tick_cb(void)
{
  uint8_t i;
  for (i = 0; i < SL_ZIGBEE_SUPPORTED_NETWORKS; i++) {
    (void) sl_zigbee_af_push_network_index(i);
    if (sli_zigbee_af_pro_is_current_network()
        && SL_ZIGBEE_END_DEVICE <= sli_zigbee_af_current_zigbee_pro_network->nodeType) {
      sli_zigbee_af_polling_state *state = &sli_zigbee_af_polling_states[i];
      uint32_t lastPollIntervalMs = state->pollIntervalMs;
      if (sl_zigbee_af_network_state() == SL_ZIGBEE_JOINED_NETWORK) {
        state->pollIntervalMs = sl_zigbee_af_get_current_poll_interval_ms_cb();
      } else {
        state->pollIntervalMs = 0;
      }

      // schedule for poll when following attr changes state:
      // 1) poll interval
      // 2) enablePollCompletedCallback
      if (state->pollIntervalMs != lastPollIntervalMs
          || sli_zigbee_af_enable_poll_completed_callback != enablePollCompletedCallback) {
        sl_status_t status;
        uint16_t duration;
        sl_zigbee_event_units_t units;
        uint8_t ncpFailureLimit;

        enablePollCompletedCallback = sli_zigbee_af_enable_poll_completed_callback;
        if (sli_zigbee_af_enable_poll_completed_callback) {
          ncpFailureLimit = 0;
          numPollsFailingLimit = SL_ZIGBEE_AF_PLUGIN_END_DEVICE_SUPPORT_MAX_MISSED_POLLS;
        } else {
          ncpFailureLimit = SL_ZIGBEE_AF_PLUGIN_END_DEVICE_SUPPORT_MAX_MISSED_POLLS;
          numPollsFailingLimit = 0;
        }

        sli_zigbee_af_get_timer_duration_and_unit_from_ms(state->pollIntervalMs,
                                                          &duration,
                                                          &units);
        status = sl_zigbee_ezsp_poll_for_data(duration, units, ncpFailureLimit);
        if (status != SL_STATUS_OK) {
          sl_zigbee_af_core_println("poll nwk %d: 0x%02X", i, status);
        } else {
          sl_zigbee_af_core_println("Next poll nwk %d in %ld ms",
                                    i,
                                    state->pollIntervalMs);
        }
      }
    }
    (void) sl_zigbee_af_pop_network_index();
  }
}

// The NCP schedules and manages polling, so we do not schedule our own events
// and therefore this handler should never fire.
void sl_zigbee_af_end_device_support_polling_network_event_handler(void)
{
}

// This function is called when a poll completes and explains what happend with
// the poll.  If no ACKs are received from the parent, we will try to find a
// new parent.
void sli_zigbee_af_end_device_support_poll_complete_callback(sl_status_t status)
{
  sli_zigbee_af_poll_complete_handler(status, numPollsFailingLimit);
}

void sl_zigbee_af_pre_ncp_reset_cb(void)
{
  // Reset the poll intervals so the NCP will be instructed to poll if
  // necessary.
  uint8_t i;
  for (i = 0; i < SL_ZIGBEE_SUPPORTED_NETWORKS; i++) {
    sli_zigbee_af_polling_states[i].pollIntervalMs = 0;
    sli_zigbee_af_polling_states[i].numPollsFailing = 0;
  }
  numPollsFailingLimit = SL_ZIGBEE_AF_PLUGIN_END_DEVICE_SUPPORT_MAX_MISSED_POLLS;
  enablePollCompletedCallback = false;
}
