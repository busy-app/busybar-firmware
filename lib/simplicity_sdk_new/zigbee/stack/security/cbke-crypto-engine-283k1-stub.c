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

const sl_zigbee_library_status_t sli_zigbee_cbke283k1_library_status = SL_ZIGBEE_LIBRARY_IS_STUB;

bool sli_zigbee_real_ecc_library_present283k1(void)
{
  return false;
}

sl_status_t sli_zigbee_stack_calculate_smacs_283k1(bool amInitiator,
                                                   sl_zigbee_certificate_283k1_data_t* partnerCert,
                                                   sl_zigbee_public_key_283k1_data_t* partnerEphemeralPublicKey)
{
  (void)amInitiator;
  (void)partnerCert;
  (void)partnerEphemeralPublicKey;
  return SL_STATUS_NOT_AVAILABLE;
}

sl_status_t sli_zigbee_stack_generate_cbke_keys_283k1(void)
{
  return SL_STATUS_NOT_AVAILABLE;
}

void generateCbkeKeys283k1(void)
{
}

void calculateSmacs283k1(void)
{
}

void invalidatePartnerData283k1(void)
{
}

void sli_zigbee_initialize_cbke283k1(void)
{
}

sl_status_t sli_zigbee_stack_get_certificate_283k1(sl_zigbee_certificate_283k1_data_t* result)
{
  (void)result;
  return SL_STATUS_NOT_AVAILABLE;
}

sl_status_t sli_zigbee_stack_set_preinstalled_cbke_data_283k1(const sl_zigbee_public_key_283k1_data_t* caPublic,
                                                              const sl_zigbee_certificate_283k1_data_t* myCert,
                                                              const sl_zigbee_private_key_283k1_data_t* myKey)
{
  (void)caPublic;
  (void)myCert;
  (void)myKey;
  return SL_STATUS_NOT_AVAILABLE;
}

sl_status_t sli_zigbee_get_cbke_data283k1(sl_zigbee_certificate_283k1_data_t* myCertPtr,
                                          sl_zigbee_private_key_283k1_data_t* myStaticPrivateKeyPtr,
                                          sl_zigbee_public_key_283k1_data_t* caPublicKeyPtr)
{
  (void)myCertPtr;
  (void)myStaticPrivateKeyPtr;
  (void)caPublicKeyPtr;
  return SL_STATUS_NOT_AVAILABLE;
}

sl_status_t sli_zigbee_stack_clear_temporary_data_maybe_store_link_key_283k1(bool storeLinkKey)
{
  (void)storeLinkKey;
  return SL_STATUS_NOT_AVAILABLE;
}
