/***************************************************************************//**
 * @file
 * @brief ZigBee 3.0 nwk test harness functionality
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

#include "test-harness-z3-core.h"
#include "test-harness-z3-nwk.h"
#include "stack/include/message.h"
#include "stack/include/network-formation.h"

// -----------------------------------------------------------------------------
// Globals

bool sli_zigbee_af_test_harness_z3_ignore_leave_commands = false;
bool sli_zigbee_test_harness_z3_ignore_rejoin_commands = false;

// -----------------------------------------------------------------------------
// NWK CLI Commands

#ifdef EZSP_HOST
  #define slx_zigbee_network_send_command(...) false
#else

extern sl_status_t sl_zigbee_set_nwk_update_id(uint8_t nwkUpdateId, bool set_when_on_network);

#endif /* EZSP_HOST */

static sl_status_t sendNetworkCommand(sl_802154_short_addr_t destinationShort,
                                      uint8_t *commandFrame,
                                      uint8_t length)
{
  sl_802154_long_addr_t destinationLong;
  bool haveDestinationLong = false;

  haveDestinationLong
    = (sl_zigbee_lookup_eui64_by_node_id(destinationShort, destinationLong)
       == SL_STATUS_OK);

  return (slx_zigbee_network_send_command(destinationShort,
                                          commandFrame,
                                          length,
                                          haveDestinationLong,
                                          destinationLong)
          ? SL_STATUS_OK
          : SL_STATUS_ZIGBEE_DELIVERY_FAILED);
}

// plugin test-harness z3 nwk nwk-rejoin-request <dstShort:2>
void sli_zigbee_af_test_harness_z3_nwk_nwk_rejoin_request_command(sl_cli_command_arg_t *arguments)
{
  sl_status_t status;
  sl_802154_short_addr_t destinationShort = sl_cli_get_argument_uint16(arguments, 0);
  sl_zigbee_node_type_t nodeType;
  uint8_t commandFrame[2];

  status = sl_zigbee_af_get_node_type(&nodeType);
  if (status != SL_STATUS_OK) {
    goto done;
  }

  commandFrame[0] = NWK_REJOIN_REQUEST_COMMAND;
  commandFrame[1] = 0;
  switch (nodeType) {
    case SL_ZIGBEE_COORDINATOR:
      commandFrame[1] |= BIT(0); // alternate pan coordinator
    // fallthrough
    case SL_ZIGBEE_ROUTER:
      commandFrame[1] |= BIT(1); // device type
    // fallthrough
    case SL_ZIGBEE_END_DEVICE:
      commandFrame[1] |= BIT(2); // mains-powered
      commandFrame[1] |= BIT(3); // rx on when idle
    // fallthrough
    case SL_ZIGBEE_SLEEPY_END_DEVICE:
      commandFrame[1] |= BIT(7); // allocate address
      break;
    default:
      status = SL_STATUS_FAIL;
      goto done;
  }

  status = sendNetworkCommand(destinationShort,
                              commandFrame,
                              sizeof(commandFrame));

  done:
  sl_zigbee_af_core_println("%s: %s: 0x%02X",
                            TEST_HARNESS_Z3_PRINT_NAME,
                            "Rejoin request",
                            status);
}

// plugin test-harness z3 nwk nwk-rejoin-response <addr:2> <status:1> <dstShort:2>
void sli_zigbee_af_test_harness_z3_nwk_nwk_rejoin_response_command(sl_cli_command_arg_t *arguments)
{
  uint8_t     frame[4];
  sl_status_t status;

  sl_802154_short_addr_t networkAddress   = sl_cli_get_argument_uint16(arguments, 0);
  uint8_t     rejoinStatus     = sl_cli_get_argument_uint8(arguments, 1);
  sl_802154_short_addr_t destinationShort = sl_cli_get_argument_uint16(arguments, 2);

  frame[0] = NWK_REJOIN_RESPONSE_COMMAND;
  frame[1] = LOW_BYTE(networkAddress);
  frame[2] = HIGH_BYTE(networkAddress);
  frame[3] = rejoinStatus;

  status = sendNetworkCommand(destinationShort, frame, sizeof(frame));

  sl_zigbee_af_core_println("%s: %s: 0x%02X",
                            TEST_HARNESS_Z3_PRINT_NAME,
                            "Rejoin response",
                            status);
}

// plugin test-harness z3 nwk nwk-leave <rejoin:1> <request:1>
// <removeChildren:1> <dstShort:2> <options:4>
void sli_zigbee_af_test_harness_z3_nwk_nwk_leave_command(sl_cli_command_arg_t *arguments)
{
  sl_status_t status;
  uint8_t frame[2] = { NWK_LEAVE_COMMAND, 0, };

  bool rejoin                  = (bool)sl_cli_get_argument_uint32(arguments, 0);
  bool request                 = (bool)sl_cli_get_argument_uint32(arguments, 1);
  // CCB 2047
  // - CCB makes the first step to depracate the 'leave and remove children' functionality.
  // - We were proactive here and deprecated it right away.
  // bool removeChildren          = (bool)sl_zigbee_unsigned_command_argument(2);

  sl_802154_short_addr_t destinationShort = sl_cli_get_argument_uint16(arguments, 3);
  uint32_t options      = sli_zigbee_af_test_harness_z3_get_significant_bit(arguments, 4);

  if (rejoin) {
    frame[1] |= BIT(5);
  }
  if (request) {
    frame[1] |= BIT(6);
  }

  switch (options) {
    case BIT(0): // no ieee addr field
      status = (slx_zigbee_network_send_command(destinationShort,
                                                frame,
                                                sizeof(frame),
                                                false,
                                                NULL)
                ? SL_STATUS_OK
                : SL_STATUS_ZIGBEE_DELIVERY_FAILED);
      break;
    case BIT(1): // don't actually leave network
    // fallthrough
    default:
      status = sendNetworkCommand(destinationShort, frame, sizeof(frame));
  }

  sl_zigbee_af_core_println("%s: %s: 0x%02X",
                            TEST_HARNESS_Z3_PRINT_NAME,
                            "Leave",
                            status);
}

// plugin test-harness z3 nwk nwk-leave-suppression on
// plugin test-harness z3 nwk nwk-leave-suppression off
void sli_zigbee_af_test_harness_z3_nwk_leave_suppression(sl_cli_command_arg_t *arguments)
{
  sli_zigbee_af_test_harness_z3_ignore_leave_commands
    = (sl_zigbee_cli_get_argument_string_and_length(arguments, -1, NULL)[1] == 'n');

  sl_zigbee_af_core_println("%s: %s: %s",
                            TEST_HARNESS_Z3_PRINT_NAME,
                            "Suppress network leave",
                            (sli_zigbee_af_test_harness_z3_ignore_leave_commands
                             ? "YES"
                             : "NO"));
}

// plugin test-harness z3 nwk nwk-rejoin-suppression on
// plugin test-harness z3 nwk nwk-rejoin-suppression off
void sli_zigbee_af_test_harness_z3_nwk_rejoin_suppression(sl_cli_command_arg_t *arguments)
{
  sli_zigbee_test_harness_z3_ignore_rejoin_commands
    = (sl_zigbee_cli_get_argument_string_and_length(arguments, -1, NULL)[1] == 'n');
  sl_zigbee_af_core_println("%s: %s: %s",
                            TEST_HARNESS_Z3_PRINT_NAME,
                            "Suppress network rejoin",
                            (sli_zigbee_test_harness_z3_ignore_rejoin_commands
                             ? "YES"
                             : "NO"));
}

// plugin test-harness z3 nwk nwk-key-suppression on
// plugin test-harness z3 nwk nwk-key-suppression off
void sli_zigbee_af_test_harness_z3_nwk_nwk_key_suppression(sl_cli_command_arg_t *arguments)
{
  (void)arguments;
  // TODO:
  UNUSED_VAR(arguments);
}

// plugin test-harness z3 nwk get-network-update-id
void sli_zigbee_af_test_harness_z3_nwk_get_network_update_id(sl_cli_command_arg_t *arguments)
{
  UNUSED_VAR(arguments);

  sl_zigbee_network_parameters_t networkParameters;
  sl_zigbee_node_type_t nodeType;
  sl_zigbee_af_get_network_parameters(&nodeType, &networkParameters);
  sl_zigbee_af_core_println("%s: %s: 0x%02X",
                            TEST_HARNESS_Z3_PRINT_NAME,
                            "Network Update Id",
                            networkParameters.nwkUpdateId);
}

// plugin test-harness z3 nwk set-network-update-id
void sli_zigbee_af_test_harness_z3_nwk_set_network_update_id(sl_cli_command_arg_t *arguments)
{
  uint8_t updateId = sl_cli_get_argument_uint8(arguments, 0);
  sl_zigbee_set_nwk_update_id(updateId, TRUE);
  sl_zigbee_af_core_println("%s: %s: 0x%02X",
                            TEST_HARNESS_Z3_PRINT_NAME,
                            "Set network update id",
                            updateId);
}

// plugin test-harness z3 nwk nwk-update-request <newPanId:2> <dstShort:2>
void sli_zigbee_af_test_harness_z3_nwk_update_request_command(sl_cli_command_arg_t *arguments)
{
  sl_802154_pan_id_t newPanId = (sl_802154_pan_id_t)sl_cli_get_argument_uint16(arguments, 0);
  sl_802154_short_addr_t destinationShort = (sl_802154_short_addr_t)sl_cli_get_argument_uint16(arguments, 1);
  sl_status_t status;

  uint8_t commandFrame[13];
  uint8_t updateCommandIdentifier = 0; // PAN id update

  sl_zigbee_node_type_t nodeType;
  sl_zigbee_network_parameters_t networkParams;
  status = sl_zigbee_af_get_network_parameters(&nodeType, &networkParams);

// The command currently only supports a single information type.
  if (updateCommandIdentifier == 0) {
    // Update the network short PAN id. The local PAN id will not be updated.
    commandFrame[0] = NWK_NETWORK_UPDATE_COMMAND;
    commandFrame[1] = 1;  // Command options field: info count = 1, command id = 0
    memmove(commandFrame + 2, networkParams.extendedPanId, EXTENDED_PAN_ID_SIZE);
    commandFrame[10] = networkParams.nwkUpdateId;;
    commandFrame[11] = LOW_BYTE(newPanId);
    commandFrame[12] = HIGH_BYTE(newPanId);
  } else {
    status = SL_STATUS_FAIL;
    goto done;
  }

  status = sendNetworkCommand(destinationShort,
                              commandFrame,
                              sizeof(commandFrame));

  done:
  sl_zigbee_af_core_println("%s: %s: 0x%02X",
                            TEST_HARNESS_Z3_PRINT_NAME,
                            "Network update request",
                            status);
}

// plugin test-harness z3 change-pan-id <newPanId:2>
void sli_zigbee_af_test_harness_z3_change_pan_id(sl_cli_command_arg_t *arguments)
{
  sl_802154_pan_id_t newPanId = (sl_802154_pan_id_t)sl_cli_get_argument_uint16(arguments, 0);

  sl_zigbee_af_core_println("New PAN: %04X", newPanId);

  // Just do it, no delays.
  slx_zigbee_change_pan_id_now(newPanId);
}

// plugin test-harness z3 nwk set-long-up-time <longUpTime:1>
void sli_zigbee_test_harness_z3_nwk_set_long_up_time(sl_cli_command_arg_t *arguments)
{
  bool longUpTime = (bool)sl_cli_get_argument_uint8(arguments, 0);

  sl_zigbee_beacon_classification_params_t param;
  sl_zigbee_get_beacon_classification_params(&param);

  param.beaconClassificationMask |= longUpTime ? LONG_UPTIME : 0;
  sl_zigbee_set_beacon_classification_params(&param);
  sl_zigbee_af_core_println("%s: %s: %s",
                            TEST_HARNESS_Z3_PRINT_NAME,
                            "Set long up time",
                            (longUpTime
                             ? "True"
                             : "False"));
}

// plugin test-harness z3 nwk set-hub-connectivity <hubConnectivity:1>
void sli_zigbee_test_harness_z3_nwk_set_hub_connectivity(sl_cli_command_arg_t *arguments)
{
  bool hubConnectivity = (bool)sl_cli_get_argument_uint8(arguments, 0);
  sl_zigbee_beacon_classification_params_t param;
  sl_zigbee_get_beacon_classification_params(&param);

  param.beaconClassificationMask |= hubConnectivity ? TC_CONNECTIVITY : 0;
  sl_zigbee_set_beacon_classification_params(&param);
  sl_zigbee_af_core_println("%s: %s: %s",
                            TEST_HARNESS_Z3_PRINT_NAME,
                            "Set hub connectivity",
                            (hubConnectivity
                             ? "True"
                             : "False"));
}

// plugin test-harness z3 nwk set-parent-classification <parentClassification:1>
void sli_zigbee_test_harness_z3_nwk_set_parent_classification(sl_cli_command_arg_t *arguments)
{
  bool parentClassification = (bool)sl_cli_get_argument_uint8(arguments, 0);
  sl_zigbee_beacon_classification_params_t param;
  sl_zigbee_get_beacon_classification_params(&param);

  param.beaconClassificationMask |= parentClassification ? PRIORITIZE_BEACONS_BASED_ON_PARENT_CLASSIFICATION : 0;
  sl_zigbee_set_beacon_classification_params(&param);
  sl_zigbee_af_core_println("%s: %s: %s",
                            TEST_HARNESS_Z3_PRINT_NAME,
                            "Set parent classification",
                            (parentClassification
                             ? "True"
                             : "False"));
}
