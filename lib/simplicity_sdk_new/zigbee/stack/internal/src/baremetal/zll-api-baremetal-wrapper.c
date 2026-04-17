/***************************************************************************//**
 * @file zll-api-baremetal-wrapper.c
 * @brief internal implementations for 'zll-api' as a thin-wrapper
 *******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
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
// automatically generated from zll-api.h.  Do not manually edit
#include "stack/include/zll-api.h"
#include "stack/internal/inc/zll-api-internal-def.h"

uint32_t sl_zigbee_get_zll_primary_channel_mask(void)
{
  return sli_zigbee_stack_get_zll_primary_channel_mask();
}

uint32_t sl_zigbee_get_zll_secondary_channel_mask(void)
{
  return sli_zigbee_stack_get_zll_secondary_channel_mask();
}

bool sl_zigbee_is_zll_network(void)
{
  return sli_zigbee_stack_is_zll_network();
}

void sl_zigbee_set_zll_additional_state(uint16_t mask)
{
  sli_zigbee_stack_set_zll_additional_state(mask);
}

void sl_zigbee_set_zll_node_type(sl_zigbee_node_type_t nodeType)
{
  sli_zigbee_stack_set_zll_node_type(nodeType);
}

void sl_zigbee_set_zll_primary_channel_mask(uint32_t mask)
{
  sli_zigbee_stack_set_zll_primary_channel_mask(mask);
}

void sl_zigbee_set_zll_secondary_channel_mask(uint32_t mask)
{
  sli_zigbee_stack_set_zll_secondary_channel_mask(mask);
}

void sl_zigbee_zll_clear_tokens(void)
{
  sli_zigbee_stack_zll_clear_tokens();
}

sl_status_t sl_zigbee_zll_form_network(sl_zigbee_zll_network_t *networkInfo,
                                       int8_t radioTxPower)
{
  return sli_zigbee_stack_zll_form_network(networkInfo,
                                           radioTxPower);
}

sl_zigbee_zll_policy_t sl_zigbee_zll_get_policy(void)
{
  return sli_zigbee_stack_zll_get_policy();
}

uint8_t sl_zigbee_zll_get_radio_idle_mode(void)
{
  return sli_zigbee_stack_zll_get_radio_idle_mode();
}

void sl_zigbee_zll_get_token_stack_zll_data(sl_zigbee_tok_type_stack_zll_data_t *token)
{
  sli_zigbee_stack_zll_get_token_stack_zll_data(token);
}

void sl_zigbee_zll_get_token_stack_zll_security(sl_zigbee_tok_type_stack_zll_security_t *token)
{
  sli_zigbee_stack_zll_get_token_stack_zll_security(token);
}

void sl_zigbee_zll_get_tokens_stack_zll(sl_zigbee_tok_type_stack_zll_data_t *data,
                                        sl_zigbee_tok_type_stack_zll_security_t *security)
{
  sli_zigbee_stack_zll_get_tokens_stack_zll(data,
                                            security);
}

sl_status_t sl_zigbee_zll_join_target(const sl_zigbee_zll_network_t *targetNetworkInfo)
{
  return sli_zigbee_stack_zll_join_target(targetNetworkInfo);
}

bool sl_zigbee_zll_operation_in_progress(void)
{
  return sli_zigbee_stack_zll_operation_in_progress();
}

bool sl_zigbee_zll_rx_on_when_idle_get_active(void)
{
  return sli_zigbee_stack_zll_rx_on_when_idle_get_active();
}

void sl_zigbee_zll_scanning_complete(void)
{
  sli_zigbee_stack_zll_scanning_complete();
}

sl_status_t sl_zigbee_zll_set_initial_security_state(const sl_zigbee_key_data_t *networkKey,
                                                     const sl_zigbee_zll_initial_security_state_t *securityState)
{
  return sli_zigbee_stack_zll_set_initial_security_state(networkKey,
                                                         securityState);
}

void sl_zigbee_zll_set_non_zll_network(void)
{
  sli_zigbee_stack_zll_set_non_zll_network();
}

sl_status_t sl_zigbee_zll_set_policy(sl_zigbee_zll_policy_t policy)
{
  return sli_zigbee_stack_zll_set_policy(policy);
}

void sl_zigbee_zll_set_radio_idle_mode(sl_zigbee_radio_power_mode_t mode)
{
  sli_zigbee_stack_zll_set_radio_idle_mode(mode);
}

sl_status_t sl_zigbee_zll_set_rx_on_when_idle(uint32_t durationMs)
{
  return sli_zigbee_stack_zll_set_rx_on_when_idle(durationMs);
}

sl_status_t sl_zigbee_zll_set_security_state_without_key(const sl_zigbee_zll_initial_security_state_t *securityState)
{
  return sli_zigbee_stack_zll_set_security_state_without_key(securityState);
}

void sl_zigbee_zll_set_token_stack_zll_data(sl_zigbee_tok_type_stack_zll_data_t *token)
{
  sli_zigbee_stack_zll_set_token_stack_zll_data(token);
}

sl_status_t sl_zigbee_zll_start_scan(uint32_t channelMask,
                                     int8_t radioPowerForScan,
                                     sl_zigbee_node_type_t nodeType)
{
  return sli_zigbee_stack_zll_start_scan(channelMask,
                                         radioPowerForScan,
                                         nodeType);
}
