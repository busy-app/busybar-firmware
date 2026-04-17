/***************************************************************************/ /**
 * @file psa_aes_app.h
 * @brief Top level application functions.
 *******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifndef PSA_AES_APP_H
#define PSA_AES_APP_H

// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------

#include "psa/crypto.h"

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

/// Cipher message buffer size (IV + Ciphertext)
#define KEY_SIZE_16         16
#define KEY_SIZE_32         32
#define NB_TESTS            2
#define PAYLOAD_SIZE        16
#define CIPHER_TEXT_SIZE    32
#define AES_TEST_CT_MAX_LEN 16
#define AES_TEST_PT_MAX_LEN 16

void test_psa_aes(psa_algorithm_t alg);

#endif // PSA_AES_APP_H
