/***************************************************************************//**
 * @file sl_zigbee_zdo_dlk_negotiation_internal_def.h
 * @brief internal names for 'sl_zigbee_zdo_dlk_negotiation' declarations
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
#ifndef SL_ZIGBEE_ZDO_DLK_NEGOTIATION_INTERNAL_DEF_H
#define SL_ZIGBEE_ZDO_DLK_NEGOTIATION_INTERNAL_DEF_H

#include "stack/include/sl_zigbee_zdo_dlk_negotiation.h"

// Command Indirection

bool sli_zigbee_stack_zdo_dlk_enabled(void);

void sli_zigbee_stack_zdo_dlk_get_supported_negotiation_parameters(sl_zigbee_dlk_supported_negotiation_method *method_mask,
                                                                   sl_zigbee_dlk_negotiation_supported_shared_secret_source *secret_mask);

sl_status_t sli_zigbee_stack_zdo_dlk_start_key_negotiation(sl_zigbee_address_info *partner,
                                                           sl_zigbee_dlk_negotiation_method selected_method,
                                                           sl_zigbee_dlk_negotiation_shared_secret_source selected_secret);

sl_status_t sli_zigbee_stack_zdo_dlk_start_key_update(sl_zigbee_address_info *target,
                                                      sl_zigbee_dlk_negotiation_method selected_method,
                                                      sl_zigbee_dlk_negotiation_shared_secret_source selected_secret);

bool slxi_zigbee_stack_gu_zdo_dlk_mangle_packet(sli_buffer_manager_buffer_t *buffer);

bool slxi_zigbee_stack_gu_zdo_dlk_override_psk_fetch(uint8_t *key_buffer);

void slxi_zigbee_stack_gu_zdo_dlk_override_supported_params(sl_zigbee_dlk_supported_negotiation_method *method_mask,
                                                            sl_zigbee_dlk_negotiation_supported_shared_secret_source *secret_mask);

#endif // SL_ZIGBEE_ZDO_DLK_NEGOTIATION_INTERNAL_DEF_H
