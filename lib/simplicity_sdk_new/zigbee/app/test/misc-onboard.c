/***************************************************************************//**
 * @file
 * @brief Miscellaneous Network Commands such as polling, leaving,
 * radio sleep, and setting the EUI64.
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

// Ember stack and related utilities.
#include "stack/include/sl_zigbee.h"               // Main stack definitions.
#include "stack/core/sl_zigbee_stack.h"               // Main stack definitions.
#include "core/sl_zigbee_multi_network.h"

// HAL.
#include "hal/hal.h"

// Application utilities.
#include "serial/serial.h"
#include "app/util/serial/sl_zigbee_command_interpreter.h"
#include "app/util/common/common.h"
#include "app/test/misc-common.h"
#include "app/test/security-common.h"

// Unreleased security policy stack features.
#include "stack/zigbee/aps-security-policy.h"

#include "upper-mac.h"
#include "mac-child.h" // unified-mac

#include "stack/include/pro_compliance_stack_interface.h"

//------------------------------------------------------------------------------
// Globals

// Assume Trust Center == Coordinator
#define sli_zigbee_am_trust_center (sl_zigbee_get_node_id() == 0x0000)

//------------------------------------------------------------------------------
// Forward Declarations

//------------------------------------------------------------------------------
// External Declarations

//------------------------------------------------------------------------------
// Functions

//sl_802154_long_addr_t sli_802154mac_local_eui64;

void setEui64(sl_cli_command_arg_t *arguments)
{
  sl_zigbee_copy_eui64_arg(arguments, 0, sl_zigbee_get_eui64(), false);
  (void) sl_mac_test_set_nwk_radio_params_eui(0, sl_zigbee_get_eui64());
  (void) sl_mac_test_set_nwk_radio_params_eui(1, sl_zigbee_get_eui64());
/*#ifdef MAC_DUAL_PRESENT
   // We don't set eui64 on real hardware and also don't have simulation_dual
   // support on zcp simulation test framework. Hence commenting to
   // save code space.
   sli_mac_get_nwk_radio_parameters(1, 0, &radio_parameters);
   memcpy(radio_parameters.local_eui, sli_zigbee_stack_get_eui64(), EUI64_SIZE);
   sli_mac_set_nwk_radio_parameters(1, 0, &radio_parameters);
 #endif*/// MAC_DUAL_PRESENT
}

//------------------------------------------------------------------------------
// Turn on/off the radio.

void radioCommand(sl_cli_command_arg_t *arguments)
{
  uint8_t length = 0;

  uint8_t *command = sl_zigbee_cli_get_argument_string_and_length(arguments, -1, &length);
  if (command[6] == 's') {
    sl_mac_lower_mac_radio_sleep();
  } else {
    sl_mac_lower_mac_radio_wakeup();
  }
}

//------------------------------------------------------------------------------
// Sleep

static void powerCycle(uint32_t delay)
{
  UNUSED_VAR(delay);
  ATOMIC(
    sl_zigbee_stack_power_down();
    sl_zigbee_stack_power_up();
    )
}

void sleepCommand(sl_cli_command_arg_t *arguments)
{
  uint32_t delay = sl_cli_get_argument_uint32(arguments, 0);
  bool slept = false;
  sl_zigbee_node_type_t type;
  sl_zigbee_get_network_parameters(&type, NULL);

  if (type >= SL_ZIGBEE_SLEEPY_END_DEVICE) {
    if (sl_zigbee_ok_to_nap()) {
      powerCycle(delay);
      slept = true;
    }
  }

  if (slept) {
    sl_zigbee_core_debug_println("Slept %lu QS type:%d", (unsigned long)delay, type);
  } else {
    sl_zigbee_core_debug_println("Not OK");
  }
}

//------------------------------------------------------------------------------

void zigbeeLeaveCommand(sl_cli_command_arg_t *arguments)
{
#ifdef SL_CATALOG_CLI_PRESENT
  UNUSED_VAR(arguments);
#endif // SL_CATALOG_CLI_PRESENT
  // CCB 2047
  // - CCB makes the first step to depracate the 'leave and remove children' functionality.
  // - We were proactive here and deprecated it right away.
  sl_status_t status = sl_zigbee_leave_network(SL_ZIGBEE_LEAVE_NWK_WITH_NO_OPTION);
  printCommandStatus(status, "Left", "Leave failed");
}

//------------------------------------------------------------------------------

void zigbeeEvictCommand(sl_cli_command_arg_t *arguments)
{
  sl_802154_long_addr_t childEui64;
  sl_802154_short_addr_t childId;
  sl_802154_short_addr_t destId = sl_cli_get_argument_uint16(arguments, 0);
  sl_status_t status;

  sl_zigbee_copy_eui64_arg(arguments, 1, childEui64, false);
  childId = sl_mac_find_child_short_id(childEui64);
  if (childId != SL_ZIGBEE_NULL_NODE_ID) {
    // CCB 2047
    // - CCB makes the first step to depracate the 'leave and remove children' functionality.
    // - We were proactive here and deprecated it right away.
    status = sl_zigbee_zigbee_remove_child(childId, 0);
  } else if ( sli_zigbee_am_trust_center ) {
    sl_802154_long_addr_t parentEui64;
    if (SL_STATUS_OK != sl_zigbee_lookup_eui64_by_node_id(destId,
                                                          parentEui64)) {
      status = SL_STATUS_FAIL;
    } else {
      status = sl_zigbee_send_remove_device(destId,
                                            parentEui64,
                                            childEui64);
    }
  } else {
    status = SL_STATUS_INVALID_STATE;
  }

  printCommandStatus(status, "Evicted", "Evict failed");
}

//------------------------------------------------------------------------------

void pollEventHandler(uint8_t nwkIndex)
{
  sl_status_t status;
  UNUSED uint8_t savedNetworkIndex = sl_zigbee_get_current_network();

  (void) sl_zigbee_set_current_network(nwkIndex);

  status = sl_zigbee_poll_for_data();

  if (status != SL_STATUS_OK) {
    sl_zigbee_core_debug_println("poll for data, status 0x%02X", status);
  }

  if (pollDelay[nwkIndex] != 0) {
    sl_zigbee_af_event_set_delay_ms(pollEvent[nwkIndex], pollDelay[nwkIndex]);
  } else {
    sl_zigbee_af_event_set_inactive(pollEvent[nwkIndex]);
  }

  (void) sl_zigbee_set_current_network(savedNetworkIndex);
}

void pollEventHandler0(sl_zigbee_af_event_t * event)
{
  UNUSED_VAR(event);
  pollEventHandler(0);
}

void pollEventHandler1(sl_zigbee_af_event_t * event)
{
  UNUSED_VAR(event);
  pollEventHandler(1);
}

void pollEventHandler2(sl_zigbee_af_event_t * event)
{
  UNUSED_VAR(event);
  pollEventHandler(2);
}

void pollEventHandler3(sl_zigbee_af_event_t * event)
{
  UNUSED_VAR(event);
  pollEventHandler(3);
}

//----------------------------------------------------------------

bool allowBeacons = true;
extern sli_zigbee_event_t sli_zigbee_beacon_events[];

void beaconSuppressionTick(void)
{
  if (!allowBeacons) {
    sli_zigbee_event_set_inactive(&sli_zigbee_beacon_events[0]);
  }
}

void allowBeaconsCommand(sl_cli_command_arg_t *arguments)
{
  allowBeacons = (sl_cli_get_argument_uint32(arguments, 0) != 0);
}
