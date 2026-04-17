/*******************************************************************************
 * @file  psa_cmac_app.c
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
#include "psa_cmac_app.h"
#include <stdio.h>
#include <stdlib.h>

void test_psa_cmac()
{
  uint8_t cmac_key[] = {
    0x60, 0x3d, 0xeb, 0x10, 0x15, 0xca, 0x71, 0xbe, 0x2b, 0x73, 0xae, 0xf0, 0x85, 0x7d, 0x77, 0x81,
    0x1f, 0x35, 0x2c, 0x07, 0x3b, 0x61, 0x08, 0xd7, 0x2d, 0x98, 0x10, 0xa3, 0x09, 0x14, 0xdf, 0xf4
  };

  uint8_t cmac_msg[] = { 0x6b, 0xc1, 0xbe, 0xe2, 0x2e, 0x40, 0x9f, 0x96, 0xe9, 0x3d, 0x7e, 0x11, 0x73, 0x93,
                         0x17, 0x2a, 0xae, 0x2d, 0x8a, 0x57, 0x1e, 0x03, 0xac, 0x9c, 0x9e, 0xb7, 0x6f, 0xac,
                         0x45, 0xaf, 0x8e, 0x51, 0x30, 0xc8, 0x1c, 0x46, 0xa3, 0x5c, 0xe4, 0x11 };

  uint8_t expected_mac[] = { 0xaa, 0xf3, 0xd8, 0xf1, 0xde, 0x56, 0x40, 0xc2,
                             0x32, 0xf5, 0xb1, 0x69, 0xb9, 0xc9, 0x11, 0xe6 };
  uint8_t mac_buf[16];
  size_t mac_len;

  psa_status_t ret;
  psa_key_id_t key_id;
  psa_key_attributes_t key_attr;

  ret = psa_crypto_init();
  if (ret != PSA_SUCCESS) {
    printf("PSA Crypto Init Failed with error : %ld\n", ret);
  } else {
    printf("PSA Crypto Init Success!\n");
  }
  // Set up attributes for a CMAC key
  key_attr = psa_key_attributes_init();
  psa_set_key_type(&key_attr, PSA_KEY_TYPE_AES);
  psa_set_key_usage_flags(&key_attr, PSA_KEY_USAGE_SIGN_MESSAGE | PSA_KEY_USAGE_VERIFY_MESSAGE);
  psa_set_key_algorithm(&key_attr, PSA_ALG_CMAC);

  // Import a volatile plain key for CMAC
  ret = psa_import_key(&key_attr, cmac_key, sizeof(cmac_key), &key_id);
  if (ret != PSA_SUCCESS) {
    printf("PSA Import key failed with error : %ld\n", ret);
  } else {
    printf("PSA Import Key Success!\n");
  }

  // Calculate the CMAC MAC of a message
  // Expected CMAC MAC: aa f3 d8 f1 de 56 40 c2 32 f5 b1 69 b9 c9 11 e6
  ret = psa_mac_compute(key_id, PSA_ALG_CMAC, cmac_msg, sizeof(cmac_msg), mac_buf, sizeof(mac_buf), &mac_len);
  if (ret != PSA_SUCCESS || (memcmp(mac_buf, expected_mac, sizeof(expected_mac)))) {
    printf("MAC Compute failed with error : %ld\n", ret);
  } else {
    printf("MAC Compute is Successful!\n");
  }

  // Verify the CMAC MAC of a message
  ret = psa_mac_verify(key_id, PSA_ALG_CMAC, cmac_msg, sizeof(cmac_msg), mac_buf, mac_len);
  if (ret != PSA_SUCCESS) {
    printf("MAC Verify Failed with error : %ld\n", ret);
  } else {
    printf("MAC Verification Success!\n");
  }

  // Destroy a volatile plain key for CMAC
  ret = psa_destroy_key(key_id);
  if (ret != PSA_SUCCESS) {
    printf("PSA Destroy Key failed with error : %ld\n", ret);
  } else {
    printf("PSA Destroy Key Success!\n");
  }

  return;
}
