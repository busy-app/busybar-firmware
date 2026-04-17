/***************************************************************************//**
 * @file security_baremetal_wrapper.c
 * @brief internal implementations for 'security' as a thin-wrapper
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
// automatically generated from security.h.  Do not manually edit
#include "stack/include/security.h"
#include "stack/internal/inc/security_internal_def.h"

sl_status_t sl_zigbee_aps_crypt_message(bool encrypt,
                                        uint8_t length,
                                        const uint8_t *message,
                                        uint8_t apsHeaderEndIndex,
                                        sl_802154_long_addr_t remoteEui64)
{
  return sli_zigbee_stack_aps_crypt_message(encrypt,
                                            length,
                                            message,
                                            apsHeaderEndIndex,
                                            remoteEui64);
}

sl_status_t sl_zigbee_clear_key_table(void)
{
  return sli_zigbee_stack_clear_key_table();
}

void sl_zigbee_clear_transient_link_keys(void)
{
  sli_zigbee_stack_clear_transient_link_keys();
}

sl_status_t sl_zigbee_erase_key_table_entry(uint8_t index)
{
  return sli_zigbee_stack_erase_key_table_entry(index);
}

uint8_t sl_zigbee_find_key_table_entry(sl_802154_long_addr_t address,
                                       bool linkKey)
{
  return sli_zigbee_stack_find_key_table_entry(address,
                                               linkKey);
}

sl_status_t sl_zigbee_generate_random_key(sl_zigbee_key_data_t *keyAddress)
{
  return sli_zigbee_stack_generate_random_key(keyAddress);
}

uint32_t sl_zigbee_get_aps_frame_counter(void)
{
  return sli_zigbee_stack_get_aps_frame_counter();
}

sl_status_t sl_zigbee_get_current_security_state(sl_zigbee_current_security_state_t *state)
{
  return sli_zigbee_stack_get_current_security_state(state);
}

sl_status_t sl_zigbee_get_extended_security_bitmask(sl_zigbee_extended_security_bitmask_t *mask)
{
  return sli_zigbee_stack_get_extended_security_bitmask(mask);
}

uint32_t sl_zigbee_get_incoming_tc_link_key_frame_counter(void)
{
  return sli_zigbee_stack_get_incoming_tc_link_key_frame_counter();
}

sl_status_t sl_zigbee_get_mfg_security_config(sl_zigbee_mfg_security_struct_t *settings)
{
  return sli_zigbee_stack_get_mfg_security_config(settings);
}

sl_zigbee_aps_rejoin_mode_t sl_zigbee_get_rejoin_mode(void)
{
  return sli_zigbee_stack_get_rejoin_mode();
}

uint32_t sl_zigbee_get_security_frame_counter(void)
{
  return sli_zigbee_stack_get_security_frame_counter();
}

uint16_t sl_zigbee_get_transient_key_timeout_s(void)
{
  return sli_zigbee_stack_get_transient_key_timeout_s();
}

sl_status_t sl_zigbee_request_link_key(sl_802154_long_addr_t partner)
{
  return sli_zigbee_stack_request_link_key(partner);
}

sl_status_t sl_zigbee_set_extended_security_bitmask(sl_zigbee_extended_security_bitmask_t mask)
{
  return sli_zigbee_stack_set_extended_security_bitmask(mask);
}

void sl_zigbee_set_incoming_tc_link_key_frame_counter(uint32_t frameCounter)
{
  sli_zigbee_stack_set_incoming_tc_link_key_frame_counter(frameCounter);
}

sl_status_t sl_zigbee_set_initial_security_state(sl_zigbee_initial_security_state_t *state)
{
  return sli_zigbee_stack_set_initial_security_state(state);
}

sl_status_t sl_zigbee_set_mfg_security_config(uint32_t magicNumber,
                                              const sl_zigbee_mfg_security_struct_t *settings)
{
  return sli_zigbee_stack_set_mfg_security_config(magicNumber,
                                                  settings);
}

sl_status_t sl_zigbee_set_outgoing_aps_frame_counter(uint32_t desiredValue)
{
  return sli_zigbee_stack_set_outgoing_aps_frame_counter(desiredValue);
}

sl_status_t sl_zigbee_set_outgoing_nwk_frame_counter(uint32_t desiredValue)
{
  return sli_zigbee_stack_set_outgoing_nwk_frame_counter(desiredValue);
}

void sl_zigbee_set_rejoin_mode(sl_zigbee_aps_rejoin_mode_t mode)
{
  sli_zigbee_stack_set_rejoin_mode(mode);
}

void sl_zigbee_set_transient_key_timeout_s(uint16_t seconds)
{
  sli_zigbee_stack_set_transient_key_timeout_s(seconds);
}

sl_status_t sl_zigbee_start_writing_stack_tokens(void)
{
  return sli_zigbee_stack_start_writing_stack_tokens();
}

sl_status_t sl_zigbee_stop_writing_stack_tokens(void)
{
  return sli_zigbee_stack_stop_writing_stack_tokens();
}

sl_status_t sl_zigbee_terminate_app_link_key_request(void)
{
  return sli_zigbee_stack_terminate_app_link_key_request();
}

sl_status_t sl_zigbee_update_app_link_key(sl_802154_long_addr_t partnerEui64)
{
  return sli_zigbee_stack_update_app_link_key(partnerEui64);
}

sl_status_t sl_zigbee_update_tc_link_key(uint8_t maxAttempts)
{
  return sli_zigbee_stack_update_tc_link_key(maxAttempts);
}

sl_status_t sl_zigbee_verify_partner_link_key(sl_802154_short_addr_t shortID,
                                              sl_802154_long_addr_t eui)
{
  return sli_zigbee_stack_verify_partner_link_key(shortID,
                                                  eui);
}

bool sl_zigbee_writing_stack_tokens_enabled(void)
{
  return sli_zigbee_stack_writing_stack_tokens_enabled();
}
