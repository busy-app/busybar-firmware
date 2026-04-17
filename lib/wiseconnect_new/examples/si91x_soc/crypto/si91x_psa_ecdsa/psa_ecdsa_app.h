/*******************************************************************************
 * @file  psa_ecdsa_app.h
 * @brief
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

#ifndef PSA_ECDSA_APP_H
#define PSA_ECDSA_APP_H

#include "psa/crypto.h"

#define PUBLIC_KEY_SIZE_P192R1       49
#define PUBLIC_KEY_SIZE_P224R1       57
#define PUBLIC_KEY_SIZE_P256R1       65
#define PRIVATE_KEY_SIZE_P192R1      24
#define PRIVATE_KEY_SIZE_P224R1      28
#define PRIVATE_KEY_SIZE_P256R1      32
#define SIGNATURE_SIZE_P192R1        48
#define SIGNATURE_SIZE_P224R1        56
#define SIGNATURE_SIZE_P256R1        64
#define PRIVATE_KEY_SIZE_P192R1_BITS 192
#define PRIVATE_KEY_SIZE_P224R1_BITS 224
#define PRIVATE_KEY_SIZE_P256R1_BITS 256

void test_psa_ecdsa(void);

#endif /* PSA_ECDSA_APP_H */