/***************************************************************************//**
 * @file sl_zigbee_zdo_dlk_negotiation_baremetal_wrapper.c
 * @brief internal implementations for 'sl_zigbee_zdo_dlk_negotiation' as a thin-wrapper
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
// automatically generated from sl_zigbee_zdo_dlk_negotiation.h.  Do not manually edit
#include "stack/include/sl_zigbee_zdo_dlk_negotiation.h"
#include "stack/internal/inc/sl_zigbee_zdo_dlk_negotiation_internal_def.h"

bool sl_zigbee_zdo_dlk_enabled(void)
{
  return sli_zigbee_stack_zdo_dlk_enabled();
}

void sl_zigbee_zdo_dlk_get_supported_negotiation_parameters(sl_zigbee_dlk_supported_negotiation_method *method_mask,
                                                            sl_zigbee_dlk_negotiation_supported_shared_secret_source *secret_mask)
{
  sli_zigbee_stack_zdo_dlk_get_supported_negotiation_parameters(method_mask,
                                                                secret_mask);
}

sl_status_t sl_zigbee_zdo_dlk_start_key_negotiation(sl_zigbee_address_info *partner,
                                                    sl_zigbee_dlk_negotiation_method selected_method,
                                                    sl_zigbee_dlk_negotiation_shared_secret_source selected_secret)
{
  return sli_zigbee_stack_zdo_dlk_start_key_negotiation(partner,
                                                        selected_method,
                                                        selected_secret);
}

sl_status_t sl_zigbee_zdo_dlk_start_key_update(sl_zigbee_address_info *target,
                                               sl_zigbee_dlk_negotiation_method selected_method,
                                               sl_zigbee_dlk_negotiation_shared_secret_source selected_secret)
{
  return sli_zigbee_stack_zdo_dlk_start_key_update(target,
                                                   selected_method,
                                                   selected_secret);
}

bool slx_zigbee_gu_zdo_dlk_mangle_packet(sli_buffer_manager_buffer_t *buffer)
{
  return slxi_zigbee_stack_gu_zdo_dlk_mangle_packet(buffer);
}

bool slx_zigbee_gu_zdo_dlk_override_psk_fetch(uint8_t *key_buffer)
{
  return slxi_zigbee_stack_gu_zdo_dlk_override_psk_fetch(key_buffer);
}

void slx_zigbee_gu_zdo_dlk_override_supported_params(sl_zigbee_dlk_supported_negotiation_method *method_mask,
                                                     sl_zigbee_dlk_negotiation_supported_shared_secret_source *secret_mask)
{
  slxi_zigbee_stack_gu_zdo_dlk_override_supported_params(method_mask,
                                                         secret_mask);
}
