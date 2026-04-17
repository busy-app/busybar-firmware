/***************************************************************************//**
 * @file
 * @brief PSA Crypto opaque driver wrapper functions for HostCryptoSubSystem.
 *******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 ******************************************************************************/

#include "sli_psa_driver_features.h"

#if defined(SLI_MBEDTLS_DEVICE_HC)

#include "sli_hostcrypto_opaque_functions.h"
#include "sli_hostcrypto_transparent_functions.h"

// -----------------------------------------------------------------------------
// Opaque Cipher Functions

psa_status_t sli_hostcrypto_opaque_cipher_encrypt(
  const psa_key_attributes_t *attributes,
  const uint8_t *key_buffer,
  size_t key_buffer_size,
  psa_algorithm_t alg,
  const uint8_t *iv,
  size_t iv_length,
  const uint8_t *input,
  size_t input_length,
  uint8_t *output,
  size_t output_size,
  size_t *output_length)
{
  return sli_hostcrypto_transparent_cipher_encrypt(
    attributes,
    key_buffer,
    key_buffer_size,
    alg,
    iv,
    iv_length,
    input,
    input_length,
    output,
    output_size,
    output_length);
}

psa_status_t sli_hostcrypto_opaque_cipher_decrypt(
  const psa_key_attributes_t *attributes,
  const uint8_t *key_buffer,
  size_t key_buffer_size,
  psa_algorithm_t alg,
  const uint8_t *input,
  size_t input_length,
  uint8_t *output,
  size_t output_size,
  size_t *output_length)
{
  return sli_hostcrypto_transparent_cipher_decrypt(
    attributes,
    key_buffer,
    key_buffer_size,
    alg,
    input,
    input_length,
    output,
    output_size,
    output_length);
}

psa_status_t sli_hostcrypto_opaque_cipher_encrypt_setup(
  sli_hostcrypto_transparent_cipher_operation_t *operation,
  const psa_key_attributes_t *attributes,
  const uint8_t *key_buffer,
  size_t key_buffer_size,
  psa_algorithm_t alg)
{
  return sli_hostcrypto_transparent_cipher_encrypt_setup(
    operation,
    attributes,
    key_buffer,
    key_buffer_size,
    alg);
}

psa_status_t sli_hostcrypto_opaque_cipher_decrypt_setup(
  sli_hostcrypto_transparent_cipher_operation_t *operation,
  const psa_key_attributes_t *attributes,
  const uint8_t *key_buffer,
  size_t key_buffer_size,
  psa_algorithm_t alg)
{
  return sli_hostcrypto_transparent_cipher_decrypt_setup(
    operation,
    attributes,
    key_buffer,
    key_buffer_size,
    alg);
}

// -----------------------------------------------------------------------------
// Opaque AEAD Functions

psa_status_t sli_hostcrypto_opaque_aead_encrypt(
  const psa_key_attributes_t *attributes,
  const uint8_t *key_buffer,
  size_t key_buffer_size,
  psa_algorithm_t alg,
  const uint8_t *nonce,
  size_t nonce_length,
  const uint8_t *additional_data,
  size_t additional_data_length,
  const uint8_t *plaintext,
  size_t plaintext_length,
  uint8_t *ciphertext,
  size_t ciphertext_size,
  size_t *ciphertext_length)
{
  return sli_hostcrypto_transparent_aead_encrypt(
    attributes,
    key_buffer,
    key_buffer_size,
    alg,
    nonce,
    nonce_length,
    additional_data,
    additional_data_length,
    plaintext,
    plaintext_length,
    ciphertext,
    ciphertext_size,
    ciphertext_length);
}

psa_status_t sli_hostcrypto_opaque_aead_decrypt(
  const psa_key_attributes_t *attributes,
  const uint8_t *key_buffer,
  size_t key_buffer_size,
  psa_algorithm_t alg,
  const uint8_t *nonce,
  size_t nonce_length,
  const uint8_t *additional_data,
  size_t additional_data_length,
  const uint8_t *ciphertext,
  size_t ciphertext_length,
  uint8_t *plaintext,
  size_t plaintext_size,
  size_t *plaintext_length)
{
  return sli_hostcrypto_transparent_aead_decrypt(
    attributes,
    key_buffer,
    key_buffer_size,
    alg,
    nonce,
    nonce_length,
    additional_data,
    additional_data_length,
    ciphertext,
    ciphertext_length,
    plaintext,
    plaintext_size,
    plaintext_length);
}

// -----------------------------------------------------------------------------
// Opaque MAC Functions

psa_status_t sli_hostcrypto_opaque_mac_compute(
  const psa_key_attributes_t *attributes,
  const uint8_t *key_buffer,
  size_t key_buffer_size,
  psa_algorithm_t alg,
  const uint8_t *input,
  size_t input_length,
  uint8_t *mac,
  size_t mac_size,
  size_t *mac_length)
{
  return sli_hostcrypto_transparent_mac_compute(
    attributes,
    key_buffer,
    key_buffer_size,
    alg,
    input,
    input_length,
    mac,
    mac_size,
    mac_length);
}

psa_status_t sli_hostcrypto_opaque_mac_sign_setup(
  sli_hostcrypto_transparent_mac_operation_t *operation,
  const psa_key_attributes_t *attributes,
  const uint8_t *key_buffer,
  size_t key_buffer_size,
  psa_algorithm_t alg)
{
  return sli_hostcrypto_transparent_mac_sign_setup(
    operation,
    attributes,
    key_buffer,
    key_buffer_size,
    alg);
}

psa_status_t sli_hostcrypto_opaque_mac_verify_setup(
  sli_hostcrypto_transparent_mac_operation_t *operation,
  const psa_key_attributes_t *attributes,
  const uint8_t *key_buffer,
  size_t key_buffer_size,
  psa_algorithm_t alg)
{
  return sli_hostcrypto_transparent_mac_verify_setup(
    operation,
    attributes,
    key_buffer,
    key_buffer_size,
    alg);
}

#endif /* SLI_MBEDTLS_DEVICE_HC */
