/***************************************************************************//**
 * @file
 * @brief Routines to initiate key establishment, handle key establishment callbacks,
 * and print info about the security keys on the device.
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
#include "app/framework/util/af-main.h"
#include "app/util/serial/sl_zigbee_command_interpreter.h"
#include "stack/include/zigbee-security-manager.h"
#include "stack/include/trust-center.h"
#include "stack/include/network-formation.h"

sl_zigbee_key_data_t cliPreconfiguredLinkKey = DUMMY_KEY;
sl_zigbee_key_data_t cliNetworkKey           = DUMMY_KEY;

static uint32_t getOutgoingApsFrameCounter(void)
{
  sl_zigbee_sec_man_context_t context;
  sl_zigbee_sec_man_aps_key_metadata_t key_data;
  sl_zigbee_sec_man_init_context(&context);

  context.core_key_type = SL_ZB_SEC_MAN_KEY_TYPE_TC_LINK;

  if (sl_zigbee_sec_man_get_aps_key_info(&context, &key_data) != SL_STATUS_OK) {
    return 0xFFFFFFFFUL;
  }
  return key_data.outgoing_frame_counter;
}

uint8_t printTransientKeyTable(void)
{
  sl_status_t status;
  sl_zigbee_sec_man_context_t context;
  sl_zigbee_sec_man_key_t plaintext_key;
  sl_zigbee_sec_man_aps_key_metadata_t key_info;
  uint8_t index = 0;

  sl_zigbee_core_debug_println("Index IEEE Address         NWKIndex  In FC     TTL(s) Flag    Key    ");

  status = sl_zigbee_sec_man_export_transient_key_by_index(index,
                                                           &context,
                                                           &plaintext_key,
                                                           &key_info);

  while (status == SL_STATUS_OK) {
    sl_zigbee_core_debug_print("%d     ", index);
    sl_zigbee_af_core_debug_exec(sl_zigbee_af_print_big_endian_eui64(context.eui64));
    sl_zigbee_core_debug_print("  %d       ", context.multi_network_index);
    sl_zigbee_core_debug_print("  %08X  ", key_info.incoming_frame_counter);
    sl_zigbee_core_debug_print("0x%04X", key_info.ttl_in_seconds);
    sl_zigbee_core_debug_print(" 0x%04X  ", key_info.bitmask);
    sl_zigbee_af_print_zigbee_key((const uint8_t*)&(plaintext_key.key));
    sl_zigbee_core_debug_println("");
    sl_zigbee_af_core_flush();

    index += 1;
    status = sl_zigbee_sec_man_export_transient_key_by_index(index,
                                                             &context,
                                                             &plaintext_key,
                                                             &key_info);
  }

  return index;
}

uint8_t printKeyTable(bool preconfiguredKey)
{
  uint8_t i;
  uint8_t entriesUsed = 0;
  uint8_t loopCount = (preconfiguredKey ? 1 : sl_zigbee_af_get_key_table_size());

  sl_zigbee_sec_man_context_t context;
  sl_zigbee_sec_man_aps_key_metadata_t key_data;
  sl_zigbee_sec_man_init_context(&context);

  sl_zigbee_core_debug_println("Index IEEE Address         In FC     Type  Auth  Key");

  for (i = 0; i < loopCount; i++) {
    sl_zigbee_sec_man_context_t context;
    sl_zigbee_sec_man_init_context(&context);
    sl_zigbee_sec_man_key_t plaintext_key;
    sl_zigbee_sec_man_aps_key_metadata_t metadata;

    if (preconfiguredKey) {
      i = 0xFE; // last
      context.core_key_type = SL_ZB_SEC_MAN_KEY_TYPE_TC_LINK;

      // Try to get whatever key type is stored in the pre-configured key slot.
      context.core_key_type = SL_ZB_SEC_MAN_KEY_TYPE_TC_LINK;
      if ( SL_STATUS_OK
           != sl_zigbee_sec_man_export_key(&context, &plaintext_key) ) {
        continue;
      }
    } else if ( SL_STATUS_OK != sl_zigbee_sec_man_export_link_key_by_index(i, &context, &plaintext_key, &metadata) ) {
      continue;
    } else {
      // MISRA requires ..else if.. to have a terminating else.
    }

    (void)sl_zigbee_sec_man_get_aps_key_info(&context, &key_data);
    context.flags |= ZB_SEC_MAN_FLAG_KEY_INDEX_IS_VALID;

    if (!preconfiguredKey) {
      sl_zigbee_core_debug_print("%d     ", i);
    } else {
      sl_zigbee_core_debug_print("-     ");
    }
    context.flags |= ZB_SEC_MAN_FLAG_KEY_INDEX_IS_VALID;
    (void) sl_zigbee_sec_man_get_aps_key_info(&context, &metadata);
    sl_zigbee_af_core_debug_exec(sl_zigbee_af_print_big_endian_eui64(context.eui64));
    sl_zigbee_core_debug_print("  %08X  ", metadata.incoming_frame_counter);
    sl_zigbee_core_debug_print("%c     %c     ",
                               (metadata.bitmask & SL_ZIGBEE_KEY_IS_AUTHENTICATION_TOKEN
                                ? 'A'
                                : 'L'),
                               (metadata.bitmask & SL_ZIGBEE_KEY_IS_AUTHORIZED
                                ? 'y'
                                : 'n'));

    sl_zigbee_af_print_zigbee_key((const uint8_t *)&(plaintext_key.key));

    sl_zigbee_af_core_flush();
    entriesUsed++;
  }

  return entriesUsed;
}

void printKeyInfo(void)
{
  uint8_t entriesUsed;

  sl_zigbee_sec_man_context_t context;
  sl_zigbee_sec_man_key_t plaintext_key;
  sl_zigbee_sec_man_network_key_info_t network_key_info;

  sl_zigbee_sec_man_init_context(&context);
  context.core_key_type = SL_ZB_SEC_MAN_KEY_TYPE_NETWORK;

#ifndef SL_ZIGBEE_AF_HAS_SECURITY_PROFILE_NONE
  {
    sl_zigbee_mfg_security_struct_t config;
    if (SL_STATUS_OK == sl_zigbee_get_mfg_security_config(&config)) {
      if (!(config.keySettings & SL_ZIGBEE_KEY_PERMISSIONS_READING_ALLOWED)) {
        sl_zigbee_core_debug_println("");
        sl_zigbee_core_debug_print("WARNING:  READING OF KEY VALUES DISABLED BY MFG TOKEN");
        sl_zigbee_core_debug_println("");
        sl_zigbee_core_debug_println("");
        sl_zigbee_af_core_flush();
      }
    }
  }
#endif

  if ( SL_STATUS_OK != sl_zigbee_sec_man_export_key(&context, &plaintext_key)) {
    memset(&plaintext_key.key, 0xFF, sizeof(sl_zigbee_sec_man_key_t));
  }

  (void)sl_zigbee_sec_man_get_network_key_info(&network_key_info);
  sl_zigbee_core_debug_println("%s out FC: %08X",
                               "NWK Key",
                               network_key_info.network_key_frame_counter);
  sl_zigbee_core_debug_println("%s seq num: 0x%02X",
                               "NWK Key",
                               network_key_info.network_key_sequence_number);
  sl_zigbee_core_debug_print("%s: ", "NWK Key");
  sl_zigbee_af_print_zigbee_key(plaintext_key.key);

  sl_zigbee_core_debug_println("%s out FC: %08x",
                               "Link Key",
                               getOutgoingApsFrameCounter());

  sl_zigbee_core_debug_println("TC %s ", "Link Key");
  sl_zigbee_af_core_flush();
  printKeyTable(true);

  sl_zigbee_core_debug_println("%s Table", "Link Key");
  sl_zigbee_af_core_flush();
  entriesUsed = printKeyTable(false);
  UNUSED_VAR(entriesUsed);

  sl_zigbee_core_debug_println("%d/%d entries used.",
                               entriesUsed,
                               sl_zigbee_af_get_key_table_size());
  sl_zigbee_af_core_flush();

  sl_zigbee_core_debug_println("%s Table", "Transient Key");
  sl_zigbee_af_core_flush();

  entriesUsed = printTransientKeyTable();
  UNUSED_VAR(entriesUsed);

  sl_zigbee_core_debug_println("%d entr%s consuming %d packet buffer%s.",
                               entriesUsed,
                               entriesUsed > 1 ? "ies" : "y",
                               entriesUsed,
                               entriesUsed > 1 ? "s" : "");
  sl_zigbee_af_core_flush();
}

#if !(defined(SL_ZIGBEE_LEAF_STACK))
void optionSecurityAllowTrustCenterRejoinUsingWellKnownKey(sl_cli_command_arg_t *arguments)
{
  bool allow = (bool)sl_cli_get_argument_uint8(arguments, 0);
  sl_zigbee_set_tc_rejoins_using_well_known_key_allowed(allow);
  sl_zigbee_af_core_println("Trust Center %s allow trust center rejoins using "
                            "well-known key.",
                            allow
                            ? "WILL"
                            : "WILL NOT");
}

void optionSecurityAllowTrustCenterRejoinUsingWellKnownKeyTimeout(sl_cli_command_arg_t *arguments)
{
  uint16_t timeout = sl_cli_get_argument_uint16(arguments, 0);
  sl_zigbee_set_tc_rejoins_using_well_known_key_timeout_sec(timeout);
  sl_zigbee_af_core_println("Set timeout for Trust Center rejoins using well-known "
                            "link key to %d seconds.",
                            timeout);
}
#endif

#if (!defined(SL_ZIGBEE_LEAF_STACK) && !defined(SL_ZIGBEE_ROUTER_STACK))
void optionSecuritySetKeyRequestPolicy(sl_cli_command_arg_t *arguments)
{
  sl_zigbee_set_trust_center_link_key_request_policy(sl_cli_get_argument_uint8(arguments, 0));
  sl_zigbee_set_app_link_key_request_policy(sl_cli_get_argument_uint8(arguments, 1));
}
#endif

void changeKeyCommand(sl_cli_command_arg_t *arguments)
{
  if (SL_ZIGBEE_NO_NETWORK != sl_zigbee_af_network_state()) {
    sl_zigbee_af_core_println("%sstack must be down.", "ERROR: ");
  } else {
    uint8_t position = sl_cli_get_command_count(arguments) - 1;

    uint8_t command_first_character = sl_cli_get_command_string(arguments, position)[0];
    sl_zigbee_copy_hex_arg((arguments), 0, sl_zigbee_key_contents((command_first_character == 'l'
                                                                   ? &cliPreconfiguredLinkKey
                                                                   : &cliNetworkKey)), SL_ZIGBEE_ENCRYPTION_KEY_SIZE, true);
    sl_zigbee_af_debug_println("set key");
  }
}

void getSetMfgToken(sl_cli_command_arg_t *arguments)
{
#ifndef SL_ZIGBEE_AF_HAS_SECURITY_PROFILE_NONE
  sl_zigbee_mfg_security_struct_t config;
  sl_status_t status;

  if (sl_cli_get_command_string(arguments, 2)[0] == 'g') {
    status = sl_zigbee_get_mfg_security_config(&config);
    if (status == SL_STATUS_OK) {
      sl_zigbee_af_security_println("sl_zigbee_key_settings_t: 0x%04X", config.keySettings);
      sl_zigbee_af_security_println("  Permissions: %s",
                                    ((config.keySettings & SL_ZIGBEE_KEY_PERMISSIONS_READING_ALLOWED)
                                     ? "Reading Allowed"
                                     : ((config.keySettings & SL_ZIGBEE_KEY_PERMISSIONS_HASHING_ALLOWED)
                                        ? "Hashing only"
                                        : "NONE")));
    } else {
      sl_zigbee_af_security_println("Error: Failed to get config, status: 0x%08X", status);
    }
  } else {
    uint32_t magicNumber = sl_cli_get_argument_uint32(arguments, 0);
    config.keySettings = (sl_zigbee_key_settings_t)sl_cli_get_argument_uint32(arguments, 1);
    status = sl_zigbee_set_mfg_security_config(magicNumber, &config);
  }

  if (status != SL_STATUS_OK) {
    sl_zigbee_af_security_println("Failed: 0x%08X", status);
  }
#else
  UNUSED_VAR(arguments);
  sl_zigbee_af_security_println("Cannot get/set mfg tokens with no security profile");
#endif // SL_ZIGBEE_AF_HAS_SECURITY_PROFILE_NONE
}
