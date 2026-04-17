/***************************************************************************//**
 * @file
 * @brief This file handles common routines for applications
 * that implement basic Security.
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
// HAL.
#include "hal/hal.h"

// Application utilities.
#include "serial/serial.h"
#include "app/util/serial/sl_zigbee_command_interpreter.h"
#include "app/util/common/common.h"
#include "app/util/zigbee-framework/zigbee-device-library.h"

// Unreleased security policy stack features.
#include "stack/zigbee/aps-security-policy.h"

#include "misc-common.h"  // for printFailedToErrorMessage()
#include "security-common.h"
#include "stack/include/zigbee-security-manager.h"

#if defined(EZSP_HOST)
bool sl_zigbee_stack_is_up(void);
sl_status_t sl_zigbee_generate_random_key(sl_zigbee_key_data_t* result);
#endif

//------------------------------------------------------------------------------
// External Declarations

//------------------------------------------------------------------------------
// Globals

// Assume Trust Center == Coordinator
#define sli_zigbee_am_trust_center (sl_zigbee_get_node_id() == 0x0000)

//------------------------------------------------------------------------------

bool isSecurityStateValid(void)
{
  if ( !sl_zigbee_stack_is_up() || getSecurityLevel() == 0 || !sli_zigbee_am_trust_center ) {
    printErrorMessage("Invalid stack state for command.");
    return false;
  }

  return true;
}

//------------------------------------------------------------------------------
// Indicates when the network key has been changed.
WEAK(void sl_zigbee_switch_network_key_handler(uint8_t sequenceNumber))
{
  sl_zigbee_core_debug_println("Switched to NWK Key %d.",
                               sequenceNumber);
  (void) sli_legacy_serial_wait_send(serialPort);
}

//------------------------------------------------------------------------------

void printYesOrNo(uint16_t yes)
{
  const char * yesNo[] = { "No",
                           "Yes" };
  sl_zigbee_core_debug_println("%s", yesNo[(yes > 0 ? 1 : 0)]);
  (void) sli_legacy_serial_wait_send(serialPort);
}

//------------------------------------------------------------------------------

void printStringWithYesOrNo(const char * formattedString, uint16_t yes)
{
  sl_zigbee_core_debug_println(formattedString);
  printYesOrNo(yes);
}

// Set the default joining behavior on the trust center
void setTrustCenterPolicyCommand(sl_cli_command_arg_t *arguments)
{
  uint8_t command = sl_cli_get_argument_uint8(arguments, 0);

  (void) setTrustCenterJoinDecision(command);
}

// CLI Command
// Print the security level, if not 0 print additional security info.
//   Trust center address, current key, and alternate key
void getSecurityCommand(sl_cli_command_arg_t *arguments)
{
  UNUSED_VAR(arguments);
  const char * frameCounterText = "  Frame Counter ";
  uint8_t key;
  uint8_t securityLevel = getSecurityLevel();
  sl_zigbee_core_debug_print("Level: %d\n",
                             securityLevel);
  if ( securityLevel > 0 && securityLevel != 0xFF ) {
    // Must match the ordering of sl_zigbee_key_type_t
    const char * keyText[] = { "TC Link Key",
                               "TC Master",
                               "Current NWK Key",
                               "Alt. NWK Key" };

    printSecurityInfo();

    printCarriageReturn();
    sl_zigbee_core_debug_println("EA:");

    for ( key = SL_ZIGBEE_TRUST_CENTER_LINK_KEY;  // assumed to be first key
          key <= SL_ZIGBEE_NEXT_NETWORK_KEY;
          key++ ) {
      sl_zigbee_sec_man_context_t context;
      sl_zigbee_sec_man_key_t plaintext_key;
      //each key type here will use one of these two structs
      UNUSED sl_zigbee_sec_man_aps_key_metadata_t metadata;
      UNUSED sl_zigbee_sec_man_network_key_info_t nwk_key_info;

      sl_zigbee_key_struct_t keyStruct = { 0 };
      sl_zigbee_sec_man_init_context(&context);

      switch (key) {
        case SL_ZIGBEE_TRUST_CENTER_LINK_KEY:

          context.core_key_type = SL_ZB_SEC_MAN_KEY_TYPE_TC_LINK;
          sl_zigbee_sec_man_export_key(&context, &plaintext_key);
          sl_zigbee_sec_man_get_aps_key_info(&context, &metadata);
          keyStruct.bitmask = metadata.bitmask;
          keyStruct.incomingFrameCounter = metadata.incoming_frame_counter;
          keyStruct.outgoingFrameCounter = metadata.outgoing_frame_counter;
          break;
        case SL_ZIGBEE_CURRENT_NETWORK_KEY:
          context.core_key_type = SL_ZB_SEC_MAN_KEY_TYPE_NETWORK;
          sl_zigbee_sec_man_export_key(&context, &plaintext_key);
          sl_zigbee_sec_man_get_network_key_info(&nwk_key_info);
          keyStruct.bitmask |= SL_ZIGBEE_KEY_HAS_SEQUENCE_NUMBER;
          keyStruct.bitmask |= SL_ZIGBEE_KEY_HAS_OUTGOING_FRAME_COUNTER;
          keyStruct.sequenceNumber = nwk_key_info.network_key_sequence_number;
          keyStruct.outgoingFrameCounter = nwk_key_info.network_key_frame_counter;
          break;
        case SL_ZIGBEE_NEXT_NETWORK_KEY:
          context.core_key_type = SL_ZB_SEC_MAN_KEY_TYPE_NETWORK;
          context.key_index = 1;
          sl_zigbee_sec_man_export_key(&context, &plaintext_key);
          sl_zigbee_sec_man_get_network_key_info(&nwk_key_info);
          keyStruct.bitmask |= SL_ZIGBEE_KEY_HAS_SEQUENCE_NUMBER;
          keyStruct.bitmask |= SL_ZIGBEE_KEY_HAS_OUTGOING_FRAME_COUNTER;
          keyStruct.sequenceNumber = nwk_key_info.alt_network_key_sequence_number;
          keyStruct.outgoingFrameCounter = nwk_key_info.network_key_frame_counter;
          break;
      }

      switch ( key ) {
        case SL_ZIGBEE_TRUST_CENTER_LINK_KEY:
        case SL_ZIGBEE_CURRENT_NETWORK_KEY:
        case SL_ZIGBEE_NEXT_NETWORK_KEY:
          // Nothing more to do.
          break;
        default:
          continue;
      }
      sl_zigbee_core_debug_println("");
      sl_zigbee_core_debug_println("%s", keyText[key - 1]);

      if ( keyStruct.bitmask & SL_ZIGBEE_KEY_HAS_SEQUENCE_NUMBER ) {
        sl_zigbee_core_debug_println("  Seq. Num.: %d (0x%02X)",
                                     keyStruct.sequenceNumber,
                                     keyStruct.sequenceNumber);
      }
      if ( keyStruct.bitmask & SL_ZIGBEE_KEY_HAS_OUTGOING_FRAME_COUNTER ) {
        sl_zigbee_core_debug_println("%s(%s): 0x%4lx",
                                     frameCounterText,
                                     "Out",
                                     (unsigned long)keyStruct.outgoingFrameCounter);
      }
      if ( keyStruct.bitmask & SL_ZIGBEE_KEY_HAS_INCOMING_FRAME_COUNTER ) {
        sl_zigbee_core_debug_println("%s(%s):  0x%4lx",
                                     frameCounterText,
                                     "In",
                                     (unsigned long)keyStruct.incomingFrameCounter);
      }
      sl_zigbee_core_debug_print("  Data: ");
      (void) sli_legacy_serial_wait_send(serialPort);
      printEncryptionKey(serialPort, (uint8_t*)&plaintext_key);
      (void) sli_legacy_serial_wait_send(serialPort);
    }

    printCarriageReturn();
    (void) sli_legacy_serial_wait_send(serialPort);
  }
}

//------------------------------------------------------------------------------
// Send just a Transport Key Message to the broadcast address
// Optional 16-byte key may be provided.

void keyUpdateCommand(sl_cli_command_arg_t *arguments)
{
  sl_zigbee_key_data_t newKey;
  sl_802154_short_addr_t targetShort = sl_cli_get_argument_uint16(arguments, 0);
  sl_802154_long_addr_t targetLong;
  sl_status_t status = SL_STATUS_OK;

  sl_zigbee_core_debug_print("New Key: ");

  if (!sl_zigbee_copy_hex_arg(arguments, 1, sl_zigbee_key_contents(&newKey), SL_ZIGBEE_ENCRYPTION_KEY_SIZE, true)) {
    sl_zigbee_generate_random_key(&newKey);
  }
  printEncryptionKey(serialPort, sl_zigbee_key_contents(&newKey));
  printCarriageReturn();

  if (!isBroadcastAddress(targetShort)) {
    status = sl_zigbee_lookup_eui64_by_node_id(targetShort,
                                               targetLong);
  }

  if (status == SL_STATUS_OK) {
    status = sendKeyUpdateToTarget(targetShort, targetLong, &newKey);
  }

  printCommandStatus(status,
                     (isBroadcastAddress(targetShort)
                      ? "Broadcasting Key Update."
                      : "Unicasting Key Update."),
                     "Failed to send");
  (void) sli_legacy_serial_wait_send(serialPort);

  return;
}

//------------------------------------------------------------------------------
// Send just a Key Switch Message to an address.
// This will also cause our key to change.
// Only a valid call for a TC/Coordinator.

void keySwitchCommand(sl_cli_command_arg_t *arguments)
{
  UNUSED_VAR(arguments);
  sl_status_t status;

  if ( !isSecurityStateValid() ) {
    return;
  }

  status = sl_zigbee_broadcast_network_key_switch();
  printCommandStatus(status,
                     "Sent Switch Key.",
                     "Failed to send");
  (void) sli_legacy_serial_wait_send(serialPort);
}

//------------------------------------------------------------------------------

void requestKeyCommand(sl_cli_command_arg_t *arguments)
{
  sl_802154_long_addr_t partnerAddress;
  sl_status_t status;
  uint8_t* address;
  sl_zigbee_current_security_state_t securityState;
  if (sl_zigbee_copy_eui64_arg(arguments, 0, partnerAddress, false)) {
    address = partnerAddress;
  } else {
    if ( SL_STATUS_OK != sl_zigbee_get_current_security_state(&securityState) ) {
      printErrorMessage("TC Address unknown.");
      return;
    }
    address = securityState.trustCenterLongAddress;
  }

  status = sl_zigbee_request_link_key(address);
  if ( SL_STATUS_OK != status ) {
    printCommandStatus(status, NULL, "Request Link Key failed.");
  }
}

void verifyKeyCommand(sl_cli_command_arg_t *arguments)
{
  sl_802154_short_addr_t shortID = sl_cli_get_argument_uint16(arguments, 0);
  sl_802154_long_addr_t eui;
  sl_status_t status;

  if (sl_zigbee_copy_eui64_arg(arguments, 1, eui, false) == 0) {
    printErrorMessage("partner address unknown.");
    return;
  }

  status = sl_zigbee_verify_partner_link_key(shortID, eui);
  if ( SL_STATUS_OK != status ) {
    printCommandStatus(status, NULL, "Verify Link Key failed.");
  }
}

//------------------------------------------------------------------------------

void setExtendedSecurityBitmaskCommand(sl_cli_command_arg_t *arguments)
{
  uint16_t mask = sl_cli_get_argument_uint16(arguments, 0);

  sl_status_t status = sl_zigbee_set_extended_security_bitmask(mask);

  if (status == SL_STATUS_OK) {
    sl_zigbee_core_debug_print("Set ESB to 0x%04X.\r\n",
                               mask);
  } else {
    sl_zigbee_core_debug_print("Set failed.\r\n");
  }

  (void) sli_legacy_serial_wait_send(serialPort);
}

//------------------------------------------------------------------------------

void getExtendedSecurityBitmaskCommand(sl_cli_command_arg_t *arguments)
{
  UNUSED_VAR(arguments);
  uint16_t mask;

  sl_status_t status = sl_zigbee_get_extended_security_bitmask(&mask);

  if (status == SL_STATUS_OK) {
    sl_zigbee_core_debug_print("Got ESB, 0x%04X.\r\n",
                               mask);
  } else {
    sl_zigbee_core_debug_print("Get failed\r\n");
  }

  (void) sli_legacy_serial_wait_send(serialPort);
}
