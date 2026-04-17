/*******************************************************************************
 * @file  psa_sha_app.c
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
#include "psa_sha_app.h"
#include "psa/crypto.h"
#include <stdio.h>

#ifdef SL_SLI91x_SHA_1_EN
void test_psa_sha_1()
{
  psa_status_t ret;

  //! Test message for streaming
  char test_msg[] = { "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq" };

  //! Expected SHA1 hash value of test message
  uint8_t expect_sha1_hash[] = { 0x84, 0x98, 0x3e, 0x44, 0x1c, 0x3b, 0xd2, 0x6e, 0xba, 0xae,
                                 0x4a, 0xa1, 0xf9, 0x51, 0x29, 0xe5, 0xe5, 0x46, 0x70, 0xf1 };

  //! Hash Buffer
  uint8_t hash_buf[SL_SI91X_SHA_1_DIGEST_LEN] = { 0 };

  //! Hash length
  size_t hash_len;

  ret = psa_hash_compute(PSA_ALG_SHA_1,
                         (const unsigned char *)test_msg,
                         strlen(test_msg),
                         hash_buf,
                         sizeof(hash_buf),
                         &hash_len);

  printf("\n\r SHA 1 STATUS - %ld", ret);
  printf("\n\r SHA 1 Val - ");

  for (int i = 0; i < (int)hash_len; i++) {
    printf(" 0x%x", hash_buf[i]);
  }

  // Compare with the expected result
  if (ret != 0) {
    printf("\n SHA 1 Operation Failed Status 0x%lx \n", ret);
  } else if (ret == 0 && memcmp(hash_buf, expect_sha1_hash, sizeof(expect_sha1_hash)) == 0) {
    printf("\n SHA 1 Operation Success Status 0x%lx \n", ret);
  } else {
    printf("\n SHA 1 Compare with expected value failed \n");
  }
}
#endif

#ifdef SL_SLI91x_SHA_224_EN
void test_psa_sha_224()
{
  psa_status_t ret;

  //! Test message for streaming
  char test_msg[] = { "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq" };

  //! Expected SHA224 hash value of test message
  uint8_t expect_sha224_hash[] = { 0x75, 0x38, 0x8b, 0x16, 0x51, 0x27, 0x76, 0xcc, 0x5d, 0xba, 0x5d, 0xa1, 0xfd, 0x89,
                                   0x01, 0x50, 0xb0, 0xc6, 0x45, 0x5c, 0xb4, 0xf5, 0x8b, 0x19, 0x52, 0x52, 0x25, 0x25 };

  //! Hash Buffer
  uint8_t hash_buf[SL_SI91X_SHA_224_DIGEST_LEN] = { 0 };

  //! Hash length
  size_t hash_len;

  ret = psa_hash_compute(PSA_ALG_SHA_224,
                         (const unsigned char *)test_msg,
                         strlen(test_msg),
                         hash_buf,
                         sizeof(hash_buf),
                         &hash_len);

  printf("\n\r SHA 224 STATUS - %ld", ret);
  printf("\n\r SHA 224 Val - ");

  for (int i = 0; i < (int)hash_len; i++) {
    printf(" 0x%x", hash_buf[i]);
  }

  // Compare with the expected result
  if (ret != 0) {
    printf("\n SHA 224 Operation Failed Status 0x%lx \n", ret);
  } else if (ret == 0 && memcmp(hash_buf, expect_sha224_hash, sizeof(expect_sha224_hash)) == 0) {
    printf("\n SHA 224 Operation Success Status 0x%lx \n", ret);
  } else {
    printf("\n SHA 224 Compare with expected value failed \n");
  }
}
#endif

#ifdef SL_SLI91x_SHA_256_EN
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

  printf("\n\r SHA 256 STATUS - %ld", ret);
  printf("\n\r SHA 256 Val -");

  for (int i = 0; i < (int)hash_len; i++) {
    printf(" 0x%x", hash_buf[i]);
  }

  // Compare with the expected result
  if (ret != 0) {
    printf("\n SHA 256 Operation Failed Status 0x%lx \n", ret);
  } else if (ret == 0 && memcmp(hash_buf, expect_sha256_hash, sizeof(expect_sha256_hash)) == 0) {
    printf("\n SHA 256 Operation Success Status 0x%lx \n", ret);
  } else {
    printf("\n SHA 256 Compare with expected value failed \n");
  }
}
#endif

#ifdef SL_SLI91x_SHA_384_EN
void test_psa_sha_384()
{
  psa_status_t ret;

  //! Test message for streaming
  char test_msg[] = { "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq" };

  //! Expected SHA256 hash value of test message
  uint8_t expect_sha384_hash[] = { 0x33, 0x91, 0xfd, 0xdd, 0xfc, 0x8d, 0xc7, 0x39, 0x37, 0x07, 0xa6, 0x5b,
                                   0x1b, 0x47, 0x09, 0x39, 0x7c, 0xf8, 0xb1, 0xd1, 0x62, 0xaf, 0x05, 0xab,
                                   0xfe, 0x8f, 0x45, 0x0d, 0xe5, 0xf3, 0x6b, 0xc6, 0xb0, 0x45, 0x5a, 0x85,
                                   0x20, 0xbc, 0x4e, 0x6f, 0x5f, 0xe9, 0x5b, 0x1f, 0xe3, 0xc8, 0x45, 0x2b };

  //! Hash Buffer
  uint8_t hash_buf[SL_SI91X_SHA_384_DIGEST_LEN] = { 0 };

  //! Hash length
  size_t hash_len;

  ret = psa_hash_compute(PSA_ALG_SHA_384,
                         (const unsigned char *)test_msg,
                         strlen(test_msg),
                         hash_buf,
                         sizeof(hash_buf),
                         &hash_len);

  printf("\n\r SHA 384 STATUS - %ld", ret);
  printf("\n\r SHA 384 Val - ");

  for (int i = 0; i < (int)hash_len; i++) {
    printf(" 0x%x", hash_buf[i]);
  }

  // Compare with the expected result
  if (ret != 0) {
    printf("\n SHA 384 Operation Failed Status 0x%lx \n", ret);
  } else if (ret == 0 && memcmp(hash_buf, expect_sha384_hash, sizeof(expect_sha384_hash)) == 0) {
    printf("\n SHA 384 Operation Success Status 0x%lx \n", ret);
  } else {
    printf("\n SHA 384 Compare with expected value failed \n");
  }
}
#endif

#ifdef SL_SLI91x_SHA_512_EN
void test_psa_sha_512()
{
  psa_status_t ret;

  //! Test message for streaming
  char test_msg[] = { "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq" };

  //! Expected SHA256 hash value of test message
  uint8_t expect_sha512_hash[] = { 0x20, 0x4a, 0x8f, 0xc6, 0xdd, 0xa8, 0x2f, 0x0a, 0x0c, 0xed, 0x7b, 0xeb, 0x8e,
                                   0x08, 0xa4, 0x16, 0x57, 0xc1, 0x6e, 0xf4, 0x68, 0xb2, 0x28, 0xa8, 0x27, 0x9b,
                                   0xe3, 0x31, 0xa7, 0x03, 0xc3, 0x35, 0x96, 0xfd, 0x15, 0xc1, 0x3b, 0x1b, 0x07,
                                   0xf9, 0xaa, 0x1d, 0x3b, 0xea, 0x57, 0x78, 0x9c, 0xa0, 0x31, 0xad, 0x85, 0xc7,
                                   0xa7, 0x1d, 0xd7, 0x03, 0x54, 0xec, 0x63, 0x12, 0x38, 0xca, 0x34, 0x45 };

  //! Hash Buffer
  uint8_t hash_buf[SL_SI91X_SHA_512_DIGEST_LEN] = { 0 };

  //! Hash length
  size_t hash_len;

  ret = psa_hash_compute(PSA_ALG_SHA_512,
                         (const unsigned char *)test_msg,
                         strlen(test_msg),
                         hash_buf,
                         sizeof(hash_buf),
                         &hash_len);

  printf("\n\r SHA 512 STATUS - %ld", ret);
  printf("\n\r SHA 512 Val - ");

  for (int i = 0; i < (int)hash_len; i++) {
    printf(" 0x%x", hash_buf[i]);
  }

  // Compare with the expected result
  if (ret != 0) {
    printf("\n SHA 512 Operation Failed Status 0x%lx \n", ret);
  } else if (ret == 0 && memcmp(hash_buf, expect_sha512_hash, sizeof(expect_sha512_hash)) == 0) {
    printf("\n SHA 512 Operation Success Status 0x%lx \n", ret);
  } else {
    printf("\n SHA 512 Compare with expected value failed \n");
  }
}
#endif

void psa_app_process_action()
{
  psa_status_t ret;
  ret = psa_crypto_init();
  if (ret != PSA_SUCCESS) {
    printf("PSA Crypto Init failed with status : %ld\n", ret);
  } else {
    printf("PSA Crypto Init Success\n");
  }

#ifdef SL_SLI91x_SHA_1_EN
  test_psa_sha_1();
#endif
#ifdef SL_SLI91x_SHA_224_EN
  test_psa_sha_224();
#endif
#ifdef SL_SLI91x_SHA_256_EN
  test_psa_sha_256();
#endif
#ifdef SL_SLI91x_SHA_384_EN
  test_psa_sha_384();
#endif
#ifdef SL_SLI91x_SHA_512_EN
  test_psa_sha_512();
#endif
}
