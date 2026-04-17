/***************************************************************************//**
 * @file
 * @brief SoC routines for the Polling plugin, which controls an end device's
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
#include "app/framework/plugin/end-device-support/end-device-support.h"

// *****************************************************************************
// Globals

sl_zigbee_af_event_t sl_zigbee_af_end_device_support_polling_network_events[SL_ZIGBEE_SUPPORTED_NETWORKS];
void sl_zigbee_af_end_device_support_polling_network_event_handler(sl_zigbee_af_event_t * event);

// *****************************************************************************
// Functions

void sli_zigbee_af_end_device_support_polling_init(uint8_t init_level)
{
  (void) init_level;
  sl_zigbee_af_network_event_init(sl_zigbee_af_end_device_support_polling_network_events,
                                  sl_zigbee_af_end_device_support_polling_network_event_handler);
}

// This is called to scheduling polling events for the network(s).  We only
// care about end device networks.  For each of those, a polling event will be
// scheduled for joined networks or canceled otherwise.
void sl_zigbee_af_end_device_support_tick_cb(void)
{
  uint8_t i;
  for (i = 0; i < SL_ZIGBEE_SUPPORTED_NETWORKS; i++) {
    (void) sl_zigbee_af_push_network_index(i);
    if (sli_zigbee_af_pro_is_current_network()
        && SL_ZIGBEE_END_DEVICE <= sli_zigbee_af_current_zigbee_pro_network->nodeType) {
      if (sl_zigbee_af_network_state() == SL_ZIGBEE_JOINED_NETWORK) {
        sli_zigbee_af_polling_state *state = &sli_zigbee_af_polling_states[i];
        uint32_t lastPollIntervalMs = state->pollIntervalMs;
        state->pollIntervalMs = sl_zigbee_af_get_current_poll_interval_ms_cb();
        if (state->pollIntervalMs != lastPollIntervalMs
            || !sl_zigbee_af_event_is_scheduled(sl_zigbee_af_end_device_support_polling_network_events)) {
          sl_zigbee_af_debug_println("Next poll nwk %d in %ld ms",
                                     i,
                                     state->pollIntervalMs);
          sl_zigbee_af_event_set_delay_ms(sl_zigbee_af_end_device_support_polling_network_events,
                                          state->pollIntervalMs);
        }
      } else {
        sl_zigbee_af_event_set_inactive(sl_zigbee_af_end_device_support_polling_network_events);
      }
    }
    (void) sl_zigbee_af_pop_network_index();
  }
}

// Whenever the polling event fires for a network, a MAC data poll is sent.
void sl_zigbee_af_end_device_support_polling_network_event_handler(sl_zigbee_af_event_t * event)
{
  (void)event;
  sl_zigbee_network_status_t state = sl_zigbee_af_network_state();
  if (state == SL_ZIGBEE_JOINED_NETWORK) {
    sl_status_t status = sl_zigbee_poll_for_data();
    if (status != SL_STATUS_OK) {
      sl_zigbee_af_core_println("poll nwk %d: 0x%02X", sl_zigbee_get_current_network(), status);
    }
  }
}

// This function is called when a poll completes and explains what happend with
// the poll.  If the number of sequential data polls not ACKed by the parent
// exceeds the threshold, we will try to find a new parent.
void sli_zigbee_af_end_device_support_poll_complete_callback(sl_status_t status)
{
  sli_zigbee_af_poll_complete_handler(status,
                                      SL_ZIGBEE_AF_PLUGIN_END_DEVICE_SUPPORT_MAX_MISSED_POLLS);
}
