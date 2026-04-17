/***************************************************************************//**
 * @file sl_zigbee_stack_specific_tlv_internal_def.h
 * @brief internal names for 'sl_zigbee_stack_specific_tlv' declarations
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
// automatically generated from sl_zigbee_stack_specific_tlv.h.  Do not manually edit
#ifndef SL_ZIGBEE_STACK_SPECIFIC_TLV_INTERNAL_DEF_H
#define SL_ZIGBEE_STACK_SPECIFIC_TLV_INTERNAL_DEF_H

#include "stack/include/sl_zigbee_stack_specific_tlv.h"

// Command Indirection

sl_status_t sli_zigbee_stack_global_tlv_add_configurations(sli_buffer_manager_buffer_t *buffer,
                                                           uint16_t index,
                                                           uint8_t tag_c,
                                                           const uint8_t *tag_v);

sl_status_t sli_zigbee_stack_global_tlv_beacon_appendix_encapsulation(sl_zigbee_global_tlv_beacon_appendix_encap_t *encapTlv);

sl_zigbee_global_tlv_configuration_params_t sli_zigbee_stack_global_tlv_configuration_params_from_bitmask(uint16_t mask);

uint16_t sli_zigbee_stack_global_tlv_configuration_params_get_bitmask(sl_zigbee_global_tlv_configuration_params_t *tlv);

sl_zigbee_global_tlv_device_capability_extension_t sli_zigbee_stack_global_tlv_device_capability_extension_param(uint16_t bitmask);

sl_zigbee_global_tlv_fragmentation_param_t sli_zigbee_stack_global_tlv_fragmentation_param(void);

sli_buffer_manager_buffer_t sli_zigbee_stack_global_tlv_get_configurations(uint8_t tag_c,
                                                                           const uint8_t *tag_v);

sl_status_t sli_zigbee_stack_global_tlv_joiner_encapsulation(sl_zigbee_global_tlv_joiner_encap_t *encapTlv);

uint32_t sli_zigbee_stack_global_tlv_next_channel_change_get_bitmask(sl_zigbee_global_tlv_next_channel_change_t *tlv);

sl_zigbee_global_tlv_next_channel_change_t sli_zigbee_stack_global_tlv_next_channel_from_pg_ch(uint8_t page,
                                                                                               uint8_t channel);

sl_zigbee_global_tlv_next_pan_id_t sli_zigbee_stack_global_tlv_next_pan_from_pan(sl_802154_pan_id_t pan_id);

sl_802154_pan_id_t sli_zigbee_stack_global_tlv_next_pan_get_pan(sl_zigbee_global_tlv_next_pan_id_t *tlv);

sl_zigbee_global_tlv_pan_id_conflict_t sli_zigbee_stack_global_tlv_pan_id_conflict(void);

sl_zigbee_global_tlv_router_information_t sli_zigbee_stack_global_tlv_router_information(void);

uint16_t sli_zigbee_stack_global_tlv_router_information_get_bitmask(sl_zigbee_global_tlv_router_information_t *tlv);

sl_zigbee_global_tlv_supp_key_negotiation_t sli_zigbee_stack_global_tlv_supp_key_negotiation(void);

sl_zigbee_global_tlv_symmetric_passphrase_t sli_zigbee_stack_global_tlv_symmetric_passphrase(const uint8_t *passphrase);

#endif // SL_ZIGBEE_STACK_SPECIFIC_TLV_INTERNAL_DEF_H
