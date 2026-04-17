/***************************************************************************//**
 * @file
 * @brief CLI commands various options.
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

// This is needed to bring in the sl_zigbee_af_get_binding_table_size() prototype.
#include "app/framework/util/af-main.h"
#include "app/framework/security/af-security.h"
#include "stack/include/zigbee-security-manager.h"
#include "stack/include/zigbee-device-stack.h"

void sli_zigbee_af_cli_service_discovery_callback(const sl_zigbee_af_service_discovery_result_t* result)
{
#if ((defined(EMBER_AF_PRINT_ENABLE) && defined(SL_ZIGBEE_AF_PRINT_APP)) \
  || (defined(SL_CATALOG_ZIGBEE_DEBUG_PRINT_PRESENT)))
  if (!sl_zigbee_af_have_discovery_response_status(result->status)) {
    // Do nothing
  } else if (result->zdoRequestClusterId == MATCH_DESCRIPTORS_REQUEST) {
    const sl_zigbee_af_endpoint_list_t* epList = (const sl_zigbee_af_endpoint_list_t*)result->responseData;
    UNUSED_VAR(epList);
    sl_zigbee_app_debug_println("Match %sy from 0x%04X, ep %d",
                                "discover",
                                result->matchAddress,
                                epList->list[0]);
  } else if (result->zdoRequestClusterId == NETWORK_ADDRESS_REQUEST) {
    sl_zigbee_app_debug_println("NWK Address response: 0x%04X", result->matchAddress);
  } else if (result->zdoRequestClusterId == IEEE_ADDRESS_REQUEST) {
    const uint8_t* eui64ptr = (uint8_t*)(result->responseData);
    sl_zigbee_af_app_print("IEEE Address response: ");
    sl_zigbee_af_print_big_endian_eui64(eui64ptr);
    sl_zigbee_app_debug_println("");
  } else {
    // MISRA requires ..else if.. to have a terminating else.
  }

  if (result->status != SL_ZIGBEE_AF_BROADCAST_SERVICE_DISCOVERY_RESPONSE_RECEIVED) {
    sl_zigbee_app_debug_println("Service %sy done.",
                                "discover");
  }
#endif //((defined(EMBER_AF_PRINT_ENABLE) && defined(SL_ZIGBEE_AF_PRINT_APP)) || (defined(SL_CATALOG_ZIGBEE_DEBUG_PRINT_PRESENT)))
}

static sl_802154_short_addr_t discoveryTargetNodeId = SL_ZIGBEE_RX_ON_WHEN_IDLE_BROADCAST_ADDRESS;

#include "app/util/serial/sl_zigbee_command_interpreter.h"
// Common include files:
//many of the variables are defined here
#include "app/framework/util/common.h"

void optionPrintRxCommand(sl_cli_command_arg_t *arguments);
void optionBindingTablePrintCommand(sl_cli_command_arg_t *arguments);

// option binding-table print
void optionBindingTablePrintCommand(sl_cli_command_arg_t *arguments)
{
  (void) arguments;
  uint8_t i;
  sl_zigbee_binding_table_entry_t result;

  const char * typeStrings[] = {
    "EMPTY",
    "UNICA",
    "M2ONE",
    "MULTI",
    "?    ",
  };
  uint8_t bindings = 0;

  sl_zigbee_core_debug_println("#  type   nwk  loc   rem   clus   node   eui");
  for (i = 0; i < sl_zigbee_af_get_binding_table_size(); i++) {
    sl_status_t status = sl_zigbee_get_binding(i, &result);
    if (status == SL_STATUS_OK) {
      if (result.type > SL_ZIGBEE_MULTICAST_BINDING) {
        result.type = 4;  // last entry in the string list above
      }
      if (result.type != SL_ZIGBEE_UNUSED_BINDING) {
        bindings++;
        sl_zigbee_core_debug_print("%d: ", i);
        sl_zigbee_core_debug_print("%s", typeStrings[result.type]);
        sl_zigbee_core_debug_print("  %d    0x%04X  0x%04X  0x%04X 0x%04X ",
                                   result.networkIndex,
                                   result.local,
                                   result.remote,
                                   result.clusterId,
                                   sl_zigbee_get_binding_remote_node_id(i));
        sl_zigbee_af_app_debug_exec(sl_zigbee_af_print_big_endian_eui64(result.identifier));
        sl_zigbee_core_debug_print("\n");
      }
    } else {
      sl_zigbee_core_debug_print("sl_zigbee_get_binding Error: 0x%08X\n", status);
      sl_zigbee_af_app_flush();
    }
    sl_zigbee_af_app_flush();
  }
  sl_zigbee_core_debug_println("%d of %d bindings used",
                               bindings,
                               sl_zigbee_af_get_binding_table_size());
}

// option binding-table clear
void optionBindingTableClearCommand(sl_cli_command_arg_t *arguments)
{
  (void) arguments;
  sl_zigbee_clear_binding_table();
}

// option print-rx-msgs [enable/disable]
void optionPrintRxCommand(sl_cli_command_arg_t *arguments)
{
  uint8_t position = sl_cli_get_command_count(arguments) - 1;

  uint8_t command_first_character = sl_cli_get_command_string(arguments, position)[0];
  if (command_first_character == 'e') {
    sl_zigbee_af_print_received_messages = true;
    sl_zigbee_core_debug_println("enabled print");
  } else {
    sl_zigbee_af_print_received_messages = false;
    sl_zigbee_core_debug_println("disabled print");
  }
}

// option register
void optionRegisterCommand(sl_cli_command_arg_t *arguments)
{
  (void) arguments;
  sl_zigbee_af_registration_start_cb();
}

void optionDiscoveryTargetCommand(sl_cli_command_arg_t *arguments)
{
  discoveryTargetNodeId = sl_cli_get_argument_uint16(arguments, 0);
}

// option disc <profileId> <clusterId>
void optionDiscoverCommand(sl_cli_command_arg_t *arguments)
{
  sl_zigbee_af_profile_id_t profileId = sl_cli_get_argument_uint16(arguments, 0);
  sl_zigbee_af_cluster_id_t clusterId = sl_cli_get_argument_uint16(arguments, 1);
  sl_zigbee_af_find_devices_by_profile_and_cluster(discoveryTargetNodeId,
                                                   profileId,
                                                   clusterId,
                                                   SL_ZIGBEE_AF_SERVER_CLUSTER_DISCOVERY,
                                                   sli_zigbee_af_cli_service_discovery_callback);
}

// option apsretry on/off/...
void optionApsRetryCommand(sl_cli_command_arg_t *arguments)
{
  uint8_t position = sl_cli_get_command_count(arguments) - 1;

  uint8_t command_first_character = sl_cli_get_command_string(arguments, position)[1];

  if ( command_first_character == 'e' ) {
    sl_zigbee_af_set_retry_override(SL_ZIGBEE_AF_RETRY_OVERRIDE_NONE);
  } else if ( command_first_character == 'n' ) {
    sl_zigbee_af_set_retry_override(SL_ZIGBEE_AF_RETRY_OVERRIDE_SET);
  } else if ( command_first_character == 'f' ) {
    sl_zigbee_af_set_retry_override(SL_ZIGBEE_AF_RETRY_OVERRIDE_UNSET);
  } else {
    // MISRA requires ..else if.. to have a terminating else.
  }
}

// option security aps <off | on>
void optionApsSecurityCommand(sl_cli_command_arg_t *arguments)
{
  uint8_t position = sl_cli_get_command_count(arguments) - 1;

  uint8_t command_first_character = sl_cli_get_command_string(arguments, position)[1];
  sli_zigbee_af_test_aps_security_override = (command_first_character == 'f'
                                              ? APS_TEST_SECURITY_DISABLED
                                              : APS_TEST_SECURITY_ENABLED);
}

// option link <index> <eui64 in big endian format> <key in big endian format>
void optionLinkCommand(sl_cli_command_arg_t *arguments)
{
  sl_802154_long_addr_t partnerEUI64;
  sl_zigbee_key_data_t newKey;
  sl_status_t status;
  //uint8_t i;
  uint8_t index = sl_cli_get_argument_uint8(arguments, 0);
  sl_zigbee_copy_eui64_arg(arguments, 1, partnerEUI64, true);
  sl_zigbee_copy_hex_arg((arguments), 2, sl_zigbee_key_contents((&newKey)), SL_ZIGBEE_ENCRYPTION_KEY_SIZE, true);
  status = sl_zigbee_sec_man_import_link_key(index,
                                             partnerEUI64,
                                             (sl_zigbee_sec_man_key_t*)&newKey);
  UNUSED_VAR(status);
  sl_zigbee_af_app_debug_exec(sli_zigbee_af_print_status("add link key", status));
  sl_zigbee_af_app_println("");
  sl_zigbee_af_core_flush();
}

// option install-code <index> <eui64> <install code>
void optionInstallCodeCommand(sl_cli_command_arg_t *arguments)
{
#if (defined(SL_ZIGBEE_AF_HAS_SECURITY_PROFILE_SE_TEST) \
  || defined(SL_ZIGBEE_AF_HAS_SECURITY_PROFILE_SE_FULL) \
  || defined (SL_ZIGBEE_AF_HAS_SECURITY_PROFILE_Z3)     \
  || defined(SL_ZIGBEE_AF_HAS_SECURITY_PROFILE_Z4))

  sl_802154_long_addr_t eui64;
  sl_zigbee_key_data_t key;
  sl_status_t status;
  uint8_t code[16 + 2]; // 6, 8, 12, or 16 bytes plus two-byte CRC
  uint8_t length;

  length = sl_zigbee_copy_hex_arg(arguments, 2, code, sizeof(code), false);

  // Convert the install code to a key.
  status = sli_zigbee_af_install_code_to_key(code, length, &key);

  if (SL_STATUS_OK != status) {
    if (SL_STATUS_INVALID_CONFIGURATION == status) {
      sl_zigbee_app_debug_println("ERR: Calculated CRC does not match");
    } else if (SL_STATUS_INVALID_PARAMETER == status) {
      sl_zigbee_app_debug_println("ERR: Install Code must be 8, 10, 14, or 18 bytes in "
                                  "length");
    } else {
      sl_zigbee_app_debug_println("ERR: AES-MMO hash failed: 0x%02X", status);
    }
    return;
  }

  sl_zigbee_copy_eui64_arg(arguments, 1, eui64, true);

  // In Smart Energy, initial install code keys are programmed to nvm, so they must go in the link key table
  // In Z3/Z4, the initial install code keys are ephemeral, so transient key table it is
#if defined(SL_ZIGBEE_AF_HAS_SECURITY_PROFILE_SE_TEST) || defined(SL_ZIGBEE_AF_HAS_SECURITY_PROFILE_SE_FULL)
  // Add the key to the link key table.

  status = sl_zigbee_sec_man_import_link_key(sl_cli_get_argument_uint8(arguments, 0),     // index
                                             eui64,
                                             (sl_zigbee_sec_man_key_t*) &key);
  sl_zigbee_af_app_debug_exec(sli_zigbee_af_print_status("add link key", status));

  sl_zigbee_app_debug_println("");
  sl_zigbee_af_app_flush();
#else // SL_ZIGBEE_AF_HAS_SECURITY_PROFILE_SE_TEST || SL_ZIGBEE_AF_HAS_SECURITY_PROFILE_SE_FULL
  // Add the key to the transient key table.
  // This will be used while the DUT joins.
  if (SL_STATUS_OK == status) {
    status = sl_zigbee_sec_man_import_transient_key(eui64, (sl_zigbee_sec_man_key_t*)&key);
    sl_zigbee_af_app_debug_exec(sli_zigbee_af_print_status("Set joining link key", status));
    sl_zigbee_app_debug_println("");
    sl_zigbee_af_app_flush();
  }
#endif // SL_ZIGBEE_AF_HAS_SECURITY_PROFILE_SE_TEST || SL_ZIGBEE_AF_HAS_SECURITY_PROFILE_SE_FULL

#else
  UNUSED_VAR(arguments);
  sl_zigbee_app_debug_println("This command only supports the Z3, Z4, or SE application profile.");
#endif
}

// option binding-table set <index> <cluster> <local ep> <remote ep> <EUI>
void optionBindingTableSetCommand(sl_cli_command_arg_t *arguments)
{
  sl_zigbee_binding_table_entry_t entry;
  uint8_t index = sl_cli_get_argument_uint8(arguments, 0);
  uint8_t endpoint = sl_cli_get_argument_uint8(arguments, 2);
  sl_status_t status = sl_zigbee_af_push_endpoint_network_index(endpoint);
  if (status == SL_STATUS_OK) {
    entry.type = SL_ZIGBEE_UNICAST_BINDING;
    entry.clusterId = (sl_zigbee_af_cluster_id_t)sl_cli_get_argument_uint16(arguments, 1);
    entry.local = endpoint;
    entry.remote = sl_cli_get_argument_uint8(arguments, 3);
    sl_zigbee_copy_eui64_arg(arguments, 4, entry.identifier, true);
    entry.networkIndex = sl_zigbee_get_current_network();
    status = sl_zigbee_set_binding(index, &entry);
    (void) sl_zigbee_af_pop_network_index();
  }
  sl_zigbee_app_debug_println("set bind %d: 0x%02x", index, status);
}
