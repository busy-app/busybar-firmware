/***************************************************************************//**
 * @file zigbee_direct_session_key_negotiation.h
 * @brief header definitions for Zigbee Direct Session Key
 *        Negotiation procedure
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

#ifndef SILABS_ZIGBEE_DIRECT_SESSION_KEY_NEGOTIATION_H
#define SILABS_ZIGBEE_DIRECT_SESSION_KEY_NEGOTIATION_H

#include "stack/include/sl_zigbee_security_manager_dlk_ecc.h"

/**
 * @brief selected key negotiation method
 */
typedef enum {
  DLK_ECC_NONE = 0,
  DLK_ECC_CURVE25519_AES_MMO_128 = 1,
  DLK_ECC_CURVE25519_SHA_256 = 2,
  DLK_ECC_P256_SHA_256 = 3,
} sli_dlk_ecc_selected_method;

// Local TLV Defines
#define SL_ZIGBEE_DIRECT_DLK_TLV_SELECTED_NEGOTIATION_METHOD_TAG_ID 0
#define SL_ZIGBEE_DIRECT_DLK_TLV_SELECTED_NEGOTIATION_METHOD_MIN_LEN 1
#define SL_ZIGBEE_DIRECT_DLK_TLV_SELECTED_NEGOTIATION_METHOD_MAX_LEN 2

#define SL_ZIGBEE_DIRECT_DLK_TLV_NWK_KEY_SEQ_NO_TAG_ID 3
#define SL_ZIGBEE_DIRECT_DLK_TLV_NWK_KEY_SEQ_NO_MIN_LEN 1
#define SL_ZIGBEE_DIRECT_DLK_TLV_NWK_KEY_SEQ_NO_MAX_LEN 1

#define SL_ZIGBEE_DIRECT_DLK_TLV_PUBLIC_KEY_P256_MIN_LEN DLK_ECC_P256_PUBLIC_KEY_SIZE
#define SL_ZIGBEE_DIRECT_DLK_TLV_PUBLIC_KEY_P256_MAX_LEN (EUI64_SIZE + DLK_ECC_P256_PUBLIC_KEY_SIZE)

#define SL_ZIGBEE_DIRECT_DLK_TLV_PUBLIC_KEY_C25519_MIN_LEN DLK_ECC_COORDINATE_SIZE
#define SL_ZIGBEE_DIRECT_DLK_TLV_PUBLIC_KEY_C25519_MAX_LEN (EUI64_SIZE + DLK_ECC_COORDINATE_SIZE)

typedef CONCRETE_TLV_DEFINE (SL_ZIGBEE_DIRECT_DLK_TLV_NWK_KEY_SEQ_NO_MAX_LEN) sl_zigbee_direct_dlk_nwk_seq_no_tlv_t;
typedef CONCRETE_TLV_DEFINE (SL_ZIGBEE_DIRECT_DLK_TLV_SELECTED_NEGOTIATION_METHOD_MAX_LEN) sl_zigbee_direct_dlk_selected_negotiation_method_tlv_t;
// we can make use of the same struct for both public point tlvs
typedef CONCRETE_TLV_DEFINE (SL_ZIGBEE_DIRECT_DLK_TLV_PUBLIC_KEY_P256_MAX_LEN) sl_zigbee_direct_dlk_public_key_tlv_t;

/**
 * @brief called when a Start Key Negotiation Request is received, processes the
 *        request and crafts a response
 * @param  messageLength   length of the incoming request message
 * @param  messageContents pointer to the message, containing a sequence number and tlv payload
 * @param  payloadIndex    index of the tlv payload within messageContents
 * @param  partnerId       short id of the node to send the request to
 * @param  characteristic  authentication method being requested
 * @return                 SL_STATUS_OK if sending the response was successful,
 *                         SL_STATUS_INVALID_CONFIGURATION otherwise
 */
sl_status_t sli_zigbee_direct_handle_incoming_dlk_negotiation_request(uint8_t messageLength,
                                                                      uint8_t* messageContents,
                                                                      uint16_t payloadIndex,
                                                                      sl_802154_short_addr_t partnerId,
                                                                      uint16_t characteristic);

#endif //SILABS_ZIGBEE_DIRECT_SESSION_KEY_NEGOTIATION_H
