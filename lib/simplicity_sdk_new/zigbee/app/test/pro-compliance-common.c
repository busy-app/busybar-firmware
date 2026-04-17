/***************************************************************************//**
 * @file
 * @brief Common functionality between the host and onboard versions
 * of Ember's Zigbee Pro Compliance application.
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
#endif // EZSP_HOST
#include "hal/hal.h"
#include "serial/serial.h"
#include "app/util/serial/sl_zigbee_command_interpreter.h"
#include "app/util/common/common.h"
#include "stack/include/source-route.h"
#include "stack/include/network-formation.h"
#include "stack/include/zigbee-security-manager.h"

#include "app/test/pro-compliance-common.h"
#include "app/test/test-profile.h"
#include "core/sl_zigbee_multi_network.h"
#if defined (EZSP_HOST)
extern uint8_t sl_zigbee_key_table_size;

  #define sl_zigbee_debug_printf(...) do {} while (false)
#endif
#include <inttypes.h>
#include "misc-common.h"  // for printFailedToErrorMessage()
#ifdef SL_COMPONENT_CATALOG_PRESENT
#include "sl_component_catalog.h"
#endif

extern uint16_t sli_zigbee_stack_get_transient_key_timeout_s(void);
extern void sli_zigbee_refresh_aps_transient_key_timeout_seconds(sl_802154_long_addr_t device_long, uint16_t refresh_duration);

extern uint16_t sli_zigbee_stack_get_transient_key_timeout_s(void);
extern void sli_zigbee_refresh_aps_transient_key_timeout_seconds(sl_802154_long_addr_t device_long, uint16_t refresh_duration);

#ifdef SL_CATALOG_ZIGBEE_TEST_HARNESS_Z3_PRESENT
extern void keysPrintCommand(SL_CLI_COMMAND_ARG);
extern void keysDeleteCommand(SL_CLI_COMMAND_ARG);
extern void keysClearCommand(SL_CLI_COMMAND_ARG);
#endif
//------------------------------------------------------------------------------
// Globals

bool sli_zigbee_note_incoming_packet_enabled = true;

sl_zigbee_af_event_t wildcard_key_refresh_event[4] = { 0, };

static sl_zigbee_sec_man_key_t za09key = {
  .key = { 0x5A, 0x69, 0x67, 0x42, 0x65, 0x65, 0x41, 0x6C, 0x6C, 0x69, 0x61, 0x6E, 0x63, 0x65, 0x30, 0x39 }
};
static sl_802154_long_addr_t wildcard_long = {
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};

//------------------------------------------------------------------------------
// External Declarations

extern sl_zigbee_outgoing_message_type_t outgoingMessageType;

#ifdef MAC_TEST_COMMANDS_SUPPORT
// app/test/mac-test-commands.c
extern void sl_zigbee_scan_return(uint8_t channel, sl_status_t status);
#endif  // MAC_TEST_COMMANDS_SUPPORT

// stack/routing/zigbee/association.h
extern bool sli_zigbee_enable_mac_certification_test_mode;

// This file is compiled by host and non-host versions and
// the function sli_zigbee_stack_erase_key_table_entry() is a #define for the onboard version
// and a real function for the EZSP version.  Therefore we must break out the
// real functionality so that the compiler can compile the same object code for
// this file.  This is only an issue for the simulation executable.
sl_status_t eraseKeyTableEntryZCP(uint8_t index);

uint32_t getApsFrameCounter(void);

//------------------------------------------------------------------------------
// Forward Declarations

static bool endpointMatch(uint8_t ourEndpoint,
                          sl_zigbee_incoming_message_type_t type,
                          uint8_t messageEndpoint,
                          uint8_t groupId);

//------------------------------------------------------------------------------
// Functions

void nwkUpdateCommand(SL_CLI_COMMAND_ARG)
{
  sl_802154_short_addr_t target  = sl_cli_get_argument_uint16(arguments, 0);
  uint32_t scanChannels;
  uint8_t scanDuration = sl_cli_get_argument_uint8(arguments, 3);
  uint16_t scanCountOrNwkManagerAddress = sl_cli_get_argument_uint16(arguments, 4);

  scanChannels = sl_cli_get_argument_uint32(arguments, 1);
  scanChannels |= ((sl_cli_get_argument_uint32(arguments, 2)) << 16);

  if (SL_STATUS_OK != sl_zigbee_energy_scan_request(target,
                                                    scanChannels,
                                                    scanDuration,
                                                    scanCountOrNwkManagerAddress)) {
    printFailedToErrorMessage("send NWK Update.");
  }
}

void manyToOneCommand(SL_CLI_COMMAND_ARG)
{
  bool highRam = (bool)sl_cli_get_argument_uint32(arguments, 0);
  uint8_t radius = sl_cli_get_argument_uint8(arguments, 1);
  if (sl_zigbee_send_many_to_one_route_request((highRam
                                                ? SL_ZIGBEE_HIGH_RAM_CONCENTRATOR
                                                : SL_ZIGBEE_LOW_RAM_CONCENTRATOR),
                                               radius)
      != SL_STATUS_OK) {
    printFailedToErrorMessage("send many to one route req.");
  }
}

void setSendOptionsCommand(SL_CLI_COMMAND_ARG)
{
  outgoingMessageType = sl_cli_get_argument_uint8(arguments, 0);
}

void linkKeyCommand(SL_CLI_COMMAND_ARG)
{
  sl_802154_long_addr_t partnerEui64;
  sl_zigbee_key_data_t newKey;
  sl_zigbee_copy_eui64_arg(arguments, 0, partnerEui64, false);
  if (!(sl_zigbee_copy_hex_arg(arguments, 1, sl_zigbee_key_contents(&newKey), SL_ZIGBEE_ENCRYPTION_KEY_SIZE, true))) {
    sl_zigbee_generate_random_key(&newKey);
  }
  if (SL_STATUS_OK != sl_zigbee_sec_man_import_link_key(0xFF,
                                                        partnerEui64,
                                                        (sl_zigbee_sec_man_key_t *) &newKey)) {
    //failed to add this key
    printFailedToErrorMessage("add link key.");
  }
}

static uint32_t transient_key_refresh_delay_ms(void)
{
  return (sli_zigbee_stack_get_transient_key_timeout_s() - 1) * 1000u;
}

static sl_status_t add_transient_wellknown_key(void)
{
  return sl_zigbee_sec_man_import_transient_key(wildcard_long, &za09key);
}

static sl_status_t remove_transient_wellknown_key(void)
{
  sl_zigbee_sec_man_context_t ctx;
  sl_zigbee_sec_man_init_context(&ctx);
  memset(&ctx.eui64, 0xFF, EUI64_SIZE);
  ctx.flags |= ZB_SEC_MAN_FLAG_EUI_IS_VALID;
  return sl_zigbee_sec_man_delete_transient_key(&ctx);
}

void wildcard_key_refresh_event_handler(sl_zigbee_af_event_t * event)
{
  sli_zigbee_refresh_aps_transient_key_timeout_seconds(wildcard_long, sli_zigbee_stack_get_transient_key_timeout_s());
  sl_zigbee_af_event_set_delay_ms(event, transient_key_refresh_delay_ms());
}

void openWildcardZa09JoinWindow(SL_CLI_COMMAND_ARG)
{
#ifdef SL_CATALOG_CLI_PRESENT
  UNUSED_VAR(arguments);
#endif // SL_CATALOG_CLI_PRESENT
  uint8_t nwkIndex = sl_zigbee_get_current_network();
  sl_status_t status = add_transient_wellknown_key();

  sl_zigbee_core_debug_println("open wildcard key 0x%02lX \r\n",
                               (unsigned long)status);
  if (status == SL_STATUS_OK) {
    sl_zigbee_af_event_set_delay_ms(&wildcard_key_refresh_event[nwkIndex], transient_key_refresh_delay_ms());
  }
}

void closeWildcardZa09JoinWindow(SL_CLI_COMMAND_ARG)
{
#ifdef SL_CATALOG_CLI_PRESENT
  UNUSED_VAR(arguments);
#endif // SL_CATALOG_CLI_PRESENT
  uint8_t nwkIndex = sl_zigbee_get_current_network();
  sl_status_t status = remove_transient_wellknown_key();
  sl_zigbee_core_debug_println("close wildcard key 0x%02lX \r\n",
                               (unsigned long)status);
  sl_zigbee_af_event_set_inactive(&wildcard_key_refresh_event[nwkIndex]);
}

#ifdef SL_CATALOG_ZIGBEE_TEST_HARNESS_Z3_PRESENT
void sl_zigbee_switch_network_key_handler_pro_compliance(uint8_t sequenceNumber)
#else
void sl_zigbee_switch_network_key_handler(uint8_t sequenceNumber)
#endif
{
  sl_zigbee_core_debug_println("Switched to NWK Key %d.",
                               sequenceNumber);
  (void) sli_legacy_serial_wait_send(serialPort);
}

#ifdef SL_CATALOG_ZIGBEE_TEST_HARNESS_Z3_PRESENT
sl_status_t sl_zigbee_internal_pan_id_conflict_handler_pro_compliance(int8_t conflictCount)
#else
sl_status_t sl_zigbee_internal_pan_id_conflict_handler(int8_t conflictCount)
#endif
{
  (void) conflictCount;
  sli_zigbee_set_current_network(sl_zigbee_get_callback_network());
  sl_zigbee_send_pan_id_update(halCommonGetRandom());
  sli_zigbee_restore_current_network();

  return SL_STATUS_OK;
}

void deleteKeyCommand(SL_CLI_COMMAND_ARG)
{
  sl_802154_long_addr_t entryAddress;
  sl_zigbee_copy_eui64_arg(arguments, 0, entryAddress, false);

  sl_zigbee_sec_man_context_t context;
  sl_zigbee_sec_man_export_link_key_by_eui(entryAddress, &context, NULL, NULL);

  if ( context.key_index == 0xFF ) {
    printErrorMessage("No such address");
    (void) sli_legacy_serial_wait_send(serialPort);
    return;
  }
  eraseKeyTableEntryZCP(context.key_index);
}

void clearKeysCommand(SL_CLI_COMMAND_ARG)
{
#ifdef SL_CATALOG_CLI_PRESENT
  UNUSED_VAR(arguments);
#endif // SL_CATALOG_CLI_PRESENT
  for (uint8_t i = 0; i < sl_zigbee_key_table_size; i++) {
    eraseKeyTableEntryZCP(i);
  }
}

void noteIncomingCommand(SL_CLI_COMMAND_ARG)
{
  (void)arguments;
  sli_zigbee_note_incoming_packet_enabled = (sl_cli_get_argument_uint32(arguments, 0) > 0);

  sl_zigbee_core_debug_println("Note incoming %s",
                               (sli_zigbee_note_incoming_packet_enabled ? "enabled" : "disabled"));
}

void printEncryptionKey2(uint8_t port, uint8_t* key)
{
  printEncryptionKey(port, key);
  (void) sli_legacy_serial_wait_send(port);
  printCarriageReturn();
}

static uint8_t printKeyTable(uint8_t keyType, bool preconfiguredKey)
{
  uint8_t entriesUsed = 0;
  sl_zigbee_core_debug_print("IEEE Address FC Type Auth Key\n");

  for (uint8_t i = 0; i < sl_zigbee_key_table_size; i++ ) {
    sl_zigbee_key_data_t* keyToPrint;
    sl_zigbee_key_data_t derivedKey;

    sl_zigbee_sec_man_context_t context;
    sl_zigbee_sec_man_init_context(&context);
    sl_zigbee_sec_man_key_t plaintext_key;
    sl_zigbee_sec_man_aps_key_metadata_t key_data;

    context.core_key_type = SL_ZB_SEC_MAN_KEY_TYPE_APP_LINK;

    if (preconfiguredKey) {
      i = 0xFE; // last

      context.core_key_type = SL_ZB_SEC_MAN_KEY_TYPE_TC_LINK;
      // Try to get whatever key type is stored in the preconfigured key slot.
      if (SL_STATUS_OK != sl_zigbee_sec_man_export_key(&context, &plaintext_key)) {
        continue;
      }
    } else if ( SL_STATUS_OK != sl_zigbee_sec_man_export_link_key_by_index(i, &context, &plaintext_key, &key_data) ) {
      continue;
    }

    context.flags |= ZB_SEC_MAN_FLAG_KEY_INDEX_IS_VALID;
    (void) sl_zigbee_sec_man_get_aps_key_info(&context, &key_data);
    if ( keyType != NO_DERIVED_KEY ) {
      sl_zigbee_sec_man_load_key_context(&context);
      sl_zigbee_sec_man_hmac_aes_mmo(&keyType, 1, (uint8_t *) &derivedKey);
      keyToPrint = &derivedKey;
    } else {
      keyToPrint = (sl_zigbee_key_data_t*) &plaintext_key;
    }

    printLittleEndianEui64(serialPort, context.eui64);
    sl_zigbee_core_debug_print("  %4lx  ", (unsigned long)key_data.incoming_frame_counter);
    sl_zigbee_core_debug_print("%c     %c    ",
                               ((key_data.bitmask & SL_ZIGBEE_KEY_IS_AUTHENTICATION_TOKEN)
                                ? 'A'
                                : 'L'),
                               ((key_data.bitmask & SL_ZIGBEE_KEY_IS_AUTHORIZED)
                                ? 'y'
                                : 'n'));

    // Print one less space since printEncryptionKey2 ends up prepending one
    // before the key.
    printEncryptionKey2(serialPort, sl_zigbee_key_contents(keyToPrint));
    (void) sli_legacy_serial_wait_send(serialPort);
    entriesUsed++;
  }
  return entriesUsed;
}

static uint8_t printTransientKeyTable(void)
{
  sl_status_t status;
  sl_zigbee_sec_man_context_t context;
  sl_zigbee_sec_man_key_t plaintext_key;
  sl_zigbee_sec_man_aps_key_metadata_t key_info;
  uint8_t index = 0;

  sl_zigbee_core_debug_println("Index IEEE Address         NWKIndex  In FC     TTL(s) Flag    Key    ");
  (void)sli_legacy_serial_wait_send(serialPort);

  status = sl_zigbee_sec_man_export_transient_key_by_index(index,
                                                           &context,
                                                           &plaintext_key,
                                                           &key_info);

  while (status == SL_STATUS_OK) {
    sl_zigbee_core_debug_print("%d     ", index);
    printBigEndianEui64(serialPort, context.eui64);
    sl_zigbee_core_debug_print("  %d       ", context.multi_network_index);
    sl_zigbee_core_debug_print("  %4lx  ", (unsigned long)key_info.incoming_frame_counter);
    sl_zigbee_core_debug_print("0x%02X", key_info.ttl_in_seconds);
    sl_zigbee_core_debug_print(" 0x%02X  ", key_info.bitmask);
    sl_zigbee_core_debug_print("%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X",
                               plaintext_key.key[0],
                               plaintext_key.key[1],
                               plaintext_key.key[2],
                               plaintext_key.key[3],
                               plaintext_key.key[4],
                               plaintext_key.key[5],
                               plaintext_key.key[6],
                               plaintext_key.key[7],
                               plaintext_key.key[8],
                               plaintext_key.key[9],
                               plaintext_key.key[10],
                               plaintext_key.key[11],
                               plaintext_key.key[12],
                               plaintext_key.key[13],
                               plaintext_key.key[14],
                               plaintext_key.key[15]
                               );
    sl_zigbee_core_debug_println("");
    (void)sli_legacy_serial_wait_send(serialPort);

    index += 1;
    status = sl_zigbee_sec_man_export_transient_key_by_index(index,
                                                             &context,
                                                             &plaintext_key,
                                                             &key_info);
  }

  return index;
}

// Print the Master Key and Link Keys Table
void keysCommand(SL_CLI_COMMAND_ARG)
{
#ifdef SL_CATALOG_ZIGBEE_TEST_HARNESS_Z3_PRESENT
  // This workaround for the conflict that pro-compliance has keys command
  // and zcl core has keys groupd command, which has print/delete/clear
  if (sl_cli_get_argument_count(arguments) >= 1) {
    char *option = sl_cli_get_argument_string(arguments, 0);
    if (strcmp(option, "print") == 0) {
      keysPrintCommand(arguments);
    } else if (strcmp(option, "delete") == 0) {
      keysDeleteCommand(arguments);
    } else if (strcmp(option, "clear") == 0) {
      keysClearCommand(arguments);
    } else {
      sl_zigbee_core_debug_println("Invalid argument");
    }
    return;
  }
#else
  (void)arguments;
#endif

  uint8_t entriesUsed = 0;

  sl_zigbee_core_debug_println("Link Key FC: %4lx", (unsigned long)getApsFrameCounter());

  sl_zigbee_core_debug_println("Preconfigured Key");
  (void) sli_legacy_serial_wait_send(serialPort);
  printKeyTable(NO_DERIVED_KEY, true);

  sl_zigbee_core_debug_println("Link Key Table");
  (void) sli_legacy_serial_wait_send(serialPort);
  entriesUsed = printKeyTable(NO_DERIVED_KEY, false);

  sl_zigbee_core_debug_println("%d/%d entries",
                               entriesUsed,
                               sl_zigbee_key_table_size);
  (void) sli_legacy_serial_wait_send(serialPort);

  sl_zigbee_core_debug_println("Transient Key Table");
  (void) sli_legacy_serial_wait_send(serialPort);

  entriesUsed = printTransientKeyTable();

  sl_zigbee_core_debug_println("%d entr%s consuming %d packet buffer%s.\n",
                               entriesUsed,
                               entriesUsed > 1 ? "ies" : "y",
                               entriesUsed,
                               entriesUsed > 1 ? "s" : "");
  (void) sli_legacy_serial_wait_send(serialPort);
}

//------------------------------------------------------------------------------
// Send to all recipients in our binding table that match the local
// source endpoint and cluster ID passed in.  Cluster must be either
// Buffer Test Request (0x001C), or Freeform Request (0xA0A8).

void sendViaBind(SL_CLI_COMMAND_ARG)
{
  uint8_t sourceEndpoint = sl_cli_get_argument_uint8(arguments, 0);
  uint16_t clusterId     = sl_cli_get_argument_uint16(arguments, 1);
  uint8_t data           = sl_cli_get_argument_uint8(arguments, 2);
  bool bindingFound = false;
  uint8_t i;
  sl_zigbee_outgoing_message_type_t temp = outgoingMessageType;

  if ( !(clusterId == CLUSTER_BUFFER_TEST_REQUEST
         || clusterId == CLUSTER_FREEFORM_MSG_REQUEST) ) {
    sl_zigbee_core_debug_println("Unsupported cluster ID for send via binding.");
    return;
  }

  outgoingMessageType = SL_ZIGBEE_OUTGOING_VIA_BINDING;
  for (i = 0; i < sl_zigbee_get_binding_table_size(); i++) {
    sl_zigbee_binding_table_entry_t binding;
    sl_status_t status = sl_zigbee_get_binding(i, &binding);
    if (status == SL_STATUS_OK
        && binding.local == sourceEndpoint
        && binding.clusterId == clusterId) {
      sl_zigbee_aps_option_t options = (SL_ZIGBEE_APS_OPTION_RETRY
                                        | SL_ZIGBEE_APS_OPTION_ENABLE_ROUTE_DISCOVERY
                                        | SL_ZIGBEE_APS_OPTION_ENABLE_ADDRESS_DISCOVERY);
      sl_802154_short_addr_t groupDest = SL_ZIGBEE_NULL_NODE_ID;

      if (binding.type == SL_ZIGBEE_MULTICAST_BINDING) {
        options |= ZIGBEE_APS_FRAME_CONTROL_MODE_MULTICAST;
        groupDest = (sl_802154_short_addr_t)(binding.identifier[0]
                                             + (((uint16_t)(binding.identifier[1])) << 8));
      }
      bindingFound = true;
      status = transmitMessage((binding.type == SL_ZIGBEE_MULTICAST_BINDING
                                ? groupDest
                                : i),
                               1,
                               &data,
                               sourceEndpoint,
                               binding.remote,
                               binding.clusterId,
                               NULL,
                               options,
                               DEFAULT_MULTICAST_NWK_BROADCAST_ADDRESS);
      if (status == SL_STATUS_OK) {
        sl_zigbee_core_debug_println("Sent to binding %d", i);
      } else {
        sl_zigbee_core_debug_print(// this is broken up to duplicate the const string
                                   // above in printFailedToErrorMessage(), so it
                                   // can be compressed by the XAP compiler.
                                   // However we don't want to print the CR just yet,
                                   // so we don't just use that function.
          "%sFailed to %s",
          "Error: ",
          "send to binding ");
        sl_zigbee_core_debug_println("%d: 0x%0x",
                                     i,
                                     status);
      }
    }
  }
  if (!bindingFound) {
    sl_zigbee_core_debug_println("%sNo bindings found with src ep %d and cluster ID 0x%02x",
                                 "Error: ",
                                 sourceEndpoint,
                                 clusterId);
  }
  outgoingMessageType = temp;
  (void) sli_legacy_serial_wait_send(serialPort);
}

//------------------------------------------------------------------------------
// Handlers

void networkFoundHandler(sl_zigbee_zigbee_network_t *networkFound)
{
  sl_zigbee_core_debug_print("Nwk found: ch %d, PAN ID %02X, join %02X, profile %0X, id:%d ",
                             networkFound->channel,
                             networkFound->panId,
                             networkFound->allowingJoin,
                             networkFound->stackProfile,
                             networkFound->nwkUpdateId);
  printLittleEndianEui64(serialPort, networkFound->extendedPanId);
  printCarriageReturn();
}

void scanCompleteHandler(uint8_t channel, sl_status_t status)
{
#ifdef MAC_TEST_COMMANDS_SUPPORT
  if (sli_zigbee_enable_mac_certification_test_mode) {
    sl_zigbee_scan_return(channel, status);
  } else {
#endif  // MAC_TEST_COMMANDS_SUPPORT
  printCommandStatus(status, "Scan complete", "Scan failed");
#ifdef MAC_TEST_COMMANDS_SUPPORT
}
#endif  // MAC_TEST_COMMANDS_SUPPORT
}

void stackStatusHandler(sl_status_t status)
{
  if (status == SL_STATUS_ZIGBEE_NETWORK_OPENED
      || status == SL_STATUS_ZIGBEE_NETWORK_CLOSED) {
    return;
  }

  switch (status) {
    case SL_STATUS_NETWORK_UP:
    case SL_STATUS_ZIGBEE_NODE_ID_CHANGED:
    case SL_STATUS_ZIGBEE_PAN_ID_CHANGED:
    case SL_STATUS_ZIGBEE_CHANNEL_CHANGED:
    case SL_STATUS_ZIGBEE_TRUST_CENTER_SWAP_EUI_HAS_CHANGED:
    case SL_STATUS_ZIGBEE_TRUST_CENTER_SWAP_EUI_HAS_NOT_CHANGED:
      sl_zigbee_core_debug_println("Stack up id:0x%04X",
                                   sl_zigbee_get_node_id());
      // To make life easier, we allow joining by default when the stack comes up.
      // We could do that for the 250 version by manipulating the permitJoining
      // global directly, but that doesn't work for the 260.  So we do it here.
      sl_zigbee_permit_joining(0xFF);
      break;
    default:
      sl_zigbee_core_debug_print("Stack down status:0x%02X\n",
                                 status);
      break;
  }

  // Additional info for special cases
  switch (status) {
    case SL_STATUS_ZIGBEE_NODE_ID_CHANGED:
      sl_zigbee_core_debug_println("Node ID has changed");
      break;
    case SL_STATUS_ZIGBEE_PAN_ID_CHANGED:
      sl_zigbee_core_debug_println("PAN ID has changed");
      break;
    case SL_STATUS_ZIGBEE_CHANNEL_CHANGED:
      sl_zigbee_core_debug_println("Network channel has changed");
      break;
    case SL_STATUS_ZIGBEE_TRUST_CENTER_SWAP_EUI_HAS_CHANGED:
      sl_zigbee_core_debug_println("TCSO EUI has changed");
      break;
    case SL_STATUS_ZIGBEE_TRUST_CENTER_SWAP_EUI_HAS_NOT_CHANGED:
      sl_zigbee_core_debug_println("TCSO EUI has not changed");
      break;
    default:
      break;
  }
}

extern void slx_zigbee_application_handle_new_aps_link_key_with_partner(sl_802154_long_addr_t partner);

void keyEstablishmentHandler(sl_802154_long_addr_t partner, sl_zigbee_key_status_t status)
{
  const char *msg;
  switch (status) {
    case SL_ZIGBEE_APP_LINK_KEY_ESTABLISHED:
    case SL_ZIGBEE_TRUST_CENTER_LINK_KEY_ESTABLISHED:
      msg = "Key established";
      break;
    case SL_ZIGBEE_TC_RESPONDED_TO_KEY_REQUEST:
    case SL_ZIGBEE_TC_APP_KEY_SENT_TO_REQUESTER:
      msg = "TC answered key request";
      break;
    case SL_ZIGBEE_VERIFY_LINK_KEY_SUCCESS:
      msg = "Key verified";
      slx_zigbee_application_handle_new_aps_link_key_with_partner(partner);
      break;
    case SL_ZIGBEE_TC_REQUESTER_VERIFY_KEY_SUCCESS:
      msg = "TC verified key request";
      slx_zigbee_application_handle_new_aps_link_key_with_partner(partner);
      break;
    case SL_ZIGBEE_TC_REQUESTER_VERIFY_KEY_TIMEOUT:
      msg = "timeout";
      break;
    default:
      msg = "Error: Failed to establish key";
      break;
  }

  sl_zigbee_core_debug_println("%s: 0x%02X",
                               msg,
                               status);
  (void) sli_legacy_serial_wait_send(serialPort);
}

void childJoinHandler(uint8_t index,
                      bool joining,
                      sl_802154_short_addr_t newNodeId,
                      sl_802154_long_addr_t childEui64,
                      sl_zigbee_node_type_t childType)
{
  (void)index;
  sl_zigbee_core_debug_print("%s indication %02X ",
                             joining ? "Join" : "Leave",
                             newNodeId);
  printLittleEndianEui64(serialPort, childEui64);
  sl_zigbee_core_debug_print(" (type: %d)\n", childType);
}

void incomingRouteErrorHandler(sl_status_t status, sl_802154_short_addr_t target)
{
  sl_zigbee_core_debug_println("%s0x%02X for 0x%04X",
                               "Route error ",
                               status,
                               target);
  sl_zigbee_debug_printf("%s, errCode 0x%08" PRIX32 " target 0x%04X",
                         "Route error",
                         status,
                         target);
}

void incomingMessageHandler(sl_zigbee_incoming_message_type_t type,
                            sl_zigbee_aps_frame_t *apsFrame,
                            sl_zigbee_rx_packet_info_t *packetInfo,
                            uint8_t messageLength,
                            uint8_t *message)
{
  uint8_t destEndpoint = apsFrame->destinationEndpoint;
  bool broadcastEndpoint = (apsFrame->destinationEndpoint == 0xFF);
  noteIncoming(type, apsFrame, packetInfo, messageLength, message);
  if (apsFrame->profileId == 0xFFFF) {
    // Wildcard profile.
    // Since we only handle a single profile ID, we override
    // the profile Id in the incoming message so that any
    // outgoing message response uses the actual profile ID.
    apsFrame->profileId = testProfileId;
  }

  if (apsFrame->profileId != testProfileId) {
    return;
  }
  for (uint8_t i = 0; i < sl_zigbee_endpoint_count; i++) {
    if (endpointMatch(sl_zigbee_endpoints[i].endpoint,
                      type,
                      destEndpoint,
                      (uint8_t)apsFrame->groupId)) {
      // Rewrite the APS Frame for the broadcast endpoint
      if ( broadcastEndpoint ) {
        apsFrame->destinationEndpoint = sl_zigbee_endpoints[i].endpoint;
      }
      testProfileMessageHandler(packetInfo->sender_short_id, apsFrame, messageLength, message);
    }
  }
}

void noteIncoming(sl_zigbee_incoming_message_type_t type,
                  sl_zigbee_aps_frame_t *apsFrame,
                  sl_zigbee_rx_packet_info_t *packetInfo,
                  uint8_t messageLength,
                  uint8_t *message)
{
  if (!sli_zigbee_note_incoming_packet_enabled) {
    return;
  }

  sl_zigbee_core_debug_print("Incoming MSG from %04X (",
                             packetInfo->sender_short_id);
  if (type == SL_ZIGBEE_INCOMING_MULTICAST
      || type == SL_ZIGBEE_INCOMING_MULTICAST_LOOPBACK) {
    sl_zigbee_core_debug_print("Ep %02X -> Group %04X",
                               apsFrame->sourceEndpoint,
                               apsFrame->groupId);
  } else {
    sl_zigbee_core_debug_print("%02X -> %02X",
                               apsFrame->sourceEndpoint,
                               apsFrame->destinationEndpoint);
  }
  sl_zigbee_core_debug_println(", cluster %04X, profile %04X)",
                               apsFrame->clusterId,
                               apsFrame->profileId);

  printMessagePayload(apsFrame, messageLength, message);

  (void) sli_legacy_serial_wait_send(serialPort);
}

void printMessagePayload(sl_zigbee_aps_frame_t *apsFrame,
                         uint8_t messageLength,
                         uint8_t *message)
{
  (void)messageLength;

  switch (apsFrame->clusterId) {
    case MATCH_DESCRIPTORS_RESPONSE: {
      sl_zigbee_core_debug_println("Match descr. resp.: status %0X, length %0X", message[1], message[4]);
      break;
    }
    // add here other clusters.

    default:
      break;
  }
}

//------------------------------------------------------------------------------
// Internal Functions

static bool endpointMatch(uint8_t ourEndpoint,
                          sl_zigbee_incoming_message_type_t type,
                          uint8_t messageEndpoint,
                          uint8_t groupId)
{
  // XXX: This doesn't handle all cases.
  // We should determine whether this was a NWK or APS multicast
  // and whether we are using one or the other.
  if (type == SL_ZIGBEE_INCOMING_MULTICAST
      || type == SL_ZIGBEE_INCOMING_MULTICAST_LOOPBACK) {
    return lookupGroupEndpoint(ourEndpoint, groupId);
  } else {
    return (messageEndpoint == ourEndpoint
            || messageEndpoint == 0xFF);
  }
}
