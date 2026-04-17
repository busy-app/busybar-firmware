/***************************************************************************//**
 * @file sl_zigbee_r23_misc_support_stubs.c
 * @brief various non-specific stubs for stack features
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

#include "sl_common.h"
#include "sl_zigbee_types.h"
#include "sl_zigbee_tlv_core.h"
#include "sl_zigbee_types_internal.h"
#include "sl_zigbee_address_info.h"
#include "sl_zigbee_stack_specific_tlv.h"

// refer to sl_router_info_bitmask_fields in sl_zigbee_stack_specific_tlvs.h
SL_WEAK uint16_t sli_zigbee_stack_get_router_info_bitmask(void)
{
  return 0U;
}

SL_WEAK uint16_t sli_zigbee_get_pan_id_conflict_count(void)
{
  return 0U;
}

SL_WEAK sl_status_t sli_zigbee_zdo_handle_rsp_extended(uint16_t cluster_id,
                                                       sl_zigbee_aps_frame_t *apsFrame,
                                                       sli_zigbee_packet_header_t header,
                                                       uint8_t payload_index,
                                                       sl_802154_short_addr_t source)
{
  (void)apsFrame;
  UNUSED_VAR(cluster_id);
  UNUSED_VAR(header);
  UNUSED_VAR(payload_index);
  UNUSED_VAR(source);
  return SL_STATUS_NOT_SUPPORTED;
}

SL_WEAK sl_status_t sli_zigbee_zdo_handle_req_extended(uint16_t cluster_id,
                                                       sli_zigbee_packet_header_t header,
                                                       uint8_t payload_index,
                                                       sl_802154_short_addr_t source,
                                                       bool broadcast_request,
                                                       sl_zigbee_aps_option_t options,
                                                       uint8_t sequence)
{
  (void)broadcast_request;
  UNUSED_VAR(cluster_id);
  UNUSED_VAR(header);
  UNUSED_VAR(payload_index);
  UNUSED_VAR(source);
  UNUSED_VAR(options);
  UNUSED_VAR(sequence);
  return SL_STATUS_NOT_SUPPORTED;
}

SL_WEAK sl_status_t sli_zigbee_zdo_handle_extended_rsp(uint16_t cluster_id,
                                                       sli_zigbee_packet_header_t header,
                                                       uint8_t payload_index,
                                                       sl_802154_short_addr_t source)
{
  (void)cluster_id;
  (void)header;
  (void)payload_index;
  (void)source;
  return SL_STATUS_NOT_SUPPORTED;
}

SL_WEAK sl_status_t sli_zigbee_zdo_handle_extended_req(uint16_t cluster_id,
                                                       sli_zigbee_packet_header_t header,
                                                       uint8_t payload_index,
                                                       sl_802154_short_addr_t source,
                                                       sl_zigbee_aps_option_t options)

{
  (void)cluster_id;
  (void)header;
  (void)payload_index;
  (void)source;
  (void)options;
  return SL_STATUS_NOT_SUPPORTED;
}

SL_WEAK bool sli_zigbee_get_global_fc_sync_flag(void)
{
  return false;
}

SL_WEAK sl_status_t sli_zigbee_zdo_generate_security_challenge_req(sl_802154_short_addr_t destShort,
                                                                   sl_zigbee_key_data_t* keyToChallenge)

{
  (void)destShort;
  (void)keyToChallenge;
  return SL_STATUS_NOT_SUPPORTED;
}

SL_WEAK sl_status_t sli_zigbee_set_aps_fc_sync_required(void)
{
  return SL_STATUS_NOT_SUPPORTED;
}

SL_WEAK sl_status_t sl_zigbee_tlv_initialize_chain(sl_zigbee_tlv_chain *chain,
                                                   uint8_t *memory,
                                                   const size_t chain_length,
                                                   const size_t memory_length)
{
  UNUSED_VAR(chain);
  UNUSED_VAR(memory);
  UNUSED_VAR(chain_length);
  UNUSED_VAR(memory_length);
  return SL_STATUS_NOT_SUPPORTED;
}

SL_WEAK uint16_t sl_zigbee_tlv_serial_length(const sl_zigbee_tlv_t *tlv)
{
  UNUSED_VAR(tlv);
  return 0;
}

SL_WEAK bool sli_zigbee_process_network_commissioning_messages(uint8_t firstNetworkPayloadByte,
                                                               sli_zigbee_packet_header_t header,
                                                               uint8_t* frame,
                                                               uint8_t network_payload_index)
{
  (void)frame;
  (void)firstNetworkPayloadByte;
  (void)header;
  (void)network_payload_index;
  return false;
}

WEAK(bool sli_zigbee_prioritize_on_parent_selection(sl_zigbee_beacon_data_t storedBeacon, sl_zigbee_beacon_data_t rxBeacon))
{
  (void)storedBeacon;
  (void)rxBeacon;
  return false;
}

WEAK(bool sli_zigbee_append_beacon_tlvs(sli_zigbee_packet_header_t* beacon))
{
  (void)beacon;
  return false;
}

WEAK(void sli_zigbee_parse_beacon_tlvs(sli_zigbee_packet_header_t beacon,
                                       uint8_t tlvPayloadStartIndex,
                                       sl_zigbee_beacon_data_t* beaconData))
{
  (void)beacon;
  (void)tlvPayloadStartIndex;
  (void)beaconData;
}

WEAK(void sli_zigbee_update_function_and_state_for_network_commissioining(bool extended_beacon,
                                                                          sli_zigbee_join_state_t* joinState,
                                                                          sli_zigbee_network_request_t* network_req_fn))
{
  (void)extended_beacon;
  (void)network_req_fn;
  (void)joinState;
}

WEAK(void sli_zigbee_update_rejoin_state_for_network_commissioining(bool extended_beacon,
                                                                    sli_zigbee_join_state_t* joinState))
{
  (void)extended_beacon;
  (void)joinState;
}

SL_WEAK bool sli_zigbee_aps_relay_frame_required(sl_802154_short_addr_t dest_short_id)
{
  UNUSED_VAR(dest_short_id);
  return false;
}

SL_WEAK sl_status_t sli_zigbee_reconstruct_as_relay_packet(sli_zigbee_packet_header_t original_packet,
                                                           sl_802154_short_addr_t dest_short,
                                                           sl_802154_long_addr_t dest_long,
                                                           sli_zigbee_packet_header_t *header_result)
{
  UNUSED_VAR(original_packet);
  UNUSED_VAR(dest_short);
  UNUSED_VAR(dest_long);
  UNUSED_VAR(header_result);
  return SL_STATUS_NOT_SUPPORTED;
}

SL_WEAK void sli_zigbee_handle_aps_relay_command(sli_zigbee_packet_header_t nwk_header)
{
  UNUSED_VAR(nwk_header);
}

// The stub when R23 APS relay is not present
SL_WEAK sl_status_t sli_zigbee_aps_relay_check_and_add_relay_if_required(sli_zigbee_packet_header_t *header)
{
  (void)header;
  // always return false if AF Support component is not present
  return SL_STATUS_OK;
}

SL_WEAK void sli_zigbee_clone_packet_header_for_relay_frames(sli_zigbee_packet_header_t *original, sli_zigbee_packet_header_t *header)
{
  (void)original;
  (void)header;
}

SL_WEAK sl_status_t sli_zigbee_stack_initiate_security_challenge(sl_802154_short_addr_t partnerNodeId,
                                                                 sl_802154_long_addr_t partnerLong,
                                                                 uint8_t keyIndex)
{
  (void)partnerNodeId;
  (void)partnerLong;
  (void)keyIndex;
  // always return false if AF Support component is not present
  return SL_STATUS_OK;
}

SL_WEAK sl_status_t sli_zigbee_zdo_retry_with_fragmentation_for_new_requests(uint8_t mode,
                                                                             sl_802154_short_addr_t destination,
                                                                             sl_zigbee_aps_frame_t *zigDevApsStruct,
                                                                             sli_buffer_manager_buffer_t* message)
{
  (void)mode;
  (void)destination;
  (void)zigDevApsStruct;
  (void)message;
  return SL_STATUS_FAIL;
}

SL_WEAK sli_zigbee_packet_header_t sli_zigbee_aps_relay_maybe_reconstruct_command(sli_zigbee_packet_header_t header,
                                                                                  bool use_tunnel,
                                                                                  uint8_t aps_command,
                                                                                  sl_802154_short_addr_t destination_short,
                                                                                  sl_802154_long_addr_t destination_long)
{
  UNUSED_VAR(use_tunnel);
  UNUSED_VAR(aps_command);
  UNUSED_VAR(destination_short);
  UNUSED_VAR(destination_long);
  return header;
}

SL_WEAK sl_status_t sli_zigbee_get_device_capability_extension_data(bool* attach_tlv, uint16_t* bitmask)
{
  (void)attach_tlv;
  (void)bitmask;

  return SL_STATUS_NOT_SUPPORTED;
}

SL_WEAK bool  sli_zigbee_zdo_check_and_handle_fragmented_messages(uint8_t* apsFrameControl,
                                                                  sl_zigbee_aps_frame_t *requestApsFrame,
                                                                  sli_zigbee_packet_header_t* header)
{
  (void)apsFrameControl;
  (void)requestApsFrame;
  (void)header;
  return false;
}

SL_WEAK sl_status_t sli_zigbee_zdo_generate_get_authentication_level_req(sl_802154_short_addr_t dest,
                                                                         sl_zigbee_aps_option_t aps_options,
                                                                         sl_802154_long_addr_t target)
{
  (void)dest;
  (void)aps_options;
  (void)target;
  return SL_STATUS_NOT_SUPPORTED;
}

SL_WEAK sl_zigbee_global_tlv_fragmentation_param_t sli_zigbee_stack_global_tlv_fragmentation_param(void)
{
  sl_zigbee_global_tlv_fragmentation_param_t tlv;
  memset(&tlv, 0xAB, sizeof(tlv));
  return tlv;
}

SL_WEAK void sli_zdo_dlk_aps_ack_received_for_key_update_req(uint16_t short_address)
{
  (void) short_address;
}

void sli_zigbee_stack_parse_permit_joining_beacon_appendix(uint8_t *payload,
                                                           uint8_t payload_len)
{
  (void) payload;
  (void) payload_len;
}
