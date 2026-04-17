/***************************************************************************//**
 * @file sl_zigbee_r23_zcp_test_support.c
 * @brief additional cli commands for operating R23 compliance tests
 *******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
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

#include "sl_component_catalog.h"
#include "app/util/serial/sl_zigbee_command_interpreter.h"
#include "sl_zigbee_types.h"
#include "stack/include/security.h"
#include "stack/include/zigbee-device-stack.h"
#include "aps-keys.h"
#include "stack/include/sl_zigbee_zdo_dlk_negotiation.h"
#include "stack/include/sl_zigbee_zdo_security.h"
#include "stack/include/sl_zigbee_zdo_management.h"
#include "stack/include/sl_zigbee_stack_specific_tlv.h"
#include "stack/zigbee/sli_zigbee_zdo_security.h"
#include "stack/security/aps-frame-counter-sync.h"
#include "stack/core/sl_zigbee_stack.h"
// NOTE for R23_COMPLIANCE_REVISION
#include "stack/zigbee/zigbee-device.h"
// NOTE for SL_ZIGBEE_INIT_LEVEL_DONE
#include "sl_zigbee_system_common.h"
#include "zigbee-security-manager.h"

#include "stack/core/sl_zigbee_stack.h"
#include "stack/zigbee/aps-security.h"
#include "stack/routing/zigbee/child.h"
#include "stack/include/sl_zigbee_dynamic_commissioning.h"
#include "stack/framework/sli_zigbee_dynamic_commissioning.h"

#include "stack/routing/zigbee/management.h"
#include "stack/routing/zigbee/sl_zigbee_beacon_tlvs.h"
#include "stack/core/sli_zigbee_tlv_core.h"

#include "stack/include/sl_zigbee_zdo_configuration.h"

#include "stack/include/pro_compliance_stack_interface.h"

static sl_zigbee_dlk_supported_negotiation_method gu_dlk_override_supported_methods = DLK_PROTOCOL_MASK_RESERVED;
static sl_zigbee_dlk_negotiation_supported_shared_secret_source gu_dlk_override_supported_secrets = DLK_SECRET_MASK_RESERVED;
static sl_zigbee_initial_join_method gu_override_initial_join_method = INITIAL_JOIN_METHOD_TYPE_ANONYMOUS;  // default values from prototype
static sl_zigbee_active_link_key_type gu_override_active_link_key_type = ACITVE_LINK_KEY_TYPE_ANONYMOUS_KEY_NEGOTIATION;  // default values from prototype
static sl_zigbee_af_event_t DeviceInterviewEvent;
static void zcp_device_interview_keepalive(sl_zigbee_af_event_t *p_event);

void slxi_zigbee_stack_gu_zdo_dlk_override_supported_params(uint8_t *method_mask,
                                                            uint8_t *secret_mask)
{
  if (gu_dlk_override_supported_methods != DLK_PROTOCOL_MASK_RESERVED) {
    *method_mask = gu_dlk_override_supported_methods;
  }

  *secret_mask = gu_dlk_override_supported_secrets;
}

void initiate_tclk_update(sl_cli_command_arg_t *arguments)
{
  uint16_t retry_attempts = sl_zigbee_bdb_tclk_max_exchange_attempts();   // default
  if (sl_cli_get_argument_count(arguments) >= 1) {
    retry_attempts = sl_cli_get_argument_uint16(arguments, 0);
  }
  (void) sl_zigbee_update_tc_link_key(retry_attempts);
}

#define MAX_NUMBER_ZDO_CONFIG_INPUT 40 // this is just a random max number of arguments

void sl_zigbee_zdo_get_config_response_handler(uint8_t response_length,
                                               uint8_t *response,
                                               uint16_t payload_index,
                                               sl_802154_short_addr_t source)
{
  UNUSED_VAR(source);
  uint8_t *payload = &(response[payload_index]);
  sl_zigbee_zdo_status_t response_status = payload[0];
  sl_zigbee_core_debug_println("ZDO Get Config Response %02x", response_status);
  if (response_status != SL_ZIGBEE_ZDP_SUCCESS) {
    return;
  }
  payload++;
  sl_zigbee_tlv_chain config_tlvs;
  sl_status_t status = sl_zigbee_tlv_initialize_full_chain(&config_tlvs,
                                                           payload,
                                                           response_length - payload_index - 1);
  if (status != SL_STATUS_OK) {
    sl_zigbee_core_debug_println("bad tlvs...");
  }
  sl_zigbee_tlv_t *tlv;
  while (sl_zigbee_tlv_chain_next_tlv(&config_tlvs, &tlv) == SL_STATUS_OK) {
    uint8_t tag = sl_zigbee_tlv_get_tag(tlv);
    uint8_t length = sl_zigbee_tlv_get_length(tlv);
    uint8_t *value = sl_zigbee_tlv_get_value_ptr(tlv);
    switch (tag) {
      case SL_ZIGBEE_GLOBAL_TLV_PAN_ID_CONFLICT_TAG_ID:
        sl_zigbee_core_debug_println("PAN ID Conflicts - %d", sl_util_fetch_low_high_int16u(value));
        break;
      default:
        sl_zigbee_core_debug_println("got tag id %d (%d bytes)", tag, length);
        sl_zigbee_core_debug_print_buffer(value, length, true);
        sl_zigbee_core_debug_println("");
    }
  }
}

void sl_zigbee_zdo_set_config_response_handler(sli_buffer_manager_buffer_t response,
                                               sl_zigbee_zdo_status_t zdoStatus,
                                               uint16_t payload_index,
                                               sl_802154_short_addr_t source)
{
  UNUSED_VAR(response);
  UNUSED_VAR(zdoStatus);
  UNUSED_VAR(payload_index);
  UNUSED_VAR(source);
}

void sl_zigbee_af_zdo_set_configuration_req_callback(uint8_t* message_ptr, uint8_t message_length)
{
  UNUSED_VAR(message_ptr);
  UNUSED_VAR(message_length);
}

void zdo_get_configuration_handler(sl_cli_command_arg_t *arguments)
{
  uint8_t tag_ids[MAX_NUMBER_ZDO_CONFIG_INPUT];
  uint8_t i = 0;
  uint8_t count = sl_cli_get_argument_count(arguments);
  sl_802154_short_addr_t device_short = sl_cli_get_argument_uint16(arguments, 0);
  bool encrypt = (bool)sl_cli_get_argument_uint8(arguments, 1);
  tag_ids[i++] = sl_cli_get_argument_uint8(arguments, 2);
  if (sl_cli_get_argument_count(arguments) > 3) {
    while (i < (count - 2) && i < MAX_NUMBER_ZDO_CONFIG_INPUT ) {
      tag_ids[i] = sl_cli_get_argument_uint8(arguments, i + 2);
      i++;
    }
  }
  sl_status_t status = sl_zigbee_zdo_get_configuration_req(device_short, encrypt, tag_ids, count - 2);
  sl_zigbee_core_debug_println("Get ZDO Config 0x%02X for %02X", status, device_short);
}

void zdo_add_configuration_handler(sl_cli_command_arg_t *arguments)
{
  uint8_t tag_id;
  size_t zdo_config_arg_len;
  tag_id = sl_cli_get_argument_uint8(arguments, 0);
  uint8_t *zdo_config_arg = sl_cli_get_argument_hex(arguments, 1, &zdo_config_arg_len);
  sl_status_t status = sl_zigbee_zdo_set_add_configuration(tag_id, zdo_config_arg_len, zdo_config_arg);
  sl_zigbee_core_debug_println("ZDO Add Config 0x%02X", status);
}

void zdo_set_send_configuration_handler(sl_cli_command_arg_t *arguments)
{
  sl_802154_short_addr_t device_short;
  device_short = sl_cli_get_argument_uint16(arguments, 0);
  bool encrypt = (bool)sl_cli_get_argument_uint8(arguments, 1);

  sl_status_t status = sl_zigbee_zdo_set_send_configuration_req(device_short, encrypt);
  sl_zigbee_core_debug_println("Send Set ZDO Config Request: 0x%02X", status);
}

void start_key_negotiation(sl_cli_command_arg_t *arguments)
{
  sl_zigbee_address_info partner;
  sl_zigbee_dlk_negotiation_method selected_method = DLK_PROTOCOL_ENUM_STATIC_KEY;
  sl_zigbee_dlk_negotiation_shared_secret_source selected_secret = DLK_SECRET_ENUM_WELL_KNOWN_KEY;
  if (sl_cli_get_argument_count(arguments) >= 4) {
    partner.device_short = sl_cli_get_argument_uint16(arguments, 0);
    sl_zigbee_copy_eui64_arg(arguments, 1, partner.device_long, true);
    selected_method = sl_cli_get_argument_uint8(arguments, 2);
    selected_secret = sl_cli_get_argument_uint8(arguments, 3);
  }
  sl_status_t status = sl_zigbee_zdo_dlk_start_key_negotiation(&partner, selected_method, selected_secret);
  sl_zigbee_core_debug_println("ZDO Start Key Negotiation Request: 0x%02X", status);
}

void sl_zigbee_zdo_retrieve_authentication_token_complete_callback(sl_status_t status)
{
  sl_zigbee_core_debug_println("ZDO Retrieve Auth Token Response: 0x%02X", status);
}

void retrieve_auth_token_command(sl_cli_command_arg_t *arguments)
{
  sl_zigbee_aps_option_t options;
  sl_802154_short_addr_t device_short = sl_cli_get_argument_uint16(arguments, 0);
  options = SL_ZIGBEE_APS_OPTION_ENCRYPTION;
  sl_zigbee_retrieve_authentication_token(device_short, options);
}

void sl_zigbee_beacon_survey_complete_callback(sl_zigbee_zdo_status_t status,
                                               sl_zigbee_beacon_survey_results_t *survey_results,
                                               uint8_t potential_parent_count,
                                               sl_zigbee_potential_parent_t *potential_parents,
                                               uint16_t pan_id_conflicts)
{
  sl_zigbee_core_debug_print("ZDO Beacon Survey: 0x%02x ", status);
  if (status == SL_STATUS_OK) {
    sl_zigbee_core_debug_print("tot: %u ", survey_results->total_beacons);
    sl_zigbee_core_debug_print("on-net: %u ", survey_results->on_network_beacons);
    sl_zigbee_core_debug_print("pt p: %u ", survey_results->potential_parent_beacons);
    sl_zigbee_core_debug_print("othr: %u ", survey_results->other_network_beacons);
    for (int i = 0; i < potential_parent_count; i++) {
      sl_zigbee_core_debug_print("pt p [%d] id: 0x%04x  ", i, potential_parents[i].short_id);
      sl_zigbee_core_debug_print("pt p[%d] lqi: %u ", i, potential_parents[i].lqa);
    }
    sl_zigbee_core_debug_print("id conf: %u", pan_id_conflicts);
  }
  sl_zigbee_core_debug_println("");
}

void sl_zigbee_get_authentication_level_callback(sl_zigbee_zdo_status_t rsp_status, sl_802154_long_addr_t target, uint8_t join_method, uint8_t link_key_update)
{
  sl_zigbee_core_debug_print("ZDO Get Auth Level Rsp: 0x%02x ", rsp_status);
  if (rsp_status == SL_ZIGBEE_ZDP_SUCCESS) {
    sl_zigbee_core_debug_print("Target EUI {");
    sl_zigbee_core_debug_print_buffer(target, EUI64_SIZE, false);
    sl_zigbee_core_debug_print("} join method: %u link key update: %u", join_method, link_key_update);
  }
  sl_zigbee_core_debug_println("");
}

#define CHANNEL_PAGE_COUNT 1
void beacon_survey_command(sl_cli_command_arg_t *arguments)
{
  uint8_t scan_config_mask;
  sl_802154_short_addr_t device_short;
  uint32_t masks[CHANNEL_PAGE_COUNT];  // NOTE only works with one channel mask
  uint32_t *channel_masks = (uint32_t*)masks;
  device_short = sl_cli_get_argument_uint16(arguments, 0);
  channel_masks[0] = sl_cli_get_argument_uint32(arguments, 1);
  scan_config_mask = sl_cli_get_argument_uint8(arguments, 2);
  sl_status_t status = sl_zigbee_request_beacon_survey(device_short,
                                                       CHANNEL_PAGE_COUNT,
                                                       channel_masks,
                                                       scan_config_mask);
  sl_zigbee_core_debug_println("ZDO Generate Beacon Survey Request: 0x%02X", status);
}

void sl_zigbee_set_authenticaion_level_callback(sl_802154_long_addr_t target,
                                                sl_zigbee_initial_join_method *initial_join_method,
                                                sl_zigbee_active_link_key_type *active_link_key_type)
{
  if (target != NULL) {
    *initial_join_method = gu_override_initial_join_method;
    *active_link_key_type = gu_override_active_link_key_type;
  }
}

void set_override_auth_level_methods(sl_cli_command_arg_t *arguments)
{
  uint8_t join = sl_cli_get_argument_uint8(arguments, 0);
  uint8_t update = sl_cli_get_argument_uint8(arguments, 1);
  gu_override_initial_join_method = join;
  gu_override_active_link_key_type = update;
  sl_zigbee_core_debug_println("auth lvl set: join %x - update %x", join, update);
  sl_zigbee_core_debug_println("override selection set");
}

void get_auth_level_command(sl_cli_command_arg_t *arguments)
{
  sl_802154_long_addr_t target_eui64;
  sl_zigbee_aps_option_t option = SL_ZIGBEE_APS_OPTION_NONE;
  sl_802154_short_addr_t device_short = sl_cli_get_argument_uint16(arguments, 0);
  sl_zigbee_copy_eui64_arg(arguments, 1, target_eui64, false);
  if (sl_cli_get_argument_count(arguments) >= 3) {
    option = sl_cli_get_argument_uint16(arguments, 2);
  } else {
    option = SL_ZIGBEE_APS_OPTION_ENCRYPTION;
  }
  sl_status_t status = sl_zigbee_test_zdo_generate_get_authentication_level_req(device_short, option, target_eui64);
  sl_zigbee_core_debug_println("ZDO Get Auth Level Req: 0x%02X", status);
}

sl_status_t sli_zigbee_stack_test_zdo_generate_get_authentication_level_req(sl_802154_short_addr_t dest,
                                                                            sl_zigbee_aps_option_t aps_options,
                                                                            sl_802154_long_addr_t target)
{
  return sli_zigbee_zdo_generate_get_authentication_level_req(dest, aps_options, target);
}

#define DEFAULT_DLK_NEGOTIATION_METHOD DLK_PROTOCOL_ENUM_SPEKE_C25519_AES128
#define DEFAULT_DLK_NEGOTIATION_SECRET DLK_SECRET_ENUM_WELL_KNOWN_KEY
static sl_zigbee_dlk_negotiation_method gu_override_selected_method = DLK_PROTOCOL_ENUM_RESERVED;
static sl_zigbee_dlk_negotiation_shared_secret_source gu_override_selected_secret = DLK_SECRET_ENUM_RESERVED;

sl_status_t sl_zigbee_zdo_dlk_select_negotiation_parameters_callback(sl_zigbee_address_info *partner,
                                                                     sl_zigbee_dlk_supported_negotiation_method their_supported_methods,
                                                                     sl_zigbee_dlk_negotiation_supported_shared_secret_source their_supported_secrets,
                                                                     sl_zigbee_dlk_negotiation_method *selected_method,
                                                                     sl_zigbee_dlk_negotiation_shared_secret_source *selected_secret)
{
  // check the overlapping supported methods
  sl_zigbee_dlk_supported_negotiation_method our_methods;
  sl_zigbee_dlk_negotiation_supported_shared_secret_source our_secrets;
  sl_zigbee_zdo_dlk_get_supported_negotiation_parameters(&our_methods, &our_secrets);
  sl_zigbee_dlk_supported_negotiation_method overlap_method = our_methods & their_supported_methods;
  sl_zigbee_dlk_negotiation_supported_shared_secret_source overlap_secret = our_secrets & their_supported_secrets;
  sl_zigbee_core_debug_println("compare dlk support (partner: %02x) (mutual methods: %0x, secrets: %0x)",
                               partner->device_short,
                               overlap_method,
                               overlap_secret);
  // select method based on overlap or the overrides
  if (gu_override_selected_method == DLK_PROTOCOL_ENUM_RESERVED) {
    if (overlap_method == 0) {
      *selected_method = DLK_PROTOCOL_ENUM_RESERVED;   // No common method
    } else {
      if (overlap_method & DLK_PROTOCOL_MASK_SPEKE_C25519_SHA256) {
        *selected_method = DLK_PROTOCOL_ENUM_SPEKE_C25519_SHA256;
      } else if (overlap_method & DLK_PROTOCOL_MASK_SPEKE_C25519_AES128) {
        *selected_method = DLK_PROTOCOL_ENUM_SPEKE_C25519_AES128;
      } else if (overlap_method & DLK_PROTOCOL_MASK_STATIC_KEY_REQUEST) {
        *selected_method = DLK_PROTOCOL_ENUM_STATIC_KEY;
      }
    }
  } else {
    *selected_method = gu_override_selected_method;   // Use override if set
  }

  // select secret based on overlap or the overrides
  if (gu_override_selected_secret == DLK_SECRET_ENUM_RESERVED) {
    if (overlap_secret == 0) {
      *selected_secret = DLK_SECRET_ENUM_WELL_KNOWN_KEY;
    } else {
      if (overlap_secret & DLK_SECRET_MASK_SYMMETRIC_AUTH_TOKEN) {
        *selected_secret = DLK_SECRET_ENUM_SYMMETRIC_AUTH_TOKEN;
      } else if (overlap_secret & DLK_SECRET_MASK_ADMIN_ACCESS_KEY) {
        *selected_secret = DLK_SECRET_ENUM_ADMIN_ACCESS_KEY;
      } else if (overlap_secret & DLK_SECRET_MASK_BASIC_ACCESS_KEY) {
        *selected_secret = DLK_SECRET_ENUM_BASIC_ACCESS_KEY;
      } else if (overlap_secret & DLK_SECRET_MASK_PRECONFIG_INSTALL_CODE) {
        *selected_secret = DLK_SECRET_ENUM_PRECONFIG_INSTALL_CODE;
      } else if (overlap_secret & DLK_SECRET_MASK_VARIABLE_LENGTH_PASSCODE) {
        *selected_secret = DLK_SECRET_ENUM_VARIABLE_LENGTH_PASSCODE;
      }
    }
  } else {
    *selected_secret = gu_override_selected_secret;
  }
  sl_zigbee_core_debug_println("selecting dlk params: method %0x, secret %0x", *selected_method, *selected_secret);
  return SL_STATUS_OK;
}

static void toggle_r23_feature_support(bool r23_enabled)
{
  uint8_t i;
  if (r23_enabled) {
    sl_zigbee_set_stack_compliance_revision(R23_COMPLIANCE_REVISION);
  } else {
    sl_zigbee_set_stack_compliance_revision(R22_COMPLIANCE_REVISION);
  }
  sl_disable_beacon_tlvs((r23_enabled ? false : true));
  slx_zigbee_gu_zdo_toggle_dlk(r23_enabled, r23_enabled);
  for (i = 0; i < SL_ZIGBEE_SUPPORTED_NETWORKS; i++) {
    sl_zigbee_set_pan_id_conflict_report((r23_enabled ? false : true), i);
  }
}

void enable_r23_features_command(sl_cli_command_arg_t *arguments)
{
  bool enable_r23 = true;
  if (sl_cli_get_argument_count(arguments) >= 1) {
    enable_r23 = sl_cli_get_argument_uint16(arguments, 0);
  }
  toggle_r23_feature_support(enable_r23);
  sl_zigbee_core_debug_println("enable R23 features? %c", enable_r23 ? 'y' : 'n');
}

void sl_zigbee_r23_gu_init(uint8_t init_level)
{
  switch (init_level) {
    case SL_ZIGBEE_INIT_LEVEL_DONE:
      // NOTE we want to act like a 'legacy' stack by default
      sl_zigbee_af_event_init(&DeviceInterviewEvent, zcp_device_interview_keepalive);
      toggle_r23_feature_support(false);
      break;
    default:
      break;
  }
}

void start_key_update_command(sl_cli_command_arg_t *arguments)
{
  sl_zigbee_address_info target;
  target.device_short = sl_cli_get_argument_uint16(arguments, 0);
  if (sl_zigbee_lookup_eui64_by_node_id(target.device_short, target.device_long) != SL_STATUS_OK) {
    sl_zigbee_core_debug_println("ERROR: could not find long id corresponding to %02x", target.device_short);
    return;
  }
  sl_zigbee_dlk_negotiation_method selected_method = sl_cli_get_argument_uint8(arguments, 1);
  sl_zigbee_dlk_negotiation_shared_secret_source selected_secret = sl_cli_get_argument_uint8(arguments, 2);
  sl_status_t status = sl_zigbee_zdo_dlk_start_key_update(&target, selected_method, selected_secret);
  sl_zigbee_core_debug_println("ZDO Start Key Update Request: 0x%02X", status);
}

void set_dlk_override_supported_methods(sl_cli_command_arg_t *arguments)
{
  if (sl_cli_get_argument_count(arguments) == 0) {
    sl_zigbee_core_debug_println("clear dlk supported method overrides");
    gu_dlk_override_supported_methods = DLK_PROTOCOL_MASK_RESERVED;
    return;
  }
  // NOTE set it to zero so it's stateless
  gu_dlk_override_supported_methods = 0;
  for (uint8_t i = 0; i < sl_cli_get_argument_count(arguments); i++) {
    uint8_t set_bit = sl_cli_get_argument_uint8(arguments, i);
    if (set_bit >= DLK_PROTOCOL_ENUM_RESERVED) {
      sl_zigbee_core_debug_println("bit %i is out of bounds (max %d)", set_bit, DLK_PROTOCOL_ENUM_RESERVED - 1);
    } else {
      gu_dlk_override_supported_methods |= BIT(set_bit);
    }
  }
  sl_zigbee_core_debug_println("supported dlk methods = %0x", gu_dlk_override_supported_methods);
}

void set_dlk_override_supported_secrets(sl_cli_command_arg_t *arguments)
{
  if (sl_cli_get_argument_count(arguments) == 0) {
    sl_zigbee_core_debug_println("clear dlk supported secrets overrides");
    gu_dlk_override_supported_secrets = DLK_SECRET_MASK_RESERVED;
    return;
  }
  // NOTE set it to zero so it's stateless
  gu_dlk_override_supported_secrets = 0;
  for (uint8_t i = 0; i < sl_cli_get_argument_count(arguments); i++) {
    uint8_t set_bit = sl_cli_get_argument_uint8(arguments, i);
    if (set_bit >= DLK_SECRET_ENUM_RESERVED) {
      sl_zigbee_core_debug_println("bit %i is out of bounds (max %d)", set_bit, DLK_SECRET_ENUM_RESERVED - 1);
    } else {
      gu_dlk_override_supported_secrets |= BIT(set_bit);
    }
  }
  sl_zigbee_core_debug_println("supported dlk secrets = %0x", gu_dlk_override_supported_secrets);
}

void set_dlk_override_selected_parameters(sl_cli_command_arg_t *arguments)
{
  gu_override_selected_method = sl_cli_get_argument_uint8(arguments, 0);
  gu_override_selected_secret = sl_cli_get_argument_uint8(arguments, 1);
  sl_zigbee_core_debug_println("override selection set");
}

static sl_zigbee_sec_man_key_t gu_dlk_override_psk_data;
static bool gu_dlk_override_psk;
void set_dlk_override_psk(sl_cli_command_arg_t *arguments)
{
  if (sl_cli_get_argument_count(arguments) == 0) {
    sl_zigbee_core_debug_println("clear override psk");
    memset(gu_dlk_override_psk_data.key, 0, SL_ZIGBEE_ENCRYPTION_KEY_SIZE);
    gu_dlk_override_psk = false;
    return;
  }
  size_t psk_arg_len;
  uint8_t *psk_arg = sl_cli_get_argument_hex(arguments, 0, &psk_arg_len);
  memcpy(gu_dlk_override_psk_data.key, psk_arg, psk_arg_len);
  // NOTE right pad short keys
  if (psk_arg_len < SL_ZIGBEE_ENCRYPTION_KEY_SIZE) {
    sl_zigbee_core_debug_println("%d bytes of padding", SL_ZIGBEE_ENCRYPTION_KEY_SIZE - psk_arg_len);
    memset(&gu_dlk_override_psk_data.key[psk_arg_len], 0, SL_ZIGBEE_ENCRYPTION_KEY_SIZE - psk_arg_len);
  }
  gu_dlk_override_psk = true;
  sl_zigbee_core_debug_println("override psk set");
}

void toggle_dlk_enabled(sl_cli_command_arg_t *arguments)
{
  bool do_enable = sl_zigbee_zdo_dlk_enabled();
  if (sl_cli_get_argument_count(arguments) == 0) {
    do_enable = !do_enable;
  } else {
    do_enable = (bool) sl_cli_get_argument_uint8(arguments, 0);
  }
  slx_zigbee_gu_zdo_toggle_dlk(do_enable, do_enable);
  sl_zigbee_core_debug_println("dlk %s-abled", do_enable ? "en" : "dis");
}

bool slxi_zigbee_stack_gu_zdo_dlk_override_psk_fetch(uint8_t *key_buffer)
{
  if (gu_dlk_override_psk) {
    memcpy(key_buffer, gu_dlk_override_psk_data.key, SL_ZIGBEE_ENCRYPTION_KEY_SIZE);
  }
  return gu_dlk_override_psk;
}

enum DlkTestCaseScenario {
  DLK_TEST_SCENARIO_NONE = 0,
  DLK_TEST_SCENARIO_SECURITY_POLICY = 1,
  DLK_TEST_SCENARIO_MANGLED_TLV = 2,
  DLK_TEST_SCENARIO_MISSING_TLV = 3,
  DLK_TEST_SCENARIO_EXTRA_TLVS = 4,
  DLK_TEST_SCENARIO_END,
};

static uint8_t gu_dlk_negative_test_scenario = DLK_TEST_SCENARIO_NONE;

void setup_dlk_negative_testcase(sl_cli_command_arg_t *arguments)
{
  uint8_t scenario;
  if (sl_cli_get_argument_count(arguments) == 0) {
    scenario = DLK_TEST_SCENARIO_NONE;
  } else {
    scenario = sl_cli_get_argument_uint8(arguments, 0);
  }

  if (scenario == DLK_TEST_SCENARIO_NONE) {
    sl_zigbee_core_debug_println("clearing dlk-- behavior");
    gu_dlk_negative_test_scenario = DLK_TEST_SCENARIO_NONE;
    sl_zigbee_set_zdo_dlk_save_derived_key(true);
    sl_zigbee_set_trust_center_link_key_request_policy(SL_ZIGBEE_ALLOW_TC_LINK_KEY_REQUEST_AND_GENERATE_NEW_KEY);
    return;
  }

  if (scenario == DLK_TEST_SCENARIO_SECURITY_POLICY) {
    sl_zigbee_set_trust_center_link_key_request_policy(SL_ZIGBEE_DENY_TC_LINK_KEY_REQUESTS);
  } else {
    sl_zigbee_set_trust_center_link_key_request_policy(SL_ZIGBEE_ALLOW_TC_LINK_KEY_REQUEST_AND_GENERATE_NEW_KEY);
  }

  if (scenario == DLK_TEST_SCENARIO_EXTRA_TLVS) {
    sl_zigbee_set_zdo_dlk_save_derived_key(true);
  } else {
    sl_zigbee_set_zdo_dlk_save_derived_key(false);
  }
  gu_dlk_negative_test_scenario = scenario;
  sl_zigbee_core_debug_println("dlk-- test %d set", scenario);
}

bool slxi_zigbee_stack_gu_zdo_dlk_mangle_packet(sli_buffer_manager_buffer_t *buffer)
{
  sl_zigbee_tlv_chain chain;
  sl_status_t status;
  sli_buffer_manager_buffer_t scratch = NULL_BUFFER;
  // TODO remove magic values
  switch (gu_dlk_negative_test_scenario) {
    case DLK_TEST_SCENARIO_MANGLED_TLV: {
      sl_zigbee_core_debug_println("<DLK> add mangled tlvs");
      scratch = sli_legacy_buffer_manager_allocate_buffer(50);
      status = sl_zigbee_tlv_initialize_empty_chain(&chain,
                                                    sli_legacy_buffer_manager_get_buffer_pointer(scratch),
                                                    sli_legacy_buffer_manager_get_buffer_length(scratch));
      if (status != SL_STATUS_OK) {
        return false;
      }
      sl_zigbee_tlv_t *t;
      status = sl_zigbee_tlv_chain_add_tlv_block(&chain,
                                                 0xDD,
                                                 15,
                                                 &t);
      if (status != SL_STATUS_OK) {
        return false;
      }
      // NOTE actually mangle the tlvs
      sl_zigbee_tlv_set_length(t, 0xff);
      memset(sl_zigbee_tlv_get_value_ptr(t), 0xCD, 15);
      status = sli_zigbee_tlv_chain_append_to_buffer(&chain, buffer);
      if (status != SL_STATUS_OK) {
        return false;
      }
      return true;
    }
    case DLK_TEST_SCENARIO_MISSING_TLV: {
      // NOTE don't add anything to the tlv
      sl_zigbee_core_debug_println("<DLK> missing tlvs");
      return true;
    }
    case DLK_TEST_SCENARIO_EXTRA_TLVS: {
      sl_zigbee_core_debug_println("<DLK> extra tlvs");
      sl_zigbee_tlv_t *t;
      scratch = sli_legacy_buffer_manager_allocate_buffer(75);
      status = sl_zigbee_tlv_initialize_empty_chain(&chain,
                                                    sli_legacy_buffer_manager_get_buffer_pointer(scratch),
                                                    sli_legacy_buffer_manager_get_buffer_length(scratch));
      if (status != SL_STATUS_OK) {
        return false;
      }
      status = sl_zigbee_tlv_chain_add_tlv_block(&chain,
                                                 32,
                                                 5,
                                                 &t);
      if (status != SL_STATUS_OK) {
        return false;
      }
      memset(sl_zigbee_tlv_get_value_ptr(t), 0, sl_zigbee_tlv_get_length(t));

      status = sl_zigbee_tlv_chain_add_tlv_block(&chain,
                                                 SL_ZIGBEE_GLOBAL_TLV_MFG_SPECIFIC_TAG_ID,
                                                 10,        // TODO EMZIGBEE-8046 send a bigger tlv
                                                 &t);
      if (status != SL_STATUS_OK) {
        return false;
      }
      uint8_t *val = sl_zigbee_tlv_get_value_ptr(t);
      val[0] = 0x34;
      val[1] = 0x12;
      memset(val + 2, 0, 8);
      status = sli_zigbee_tlv_chain_append_to_buffer(&chain, buffer);
      if (status != SL_STATUS_OK) {
        return false;
      }
      return false;
    }
    case DLK_TEST_SCENARIO_NONE:
    case DLK_TEST_SCENARIO_SECURITY_POLICY:
    default: {
      return false;
    }
  }
  return false;
}

#define MIN_EUI64_LIST_CLI_ARG_NO 2
#define MAX_TLV_EUI64_LIST_SIZE 5 // 255? if fragmentation is allowed

void start_zdo_decommission_device_command(sl_cli_command_arg_t *arguments)
{
  uint8_t count = sl_cli_get_argument_count(arguments);
  if (count <= MIN_EUI64_LIST_CLI_ARG_NO || count > (MAX_TLV_EUI64_LIST_SIZE + MIN_EUI64_LIST_CLI_ARG_NO)) {
    sl_zigbee_core_debug_println("INVALID ARGS: There needs to be at least one target, encryption_enabled, and  1 <= EUI64 < MAX_TLV_EUI64_LIST_SIZE (5) for the device(s) that need to be decommissioned");
    return;
  }
  sl_802154_long_addr_t device_long[MAX_TLV_EUI64_LIST_SIZE];
  sl_802154_short_addr_t target = sl_cli_get_argument_uint16(arguments, 0);
  bool encrypt = (bool) sl_cli_get_argument_uint8(arguments, 1);
  uint8_t arg = MIN_EUI64_LIST_CLI_ARG_NO;
  while (arg < count) {
    sl_zigbee_copy_eui64_arg(arguments, arg, device_long[arg - MIN_EUI64_LIST_CLI_ARG_NO], false);
    arg++;
  }

  sl_status_t status = sl_zigbee_test_zdo_generate_security_decommission_req(target, encrypt, (uint8_t *)device_long, count - MIN_EUI64_LIST_CLI_ARG_NO);
  sl_zigbee_core_debug_println("sli_zigbee_zdo_generate_security_decommission_req: 0x%02X", status);
}

sl_status_t sli_zigbee_stack_test_zdo_generate_security_decommission_req(sl_802154_short_addr_t destination,
                                                                         bool encrypt,
                                                                         uint8_t* eui64_list,
                                                                         uint8_t counts)
{
  return sli_zigbee_zdo_generate_security_decommission_req(destination, encrypt, (void*)eui64_list, counts);
}

void set_ignore_acks_command(sl_cli_command_arg_t *arguments)
{
  bool ignore = (bool) sl_cli_get_argument_uint8(arguments, 0);
  sl_zigbee_set_ignore_aps_acks(ignore);
  sl_zigbee_core_debug_println("ok");
}

void send_route_discovery_with_tlvs(sl_cli_command_arg_t *arguments)
{
#ifndef SL_ZIGBEE_LEAF_STACK
  sl_802154_short_addr_t target = sl_cli_get_argument_uint16(arguments, 0);
  (void)sl_zigbee_test_send_route_request_with_tlv(target);
#else
  UNUSED_VAR(arguments);
#endif
}

void send_clear_all_bindings_command(sl_cli_command_arg_t *arguments)
{
  uint8_t count = sl_cli_get_argument_count(arguments);
  if (count <= MIN_EUI64_LIST_CLI_ARG_NO || count > (MAX_TLV_EUI64_LIST_SIZE + MIN_EUI64_LIST_CLI_ARG_NO)) {
    sl_zigbee_core_debug_println("INAVLID ARGS: There needs to be at least one target, encryption_enabled, and  1 <= EUI64 < MAX_TLV_EUI64_LIST_SIZE (5) for the device(s) that need to be decommissioned");
    return;
  }
  sl_802154_long_addr_t device_long[MAX_TLV_EUI64_LIST_SIZE];
  sl_802154_short_addr_t target = sl_cli_get_argument_uint16(arguments, 0);
  bool encrypt = (bool) sl_cli_get_argument_uint8(arguments, 1);
  uint8_t arg = MIN_EUI64_LIST_CLI_ARG_NO;
  while (arg < count) {
    sl_zigbee_copy_eui64_arg(arguments, arg, device_long[arg - MIN_EUI64_LIST_CLI_ARG_NO], false);
    arg++;
  }

  sl_status_t status = sl_zigbee_test_zdo_generate_clear_all_bindings_req(target, encrypt, (void *)device_long, count - MIN_EUI64_LIST_CLI_ARG_NO);
  sl_zigbee_core_debug_println("Send Clear All Bindings 0x%02X", status);
}

sl_status_t sli_zigbee_stack_test_zdo_generate_clear_all_bindings_req(sl_802154_short_addr_t target, bool encrypt, uint8_t* eui64_list, uint8_t counts)
{
  return sli_zigbee_zdo_generate_clear_all_bindings_req(target, encrypt, (void*)eui64_list, counts);
}

// NOTE for over the air key dumps
#include "stack/framework/slx_zigbee_insecure_debug_key_trace.h"

// The following CLI in pro-compliance on a TC node simulates a TC back up and restore.
// It looks for the link keys and reloads the hash of the current link keys, leaves the
// auth token for the joined devices in the key slot, changes the EUI64 of the TC
// initializes the network. This simulates as if the TC is changed and the hashed keys
// and auth tokens are loaded.
// In a real scenario (or in 17.5.3 using two TCs), the auth tokens and hashed keys from
// key table of the old TC will be saved somewhere outside and then transferred to the new
// TC.
// For testing on hw, use the security manager key export CLI to get all the keys and the
// auth tokens from old TC and then load then on the new TC using the security manager CLIs.
// For a TC application to do the same, trust center backup component.
void tc_backup_restore(sl_cli_command_arg_t *arguments)
{
  UNUSED_VAR(arguments);
  sl_zigbee_sec_man_context_t context;
  sl_zigbee_sec_man_key_t plaintext_key;
  sl_zigbee_sec_man_aps_key_metadata_t key_data;

  // Go through the key table and replace the keys with hash key
  for (uint8_t i = 0; i < SL_ZIGBEE_KEY_TABLE_SIZE; i++) {
    sl_status_t status = sl_zigbee_sec_man_export_link_key_by_index(i,
                                                                    &context,
                                                                    &plaintext_key,
                                                                    &key_data);
    // The link key and the auth token are stored in the same key table and returned
    // by the above api with their respective bit mask.
    // So,find the key, not the authentication token, that needs to be hshed and backed up.
    if (status == SL_STATUS_OK
        && !(key_data.bitmask & SL_ZIGBEE_KEY_IS_AUTHENTICATION_TOKEN)) {
      sl_zigbee_sec_man_key_t hashed_key; // The hash of the key, that will be used for next joining for the same device.
      (void) sl_zigbee_aes_hash_simple(16,
                                       (const uint8_t *)plaintext_key.key,
                                       (uint8_t *)hashed_key.key);
      // The following is a short-cut to saving the hashed key somewhere else as part of back up and then
      // load on a new TC, it just adds the hashed key in the key slot. The same node in the ZCP test becomes
      // a new TC by changing its eui64.
      (void) sl_zigbee_sec_man_import_link_key(i,
                                               context.eui64,
                                               &hashed_key);        // Set the hash key in the key slot for the same device
      slx_zigbee_insecure_debug_generate_trace(
        SLX_ZIGBEE_INSECURE_DEBUG_NWK_REPORT_KEY_PACKET,
        (void *) hashed_key.key
        );
    }
  }
  // This part of the code is making the same node as a new TC by changing the EUI64 for ZCP TCSO test.
  sl_zigbee_key_data_t keyNew = {
    .contents = { 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
                  0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb }
  };
  uint8_t newEui64[] = { 0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB };

  //change the extended address
  sl_zigbee_set_eui64(newEui64);
  sli_zigbee_set_trust_center_data(0, 0, newEui64, NULL);
  // Set a new network key
  sli_zigbee_set_network_key(&keyNew, 0, true);

  // Erase/initialise all other data from stack context to simulate a new TC
  sl_zigbee_clear_binding_table();
  sli_zigbee_erase_child_table();
}

/**
 * @brief sends a an aps security challenge command to the given short id
 * @args "v!b"
 * @note if it's not the trust center please supply either the key data or the eui64
 */
// TODO change this cli command to always take an eui?
// TODO this cli command is probably overtuned and should be simpler
union ChallengeStore {
  uint8_t bytes[8];
  uint16_t shorts[4];
};

enum SecurityChallengeTestCaseScenario {
  SECURITY_CHALLENGE_TEST_SCENARIO_STANDARD = 0,
  SECURITY_CHALLENGE_TEST_SCENARIO_EMPTY_MESSAGE = 1,
  SECURITY_CHALLENGE_TEST_SCENARIO_ADDITIONAL_TLVS = 2,
};

sl_status_t sli_zigbee_stack_test_custom_send_security_challenge_request(sl_802154_short_addr_t destShort,
                                                                         sl_zigbee_sec_man_context_t *context,
                                                                         uint8_t cmdoptions)
{
  // TODO maybe we should validate and or set up our session data here
  if (context == NULL) {
    return SL_STATUS_NULL_POINTER;
  }
  // construct challenge tlvs
  // generate a random challenge value
  union ChallengeStore challenge;
  sl_status_t status = sl_zigbee_get_strong_random_number_array(challenge.shorts, 4);
  if (status != SL_STATUS_OK) {
    return SL_STATUS_FAIL;
  }
  sli_buffer_manager_buffer_t challengeTlvs = sli_zigbee_aps_create_challenge_tlvs(false, // isRsp
                                                                                   sl_zigbee_get_eui64(),
                                                                                   challenge.bytes,
                                                                                   context,
                                                                                   0xFFFF); // ignored
  // NOTE some test cases require us to include additional tlvs
  if (challengeTlvs == NULL_BUFFER) {
    return SL_STATUS_ALLOCATION_FAILED;
  }

  sl_zigbee_aps_frame_t frameStruct;
  sl_zigbee_aps_option_t options = SL_ZIGBEE_APS_OPTION_RETRY;
  sli_zigbee_zig_dev_prepare_zdo_request(&frameStruct, SECURITY_CHALLENGE_REQUEST, options);
  // send request

  sli_buffer_manager_buffer_t request = sli_zigbee_make_zig_dev_message("", NULL);
  //For Golden Unit test behaviors
  //0 sends standard security challenge request
  //1 sends an empty message to target (doesn't append the request TLV)
  //2 appends other specific TLVs to the request message
  if (cmdoptions == SECURITY_CHALLENGE_TEST_SCENARIO_STANDARD
      || cmdoptions == SECURITY_CHALLENGE_TEST_SCENARIO_ADDITIONAL_TLVS) {
    status = sl_legacy_buffer_manager_append_to_linked_buffers(request,
                                                               sli_legacy_buffer_manager_get_buffer_pointer(challengeTlvs),
                                                               sli_legacy_buffer_manager_get_buffer_length(challengeTlvs));
  } else {
    status = SL_STATUS_OK;
  }

  if (cmdoptions == SECURITY_CHALLENGE_TEST_SCENARIO_ADDITIONAL_TLVS) {
    uint8_t temp[20] = { 64, 9, 0xff, 0xfe, 0, 0, 0, 0, 0, 0, 0, 0, 60, 5, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
    // append mfg specific tlv (first 12 bytes) and unknown tlv (last 8 bytes)
    status = sl_legacy_buffer_manager_append_to_linked_buffers(request,
                                                               temp,
                                                               20);
  }
  if (status == SL_STATUS_OK) {
    status = sli_zigbee_send_unicast_zig_dev_message(destShort, &frameStruct, request);
  }
  return (status == SL_STATUS_OK) ? SL_STATUS_OK : SL_STATUS_FAIL;
}

static uint8_t challenge_rsp_tlv_options = 0;

void update_challenge_rsp_tlv(sl_cli_command_arg_t *arguments)
{
  challenge_rsp_tlv_options = sl_cli_get_argument_uint8(arguments, 0);
}

sli_buffer_manager_buffer_t slx_gu_fc_challenge_finalize_cb(sli_buffer_manager_buffer_t tlvs)
{
  // For Golden Unit test behavior (confirm that an invalid
  // response TLV doesn't successfully complete challenge)
  if (challenge_rsp_tlv_options == 1) {
    int i = 26;
    uint8_t *tlvBytes = sli_legacy_buffer_manager_get_buffer_pointer(tlvs);
    tlvBytes[i++] = 0xFF;
    tlvBytes[i++] = 0xFF;
    tlvBytes[i++] = 0xFF;
    tlvBytes[i++] = 0xFF;
  }
  return tlvs;
}

void send_security_challenge_command(sl_cli_command_arg_t *arguments)
{
  sl_802154_short_addr_t shortDest = (sl_802154_short_addr_t)sl_cli_get_argument_uint16(arguments, 0);
  sl_zigbee_sec_man_key_t keyBytes;
  if (shortDest != SL_ZIGBEE_TRUST_CENTER_NODE_ID
      && sl_cli_get_argument_count(arguments) < 2) {
    sl_zigbee_core_debug_println("tc is not dest,please provide either 8byte EUI or 16byte key");
    return;
  }

  sl_zigbee_sec_man_context_t context;
  sl_zigbee_sec_man_aps_key_metadata_t key_data;
  sl_zigbee_sec_man_init_context(&context);

  sl_status_t status;

  uint8_t options = sl_cli_get_argument_uint8(arguments, 1);
  // get the keyToChallenge
  if (shortDest == SL_ZIGBEE_TRUST_CENTER_NODE_ID) {
    // get the key from the preconfigured key slot
    context.core_key_type = SL_ZB_SEC_MAN_KEY_TYPE_TC_LINK;
    status = sl_zigbee_sec_man_check_key_context(&context);
    if (status != SL_STATUS_OK) {
      sl_zigbee_core_debug_println("Failed to get TC link key");
      return;
    }
  } else {
    // parse the second command argument and figure out what type it is
    uint8_t hint[16];
    uint8_t copyBytes = sl_zigbee_copy_hex_arg(arguments,
                                               2, // arg position
                                               hint,
                                               SL_ZIGBEE_ENCRYPTION_KEY_SIZE,
                                               true); // left pad with 0s
    switch (copyBytes) {
      case EUI64_SIZE: {
        // use the given eui64 to find the entry in the key table
        status = sl_zigbee_sec_man_export_link_key_by_eui(hint,
                                                          &context,
                                                          NULL,
                                                          &key_data);
        if (status != SL_STATUS_OK) {
          sl_zigbee_core_debug_println("sorry key not found for that long id");
          return;
        }
        break;
      }
      case SL_ZIGBEE_ENCRYPTION_KEY_SIZE: {
        // copy the bytes to be used in the challenge
        memmove(&keyBytes, (sl_zigbee_sec_man_key_t*)&hint, SL_ZIGBEE_ENCRYPTION_KEY_SIZE);
        context.core_key_type = SL_ZB_SEC_MAN_KEY_TYPE_INTERNAL;
        sl_zigbee_sec_man_import_key(&context, &keyBytes);
        break;
      }
      default:
        sl_zigbee_core_debug_println("must provide either 8byte EUI or 16byte key");
        return;
    }
  }
  status = sl_zigbee_test_custom_send_security_challenge_request(shortDest,
                                                                 &context,
                                                                 options);
  sl_zigbee_core_debug_println("send challenge returned %02X", status);
}

/**
 * @brief prints out information about the aps link key, and optionally modifies values
 * @args "u!u!w"
 */
void aps_key_sync_status_command(sl_cli_command_arg_t *arguments)
{
  sl_status_t status;

  // HACK this is complicated/lot's of duplication so we should rewrite
  // -- or be ready to document if we never get to it...
  // use an index of 0xFF to indicate the preconfigured key slot, get rid of one argument
  // NOTE the arg format here is a little tricky
  // ARG-0: key table index
  //   0xFF - TC Link Key
  //   ELSE - key table index
  // ARG-1 [optional]: sets the status of the link key
  //  0 - frame counter not in sync
  //  1 - frame counter sync
  // ARG-2 [optional]: 32bit frame counter value to set
  uint8_t index = sl_cli_get_argument_uint8(arguments, 0);

  sl_zigbee_sec_man_context_t context;
  sl_zigbee_sec_man_key_t plaintext_key;
  sl_zigbee_sec_man_aps_key_metadata_t key_metadata;
  bool inSync = false;
  uint32_t apsIncomingframeCounter = 0;
  uint32_t apsOutgoingframeCounter = 0;

  sl_zigbee_sec_man_init_context(&context);

  if (index == 0xFF) {
    // Preconfigured Key Slot
    context.core_key_type = SL_ZB_SEC_MAN_KEY_TYPE_TC_LINK;
    status = sl_zigbee_sec_man_export_key(&context, &plaintext_key);
    if (status != SL_STATUS_OK) {
      sl_zigbee_core_debug_println("ERROR preconfigured key fetch returned %02X", status);
      return;
    }
    //ensure context's EUI64 is set
    (void) sl_zigbee_sec_man_get_aps_key_info(&context, &key_metadata);
    if (sl_cli_get_argument_count(arguments) >= 2) {
      bool setIsSync = (bool)sl_cli_get_argument_uint8(arguments, 1);
      // set the sync status
      (void)sl_zigbee_test_aps_key_set_sync_status(context.eui64, setIsSync);
      if (sl_cli_get_argument_count(arguments) >= 4) {
        uint32_t setFrameCounter = sl_cli_get_argument_uint32(arguments, 3);
        uint8_t option = sl_cli_get_argument_uint8(arguments, 2);
        if (option == 0) {
          // set the frame counter
          sl_zigbee_set_incoming_tc_link_key_frame_counter(setFrameCounter);
        }
        if (option == 1) {
          sl_zigbee_set_outgoing_aps_frame_counter(setFrameCounter);
        }
      }
    }
    (void) sl_zigbee_sec_man_get_aps_key_info(&context, &key_metadata);
    // get the sync value
    inSync = sl_zigbee_test_aps_key_in_sync(context.eui64);
    // get the frame counter
    apsIncomingframeCounter = key_metadata.incoming_frame_counter;
    apsOutgoingframeCounter = key_metadata.outgoing_frame_counter;
  } else {
    // APS Key Table
    status = sl_zigbee_sec_man_export_link_key_by_index(index,
                                                        &context,
                                                        &plaintext_key,
                                                        &key_metadata);
    if (status != SL_STATUS_OK) {
      sl_zigbee_core_debug_println("ERROR key table returned %02X", status);
      return;
    }
    if (sl_cli_get_argument_count(arguments) >= 2) {
      bool setIsSync = (bool)sl_cli_get_argument_uint8(arguments, 1);
      // set the sync status
      status = sl_zigbee_test_aps_key_set_sync_status(context.eui64, setIsSync);
      (void) status;
      if (sl_cli_get_argument_count(arguments) >= 4) {
        uint8_t option = sl_cli_get_argument_uint8(arguments, 2);
        uint32_t setFrameCounter = sl_cli_get_argument_uint32(arguments, 3);
        if (option == 0) {
          // set the frame counter
          sli_zigbee_update_link_key_frame_counter(context.eui64, setFrameCounter);
        }
        if (option == 1) {
          sl_zigbee_set_outgoing_aps_frame_counter(setFrameCounter);
        }
      }
      // get the sync value
      inSync = sl_zigbee_test_aps_key_in_sync(context.eui64);
      // get most recent frame counters
      context.core_key_type = SL_ZB_SEC_MAN_KEY_TYPE_APP_LINK;
      context.key_index = index;
      context.flags |= ZB_SEC_MAN_FLAG_KEY_INDEX_IS_VALID;
      (void) sl_zigbee_sec_man_get_aps_key_info(&context, &key_metadata);
      // get the frame counter
      apsIncomingframeCounter = key_metadata.incoming_frame_counter;
      apsOutgoingframeCounter = key_metadata.outgoing_frame_counter;
    }
  }
  // printf the key values
  sl_zigbee_core_debug_print("{ partner: %02X%02X%02X%02X%02X%02X%02X%02X, ",
                             context.eui64[0],
                             context.eui64[1],
                             context.eui64[2],
                             context.eui64[3],
                             context.eui64[4],
                             context.eui64[5],
                             context.eui64[6],
                             context.eui64[7]);
  sl_zigbee_core_debug_print("sync: %c, Infc: %08X OutFc %08X, ",
                             inSync ? 'y' : 'n',
                             apsIncomingframeCounter,
                             apsOutgoingframeCounter);
  uint8_t *keyPtr = (uint8_t*)&(plaintext_key.key);
  sl_zigbee_core_debug_println("key: %02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X }\n",
                               keyPtr[0],
                               keyPtr[1],
                               keyPtr[2],
                               keyPtr[3],
                               keyPtr[4],
                               keyPtr[5],
                               keyPtr[6],
                               keyPtr[7],
                               keyPtr[8],
                               keyPtr[9],
                               keyPtr[10],
                               keyPtr[11],
                               keyPtr[12],
                               keyPtr[13],
                               keyPtr[14],
                               keyPtr[15]);
}

//wrappers for thread-safety
bool sli_zigbee_stack_test_aps_key_in_sync(sl_802154_long_addr_t eui64)
{
  return sli_zigbee_aps_key_in_sync(eui64);
}

sl_status_t sli_zigbee_stack_test_aps_key_set_sync_status(sl_802154_long_addr_t eui64, bool setSync)
{
  return sli_zigbee_aps_key_set_sync_status(eui64, setSync);
}

void aps_frame_counter_verify_command(sl_cli_command_arg_t *arguments)
{
  bool enable = sl_cli_get_argument_uint8(arguments, 0);
  sli_zigbee_set_global_fc_sync_flag(enable);
}

void beacon_tlvs_disable_command(sl_cli_command_arg_t *arguments)
{
  bool disable = sl_cli_get_argument_uint8(arguments, 0);
  sl_disable_beacon_tlvs(disable);
}

static bool guDuplicateRelays = false;
void enable_aps_relay_duplication_command(sl_cli_command_arg_t *arguments)
{
  bool do_enable = (bool) sl_cli_get_argument_uint8(arguments, 0);
  guDuplicateRelays = do_enable;
  sl_zigbee_core_debug_println("%s duplicates", do_enable ? "enable" : "disable");
}

bool slx_gu_do_relay_dual_submit(void)
{
  return guDuplicateRelays;
}

// device interview
struct interview_context {
  sl_zigbee_address_info id;
};

static struct interview_context g_interview_ctx = {
  .id = { .device_long = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, },
          .device_short = 0xFFFF }
};

//static uint8_t challeng_rsp_tlv_options = 0;
static uint32_t interview_backoff_ms = 1000;

void sl_zigbee_dynamic_commissioning_alert_callback(sl_zigbee_address_info *ids,
                                                    sl_zigbee_dynamic_commissioning_event_t event)
{
  if (event == SL_ZIGBEE_DYNAMIC_COMMISSIONING_EVENT_OPEN_REQUEST) {
    sl_zigbee_core_debug_println("dynamic commissioning request");
    return;
  }
  if (event == SL_ZIGBEE_DEVICE_INTERVIEW_EVENT_READY) {
    if (sl_zigbee_dynamic_commissioning_is_open_for_interview()) {
      g_interview_ctx.id.device_short = ids->device_short;
      memcpy(&g_interview_ctx.id, ids, sizeof(sl_zigbee_address_info));
      sl_zigbee_af_event_set_delay_ms(&DeviceInterviewEvent, interview_backoff_ms);
      sl_zigbee_core_debug_println("interview started");
    } else {
      sl_zigbee_core_debug_println("no interview requested allow join\n");
      return;
    }
  }
  if ( sl_zigbee_dynamic_commissioning_is_open_for_interview()
       && event == SL_ZIGBEE_DEVICE_INTERVIEW_EVENT_DOWNSTREAM_MIDPOINT) {
    g_interview_ctx.id.device_short = ids->device_short;
    memcpy(&g_interview_ctx.id, ids, sizeof(sl_zigbee_address_info));
    sl_zigbee_af_event_set_delay_ms(&DeviceInterviewEvent, interview_backoff_ms);
  }
  if (event == SL_ZIGBEE_DYNAMIC_COMMISSIONING_EVENT_ERROR) {
    sl_zigbee_dynamic_commissioning_set_open_for_interview(false);     // stop application timer
    sl_zigbee_core_debug_println("interview failed %02X \n", event);
  }
}

static void zcp_device_interview_keepalive(sl_zigbee_af_event_t *p_event)
{
  UNUSED_VAR(p_event);
  if (sl_zigbee_dynamic_commissioning_is_open_for_interview()) {
    // refresh all timers
    sl_zigbee_af_event_set_delay_ms(&DeviceInterviewEvent, interview_backoff_ms);
    sli_zigbee_dynamic_commissioning_refresh_timers(g_interview_ctx.id.device_short, g_interview_ctx.id.device_long);
  }
}

/**
 * @brief option to enable the device interview during commissioning process
 * @args "u"
 */
void sl_device_interview_control_command(sl_cli_command_arg_t *arguments)
{
  bool previous = sl_zigbee_dynamic_commissioning_is_open_for_interview();
  bool enableInterview = (bool)sl_cli_get_argument_uint8(arguments, 0);
  sl_zigbee_dynamic_commissioning_set_open_for_interview(enableInterview);
  if (previous == true && enableInterview == false) {
    if (sli_zigbee_am_trust_center) {
      sl_zigbee_core_debug_println("close interview");
      sl_zigbee_address_info info = sl_zigbee_make_device_id_pair(g_interview_ctx.id.device_short,
                                                                  g_interview_ctx.id.device_long);
      sl_zigbee_device_interview_status_update(&info,
                                               SL_ZIGBEE_DYNAMIC_COMMISSIONING_EVENT_ACCEPTED);
    }
  }
}

// duplicate / compatibility commands

#define MACRO_FMT_HEX_EIGHT "%02x %02x %02x %02x %02x %02x %02x %02x"
void exportTcBackupData(sl_cli_command_arg_t *arguments)
{
  UNUSED_VAR(arguments);
  // This command is used to export the TC backup data
  // It is used in the ZCP test to simulate a TC backup and restore
  // In reality now the majority of the work is done by the restore function
  sl_zigbee_core_debug_println("Backup TC Data", 0);
  sl_zigbee_network_parameters_t params = { 0, };
  sl_zigbee_node_type_t node_kind = { 0, };
  sl_status_t status = sl_zigbee_get_network_parameters(&node_kind, &params);
  if (status != SL_STATUS_OK) {
    sl_zigbee_core_debug_println("[Error %0X] during network parameter fetching", status);
    return;
  }
  sl_zigbee_core_debug_print("Ext-PAN ");
  sl_zigbee_core_debug_println(
    MACRO_FMT_HEX_EIGHT,
    params.extendedPanId[0],
    params.extendedPanId[1],
    params.extendedPanId[2],
    params.extendedPanId[3],
    params.extendedPanId[4],
    params.extendedPanId[5],
    params.extendedPanId[6],
    params.extendedPanId[7]
    );
  sl_zigbee_core_debug_println("Key Digests");
  sl_zigbee_sec_man_context_t context;
  sl_zigbee_sec_man_key_t plaintext_key;
  sl_zigbee_sec_man_key_t hashed_key; // The hash of the key, that will be used for next joining for the same device.
  sl_zigbee_sec_man_aps_key_metadata_t aps_key_data;
  for (uint8_t i = 0; i < SL_ZIGBEE_KEY_TABLE_SIZE; i++) {
    sl_status_t status = sl_zigbee_sec_man_export_link_key_by_index(i,
                                                                    &context,
                                                                    &plaintext_key,
                                                                    &aps_key_data);
    // The link key and the auth token are stored in the same key table and returned
    // by the above api with their respective bit mask.
    // So,find the key, not the authentication token, that needs to be hshed and backed up.
    if (status != SL_STATUS_OK) {
      sl_zigbee_core_debug_println("[Error %0X] exporting key table entry %d", status, i);
      continue;
    }
    sl_zigbee_core_debug_print("EUI ");
    sl_zigbee_core_debug_println(
      MACRO_FMT_HEX_EIGHT,
      context.eui64[0],
      context.eui64[1],
      context.eui64[2],
      context.eui64[3],
      context.eui64[4],
      context.eui64[5],
      context.eui64[6],
      context.eui64[7]
      );
    sl_zigbee_sec_man_key_t *key_data = NULL;
    char *type_str;
    if (aps_key_data.bitmask & SL_ZIGBEE_KEY_IS_AUTHENTICATION_TOKEN) {
      key_data = &plaintext_key;
      type_str = "pass";
    } else {
      type_str = "hash";
      (void) sl_zigbee_aes_hash_simple(16,
                                       (const uint8_t *)plaintext_key.key,
                                       (uint8_t *)hashed_key.key);

      key_data = &hashed_key;
    }
    sl_zigbee_core_debug_print("%s Key Data ", type_str);
    sl_zigbee_core_debug_print(
      MACRO_FMT_HEX_EIGHT,
      key_data[0], key_data[1],
      key_data[2], key_data[3],
      key_data[4], key_data[5],
      key_data[6], key_data[7]
      );
    sl_zigbee_core_debug_print(
      MACRO_FMT_HEX_EIGHT,
      key_data[8], key_data[9],
      key_data[10], key_data[11],
      key_data[12], key_data[13],
      key_data[14], key_data[15]
      );
    sl_zigbee_core_debug_println("");
  }
}

void sendZdoGetAuthTokenCommand(sl_cli_command_arg_t *arguments)
{
  // This command sends a ZDO Get Authentication Token request
  // Used in R23 compliance testing for authentication token management
  sl_802154_short_addr_t device_short = sl_cli_get_argument_uint16(arguments, 0);
  bool use_encrypt = (bool) sl_cli_get_argument_uint8(arguments, 1);
  sl_zigbee_aps_option_t options = SL_ZIGBEE_APS_OPTION_NONE;
  if (use_encrypt) {
    // If encryption is requested, set the option
    options |= SL_ZIGBEE_APS_OPTION_ENCRYPTION;
  }
  sl_zigbee_retrieve_authentication_token(device_short, options);
}

void setAuthenticationLevel(sl_cli_command_arg_t *arguments)
{
  // This command sets the authentication level for join and update operations
  // Used in R23 compliance testing for authentication level configuration
  uint8_t join = (uint8_t) sl_cli_get_argument_uint8(arguments, 0);
  uint8_t update = (uint8_t) sl_cli_get_argument_uint8(arguments, 1);
  gu_override_initial_join_method = join;
  gu_override_active_link_key_type = update;
  sl_zigbee_core_debug_println("auth lvl set: join %x - update %x", join, update);
}

void setDlkPskSecretValue(sl_cli_command_arg_t *arguments)
{
  // This command sets the Device Link Key Pre-Shared Key secret value
  // Used in R23 compliance testing for DLK PSK configuration
  if (sl_cli_get_argument_count(arguments) == 0) {
    sl_zigbee_core_debug_print("psk - ");
    for (uint8_t i = 0; i < SL_ZIGBEE_ENCRYPTION_KEY_SIZE; i++) {
      sl_zigbee_core_debug_print(" %x", gu_dlk_override_psk_data.key[i]);
    }
    sl_zigbee_core_debug_println("");
    return;
  }
  size_t psk_arg_len = 0;
  uint8_t *psk_arg = sl_cli_get_argument_hex(arguments, 0, &psk_arg_len);
  memcpy(gu_dlk_override_psk_data.key, psk_arg, psk_arg_len);
  if (psk_arg_len < SL_ZIGBEE_ENCRYPTION_KEY_SIZE) {
    memset(&gu_dlk_override_psk_data.key[psk_arg_len], 0, SL_ZIGBEE_ENCRYPTION_KEY_SIZE - psk_arg_len);
  }
  gu_dlk_override_psk = true;
  sl_zigbee_core_debug_println("psk set %02X", 0);
}

void setBeaconParameters(sl_cli_command_arg_t *arguments)
{
  // This command sets beacon classification parameters
  // Used in R23 compliance testing for beacon parameter configuration
  sl_zigbee_beacon_classification_params_t params;
  sl_status_t status = sl_zigbee_get_beacon_classification_params(&params);
  if (status != SL_STATUS_OK) {
    sl_zigbee_core_debug_println("[Error %0x] getting beacon classification parameters", status);
    return;
  }
  if (sl_cli_get_argument_count(arguments) == 0) {
    sl_zigbee_core_debug_println("beacon params:");
    sl_zigbee_core_debug_println(
      "hub? %c - uptime? %c - prefer? %c  - update id %d",
      params.beaconClassificationMask & TC_CONNECTIVITY ? 'y' : 'n',
      params.beaconClassificationMask & LONG_UPTIME ? 'y' : 'n',
      params.beaconClassificationMask & PREFERRED_PARENT ? 'y' : 'n',
      sli_zigbee_stack_get_nwk_update_id()
      );
  } else {
    bool hasHubConnectivity = (bool) sl_cli_get_argument_uint8(arguments, 0);
    params.beaconClassificationMask |= hasHubConnectivity ? TC_CONNECTIVITY : 0;
    bool hasLongUptime = (bool) sl_cli_get_argument_uint8(arguments, 1);
    params.beaconClassificationMask |= hasLongUptime ? LONG_UPTIME : 0;
    bool hasPreferredParent = (bool) sl_cli_get_argument_uint8(arguments, 2);
    params.beaconClassificationMask |= hasPreferredParent ? PREFERRED_PARENT : 0;
    status = sl_zigbee_set_beacon_classification_params(&params);
    if (status != SL_STATUS_OK) {
      sl_zigbee_core_debug_println("[Error %0x] setting beacon classification params");
    }
    uint8_t nwkUpdateId = (uint8_t) sl_cli_get_argument_uint8(arguments, 3);
    status = sl_zigbee_set_nwk_update_id(nwkUpdateId, true);
    if (status != SL_STATUS_OK) {
      sl_zigbee_core_debug_println("[Error %0x] setting network update id");
    }
  }
}

void sendZdoBeaconSurveyCommand(sl_cli_command_arg_t *arguments)
{
  // This command sends a ZDO Management Beacon Survey request
  // Used in R23 compliance testing for beacon survey operations
  sl_802154_short_addr_t device_short = (sl_802154_short_addr_t) sl_cli_get_argument_uint16(arguments, 0);
  uint8_t scan_config_mask = (uint8_t) sl_cli_get_argument_uint8(arguments, 1);
  uint32_t masks[CHANNEL_PAGE_COUNT] = { 0, };  // NOTE only works with one channel mask
  uint32_t *channel_masks = (uint32_t*)masks;
  channel_masks[0] = sl_cli_get_argument_uint32(arguments, 2);
  sl_zigbee_core_debug_println("survey beacon config %x %4x", scan_config_mask, channel_masks[0]);
  sl_status_t status = sl_zigbee_request_beacon_survey(device_short,
                                                       CHANNEL_PAGE_COUNT,
                                                       channel_masks,
                                                       scan_config_mask);
  sl_zigbee_core_debug_println("request sent %s!", status == SL_STATUS_OK ? "done" : "fail");
}
void disableBeaconTlvsCommand(sl_cli_command_arg_t *arguments)
{
  bool tlvs_disable = (bool) sl_cli_get_argument_uint8(arguments, 0);
  sl_disable_beacon_tlvs(tlvs_disable);
}

void disableDlkBehaviors(sl_cli_command_arg_t *arguments)
{
  bool dlk_disable = (bool) sl_cli_get_argument_uint8(arguments, 0);
  slx_zigbee_gu_zdo_toggle_dlk(dlk_disable, dlk_disable);
  sl_disable_beacon_tlvs(dlk_disable);
  sl_zigbee_set_stack_compliance_revision(dlk_disable ? R22_COMPLIANCE_REVISION : R23_COMPLIANCE_REVISION);
  sl_zigbee_core_debug_println("%sable dlk", (dlk_disable ? "dis" : "en"));
}
