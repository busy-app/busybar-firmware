/*******************************************************************************
 * @file  psa_sha_app.h
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

#ifndef PSA_SHA_APP_H
#define PSA_SHA_APP_H

#include "psa/crypto.h"
#include "sl_si91x_sha.h"
#include "sl_si91x_psa_sha.h"

#define SL_SLI91x_SHA_256_EN
void psa_app_process_action();

#ifdef SL_SLI91x_SHA_1_EN
void test_psa_sha_1();
#endif
#ifdef SL_SLI91x_SHA_224_EN
void test_psa_sha_224();
#endif
#ifdef SL_SLI91x_SHA_256_EN
void test_psa_sha_256();
#endif
#ifdef SL_SLI91x_SHA_384_EN
void test_psa_sha_384();
#endif
#ifdef SL_SLI91x_SHA_512_EN
void test_psa_sha_512();
#endif

#endif /* PSA_SHA_APP_H */
