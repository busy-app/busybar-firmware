/***************************************************************************//**
 * @file sl_zigbee_stack_specific_tlv_ipc_command_messages.h
 * @brief defines structured format for 'sl_zigbee_stack_specific_tlv' ipc messages
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
#ifndef SL_ZIGBEE_STACK_SPECIFIC_TLV_IPC_COMMAND_MESSAGES_H
#define SL_ZIGBEE_STACK_SPECIFIC_TLV_IPC_COMMAND_MESSAGES_H

#include "stack/include/sl_zigbee_stack_specific_tlv.h"
#include "stack/internal/inc/sl_zigbee_stack_specific_tlv_internal_def.h"

typedef struct {
  sli_buffer_manager_buffer_t buffer;
  uint16_t index;
  uint8_t tag_c;
  uint8_t tag_v[10];
} sli_zigbee_stack_global_tlv_add_configurations_ipc_req_t;

typedef struct {
  sl_status_t result;
} sli_zigbee_stack_global_tlv_add_configurations_ipc_rsp_t;

typedef struct {
  sli_zigbee_stack_global_tlv_add_configurations_ipc_req_t request;
  sli_zigbee_stack_global_tlv_add_configurations_ipc_rsp_t response;
} sli_zigbee_stack_global_tlv_add_configurations_ipc_msg_t;

typedef struct {
  sl_zigbee_global_tlv_beacon_appendix_encap_t encapTlv;
} sli_zigbee_stack_global_tlv_beacon_appendix_encapsulation_ipc_req_t;

typedef struct {
  sl_status_t result;
} sli_zigbee_stack_global_tlv_beacon_appendix_encapsulation_ipc_rsp_t;

typedef struct {
  sli_zigbee_stack_global_tlv_beacon_appendix_encapsulation_ipc_req_t request;
  sli_zigbee_stack_global_tlv_beacon_appendix_encapsulation_ipc_rsp_t response;
} sli_zigbee_stack_global_tlv_beacon_appendix_encapsulation_ipc_msg_t;

typedef struct {
  uint16_t mask;
} sli_zigbee_stack_global_tlv_configuration_params_from_bitmask_ipc_req_t;

typedef struct {
  sl_zigbee_global_tlv_configuration_params_t result;
} sli_zigbee_stack_global_tlv_configuration_params_from_bitmask_ipc_rsp_t;

typedef struct {
  sli_zigbee_stack_global_tlv_configuration_params_from_bitmask_ipc_req_t request;
  sli_zigbee_stack_global_tlv_configuration_params_from_bitmask_ipc_rsp_t response;
} sli_zigbee_stack_global_tlv_configuration_params_from_bitmask_ipc_msg_t;

typedef struct {
  sl_zigbee_global_tlv_configuration_params_t tlv;
} sli_zigbee_stack_global_tlv_configuration_params_get_bitmask_ipc_req_t;

typedef struct {
  uint16_t result;
} sli_zigbee_stack_global_tlv_configuration_params_get_bitmask_ipc_rsp_t;

typedef struct {
  sli_zigbee_stack_global_tlv_configuration_params_get_bitmask_ipc_req_t request;
  sli_zigbee_stack_global_tlv_configuration_params_get_bitmask_ipc_rsp_t response;
} sli_zigbee_stack_global_tlv_configuration_params_get_bitmask_ipc_msg_t;

typedef struct {
  uint16_t bitmask;
} sli_zigbee_stack_global_tlv_device_capability_extension_param_ipc_req_t;

typedef struct {
  sl_zigbee_global_tlv_device_capability_extension_t result;
} sli_zigbee_stack_global_tlv_device_capability_extension_param_ipc_rsp_t;

typedef struct {
  sli_zigbee_stack_global_tlv_device_capability_extension_param_ipc_req_t request;
  sli_zigbee_stack_global_tlv_device_capability_extension_param_ipc_rsp_t response;
} sli_zigbee_stack_global_tlv_device_capability_extension_param_ipc_msg_t;

typedef struct {
  sl_zigbee_global_tlv_fragmentation_param_t result;
} sli_zigbee_stack_global_tlv_fragmentation_param_ipc_rsp_t;

typedef struct {
  sli_zigbee_stack_global_tlv_fragmentation_param_ipc_rsp_t response;
} sli_zigbee_stack_global_tlv_fragmentation_param_ipc_msg_t;

typedef struct {
  uint8_t tag_c;
  uint8_t tag_v[10];
} sli_zigbee_stack_global_tlv_get_configurations_ipc_req_t;

typedef struct {
  sli_buffer_manager_buffer_t result;
} sli_zigbee_stack_global_tlv_get_configurations_ipc_rsp_t;

typedef struct {
  sli_zigbee_stack_global_tlv_get_configurations_ipc_req_t request;
  sli_zigbee_stack_global_tlv_get_configurations_ipc_rsp_t response;
} sli_zigbee_stack_global_tlv_get_configurations_ipc_msg_t;

typedef struct {
  sl_zigbee_global_tlv_joiner_encap_t encapTlv;
} sli_zigbee_stack_global_tlv_joiner_encapsulation_ipc_req_t;

typedef struct {
  sl_status_t result;
} sli_zigbee_stack_global_tlv_joiner_encapsulation_ipc_rsp_t;

typedef struct {
  sli_zigbee_stack_global_tlv_joiner_encapsulation_ipc_req_t request;
  sli_zigbee_stack_global_tlv_joiner_encapsulation_ipc_rsp_t response;
} sli_zigbee_stack_global_tlv_joiner_encapsulation_ipc_msg_t;

typedef struct {
  sl_zigbee_global_tlv_next_channel_change_t tlv;
} sli_zigbee_stack_global_tlv_next_channel_change_get_bitmask_ipc_req_t;

typedef struct {
  uint32_t result;
} sli_zigbee_stack_global_tlv_next_channel_change_get_bitmask_ipc_rsp_t;

typedef struct {
  sli_zigbee_stack_global_tlv_next_channel_change_get_bitmask_ipc_req_t request;
  sli_zigbee_stack_global_tlv_next_channel_change_get_bitmask_ipc_rsp_t response;
} sli_zigbee_stack_global_tlv_next_channel_change_get_bitmask_ipc_msg_t;

typedef struct {
  uint8_t page;
  uint8_t channel;
} sli_zigbee_stack_global_tlv_next_channel_from_pg_ch_ipc_req_t;

typedef struct {
  sl_zigbee_global_tlv_next_channel_change_t result;
} sli_zigbee_stack_global_tlv_next_channel_from_pg_ch_ipc_rsp_t;

typedef struct {
  sli_zigbee_stack_global_tlv_next_channel_from_pg_ch_ipc_req_t request;
  sli_zigbee_stack_global_tlv_next_channel_from_pg_ch_ipc_rsp_t response;
} sli_zigbee_stack_global_tlv_next_channel_from_pg_ch_ipc_msg_t;

typedef struct {
  sl_802154_pan_id_t pan_id;
} sli_zigbee_stack_global_tlv_next_pan_from_pan_ipc_req_t;

typedef struct {
  sl_zigbee_global_tlv_next_pan_id_t result;
} sli_zigbee_stack_global_tlv_next_pan_from_pan_ipc_rsp_t;

typedef struct {
  sli_zigbee_stack_global_tlv_next_pan_from_pan_ipc_req_t request;
  sli_zigbee_stack_global_tlv_next_pan_from_pan_ipc_rsp_t response;
} sli_zigbee_stack_global_tlv_next_pan_from_pan_ipc_msg_t;

typedef struct {
  sl_zigbee_global_tlv_next_pan_id_t tlv;
} sli_zigbee_stack_global_tlv_next_pan_get_pan_ipc_req_t;

typedef struct {
  sl_802154_pan_id_t result;
} sli_zigbee_stack_global_tlv_next_pan_get_pan_ipc_rsp_t;

typedef struct {
  sli_zigbee_stack_global_tlv_next_pan_get_pan_ipc_req_t request;
  sli_zigbee_stack_global_tlv_next_pan_get_pan_ipc_rsp_t response;
} sli_zigbee_stack_global_tlv_next_pan_get_pan_ipc_msg_t;

typedef struct {
  sl_zigbee_global_tlv_pan_id_conflict_t result;
} sli_zigbee_stack_global_tlv_pan_id_conflict_ipc_rsp_t;

typedef struct {
  sli_zigbee_stack_global_tlv_pan_id_conflict_ipc_rsp_t response;
} sli_zigbee_stack_global_tlv_pan_id_conflict_ipc_msg_t;

typedef struct {
  sl_zigbee_global_tlv_router_information_t result;
} sli_zigbee_stack_global_tlv_router_information_ipc_rsp_t;

typedef struct {
  sli_zigbee_stack_global_tlv_router_information_ipc_rsp_t response;
} sli_zigbee_stack_global_tlv_router_information_ipc_msg_t;

typedef struct {
  sl_zigbee_global_tlv_router_information_t tlv;
} sli_zigbee_stack_global_tlv_router_information_get_bitmask_ipc_req_t;

typedef struct {
  uint16_t result;
} sli_zigbee_stack_global_tlv_router_information_get_bitmask_ipc_rsp_t;

typedef struct {
  sli_zigbee_stack_global_tlv_router_information_get_bitmask_ipc_req_t request;
  sli_zigbee_stack_global_tlv_router_information_get_bitmask_ipc_rsp_t response;
} sli_zigbee_stack_global_tlv_router_information_get_bitmask_ipc_msg_t;

typedef struct {
  sl_zigbee_global_tlv_supp_key_negotiation_t result;
} sli_zigbee_stack_global_tlv_supp_key_negotiation_ipc_rsp_t;

typedef struct {
  sli_zigbee_stack_global_tlv_supp_key_negotiation_ipc_rsp_t response;
} sli_zigbee_stack_global_tlv_supp_key_negotiation_ipc_msg_t;

typedef struct {
  uint8_t passphrase;
} sli_zigbee_stack_global_tlv_symmetric_passphrase_ipc_req_t;

typedef struct {
  sl_zigbee_global_tlv_symmetric_passphrase_t result;
} sli_zigbee_stack_global_tlv_symmetric_passphrase_ipc_rsp_t;

typedef struct {
  sli_zigbee_stack_global_tlv_symmetric_passphrase_ipc_req_t request;
  sli_zigbee_stack_global_tlv_symmetric_passphrase_ipc_rsp_t response;
} sli_zigbee_stack_global_tlv_symmetric_passphrase_ipc_msg_t;

#endif // SL_ZIGBEE_STACK_SPECIFIC_TLV_IPC_COMMAND_MESSAGES_H
