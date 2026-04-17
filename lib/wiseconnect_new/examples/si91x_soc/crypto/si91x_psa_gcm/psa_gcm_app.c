/*******************************************************************************
 * @file  psa_gcm_app.c
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
#include "psa_gcm_app.h"
#include "psa/crypto.h"
#include "sl_si91x_psa_wrap.h"
#include <stdio.h>

#define WRAP_INPUT_KEYS     0 // Enable this if the input private key needs to be wrapped before use
#define IMPORT_WRAPPED_KEYS 0 // Enable this if the input key is wrapped

//!GCM inputs

static const unsigned char key_test_data[NB_TESTS][16] = {
  { 0xca, 0x74, 0xfa, 0xb, 0xb5, 0x9a, 0xb5, 0x28, 0x93, 0x70, 0x35, 0xf2, 0x72, 0xd2, 0x16, 0x32 },
  { 0xdf, 0xef, 0xde, 0x23, 0xc6, 0x12, 0x2b, 0xf0, 0x37, 0x0a, 0xb5, 0x89, 0x0e, 0x80, 0x4b, 0x73 },
  { 0x99, 0x71, 0x07, 0x10, 0x59, 0xab, 0xc0, 0x09, 0xe4, 0xf2, 0xbd, 0x69, 0x86, 0x9d, 0xb3, 0x38 }
};

//! IV should be 12 Bytes/96 bits
static const unsigned char iv_test_data[NB_TESTS][16] = {
  { 0x40, 0x92, 0xa5, 0x2c, 0xc9, 0x55, 0xc4, 0xcb, 0xee, 0x97, 0x93, 0x8d },
  { 0x92, 0xd6, 0xa8, 0x02, 0x99, 0x90, 0x67, 0x0f, 0x16, 0xde, 0x79, 0xe2 },
  { 0x07, 0xa9, 0xa9, 0x5e, 0xa3, 0x82, 0x1e, 0x9c, 0x13, 0xc6, 0x32, 0x51 }
};

static const unsigned char msg_test_data[NB_TESTS][GCM_TEST_PT_MAX_LEN] = {
  { 0x5d, 0x80, 0x2f, 0xb6, 0x5e, 0x25, 0x20, 0x21, 0xb9, 0xc1, 0xec, 0x63, 0x34, 0x1c, 0x65, 0x41 },
  { 0x64, 0x26, 0x0a, 0x8c, 0x28, 0x7d, 0xe9, 0x78, 0xe9, 0x6c, 0x75, 0x21, 0xd0 },
  { 0xf5, 0x4b, 0xc3, 0x50, 0x1f, 0xed, 0x4f, 0x6f, 0x6d, 0xfb, 0x5e, 0xa8, 0x01, 0x06, 0xdf, 0x0b,
    0xd8, 0x36, 0xe6, 0x82, 0x62, 0x25, 0xb7, 0x5c, 0x02, 0x22, 0xf6, 0xe8, 0x59, 0xb3, 0x59, 0x83 }
};

//test_additional_data vectors with varying lengths - 13, 16 and 0 (empty vector)
static const unsigned char ad_test_data[NB_TESTS][20] = {
  { 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x16, 0x3, 0x3, 0x0, 0x10 },
  { 0xa2, 0xb1, 0x6d, 0x78, 0x25, 0x1d, 0xe6, 0xc1, 0x91, 0xce, 0x35, 0x0e, 0x5c, 0x5e, 0xf2, 0x42 },
  {}
};

static const unsigned char res_test_data[NB_TESTS][GCM_TEST_CT_MAX_LEN] = {
  { 0x14, 0x00, 0x00, 0x0c, 0x79, 0xb2, 0x1a, 0xa0, 0xf7, 0xff, 0x7a, 0x77, 0x34, 0x86, 0xaa, 0x32 },
  { 0xbf, 0x78, 0xde, 0x94, 0x8a, 0x84, 0x7c, 0x17, 0x36, 0x49, 0xd4, 0xb4, 0xd0 },
  { 0x05, 0x56, 0xc1, 0x59, 0xf8, 0x4e, 0xf3, 0x6c, 0xb1, 0x60, 0x2b, 0x45, 0x26, 0xb1, 0x20, 0x09,
    0xc7, 0x75, 0x61, 0x1b, 0xff, 0xb6, 0x4d, 0xc0, 0xd9, 0xca, 0x92, 0x97, 0xcd, 0x2c, 0x6a, 0x01 }
};

static const size_t key_len_test_data[NB_TESTS]   = { 16, 16, 16 };
static const size_t iv_len_test_data[NB_TESTS]    = { 12, 12, 12 };
static const size_t msg_len_test_data[NB_TESTS]   = { 16, 13, 32 };
static const size_t add_len_test_data[NB_TESTS]   = { 13, 16, 0 };
static const size_t tag_len_test_data[NB_TESTS]   = { 16, 16, 16 };
static const size_t crypt_len_test_data[NB_TESTS] = { 32, 29, 48 };

unsigned char cipher_tag_buffer[NB_TESTS][GCM_TEST_CT_MAX_LEN];
unsigned char decryption_output[NB_TESTS][GCM_TEST_CT_MAX_LEN];

void test_psa_gcm()
{
  struct test_vector vector[NB_TESTS];
  psa_status_t ret;

  /* psa crypto library initialization */
  ret = psa_crypto_init();
  if (ret == PSA_SUCCESS) {
    printf("\n PSA crypto library initialization Success \n");
  } else {
    printf("\n PSA crypto library initialization failed with error: %ld\n", ret);
  }

  for (int i = 0; i < NB_TESTS; i++) {
    memcpy(vector[i].key_test_data, key_test_data[i], key_len_test_data[i]);
    memcpy(vector[i].iv_test_data, iv_test_data[i], iv_len_test_data[i]);
    memcpy(vector[i].ad_test_data, ad_test_data[i], add_len_test_data[i]);
    memcpy(vector[i].msg_test_data, msg_test_data[i], msg_len_test_data[i]);
    memcpy(vector[i].res_test_data, res_test_data[i], crypt_len_test_data[i]);
    vector[i].tag_len = tag_len_test_data[i];
    size_t out_len;

    psa_key_id_t key_id;
    psa_key_attributes_t key_attr = psa_key_attributes_init();

    //Set up attributes for a AES GCM key
    psa_set_key_type(&key_attr, PSA_KEY_TYPE_AES);
    psa_set_key_usage_flags(&key_attr, PSA_KEY_USAGE_ENCRYPT | PSA_KEY_USAGE_DECRYPT);
    psa_set_key_algorithm(&key_attr, PSA_ALG_AEAD_WITH_SHORTENED_TAG(PSA_ALG_GCM, vector[i].tag_len));
#if WRAP_INPUT_KEYS
    psa_set_key_lifetime(&key_attr,
                         PSA_KEY_LIFETIME_FROM_PERSISTENCE_AND_LOCATION(PSA_KEY_PERSISTENCE_VOLATILE,
                                                                        PSA_KEY_VOLATILE_PERSISTENT_WRAP_IMPORT));
#elif IMPORT_WRAPPED_KEYS
    psa_set_key_lifetime(&key_attr,
                         PSA_KEY_LIFETIME_FROM_PERSISTENCE_AND_LOCATION(PSA_KEY_PERSISTENCE_VOLATILE,
                                                                        PSA_KEY_VOLATILE_PERSISTENT_WRAPPED));
#endif
    //Import a volatile key for AES GCM Key
    ret = psa_import_key(&key_attr, vector[i].key_test_data, sizeof(key_test_data[i]), &key_id);

    if (ret == PSA_SUCCESS) {
      printf("\n Key import Success \n");
    } else {
      printf("\n Key import Failed with error: %ld\n", ret);
    }

    ret = psa_aead_encrypt(key_id,
                           PSA_ALG_AEAD_WITH_SHORTENED_TAG(PSA_ALG_GCM, vector[i].tag_len),
                           vector[i].iv_test_data,
                           iv_len_test_data[i],
                           vector[i].ad_test_data,
                           add_len_test_data[i],
                           vector[i].msg_test_data,
                           msg_len_test_data[i],
                           cipher_tag_buffer[i],
                           crypt_len_test_data[i],
                           &out_len);

    if (ret == 0 && memcmp(cipher_tag_buffer[i], res_test_data[i], msg_len_test_data[i]) == 0) {
      printf("\n Encryption Success \n");
    } else {
      printf("\n Encryption Failed with error: %ld\n", ret);
    }
    memset(decryption_output[i], 0, GCM_TEST_PT_MAX_LEN);

    ret = psa_aead_decrypt(key_id,
                           PSA_ALG_AEAD_WITH_SHORTENED_TAG(PSA_ALG_GCM, vector[i].tag_len),
                           vector[i].iv_test_data,
                           iv_len_test_data[i],
                           vector[i].ad_test_data,
                           add_len_test_data[i],
                           cipher_tag_buffer[i],
                           crypt_len_test_data[i],
                           decryption_output[i],
                           msg_len_test_data[i],
                           &out_len);

    if (ret == 0 && memcmp(decryption_output[i], msg_test_data[i], msg_len_test_data[i]) == 0) {
      printf("\n Decryption Success \n");
    } else {
      printf("\n Decryption Failed with error: %ld\n", ret);
    }

    // Destroy plain key for AES GCM
    ret = psa_destroy_key(key_id);
    if (ret != PSA_SUCCESS) {
      printf("Destroy key failed with error: %ld\n", ret);
    }
  }
}
