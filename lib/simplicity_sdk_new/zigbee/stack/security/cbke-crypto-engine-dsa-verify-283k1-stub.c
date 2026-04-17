/***************************************************************************//**
 * @file
 * @brief Functionality specific to verifying a 283k1 DSA signature at the end of a
 * message.
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

const sl_zigbee_library_status_t sli_zigbee_cbke283k1_dsa_verify_library_status = SL_ZIGBEE_LIBRARY_IS_STUB;

sl_status_t sli_zigbee_stack_dsa_verify_283k1(const sl_zigbee_message_digest_t* digest,
                                              const sl_zigbee_certificate_283k1_data_t* signerCertificate,
                                              const sl_zigbee_signature_283k1_data_t* receivedSig)
{
  (void)digest;
  (void)signerCertificate;
  (void)receivedSig;
  return SL_STATUS_NOT_AVAILABLE;
}

void sli_zigbee_dsa_verify283k1(void)
{
}
