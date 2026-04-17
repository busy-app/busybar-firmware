/***************************************************************************//**
 * @file
 * @brief This file implements the routines for storing temporary data that
 * is needed for key establishment.  This is data is completely
 * public and is sent over-the-air and thus not required to be
 * closely protected.
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

// this file contains all the common includes for clusters in the zcl-util
#include "../../util/common.h"

#include "key-establishment-storage.h"

#ifndef EZSP_HOST
  #include "stack/include/cbke-crypto-engine.h"
#endif

//------------------------------------------------------------------------------
// Globals

static sl_zigbee_certificate_data_t partnerCert;
static sl_zigbee_public_key_data_t partnerPublicKey;
static sl_zigbee_certificate_283k1_data_t partnerCert283k1;
static sl_zigbee_public_key_283k1_data_t partnerPublicKey283k1;
static sl_zigbee_smac_data_t storedSmac;

//------------------------------------------------------------------------------

bool storePublicPartnerData163k1(bool isCertificate,
                                 uint8_t* data)
{
  uint8_t* ptr = (isCertificate
                  ? sl_zigbee_certificate_contents(&partnerCert)
                  : sl_zigbee_public_key_contents(&partnerPublicKey));
  uint8_t size = (isCertificate
                  ? SL_ZIGBEE_CERTIFICATE_SIZE
                  : SL_ZIGBEE_PUBLIC_KEY_SIZE);
  memmove(ptr, data, size);
  return true;
}

bool retrieveAndClearPublicPartnerData163k1(sl_zigbee_certificate_data_t* partnerCertificate,
                                            sl_zigbee_public_key_data_t* partnerEphemeralPublicKey)
{
  if ( partnerCertificate != NULL ) {
    memmove(partnerCertificate,
            &partnerCert,
            SL_ZIGBEE_CERTIFICATE_SIZE);
  }
  if ( partnerEphemeralPublicKey != NULL ) {
    memmove(partnerEphemeralPublicKey,
            &partnerPublicKey,
            SL_ZIGBEE_PUBLIC_KEY_SIZE);
  }
  memset(&partnerCert, 0, SL_ZIGBEE_CERTIFICATE_SIZE);
  memset(&partnerPublicKey, 0, SL_ZIGBEE_PUBLIC_KEY_SIZE);
  return true;
}

bool storePublicPartnerData283k1(bool isCertificate,
                                 uint8_t* data)
{
  uint8_t* ptr = (isCertificate
                  ? sl_zigbee_certificate_283k1_contents(&partnerCert283k1)
                  : sl_zigbee_public_key_283k1_contents(&partnerPublicKey283k1));
  uint8_t size = (isCertificate
                  ? SL_ZIGBEE_CERTIFICATE_283K1_SIZE
                  : SL_ZIGBEE_PUBLIC_KEY_283K1_SIZE);
  memmove(ptr, data, size);
  return true;
}

bool retrieveAndClearPublicPartnerData283k1(sl_zigbee_certificate_283k1_data_t* partnerCertificate,
                                            sl_zigbee_public_key_283k1_data_t* partnerEphemeralPublicKey)
{
  if ( partnerCertificate != NULL ) {
    memmove(partnerCertificate,
            &partnerCert283k1,
            SL_ZIGBEE_CERTIFICATE_283K1_SIZE);
  }
  if ( partnerEphemeralPublicKey != NULL ) {
    memmove(partnerEphemeralPublicKey,
            &partnerPublicKey283k1,
            SL_ZIGBEE_PUBLIC_KEY_283K1_SIZE);
  }
  memset(&partnerCert283k1, 0, SL_ZIGBEE_CERTIFICATE_283K1_SIZE);
  memset(&partnerPublicKey283k1, 0, SL_ZIGBEE_PUBLIC_KEY_283K1_SIZE);
  return true;
}

bool storeSmac(sl_zigbee_smac_data_t* smac)
{
  memmove(&storedSmac, smac, SL_ZIGBEE_SMAC_SIZE);
  return true;
}

bool getSmacPointer(sl_zigbee_smac_data_t** smacPtr)
{
  *smacPtr = &storedSmac;
  return true;
}

void clearAllTemporaryPublicData(void)
{
  memset(&storedSmac, 0, SL_ZIGBEE_SMAC_SIZE);
  retrieveAndClearPublicPartnerData(NULL, NULL);
  retrieveAndClearPublicPartnerData283k1(NULL, NULL);
}
