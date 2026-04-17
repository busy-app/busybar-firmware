/***************************************************************************//**
 * @file sli_zigbee_tlv_core.h
 * @brief Stack TLV defines
 * this header defines public portions of the TLV Interface (INTERNAL)
 *******************************************************************************
 * # License
 * <b>Copyright 2018 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifndef SLI_ZIGBEE_TLV_CORE_H
#define SLI_ZIGBEE_TLV_CORE_H

#include "stack/include/sl_zigbee_tlv_core.h"

// these directly reflect the raw length byte
#define sli_zigbee_tlv_get_length_raw(t) ((t)->length)
#define sli_zigbee_tlv_set_length_raw(t, set) ((t)->length = set)

// Function to Validate Tag Length
bool sli_zigbee_tlv_validate_tag_length(uint8_t tag,
                                        uint16_t len,
                                        sl_zigbee_tlv_tag_min_length_t *env);

// Helper functions for Encapsulated TLVs
bool sli_zigbee_tlv_is_encap_tag_id(uint8_t tag_id);
bool sli_zigbee_encap_tlv_check_format(sl_zigbee_tlv_t *encap_tlv);

// validates no broken links and no duplicates
// (except encapsulation tlvs for now)
sl_status_t sli_tlv_chain_is_valid_with_local_env(sl_zigbee_tlv_chain *tlvs,
                                                  sl_zigbee_tlv_tag_min_length_t *env,
                                                  bool encapRecurse);

#define sli_tlv_chain_is_valid(ts) \
  sli_tlv_chain_is_valid_with_local_env((ts), NULL, false)

/**
 * @brief takes the contents in the tlv-chain and appends
 * to the end of the given buffer
 * @param [in] chain the chain of tlvs to append (will append .length bytes)
 * @param [in|out] buffer_handle pointer to the buffer that should be appended to
 * @return a status code indicating the success
 */
sl_status_t sli_zigbee_tlv_chain_append_to_buffer(sl_zigbee_tlv_chain *chain,
                                                  sli_buffer_manager_buffer_t *buffer_handle);

sl_status_t sli_zigbee_tlv_concat_to_buffer(sli_buffer_manager_buffer_t *buffer_handle,
                                            uint16_t index,
                                            sl_zigbee_tlv_t *tlv);

#endif /* SL_ZIGBEE_TLV_CORE_H */
