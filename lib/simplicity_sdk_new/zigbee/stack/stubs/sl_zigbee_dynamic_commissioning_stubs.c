/**
 * @file sl_zigbee_zdo_dlk_negotiation.h
 * @brief declarations of ZDO interface for dynamic link key negotiation
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

#include "include/sl_zigbee.h"
#include "internal/inc/internal-defs-patch.h"
#include "buffer_manager/buffer-management.h"
#include "sl_common.h"
#include "sl_status.h"
#include "include/sl_zigbee_dynamic_commissioning.h"
#include "stack/include/sl_zigbee_stack_specific_tlv.h"

SL_WEAK sl_status_t sli_zigbee_zdo_dlk_handle_start_key_negotiation_rsp(sli_buffer_manager_buffer_t response,
                                                                        uint8_t payload_index,
                                                                        sl_802154_short_addr_t source)
{
  (void)response;
  (void)payload_index;
  (void)source;
  return SL_STATUS_NOT_SUPPORTED;
}

SL_WEAK void sli_zigbee_stack_zdo_dlk_get_supported_negotiation_parameters(uint8_t *method_mask,
                                                                           uint8_t *secret_mask)
{
  (void)method_mask;
  (void)secret_mask;
}

SL_WEAK sl_status_t sli_zigbee_zdo_dlk_handle_start_key_update_req(sli_buffer_manager_buffer_t request,
                                                                   uint8_t payload_index,
                                                                   sl_802154_short_addr_t source,
                                                                   uint8_t sequence)
{
  (void)request;
  (void)payload_index;
  (void)source;
  (void)sequence;
  return SL_STATUS_NOT_SUPPORTED;
}

SL_WEAK sl_status_t sli_zigbee_zdo_dlk_handle_start_key_negotiation_req(sli_buffer_manager_buffer_t request,
                                                                        uint8_t payload_index,
                                                                        sl_802154_short_addr_t source,
                                                                        uint8_t sequence)
{
  (void)request;
  (void)payload_index;
  (void)source;
  (void)sequence;
  return SL_STATUS_NOT_SUPPORTED;
}

SL_WEAK void sli_zigbee_zdo_dlk_node_desc_req_additional_tlvs(sl_802154_short_addr_t target, uint8_t **extra_req_data, uint8_t *data_length)
{
  (void)target;
  (void)extra_req_data;
  (void)data_length;
}

SL_WEAK sl_zigbee_global_tlv_fragmentation_param_t sli_zigbee_stack_global_tlv_fragmentation_param()
{
  sl_zigbee_global_tlv_fragmentation_param_t frag_tlv = { 0 };
  return frag_tlv;
}

SL_WEAK void sli_zigbee_zdo_dlk_node_desc_req_handle_additional_tlvs(sli_buffer_manager_buffer_t request,
                                                                     uint8_t payload_index,
                                                                     sl_802154_short_addr_t source,
                                                                     uint8_t **extra_rsp_data,
                                                                     uint8_t *data_length)
{
  (void)request;
  (void)payload_index;
  (void)source;
  (void)extra_rsp_data;
  (void)data_length;
}

SL_WEAK bool sli_zigbee_zdo_dlk_node_desc_rsp_handle_additional_tlvs(sli_buffer_manager_buffer_t request,
                                                                     uint8_t payload_index,
                                                                     sl_802154_short_addr_t source)
{
  (void)request;
  (void)payload_index;
  (void)source;
  return false;
}

SL_WEAK bool sli_zigbee_stack_zdo_dlk_enabled(void)
{
  return false;
}

SL_WEAK void slx_zigbee_gu_zdo_stack_toggle_dlk(bool do_dlk)
{
  (void) do_dlk;
}
SL_WEAK void sli_zigbee_stack_dynamic_commissioning_alert_callback(sl_zigbee_address_info *ids,
                                                                   sl_zigbee_dynamic_commissioning_event_t event)
{
  (void) ids;
  (void) event;
}

SL_WEAK sl_status_t sli_zigbee_dynamic_commissioning_route_destination(sl_802154_short_addr_t destination,
                                                                       uint8_t *status_return)
{
  UNUSED_VAR(destination);
  UNUSED_VAR(status_return);
  return SL_STATUS_NOT_SUPPORTED;
}

SL_WEAK void sli_zigbee_dynamic_commissioning_device_interview_ready(sl_zigbee_address_info *ids)
{
  UNUSED_VAR(ids);
}

SL_WEAK void sli_zigbee_dynamic_commissioning_refresh_timers(sl_802154_short_addr_t device_short,
                                                             sl_802154_long_addr_t device_long)
{
  UNUSED_VAR(device_short);
  UNUSED_VAR(device_long);
}

SL_WEAK sl_zigbee_join_decision_t sli_zigbee_handle_incoming_dynamic_commissioning_open(sl_zigbee_device_update_t update_event,
                                                                                        sl_802154_short_addr_t joiner_short,
                                                                                        sl_802154_long_addr_t joiner_long,
                                                                                        sl_802154_short_addr_t parent_short,
                                                                                        sl_zigbee_join_decision_t original,
                                                                                        void *joiner_tlvs)
{
  UNUSED_VAR(update_event);
  UNUSED_VAR(joiner_short);
  UNUSED_VAR(joiner_long);
  UNUSED_VAR(parent_short);
  UNUSED_VAR(joiner_tlvs);
  return original;
}
