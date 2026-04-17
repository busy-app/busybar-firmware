/***************************************************************************//**
 * @file
 * @brief PSA Crypto driver functions for the HostCryptoSubSystem.
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

#ifndef SLI_HOSTCRYPTO_TRANSPARENT_FUNCTIONS_H
#define SLI_HOSTCRYPTO_TRANSPARENT_FUNCTIONS_H

#include "sli_psa_driver_features.h"

#if defined(SLI_MBEDTLS_DEVICE_HC)

#include "../include/sxsymcrypt/keyref.h"
#include "psa/crypto_driver_common.h"
#include "sli_hostcrypto_transparent_types.h"

// -----------------------------------------------------------------------------
// Entry Point Declarations

#ifdef __cplusplus
extern "C" {
#endif

/** Load key material into SX key reference structure.
 *
 * \param sx_key_ref            Pointer to SX key reference structure
 * \param attributes            Attributes of the key
 * \param key_buffer            Buffer containing key data
 *
 * \retval #PSA_SUCCESS
 *         Success.
 * \retval #PSA_ERROR_INVALID_ARGUMENT
 *         \p key is not compatible with \p alg.
 */
psa_status_t sli_hostcrypto_load_key(struct sxkeyref *sx_key_ref,
                                     const psa_key_attributes_t *attributes,
                                     const uint8_t *key_buffer);

/** Encrypt a message using a symmetric cipher.
 *
 * \param attributes            Attributes of the key
 *                              It must allow the usage #PSA_KEY_USAGE_ENCRYPT.
 * \param key_buffer            Buffer containing key data
 * \param key_buffer_size       Size of key buffer in bytes
 * \param alg                   The cipher algorithm to compute
 *                              (\c PSA_ALG_XXX value such that
 *                              #PSA_ALG_IS_CIPHER(\p alg) is true).
 * \param iv                    Initialization vector to be used
 * \param iv_length             Size of IV in bytes
 * \param[in] input             Buffer containing the message to encrypt.
 * \param input_length          Size of the \p input buffer in bytes.
 * \param[out] output           Buffer where the output is to be written.
 *                              The output contains the IV followed by
 *                              the ciphertext proper.
 * \param output_size           Size of the \p output buffer in bytes.
 * \param[out] output_length    On success, the number of bytes
 *                              that make up the output.
 *
 * \retval #PSA_SUCCESS
 *         Success.
 * \retval #PSA_ERROR_INVALID_HANDLE \emptydescription
 * \retval #PSA_ERROR_NOT_PERMITTED \emptydescription
 * \retval #PSA_ERROR_INVALID_ARGUMENT
 *         \p key is not compatible with \p alg.
 * \retval #PSA_ERROR_NOT_SUPPORTED
 *         \p alg is not supported or is not a cipher algorithm.
 * \retval #PSA_ERROR_BUFFER_TOO_SMALL \emptydescription
 * \retval #PSA_ERROR_INSUFFICIENT_MEMORY \emptydescription
 * \retval #PSA_ERROR_COMMUNICATION_FAILURE \emptydescription
 * \retval #PSA_ERROR_HARDWARE_FAILURE \emptydescription
 * \retval #PSA_ERROR_CORRUPTION_DETECTED \emptydescription
 * \retval #PSA_ERROR_STORAGE_FAILURE \emptydescription
 * \retval #PSA_ERROR_BAD_STATE
 */
psa_status_t sli_hostcrypto_transparent_cipher_encrypt(
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

/** Decrypt a message using a symmetric cipher.
 *
 * \param attributes            Attributes of the key
 *                              It must allow the usage #PSA_KEY_USAGE_DECRYPT.
 * \param key_buffer            Buffer containing key data
 * \param key_buffer_size       Size of key buffer in bytes
 * \param alg                   The cipher algorithm to compute
 *                              (\c PSA_ALG_XXX value such that
 *                              #PSA_ALG_IS_CIPHER(\p alg) is true).
 * \param[in] input             Buffer containing the message to decrypt.
 *                              This consists of the IV followed by the
 *                              ciphertext proper.
 * \param input_length          Size of the \p input buffer in bytes.
 * \param[out] output           Buffer where the plaintext is to be written.
 * \param output_size           Size of the \p output buffer in bytes.
 * \param[out] output_length    On success, the number of bytes
 *                              that make up the output.
 *
 * \retval #PSA_SUCCESS
 *         Success.
 * \retval #PSA_ERROR_INVALID_HANDLE \emptydescription
 * \retval #PSA_ERROR_NOT_PERMITTED \emptydescription
 * \retval #PSA_ERROR_INVALID_ARGUMENT
 *         \p key is not compatible with \p alg.
 * \retval #PSA_ERROR_NOT_SUPPORTED
 *         \p alg is not supported or is not a cipher algorithm.
 * \retval #PSA_ERROR_BUFFER_TOO_SMALL \emptydescription
 * \retval #PSA_ERROR_INSUFFICIENT_MEMORY \emptydescription
 * \retval #PSA_ERROR_COMMUNICATION_FAILURE \emptydescription
 * \retval #PSA_ERROR_HARDWARE_FAILURE \emptydescription
 * \retval #PSA_ERROR_CORRUPTION_DETECTED \emptydescription
 * \retval #PSA_ERROR_STORAGE_FAILURE \emptydescription
 * \retval #PSA_ERROR_BAD_STATE
 */
psa_status_t sli_hostcrypto_transparent_cipher_decrypt(
  const psa_key_attributes_t *attributes,
  const uint8_t *key_buffer,
  size_t key_buffer_size,
  psa_algorithm_t alg,
  const uint8_t *input,
  size_t input_length,
  uint8_t *output,
  size_t output_size,
  size_t *output_length);

/** Set the key for a multipart symmetric encryption operation.
 *
 * \param[in,out] operation     The operation object to set up. It must have
 *                              been initialized as per the documentation for
 *                              #psa_cipher_operation_t and not yet in use.
 * \param attributes            Attributes of the key
 *                              It must allow the usage #PSA_KEY_USAGE_DECRYPT.
 * \param key_buffer            Buffer containing key data
 * \param key_buffer_size       Size of key buffer in bytes
 * \param alg                   The cipher algorithm to compute
 *                              (\c PSA_ALG_XXX value such that
 *                              #PSA_ALG_IS_CIPHER(\p alg) is true).
 *
 * \retval #PSA_SUCCESS
 *         Success.
 * \retval #PSA_ERROR_INVALID_HANDLE \emptydescription
 * \retval #PSA_ERROR_NOT_PERMITTED \emptydescription
 * \retval #PSA_ERROR_INVALID_ARGUMENT
 *         \p key is not compatible with \p alg.
 * \retval #PSA_ERROR_NOT_SUPPORTED
 *         \p alg is not supported or is not a cipher algorithm.
 * \retval #PSA_ERROR_INSUFFICIENT_MEMORY \emptydescription
 * \retval #PSA_ERROR_COMMUNICATION_FAILURE \emptydescription
 * \retval #PSA_ERROR_HARDWARE_FAILURE \emptydescription
 * \retval #PSA_ERROR_CORRUPTION_DETECTED \emptydescription
 * \retval #PSA_ERROR_STORAGE_FAILURE \emptydescription
 * \retval #PSA_ERROR_BAD_STATE.
 */
psa_status_t sli_hostcrypto_transparent_cipher_encrypt_setup(
  sli_hostcrypto_transparent_cipher_operation_t *operation,
  const psa_key_attributes_t *attributes,
  const uint8_t *key_buffer,
  size_t key_buffer_size,
  psa_algorithm_t alg);

/** Set the key for a multipart symmetric decryption operation.
 *
 * \param[in,out] operation     The operation object to set up. It must have
 *                              been initialized as per the documentation for
 *                              #psa_cipher_operation_t and not yet in use.
 * \param attributes            Attributes of the key
 *                              It must allow the usage #PSA_KEY_USAGE_DECRYPT.
 * \param key_buffer            Buffer containing key data
 * \param key_buffer_size       Size of key buffer in bytes
 * \param alg                   The cipher algorithm to compute
 *                              (\c PSA_ALG_XXX value such that
 *                              #PSA_ALG_IS_CIPHER(\p alg) is true).
 *
 * \retval #PSA_SUCCESS
 *         Success.
 * \retval #PSA_ERROR_INVALID_HANDLE \emptydescription
 * \retval #PSA_ERROR_NOT_PERMITTED \emptydescription
 * \retval #PSA_ERROR_INVALID_ARGUMENT
 *         \p key is not compatible with \p alg.
 * \retval #PSA_ERROR_NOT_SUPPORTED
 *         \p alg is not supported or is not a cipher algorithm.
 * \retval #PSA_ERROR_INSUFFICIENT_MEMORY \emptydescription
 * \retval #PSA_ERROR_COMMUNICATION_FAILURE \emptydescription
 * \retval #PSA_ERROR_HARDWARE_FAILURE \emptydescription
 * \retval #PSA_ERROR_CORRUPTION_DETECTED \emptydescription
 * \retval #PSA_ERROR_STORAGE_FAILURE \emptydescription
 * \retval #PSA_ERROR_BAD_STATE
 */
psa_status_t sli_hostcrypto_transparent_cipher_decrypt_setup(
  sli_hostcrypto_transparent_cipher_operation_t *operation,
  const psa_key_attributes_t *attributes,
  const uint8_t *key_buffer,
  size_t key_buffer_size,
  psa_algorithm_t alg);

/** Set the IV for a symmetric encryption or decryption operation.
 *
 * This function sets the IV (initialization vector), nonce
 * or initial counter value for the encryption or decryption operation.
 *
 * \param[in,out] operation     Active cipher operation.
 * \param[in] iv                Buffer containing the IV to use.
 * \param iv_length             Size of the IV in bytes.
 *
 * \retval #PSA_SUCCESS
 *         Success.
 * \retval #PSA_ERROR_INVALID_ARGUMENT
 *         The size of \p iv is not acceptable for the chosen algorithm,
 *         or the chosen algorithm does not use an IV.
 * \retval #PSA_ERROR_INSUFFICIENT_MEMORY \emptydescription
 * \retval #PSA_ERROR_COMMUNICATION_FAILURE \emptydescription
 * \retval #PSA_ERROR_HARDWARE_FAILURE \emptydescription
 * \retval #PSA_ERROR_CORRUPTION_DETECTED \emptydescription
 * \retval #PSA_ERROR_STORAGE_FAILURE \emptydescription
 * \retval #PSA_ERROR_BAD_STATE
 *         The operation state is not valid (it must be an active cipher
 *         encrypt operation, with no IV set)
 */
psa_status_t sli_hostcrypto_transparent_cipher_set_iv(
  sli_hostcrypto_transparent_cipher_operation_t *operation,
  const uint8_t *iv,
  size_t iv_length);

/** Encrypt or decrypt a message fragment in an active cipher operation.
 *
 * \param[in,out] operation     Active cipher operation.
 * \param[in] input             Buffer containing the message fragment to
 *                              encrypt or decrypt.
 * \param input_length          Size of the \p input buffer in bytes.
 * \param[out] output           Buffer where the output is to be written.
 * \param output_size           Size of the \p output buffer in bytes.
 * \param[out] output_length    On success, the number of bytes
 *                              that make up the returned output.
 *
 * \retval #PSA_SUCCESS
 *         Success.
 * \retval #PSA_ERROR_BUFFER_TOO_SMALL
 *         The size of the \p output buffer is too small.
 * \retval #PSA_ERROR_INSUFFICIENT_MEMORY \emptydescription
 * \retval #PSA_ERROR_COMMUNICATION_FAILURE \emptydescription
 * \retval #PSA_ERROR_HARDWARE_FAILURE \emptydescription
 * \retval #PSA_ERROR_CORRUPTION_DETECTED \emptydescription
 * \retval #PSA_ERROR_STORAGE_FAILURE \emptydescription
 * \retval #PSA_ERROR_BAD_STATE
 *         The operation state is not valid (it must be active, with an IV set
 *         if required for the algorithm)
 */
psa_status_t sli_hostcrypto_transparent_cipher_update(
  sli_hostcrypto_transparent_cipher_operation_t *operation,
  const uint8_t *input,
  size_t input_length,
  uint8_t *output,
  size_t output_size,
  size_t *output_length);

/** Finish encrypting or decrypting a message in a cipher operation.
 *
 * \param[in,out] operation     Active cipher operation.
 * \param[out] output           Buffer where the output is to be written.
 * \param output_size           Size of the \p output buffer in bytes.
 * \param[out] output_length    On success, the number of bytes
 *                              that make up the returned output.
 *
 * \retval #PSA_SUCCESS
 *         Success.
 * \retval #PSA_ERROR_INVALID_ARGUMENT
 *         The total input size passed to this operation is not valid for
 *         this particular algorithm. For example, the algorithm is a based
 *         on block cipher and requires a whole number of blocks, but the
 *         total input size is not a multiple of the block size.
 * \retval #PSA_ERROR_INVALID_PADDING
 *         This is a decryption operation for an algorithm that includes
 *         padding, and the ciphertext does not contain valid padding.
 * \retval #PSA_ERROR_BUFFER_TOO_SMALL
 *         The size of the \p output buffer is too small.
 * \retval #PSA_ERROR_INSUFFICIENT_MEMORY \emptydescription
 * \retval #PSA_ERROR_COMMUNICATION_FAILURE \emptydescription
 * \retval #PSA_ERROR_HARDWARE_FAILURE \emptydescription
 * \retval #PSA_ERROR_CORRUPTION_DETECTED \emptydescription
 * \retval #PSA_ERROR_STORAGE_FAILURE \emptydescription
 * \retval #PSA_ERROR_BAD_STATE
 *         The operation state is not valid (it must be active, with an IV set
 *         if required for the algorithm)
 */
psa_status_t sli_hostcrypto_transparent_cipher_finish(
  sli_hostcrypto_transparent_cipher_operation_t *operation,
  uint8_t *output,
  size_t output_size,
  size_t *output_length);

/** Abort a cipher operation.
 *
 * Aborting an operation frees all associated resources except for the
 * \p operation structure itself.
 *
 * \param[in,out] operation     Initialized cipher operation.
 *
 * \retval #PSA_SUCCESS \emptydescription
 * \retval #PSA_ERROR_COMMUNICATION_FAILURE \emptydescription
 * \retval #PSA_ERROR_HARDWARE_FAILURE \emptydescription
 * \retval #PSA_ERROR_CORRUPTION_DETECTED \emptydescription
 * \retval #PSA_ERROR_BAD_STATE
 */
psa_status_t sli_hostcrypto_transparent_cipher_abort(
  sli_hostcrypto_transparent_cipher_operation_t *operation);

/** Process an authenticated encryption operation.
 *
 * \param attributes              Attributes of the key
 * \param key_buffer              Key buffer
 * \param key_buffer_size         Size of key buffer
 *                                #PSA_KEY_USAGE_ENCRYPT.
 * \param alg                     The AEAD algorithm to compute
 *                                (\c PSA_ALG_XXX value such that
 *                                #PSA_ALG_IS_AEAD(\p alg) is true).
 * \param[in] nonce               Nonce or IV to use.
 * \param nonce_length            Size of the \p nonce buffer in bytes.
 * \param[in] additional_data     Additional data that will be authenticated
 *                                but not encrypted.
 * \param additional_data_length  Size of \p additional_data in bytes.
 * \param[in] plaintext           Data that will be authenticated and
 *                                encrypted.
 * \param plaintext_length        Size of \p plaintext in bytes.
 * \param[out] ciphertext         Output buffer for the authenticated and
 *                                encrypted data. The additional data is not
 *                                part of this output. For algorithms where the
 *                                encrypted data and the authentication tag
 *                                are defined as separate outputs, the
 *                                authentication tag is appended to the
 *                                encrypted data.
 * \param ciphertext_size         Size of the \p ciphertext buffer in bytes.
 *                                This must be appropriate for the selected
 *                                algorithm and key:
 *                                - A sufficient output size is
 *                                  #PSA_AEAD_ENCRYPT_OUTPUT_SIZE(\c key_type,
 *                                  \p alg, \p plaintext_length) where
 *                                  \c key_type is the type of \p key.
 *                                - #PSA_AEAD_ENCRYPT_OUTPUT_MAX_SIZE(\p
 *                                  plaintext_length) evaluates to the maximum
 *                                  ciphertext size of any supported AEAD
 *                                  encryption.
 * \param[out] ciphertext_length  On success, the size of the output
 *                                in the \p ciphertext buffer.
 *
 * \retval #PSA_SUCCESS
 *         Success.
 * \retval #PSA_ERROR_INVALID_HANDLE \emptydescription
 * \retval #PSA_ERROR_NOT_PERMITTED \emptydescription
 * \retval #PSA_ERROR_INVALID_ARGUMENT
 *         \p key is not compatible with \p alg.
 * \retval #PSA_ERROR_NOT_SUPPORTED
 *         \p alg is not supported or is not an AEAD algorithm.
 * \retval #PSA_ERROR_INSUFFICIENT_MEMORY \emptydescription
 * \retval #PSA_ERROR_BUFFER_TOO_SMALL
 *         \p ciphertext_size is too small.
 *         #PSA_AEAD_ENCRYPT_OUTPUT_SIZE(\c key_type, \p alg,
 *         \p plaintext_length) or
 *         #PSA_AEAD_ENCRYPT_OUTPUT_MAX_SIZE(\p plaintext_length) can be used to
 *         determine the required buffer size.
 * \retval #PSA_ERROR_COMMUNICATION_FAILURE \emptydescription
 * \retval #PSA_ERROR_HARDWARE_FAILURE \emptydescription
 * \retval #PSA_ERROR_CORRUPTION_DETECTED \emptydescription
 * \retval #PSA_ERROR_STORAGE_FAILURE \emptydescription
 * \retval #PSA_ERROR_BAD_STATE
 */
psa_status_t sli_hostcrypto_transparent_aead_encrypt(
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

/** Process an authenticated decryption operation.
 *
 * \param attributes              Attributes of the key
 * \param key_buffer              Key buffer
 * \param key_buffer_size         Size of key buffer
 * \param alg                     The AEAD algorithm to compute
 *                                (\c PSA_ALG_XXX value such that
 *                                #PSA_ALG_IS_AEAD(\p alg) is true).
 * \param[in] nonce               Nonce or IV to use.
 * \param nonce_length            Size of the \p nonce buffer in bytes.
 * \param[in] additional_data     Additional data that has been authenticated
 *                                but not encrypted.
 * \param additional_data_length  Size of \p additional_data in bytes.
 * \param[in] ciphertext          Data that has been authenticated and
 *                                encrypted. For algorithms where the
 *                                encrypted data and the authentication tag
 *                                are defined as separate inputs, the buffer
 *                                must contain the encrypted data followed
 *                                by the authentication tag.
 * \param ciphertext_length       Size of \p ciphertext in bytes.
 * \param[out] plaintext          Output buffer for the decrypted data.
 * \param plaintext_size          Size of the \p plaintext buffer in bytes.
 *                                This must be appropriate for the selected
 *                                algorithm and key:
 *                                - A sufficient output size is
 *                                  #PSA_AEAD_DECRYPT_OUTPUT_SIZE(\c key_type,
 *                                  \p alg, \p ciphertext_length) where
 *                                  \c key_type is the type of \p key.
 *                                - #PSA_AEAD_DECRYPT_OUTPUT_MAX_SIZE(\p
 *                                  ciphertext_length) evaluates to the maximum
 *                                  plaintext size of any supported AEAD
 *                                  decryption.
 * \param[out] plaintext_length   On success, the size of the output
 *                                in the \p plaintext buffer.
 *
 * \retval #PSA_SUCCESS
 *         Success.
 * \retval #PSA_ERROR_INVALID_HANDLE \emptydescription
 * \retval #PSA_ERROR_INVALID_SIGNATURE
 *         The ciphertext is not authentic.
 * \retval #PSA_ERROR_NOT_PERMITTED \emptydescription
 * \retval #PSA_ERROR_INVALID_ARGUMENT
 *         \p key is not compatible with \p alg.
 * \retval #PSA_ERROR_NOT_SUPPORTED
 *         \p alg is not supported or is not an AEAD algorithm.
 * \retval #PSA_ERROR_INSUFFICIENT_MEMORY \emptydescription
 * \retval #PSA_ERROR_BUFFER_TOO_SMALL
 *         \p plaintext_size is too small.
 *         #PSA_AEAD_DECRYPT_OUTPUT_SIZE(\c key_type, \p alg,
 *         \p ciphertext_length) or
 *         #PSA_AEAD_DECRYPT_OUTPUT_MAX_SIZE(\p ciphertext_length) can be used
 *         to determine the required buffer size.
 * \retval #PSA_ERROR_COMMUNICATION_FAILURE \emptydescription
 * \retval #PSA_ERROR_HARDWARE_FAILURE \emptydescription
 * \retval #PSA_ERROR_CORRUPTION_DETECTED \emptydescription
 * \retval #PSA_ERROR_STORAGE_FAILURE \emptydescription
 * \retval #PSA_ERROR_BAD_STATE
 */
psa_status_t sli_hostcrypto_transparent_aead_decrypt(
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

psa_status_t sli_hostcrypto_transparent_aead_encrypt_tag(
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
  size_t *ciphertext_length,
  uint8_t *tag,
  size_t tag_size,
  size_t *tag_length);

psa_status_t sli_hostcrypto_transparent_aead_decrypt_tag(
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
  const uint8_t* tag,
  size_t tag_length,
  uint8_t *plaintext,
  size_t plaintext_size,
  size_t *plaintext_length);

/** Set the key for a multipart authenticated encryption operation.
 *
 * \param[in,out] operation     The operation object to set up. It must have
 *                              been initialized as per the documentation for
 *                              #psa_aead_operation_t and not yet in use.
 * \param attributes              Attributes of the key
 * \param key_buffer              Key buffer
 * \param key_buffer_size         Size of key buffer
 * \param alg                   The AEAD algorithm to compute
 *                              (\c PSA_ALG_XXX value such that
 *                              #PSA_ALG_IS_AEAD(\p alg) is true).
 *
 * \retval #PSA_SUCCESS
 *         Success.
 * \retval #PSA_ERROR_BAD_STATE
 *         The operation state is not valid (it must be inactive)
 * \retval #PSA_ERROR_INVALID_HANDLE \emptydescription
 * \retval #PSA_ERROR_NOT_PERMITTED \emptydescription
 * \retval #PSA_ERROR_INVALID_ARGUMENT
 *         \p key is not compatible with \p alg.
 * \retval #PSA_ERROR_NOT_SUPPORTED
 *         \p alg is not supported or is not an AEAD algorithm.
 * \retval #PSA_ERROR_INSUFFICIENT_MEMORY \emptydescription
 * \retval #PSA_ERROR_COMMUNICATION_FAILURE \emptydescription
 * \retval #PSA_ERROR_HARDWARE_FAILURE \emptydescription
 * \retval #PSA_ERROR_CORRUPTION_DETECTED \emptydescription
 * \retval #PSA_ERROR_STORAGE_FAILURE
 */
psa_status_t sli_hostcrypto_transparent_aead_encrypt_setup(
  sli_hostcrypto_transparent_aead_operation_t *operation,
  const psa_key_attributes_t *attributes,
  const uint8_t *key_buffer,
  size_t key_buffer_size,
  psa_algorithm_t alg);

/** Set the key for a multipart authenticated decryption operation.
 *
 * \param[in,out] operation     The operation object to set up. It must have
 *                              been initialized as per the documentation for
 *                              #psa_aead_operation_t and not yet in use.
 * \param attributes            Attributes of the key
 * \param key_buffer            Key buffer
 * \param key_buffer_size       Size of key buffer
 * \param alg                   The AEAD algorithm to compute
 *                              (\c PSA_ALG_XXX value such that
 *                              #PSA_ALG_IS_AEAD(\p alg) is true).
 *
 * \retval #PSA_SUCCESS
 *         Success.
 * \retval #PSA_ERROR_INVALID_HANDLE \emptydescription
 * \retval #PSA_ERROR_NOT_PERMITTED \emptydescription
 * \retval #PSA_ERROR_INVALID_ARGUMENT
 *         \p key is not compatible with \p alg.
 * \retval #PSA_ERROR_NOT_SUPPORTED
 *         \p alg is not supported or is not an AEAD algorithm.
 * \retval #PSA_ERROR_INSUFFICIENT_MEMORY \emptydescription
 * \retval #PSA_ERROR_COMMUNICATION_FAILURE \emptydescription
 * \retval #PSA_ERROR_HARDWARE_FAILURE \emptydescription
 * \retval #PSA_ERROR_CORRUPTION_DETECTED \emptydescription
 * \retval #PSA_ERROR_STORAGE_FAILURE \emptydescription
 * \retval #PSA_ERROR_BAD_STATE
 *         The operation state is not valid (it must be inactive)
 */
psa_status_t sli_hostcrypto_transparent_aead_decrypt_setup(
  sli_hostcrypto_transparent_aead_operation_t *operation,
  const psa_key_attributes_t *attributes,
  const uint8_t *key_buffer,
  size_t key_buffer_size,
  psa_algorithm_t alg);

/** Declare the lengths of the message and additional data for AEAD.
 *
 * - For #PSA_ALG_CCM, calling this function is required.
 *
 * \param[in,out] operation     Active AEAD operation.
 * \param ad_length             Size of the non-encrypted additional
 *                              authenticated data in bytes.
 * \param plaintext_length      Size of the plaintext to encrypt in bytes.
 *
 * \retval #PSA_SUCCESS
 *         Success.
 * \retval #PSA_ERROR_INVALID_ARGUMENT
 *         At least one of the lengths is not acceptable for the chosen
 *         algorithm.
 * \retval #PSA_ERROR_INSUFFICIENT_MEMORY \emptydescription
 * \retval #PSA_ERROR_COMMUNICATION_FAILURE \emptydescription
 * \retval #PSA_ERROR_HARDWARE_FAILURE \emptydescription
 * \retval #PSA_ERROR_CORRUPTION_DETECTED \emptydescription
 * \retval #PSA_ERROR_BAD_STATE
 *         The operation state is not valid (it must be active, and
 *         psa_aead_update_ad() and psa_aead_update() must not have been
 *         called yet)
 */
psa_status_t sli_hostcrypto_transparent_aead_set_lengths(
  sli_hostcrypto_transparent_aead_operation_t *operation,
  size_t ad_length,
  size_t plaintext_length);

/** Set the nonce for an authenticated encryption or decryption operation.
 *
 * This function sets the nonce for the authenticated
 * encryption or decryption operation.
 *
 * \param[in,out] operation     Active AEAD operation.
 * \param[in] nonce             Buffer containing the nonce to use.
 * \param nonce_length          Size of the nonce in bytes.
 *
 * \retval #PSA_SUCCESS
 *         Success.
 * \retval #PSA_ERROR_INVALID_ARGUMENT
 *         The size of \p nonce is not acceptable for the chosen algorithm.
 * \retval #PSA_ERROR_INSUFFICIENT_MEMORY \emptydescription
 * \retval #PSA_ERROR_COMMUNICATION_FAILURE \emptydescription
 * \retval #PSA_ERROR_HARDWARE_FAILURE \emptydescription
 * \retval #PSA_ERROR_CORRUPTION_DETECTED \emptydescription
 * \retval #PSA_ERROR_STORAGE_FAILURE \emptydescription
 * \retval #PSA_ERROR_BAD_STATE
 *         The operation state is not valid (it must be active, with no nonce
 *         set)
 */
psa_status_t sli_hostcrypto_transparent_aead_set_nonce(
  sli_hostcrypto_transparent_aead_operation_t *operation,
  const uint8_t *nonce,
  size_t nonce_length);

/** Pass additional data to an active AEAD operation.
 *
 * Additional data is authenticated, but not encrypted.
 *
 * \param[in,out] operation     Active AEAD operation.
 * \param[in] input             Buffer containing the fragment of
 *                              additional data.
 * \param input_length          Size of the \p input buffer in bytes.
 *
 * \retval #PSA_SUCCESS
 *         Success.
 * \retval #PSA_ERROR_INVALID_ARGUMENT
 *         The total input length overflows the additional data length that
 *         was previously specified with psa_aead_set_lengths().
 * \retval #PSA_ERROR_INSUFFICIENT_MEMORY \emptydescription
 * \retval #PSA_ERROR_COMMUNICATION_FAILURE \emptydescription
 * \retval #PSA_ERROR_HARDWARE_FAILURE \emptydescription
 * \retval #PSA_ERROR_CORRUPTION_DETECTED \emptydescription
 * \retval #PSA_ERROR_STORAGE_FAILURE \emptydescription
 * \retval #PSA_ERROR_BAD_STATE
 *         The operation state is not valid (it must be active, have a nonce
 *         set, have lengths set if required by the algorithm, and
 *         psa_aead_update() must not have been called yet)
 */
psa_status_t sli_hostcrypto_transparent_aead_update_ad(
  sli_hostcrypto_transparent_aead_operation_t *operation,
  const uint8_t *input,
  size_t input_length);

/** Encrypt or decrypt a message fragment in an active AEAD operation.
 *
 * \warning When decrypting, until psa_aead_verify() has returned #PSA_SUCCESS,
 *          there is no guarantee that the input is valid. Therefore, until
 *          you have called psa_aead_verify() and it has returned #PSA_SUCCESS:
 *          - Do not use the output in any way other than storing it in a
 *            confidential location. If you take any action that depends
 *            on the tentative decrypted data, this action will need to be
 *            undone if the input turns out not to be valid. Furthermore,
 *            if an adversary can observe that this action took place
 *            (for example through timing), they may be able to use this
 *            fact as an oracle to decrypt any message encrypted with the
 *            same key.
 *          - In particular, do not copy the output anywhere but to a
 *            memory or storage space that you have exclusive access to.
 *
 * This function does not require the input to be aligned to any
 * particular block boundary. If the implementation can only process
 * a whole block at a time, it must consume all the input provided, but
 * it may delay the end of the corresponding output until a subsequent
 * call to psa_aead_update(), psa_aead_finish() or psa_aead_verify()
 * provides sufficient input. The amount of data that can be delayed
 * in this way is bounded by #PSA_AEAD_UPDATE_OUTPUT_SIZE.
 *
 * \param[in,out] operation     Active AEAD operation.
 * \param[in] input             Buffer containing the message fragment to
 *                              encrypt or decrypt.
 * \param input_length          Size of the \p input buffer in bytes.
 * \param[out] output           Buffer where the output is to be written.
 * \param output_size           Size of the \p output buffer in bytes.
 *                              This must be appropriate for the selected
 *                                algorithm and key:
 *                                - A sufficient output size is
 *                                  #PSA_AEAD_UPDATE_OUTPUT_SIZE(\c key_type,
 *                                  \c alg, \p input_length) where
 *                                  \c key_type is the type of key and \c alg is
 *                                  the algorithm that were used to set up the
 *                                  operation.
 *                                - #PSA_AEAD_UPDATE_OUTPUT_MAX_SIZE(\p
 *                                  input_length) evaluates to the maximum
 *                                  output size of any supported AEAD
 *                                  algorithm.
 * \param[out] output_length    On success, the number of bytes
 *                              that make up the returned output.
 *
 * \retval #PSA_SUCCESS
 *         Success.
 * \retval #PSA_ERROR_BUFFER_TOO_SMALL
 *         The size of the \p output buffer is too small.
 *         #PSA_AEAD_UPDATE_OUTPUT_SIZE(\c key_type, \c alg, \p input_length) or
 *         #PSA_AEAD_UPDATE_OUTPUT_MAX_SIZE(\p input_length) can be used to
 *         determine the required buffer size.
 * \retval #PSA_ERROR_INVALID_ARGUMENT
 *         The total length of input to psa_aead_update_ad() so far is
 *         less than the additional data length that was previously
 *         specified with psa_aead_set_lengths(), or
 *         the total input length overflows the plaintext length that
 *         was previously specified with psa_aead_set_lengths().
 * \retval #PSA_ERROR_INSUFFICIENT_MEMORY \emptydescription
 * \retval #PSA_ERROR_COMMUNICATION_FAILURE \emptydescription
 * \retval #PSA_ERROR_HARDWARE_FAILURE \emptydescription
 * \retval #PSA_ERROR_CORRUPTION_DETECTED \emptydescription
 * \retval #PSA_ERROR_STORAGE_FAILURE \emptydescription
 * \retval #PSA_ERROR_BAD_STATE
 *         The operation state is not valid (it must be active, have a nonce
 *         set, and have lengths set if required by the algorithm)
 */
psa_status_t sli_hostcrypto_transparent_aead_update(
  sli_hostcrypto_transparent_aead_operation_t *operation,
  const uint8_t *input,
  size_t input_length,
  uint8_t *output,
  size_t output_size,
  size_t *output_length);

/** Finish encrypting a message in an AEAD operation.
 *
 * This function has two output buffers:
 * - \p ciphertext contains trailing ciphertext that was buffered from
 *   preceding calls to psa_aead_update().
 * - \p tag contains the authentication tag.
 *
 * \param[in,out] operation     Active AEAD operation.
 * \param[out] ciphertext       Buffer where the last part of the ciphertext
 *                              is to be written.
 * \param ciphertext_size       Size of the \p ciphertext buffer in bytes.
 *                              This must be appropriate for the selected
 *                              algorithm and key:
 *                              - A sufficient output size is
 *                                #PSA_AEAD_FINISH_OUTPUT_SIZE(\c key_type,
 *                                \c alg) where \c key_type is the type of key
 *                                and \c alg is the algorithm that were used to
 *                                set up the operation.
 *                              - #PSA_AEAD_FINISH_OUTPUT_MAX_SIZE evaluates to
 *                                the maximum output size of any supported AEAD
 *                                algorithm.
 * \param[out] ciphertext_length On success, the number of bytes of
 *                              returned ciphertext.
 * \param[out] tag              Buffer where the authentication tag is
 *                              to be written.
 * \param tag_size              Size of the \p tag buffer in bytes.
 *                              This must be appropriate for the selected
 *                              algorithm and key:
 *                              - The exact tag size is #PSA_AEAD_TAG_LENGTH(\c
 *                                key_type, \c key_bits, \c alg) where
 *                                \c key_type and \c key_bits are the type and
 *                                bit-size of the key, and \c alg is the
 *                                algorithm that were used in the call to
 *                                psa_aead_encrypt_setup().
 *                              - #PSA_AEAD_TAG_MAX_SIZE evaluates to the
 *                                maximum tag size of any supported AEAD
 *                                algorithm.
 * \param[out] tag_length       On success, the number of bytes
 *                              that make up the returned tag.
 *
 * \retval #PSA_SUCCESS
 *         Success.
 * \retval #PSA_ERROR_BUFFER_TOO_SMALL
 *         The size of the \p ciphertext or \p tag buffer is too small.
 *         #PSA_AEAD_FINISH_OUTPUT_SIZE(\c key_type, \c alg) or
 *         #PSA_AEAD_FINISH_OUTPUT_MAX_SIZE can be used to determine the
 *         required \p ciphertext buffer size. #PSA_AEAD_TAG_LENGTH(\c key_type,
 *         \c key_bits, \c alg) or #PSA_AEAD_TAG_MAX_SIZE can be used to
 *         determine the required \p tag buffer size.
 * \retval #PSA_ERROR_INVALID_ARGUMENT
 *         The total length of input to psa_aead_update_ad() so far is
 *         less than the additional data length that was previously
 *         specified with psa_aead_set_lengths(), or
 *         the total length of input to psa_aead_update() so far is
 *         less than the plaintext length that was previously
 *         specified with psa_aead_set_lengths().
 * \retval #PSA_ERROR_INSUFFICIENT_MEMORY \emptydescription
 * \retval #PSA_ERROR_COMMUNICATION_FAILURE \emptydescription
 * \retval #PSA_ERROR_HARDWARE_FAILURE \emptydescription
 * \retval #PSA_ERROR_CORRUPTION_DETECTED \emptydescription
 * \retval #PSA_ERROR_STORAGE_FAILURE \emptydescription
 * \retval #PSA_ERROR_BAD_STATE
 *         The operation state is not valid (it must be an active encryption
 *         operation with a nonce set)
 */
psa_status_t sli_hostcrypto_transparent_aead_finish(
  sli_hostcrypto_transparent_aead_operation_t *operation,
  uint8_t *ciphertext,
  size_t ciphertext_size,
  size_t *ciphertext_length,
  uint8_t *tag,
  size_t tag_size,
  size_t *tag_length);

/** Finish authenticating and decrypting a message in an AEAD operation.
 *
 * If the authentication tag is correct, this function outputs any remaining
 * plaintext and reports success. If the authentication tag is not correct,
 * this function returns #PSA_ERROR_INVALID_SIGNATURE.
 *
 * \param[in,out] operation     Active AEAD operation.
 * \param[out] plaintext        Buffer where the last part of the plaintext
 *                              is to be written. This is the remaining data
 *                              from previous calls to psa_aead_update()
 *                              that could not be processed until the end
 *                              of the input.
 * \param plaintext_size        Size of the \p plaintext buffer in bytes.
 *                              This must be appropriate for the selected algorithm and key:
 *                              - A sufficient output size is
 *                                #PSA_AEAD_VERIFY_OUTPUT_SIZE(\c key_type,
 *                                \c alg) where \c key_type is the type of key
 *                                and \c alg is the algorithm that were used to
 *                                set up the operation.
 *                              - #PSA_AEAD_VERIFY_OUTPUT_MAX_SIZE evaluates to
 *                                the maximum output size of any supported AEAD
 *                                algorithm.
 * \param[out] plaintext_length On success, the number of bytes of
 *                              returned plaintext.
 * \param[in] tag               Buffer containing the authentication tag.
 * \param tag_length            Size of the \p tag buffer in bytes.
 *
 * \retval #PSA_SUCCESS
 *         Success.
 * \retval #PSA_ERROR_INVALID_SIGNATURE
 *         The calculations were successful, but the authentication tag is
 *         not correct.
 * \retval #PSA_ERROR_BUFFER_TOO_SMALL
 *         The size of the \p plaintext buffer is too small.
 *         #PSA_AEAD_VERIFY_OUTPUT_SIZE(\c key_type, \c alg) or
 *         #PSA_AEAD_VERIFY_OUTPUT_MAX_SIZE can be used to determine the
 *         required buffer size.
 * \retval #PSA_ERROR_INVALID_ARGUMENT
 *         The total length of input to psa_aead_update_ad() so far is
 *         less than the additional data length that was previously
 *         specified with psa_aead_set_lengths(), or
 *         the total length of input to psa_aead_update() so far is
 *         less than the plaintext length that was previously
 *         specified with psa_aead_set_lengths().
 * \retval #PSA_ERROR_INSUFFICIENT_MEMORY \emptydescription
 * \retval #PSA_ERROR_COMMUNICATION_FAILURE \emptydescription
 * \retval #PSA_ERROR_HARDWARE_FAILURE \emptydescription
 * \retval #PSA_ERROR_CORRUPTION_DETECTED \emptydescription
 * \retval #PSA_ERROR_STORAGE_FAILURE \emptydescription
 * \retval #PSA_ERROR_BAD_STATE
 *         The operation state is not valid (it must be an active decryption
 *         operation with a nonce set)
 */
psa_status_t sli_hostcrypto_transparent_aead_verify(
  sli_hostcrypto_transparent_aead_operation_t *operation,
  uint8_t *plaintext,
  size_t plaintext_size,
  size_t *plaintext_length,
  const uint8_t *tag,
  size_t tag_length);

/** Abort an AEAD operation.
 *
 * Aborting an operation frees all associated resources except for the
 * \p operation structure itself.
 *
 * \param[in,out] operation     Initialized AEAD operation.
 *
 * \retval #PSA_SUCCESS \emptydescription
 * \retval #PSA_ERROR_COMMUNICATION_FAILURE \emptydescription
 * \retval #PSA_ERROR_HARDWARE_FAILURE \emptydescription
 * \retval #PSA_ERROR_CORRUPTION_DETECTED \emptydescription
 * \retval #PSA_ERROR_BAD_STATE
 */
psa_status_t sli_hostcrypto_transparent_aead_abort(
  sli_hostcrypto_transparent_aead_operation_t *operation);

/** Set up a multipart hash operation.
 *
 * \param[in,out] operation The operation object to set up. It must have
 *                          been initialized as per the documentation for
 *                          #psa_hash_operation_t and not yet in use.
 * \param alg               The hash algorithm to compute (\c PSA_ALG_XXX value
 *                          such that #PSA_ALG_IS_HASH(\p alg) is true).
 *
 * \retval #PSA_SUCCESS
 *         Success.
 * \retval #PSA_ERROR_NOT_SUPPORTED
 *         \p alg is not a supported hash algorithm.
 * \retval #PSA_ERROR_INVALID_ARGUMENT
 *         \p alg is not a hash algorithm.
 * \retval #PSA_ERROR_INSUFFICIENT_MEMORY \emptydescription
 * \retval #PSA_ERROR_COMMUNICATION_FAILURE \emptydescription
 * \retval #PSA_ERROR_HARDWARE_FAILURE \emptydescription
 * \retval #PSA_ERROR_CORRUPTION_DETECTED \emptydescription
 * \retval #PSA_ERROR_BAD_STATE
 *         The operation state is not valid (it must be inactive)
 */
psa_status_t sli_hostcrypto_transparent_hash_setup(
  sli_hostcrypto_transparent_hash_operation_t *operation,
  psa_algorithm_t alg);

/** Add a message fragment to a multipart hash operation.
 *
 * If this function returns an error status, the operation enters an error
 * state and must be aborted by calling psa_hash_abort().
 *
 * \param[in,out] operation Active hash operation.
 * \param[in] input         Buffer containing the message fragment to hash.
 * \param input_length      Size of the \p input buffer in bytes.
 *
 * \retval #PSA_SUCCESS
 *         Success.
 * \retval #PSA_ERROR_INSUFFICIENT_MEMORY \emptydescription
 * \retval #PSA_ERROR_COMMUNICATION_FAILURE \emptydescription
 * \retval #PSA_ERROR_HARDWARE_FAILURE \emptydescription
 * \retval #PSA_ERROR_CORRUPTION_DETECTED \emptydescription
 * \retval #PSA_ERROR_BAD_STATE
 *         The operation state is not valid (it must be active)
 */
psa_status_t sli_hostcrypto_transparent_hash_update(
  sli_hostcrypto_transparent_hash_operation_t *operation,
  const uint8_t *input,
  size_t input_length);

/** Finish the calculation of the hash of a message.
 *
 * When this function returns successfully, the operation becomes inactive.
 * If this function returns an error status, the operation enters an error
 * state and must be aborted by calling psa_hash_abort().
 *
 * \param[in,out] operation     Active hash operation.
 * \param[out] hash             Buffer where the hash is to be written.
 * \param hash_size             Size of the \p hash buffer in bytes.
 * \param[out] hash_length      On success, the number of bytes
 *                              that make up the hash value. This is always
 *                              #PSA_HASH_LENGTH(\c alg) where \c alg is the
 *                              hash algorithm that is calculated.
 *
 * \retval #PSA_SUCCESS
 *         Success.
 * \retval #PSA_ERROR_BUFFER_TOO_SMALL
 *         The size of the \p hash buffer is too small. You can determine a
 *         sufficient buffer size by calling #PSA_HASH_LENGTH(\c alg)
 *         where \c alg is the hash algorithm that is calculated.
 * \retval #PSA_ERROR_INSUFFICIENT_MEMORY \emptydescription
 * \retval #PSA_ERROR_COMMUNICATION_FAILURE \emptydescription
 * \retval #PSA_ERROR_HARDWARE_FAILURE \emptydescription
 * \retval #PSA_ERROR_CORRUPTION_DETECTED \emptydescription
 * \retval #PSA_ERROR_BAD_STATE
 *         The operation state is not valid (it must be active)
 */
psa_status_t sli_hostcrypto_transparent_hash_finish(
  sli_hostcrypto_transparent_hash_operation_t *operation,
  uint8_t *hash,
  size_t hash_size,
  size_t *hash_length);

/** Abort a hash operation.
 *
 * Aborting an operation frees all associated resources except for the
 * \p operation structure itself. Once aborted, the operation object
 * can be reused for another operation by calling
 * psa_hash_setup() again.
 *
 * \param[in,out] operation     Initialized hash operation.
 *
 * \retval #PSA_SUCCESS \emptydescription
 * \retval #PSA_ERROR_COMMUNICATION_FAILURE \emptydescription
 * \retval #PSA_ERROR_HARDWARE_FAILURE \emptydescription
 * \retval #PSA_ERROR_CORRUPTION_DETECTED \emptydescription
 * \retval #PSA_ERROR_BAD_STATE
 */
psa_status_t sli_hostcrypto_transparent_hash_abort(
  sli_hostcrypto_transparent_hash_operation_t *operation);

/** Calculate the hash (digest) of a message.
 *
 * \param alg               The hash algorithm to compute (\c PSA_ALG_XXX value
 *                          such that #PSA_ALG_IS_HASH(\p alg) is true).
 * \param[in] input         Buffer containing the message to hash.
 * \param input_length      Size of the \p input buffer in bytes.
 * \param[out] hash         Buffer where the hash is to be written.
 * \param hash_size         Size of the \p hash buffer in bytes.
 * \param[out] hash_length  On success, the number of bytes
 *                          that make up the hash value. This is always
 *                          #PSA_HASH_LENGTH(\p alg).
 *
 * \retval #PSA_SUCCESS
 *         Success.
 * \retval #PSA_ERROR_NOT_SUPPORTED
 *         \p alg is not supported or is not a hash algorithm.
 * \retval #PSA_ERROR_INVALID_ARGUMENT \emptydescription
 * \retval #PSA_ERROR_BUFFER_TOO_SMALL
 *         \p hash_size is too small
 * \retval #PSA_ERROR_INSUFFICIENT_MEMORY \emptydescription
 * \retval #PSA_ERROR_COMMUNICATION_FAILURE \emptydescription
 * \retval #PSA_ERROR_HARDWARE_FAILURE \emptydescription
 * \retval #PSA_ERROR_CORRUPTION_DETECTED \emptydescription
 * \retval #PSA_ERROR_BAD_STATE
 */
psa_status_t sli_hostcrypto_transparent_hash_compute(
  psa_algorithm_t alg,
  const uint8_t *input,
  size_t input_length,
  uint8_t *hash,
  size_t hash_size,
  size_t *hash_length);

/** Clone a hash operation.
 *
 * This function copies the state of an ongoing hash operation to
 * a new operation object. In other words, this function is equivalent
 * to calling psa_hash_setup() on \p target_operation with the same
 * algorithm that \p source_operation was set up for, then
 * psa_hash_update() on \p target_operation with the same input that
 * that was passed to \p source_operation. After this function returns, the
 * two objects are independent, i.e. subsequent calls involving one of
 * the objects do not affect the other object.
 *
 * \param[in] source_operation      The active hash operation to clone.
 * \param[in,out] target_operation  The operation object to set up.
 *                                  It must be initialized but not active.
 *
 * \retval #PSA_SUCCESS \emptydescription
 * \retval #PSA_ERROR_COMMUNICATION_FAILURE \emptydescription
 * \retval #PSA_ERROR_HARDWARE_FAILURE \emptydescription
 * \retval #PSA_ERROR_CORRUPTION_DETECTED \emptydescription
 * \retval #PSA_ERROR_INSUFFICIENT_MEMORY \emptydescription
 * \retval #PSA_ERROR_BAD_STATE
 *         The \p source_operation state is not valid (it must be active), or
 *         the \p target_operation state is not valid (it must be inactive)
 */
psa_status_t sli_hostcrypto_transparent_hash_clone(
  const sli_hostcrypto_transparent_hash_operation_t *source_operation,
  sli_hostcrypto_transparent_hash_operation_t *target_operation);

/** Calculate the MAC (message authentication code) of a message.
 *
 * \param attributes        Attributes of the key being used for the operation. It
 *                          must allow the usage PSA_KEY_USAGE_SIGN_MESSAGE.
 * \param key_buffer        Key data buffer
 * \param key_buffer_size   Key data buffer size
 * \param alg               The MAC algorithm to compute (\c PSA_ALG_XXX value
 *                          such that #PSA_ALG_IS_MAC(\p alg) is true).
 * \param[in] input         Buffer containing the input message.
 * \param input_length      Size of the \p input buffer in bytes.
 * \param[out] mac          Buffer where the MAC value is to be written.
 * \param mac_size          Size of the \p mac buffer in bytes.
 * \param[out] mac_length   On success, the number of bytes
 *                          that make up the MAC value.
 *
 * \retval #PSA_SUCCESS
 *         Success.
 * \retval #PSA_ERROR_INVALID_HANDLE \emptydescription
 * \retval #PSA_ERROR_NOT_PERMITTED \emptydescription
 * \retval #PSA_ERROR_INVALID_ARGUMENT
 *         \p key is not compatible with \p alg.
 * \retval #PSA_ERROR_NOT_SUPPORTED
 *         \p alg is not supported or is not a MAC algorithm.
 * \retval #PSA_ERROR_BUFFER_TOO_SMALL
 *         \p mac_size is too small
 * \retval #PSA_ERROR_INSUFFICIENT_MEMORY \emptydescription
 * \retval #PSA_ERROR_COMMUNICATION_FAILURE \emptydescription
 * \retval #PSA_ERROR_HARDWARE_FAILURE \emptydescription
 * \retval #PSA_ERROR_CORRUPTION_DETECTED \emptydescription
 * \retval #PSA_ERROR_STORAGE_FAILURE
 *         The key could not be retrieved from storage.
 * \retval #PSA_ERROR_BAD_STATE \emptydescription
 */
psa_status_t sli_hostcrypto_transparent_mac_compute(
  const psa_key_attributes_t *attributes,
  const uint8_t *key_buffer,
  size_t key_buffer_size,
  psa_algorithm_t alg,
  const uint8_t *input,
  size_t input_length,
  uint8_t *mac,
  size_t mac_size,
  size_t *mac_length);

/** Set up a multipart MAC calculation operation.
 *
 * This function sets up the calculation of the MAC
 * (message authentication code) of a byte string.
 *
 * \param[in,out] operation The operation object to set up. It must have
 *                          been initialized as per the documentation for
 *                          #psa_mac_operation_t and not yet in use.
 * \param attributes        Attributes of key to use
 *                          It must allow the usage PSA_KEY_USAGE_SIGN_MESSAGE.
 * \param key_buffer        Key data buffer to use for the operation. It
 *                          must remain valid until the operation terminates.
 * \param key_buffer_size   Size of key data buffer
 * \param alg               The MAC algorithm to compute (\c PSA_ALG_XXX value
 *                          such that #PSA_ALG_IS_MAC(\p alg) is true).
 *
 * \retval #PSA_SUCCESS
 *         Success.
 * \retval #PSA_ERROR_INVALID_HANDLE \emptydescription
 * \retval #PSA_ERROR_NOT_PERMITTED \emptydescription
 * \retval #PSA_ERROR_INVALID_ARGUMENT
 *         \p key is not compatible with \p alg.
 * \retval #PSA_ERROR_NOT_SUPPORTED
 *         \p alg is not supported or is not a MAC algorithm.
 * \retval #PSA_ERROR_INSUFFICIENT_MEMORY \emptydescription
 * \retval #PSA_ERROR_COMMUNICATION_FAILURE \emptydescription
 * \retval #PSA_ERROR_HARDWARE_FAILURE \emptydescription
 * \retval #PSA_ERROR_CORRUPTION_DETECTED \emptydescription
 * \retval #PSA_ERROR_STORAGE_FAILURE
 *         The key could not be retrieved from storage.
 * \retval #PSA_ERROR_BAD_STATE
 *         The operation state is not valid (it must be inactive)
 */
psa_status_t sli_hostcrypto_transparent_mac_sign_setup(
  sli_hostcrypto_transparent_mac_operation_t *operation,
  const psa_key_attributes_t *attributes,
  const uint8_t *key_buffer,
  size_t key_buffer_size,
  psa_algorithm_t alg);

/** Set up a multipart MAC verification operation.
 *
 * This function sets up the verification of the MAC
 * (message authentication code) of a byte string against an expected value.
 *
 * \param[in,out] operation The operation object to set up. It must have
 *                          been initialized as per the documentation for
 *                          #psa_mac_operation_t and not yet in use.
 * \param attributes        Attributes of key to use
 *                          It must allow the usage PSA_KEY_USAGE_VERIFY_MESSAGE.
 * \param key_buffer        Key data buffer to use for the operation. It
 *                          must remain valid until the operation terminates.
 * \param key_buffer_size   Size of key data buffer
 * \param alg               The MAC algorithm to compute (\c PSA_ALG_XXX value
 *                          such that #PSA_ALG_IS_MAC(\p alg) is true).
 *
 * \retval #PSA_SUCCESS
 *         Success.
 * \retval #PSA_ERROR_INVALID_HANDLE \emptydescription
 * \retval #PSA_ERROR_NOT_PERMITTED \emptydescription
 * \retval #PSA_ERROR_INVALID_ARGUMENT
 *         \c key is not compatible with \c alg.
 * \retval #PSA_ERROR_NOT_SUPPORTED
 *         \c alg is not supported or is not a MAC algorithm.
 * \retval #PSA_ERROR_INSUFFICIENT_MEMORY \emptydescription
 * \retval #PSA_ERROR_COMMUNICATION_FAILURE \emptydescription
 * \retval #PSA_ERROR_HARDWARE_FAILURE \emptydescription
 * \retval #PSA_ERROR_CORRUPTION_DETECTED \emptydescription
 * \retval #PSA_ERROR_STORAGE_FAILURE
 *         The key could not be retrieved from storage.
 * \retval #PSA_ERROR_BAD_STATE
 *         The operation state is not valid (it must be inactive)
 */
psa_status_t sli_hostcrypto_transparent_mac_verify_setup(
  sli_hostcrypto_transparent_mac_operation_t *operation,
  const psa_key_attributes_t *attributes,
  const uint8_t *key_buffer,
  size_t key_buffer_size,
  psa_algorithm_t alg);

/** Add a message fragment to a multipart MAC operation.
 *
 * \param[in,out] operation Active MAC operation.
 * \param[in] input         Buffer containing the message fragment to add to
 *                          the MAC calculation.
 * \param input_length      Size of the \p input buffer in bytes.
 *
 * \retval #PSA_SUCCESS
 *         Success.
 * \retval #PSA_ERROR_INSUFFICIENT_MEMORY \emptydescription
 * \retval #PSA_ERROR_COMMUNICATION_FAILURE \emptydescription
 * \retval #PSA_ERROR_HARDWARE_FAILURE \emptydescription
 * \retval #PSA_ERROR_CORRUPTION_DETECTED \emptydescription
 * \retval #PSA_ERROR_STORAGE_FAILURE \emptydescription
 * \retval #PSA_ERROR_BAD_STATE
 *         The operation state is not valid (it must be active)
 */
psa_status_t sli_hostcrypto_transparent_mac_update(
  sli_hostcrypto_transparent_mac_operation_t *operation,
  const uint8_t *input,
  size_t input_length);

/** Finish the calculation of the MAC of a message.
 *
 * When this function returns successfully, the operation becomes inactive.
 *
 * \param[in,out] operation Active MAC operation.
 * \param[out] mac          Buffer where the MAC value is to be written.
 * \param mac_size          Size of the \p mac buffer in bytes.
 * \param[out] mac_length   On success, the number of bytes
 *                          that make up the MAC value. This is always
 *                          #PSA_MAC_LENGTH(\c key_type, \c key_bits, \c alg)
 *                          where \c key_type and \c key_bits are the type and
 *                          bit-size respectively of the key and \c alg is the
 *                          MAC algorithm that is calculated.
 *
 * \retval #PSA_SUCCESS
 *         Success.
 * \retval #PSA_ERROR_BUFFER_TOO_SMALL
 *         The size of the \p mac buffer is too small. You can determine a
 *         sufficient buffer size by calling PSA_MAC_LENGTH().
 * \retval #PSA_ERROR_INSUFFICIENT_MEMORY \emptydescription
 * \retval #PSA_ERROR_COMMUNICATION_FAILURE \emptydescription
 * \retval #PSA_ERROR_HARDWARE_FAILURE \emptydescription
 * \retval #PSA_ERROR_CORRUPTION_DETECTED \emptydescription
 * \retval #PSA_ERROR_STORAGE_FAILURE \emptydescription
 * \retval #PSA_ERROR_BAD_STATE
 *         The operation state is not valid (it must be an active mac sign
 *         operation)
 */
psa_status_t sli_hostcrypto_transparent_mac_sign_finish(
  sli_hostcrypto_transparent_mac_operation_t *operation,
  uint8_t *mac,
  size_t mac_size,
  size_t *mac_length);

/** Finish the calculation of the MAC of a message and compare it with
 * an expected value.
 *
 * When this function returns successfully, the operation becomes inactive.
 *
 * \param[in,out] operation Active MAC operation.
 * \param[in] mac           Buffer containing the expected MAC value.
 * \param mac_length        Size of the \p mac buffer in bytes.
 *
 * \retval #PSA_SUCCESS
 *         The expected MAC is identical to the actual MAC of the message.
 * \retval #PSA_ERROR_INVALID_SIGNATURE
 *         The MAC of the message was calculated successfully, but it
 *         differs from the expected MAC.
 * \retval #PSA_ERROR_INSUFFICIENT_MEMORY \emptydescription
 * \retval #PSA_ERROR_COMMUNICATION_FAILURE \emptydescription
 * \retval #PSA_ERROR_HARDWARE_FAILURE \emptydescription
 * \retval #PSA_ERROR_CORRUPTION_DETECTED \emptydescription
 * \retval #PSA_ERROR_STORAGE_FAILURE \emptydescription
 * \retval #PSA_ERROR_BAD_STATE
 *         The operation state is not valid (it must be an active mac verify
 *         operation)
 */
psa_status_t sli_hostcrypto_transparent_mac_verify_finish(
  sli_hostcrypto_transparent_mac_operation_t *operation,
  const uint8_t *mac,
  size_t mac_length);

/** Abort a MAC operation.
 *
 * Aborting an operation frees all associated resources except for the
 * \p operation structure itself.
 *
 * You may call this function any time after the operation object has
 * been initialized
 *
 * \param[in,out] operation Initialized MAC operation.
 *
 * \retval #PSA_SUCCESS \emptydescription
 * \retval #PSA_ERROR_COMMUNICATION_FAILURE \emptydescription
 * \retval #PSA_ERROR_HARDWARE_FAILURE \emptydescription
 * \retval #PSA_ERROR_CORRUPTION_DETECTED \emptydescription
 * \retval #PSA_ERROR_BAD_STATE \emptydescription
 */
psa_status_t sli_hostcrypto_transparent_mac_abort(
  sli_hostcrypto_transparent_mac_operation_t *operation);

#ifdef __cplusplus
}
#endif

#endif // SLI_MBEDTLS_DEVICE_HC

#endif // SLI_HOSTCRYPTO_TRANSPARENT_FUNCTIONS_H
