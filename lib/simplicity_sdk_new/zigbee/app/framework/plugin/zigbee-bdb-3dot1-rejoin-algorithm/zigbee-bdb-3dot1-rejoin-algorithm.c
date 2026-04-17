/***************************************************************************//**
 * @file
 * @brief Definitions for the BDB 3.1 rejoin algorithm.
 *******************************************************************************
 * # License
 * <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
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
#include "af.h"
#include "zigbee-bdb-3dot1-rejoin-algorithm.h"
#include "zigbee-bdb-3dot1-rejoin-algorithm-config.h"
#include "zigbee_app_framework_event.h"
#include "network-steering.h"
#include "update-tc-link-key.h"
#include "core/sl_zigbee_stack.h"

sl_zigbee_af_bdb_rejoin_state_t bdb_rejoin_state = {
  .enabled = true,
  .state = SL_ZIGBEE_AF_BDB_3DOT1_REJOIN_STATE_NONE,
  .tc_keepalive_failure = false,
  .poll_control_failure = false,
  .rejoin_attempts = 0,
};

static sl_zigbee_af_event_t bdbRejoinEvent;

void sl_zigbee_af_bdb_3dot1_enable_rejoin_algorithm(bool enable)
{
  bdb_rejoin_state.enabled = enable;
}

static void transition_bdb_rejoin_state(void)
{
  sl_zigbee_current_security_state_t securityState;
  memset(&securityState, 0, sizeof(securityState));
  (void)sl_zigbee_get_current_security_state(&securityState);

  bool in_distributed_network = (securityState.bitmask & SL_ZIGBEE_DISTRIBUTED_TRUST_CENTER_MODE);
  bool using_default_tclk = sl_zigbee_af_update_tc_link_key_is_tclk_key_default();

  // The rejoin algorithm starts from state=none, so we bump it to get to the
  // first actionable state. Subsequent bumps just move us to the next action
  if ((bdb_rejoin_state.state == SL_ZIGBEE_AF_BDB_3DOT1_REJOIN_STATE_NONE)
      && !in_distributed_network
      && !using_default_tclk) {
    bdb_rejoin_state.state = SL_ZIGBEE_AF_BDB_3DOT1_REJOIN_STATE_TC_REJOIN_ON_CURRENT_CHANNEL;
  } else if ((bdb_rejoin_state.state == SL_ZIGBEE_AF_BDB_3DOT1_REJOIN_STATE_SECURE_REJOIN_ON_CURRENT_CHANNEL)
             && (in_distributed_network || !using_default_tclk)) {
    bdb_rejoin_state.state = SL_ZIGBEE_AF_BDB_3DOT1_REJOIN_STATE_REJOIN_ON_PRIMARY_CHANNELS;
  } else {
    bdb_rejoin_state.state++;
  }
}

void bdbRejoinEventHandler(sl_zigbee_af_event_t* event)
{
  sl_status_t status;

  sl_zigbee_af_event_set_inactive(event);

  if (!bdb_rejoin_state.enabled) {
    bdb_rejoin_state.state = SL_ZIGBEE_AF_BDB_3DOT1_REJOIN_STATE_NONE;
    return;
  }

  sl_zigbee_af_core_println("BDB Rejoin: TC connectivity loss detected, beginning rejoin attempts");

  transition_bdb_rejoin_state();

  // If we've exhausted all attempts, let the keepalive mechanism kick the rejoin
  // algorithm again once it has detected TC connectivity loss again
  if (bdb_rejoin_state.state == SL_ZIGBEE_AF_BDB_3DOT1_REJOIN_STATE_REJOIN_LAST_STATE) {
    sl_zigbee_af_core_println("BDB Rejoin: all attempts exhausted. Waiting for next TC connectivity failure");
    bdb_rejoin_state.state = SL_ZIGBEE_AF_BDB_3DOT1_REJOIN_STATE_NONE;
    return;
  }

  bool secure_rejoin;
  bool do_rejoin = false;
  uint32_t channel_mask;

  sl_zigbee_current_security_state_t securityState;
  memset(&securityState, 0, sizeof(securityState));
  (void)sl_zigbee_get_current_security_state(&securityState);

  bool in_distributed_network = (securityState.bitmask & SL_ZIGBEE_DISTRIBUTED_TRUST_CENTER_MODE);

  switch (bdb_rejoin_state.state) {
    case SL_ZIGBEE_AF_BDB_3DOT1_REJOIN_STATE_NONE:
    case SL_ZIGBEE_AF_BDB_3DOT1_REJOIN_STATE_REJOIN_LAST_STATE:
      // It's not possible to get here by state transition
      SL_ZIGBEE_TEST_ASSERT(0);
      break;
    case SL_ZIGBEE_AF_BDB_3DOT1_REJOIN_STATE_SECURE_REJOIN_ON_CURRENT_CHANNEL:
      channel_mask = 0; // 0 means to use current channel in rejoin API below
      secure_rejoin = true;
      do_rejoin = true;
      break;
    case SL_ZIGBEE_AF_BDB_3DOT1_REJOIN_STATE_TC_REJOIN_ON_CURRENT_CHANNEL:
      channel_mask = 0; // 0 means to use current channel in rejoin API below
      secure_rejoin = false;
      do_rejoin = true;
      break;
    case SL_ZIGBEE_AF_BDB_3DOT1_REJOIN_STATE_REJOIN_ON_PRIMARY_CHANNELS:
      channel_mask = sl_zigbee_af_network_steering_get_channel_mask(true);
      secure_rejoin = in_distributed_network;
      do_rejoin = true;
      break;
    case SL_ZIGBEE_AF_BDB_3DOT1_REJOIN_STATE_REJOIN_ON_SECONDARY_CHANNELS:
      channel_mask = sl_zigbee_af_network_steering_get_channel_mask(false);
      secure_rejoin = in_distributed_network;
      do_rejoin = (channel_mask != 0);
      break;
    default:
      break;
  }

  if (do_rejoin) {
    sl_zigbee_af_core_println("BDB Rejoin: issuing %s rejoin on channel mask 0x%04X",
                              secure_rejoin ? "secure" : "TC",
                              channel_mask);

    status = sl_zigbee_find_and_rejoin_network(secure_rejoin,
                                               channel_mask,
                                               SL_ZIGBEE_AF_REJOIN_DUE_TO_BDB_3DOT1_REJOIN_ALGORITHM,
                                               SL_ZIGBEE_DEVICE_TYPE_UNCHANGED);

    if (status != SL_STATUS_OK) {
      sl_zigbee_af_core_println("Error: failed to initiate TC rejoin 0x%04X\n", status);
      sl_zigbee_af_event_set_delay_ms(&bdbRejoinEvent, 1000);
    } else {
      bdb_rejoin_state.rejoin_attempts++;
    }
  } else {
    // If we didn't rejoin because we're in the SL_ZIGBEE_AF_BDB_3DOT1_REJOIN_STATE_REJOIN_ON_SECONDARY_CHANNELS
    // state and the secondary channel mask is 0, we just arm the algorithm to run again from the start
    sl_zigbee_af_event_set_delay_ms(&bdbRejoinEvent, 1000);
  }
}

void sli_zigbee_af_bdb_3dot1_rejoin_algorithm_init_callback(uint8_t endpoint)
{
  (void)endpoint;
  sl_zigbee_af_network_event_init(&bdbRejoinEvent,
                                  bdbRejoinEventHandler);
}

void sl_zigbee_af_bdb_3dot1_rejoin_algorithm_stack_status_cb(sl_status_t status)
{
  if (!bdb_rejoin_state.enabled) {
    return;
  }

  static bool rejoin_failed = false;

  switch (status) {
    case SL_STATUS_ZIGBEE_REJOIN_FAILED_BUT_NETWORK_RESTORED:
      if (bdb_rejoin_state.state != SL_ZIGBEE_AF_BDB_3DOT1_REJOIN_STATE_NONE) {
        rejoin_failed = true;
      }
      break;
    case SL_STATUS_NETWORK_UP:
      if (bdb_rejoin_state.state != SL_ZIGBEE_AF_BDB_3DOT1_REJOIN_STATE_NONE) {
        if (rejoin_failed) {
          // We failed, so try the next state
          sl_zigbee_af_event_set_delay_ms(&bdbRejoinEvent, 1000);
        } else {
          // We are back up
          bdb_rejoin_state.state = SL_ZIGBEE_AF_BDB_3DOT1_REJOIN_STATE_NONE;
          bdb_rejoin_state.tc_keepalive_failure = false;
          bdb_rejoin_state.poll_control_failure = false;
          sl_zigbee_af_event_set_inactive(&bdbRejoinEvent);
        }
      }
      break;
    default:
      break;
  }

  sl_zigbee_node_type_t node_type;
  sl_zigbee_network_parameters_t network_parameters;
  sl_status_t sl_status = sl_zigbee_get_network_parameters(&node_type, &network_parameters);

  bool is_end_device = false;
  if (sl_status == SL_STATUS_OK) {
    is_end_device = ((node_type == SL_ZIGBEE_SLEEPY_END_DEVICE) || (node_type == SL_ZIGBEE_END_DEVICE));
  }

  if (!sl_zigbee_is_performing_rejoin() && is_end_device) {
    sl_zigbee_network_status_t state = sl_zigbee_af_network_state();
    if (state == SL_ZIGBEE_JOINED_NETWORK_NO_PARENT) {
      // We failed, so try the next state
      sl_zigbee_af_event_set_delay_ms(&bdbRejoinEvent, 1000);
    } else if (state == SL_ZIGBEE_NO_NETWORK) {
      sl_zigbee_af_event_set_inactive(&bdbRejoinEvent);
    }
  }
}

bool sl_zigbee_af_trust_center_keepalive_timeout_cb(void)
{
  bdb_rejoin_state.tc_keepalive_failure = true;
  if (sl_zigbee_af_event_is_scheduled(&bdbRejoinEvent) == false) {
    sl_zigbee_af_event_set_active(&bdbRejoinEvent);
  }
  // We return true to tell the TC Keepalive component not to issue any
  // rejoins. This component will handle rejoins.
  return true;
}

void sl_zigbee_af_poll_control_server_check_in_timeout_cb(void)
{
  bdb_rejoin_state.poll_control_failure = true;
  if (sl_zigbee_af_event_is_scheduled(&bdbRejoinEvent) == false) {
    sl_zigbee_af_event_set_active(&bdbRejoinEvent);
  }
}

bool sl_zigbee_af_end_device_support_pre_network_move_cb(void)
{
  // Various plugins try to issue Network Moves when certain events happen. They
  // call a callback which is defined by end-device-move.c, who begins a process
  // of network rejoins. To stop it from issuing rejoins, we simply return true
  // here if the rejoin algorithm is enabled.
  return bdb_rejoin_state.enabled;
}
