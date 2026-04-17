/***************************************************************************//**
 * @file
 * @brief Stub version of the Certificate Based Key Exchange library.  See non-stub
 * file for more information.
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

#include PLATFORM_HEADER
#include "stack/include/sl_zigbee_types.h"
#include "stack/include/library.h"

const sl_zigbee_library_status_t sli_zigbee_cbke_library_status = SL_ZIGBEE_LIBRARY_IS_STUB;

bool sli_zigbee_real_ecc_library_present(void)
{
  return false;
}

sl_status_t sli_zigbee_stack_calculate_smacs(bool amInitiator,
                                             sl_zigbee_certificate_data_t* partnerCert,
                                             sl_zigbee_public_key_data_t* partnerEphemeralPublicKey)
{
  (void)amInitiator;
  (void)partnerCert;
  (void)partnerEphemeralPublicKey;
  return SL_STATUS_NOT_AVAILABLE;
}

sl_status_t sli_zigbee_stack_generate_cbke_keys(void)
{
  return SL_STATUS_NOT_AVAILABLE;
}

void generateCbkeKeys(void)
{
}

void calculateSmacs(void)
{
}

sl_status_t reallyCalculateSmacs(bool amInitiator,
                                 sl_zigbee_certificate_data_t* partnerCert,
                                 sl_zigbee_public_key_data_t* partnerEphemeralPublicKey,
                                 sl_zigbee_smac_data_t* initiatorSmacReturn,
                                 sl_zigbee_smac_data_t* responderSmacReturn)
{
  (void)amInitiator;
  (void)partnerCert;
  (void)partnerEphemeralPublicKey;
  (void)initiatorSmacReturn;
  (void)responderSmacReturn;
  return SL_STATUS_NOT_AVAILABLE;
}

void deriveMacKeyAndKeyData(bool amInitiator,
                            uint8_t* sharedSecret,
                            uint8_t* macKeyResult,
                            uint8_t* keyDataResult)
{
  (void)amInitiator;
  (void)sharedSecret;
  (void)macKeyResult;
  (void)keyDataResult;
}

void generateMacFromKey(bool amInitiator,
                        bool deriveAsInitiator,
                        uint8_t* macKey,
                        uint8_t* mac)
{
  (void)amInitiator;
  (void)deriveAsInitiator;
  (void)macKey;
  (void)mac;
}

void invalidatePartnerData(void)
{
}

// From cbke-crypto-engine-storage-{platform}.c
void sli_zigbee_initialize_cbke(void)
{
}

sl_status_t sli_zigbee_stack_set_preinstalled_cbke_data(sl_zigbee_public_key_data_t* caPublic,
                                                        sl_zigbee_certificate_data_t* myCert,
                                                        sl_zigbee_private_key_data_t* myKey)
{
  (void)caPublic;
  (void)myCert;
  (void)myKey;
  return SL_STATUS_NOT_AVAILABLE;
}

sl_status_t sli_zigbee_stack_get_certificate(sl_zigbee_certificate_data_t* result)
{
  (void)result;
  return SL_STATUS_NOT_AVAILABLE;
}

sl_status_t sli_zigbee_get_cbke_data(sl_zigbee_certificate_data_t* myCertPtr,
                                     sl_zigbee_private_key_data_t* myStaticPrivateKeyPtr,
                                     sl_zigbee_public_key_data_t* caPublicKeyPtr)
{
  (void)myCertPtr;
  (void)myStaticPrivateKeyPtr;
  (void)caPublicKeyPtr;
  return SL_STATUS_NOT_AVAILABLE;
}

bool sli_zigbee_stack_get_stack_certificate_eui64(sl_802154_long_addr_t certEui64)
{
  (void)certEui64;
  return false;
}

sl_status_t sli_zigbee_retrieve_and_clear_unverified_link_key(sl_zigbee_key_data_t* key,
                                                              sl_802154_long_addr_t ieee)
{
  (void)key;
  (void)ieee;
  return SL_STATUS_NOT_AVAILABLE;
}

void sli_zigbee_clear_all_temporary_crypto_data(void)
{
}

sl_status_t sli_zigbee_stack_clear_temporary_data_maybe_store_link_key(bool storeLinkKey)
{
  (void)storeLinkKey;
  return SL_STATUS_NOT_AVAILABLE;
}
