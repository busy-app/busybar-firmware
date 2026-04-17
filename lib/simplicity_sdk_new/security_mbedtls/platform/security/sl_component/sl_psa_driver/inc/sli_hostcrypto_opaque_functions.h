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

#ifndef SLI_HOSTCRYPTO_OPAQUE_FUNCTIONS_H
#define SLI_HOSTCRYPTO_OPAQUE_FUNCTIONS_H

#include "sli_psa_driver_features.h"

#if defined(SLI_MBEDTLS_DEVICE_HC)

#include "psa/crypto_driver_common.h"
#include "sli_hostcrypto_transparent_types.h"

#ifdef __cplusplus
extern "C" {
#endif

// -----------------------------------------------------------------------------
// Opaque Cipher Functions

/** Opaque wrapper for cipher encryption.
 *
 * This function provides an opaque interface to the transparent hostcrypto
 * cipher encrypt functionality.
 *
 * \param attributes            Key attributes
 * \param key_buffer            Key material buffer
 * \param key_buffer_size       Size of key buffer
 * \param alg                   Cipher algorithm to use
 * \param iv                    Initialization vector
 * \param iv_length             Length of the IV
 * \param input                 Input data to encrypt
 * \param input_length          Length of input data
 * \param output                Output buffer for encrypted data
 * \param output_size           Size of output buffer
 * \param output_length         Actual length of output data
 *
 * \retval PSA_SUCCESS         Operation completed successfully
 * \retval PSA_ERROR_*         See transparent function documentation
 */
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
  size_t *output_length);

/** Opaque wrapper for cipher decryption.
 *
 * \param attributes            Key attributes
 * \param key_buffer            Key material buffer
 * \param key_buffer_size       Size of key buffer
 * \param alg                   Cipher algorithm to use
 * \param input                 Input data to decrypt (includes IV)
 * \param input_length          Length of input data
 * \param output                Output buffer for decrypted data
 * \param output_size           Size of output buffer
 * \param output_length         Actual length of output data
 *
 * \retval PSA_SUCCESS         Operation completed successfully
 * \retval PSA_ERROR_*         See transparent function documentation
 */
psa_status_t sli_hostcrypto_opaque_cipher_decrypt(
  const psa_key_attributes_t *attributes,
  const uint8_t *key_buffer,
  size_t key_buffer_size,
  psa_algorithm_t alg,
  const uint8_t *input,
  size_t input_length,
  uint8_t *output,
  size_t output_size,
  size_t *output_length);

/** Opaque wrapper for cipher encrypt setup.
 *
 * \param operation             Cipher operation context
 * \param attributes            Key attributes
 * \param key_buffer            Key material buffer
 * \param key_buffer_size       Size of key buffer
 * \param alg                   Cipher algorithm
 *
 * \retval PSA_SUCCESS         Operation completed successfully
 * \retval PSA_ERROR_*         See transparent function documentation
 */
psa_status_t sli_hostcrypto_opaque_cipher_encrypt_setup(
  sli_hostcrypto_transparent_cipher_operation_t *operation,
  const psa_key_attributes_t *attributes,
  const uint8_t *key_buffer,
  size_t key_buffer_size,
  psa_algorithm_t alg);

/** Opaque wrapper for cipher decrypt setup.
 *
 * \param operation             Cipher operation context
 * \param attributes            Key attributes
 * \param key_buffer            Key material buffer
 * \param key_buffer_size       Size of key buffer
 * \param alg                   Cipher algorithm
 *
 * \retval PSA_SUCCESS         Operation completed successfully
 * \retval PSA_ERROR_*         See transparent function documentation
 */
psa_status_t sli_hostcrypto_opaque_cipher_decrypt_setup(
  sli_hostcrypto_transparent_cipher_operation_t *operation,
  const psa_key_attributes_t *attributes,
  const uint8_t *key_buffer,
  size_t key_buffer_size,
  psa_algorithm_t alg);

// -----------------------------------------------------------------------------
// Opaque AEAD Functions

/** Opaque wrapper for AEAD encryption.
 *
 * \param attributes            Key attributes
 * \param key_buffer            Key material buffer
 * \param key_buffer_size       Size of key buffer
 * \param alg                   AEAD algorithm
 * \param nonce                 Nonce for the encryption
 * \param nonce_length          Length of nonce
 * \param additional_data       Additional authenticated data
 * \param additional_data_length Length of additional data
 * \param plaintext             Plaintext to encrypt
 * \param plaintext_length      Length of plaintext
 * \param ciphertext            Buffer for encrypted output
 * \param ciphertext_size       Size of ciphertext buffer
 * \param ciphertext_length     Actual ciphertext length
 *
 * \retval PSA_SUCCESS         Operation completed successfully
 * \retval PSA_ERROR_*         See transparent function documentation
 */
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
  size_t *ciphertext_length);

/** Opaque wrapper for AEAD decryption.
 *
 * \param attributes            Key attributes
 * \param key_buffer            Key material buffer
 * \param key_buffer_size       Size of key buffer
 * \param alg                   AEAD algorithm
 * \param nonce                 Nonce for the decryption
 * \param nonce_length          Length of nonce
 * \param additional_data       Additional authenticated data
 * \param additional_data_length Length of additional data
 * \param ciphertext            Ciphertext to decrypt
 * \param ciphertext_length     Length of ciphertext
 * \param plaintext             Buffer for decrypted output
 * \param plaintext_size        Size of plaintext buffer
 * \param plaintext_length      Actual plaintext length
 *
 * \retval PSA_SUCCESS         Operation completed successfully
 * \retval PSA_ERROR_*         See transparent function documentation
 */
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
  size_t *plaintext_length);

// -----------------------------------------------------------------------------
// Opaque MAC Functions

/** Opaque wrapper for MAC computation.
 *
 * \param attributes            Key attributes
 * \param key_buffer            Key material buffer
 * \param key_buffer_size       Size of key buffer
 * \param alg                   MAC algorithm
 * \param input                 Input data
 * \param input_length          Length of input data
 * \param mac                   Buffer for MAC output
 * \param mac_size              Size of MAC buffer
 * \param mac_length            Actual MAC length
 *
 * \retval PSA_SUCCESS         Operation completed successfully
 * \retval PSA_ERROR_*         See transparent function documentation
 */
psa_status_t sli_hostcrypto_opaque_mac_compute(
  const psa_key_attributes_t *attributes,
  const uint8_t *key_buffer,
  size_t key_buffer_size,
  psa_algorithm_t alg,
  const uint8_t *input,
  size_t input_length,
  uint8_t *mac,
  size_t mac_size,
  size_t *mac_length);

/** Opaque wrapper for MAC sign setup.
 *
 * \param operation             MAC operation context
 * \param attributes            Key attributes
 * \param key_buffer            Key material buffer
 * \param key_buffer_size       Size of key buffer
 * \param alg                   MAC algorithm
 *
 * \retval PSA_SUCCESS         Operation completed successfully
 * \retval PSA_ERROR_*         See transparent function documentation
 */
psa_status_t sli_hostcrypto_opaque_mac_sign_setup(
  sli_hostcrypto_transparent_mac_operation_t *operation,
  const psa_key_attributes_t *attributes,
  const uint8_t *key_buffer,
  size_t key_buffer_size,
  psa_algorithm_t alg);

/** Opaque wrapper for MAC verify setup.
 *
 * \param operation             MAC operation context
 * \param attributes            Key attributes
 * \param key_buffer            Key material buffer
 * \param key_buffer_size       Size of key buffer
 * \param alg                   MAC algorithm
 *
 * \retval PSA_SUCCESS         Operation completed successfully
 * \retval PSA_ERROR_*         See transparent function documentation
 */
psa_status_t sli_hostcrypto_opaque_mac_verify_setup(
  sli_hostcrypto_transparent_mac_operation_t *operation,
  const psa_key_attributes_t *attributes,
  const uint8_t *key_buffer,
  size_t key_buffer_size,
  psa_algorithm_t alg);

#ifdef __cplusplus
}
#endif

#endif /* SLI_MBEDTLS_DEVICE_HC */

#endif /* SLI_HOSTCRYPTO_OPAQUE_FUNCTIONS_H */
