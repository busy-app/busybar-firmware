/*******************************************************************************
 * @file  sl_si91x_psa_aes.c
 * @brief
 *******************************************************************************
 * # License
 * <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
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

// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------

#include "sli_si91x_crypto_driver_functions.h"
#include "sl_si91x_aes.h"
#include "sl_si91x_crypto.h"
#include "sl_status.h"
#include "sl_constants.h"
#include "sl_si91x_protocol_types.h"
#include "sl_si91x_driver.h"
#include <string.h>

#define SLI_SI91X_MAX_DATA_SIZE_IN_BYTES_FOR_AES 1408

static psa_status_t sli_si91x_psa_multipart_check_input_parameters(sli_si91x_crypto_cipher_operation_t *operation,
                                                                   const psa_key_attributes_t *attributes,
                                                                   const uint8_t *key_buffer,
                                                                   size_t key_buffer_size,
                                                                   psa_algorithm_t alg)
{
#if (defined(PSA_WANT_KEY_TYPE_AES) && defined(PSA_WANT_ALG_CTR))

  if (operation == NULL || attributes == NULL || key_buffer == NULL) {
    return PSA_ERROR_INVALID_ARGUMENT;
  }

  if (key_buffer_size < (psa_get_key_bits(attributes) / 8)
      || !(psa_get_key_bits(attributes) == 128 || psa_get_key_bits(attributes) == 192
           || psa_get_key_bits(attributes) == 256)) {
    return PSA_ERROR_NOT_SUPPORTED;
  }

  if (psa_get_key_type(attributes) != PSA_KEY_TYPE_AES) {
    return PSA_ERROR_NOT_SUPPORTED;
  }

  if (alg != PSA_ALG_CTR) {
    return PSA_ERROR_NOT_SUPPORTED;
  }

  return PSA_SUCCESS;

#else // PSA_WANT_ALG_AES && PSA_WANT_KEY_TYPE_AES

  (void)operation;
  (void)attributes;
  (void)key_buffer;
  (void)key_buffer_size;
  (void)alg;

  return PSA_ERROR_NOT_SUPPORTED;

#endif // PSA_WANT_ALG_AES && PSA_WANT_KEY_TYPE_AES
}

static void sli_si91x_configure_key_settings(const psa_key_attributes_t *attributes,
                                             void *key_config,
                                             const uint8_t *key_buffer,
                                             size_t key_buffer_size,
                                             bool is_multipart)
{
#if defined(SLI_SI917B0)
  sl_si91x_aes_key_config_b0_t *config_b0;
  if (is_multipart) {
    config_b0 = &((sli_si91x_psa_aes_multipart_config_t *)key_config)->key_config.b0;
  } else {
    config_b0 = &((sl_si91x_aes_config_t *)key_config)->key_config.b0;
  }

  /* Fetch key type from attributes */
  psa_key_location_t location = PSA_KEY_LIFETIME_GET_LOCATION(psa_get_key_lifetime(attributes));

  /* Determine key type based on storage location - different validation for different operation types */
  if (is_multipart) {
    config_b0->key_type = (location == PSA_KEY_LOCATION_LOCAL_STORAGE) ? SL_SI91X_TRANSPARENT_KEY
                                                                       : SL_SI91X_WRAPPED_KEY;
  } else {
    config_b0->key_type = (location == 0) ? SL_SI91X_TRANSPARENT_KEY : SL_SI91X_WRAPPED_KEY;
  }

  /* Set key_size from key_buffer_size */
  switch (key_buffer_size) {
    case 16:
      config_b0->key_size = SL_SI91X_AES_KEY_SIZE_128;
      break;
    case 24:
      config_b0->key_size = SL_SI91X_AES_KEY_SIZE_192;
      break;
    case 32:
      config_b0->key_size = SL_SI91X_AES_KEY_SIZE_256;
      break;
  }

  config_b0->key_slot = 0;
  memcpy(config_b0->key_buffer, key_buffer, config_b0->key_size);
#else
  sl_si91x_aes_key_config_a0_t *config_a0;
  if (is_multipart) {
    config_a0 = &((sli_si91x_psa_aes_multipart_config_t *)key_config)->key_config.a0;
  } else {
    config_a0 = &((sl_si91x_aes_config_t *)key_config)->key_config.a0;
  }

  config_a0->key        = (uint8_t *)malloc(key_buffer_size);
  config_a0->key_length = key_buffer_size;
  memcpy(config_a0->key, key_buffer, config_a0->key_length);
#endif
}

static void sli_si91x_set_input_config(const psa_key_attributes_t *attributes,
                                       sl_si91x_aes_config_t *config,
                                       const uint8_t *key_buffer,
                                       size_t key_buffer_size)
{
  sli_si91x_configure_key_settings(attributes, config, key_buffer, key_buffer_size, false);
}

static void sli_si91x_psa_multipart_set_input_config(const psa_key_attributes_t *attributes,
                                                     sli_si91x_psa_aes_multipart_config_t *config,
                                                     const uint8_t *key_buffer,
                                                     size_t key_buffer_size)
{
  sli_si91x_configure_key_settings(attributes, config, key_buffer, key_buffer_size, true);
}

// Configures AES mode, IV usage, and optional wrap-IV settings based on a PSA algorithm.
static psa_status_t sli_si91x_configure_aes_mode(psa_algorithm_t alg,
                                                 uint16_t *aes_mode,
                                                 sl_si91x_aes_config_t *config,
                                                 const uint8_t **aes_iv,
                                                 const uint8_t *iv)
{
  switch (alg) {
    case PSA_ALG_ECB_NO_PADDING:
      /* Setting mode as ECB_MODE */
      *aes_mode = SL_SI91X_AES_ECB;
#if defined(SLI_SI917B0)
      /* Setting wrap iv mode as WRAP_IV_ECB_MODE */
      config->key_config.b0.wrap_iv_mode = SL_SI91X_WRAP_IV_ECB_MODE;
#endif
      break;
    case PSA_ALG_CBC_NO_PADDING:
      /* Setting mode as CBC_MODE */
      *aes_mode = SL_SI91X_AES_CBC;
      /* setting aes_iv with iv */
      *aes_iv = iv;
#if defined(SLI_SI917B0)
      /* Setting wrap iv mode as WRAP_IV_CBC_MODE */
      config->key_config.b0.wrap_iv_mode = SL_SI91X_WRAP_IV_CBC_MODE;
#endif
      break;
    case PSA_ALG_CTR:
      /* Setting mode as CTR_MODE */
      *aes_mode = SL_SI91X_AES_CTR;
      /* setting aes_iv with iv */
      *aes_iv = iv;
      break;
    default:
      return PSA_ERROR_NOT_SUPPORTED;
  }
  return PSA_SUCCESS;
}

/*****************************************************************************
* Encrypt a message using a AES cipher.
*****************************************************************************/
psa_status_t sli_si91x_crypto_cipher_encrypt(const psa_key_attributes_t *attributes,
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
  (void)iv_length;
  (void)output_size;
  /* Local variables */
  psa_status_t status          = PSA_SUCCESS;
  sl_status_t si91x_status     = SL_STATUS_OK;
  sl_si91x_aes_config_t config = { 0 };
  uint16_t aes_mode;
  const uint8_t *aes_iv = NULL;

  //! Input check
  if ((attributes == NULL) || (key_buffer == NULL) || (input == NULL) || (output == NULL) || (output_length == NULL)) {
    return PSA_ERROR_INVALID_ARGUMENT;
  }

  if (((alg == PSA_ALG_CTR) || (alg == PSA_ALG_CBC_NO_PADDING)) && (iv == NULL)) {
    return PSA_ERROR_INVALID_ARGUMENT;
  }

  // Configure AES mode (ECB/CBC/CTR) and IV pointer based on algorithm; return on unsupported algorithm
  status = sli_si91x_configure_aes_mode(alg, &aes_mode, &config, &aes_iv, iv);
  if (status != PSA_SUCCESS) {
    return status;
  }

  config.aes_mode        = aes_mode;
  config.encrypt_decrypt = SL_SI91X_AES_ENCRYPT;
  config.msg             = input;
  config.msg_length      = input_length;
  config.iv              = aes_iv;

  sli_si91x_set_input_config(attributes, &config, key_buffer, key_buffer_size);

  /* Calling sl_si91x_aes() for AES encryption */
  si91x_status = sl_si91x_aes(&config, output);

#if !defined(SLI_SI917B0)
  free(config.key_config.a0.key);
#endif

  /* gets the si91x error codes and returns its equivalent psa_status codes */
  status = convert_si91x_error_code_to_psa_status(si91x_status);

  if (status == PSA_SUCCESS) {
    *output_length = input_length;
  }

  return status;
}

/*****************************************************************************
 * Decrypt a message using a AES cipher.
******************************************************************************/
psa_status_t sli_si91x_crypto_cipher_decrypt(const psa_key_attributes_t *attributes,
                                             const uint8_t *key_buffer,
                                             size_t key_buffer_size,
                                             psa_algorithm_t alg,
                                             const uint8_t *input,
                                             size_t input_length,
                                             uint8_t *output,
                                             size_t output_size,
                                             size_t *output_length)
{
  (void)output_size;
  /* Local variables */
  psa_status_t status          = PSA_SUCCESS;
  sl_status_t si91x_status     = SL_STATUS_OK;
  sl_si91x_aes_config_t config = { 0 };
  uint16_t aes_mode;
  const uint8_t *aes_iv = NULL;

  //! Input pointer check
  if ((attributes == NULL) || (key_buffer == NULL) || (input == NULL) || (output == NULL) || (output_length == NULL)) {
    return PSA_ERROR_INVALID_ARGUMENT;
  }

  // For decrypt, IV is extracted from input for CBC and CTR modes
  const uint8_t *iv = (alg == PSA_ALG_CBC_NO_PADDING || alg == PSA_ALG_CTR) ? input : NULL;

  status = sli_si91x_configure_aes_mode(alg, &aes_mode, &config, &aes_iv, iv);
  if (status != PSA_SUCCESS) {
    return status;
  }

  // Adjust input pointer and length for CBC and CTR modes
  if (alg == PSA_ALG_CBC_NO_PADDING || alg == PSA_ALG_CTR) {
    input += SL_SI91X_IV_SIZE;
    input_length -= SL_SI91X_IV_SIZE;
  }

  config.aes_mode        = aes_mode;
  config.encrypt_decrypt = SL_SI91X_AES_DECRYPT;
  config.msg             = input;
  config.msg_length      = input_length;
  config.iv              = aes_iv;

  sli_si91x_set_input_config(attributes, &config, key_buffer, key_buffer_size);

  /* Calling sl_si91x_aes() for AES decryption */
  si91x_status = sl_si91x_aes(&config, output);

#if !defined(SLI_SI917B0)
  free(config.key_config.a0.key);
#endif

  /* gets the si91x error codes and returns its equivalent psa_status codes */
  status = convert_si91x_error_code_to_psa_status(si91x_status);

  if (status == PSA_SUCCESS) {
    *output_length = input_length;
  }

  return status;
}

/*****************************************************************************
* Encrypt a message using a AES cipher multipart.
*****************************************************************************/
psa_status_t sli_si91x_crypto_cipher_encrypt_setup(sli_si91x_crypto_cipher_operation_t *operation,
                                                   const psa_key_attributes_t *attributes,
                                                   const uint8_t *key_buffer,
                                                   size_t key_buffer_size,
                                                   psa_algorithm_t alg)
{
  psa_status_t status =
    sli_si91x_psa_multipart_check_input_parameters(operation, attributes, key_buffer, key_buffer_size, alg);

  if (status != PSA_SUCCESS) {
    return status;
  }

  // Reset context.
  memset(operation, 0, sizeof(sli_si91x_crypto_cipher_operation_t));

  // Set up context.
  memcpy(&operation->alg, &alg, sizeof(alg));

  /* Setting mode as CTR_MODE */
  operation->config.aes_mode        = SL_SI91X_AES_CTR;
  operation->config.encrypt_decrypt = SL_SI91X_AES_ENCRYPT;
  operation->config.chunk_flag      = SL_SI91X_AES_FIRST_CHUNK;
  operation->config.iv_flag         = 0;

  sli_si91x_psa_multipart_set_input_config(attributes, &operation->config, key_buffer, key_buffer_size);

  return PSA_SUCCESS;
}

psa_status_t sli_si91x_crypto_cipher_decrypt_setup(sli_si91x_crypto_cipher_operation_t *operation,
                                                   const psa_key_attributes_t *attributes,
                                                   const uint8_t *key_buffer,
                                                   size_t key_buffer_size,
                                                   psa_algorithm_t alg)
{
  psa_status_t status =
    sli_si91x_psa_multipart_check_input_parameters(operation, attributes, key_buffer, key_buffer_size, alg);

  if (status != PSA_SUCCESS) {
    return status;
  }

  // Reset context.
  memset(operation, 0, sizeof(sli_si91x_crypto_cipher_operation_t));

  // Set up context.
  memcpy(&operation->alg, &alg, sizeof(alg));

  /* Setting mode as CTR_MODE */
  operation->config.aes_mode        = SL_SI91X_AES_CTR;
  operation->config.encrypt_decrypt = SL_SI91X_AES_DECRYPT;
  operation->config.chunk_flag      = SL_SI91X_AES_FIRST_CHUNK;

  sli_si91x_psa_multipart_set_input_config(attributes, &operation->config, key_buffer, key_buffer_size);

  return PSA_SUCCESS;
}

psa_status_t sli_si91x_crypto_cipher_update(sli_si91x_crypto_cipher_operation_t *operation,
                                            const uint8_t *input,
                                            size_t input_length,
                                            uint8_t *output,
                                            size_t output_size,
                                            size_t *output_length)
{
#if (defined(PSA_WANT_KEY_TYPE_AES) && defined(PSA_WANT_ALG_CTR))

  /* Local variables */
  psa_status_t status      = PSA_ERROR_CORRUPTION_DETECTED;
  sl_status_t si91x_status = SL_STATUS_OK;
  *output_length           = 0;

  // Argument check.
  if (operation == NULL || (input == NULL && input_length > 0) || (output == NULL && output_size > 0)
      || output_length == NULL) {
    return PSA_ERROR_INVALID_ARGUMENT;
  }

  if (operation->config.iv_flag == 0) {
    return PSA_ERROR_BAD_STATE;
  }

  if (input_length == 0 || input_length % 16 != 0 || input_length > SLI_SI91X_MAX_DATA_SIZE_IN_BYTES_FOR_AES) {
    return PSA_ERROR_INVALID_ARGUMENT;
  }

  if (output_size < input_length) {
    return PSA_ERROR_BUFFER_TOO_SMALL;
  }

  if (input_length == 0) {
    // We don't need to do anything if the input length is zero.
    *output_length = 0;
    return PSA_SUCCESS;
  }

  operation->config.msg = input;

  // Populating sl_si91x_aes_config_t structure with sli_si91x_psa_aes_multipart_config_t structure
  sl_si91x_aes_config_t config;
  memset(&config, 0, sizeof(sl_si91x_aes_config_t));

  config.aes_mode        = operation->config.aes_mode;
  config.encrypt_decrypt = operation->config.encrypt_decrypt;
  config.msg             = operation->config.msg;
  config.msg_length      = operation->config.msg_length;
  config.key_config      = operation->config.key_config;
  config.iv              = operation->config.iv;

  si91x_status = sl_si91x_aes_multipart(&config, input_length, operation->config.chunk_flag, output);
  status       = convert_si91x_error_code_to_psa_status(si91x_status);

  if (status != PSA_SUCCESS) {
    return status;
  }

  if (operation->config.chunk_flag == SL_SI91X_AES_FIRST_CHUNK) {
    operation->config.chunk_flag = SL_SI91X_AES_MIDDLE_CHUNK;
  }

  *output_length = input_length;

  return status;

#else // PSA_WANT_ALG_AES && PSA_WANT_KEY_TYPE_AES

  (void)operation;
  (void)input;
  (void)input_length;
  (void)output;
  (void)output_size;
  (void)output_length;

  return PSA_ERROR_NOT_SUPPORTED;

#endif // PSA_WANT_ALG_AES && PSA_WANT_KEY_TYPE_AES
}

psa_status_t sli_si91x_crypto_cipher_set_iv(sli_si91x_crypto_cipher_operation_t *operation,
                                            const uint8_t *iv,
                                            size_t iv_length)
{
#if (defined(PSA_WANT_KEY_TYPE_AES) && defined(PSA_WANT_ALG_CTR))

  if (operation == NULL || iv == NULL) {
    return PSA_ERROR_INVALID_ARGUMENT;
  }

  if (iv_length != SL_SI91X_IV_SIZE) {
    return PSA_ERROR_INVALID_ARGUMENT;
  }

  if (operation->config.key_config.b0.key_size == 0) {
    // context hasn't been properly initialised.
    return PSA_ERROR_BAD_STATE;
  }

  // Setting iv and iv_flag
  memcpy(operation->config.iv, iv, iv_length);
  operation->config.iv_flag = 1;

  return PSA_SUCCESS;

#else // PSA_WANT_ALG_AES && PSA_WANT_KEY_TYPE_AES

  (void)operation;
  (void)iv;
  (void)iv_length;

  return PSA_ERROR_NOT_SUPPORTED;

#endif // PSA_WANT_ALG_AES && PSA_WANT_KEY_TYPE_AES
}

psa_status_t sli_si91x_crypto_cipher_abort(sli_si91x_crypto_cipher_operation_t *operation)
{
#if (defined(PSA_WANT_KEY_TYPE_AES) && defined(PSA_WANT_ALG_CTR))

  if (operation != NULL) {
    // Wipe context.
    memset(operation, 0, sizeof(*operation));
  }

  return PSA_SUCCESS;

#else // PSA_WANT_ALG_AES && PSA_WANT_KEY_TYPE_AES

  (void)operation;

  return PSA_ERROR_NOT_SUPPORTED;

#endif // PSA_WANT_ALG_AES && PSA_WANT_KEY_TYPE_AES
}

psa_status_t sli_si91x_crypto_cipher_finish(sli_si91x_crypto_cipher_operation_t *operation,
                                            uint8_t *output,
                                            size_t output_size,
                                            size_t *output_length)
{
#if (defined(PSA_WANT_KEY_TYPE_AES) && defined(PSA_WANT_ALG_CTR))

  psa_status_t status = PSA_ERROR_GENERIC_ERROR;
  (void)output;
  (void)output_size;
  *output_length = 0;

  // Argument check.
  if (operation == NULL) {
    return PSA_ERROR_INVALID_ARGUMENT;
  }

  switch (operation->alg) {
    // Stream cipher modes.
    case PSA_ALG_CTR:
      status = PSA_SUCCESS;
      break;
    default:
      status = PSA_ERROR_BAD_STATE;
      break;
  }

  // Wipe context.
  memset(operation, 0, sizeof(sli_si91x_crypto_cipher_operation_t));

  return status;

#else // PSA_WANT_ALG_AES && PSA_WANT_KEY_TYPE_AES

  (void)operation;
  (void)output;
  (void)output_size;
  (void)output_length;

  return PSA_ERROR_NOT_SUPPORTED;

#endif // PSA_WANT_ALG_AES && PSA_WANT_KEY_TYPE_AES
}
