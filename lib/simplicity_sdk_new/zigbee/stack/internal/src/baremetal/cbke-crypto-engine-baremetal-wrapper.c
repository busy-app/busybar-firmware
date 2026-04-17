/***************************************************************************//**
 * @file cbke-crypto-engine-baremetal-wrapper.c
 * @brief internal implementations for 'cbke-crypto-engine' as a thin-wrapper
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
#include "stack/include/cbke-crypto-engine.h"
#include "stack/internal/inc/cbke-crypto-engine-internal-def.h"

sl_status_t sl_zigbee_calculate_smacs(bool amInitiator,
                                      sl_zigbee_certificate_data_t *partnerCert,
                                      sl_zigbee_public_key_data_t *partnerEphemeralPublicKey)
{
  return sli_zigbee_stack_calculate_smacs(amInitiator,
                                          partnerCert,
                                          partnerEphemeralPublicKey);
}

sl_status_t sl_zigbee_calculate_smacs_283k1(bool amInitiator,
                                            sl_zigbee_certificate_283k1_data_t *partnerCert,
                                            sl_zigbee_public_key_283k1_data_t *partnerEphemeralPublicKey)
{
  return sli_zigbee_stack_calculate_smacs_283k1(amInitiator,
                                                partnerCert,
                                                partnerEphemeralPublicKey);
}

sl_status_t sl_zigbee_clear_temporary_data_maybe_store_link_key(bool storeLinkKey)
{
  return sli_zigbee_stack_clear_temporary_data_maybe_store_link_key(storeLinkKey);
}

sl_status_t sl_zigbee_clear_temporary_data_maybe_store_link_key_283k1(bool storeLinkKey)
{
  return sli_zigbee_stack_clear_temporary_data_maybe_store_link_key_283k1(storeLinkKey);
}

sl_status_t sl_zigbee_dsa_sign(sli_buffer_manager_buffer_t messageToSign)
{
  return sli_zigbee_stack_dsa_sign(messageToSign);
}

sl_status_t sl_zigbee_dsa_verify(sl_zigbee_message_digest_t *digest,
                                 sl_zigbee_certificate_data_t *signerCertificate,
                                 sl_zigbee_signature_data_t *receivedSig)
{
  return sli_zigbee_stack_dsa_verify(digest,
                                     signerCertificate,
                                     receivedSig);
}

sl_status_t sl_zigbee_dsa_verify_283k1(const sl_zigbee_message_digest_t *digest,
                                       const sl_zigbee_certificate_283k1_data_t *signerCertificate,
                                       const sl_zigbee_signature_283k1_data_t *receivedSig)
{
  return sli_zigbee_stack_dsa_verify_283k1(digest,
                                           signerCertificate,
                                           receivedSig);
}

sl_status_t sl_zigbee_generate_cbke_keys(void)
{
  return sli_zigbee_stack_generate_cbke_keys();
}

sl_status_t sl_zigbee_generate_cbke_keys_283k1(void)
{
  return sli_zigbee_stack_generate_cbke_keys_283k1();
}

sl_status_t sl_zigbee_get_certificate(sl_zigbee_certificate_data_t *result)
{
  return sli_zigbee_stack_get_certificate(result);
}

sl_status_t sl_zigbee_get_certificate_283k1(sl_zigbee_certificate_283k1_data_t *result)
{
  return sli_zigbee_stack_get_certificate_283k1(result);
}

bool sl_zigbee_get_stack_certificate_eui64(sl_802154_long_addr_t certEui64)
{
  return sli_zigbee_stack_get_stack_certificate_eui64(certEui64);
}

sl_status_t sl_zigbee_set_preinstalled_cbke_data(sl_zigbee_public_key_data_t *caPublic,
                                                 sl_zigbee_certificate_data_t *myCert,
                                                 sl_zigbee_private_key_data_t *myKey)
{
  return sli_zigbee_stack_set_preinstalled_cbke_data(caPublic,
                                                     myCert,
                                                     myKey);
}

sl_status_t sl_zigbee_set_preinstalled_cbke_data_283k1(const sl_zigbee_public_key_283k1_data_t *caPublic,
                                                       const sl_zigbee_certificate_283k1_data_t *myCert,
                                                       const sl_zigbee_private_key_283k1_data_t *myKey)
{
  return sli_zigbee_stack_set_preinstalled_cbke_data_283k1(caPublic,
                                                           myCert,
                                                           myKey);
}
