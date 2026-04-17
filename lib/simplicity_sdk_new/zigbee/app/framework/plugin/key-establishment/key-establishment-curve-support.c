/***************************************************************************//**
 * @file
 * @brief This file calls ECC curve specific functions from the cbke library
 * based on whether the 163k1 ECC curve or the 283k1 ECC curve is
 * being used for key establishment.
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

#include "app/framework/include/af.h"

#include "key-establishment.h"
#include "key-establishment-storage.h"
#ifndef EZSP_HOST
  #include "stack/include/cbke-crypto-engine.h"
#endif

bool checkIssuer(uint8_t *issuer)
{
  uint8_t *ourIssuer;

  if (isCbkeKeyEstablishmentSuite163k1()) {
    sl_zigbee_certificate_data_t localCert;
    if (sl_zigbee_get_certificate(&localCert) != SL_STATUS_OK) {
      sl_zigbee_af_key_establishment_cluster_println("Could not retrieve certificate");
      cleanupAndStop(NO_LOCAL_RESOURCES);
      return false;
    }
    ourIssuer = &(localCert.contents[CERT_ISSUER_OFFSET]);
  } else if (isCbkeKeyEstablishmentSuite283k1()) {
    sl_zigbee_certificate_283k1_data_t localCert;
    if (sl_zigbee_get_certificate_283k1(&localCert) != SL_STATUS_OK) {
      sl_zigbee_af_key_establishment_cluster_println("Could not retrieve 283k1 certificate");
      cleanupAndStop(NO_LOCAL_RESOURCES);
      return false;
    }
    ourIssuer = &(localCert.contents[CERT_ISSUER_OFFSET_283K1]);
  } else {
    //No valid Suite.
    return false;
  }

  if (memcmp(issuer, ourIssuer, CERT_ISSUER_SIZE) == 0) {
    sl_zigbee_af_key_establishment_cluster_println("Good Certificate Issuer");
    return true;
  }
  sl_zigbee_af_key_establishment_cluster_println("Bad Certificate Issuer");
  cleanupAndStop(BAD_CERTIFICATE_ISSUER);
  return false;
}

bool storePublicPartnerData(bool isCertificate,
                            uint8_t* data)
{
  if (isCbkeKeyEstablishmentSuite163k1()) {
    return storePublicPartnerData163k1(isCertificate, data);
  } else if (isCbkeKeyEstablishmentSuite283k1()) {
    return storePublicPartnerData283k1(isCertificate, data);
  } else {
    //No valid suite
    return false;
  }
}

bool retrieveAndClearPublicPartnerData(sl_zigbee_certificate_283k1_data_t* partnerCertificate,
                                       sl_zigbee_public_key_283k1_data_t* partnerEphemeralPublicKey)
{
  if (isCbkeKeyEstablishmentSuite163k1()) {
    return retrieveAndClearPublicPartnerData163k1((sl_zigbee_certificate_data_t *)sl_zigbee_certificate_283k1_contents(partnerCertificate),
                                                  (sl_zigbee_public_key_data_t *)sl_zigbee_public_key_283k1_contents(partnerEphemeralPublicKey));
  } else if (isCbkeKeyEstablishmentSuite283k1()) {
    return retrieveAndClearPublicPartnerData283k1(partnerCertificate, partnerEphemeralPublicKey);
  } else {
    //No valid suite
    return false;
  }
}

sl_zigbee_af_key_establishment_notify_message_t sendCertificate(void)
{
  if (isCbkeKeyEstablishmentSuite163k1()) {
    sl_zigbee_certificate_data_t localCert;
    if (sl_zigbee_get_certificate(&localCert) != SL_STATUS_OK) {
      sl_zigbee_af_key_establishment_cluster_println("Failed to retrieve certificate.");
      return NO_LOCAL_RESOURCES;
    }
    sendNextKeyEstablishMessage(ZCL_INITIATE_KEY_ESTABLISHMENT_REQUEST_COMMAND_ID,
                                sl_zigbee_certificate_contents(&localCert));
  } else if (isCbkeKeyEstablishmentSuite283k1()) {
    sl_zigbee_certificate_283k1_data_t localCert;
    if (sl_zigbee_get_certificate_283k1(&localCert) != SL_STATUS_OK) {
      sl_zigbee_af_key_establishment_cluster_println("Failed to retrieve 283k1 certificate.");
      return NO_LOCAL_RESOURCES;
    }
    sendNextKeyEstablishMessage(ZCL_INITIATE_KEY_ESTABLISHMENT_REQUEST_COMMAND_ID,
                                sl_zigbee_certificate_283k1_contents(&localCert));
  } else {
    //Closest error message
    return NO_LOCAL_RESOURCES;
  }

  return NO_APP_MESSAGE;
}

sl_status_t sli_zigbee_af_generate_cbke_keys_for_curve(void)
{
  sl_status_t status;
  if (isCbkeKeyEstablishmentSuite163k1()) {
    status = sl_zigbee_generate_cbke_keys();
  } else if (isCbkeKeyEstablishmentSuite283k1()) {
    status = sl_zigbee_generate_cbke_keys_283k1();
  } else {
    status = SL_STATUS_FAIL;
  }
  return status;
}

sl_status_t sli_zigbee_af_calculate_smacs_for_curve(bool amInitiator,
                                                    sl_zigbee_certificate_283k1_data_t* partnerCert,
                                                    sl_zigbee_public_key_283k1_data_t* partnerEphemeralPublicKey)
{
  sl_status_t status;
  sl_zigbee_af_key_establishment_cluster_println("calculateSmacs()");
  if (isCbkeKeyEstablishmentSuite163k1()) {
    status = sl_zigbee_calculate_smacs(amInitiator,
                                       (sl_zigbee_certificate_data_t *)sl_zigbee_certificate_283k1_contents(partnerCert),
                                       (sl_zigbee_public_key_data_t *)sl_zigbee_public_key_283k1_contents(partnerEphemeralPublicKey));
  } else if (isCbkeKeyEstablishmentSuite283k1()) {
    status = sl_zigbee_calculate_smacs_283k1(amInitiator,
                                             partnerCert,
                                             partnerEphemeralPublicKey);
  } else {
    status = SL_STATUS_FAIL;
  }
  return status;
}

sl_status_t sl_zigbee_clear_temporary_data_maybe_store_link_key_for_curve(bool storeLinkKey)
{
  sl_status_t status;
  if ( isCbkeKeyEstablishmentSuite163k1() ) {
    status = sl_zigbee_clear_temporary_data_maybe_store_link_key(storeLinkKey);
  } else if ( isCbkeKeyEstablishmentSuite283k1()) {
    status = sl_zigbee_clear_temporary_data_maybe_store_link_key_283k1(storeLinkKey);
  } else {
    status = SL_STATUS_FAIL;
  }
  return status;
}
