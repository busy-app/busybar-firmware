/*******************************************************************************
 * @file  psa_ccm_app.h
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

#ifndef PSA_CCM_APP_H
#define PSA_CCM_APP_H

#include "psa/crypto.h"

#define CCM_TEST_PT_MAX_LEN 24
#define CCM_TEST_CT_MAX_LEN 32
#define NB_TESTS            3

struct test_vector {
  unsigned char iv_test_data[12];
  unsigned char ad_test_data[20];
  unsigned char msg_test_data[CCM_TEST_PT_MAX_LEN];
  unsigned char res_test_data[CCM_TEST_CT_MAX_LEN];
  size_t tag_len;
};

void test_psa_ccm(void);

#endif /* PSA_CCM_APP_H */
