/*******************************************************************************
 * @file  psa_aes_app.c
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
#include "psa_aes_app.h"
#include "psa/crypto.h"
#include <stdio.h>
#include <sl_constants.h>
#include <sl_si91x_crypto.h>

// Size of first chunk of data
#define FIRST_CHUNK_SIZE 16

// Size of middle chunks of data
#define MIDDLE_CHUNK_SIZE 32

/* Test vector (AES) for  AES-128/256 key */
// 128 Key Size
const uint8_t aes_key_16[KEY_SIZE_16] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                                          0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f };

// 256 Key Size
const uint8_t aes_key_32[KEY_SIZE_32] = { 0x60, 0x3d, 0xeb, 0x10, 0x15, 0xca, 0x71, 0xbe, 0x2b, 0x73, 0xae,
                                          0xf0, 0x85, 0x7d, 0x77, 0x81, 0x1f, 0x35, 0x2c, 0x07, 0x3b, 0x61,
                                          0x08, 0xd7, 0x2d, 0x98, 0x10, 0xa3, 0x09, 0x14, 0xdf, 0xf4 };

//! IV should be 16 Bytes/128 bits
const uint8_t iv_test_data[SL_SI91X_IV_SIZE] = { 0x22, 0x11, 0x1a, 0x70, 0x35, 0x87, 0xba, 0x70,
                                                 0x90, 0x54, 0x33, 0x9e, 0x76, 0xfd, 0xd1, 0xef };

static unsigned char msg_test_data[AES_TEST_PT_MAX_LEN] = {
  0x7f, 0x1a, 0x4b, 0x20, 0xd8, 0xc3, 0xf5, 0xa1, 0xb9, 0xe6, 0xd0, 0xc7, 0x89, 0xa2, 0xe3, 0xd1, 0xa8, 0xb1,
  0xc4, 0xf0, 0xe2, 0xb7, 0xa9, 0xc6, 0xf8, 0xa1, 0xd3, 0xb5, 0xe4, 0xc2, 0xa8, 0xf1, 0xc3, 0xd2, 0xe9, 0xb6,
  0xf7, 0xc4, 0xd5, 0xa1, 0xf3, 0xa9, 0xc5, 0xd8, 0xb2, 0xe1, 0xa7, 0xf4, 0xc9, 0xd2, 0xa0, 0xb7, 0xf9, 0xd3,
  0xc6, 0xe8, 0xa2, 0xb3, 0xf1, 0xc7, 0xd2, 0xe9, 0xa4, 0xb5, 0xc8, 0xa6, 0xe3, 0xd9, 0xf0, 0xc5, 0xb1, 0xa2,
  0xc4, 0xd7, 0xf6, 0xe9, 0xa0, 0xc8, 0xb3, 0xf2, 0xd7, 0xe4, 0xc1, 0xa5, 0xf9, 0xc8, 0xd0, 0xb6, 0xf7, 0xa1,
  0xc9, 0xe2, 0x7a, 0x1b, 0x3c, 0x2f, 0x5e, 0x78, 0x49, 0x3a, 0x62, 0x71, 0x9f, 0x8a, 0x16, 0x3d, 0xac, 0x11,
  0x1b, 0xd5, 0x8d, 0x50, 0x9c, 0x87, 0x64, 0xb0, 0x32, 0xe7, 0xb9, 0x6a, 0xd6, 0x0c, 0x59, 0x81, 0x34, 0xb8,
  0x27, 0xf5, 0x99, 0x42, 0x76, 0xc0, 0x3e, 0x2a, 0x53, 0x5f, 0xd4, 0x01, 0xc8, 0xb4, 0x8b, 0x24, 0x70, 0xe0
};

static const unsigned char res_test_data[NB_TESTS][AES_TEST_CT_MAX_LEN] = {
  { 0x4d, 0x1c, 0xb2, 0xfd, 0x85, 0xce, 0x19, 0x89, 0x60, 0x0d, 0x66, 0x34, 0xf2, 0x96, 0xd6, 0xe2, 0xfe, 0xd6,
    0xa0, 0x4e, 0xdb, 0x03, 0xf5, 0xe3, 0xb1, 0x0b, 0x07, 0xc4, 0xc9, 0x26, 0x86, 0x05, 0xf7, 0x78, 0xce, 0xbf,
    0xb8, 0x2b, 0xfd, 0x3a, 0xfd, 0xe7, 0xe1, 0x0a, 0xef, 0x2d, 0x79, 0xb0, 0xcf, 0x74, 0x51, 0x29, 0x7b, 0x90,
    0x19, 0x4f, 0xc1, 0x5f, 0xd9, 0xfe, 0xb9, 0xb1, 0x2b, 0x69, 0x28, 0xd4, 0x7f, 0x53, 0xd8, 0x90, 0xc6, 0xa1,
    0x71, 0xa6, 0x38, 0x07, 0xc9, 0xdb, 0xf1, 0xbf, 0x6d, 0x63, 0x4d, 0x4b, 0xb1, 0x62, 0x5f, 0x92, 0xb1, 0x53,
    0xc8, 0xa2, 0x95, 0x27, 0x1e, 0xf4, 0xc8, 0xe3, 0x72, 0x8a, 0x89, 0x16, 0xa9, 0x1b, 0x2a, 0x73, 0xc1, 0xa2,
    0xd9, 0xaf, 0x17, 0x00, 0x65, 0x38, 0x3e, 0x6e, 0x4c, 0xd5, 0xfe, 0x80, 0xac, 0xc5, 0xcd, 0x92, 0xc9, 0xa7,
    0xca, 0xf8, 0xf5, 0xe0, 0x72, 0xbf, 0x57, 0xe9, 0xc6, 0x44, 0x50, 0x87, 0xc1, 0x2a, 0xce, 0x85, 0x3c, 0x41 },
  { 0xdb, 0x36, 0x32, 0x05, 0x9c, 0x21, 0x40, 0x17, 0x21, 0x7a, 0x67, 0x10, 0x2e, 0x26, 0xfd, 0x4b, 0x25, 0xb3,
    0xd9, 0x1b, 0x05, 0x9c, 0x39, 0xcb, 0xe2, 0xf1, 0x00, 0x47, 0x77, 0x08, 0x96, 0xe6, 0x0f, 0x62, 0xc3, 0x22,
    0x5b, 0x00, 0xff, 0x99, 0x35, 0xa8, 0xf9, 0xca, 0xc3, 0x01, 0x91, 0xa4, 0xf4, 0x4d, 0x55, 0x1d, 0xab, 0x47,
    0xb4, 0x91, 0x36, 0x22, 0x3c, 0x39, 0x17, 0x54, 0x89, 0x07, 0xac, 0xce, 0xd5, 0x33, 0x27, 0x9f, 0xa6, 0x02,
    0x3b, 0xc9, 0xad, 0xaa, 0x4b, 0xde, 0x7c, 0x92, 0x65, 0x4f, 0xfb, 0x66, 0x72, 0xcd, 0x59, 0xc1, 0x5b, 0xd3,
    0xe6, 0x9d, 0x89, 0xeb, 0xbd, 0x03, 0x25, 0x49, 0x87, 0x9b, 0x82, 0x03, 0x77, 0x3b, 0x15, 0x3b, 0x52, 0x59,
    0x61, 0xd9, 0xac, 0xac, 0x60, 0xd7, 0x42, 0x14, 0x14, 0x4b, 0xdc, 0xab, 0xb7, 0x3b, 0x9a, 0x05, 0xc0, 0x43,
    0x4f, 0xcc, 0x72, 0xc3, 0x96, 0xa9, 0x27, 0x96, 0x29, 0x68, 0x33, 0x3e, 0x38, 0xfb, 0xfc, 0xd1, 0xc1, 0xb1 }
};

static const size_t key_len_test_data[NB_TESTS] = { 16, 32 };

// The state object for multi-part cipher operations
static psa_cipher_operation_t cipher_op = { 0 };

void test_psa_aes(psa_algorithm_t alg)
{
  /* Local variables */
  psa_status_t status;
  uint8_t encryption_output[PAYLOAD_SIZE] = { 0 };
  uint8_t decryption_output[PAYLOAD_SIZE] = { 0 };
  uint32_t stream_block_size              = MIDDLE_CHUNK_SIZE;

  /* psa crypto library initialization */
  status = psa_crypto_init();
  if (status == PSA_SUCCESS) {
    printf("\n PSA crypto library initialization Success \n");
  } else {
    printf("\n PSA crypto library initialization failed with error: %ld\n", status);
    SL_ASSERT(FALSE);
  }

  for (int i = 0; i < NB_TESTS; i++) {

    psa_key_id_t key_id;
    psa_key_attributes_t key_attr;
    size_t out_len   = 0;
    size_t out_total = 0;

    // Set up attributes for a AES key
    key_attr = psa_key_attributes_init();
    psa_set_key_type(&key_attr, PSA_KEY_TYPE_AES);
    psa_set_key_usage_flags(&key_attr, PSA_KEY_USAGE_ENCRYPT | PSA_KEY_USAGE_DECRYPT);
    psa_set_key_algorithm(&key_attr, alg);

    /* Import a volatile plain key for AES */
    if (i == 0) { // 128 key size
      printf("\n\n Encryption/Decryption with %d size key for AES \n\n", (key_len_test_data[i] * 8));
      printf("\n Import a volatile plain %d size key for AES \n", (key_len_test_data[i] * 8));
      status = psa_import_key(&key_attr, aes_key_16, sizeof(aes_key_16), &key_id);
    } else if (i == 1) { // 256 key size
      printf("\n\n Encryption/Decryption with %d size key for AES \n\n", (key_len_test_data[i] * 8));
      printf("\n Import a volatile plain %d size key for AES \n", (key_len_test_data[i] * 8));
      status = psa_import_key(&key_attr, aes_key_32, sizeof(aes_key_32), &key_id);
    }

    /* Check for import key status */
    if (status == PSA_SUCCESS) {
      printf("\n Key import Success \n");
    } else {
      printf("\n Key import Failed with error: %ld\n", status);
      SL_ASSERT(FALSE);
    }

    /* Encryption */

    cipher_op = psa_cipher_operation_init();

    status = psa_cipher_encrypt_setup(&cipher_op, key_id, PSA_ALG_CTR);
    if (status == PSA_SUCCESS) {
      printf("\n Encryption Setup Success \n");
    } else {
      printf("\n Encryption Setup Failed with error: %ld\n", status);
      SL_ASSERT(FALSE);
    }

    status = psa_cipher_set_iv(&cipher_op, iv_test_data, sizeof(iv_test_data));

    if (status == PSA_SUCCESS) {
      printf("\n IV Set Success \n");
    } else {
      printf("\n IV Set Failed with error: %ld\n", status);
      SL_ASSERT(FALSE);
    }

    out_total = 0; // 1st block
    status    = psa_cipher_update(&cipher_op,
                               msg_test_data + out_total,
                               FIRST_CHUNK_SIZE,
                               encryption_output + out_total,
                               sizeof(encryption_output) - out_total,
                               &out_len);
    if (status == PSA_SUCCESS) {
      printf("\n First Chunk Encryption Success \n");
    } else {
      printf("\n First Chunk Encryption Failed with error: %ld\n", status);
      SL_ASSERT(FALSE);
    }
    out_total += out_len;

    // Streaming block
    for (; out_total < PAYLOAD_SIZE; out_total = out_total + stream_block_size) {
      status = psa_cipher_update(&cipher_op,
                                 msg_test_data + out_total,
                                 stream_block_size,
                                 encryption_output + out_total,
                                 sizeof(encryption_output) - out_total,
                                 &out_len);
      if (status == PSA_SUCCESS) {
        printf("\n Middle Chunk Encryption Success \n");
      } else {
        printf("\n Middle Chunk Encryption Failed with error: %ld\n", status);
        SL_ASSERT(FALSE);
      }
    }

    status =
      psa_cipher_finish(&cipher_op, encryption_output + out_total, sizeof(encryption_output) - out_total, &out_len);

    if (status == PSA_SUCCESS && memcmp(encryption_output, res_test_data[i], sizeof(msg_test_data)) == 0) {
      printf("\n Multipart Encryption Success \n");
    } else {
      printf("\n Multipart Encryption Failed with error: %ld\n", status);
      SL_ASSERT(FALSE);
    }

    /* Decryption */

    cipher_op = psa_cipher_operation_init();

    status = psa_cipher_decrypt_setup(&cipher_op, key_id, PSA_ALG_CTR);
    if (status == PSA_SUCCESS) {
      printf("\n Decryption Setup Success \n");
    } else {
      printf("\n Decryption Setup Failed with error: %ld\n", status);
      SL_ASSERT(FALSE);
    }

    status = psa_cipher_set_iv(&cipher_op, iv_test_data, sizeof(iv_test_data));
    if (status == PSA_SUCCESS) {
      printf("\n IV Set Success \n");
    } else {
      printf("\n IV Set Failed with error: %ld\n", status);
      SL_ASSERT(FALSE);
    }

    out_total = 0; // 1st block
    status    = psa_cipher_update(&cipher_op,
                               encryption_output + out_total,
                               16,
                               decryption_output + out_total,
                               sizeof(decryption_output) - out_total,
                               &out_len);

    if (status == PSA_SUCCESS) {
      printf("\n First Chunk Decryption Success \n");
    } else {
      printf("\n First Chunk Decryption Failed with error: %ld\n", status);
      SL_ASSERT(FALSE);
    }
    out_total += out_len;

    // Streaming block
    for (; out_total < PAYLOAD_SIZE; out_total = out_total + stream_block_size) {
      status = psa_cipher_update(&cipher_op,
                                 encryption_output + out_total,
                                 stream_block_size,
                                 decryption_output + out_total,
                                 sizeof(decryption_output) - out_total,
                                 &out_len);
      if (status == PSA_SUCCESS) {
        printf("\n Middle Chunk Decryption Success \n");
      } else {
        printf("\n Middle Chunk Decryption Failed with error: %ld\n", status);
        SL_ASSERT(FALSE);
      }
    }

    status =
      psa_cipher_finish(&cipher_op, decryption_output + out_total, sizeof(decryption_output) - out_total, &out_len);

    if (status == PSA_SUCCESS && memcmp(decryption_output, msg_test_data, sizeof(msg_test_data)) == 0) {
      printf("\n Multipart Decryption Success \n");
    } else {
      printf("\n Multipart Decryption Failed with error: %ld\n", status);
      SL_ASSERT(FALSE);
    }

    /* Destroy a volatile plain key for AES */
    status = psa_destroy_key(key_id);
    if (status != PSA_SUCCESS) {
      printf("Destroy key failed with error: %ld\n", status);
    }
  }
}
