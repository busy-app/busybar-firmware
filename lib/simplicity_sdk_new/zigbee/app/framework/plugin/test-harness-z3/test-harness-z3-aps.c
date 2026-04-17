/***************************************************************************//**
 * @file
 * @brief ZigBee 3.0 aps test harness functionality
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
#include "stack/include/zigbee-security-manager.h"

// -----------------------------------------------------------------------------
// Constants

#define APS_REQUEST_KEY_COMMAND (0x08)

#define APS_REQUEST_KEY_TYPE_NETWORK     (0x01)
#define APS_REQUEST_KEY_TYPE_APPLICATION (0x02)
#define APS_REQUEST_KEY_TYPE_TCLK        (0x04)
#define APS_REQUEST_REMOVE_DEVICE        (0x07)

#define APS_COMMAND_OPTION_NWK_ENCRYPTION (0x01)
#define APS_COMMAND_OPTION_APS_ENCRYPTION (0x02)

// -----------------------------------------------------------------------------
// Reset key

// This allows us to send a message out with a wrong key, and then reset to
// use the correct key soon afterward.

// If the APS encrypted transmission fails, then it will be decrypted and re-added
// to the APS queue. Therefore, the next time we encrypt the packet for
// transmission, the key will have been reset to the original. This is an issue.

#define RANDOM_KEY                                    \
  { {                                                 \
      0xAC, 0xAC, 0xAC, 0xAC, 0xAC, 0xAC, 0xAC, 0xAC, \
      0xAC, 0xAC, 0xAC, 0xAC, 0xAC, 0xAC, 0xAC, 0xAC, \
    } }

#define ZTT_INSTALL_CODE_KEY                          \
  { {                                                 \
      0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, \
      0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, \
    } }

static sl_zigbee_key_data_t zigbeeAlliance09 = ZIGBEE_PROFILE_INTEROPERABILITY_LINK_KEY;
static sl_zigbee_key_data_t installCodeKey = ZTT_INSTALL_CODE_KEY;
static sl_zigbee_key_data_t randomKey = RANDOM_KEY;

#define KEY_TYPE_DEFAULT_TCLK (0)
#define KEY_TYPE_RANDOM       (1)
#define KEY_TYPE_INSTALL_CODE (2)
#define KEY_TYPE_MAX          ((sizeof(keys) / sizeof(keys[0])) - 1)

static sl_zigbee_key_data_t *keys[] = {
  &zigbeeAlliance09,
  &randomKey,
  &installCodeKey,
};

sl_zigbee_af_event_t sl_zigbee_af_test_harness_z3_reset_key_event;
#define z3ResetKeyEventControl (&sl_zigbee_af_test_harness_z3_reset_key_event)
void sl_zigbee_af_test_harness_z3_reset_key_event_handler(sl_zigbee_af_event_t * event);

static uint8_t mostRecentKeyIndex = 0xFF;

void sl_zigbee_af_test_harness_z3_reset_key_event_handler(sl_zigbee_af_event_t * event)
{
  UNUSED_VAR(event);

  sl_status_t status;

  sl_zigbee_af_event_set_inactive(z3ResetKeyEventControl);

  // Remove the most recently added key.
  if (mostRecentKeyIndex != 0xFF) {
    status = sl_zigbee_erase_key_table_entry(mostRecentKeyIndex);
    sl_zigbee_af_core_println("%s: %s: 0x%02X",
                              TEST_HARNESS_Z3_PRINT_NAME,
                              "Reset key handler",
                              status);
    mostRecentKeyIndex = 0xFF;
  }
}

static sl_status_t setNextKey(sl_802154_long_addr_t partner, bool linkKey, uint8_t keyType)
{
  UNUSED_VAR(linkKey);

  sl_status_t status;
  sl_zigbee_sec_man_context_t context;
  sl_zigbee_sec_man_init_context(&context);
  context.key_index = 0xFF;
  memmove(context.eui64, partner, sizeof(sl_802154_long_addr_t));
  context.core_key_type = SL_ZB_SEC_MAN_KEY_TYPE_APP_LINK;
  status = sl_zigbee_sec_man_import_key(&context,
                                        (keyType > KEY_TYPE_MAX
                                         ? (sl_zigbee_sec_man_key_t*) &(keys[KEY_TYPE_MAX])
                                         : (sl_zigbee_sec_man_key_t*) &(keys[keyType])));

  // temporary conversion until everything is ported to sl_status_t
  sl_status_t status_ember = ((status == SL_STATUS_OK) ? SL_STATUS_OK
                              : ((status == SL_STATUS_FULL) ? SL_STATUS_FULL : SL_STATUS_INVALID_PARAMETER));

  sl_zigbee_af_core_println("setNextKey: status = %02X", status_ember);

  mostRecentKeyIndex = context.key_index;
  sl_zigbee_af_event_set_active(z3ResetKeyEventControl);

  return status_ember;
}

// -----------------------------------------------------------------------------
// APS CLI Commands

#ifdef EZSP_HOST
  #define sl_zigbee_send_remove_device(...) SL_STATUS_INVALID_STATE
  #define sli_zigbee_send_aps_command(...)      SL_STATUS_INVALID_STATE
#else

// Internal stack API.
extern bool sli_zigbee_send_aps_command(sl_802154_short_addr_t destination,
                                        sl_802154_long_addr_t longDestination,
                                        sli_buffer_manager_buffer_t payload,
                                        uint8_t options);

#endif /* EZSP_HOST */

// plugin test-harness z3 aps aps-remove-device <parentLong:8> <dstLong:8> <options:4>
void sli_zigbee_af_test_harness_z3_aps_aps_remove_device(SL_CLI_COMMAND_ARG)
{
  sl_802154_long_addr_t parentLong, targetLong;
  sl_802154_short_addr_t parentShort;
  sl_status_t status = SL_STATUS_OK;
  uint32_t options = sli_zigbee_af_test_harness_z3_get_significant_bit(arguments, 2);

  sl_zigbee_copy_eui64_arg(arguments, 0, parentLong, true);
  sl_zigbee_copy_eui64_arg(arguments, 1, targetLong, true);

  switch (options) {
    case BIT32(0):
      status = setNextKey(targetLong, true, KEY_TYPE_DEFAULT_TCLK);
      break;

    case BIT32(1):
      // This is our own hook so we can test the next key functionality.
      status = setNextKey(targetLong, true, KEY_TYPE_RANDOM);
      break;

    default:
      ; // meh
  }

  if (status == SL_STATUS_OK) {
    status = sl_zigbee_lookup_node_id_by_eui64(parentLong, &parentShort);
    if (status != SL_STATUS_OK ) {
      status = SL_STATUS_INVALID_PARAMETER;
    } else {
      status = sl_zigbee_send_remove_device(parentShort, parentLong, targetLong);
    }
  }

  sl_zigbee_af_core_println("%s: %s: 0x%02X",
                            TEST_HARNESS_Z3_PRINT_NAME,
                            "Remove device",
                            status);
}

// plugin test-harness z3 aps aps-request-key <dstShort:2> <keyType:1>
// <parentLong:8> <options:4>
void sli_zigbee_af_test_harness_z3_aps_aps_request_key_command(SL_CLI_COMMAND_ARG)
{
  sl_status_t status = SL_STATUS_INVALID_STATE;
#ifndef EZSP_HOST

  sl_802154_short_addr_t destShort = sl_cli_get_argument_uint16(arguments, 0);
  uint8_t keyType       = sl_cli_get_argument_uint8(arguments, 1);
  uint32_t options      = sli_zigbee_af_test_harness_z3_get_significant_bit(arguments, 3);

  sl_802154_long_addr_t partnerLong, trustCenterEui64;
  uint8_t frame[10];
  uint8_t *finger = &frame[0];
  sli_buffer_manager_buffer_t commandBuffer;
  uint8_t apsCommandOptions;

  sl_zigbee_copy_eui64_arg(arguments, 2, partnerLong, true);

  status = sl_zigbee_lookup_eui64_by_node_id(SL_ZIGBEE_TRUST_CENTER_NODE_ID, trustCenterEui64);
  if (status != SL_STATUS_OK) {
    goto done;
  }

  // Set the default command options as APS and NWK encryption.
  apsCommandOptions = (APS_COMMAND_OPTION_NWK_ENCRYPTION
                       | APS_COMMAND_OPTION_APS_ENCRYPTION);

  // What key should we be using?
  switch (options) {
    case 0:
    case BIT32(0):
      // Handled elsewhere.
      break;
    case BIT32(1):
      // KeyLoadKey
      // TODO: key id in security control.
      status = setNextKey(trustCenterEui64, false, KEY_TYPE_DEFAULT_TCLK);
      break;
    case BIT32(2):
      // TODO: KeyTransport key id in security control.
      status = setNextKey(trustCenterEui64, false, KEY_TYPE_DEFAULT_TCLK);
      break;
    case BIT32(3):
      // TODO: DataKey key id in security control.
      status = setNextKey(trustCenterEui64, false, KEY_TYPE_DEFAULT_TCLK);
      break;
    case BIT32(4):
      // TODO: DataKey key id in security control.
      status = setNextKey(trustCenterEui64, false, KEY_TYPE_RANDOM);
      break;
    case BIT32(5):
      // APS encrypted with install code derived key.
      status = setNextKey(trustCenterEui64, false, KEY_TYPE_INSTALL_CODE);
      break;
    case BIT32(6):
      // No APS or NWK encryption.
      apsCommandOptions &= ~(APS_COMMAND_OPTION_NWK_ENCRYPTION
                             | APS_COMMAND_OPTION_APS_ENCRYPTION);
      break;
    case BIT32(7):
      // KeyType is 0x01 (network key).
      keyType = APS_REQUEST_KEY_TYPE_NETWORK;
      break;
    case BIT32(8):
    case BIT32(9):
      // KeyType is 0x03 (application link key).
      // FIXME: email sent to bozena regarding this key type disagreement.
      keyType = APS_REQUEST_KEY_TYPE_APPLICATION;
      break;
    case BIT32(10):
      // TODO: do not issue verify key.
      break;
    case BIT32(11):
      // KeyType is 0x04.
      // TODO: KeyTransportKey key id in security control.
      keyType = APS_REQUEST_KEY_TYPE_TCLK;
      status = setNextKey(trustCenterEui64, false, KEY_TYPE_DEFAULT_TCLK);
      break;
    case BIT32(12):
      // TODO: incorrect APS VerifyKey hash.
      break;
    default:
      status = SL_STATUS_INVALID_PARAMETER;
  }

  if (status != SL_STATUS_OK) {
    goto done;
  }

  *finger++ = APS_REQUEST_KEY_COMMAND;
  *finger++ = keyType;
  if (keyType == APS_REQUEST_KEY_TYPE_APPLICATION && !(options & BIT32(0))) {
    memmove(finger, partnerLong, EUI64_SIZE);
    finger += EUI64_SIZE;
  }

  commandBuffer = sl_legacy_buffer_manager_fill_linked_buffers(frame, finger - &frame[0]);
  if (commandBuffer == SL_ZIGBEE_NULL_MESSAGE_BUFFER) {
    status = SL_STATUS_ALLOCATION_FAILED;
    goto done;
  }

  status = (sli_zigbee_send_aps_command(destShort,
                                        trustCenterEui64,
                                        commandBuffer,
                                        apsCommandOptions)
            ? SL_STATUS_OK
            : SL_STATUS_FAIL);
  sl_legacy_buffer_manager_release_message_buffer(commandBuffer);

  done:
#endif /* EZSP_HOST */

  sl_zigbee_af_core_println("%s: %s: 0x%02X",
                            TEST_HARNESS_Z3_PRINT_NAME,
                            "Request key",
                            status);
}
