/***************************************************************************//**
 * @file aps-frame-counter-sync.h
 * @brief APS Frame Counter Challenges related declarations
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

#ifndef SL_ZIGBEE_APS_FRAME_COUNTER_SYNC_H
#define SL_ZIGBEE_APS_FRAME_COUNTER_SYNC_H

#include "stack/include/sl_zigbee_tlv_core.h"

// NOTE nonce is 1-byte frame control + 4-byte challenge counter + 8-byte eui
#define CHALLENGE_NONCE_LENGTH 1 + 4 + 8
#define CHALLENGE_MIC_LENGTH 8

/**
 * @brief returns false when the aps key needs to have it's incoming frame counter synchronized
 * @param eui64 is EUI64 address of the device
 * @note an index of 0xFF indicates that the key is stored in the preconfigured key slot
 */
bool sli_zigbee_aps_key_in_sync(sl_802154_long_addr_t eui64);

/**
 * @brief controls whether the aps key corresponding to idx requires frame counter sync
 * @param eui64 the EUI64 address of the partner device
 * @return sl_status_t
 */
sl_status_t sli_zigbee_aps_key_set_sync_status(sl_802154_long_addr_t eui64, bool setSync);

/**
 * @brief updates the incoming frame counter with the given values
 * and marks the key as synchronized
 * @param longId the EUI64 associated with the key
 * @param frameCounter the current frame counter value associated with the key
 * @note this procedure uses sli_zigbee_stack_sec_man_export_link_key_by_eui to determine
 * the corresponding key based on the long id
 */
sl_status_t sli_zigbee_sync_aps_frame_counter(sl_802154_long_addr_t longId,
                                              uint32_t frameCounter);

#define SECURITY_CHALLENGE_TLV_TAG_ID 0x00

// Challenge Request TLV
// EUI (8) + Challenge bytes (8)
#define SECURITY_CHALLENGE_REQ_TLV_LENGTH 16

// Challenge Response TLV
// EUI (8)
// + Rx Challenge Value (8)
// + APS Frame Counter (4)
// + Challenge Counter FC (4)
// + MIC (8)
#define SECURITY_CHALLENGE_RSP_TLV_LENGTH 32
#define SECURITY_CHALLENGE_RSP_TLV_CHALLENGE_OFFSET 20

#define SECURITY_CHALLENGE_RSP_FC_LEN 4

// NOTE challenge val is 64-bit number
#define CHALLENGE_VAL_BYTES 8
/**
 * @brief a randomly generated 8-byte (64-bit) value
 */
typedef uint8_t sli_challenge_val_t[CHALLENGE_VAL_BYTES];

/**
 * @brief returns a buffer containing frame counter challenge tlvs
 * @param is_rsp a boolean flag change the format of the challenge for req vs rsp
 * @param sender_long the eui64 of the the sender
 * @param challenge the randomly generated 64-bit challenge value
 * @param context reference to the key to be used for the security challenge
 * @param frame_counter the sender's current outgoing frame counter for the keybase
 * @return a buffer containing challenge tlvs or NULL_BUFFER (memory manager)
 * @note when is_rsp is false frame_counter is ignored
 */
sli_buffer_manager_buffer_t sli_zigbee_aps_create_challenge_tlvs(bool is_rsp,
                                                                 sl_802154_long_addr_t sender_long,
                                                                 sli_challenge_val_t challenge,
                                                                 sl_zigbee_sec_man_context_t *context,
                                                                 uint32_t frame_counter);

/**
 * @brief application callback to mangle fc challenge tlvs for GU behavior
 * @param [in] tlvs a buffer containing the tlvs produced by the standard procedure
 * @return a buffer containing the data that will be used in the over the air message
 */
extern sli_buffer_manager_buffer_t slx_gu_fc_challenge_finalize_cb(sli_buffer_manager_buffer_t tlvs);

/**
 * @brief Authenticates an APS challenge by verifying the provided base key
 * will result in the same HMAC code contained in the input data
 * @param tlv a pointer to the beginning of the Challenge TLV
 * @param context reference to the base key being used in the challenge
 * @return SL_STATUS_OK if the MICs match
 * @note tlv is a pointer to the tag id of the challenge
 */
sl_status_t sli_zigbee_aps_validate_challenge_tlvs(sl_zigbee_tlv_t *tlv,
                                                   sl_zigbee_sec_man_context_t *context);

void sli_zigbee_set_global_fc_sync_flag(bool fc_sync_flag);
bool sli_zigbee_get_global_fc_sync_flag(void);

void sli_zigbee_set_aps_fc_sync_required(void);

#endif // SL_ZIGBEE_APS_FRAME_COUNTER_SYNC_H
