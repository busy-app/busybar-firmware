/*******************************************************************************
 * @file  psa_gcm_app.h
 * @brief
 *******************************************************************************
 * # License
 * <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifndef PSA_GCM_APP_H
#define PSA_GCM_APP_H

#include "psa/crypto.h"
#include "sli_si91x_crypto_driver_functions.h"

#define KEY_SIZE_16         16
#define GCM_TEST_PT_MAX_LEN 5120
#define GCM_TEST_CT_MAX_LEN 1040

void test_psa_gcm(void);

#endif /* PSA_GCM_APP_H */
