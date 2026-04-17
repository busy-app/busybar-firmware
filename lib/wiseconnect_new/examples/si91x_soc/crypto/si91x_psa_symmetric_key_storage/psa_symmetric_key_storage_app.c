/*******************************************************************************
 * @file  psa_symmetric_key_storage_app.c
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
#include "psa_symmetric_key_storage_app.h"
#include "psa/crypto.h"
#include <stdio.h>

void test_import_and_export_volatile_plain_key()
{
  // 128-Bit Key
  uint8_t aes_ecb_key[16] = { 0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
                              0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f };
  uint8_t key_buf[16]     = { 0 };
  size_t key_len;

  psa_status_t ret;
  psa_key_id_t import_key_id;
  psa_key_attributes_t key_attr;

  // Set up attributes for a AES ECB key
  key_attr = psa_key_attributes_init();
  psa_set_key_type(&key_attr, PSA_KEY_TYPE_AES);
  psa_set_key_bits(&key_attr, sizeof(aes_ecb_key) * 8);
  psa_set_key_usage_flags(&key_attr, PSA_KEY_USAGE_EXPORT | PSA_KEY_USAGE_ENCRYPT | PSA_KEY_USAGE_DECRYPT);
  psa_set_key_algorithm(&key_attr, PSA_ALG_ECB_NO_PADDING);

  // Import a volatile plain key for AES ECB
  ret = psa_import_key(&key_attr, aes_ecb_key, sizeof(aes_ecb_key), &import_key_id);
  if (ret != PSA_SUCCESS) {
    printf("Import volatile plain Key failed with error: %ld\n", ret);
  } else {
    printf("Import Volatile Plain Key Success\n");
  }

  // Export a volatile plain key
  ret = psa_export_key(import_key_id, key_buf, sizeof(aes_ecb_key), &key_len);
  if (((memcmp(key_buf, aes_ecb_key, sizeof(aes_ecb_key))) != 0) || ret != PSA_SUCCESS) {
    printf("Export volatile plain key failed with error: %ld\n", ret);
  } else {
    printf("Export Volatile Plain Key Success\n");
  }

  // Destroy the volatile plain keys for AES ECB
  ret = psa_destroy_key(import_key_id);
  if (ret != PSA_SUCCESS) {
    printf("Destroy key failed with error: %ld\n", ret);
  } else {
    printf("Destroy Key Success\n");
  }
}

void test_generate_and_export_volatile_plain_key()
{
  uint8_t key_buf[16] = { 0 };
  size_t key_len;

  psa_status_t ret;
  psa_key_id_t generate_key_id;
  psa_key_attributes_t key_attr;

  // Set up attributes for a AES-ECB 128 bit key
  key_attr = psa_key_attributes_init();
  psa_set_key_type(&key_attr, PSA_KEY_TYPE_AES);
  psa_set_key_bits(&key_attr, 128);
  psa_set_key_usage_flags(&key_attr, PSA_KEY_USAGE_EXPORT | PSA_KEY_USAGE_ENCRYPT | PSA_KEY_USAGE_DECRYPT);
  psa_set_key_algorithm(&key_attr, PSA_ALG_ECB_NO_PADDING);

  // Generate a random volatile plain key for AES ECB
  ret = psa_generate_key(&key_attr, &generate_key_id);
  if (ret != PSA_SUCCESS) {
    printf("Generate volatile plain Key failed with error: %ld\n", ret);
  } else {
    printf("Generate Volatile Plain Key Success\n");
  }

  // Export a volatile Plain Key
  ret = psa_export_key(generate_key_id, key_buf, sizeof(key_buf), &key_len);
  if (ret != PSA_SUCCESS) {
    printf("Export volatile plain Key failed with error: %ld\n", ret);
  } else {
    printf("Export Volatile Plain Key Success\n");
  }

  // Destroy the volatile plain key
  ret = psa_destroy_key(generate_key_id);
  if (ret != PSA_SUCCESS) {
    printf("Destroy key failed with error: %ld\n", ret);
  } else {
    printf("Destroy Key Success\n");
  }
}

void test_import_and_export_persistent_plain_key()
{
  uint8_t aes_ecb_key[16] = { 0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
                              0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f };
  uint8_t key_buf[16]     = { 0 };
  size_t key_len;

  psa_status_t ret;
  psa_key_id_t import_key_id;
  psa_key_attributes_t key_attr;

  // Set up attributes for a AES ECB key
  key_attr = psa_key_attributes_init();
  psa_set_key_type(&key_attr, PSA_KEY_TYPE_AES);
  psa_set_key_bits(&key_attr, sizeof(aes_ecb_key) * 8);
  psa_set_key_usage_flags(&key_attr, PSA_KEY_USAGE_EXPORT | PSA_KEY_USAGE_ENCRYPT | PSA_KEY_USAGE_DECRYPT);
  psa_set_key_algorithm(&key_attr, PSA_ALG_ECB_NO_PADDING);

  // Import a persistent plain key for AES ECB (ID = 0x02)
  psa_set_key_id(&key_attr, 0x02);
  ret = psa_import_key(&key_attr, aes_ecb_key, sizeof(aes_ecb_key), &import_key_id);
  if (ret != PSA_SUCCESS) {
    printf("Import persistent plain Key failed with error: %ld\n", ret);
  } else {
    printf("Import Persistent Plain Key Success\n");
  }

  // Export a persistent plain key for AES ECB (ID = 0x02)
  ret = psa_export_key(import_key_id, key_buf, sizeof(aes_ecb_key), &key_len);
  if (((memcmp(key_buf, aes_ecb_key, sizeof(aes_ecb_key))) != 0) || ret != PSA_SUCCESS) {
    printf("Export persistent plain key failed with error: %ld\n", ret);
  } else {
    printf("Export Persistent Plain Key Success\n");
  }

  // Destroy the persistent plain keys for AES ECB
  ret = psa_destroy_key(import_key_id);
  if (ret != PSA_SUCCESS) {
    printf("Destroy key failed with error: %ld\n", ret);
  } else {
    printf("Destroy Key Success\n");
  }
}

void test_generate_and_export_persistent_plain_key()
{
  uint8_t key_buf[16] = { 0 };
  size_t key_len;

  psa_status_t ret;
  psa_key_id_t generate_key_id;
  psa_key_attributes_t key_attr;

  // Set up attributes for a AES ECB 128 bit key
  key_attr = psa_key_attributes_init();
  psa_set_key_type(&key_attr, PSA_KEY_TYPE_AES);
  psa_set_key_bits(&key_attr, 128);
  psa_set_key_usage_flags(&key_attr, PSA_KEY_USAGE_EXPORT | PSA_KEY_USAGE_ENCRYPT | PSA_KEY_USAGE_DECRYPT);
  psa_set_key_algorithm(&key_attr, PSA_ALG_ECB_NO_PADDING);

  // Generate a random persistent plain key for AES ECB (ID = 0x03)
  psa_set_key_id(&key_attr, 0x03);
  ret = psa_generate_key(&key_attr, &generate_key_id);
  if (ret != PSA_SUCCESS) {
    printf("Generate persistent plain Key failed with error: %ld\n", ret);
  } else {
    printf("Generate Persistent Plain Key Success\n");
  }

  // Export a persistent plain key for AES ECB (ID = 0x03)
  ret = psa_export_key(generate_key_id, key_buf, sizeof(key_buf), &key_len);
  if (ret != PSA_SUCCESS) {
    printf("Export persistent plain Key failed with error: %ld\n", ret);
  } else {
    printf("Export Persistent Plain Key Success\n");
  }

  // Destroy the persistent plain keys for AES ECB
  ret = psa_destroy_key(generate_key_id);
  if (ret != PSA_SUCCESS) {
    printf("Destroy key failed with error: %ld\n", ret);
  } else {
    printf("Destroy Key Success\n");
  }
}

void test_psa_symmetric_persistent_plain_key_storage()
{
  test_import_and_export_persistent_plain_key();
  test_generate_and_export_persistent_plain_key();
}

void test_psa_symmetric_volatile_plain_key_storage()
{
  test_import_and_export_volatile_plain_key();
  test_generate_and_export_volatile_plain_key();
}

void psa_its_app_process_action()
{
  psa_status_t ret;
  ret = psa_crypto_init();
  if (ret != PSA_SUCCESS) {
    printf("PSA Crypto Init failed with status : %ld\n", ret);
  } else {
    printf("PSA Crypto Init Success\n");
  }
  test_psa_symmetric_volatile_plain_key_storage();
  test_psa_symmetric_persistent_plain_key_storage();
}