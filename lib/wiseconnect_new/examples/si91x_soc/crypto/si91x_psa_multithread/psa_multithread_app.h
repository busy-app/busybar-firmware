/*******************************************************************************
 * @file  psa_multithread_app.h
 * @brief
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

#ifndef PSA_MULTITHREAD_APP_H
#define PSA_MULTITHREAD_APP_H

#include "psa/crypto.h"
#include "sl_si91x_sha.h"
#include "sl_si91x_aes.h"
#include "sl_si91x_psa_sha.h"
#include "sl_si91x_psa_aes.h"

/***************************************************************************/ /**
 *                          Macros and Typedefs
 ******************************************************************************/
/// Cipher message buffer size (IV + Ciphertext)
#define KEY_SIZE_16         16
#define PAYLOAD_SIZE        16
#define CIPHER_TEXT_SIZE    32
#define AES_TEST_CT_MAX_LEN 16
#define AES_TEST_PT_MAX_LEN 16

/***************************************************************************/ /**
 *                          Function declarations
 ******************************************************************************/
void aes_app_process_action(psa_algorithm_t alg);
void sha_app_process_action();
void test_psa_sha_256();
void test_psa_aes(psa_algorithm_t alg);

#endif /* PSA_MULTITHREAD_APP_H */
