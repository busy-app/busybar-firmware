/*******************************************************************************
 * @file  psa_asymmetric_key_storage_app.c
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
#include "psa_asymmetric_key_storage_app.h"
#include "psa/crypto.h"
#include <stdio.h>

void test_import_and_export_volatile_plain_key()
{
  /* Test Key */
  static const unsigned char private_key[] = { 0xdb, 0x2b, 0x7a, 0x94, 0xf5, 0x1a, 0x6a, 0x6c, 0x81, 0xa6, 0xbb,
                                               0x32, 0x14, 0xe5, 0xca, 0x01, 0xd9, 0x10, 0x1a, 0x54, 0xdf, 0xbd,
                                               0x9f, 0x49, 0xbe, 0xd9, 0x45, 0x4c, 0x7e, 0x13, 0x17, 0xa7 };

  uint8_t key_buffer[KEY_MAX_SIZE];
  size_t len;

  psa_status_t ret;
  psa_key_id_t import_key_id;
  psa_key_attributes_t key_attr;

  /* Set up attributes for asymmetric ecc key - private key */
  key_attr = psa_key_attributes_init();
  psa_set_key_type(&key_attr, PSA_KEY_TYPE_ECC_KEY_PAIR(PSA_ECC_FAMILY_SECP_R1));
  psa_set_key_usage_flags(&key_attr, PSA_KEY_USAGE_EXPORT | PSA_KEY_USAGE_DERIVE);
  psa_set_key_bits(&key_attr, KEY_SIZE_BITS);
  psa_set_key_algorithm(&key_attr, PSA_ALG_ECDH);

  /* Import a private key */
  ret = psa_import_key(&key_attr, private_key, sizeof(private_key), &import_key_id);
  if (ret != PSA_SUCCESS) {
    printf("Import Private Key failed with error: status %ld\n", ret);
  } else {
    printf("Import Private Key success\n");
  }

  /* Export the private key */
  ret = psa_export_key(import_key_id, key_buffer, sizeof(key_buffer), &len);
  if (ret != PSA_SUCCESS || memcmp(key_buffer, private_key, len) != 0) {
    printf("Export Private Key failed with error: status %ld\n", ret);
  } else {
    printf("Export Private Key success\n");
  }

  /* Uncomment below lines to print the private key */
  //  printf("The exported private key is\n");
  //  for (int i = 0; i < (int)len; i++) {
  //    printf("%x ", key_buffer[i]);
  //  }
  //  printf("\n");

  /* Generate and export the asymmetric public key */
  ret = psa_export_public_key(import_key_id, key_buffer, sizeof(key_buffer), &len);
  if (ret != PSA_SUCCESS) {
    printf("Export Public Key failed with error: status %ld\n", ret);
  } else {
    printf("Export Public Key success\n");
  }

  /* Uncomment below lines to print the public key */
  //  printf("The exported public key is\n");
  //  for (int i = 0; i < (int)len; i++) {
  //    printf("%x ", key_buffer[i]);
  //  }
  //  printf("\n");

  // Destroy key
  ret = psa_destroy_key(import_key_id);
  if (ret != PSA_SUCCESS) {
    printf("Destroy key failed with error: %ld\n", ret);
  } else {
    printf("Destroy Key Success\n");
  }
}

void test_generate_and_export_volatile_plain_key()
{
  uint8_t key_buffer[KEY_MAX_SIZE];
  size_t len;
  psa_status_t ret;
  psa_key_id_t generate_key_id;
  psa_key_attributes_t key_attr;

  /* Set up attributes for asymmetric ecc key - private key */
  key_attr = psa_key_attributes_init();
  psa_set_key_type(&key_attr, PSA_KEY_TYPE_ECC_KEY_PAIR(PSA_ECC_FAMILY_SECP_R1));
  psa_set_key_usage_flags(&key_attr, PSA_KEY_USAGE_EXPORT | PSA_KEY_USAGE_DERIVE);
  psa_set_key_bits(&key_attr, KEY_SIZE_BITS);
  psa_set_key_algorithm(&key_attr, PSA_ALG_ECDH);

  /* Generate a private key */
  ret = psa_generate_key(&key_attr, &generate_key_id);
  if (ret != PSA_SUCCESS) {
    printf("Generate Private Key failed with error: status %ld\n", ret);
  } else {
    printf("Generate Private Key success\n");
  }

  /* Export the private key */
  ret = psa_export_key(generate_key_id, key_buffer, sizeof(key_buffer), &len);
  if (ret != PSA_SUCCESS) {
    printf("Export Private Key failed with error: status %ld\n", ret);
  } else {
    printf("Export Private Key success\n");
  }

  /* Uncomment below lines to print the private key */
  //  printf("The exported private key is\n");
  //  for (uint8_t i = 0; i < (int)len; i++) {
  //    printf("%x ", key_buffer[i]);
  //  }
  //  printf("\n");

  /* Generate and export the asymmetric public key */
  ret = psa_export_public_key(generate_key_id, key_buffer, sizeof(key_buffer), &len);
  if (ret != PSA_SUCCESS) {
    printf("Export Public Key failed with error: status %ld\n", ret);
  } else {
    printf("Export Public Key success\n");
  }

  /* Uncomment below lines to print the public key */
  //  printf("The exported public key is\n");
  //  for (int i = 0; i < (int)len; i++) {
  //    printf("%x ", key_buffer[i]);
  //  }
  //  printf("\n");

  /* Destroy key */
  ret = psa_destroy_key(generate_key_id);
  if (ret != PSA_SUCCESS) {
    printf("Destroy Key failed with error: %ld\n", ret);
  } else {
    printf("Destroy Key success\n");
  }
}

void test_import_and_export_persistent_plain_key()
{
  /* Test Key */
  static const unsigned char private_key[] = { 0xdb, 0x2b, 0x7a, 0x94, 0xf5, 0x1a, 0x6a, 0x6c, 0x81, 0xa6, 0xbb,
                                               0x32, 0x14, 0xe5, 0xca, 0x01, 0xd9, 0x10, 0x1a, 0x54, 0xdf, 0xbd,
                                               0x9f, 0x49, 0xbe, 0xd9, 0x45, 0x4c, 0x7e, 0x13, 0x17, 0xa7 };

  uint8_t key_buffer[KEY_MAX_SIZE];
  size_t len;

  psa_status_t ret;
  psa_key_attributes_t key_attr;
  psa_key_id_t import_key_id = PERSISTENT_KEY_ID_1;

  /* Set up attributes for asymmetric ecc key - private key */
  key_attr = psa_key_attributes_init();
  psa_set_key_id(&key_attr, PERSISTENT_KEY_ID_1);
  psa_set_key_type(&key_attr, PSA_KEY_TYPE_ECC_KEY_PAIR(PSA_ECC_FAMILY_SECP_R1));
  psa_set_key_usage_flags(&key_attr, PSA_KEY_USAGE_EXPORT | PSA_KEY_USAGE_DERIVE);
  psa_set_key_bits(&key_attr, KEY_SIZE_BITS);
  psa_set_key_algorithm(&key_attr, PSA_ALG_ECDH);

  /* Import a private key */
  ret = psa_import_key(&key_attr, private_key, sizeof(private_key), &import_key_id);
  if (ret != PSA_SUCCESS) {
    printf("Import Private Key failed with error: status %ld\n", ret);
  } else {
    printf("Import Private Key success\n");
  }

  /* Export the private key */
  ret = psa_export_key(import_key_id, key_buffer, sizeof(key_buffer), &len);
  if (ret != PSA_SUCCESS || memcmp(key_buffer, private_key, len) != 0) {
    printf("Export Private Key failed with error: status %ld\n", ret);
  } else {
    printf("Export Private Key success\n");
  }

  /* Uncomment below lines to print the private key */
  //  printf("The exported private key is\n");
  //  for (int i = 0; i < (int)len; i++) {
  //    printf("%x ", key_buffer[i]);
  //  }
  //  printf("\n");

  /* Generate and export the asymmetric public key */
  ret = psa_export_public_key(import_key_id, key_buffer, sizeof(key_buffer), &len);
  if (ret != PSA_SUCCESS) {
    printf("Export Public Key failed with error: status %ld\n", ret);
  } else {
    printf("Export Public Key success\n");
  }

  /* Uncomment below lines to print the public key */
  //  printf("The exported public key is\n");
  //  for (uint8_t i = 0; i < (int)len; i++) {
  //    printf("%x ", key_buffer[i]);
  //  }
  //  printf("\n");

  // Destroy key
  ret = psa_destroy_key(import_key_id);
  if (ret != PSA_SUCCESS) {
    printf("Destroy key failed with error: %ld\n", ret);
  } else {
    printf("Destroy Key Success\n");
  }
}

void test_generate_and_export_persistent_plain_key()
{
  uint8_t key_buffer[KEY_MAX_SIZE];
  size_t len;

  psa_status_t ret;
  psa_key_attributes_t key_attr;
  psa_key_id_t generate_key_id = PERSISTENT_KEY_ID_2;

  /* Set up attributes for asymmetric ecc key - private key */
  key_attr = psa_key_attributes_init();
  psa_set_key_id(&key_attr, PERSISTENT_KEY_ID_2);
  psa_set_key_type(&key_attr, PSA_KEY_TYPE_ECC_KEY_PAIR(PSA_ECC_FAMILY_SECP_R1));
  psa_set_key_usage_flags(&key_attr, PSA_KEY_USAGE_EXPORT | PSA_KEY_USAGE_DERIVE);
  psa_set_key_bits(&key_attr, KEY_SIZE_BITS);
  psa_set_key_algorithm(&key_attr, PSA_ALG_ECDH);

  /* Generate a private key */
  ret = psa_generate_key(&key_attr, &generate_key_id);
  if (ret != PSA_SUCCESS) {
    printf("Generate Private Key failed with error: status %ld\n", ret);
  } else {
    printf("Generate Private Key success\n");
  }

  /* Export the private key */
  ret = psa_export_key(generate_key_id, key_buffer, sizeof(key_buffer), &len);
  if (ret != PSA_SUCCESS) {
    printf("Export Private Key failed with error: status %ld\n", ret);
  } else {
    printf("Export Private Key success\n");
  }

  /* Uncomment below lines to print the private key */
  //  printf("The exported private key is\n");
  //  for (uint8_t i = 0; i < (int)len; i++) {
  //    printf("%x ", key_buffer[i]);
  //  }
  //  printf("\n");

  /* Generate and export the asymmetric public key */
  ret = psa_export_public_key(generate_key_id, key_buffer, sizeof(key_buffer), &len);
  if (ret != PSA_SUCCESS) {
    printf("Export Public Key failed with error: status %ld\n", ret);
  } else {
    printf("Export Public Key success\n");
  }

  /* Uncomment below lines to print the public key */
  //  printf("The exported public key is\n");
  //  for (int i = 0; i < (int)len; i++) {
  //    printf("%x ", key_buffer[i]);
  //  }
  //  printf("\n");

  /* Destroy key */
  ret = psa_destroy_key(generate_key_id);
  if (ret != PSA_SUCCESS) {
    printf("Destroy Key failed with error: %ld\n", ret);
  } else {
    printf("Destroy Key success\n");
  }
}

void test_psa_asymmetric_volatile_plain_key_storage()
{
  test_import_and_export_volatile_plain_key();
  test_generate_and_export_volatile_plain_key();
}

void test_psa_asymmetric_persistent_plain_key_storage()
{
  test_import_and_export_persistent_plain_key();
  test_generate_and_export_persistent_plain_key();
}

void test_psa_asymmetric_key_storage()
{
  psa_status_t ret;
  ret = psa_crypto_init();
  if (ret != PSA_SUCCESS) {
    printf("PSA Crypto Init failed with status : %ld\n", ret);
  } else {
    printf("PSA Crypto Init Success\n");
  }
  test_psa_asymmetric_volatile_plain_key_storage();
  test_psa_asymmetric_persistent_plain_key_storage();
}
