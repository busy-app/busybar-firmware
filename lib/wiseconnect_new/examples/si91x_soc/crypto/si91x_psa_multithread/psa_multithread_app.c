/*******************************************************************************
 * @file  psa_multithread_app.c
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
#include "psa_multithread_app.h"
#include "sl_si91x_crypto_thread.h"
#include "psa/crypto.h"
#include "app.h"
#include <stdio.h>

static psa_key_id_t key_id;
static psa_status_t status;

/* Test vector (AES) for AES-128 key */
uint8_t aes_key_16[KEY_SIZE_16] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                                    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f };

static const unsigned char res_test_data[AES_TEST_CT_MAX_LEN] = { 0x69, 0xc4, 0xe0, 0xd8, 0x6a, 0x7b, 0x04, 0x30,
                                                                  0xd8, 0xcd, 0xb7, 0x80, 0x70, 0xb4, 0xc5, 0x5a };

static const unsigned char msg_test_data[AES_TEST_PT_MAX_LEN] = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
                                                                  0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff };

void test_psa_sha_256()
{
  psa_status_t ret;

  //! Test message for streaming
  const char test_msg[] = { "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq" };

  //! Expected SHA256 hash value of test message
  uint8_t expect_sha256_hash[] = { 0x24, 0x8d, 0x6a, 0x61, 0xd2, 0x06, 0x38, 0xb8, 0xe5, 0xc0, 0x26,
                                   0x93, 0x0c, 0x3e, 0x60, 0x39, 0xa3, 0x3c, 0xe4, 0x59, 0x64, 0xff,
                                   0x21, 0x67, 0xf6, 0xec, 0xed, 0xd4, 0x19, 0xdb, 0x06, 0xc1 };

  //! Hash Buffer
  uint8_t hash_buf[SL_SI91X_SHA_256_DIGEST_LEN] = { 0 };

  //! Hash length
  size_t hash_len;

  ret = psa_hash_compute(PSA_ALG_SHA_256,
                         (const unsigned char *)test_msg,
                         strlen(test_msg),
                         hash_buf,
                         sizeof(hash_buf),
                         &hash_len);

  // Compare with the expected result
  if (ret != 0) {
    printf("\n SHA 256 Operation Failed Status 0x%lx \n", ret);
  } else if (ret == 0 && memcmp(hash_buf, expect_sha256_hash, sizeof(expect_sha256_hash)) == 0) {
    printf("\n SHA 256 Operation Success Status 0x%lx \n", ret);
  } else {
    printf("\n SHA 256 Compare with expected value failed \n");
  }
}

void sha_app_process_action()
{
  test_psa_sha_256();
}

void test_psa_aes(psa_algorithm_t alg)
{

  size_t out_len;

  uint8_t encryption_output[PAYLOAD_SIZE];
  uint8_t decryption_output[PAYLOAD_SIZE];

  /* Encryption */
  status = psa_cipher_encrypt(key_id,
                              alg,
                              msg_test_data,
                              sizeof(msg_test_data),
                              encryption_output,
                              sizeof(encryption_output),
                              &out_len);

  if ((status == 0) && (memcmp(encryption_output, res_test_data, sizeof(msg_test_data)) == 0)) {
    printf("\n Encryption Success \r\n");
  } else {
    printf("\n Encryption Failed with error: %ld\r\n", status);
  }

  /* Decryption */
  status = psa_cipher_decrypt(key_id,
                              alg,
                              encryption_output,
                              sizeof(encryption_output),
                              decryption_output,
                              sizeof(decryption_output),
                              &out_len);
  if ((status == 0) && (memcmp(decryption_output, msg_test_data, sizeof(decryption_output)) == 0)) {
    printf("\n Decryption Success \n");
  } else {
    printf("\n Decryption Failed with error: %ld\r\n", status);
  }

  /* Destroy a volatile plain key for AES */
  status = psa_destroy_key(key_id);
  if (status != PSA_SUCCESS) {
    printf("Destroy key failed with error: %ld\r\n", status);
  }
}

void aes_app_process_action(psa_algorithm_t alg)
{
  psa_key_attributes_t key_attr;

  // Set up attributes for a AES key
  key_attr = psa_key_attributes_init();
  psa_set_key_type(&key_attr, PSA_KEY_TYPE_AES);
  psa_set_key_usage_flags(&key_attr, PSA_KEY_USAGE_ENCRYPT | PSA_KEY_USAGE_DECRYPT);
  psa_set_key_algorithm(&key_attr, PSA_ALG_ECB_NO_PADDING);

  /* Import a volatile plain key for AES */
  status = psa_import_key(&key_attr, aes_key_16, sizeof(aes_key_16), &key_id);

  /* Check for import key status */
  if (status == PSA_SUCCESS) {
    printf("\n Key import Success \r\n");

  } else {
    printf("\n Key import Failed with error: %ld\r\n", status);
  }

  test_psa_aes(alg);
}
