/***************************************************************************//**
 * @file cbke-crypto-engine-baremetal-callbacks.c
 * @brief internal dispatch for 'cbke-crypto-engine' callbacks as a thin-wrapper
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

void sli_zigbee_stack_calculate_smacs_283k1_handler(sl_status_t status,
                                                    sl_zigbee_smac_data_t *initiatorSmac,
                                                    sl_zigbee_smac_data_t *responderSmac)
{
  sl_zigbee_calculate_smacs_283k1_handler(status,
                                          initiatorSmac,
                                          responderSmac);
}

void sli_zigbee_stack_calculate_smacs_handler(sl_status_t status,
                                              sl_zigbee_smac_data_t *initiatorSmac,
                                              sl_zigbee_smac_data_t *responderSmac)
{
  sl_zigbee_calculate_smacs_handler(status,
                                    initiatorSmac,
                                    responderSmac);
}

void sli_zigbee_stack_dsa_sign_handler(sl_status_t status,
                                       uint8_t messageLength,
                                       uint8_t *messageContents)
{
  sl_zigbee_dsa_sign_handler(status,
                             messageLength,
                             messageContents);
}

void sli_zigbee_stack_dsa_verify_handler(sl_status_t status)
{
  sl_zigbee_dsa_verify_handler(status);
}

void sli_zigbee_stack_generate_cbke_keys_283k1_handler(sl_status_t status,
                                                       sl_zigbee_public_key_283k1_data_t *ephemeralPublicKey)
{
  sl_zigbee_generate_cbke_keys_283k1_handler(status,
                                             ephemeralPublicKey);
}

void sli_zigbee_stack_generate_cbke_keys_handler(sl_status_t status,
                                                 sl_zigbee_public_key_data_t *ephemeralPublicKey)
{
  sl_zigbee_generate_cbke_keys_handler(status,
                                       ephemeralPublicKey);
}
