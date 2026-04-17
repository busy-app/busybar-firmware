/*******************************************************************************
 * @file  psa_ecdsa_app.c
 * @brief
 *******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
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
#include "psa_ecdsa_app.h"
#include "psa/crypto.h"
#include "sl_si91x_psa_wrap.h"
#include <stdio.h>

#define WRAP_INPUT_KEYS     0 // Enable this if the input private key needs to be wrapped before use
#define IMPORT_WRAPPED_KEYS 0 // Enable this if the input key is wrapped

static const unsigned char input_data[] = { 0x24, 0x8d, 0x6a, 0x61, 0xd2, 0x06, 0x38, 0xb8, 0xe5, 0xc0, 0x26,
                                            0x93, 0x0c, 0x3e, 0x60, 0x39, 0xa3, 0x3c, 0xe4, 0x59, 0x64, 0xff,
                                            0x21, 0x67, 0xf6, 0xec, 0xed, 0xd4, 0x19, 0xdb, 0x06, 0xc1 };

static const unsigned char private_key[] = { 0x95, 0xCD, 0x3A, 0x36, 0x25, 0xD6, 0xF6, 0x06, 0xBD, 0xC8, 0x64,
                                             0x77, 0x8D, 0x4A, 0xA6, 0x50, 0xC2, 0xD7, 0x9A, 0x05, 0x94, 0xDD,
                                             0x10, 0xCF, 0x4C, 0x47, 0x4B, 0x83, 0xD2, 0x87, 0x0D, 0x1A };

void test_psa_ecdsa()
{
  uint8_t public_key[PUBLIC_KEY_SIZE_P256R1]; // Uncompressed point format
  size_t pubkey_len;
  uint8_t signature_buf[SIGNATURE_SIZE_P256R1]; // DER format
  size_t signature_len;

  psa_status_t ret;
  psa_key_id_t key_id;
  psa_key_attributes_t key_attr;

  /* psa crypto library initialization */
  ret = psa_crypto_init();
  if (ret != PSA_SUCCESS) {
    printf("PSA crypto library initialization failed with error: %ld\r\n", ret);
  } else {
    printf("PSA crypto library initialization Success\r\n");
  }

  /* Import (private key) and Generate (public key) ECC key pair */

  // Set up attributes for a volatile private key
  key_attr = psa_key_attributes_init();
  psa_set_key_type(&key_attr, PSA_KEY_TYPE_ECC_KEY_PAIR(PSA_ECC_FAMILY_SECP_R1));
  psa_set_key_bits(&key_attr, PRIVATE_KEY_SIZE_P256R1_BITS); // Set PRIVATE_KEY_SIZE_P192R1_BITS for secp192r1
  psa_set_key_usage_flags(&key_attr, PSA_KEY_USAGE_SIGN_MESSAGE | PSA_KEY_USAGE_VERIFY_MESSAGE);
  psa_set_key_algorithm(&key_attr, PSA_ALG_ECDSA(PSA_ALG_SHA_256));

  // Import a private key
  ret = psa_import_key(&key_attr, private_key, sizeof(private_key), &key_id);
  if (ret != PSA_SUCCESS) {
    printf("Import Key failed with error: status %ld\r\n", ret);
  } else {
    printf("Import Key success\r\n");
  }

  // Export a public key from a volatile private key
  ret = psa_export_public_key(key_id, public_key, sizeof(public_key), &pubkey_len);
  if (ret != PSA_SUCCESS) {
    printf("Exporting a Public Key from Private key Failed with error: %ld\r\n", ret);
  } else {
    printf("Export Public Key from Private Key Success\r\n");
  }

  // Destroy the private key
  ret = psa_destroy_key(key_id);
  if (ret != PSA_SUCCESS) {
    printf("Destroy Key failed with error : %ld\r\n", ret);
  } else {
    printf("Destroy Key Success\r\n");
  }

  /* Import private key and generate signature */

  // Set up attributes for a volatile private key
  key_attr = psa_key_attributes_init();
  psa_set_key_type(&key_attr, PSA_KEY_TYPE_ECC_KEY_PAIR(PSA_ECC_FAMILY_SECP_R1));
  psa_set_key_bits(&key_attr, PRIVATE_KEY_SIZE_P256R1_BITS); // Set PRIVATE_KEY_SIZE_P192R1_BITS for secp192r1
  psa_set_key_usage_flags(&key_attr, PSA_KEY_USAGE_SIGN_MESSAGE | PSA_KEY_USAGE_VERIFY_MESSAGE);
  psa_set_key_algorithm(&key_attr, PSA_ALG_ECDSA(PSA_ALG_SHA_256));
#if WRAP_INPUT_KEYS
  psa_set_key_lifetime(&key_attr,
                       PSA_KEY_LIFETIME_FROM_PERSISTENCE_AND_LOCATION(PSA_KEY_PERSISTENCE_VOLATILE,
                                                                      PSA_KEY_VOLATILE_PERSISTENT_WRAP_IMPORT));
#elif IMPORT_WRAPPED_KEYS
  psa_set_key_lifetime(
    &key_attr,
    PSA_KEY_LIFETIME_FROM_PERSISTENCE_AND_LOCATION(PSA_KEY_PERSISTENCE_VOLATILE, PSA_KEY_VOLATILE_PERSISTENT_WRAPPED));
#endif

  // Import a private key
  ret = psa_import_key(&key_attr, private_key, sizeof(private_key), &key_id);
  if (ret != PSA_SUCCESS) {
    printf("Import Key failed with error: status %ld\r\n", ret);
  } else {
    printf("Import Key success\r\n");
  }

  // Sign a message with a volatile private key
  ret = psa_sign_message(key_id,
                         PSA_ALG_ECDSA(PSA_ALG_SHA_256),
                         input_data,
                         sizeof(input_data),
                         signature_buf,
                         sizeof(signature_buf),
                         &signature_len);

  if (ret != PSA_SUCCESS) {
    printf("Sign Message with Private key Failed with error: %ld\r\n", ret);
  } else {
    printf("Sign Message with Private Key Success\r\n");
  }

  // Destroy the wrapped/plain private key
  ret = psa_destroy_key(key_id);
  if (ret != PSA_SUCCESS) {
    printf("Destroy Key failed with error : %ld\r\n", ret);
  } else {
    printf("Destroy Key Success\r\n");
  }

  /* Import public key and verify signature */

  // Set up attributes for the generated public key
  key_attr = psa_key_attributes_init();
  psa_set_key_type(&key_attr, PSA_KEY_TYPE_ECC_PUBLIC_KEY(PSA_ECC_FAMILY_SECP_R1));
  psa_set_key_usage_flags(&key_attr, PSA_KEY_USAGE_VERIFY_MESSAGE);
  psa_set_key_algorithm(&key_attr, PSA_ALG_ECDSA(PSA_ALG_SHA_256));

  // Import public key
  ret = psa_import_key(&key_attr, public_key, sizeof(public_key), &key_id);
  if (ret != PSA_SUCCESS) {
    printf("Import Public Key failed with error : %ld\r\n", ret);
  } else {
    printf("Import Public Key Success\r\n");
  }

  // Verify signature with public key
  ret = psa_verify_message(key_id,
                           PSA_ALG_ECDSA(PSA_ALG_SHA_256),
                           input_data,
                           sizeof(input_data),
                           signature_buf,
                           signature_len);
  if (ret != PSA_SUCCESS) {
    printf("Signature Verification with Public Key failed with error: %ld\r\n", ret);
  } else {
    printf("Signature Verification with Public Key Success\r\n");
  }

  // Destroy public key
  ret = psa_destroy_key(key_id);
  if (ret != PSA_SUCCESS) {
    printf("Destroy Key failed with error : %ld\r\n", ret);
  } else {
    printf("Destroy Key Success\r\n");
  }

  return;
}
