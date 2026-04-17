/***************************************************************************//**
 * @file sli_zigbee_zdo_dlk_negotiation.h
 * @brief internal declarations of ZDO interface for dynamic link key negotiation
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

#ifndef SLI_ZIGBEE_ZDO_DLK_NEGOTIATION_H
#define SLI_ZIGBEE_ZDO_DLK_NEGOTIATION_H

#include "stack/include/sl_zigbee_zdo_dlk_negotiation.h"
#include "stack/security/sli_zigbee_dlk_negotiation.h"

// local TLV definitions
#define SLI_ZIGBEE_ZDO_DLK_TLV_SELECTED_NEGOTIATION_PARAMS_TAG_ID 0
#define SLI_ZIGBEE_ZDO_DLK_TLV_SELECTED_NEGOTIATION_PARAMS_MIN_LEN 10
#define SLI_ZIGBEE_ZDO_DLK_TLV_SELECTED_NEGOTIATION_PARAMS_MAX_LEN 10
// NOTE tlv format
// selected protocol (1) |> selected secret (1) |> device eui (8)
#define SLI_ZIGBEE_ZDO_DLK_TLV_SELECTED_PARAMS_METHOD_INDEX 0
#define SLI_ZIGBEE_ZDO_DLK_TLV_SELECTED_PARAMS_SECRET_INDEX 1
#define SLI_ZIGBEE_ZDO_DLK_TLV_SELECTED_PARAMS_ID_INDEX 2

typedef CONCRETE_TLV_DEFINE (SLI_ZIGBEE_ZDO_DLK_TLV_SELECTED_NEGOTIATION_PARAMS_MAX_LEN)
sli_zigbee_zdo_dlk_selected_key_negotiation_parameters_tlv_t;

// this is used in both the Start Key Negotiation Start Request and Response
// NOTE tlv format
// sender EUI (8) |> public key (32)
#define SLI_ZIGBEE_ZDO_DLK_TLV_PUBLIC_KEY_C25519_TAG_ID 0
#define SLI_ZIGBEE_ZDO_DLK_TLV_PUBLIC_KEY_C25519_MIN_LEN (EUI64_SIZE + DLK_ECC_CURVE25519_PUBLIC_KEY_SIZE)
#define SLI_ZIGBEE_ZDO_DLK_TLV_PUBLIC_KEY_C25519_MAX_LEN (EUI64_SIZE + DLK_ECC_CURVE25519_PUBLIC_KEY_SIZE)

#define SLI_ZIGBEE_ZDO_DLK_TLV_PUBLIC_KEY_EUI64_INDEX 0
#define SLI_ZIGBEE_ZDO_DLK_TLV_PUBLIC_KEY_ECC_COORD_INDEX EUI64_SIZE

// NOTE we could make use of the same struct for both public point tlvs
typedef CONCRETE_TLV_DEFINE (SLI_ZIGBEE_ZDO_DLK_TLV_PUBLIC_KEY_C25519_MAX_LEN)
sli_zigbee_zdo_dlk_public_key_c25519_tlv_t;

// internal apis, no state checks, generates and handles zdo frames
// ZDO start key update service handlers
sl_status_t sli_zigbee_zdo_dlk_generate_start_key_update_req(sl_zigbee_address_info *target,
                                                             sl_zigbee_dlk_negotiation_method selected_method,
                                                             sl_zigbee_dlk_negotiation_shared_secret_source selected_secret);
sl_status_t sli_zigbee_zdo_dlk_handle_start_key_update_req(sli_buffer_manager_buffer_t request,
                                                           uint8_t payload_index,
                                                           sl_802154_short_addr_t source,
                                                           uint8_t sequence);
sl_status_t sli_zigbee_zdo_dlk_handle_start_key_update_rsp(sli_buffer_manager_buffer_t response,
                                                           uint8_t payload_index,
                                                           sl_802154_short_addr_t source);
// ZDO start key negotiation service handlers
sl_status_t sli_zigbee_zdo_dlk_generate_start_key_negotiation_req(sl_802154_short_addr_t target,
                                                                  sl_zigbee_aps_option_t send_options,
                                                                  sl_zigbee_tlv_chain *tlv_payload);
sl_status_t sli_zigbee_zdo_dlk_handle_start_key_negotiation_req(sli_buffer_manager_buffer_t request,
                                                                uint8_t payload_index,
                                                                sl_802154_short_addr_t source,
                                                                uint8_t sequence);
sl_status_t sli_zigbee_zdo_dlk_handle_start_key_negotiation_rsp(sli_buffer_manager_buffer_t response,
                                                                uint8_t payload_index,
                                                                sl_802154_short_addr_t source);
#endif // SLI_ZIGBEE_ZDO_DLK_NEGOTIATION_H
