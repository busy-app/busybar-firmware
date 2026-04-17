/*******************************************************************************
 * @file  psa_ccm_app.c
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
#include "psa_ccm_app.h"
#include "psa/crypto.h"
#include "sl_si91x_psa_wrap.h"
#include <stdio.h>

#define WRAP_INPUT_KEYS     0 // Enable this if the input key needs to be wrapped before use
#define IMPORT_WRAPPED_KEYS 0 // Enable this if the input key is wrapped

static const unsigned char key_test_data[] = { 0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
                                               0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f };

static const unsigned char iv_test_data[] = { 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b };

static const unsigned char ad_test_data[] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09,
                                              0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13 };

static const unsigned char msg_test_data[CCM_TEST_PT_MAX_LEN] = {
  0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b,
  0x2c, 0x2d, 0x2e, 0x2f, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
};

static const size_t iv_len_test_data[3]    = { 7, 8, 12 };
static const size_t msg_len_test_data[3]   = { 4, 16, 24 };
static const size_t add_len_test_data[3]   = { 8, 16, 20 };
static const size_t tag_len_test_data[3]   = { 4, 6, 8 };
static const size_t crypt_len_test_data[3] = { 8, 22, 32 };

static const unsigned char res_test_data[3][CCM_TEST_CT_MAX_LEN] = {
  { 0x71, 0x62, 0x01, 0x5b, 0x4d, 0xac, 0x25, 0x5d },
  { 0xd2, 0xa1, 0xf0, 0xe0, 0x51, 0xea, 0x5f, 0x62, 0x08, 0x1a, 0x77,
    0x92, 0x07, 0x3d, 0x59, 0x3d, 0x1f, 0xc6, 0x4f, 0xbf, 0xac, 0xcd },
  { 0xe3, 0xb2, 0x01, 0xa9, 0xf5, 0xb7, 0x1a, 0x7a, 0x9b, 0x1c, 0xea, 0xec, 0xcd, 0x97, 0xe7, 0x0b,
    0x61, 0x76, 0xaa, 0xd9, 0xa4, 0x42, 0x8a, 0xa5, 0x48, 0x43, 0x92, 0xfb, 0xc1, 0xb0, 0x99, 0x51 }
};

unsigned char cipher_tag_buffer[3][CCM_TEST_CT_MAX_LEN];
unsigned char decryption_output[3][CCM_TEST_CT_MAX_LEN];

void test_psa_ccm()
{
  struct test_vector vector[3];
  psa_status_t ret;

  /* psa crypto library initialization */
  ret = psa_crypto_init();
  if (ret == PSA_SUCCESS) {
    printf("\n PSA crypto library initialization Success \n");
  } else {
    printf("\n PSA crypto library initialization failed with error: %ld\n", ret);
  }

  for (int i = 0; i < NB_TESTS; i++) {
    memcpy(vector[i].iv_test_data, iv_test_data, iv_len_test_data[i]);
    memcpy(vector[i].ad_test_data, ad_test_data, add_len_test_data[i]);
    memcpy(vector[i].msg_test_data, msg_test_data, msg_len_test_data[i]);
    memcpy(vector[i].res_test_data, res_test_data[i], crypt_len_test_data[i]);
    vector[i].tag_len = tag_len_test_data[i];
    size_t out_len;

    psa_key_id_t key_id;
    psa_key_attributes_t key_attr = psa_key_attributes_init();

    //Set up attributes for a AES CCM key
    psa_set_key_type(&key_attr, PSA_KEY_TYPE_AES);
    psa_set_key_usage_flags(&key_attr, PSA_KEY_USAGE_ENCRYPT | PSA_KEY_USAGE_DECRYPT | PSA_KEY_USAGE_EXPORT);
    psa_set_key_algorithm(&key_attr, PSA_ALG_AEAD_WITH_SHORTENED_TAG(PSA_ALG_CCM, vector[i].tag_len));
#if WRAP_INPUT_KEYS
    psa_set_key_lifetime(&key_attr,
                         PSA_KEY_LIFETIME_FROM_PERSISTENCE_AND_LOCATION(PSA_KEY_PERSISTENCE_VOLATILE,
                                                                        PSA_KEY_VOLATILE_PERSISTENT_WRAP_IMPORT));
#elif IMPORT_WRAPPED_KEYS
    psa_set_key_lifetime(&key_attr,
                         PSA_KEY_LIFETIME_FROM_PERSISTENCE_AND_LOCATION(PSA_KEY_PERSISTENCE_VOLATILE,
                                                                        PSA_KEY_VOLATILE_PERSISTENT_WRAPPED));
#endif
    //Import a volatile plain/wrapped key for AES CCM Key
    ret = psa_import_key(&key_attr, key_test_data, sizeof(key_test_data), &key_id);

    if (ret == PSA_SUCCESS) {
      printf("\n Key import Success \n");
    } else {
      printf("\n Key import Failed with error: %ld\n", ret);
    }

    ret = psa_aead_encrypt(key_id,
                           PSA_ALG_AEAD_WITH_SHORTENED_TAG(PSA_ALG_CCM, vector[i].tag_len),
                           vector[i].iv_test_data,
                           iv_len_test_data[i],
                           vector[i].ad_test_data,
                           add_len_test_data[i],
                           vector[i].msg_test_data,
                           msg_len_test_data[i],
                           cipher_tag_buffer[i],
                           crypt_len_test_data[i],
                           &out_len);

    if (ret == 0 && memcmp(cipher_tag_buffer[i], res_test_data[i], crypt_len_test_data[i]) == 0) {
      printf("\n Encryption Success \n");
    } else {
      printf("\n Encryption Failed with error: %ld\n", ret);
    }

    memset(decryption_output[i], 0, CCM_TEST_PT_MAX_LEN);

    ret = psa_aead_decrypt(key_id,
                           PSA_ALG_AEAD_WITH_SHORTENED_TAG(PSA_ALG_CCM, vector[i].tag_len),
                           vector[i].iv_test_data,
                           iv_len_test_data[i],
                           vector[i].ad_test_data,
                           add_len_test_data[i],
                           cipher_tag_buffer[i],
                           crypt_len_test_data[i],
                           decryption_output[i],
                           msg_len_test_data[i],
                           &out_len);

    if (ret == 0 && memcmp(decryption_output[i], msg_test_data, msg_len_test_data[i]) == 0) {
      printf("\n Decryption Success \n");
    } else {
      printf("\n Decryption Failed with error: %ld\n", ret);
    }

    // Destroy plain key for AES CCM
    ret = psa_destroy_key(key_id);
    if (ret != PSA_SUCCESS) {
      printf("Destroy key failed with error: %ld\n", ret);
    }
  }
}
