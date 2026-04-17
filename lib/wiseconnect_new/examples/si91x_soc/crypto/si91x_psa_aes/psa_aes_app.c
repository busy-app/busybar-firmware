/*******************************************************************************
 * @file  psa_aes_app.c
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
#include "psa_aes_app.h"
#include "psa/crypto.h"
#include <stdio.h>

/* Test vector (AES) for  AES-128/256 key */
// 128 Key Size
uint8_t aes_key_16[KEY_SIZE_16] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                                    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f };

// 256 Key Size
uint8_t aes_key_32[KEY_SIZE_32] = { 0x60, 0x3d, 0xeb, 0x10, 0x15, 0xca, 0x71, 0xbe, 0x2b, 0x73, 0xae,
                                    0xf0, 0x85, 0x7d, 0x77, 0x81, 0x1f, 0x35, 0x2c, 0x07, 0x3b, 0x61,
                                    0x08, 0xd7, 0x2d, 0x98, 0x10, 0xa3, 0x09, 0x14, 0xdf, 0xf4 };

static unsigned char msg_test_data[NB_TESTS][AES_TEST_PT_MAX_LEN] = {
  { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff },
  { 0x6b, 0xc1, 0xbe, 0xe2, 0x2e, 0x40, 0x9f, 0x96, 0xe9, 0x3d, 0x7e, 0x11, 0x73, 0x93, 0x17, 0x2a }
};

static const size_t key_len_test_data[NB_TESTS] = { 16, 32 };

void test_psa_aes(psa_algorithm_t alg)
{
  /* Local variables */
  psa_status_t status;
  uint8_t encryption_output[PAYLOAD_SIZE];
  uint8_t decryption_output[PAYLOAD_SIZE];

  /* psa crypto library initialization */
  status = psa_crypto_init();
  if (status == PSA_SUCCESS) {
    printf("\n PSA crypto library initialization Success \n");
  } else {
    printf("\n PSA crypto library initialization failed with error: %ld\n", status);
  }

  for (int i = 0; i < NB_TESTS; i++) {

    psa_key_id_t key_id;
    psa_key_attributes_t key_attr;
    size_t out_len;

    // Set up attributes for a AES key
    key_attr = psa_key_attributes_init();
    psa_set_key_type(&key_attr, PSA_KEY_TYPE_AES);
    psa_set_key_usage_flags(&key_attr, PSA_KEY_USAGE_ENCRYPT | PSA_KEY_USAGE_DECRYPT);
    psa_set_key_algorithm(&key_attr, alg);

    /* Import a volatile plain key for AES */
    if (i == 0) { // 128 key size
      printf("\n Import a volatile plain %d size key for AES \n", (key_len_test_data[i] * 8));
      status = psa_import_key(&key_attr, aes_key_16, sizeof(aes_key_16), &key_id);
    } else if (i == 1) { // 256 key size
      printf("\n Import a volatile plain %d size key for AES \n", (key_len_test_data[i] * 8));
      status = psa_import_key(&key_attr, aes_key_32, sizeof(aes_key_32), &key_id);
    }

    /* Check for import key status */
    if (status == PSA_SUCCESS) {
      printf("\n Key import Success \n");
    } else {
      printf("\n Key import Failed with error: %ld\n", status);
    }

    /* Encryption */
    status = psa_cipher_encrypt(key_id,
                                alg,
                                msg_test_data[i],
                                sizeof(msg_test_data[i]),
                                encryption_output,
                                sizeof(encryption_output),
                                &out_len);

    if (status == 0) {
      printf("\n Encryption Success \n");
    } else {
      printf("\n Encryption Failed with error: %ld\n", status);
    }

    /* Decryption */
    status = psa_cipher_decrypt(key_id,
                                alg,
                                encryption_output,
                                sizeof(encryption_output),
                                decryption_output,
                                sizeof(decryption_output),
                                &out_len);
    if (status == 0) {
      printf("\n Decryption Success \n");
    } else {
      printf("\n Decryption Failed with error: %ld\n", status);
    }

    /* Destroy a volatile plain key for AES */
    status = psa_destroy_key(key_id);
    if (status != PSA_SUCCESS) {
      printf("Destroy key failed with error: %ld\n", status);
    }
  }
}
