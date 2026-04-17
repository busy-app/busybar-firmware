/***************************************************************************//**
 * @file cbke-crypto-engine-internal-def.h
 * @brief internal names for 'cbke-crypto-engine' declarations
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
// automatically generated from cbke-crypto-engine.h.  Do not manually edit
#ifndef CBKE_CRYPTO_ENGINE_INTERNAL_DEF_H
#define CBKE_CRYPTO_ENGINE_INTERNAL_DEF_H

#include "stack/include/cbke-crypto-engine.h"

// Command Indirection

sl_status_t sli_zigbee_stack_calculate_smacs(bool amInitiator,
                                             sl_zigbee_certificate_data_t *partnerCert,
                                             sl_zigbee_public_key_data_t *partnerEphemeralPublicKey);

sl_status_t sli_zigbee_stack_calculate_smacs_283k1(bool amInitiator,
                                                   sl_zigbee_certificate_283k1_data_t *partnerCert,
                                                   sl_zigbee_public_key_283k1_data_t *partnerEphemeralPublicKey);

sl_status_t sli_zigbee_stack_clear_temporary_data_maybe_store_link_key(bool storeLinkKey);

sl_status_t sli_zigbee_stack_clear_temporary_data_maybe_store_link_key_283k1(bool storeLinkKey);

sl_status_t sli_zigbee_stack_dsa_sign(sli_buffer_manager_buffer_t messageToSign);

sl_status_t sli_zigbee_stack_dsa_verify(sl_zigbee_message_digest_t *digest,
                                        sl_zigbee_certificate_data_t *signerCertificate,
                                        sl_zigbee_signature_data_t *receivedSig);

sl_status_t sli_zigbee_stack_dsa_verify_283k1(const sl_zigbee_message_digest_t *digest,
                                              const sl_zigbee_certificate_283k1_data_t *signerCertificate,
                                              const sl_zigbee_signature_283k1_data_t *receivedSig);

sl_status_t sli_zigbee_stack_generate_cbke_keys(void);

sl_status_t sli_zigbee_stack_generate_cbke_keys_283k1(void);

sl_status_t sli_zigbee_stack_get_certificate(sl_zigbee_certificate_data_t *result);

sl_status_t sli_zigbee_stack_get_certificate_283k1(sl_zigbee_certificate_283k1_data_t *result);

bool sli_zigbee_stack_get_stack_certificate_eui64(sl_802154_long_addr_t certEui64);

sl_status_t sli_zigbee_stack_set_preinstalled_cbke_data(sl_zigbee_public_key_data_t *caPublic,
                                                        sl_zigbee_certificate_data_t *myCert,
                                                        sl_zigbee_private_key_data_t *myKey);

sl_status_t sli_zigbee_stack_set_preinstalled_cbke_data_283k1(const sl_zigbee_public_key_283k1_data_t *caPublic,
                                                              const sl_zigbee_certificate_283k1_data_t *myCert,
                                                              const sl_zigbee_private_key_283k1_data_t *myKey);

// Callback Indirection

void sli_zigbee_stack_calculate_smacs_283k1_handler(sl_status_t status,
                                                    sl_zigbee_smac_data_t *initiatorSmac,
                                                    sl_zigbee_smac_data_t *responderSmac);

void sli_zigbee_stack_calculate_smacs_handler(sl_status_t status,
                                              sl_zigbee_smac_data_t *initiatorSmac,
                                              sl_zigbee_smac_data_t *responderSmac);

void sli_zigbee_stack_dsa_sign_handler(sl_status_t status,
                                       uint8_t messageLength,
                                       uint8_t *messageContents);

void sli_zigbee_stack_dsa_verify_handler(sl_status_t status);

void sli_zigbee_stack_generate_cbke_keys_283k1_handler(sl_status_t status,
                                                       sl_zigbee_public_key_283k1_data_t *ephemeralPublicKey);

void sli_zigbee_stack_generate_cbke_keys_handler(sl_status_t status,
                                                 sl_zigbee_public_key_data_t *ephemeralPublicKey);

#endif // CBKE_CRYPTO_ENGINE_INTERNAL_DEF_H
