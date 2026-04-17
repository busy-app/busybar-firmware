/*******************************************************************************
 * @file  psa_chachapoly_app.h
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

#ifndef PSA_CHACHAPOLY_APP_H
#define PSA_CHACHAPOLY_APP_H

#include "psa/crypto.h"
#include "sli_si91x_crypto_driver_functions.h"

#define CHACHAPOLY_TEST_PT_MAX_LEN 114
#define CHACHAPOLY_TEST_CT_MAX_LEN 130
#define NB_TESTS                   1

struct test_vector {
  unsigned char key_test_data[16];
  unsigned char iv_test_data[12];
  unsigned char ad_test_data[20];
  unsigned char msg_test_data[114];
  unsigned char res_test_data[130];
  size_t tag_len;
};

void test_psa_chachapoly(void);

#endif /* PSA_CHACHAPOLY_APP_H */
