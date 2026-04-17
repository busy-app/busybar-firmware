/***************************************************************************//**
 * @file
 * @brief  * API for the storage of public temporary partner data.
 * - Partner Certificate
 * - Partner Ephemeral Public Key
 * - A single SMAC
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

// If isCertificate is false, data is a public key.
bool storePublicPartnerData(bool isCertificate,
                            uint8_t* data);
bool storePublicPartnerData163k1(bool isCertificate,
                                 uint8_t* data);
bool storePublicPartnerData283k1(bool isCertificate,
                                 uint8_t* data);
bool retrieveAndClearPublicPartnerData163k1(sl_zigbee_certificate_data_t* partnerCertificate,
                                            sl_zigbee_public_key_data_t* partnerEphemeralPublicKey);

bool retrieveAndClearPublicPartnerData(sl_zigbee_certificate_283k1_data_t* partnerCertificate,
                                       sl_zigbee_public_key_283k1_data_t* partnerEphemeralPublicKey);
bool retrieveAndClearPublicPartnerData283k1(sl_zigbee_certificate_283k1_data_t* partnerCertificate,
                                            sl_zigbee_public_key_283k1_data_t* partnerEphemeralPublicKey);
bool storeSmac(sl_zigbee_smac_data_t* smac);
bool getSmacPointer(sl_zigbee_smac_data_t** smacPtr);

void clearAllTemporaryPublicData(void);
