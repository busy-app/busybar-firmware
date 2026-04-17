/*******************************************************************************
 * @file  psa_hmac_app.c
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
#include "psa_hmac_app.h"
#include <stdio.h>
#include <stdlib.h>

void test_psa_hmac(psa_algorithm_t hash_alg)
{

  //Test vectors for HMAC SHA-256
  uint8_t hmac_key[] = {
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66
  };
  uint8_t hmac_msg[] = {
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66
  };
  uint8_t expected_mac[] = { 0xfb, 0x5b, 0x26, 0x22, 0x9c, 0x20, 0xb7, 0xed, 0x86, 0x67, 0x06,
                             0xa2, 0xfb, 0xfa, 0xe6, 0x7e, 0x3f, 0x40, 0x4b, 0xb6, 0xab, 0xe7,
                             0x7f, 0xf4, 0x50, 0x63, 0xa4, 0x59, 0xa4, 0x29, 0x24, 0xa4 };

  size_t mac_len   = PSA_HASH_LENGTH(hash_alg);
  uint8_t *mac_buf = malloc(mac_len);
  memset(mac_buf, 0, mac_len);

  psa_status_t ret;
  psa_key_id_t key_id;
  psa_key_attributes_t key_attr;
  ret = psa_crypto_init();
  if (ret != PSA_SUCCESS) {
    printf("PSA Crypto Init Failed with error : %ld\n", ret);
  } else {
    printf("PSA Crypto Init Success!\n");
  }

  // Set up attributes for a HMAC key
  key_attr = psa_key_attributes_init();
  psa_set_key_type(&key_attr, PSA_KEY_TYPE_HMAC);
  psa_set_key_usage_flags(&key_attr, PSA_KEY_USAGE_SIGN_MESSAGE | PSA_KEY_USAGE_VERIFY_MESSAGE);
  psa_set_key_algorithm(&key_attr, PSA_ALG_HMAC(hash_alg));

  // Import a volatile plain key for HMAC
  ret = psa_import_key(&key_attr, hmac_key, sizeof(hmac_key), &key_id);
  if (ret != PSA_SUCCESS) {
    printf("PSA Import key failed with error : %ld\n", ret);
  } else {
    printf("PSA Import Key Success!\n");
  }

  // Calculate the HMAC MAC of a message
  // Expected HMAC MAC:
  // fb 5b 26 22 9c 20 b7 ed 86 67 06 a2 fb fa e6 7e 3f 40 4b b6 ab e7 7f f4 50 63 a4 59 a4 29 24 a4
  ret = psa_mac_compute(key_id, PSA_ALG_HMAC(hash_alg), hmac_msg, sizeof(hmac_msg), mac_buf, mac_len, &mac_len);
  if (ret != PSA_SUCCESS || (memcmp(mac_buf, expected_mac, sizeof(expected_mac)))) {
    printf("MAC Compute failed with error : %ld\n", ret);
  } else {
    printf("MAC Compute is Successful!\n");
  }

  // Verify the HMAC MAC of a message
  ret = psa_mac_verify(key_id, PSA_ALG_HMAC(hash_alg), hmac_msg, sizeof(hmac_msg), mac_buf, mac_len);
  if (ret != PSA_SUCCESS) {
    printf("MAC Verify Failed with error : %ld\n", ret);
  } else {
    printf("MAC Verification Success!\n");
  }
  // Destroy a volatile plain key for HMAC
  ret = psa_destroy_key(key_id);
  free(mac_buf);
  return;
}
