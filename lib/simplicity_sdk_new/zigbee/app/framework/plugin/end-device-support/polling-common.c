/***************************************************************************//**
 * @file
 * @brief Common routines for the Polling plugin, which controls an end device's
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

// The polling task is used in SoC or on the host if Polling callback is defined.
#if !defined (EZSP_HOST) || defined (ENABLE_POLL_COMPLETED_CALLBACK)
  #define ADD_POLLING_TASK()    sl_zigbee_af_add_to_current_app_tasks(SL_ZIGBEE_AF_LAST_POLL_GOT_DATA)
  #define REMOVE_POLLING_TASK() sl_zigbee_af_remove_from_current_app_tasks(SL_ZIGBEE_AF_LAST_POLL_GOT_DATA)
  #define REMOVE_SHORT_POLLING_FOR_PARENT_CONNECTIVITY_TASK() sl_zigbee_af_remove_from_current_app_tasks(SL_ZIGBEE_AF_FORCE_SHORT_POLL_FOR_PARENT_CONNECTIVITY)
#else
  #define ADD_POLLING_TASK()
  #define REMOVE_POLLING_TASK()
  #define REMOVE_SHORT_POLLING_FOR_PARENT_CONNECTIVITY_TASK()
#endif

sli_zigbee_af_polling_state sli_zigbee_af_polling_states[SL_ZIGBEE_SUPPORTED_NETWORKS];

// This function is called when a poll completes and explains what happend with
// the poll.  If the number of sequential data polls not ACKed by the parent
// exceeds the threshold, we will try to find a new parent.
void sli_zigbee_af_poll_complete_handler(sl_status_t status, uint8_t limit)
{
  sli_zigbee_af_polling_state *state;
  uint8_t networkIndex;

  (void) sl_zigbee_af_push_callback_network_index();
  networkIndex = sl_zigbee_get_current_network();
  state = &sli_zigbee_af_polling_states[networkIndex];

  if (sli_zigbee_af_enable_poll_completed_callback) {
    sl_zigbee_af_end_device_support_poll_completed_cb(status);
  }

  switch (status) {
    case SL_STATUS_OK:
      REMOVE_SHORT_POLLING_FOR_PARENT_CONNECTIVITY_TASK();
      ADD_POLLING_TASK();
      sl_zigbee_af_debug_println("poll nwk %d: got data", networkIndex);
      state->numPollsFailing = 0;
      break;
    case SL_STATUS_MAC_NO_DATA:
      REMOVE_SHORT_POLLING_FOR_PARENT_CONNECTIVITY_TASK();
      REMOVE_POLLING_TASK();
      sl_zigbee_af_debug_println("poll nwk %d: no data", networkIndex);
      state->numPollsFailing = 0;
      break;
    case SL_STATUS_TRANSMIT_BLOCKED:
    case SL_STATUS_TRANSMIT_SCHEDULER_FAIL:
    case SL_STATUS_CCA_FAILURE:
      // This means the air was busy, which we don't count as a failure.
      REMOVE_POLLING_TASK();
      sl_zigbee_af_debug_println("poll nwk %d: channel is busy", networkIndex);
      break;
    case SL_STATUS_MAC_NO_ACK_RECEIVED:
      // If we are performing key establishment, we can ignore this since the
      // parent could go away for long periods of time while doing ECC processes.
      if (sl_zigbee_af_performing_key_establishment()) {
        break;
      }
      // Count failures until we hit the limit, then we try a rejoin. If rejoin
      // fails, it will trigger a move.
      state->numPollsFailing++;
      REMOVE_POLLING_TASK();
      sl_zigbee_af_debug_println("Number of short poll attempts made to resolve parent connectivity: %d", state->numPollsFailing);
      sl_zigbee_af_add_to_current_app_tasks_cb(SL_ZIGBEE_AF_FORCE_SHORT_POLL_FOR_PARENT_CONNECTIVITY);
      if (limit <= state->numPollsFailing) {
        if (!sl_zigbee_af_end_device_support_lost_parent_connectivity_cb()) {
          sl_zigbee_af_start_move_cb();
        }
      }
      break;
    default:
      sl_zigbee_af_debug_println("poll nwk %d: 0x%02X", networkIndex, status);
  }

  (void) sl_zigbee_af_pop_network_index();
}
