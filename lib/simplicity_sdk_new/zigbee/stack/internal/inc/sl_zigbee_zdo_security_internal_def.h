/***************************************************************************//**
 * @file sl_zigbee_zdo_security_internal_def.h
 * @brief internal names for 'sl_zigbee_zdo_security' declarations
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
// automatically generated from sl_zigbee_zdo_security.h.  Do not manually edit
#ifndef SL_ZIGBEE_ZDO_SECURITY_INTERNAL_DEF_H
#define SL_ZIGBEE_ZDO_SECURITY_INTERNAL_DEF_H

#include "stack/include/sl_zigbee_zdo_security.h"

// Command Indirection

sl_status_t sli_zigbee_stack_get_authentication_level(sl_802154_short_addr_t dest,
                                                      sl_802154_long_addr_t target);

sl_status_t sli_zigbee_stack_get_symmetric_passphrase(sl_802154_long_addr_t eui64,
                                                      sl_802154_short_addr_t short_id,
                                                      uint8_t *passphrase);

sl_status_t sli_zigbee_stack_initiate_security_challenge(sl_802154_short_addr_t partnerNodeId,
                                                         sl_802154_long_addr_t partnerLong,
                                                         uint8_t keyIndex);

void sli_zigbee_stack_retrieve_authentication_token(sl_802154_short_addr_t destination,
                                                    sl_zigbee_aps_option_t options);

// Callback Indirection

void sli_zigbee_stack_get_authentication_level_callback(sl_zigbee_zdo_status_t rsp_status,
                                                        sl_802154_long_addr_t target,
                                                        uint8_t join_method,
                                                        uint8_t link_key_update);

void sli_zigbee_stack_set_authenticaion_level_callback(sl_802154_long_addr_t target,
                                                       sl_zigbee_initial_join_method *initial_join_method,
                                                       sl_zigbee_active_link_key_type *active_link_key_type);

void sli_zigbee_stack_zdo_retrieve_authentication_token_complete_callback(sl_status_t status);

#endif // SL_ZIGBEE_ZDO_SECURITY_INTERNAL_DEF_H
