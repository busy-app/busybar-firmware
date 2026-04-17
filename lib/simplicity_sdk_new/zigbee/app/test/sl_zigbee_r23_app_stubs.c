/***************************************************************************//**
 * @file sl_zigbee_r23_app_stubs.c
 * @brief application callbacks needed for r23 stack operation
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
#include "sl_zigbee_types.h"
#include "sl_zigbee_address_info.h"
#include "sl_zigbee_dlk_negotiation.h"
#include "sl_zigbee_zdo_management.h"
#include "sl_zigbee_zdo_security.h"
#include "sl_zigbee_dynamic_commissioning.h"
#include "stack/internal/inc/internal-defs-patch.h"

WEAK(void slx_zigbee_gu_zdo_dlk_override_supported_params(uint8_t *method_mask,
                                                          uint8_t *secret_mask))
{
  (void)method_mask;
  (void)secret_mask;
}

WEAK(sl_status_t sl_zigbee_zdo_dlk_select_negotiation_parameters_callback(sl_zigbee_address_info *partner,
                                                                          uint8_t their_supported_methods,
                                                                          uint8_t their_supported_secrets,
                                                                          uint8_t *selected_method,
                                                                          uint8_t *selected_secret))
{
  (void)partner;
  // check the overlapping supported methods
  sl_zigbee_dlk_supported_negotiation_method our_methods = DLK_PROTOCOL_MASK_STATIC_KEY_REQUEST
                                                           | DLK_PROTOCOL_MASK_SPEKE_C25519_AES128
                                                           | DLK_PROTOCOL_MASK_SPEKE_C25519_SHA256;
  sl_zigbee_dlk_negotiation_supported_shared_secret_source our_secrets = DLK_SECRET_MASK_PRECONFIG_INSTALL_CODE;
  //  | DLK_SECRET_MASK_SYMMETRIC_AUTH_TOKEN
  // | DLK_SECRET_MASK_ADMIN_ACCESS_KEY | DLK_SECRET_MASK_BASIC_ACCESS_KEY | DLK_SECRET_MASK_VARIABLE_LENGTH_PASSCODE;
  // ;
  // sl_zigbee_zdo_dlk_get_supported_negotiation_parameters(&our_methods, &our_secrets);
  sl_zigbee_dlk_supported_negotiation_method overlap_method = our_methods & their_supported_methods;
  sl_zigbee_dlk_negotiation_supported_shared_secret_source overlap_secret = our_secrets & their_supported_secrets;

  // select method based on overlap or the overrides
  if (overlap_method == 0) {
    *selected_method = DLK_PROTOCOL_ENUM_RESERVED;     // No common method
  } else {
    if (overlap_method & DLK_PROTOCOL_MASK_SPEKE_C25519_SHA256) {
      *selected_method = DLK_PROTOCOL_ENUM_SPEKE_C25519_SHA256;
    } else if (overlap_method & DLK_PROTOCOL_MASK_SPEKE_C25519_AES128) {
      *selected_method = DLK_PROTOCOL_ENUM_SPEKE_C25519_AES128;
    } else if (overlap_method & DLK_PROTOCOL_MASK_STATIC_KEY_REQUEST) {
      *selected_method = DLK_PROTOCOL_ENUM_STATIC_KEY;
    }
  }

  // select secret based on overlap or the overrides
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
  return SL_STATUS_OK;
}

WEAK(bool slx_zigbee_gu_zdo_dlk_mangle_packet(sli_buffer_manager_buffer_t *buffer))
{
  (void)buffer;
  return false;
}

WEAK(bool slx_zigbee_gu_zdo_dlk_override_psk(uint8_t *key_buffer))
{
  (void)key_buffer;
  return false;
}

WEAK(void sl_zigbee_zdo_retrieve_authentication_token_complete_callback(sl_status_t status))
{
  (void)status;
}

WEAK(void sl_zigbee_beacon_survey_complete_callback(sl_zigbee_zdo_status_t status,
                                                    sl_zigbee_beacon_survey_results_t *survey_results,
                                                    uint8_t potential_parent_count,
                                                    sl_zigbee_potential_parent_t *potential_parents,
                                                    uint16_t pan_id_conflicts))
{
  UNUSED_VAR(status);
  UNUSED_VAR(survey_results);
  UNUSED_VAR(potential_parent_count);
  UNUSED_VAR(potential_parents);
  UNUSED_VAR(pan_id_conflicts);
}

WEAK(void sl_zigbee_set_authenticaion_level_callback(sl_802154_long_addr_t target,
                                                     sl_zigbee_initial_join_method *initial_join_method,
                                                     sl_zigbee_active_link_key_type *active_link_key_type))
{
  (void)target;
  UNUSED_VAR(initial_join_method);
  UNUSED_VAR(active_link_key_type);
}

WEAK(void sl_zigbee_get_authentication_level_callback(sl_zigbee_zdo_status_t rsp_status,
                                                      sl_802154_long_addr_t target,
                                                      uint8_t join_method,
                                                      uint8_t link_key_update))
{
  UNUSED_VAR(rsp_status);
  UNUSED_VAR(target);
  UNUSED_VAR(join_method);
  UNUSED_VAR(link_key_update);
}

WEAK(void sl_zigbee_zdo_get_config_response_handler(uint8_t response_length,
                                                    uint8_t *response,
                                                    uint16_t payload_index,
                                                    sl_802154_short_addr_t source))
{
  (void)response_length;
  (void)response;
  (void)payload_index;
  (void)source;
}

WEAK(void sl_zigbee_zdo_set_config_response_handler(sli_buffer_manager_buffer_t response,
                                                    sl_zigbee_zdo_status_t zdoStatus,
                                                    uint16_t payload_index,
                                                    sl_802154_short_addr_t source))
{
  (void)response;
  (void)zdoStatus;
  (void)payload_index;
  (void)source;
}

WEAK(void sl_zigbee_af_zdo_set_configuration_req_callback(uint8_t* message_ptr, uint8_t message_length))
{
  (void)message_ptr;
  (void)message_length;
}

WEAK(sli_buffer_manager_buffer_t slx_gu_fc_challenge_finalize_cb(sli_buffer_manager_buffer_t tlvs))
{
  return tlvs;
}

WEAK(void sl_zigbee_dynamic_commissioning_alert_callback(sl_zigbee_address_info *ids,
                                                         sl_zigbee_dynamic_commissioning_event_t event))
{
  UNUSED_VAR(ids);
  UNUSED_VAR(event);
}

WEAK(bool slx_gu_do_relay_dual_submit(void))
{
  return false;
}
