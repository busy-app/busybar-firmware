/*******************************************************************************
 * @file  sl_si91x_psa_aead.c
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
#if defined(SLI_PSA_DRIVER_FEATURE_CCM)
#include "sl_si91x_ccm.h"
#endif
#if defined(SLI_PSA_DRIVER_FEATURE_GCM)
#include "sl_si91x_gcm.h"
#endif
#if defined(SLI_PSA_DRIVER_FEATURE_CHACHAPOLY)
#include "sl_si91x_chachapoly.h"
#endif
#if defined(SLI_SECURE_KEY_STORAGE_DEVICE_SI91X)
#include "sl_si91x_psa_wrap.h"
#endif
#include "sl_si91x_crypto.h"
#include "sl_status.h"
#include "sl_constants.h"
#include "sl_si91x_protocol_types.h"
#include "sl_si91x_driver.h"
#include <string.h>

/**
 * \brief Verify the parameters of authenticated encryption and decryption operations.
 *
 * \param[in]  attributes         The attributes of the key to use for the
 *                                operation.
 * \param      alg                The AEAD algorithm to compute.
 * \param      nonce_length       Size of the nonce buffer in bytes. This must
 *                                be appropriate for the selected algorithm.
 *                                The default nonce size is
 *                                PSA_AEAD_NONCE_LENGTH(key_type, alg) where
 *                                key_type is the type of key.
 * \param      additional_data_length  Size of additional_data in bytes.
 *
 * \retval #PSA_SUCCESS Success.
 * \retval #PSA_ERROR_NOT_SUPPORTED
 *         \p alg is not supported.
 * \retval #PSA_ERROR_INVALID_ARGUMENT
 */
static psa_status_t sli_si91x_check_aead_parameters(const psa_key_attributes_t *attributes,
                                                    psa_algorithm_t alg,
                                                    size_t nonce_length,
                                                    size_t additional_data_length)
{
  size_t tag_length = PSA_AEAD_TAG_LENGTH(psa_get_key_type(attributes), psa_get_key_bits(attributes), alg);

  switch (PSA_ALG_AEAD_WITH_SHORTENED_TAG(alg, 0)) {
#if defined(SLI_PSA_DRIVER_FEATURE_CCM)
    case PSA_ALG_AEAD_WITH_SHORTENED_TAG(PSA_ALG_CCM, 0):
      // verify key type
      if (psa_get_key_type(attributes) != PSA_KEY_TYPE_AES) {
        return PSA_ERROR_NOT_SUPPORTED;
      }
      switch (psa_get_key_bits(attributes)) {
        case 128: // Fallthrough
        case 192: // Fallthrough
        case 256:
          break;
        default:
          return PSA_ERROR_INVALID_ARGUMENT;
      }
      // verify nonce and tag lengths
      if ((tag_length < 4) || (tag_length > 16) || (tag_length % 2 != 0) || (nonce_length < SLI_SI91X_CCM_IV_MIN_SIZE)
          || (nonce_length > SLI_SI91X_CCM_IV_MAX_SIZE)) {
        return PSA_ERROR_INVALID_ARGUMENT;
      }
      break;
#endif // SLI_PSA_DRIVER_FEATURE_CCM
#if defined(SLI_PSA_DRIVER_FEATURE_GCM)
    case PSA_ALG_AEAD_WITH_SHORTENED_TAG(PSA_ALG_GCM, 0):
      // verify key type
      if (psa_get_key_type(attributes) != PSA_KEY_TYPE_AES) {
        return PSA_ERROR_NOT_SUPPORTED;
      }
      switch (psa_get_key_bits(attributes)) {
        case 128: // Fallthrough
        case 192: // Fallthrough
        case 256:
          break;
        default:
          return PSA_ERROR_INVALID_ARGUMENT;
      }
      // verify nonce and tag lengths
      if ((tag_length < 4) || (tag_length > 16) || nonce_length != SLI_SI91X_GCM_IV_SIZE) {
        return PSA_ERROR_INVALID_ARGUMENT;
      }
      break;
#endif // SLI_PSA_DRIVER_FEATURE_GCM
#if defined(SLI_PSA_DRIVER_FEATURE_CHACHAPOLY)
    case PSA_ALG_AEAD_WITH_SHORTENED_TAG(PSA_ALG_CHACHA20_POLY1305, 0):
      // verify key type
      if (psa_get_key_type(attributes) != PSA_KEY_TYPE_CHACHA20) {
        return PSA_ERROR_NOT_SUPPORTED;
      }
      switch (psa_get_key_bits(attributes)) {
        case 256:
          break;
        default:
          return PSA_ERROR_INVALID_ARGUMENT;
      }
      // verify nonce and tag lengths
      if (tag_length != SL_SI91X_TAG_SIZE || nonce_length != SLI_SI91X_CHACHAPOLY_IV_SIZE) {
        return PSA_ERROR_INVALID_ARGUMENT;
      }
      break;
#endif // SLI_PSA_DRIVER_FEATURE_CHACHAPOLY
    default:
      return PSA_ERROR_NOT_SUPPORTED;
  }

#if !defined(SLI_PSA_DRIVER_FEATURE_CCM) && !defined(SLI_PSA_DRIVER_FEATURE_GCM) \
  && !defined(SLI_PSA_DRIVER_FEATURE_CHACHAPOLY)
  UNUSED_PARAMETER(nonce_length);
  UNUSED_VARIABLE(tag_length);
#endif
  UNUSED_PARAMETER(additional_data_length);

  return PSA_SUCCESS;
}

#if defined(SLI_PSA_DRIVER_FEATURE_CCM)
static void sli_si91x_set_input_config_ccm(const psa_key_attributes_t *attributes,
                                           sl_si91x_ccm_config_t *config_ccm,
                                           const uint8_t *key_buffer,
                                           size_t key_buffer_size)
{
#if defined(SLI_SI917B0)
  /* Fetch key type from attributes */
  psa_key_location_t location_ccm = PSA_KEY_LIFETIME_GET_LOCATION(psa_get_key_lifetime(attributes));
  if (location_ccm == 0) {
    config_ccm->key_config.b0.key_type = SL_SI91X_TRANSPARENT_KEY;
  } else {
    config_ccm->key_config.b0.key_type = SL_SI91X_WRAPPED_KEY;
  }

  /* Set key_size from key_buffer_size */
  if (key_buffer_size == 16) {
    config_ccm->key_config.b0.key_size = SL_SI91X_CCM_KEY_SIZE_128;
  }
  if (key_buffer_size == 24) {
    config_ccm->key_config.b0.key_size = SL_SI91X_CCM_KEY_SIZE_192;
  }
  if (key_buffer_size == 32) {
    config_ccm->key_config.b0.key_size = SL_SI91X_CCM_KEY_SIZE_256;
  }

  config_ccm->key_config.b0.key_slot     = 0;
  config_ccm->key_config.b0.wrap_iv_mode = SL_SI91X_WRAP_IV_CBC_MODE;
  if (config_ccm->key_config.b0.wrap_iv_mode == SL_SI91X_WRAP_IV_CBC_MODE) {
    memcpy(config_ccm->key_config.b0.wrap_iv, WRAP_IV, SL_SI91X_IV_SIZE);
  }
  memcpy(config_ccm->key_config.b0.key_buffer, key_buffer, config_ccm->key_config.b0.key_size);
#else
  config_ccm->key_config.a0.key        = (uint8_t *)malloc(key_buffer_size);
  config_ccm->key_config.a0.key_length = key_buffer_size;
  memcpy(config_ccm->key_config.a0.key, key_buffer, config_ccm->key_config.a0.key_length);
#endif
}
#endif

#if defined(SLI_PSA_DRIVER_FEATURE_GCM)
static void sli_si91x_set_input_config_gcm(const psa_key_attributes_t *attributes,
                                           sl_si91x_gcm_config_t *config_gcm,
                                           const uint8_t *key_buffer,
                                           size_t key_buffer_size)
{
#if defined(SLI_SI917B0)
  /* Fetch key type from attributes */
  psa_key_location_t location_gcm = PSA_KEY_LIFETIME_GET_LOCATION(psa_get_key_lifetime(attributes));
  if (location_gcm == 0) {
    config_gcm->key_config.b0.key_type = SL_SI91X_TRANSPARENT_KEY;
  } else {
    config_gcm->key_config.b0.key_type = SL_SI91X_WRAPPED_KEY;
  }

  /* Set key_size from key_buffer_size */
  if (key_buffer_size == 16) {
    config_gcm->key_config.b0.key_size = SL_SI91X_GCM_KEY_SIZE_128;
  }
  if (key_buffer_size == 24) {
    config_gcm->key_config.b0.key_size = SL_SI91X_GCM_KEY_SIZE_192;
  }
  if (key_buffer_size == 32) {
    config_gcm->key_config.b0.key_size = SL_SI91X_GCM_KEY_SIZE_256;
  }

  config_gcm->gcm_mode                   = SL_SI91X_GCM_MODE;
  config_gcm->key_config.b0.key_slot     = 0;
  config_gcm->key_config.b0.wrap_iv_mode = SL_SI91X_WRAP_IV_CBC_MODE;
  if (config_gcm->key_config.b0.wrap_iv_mode == SL_SI91X_WRAP_IV_CBC_MODE) {
    memcpy(config_gcm->key_config.b0.wrap_iv, WRAP_IV, SL_SI91X_IV_SIZE);
  }
  memcpy(config_gcm->key_config.b0.key_buffer, key_buffer, config_gcm->key_config.b0.key_size);
#else
  config_gcm->key_config.a0.key        = (uint8_t *)malloc(key_buffer_size);
  config_gcm->key_config.a0.key_length = key_buffer_size;
  memcpy(config_gcm->key_config.a0.key, key_buffer, config_gcm->key_config.a0.key_length);
#endif
}
#endif

#if defined(SLI_PSA_DRIVER_FEATURE_CHACHAPOLY)
static void sli_si91x_set_input_config_chachapoly(const psa_key_attributes_t *attributes,
                                                  sl_si91x_chachapoly_config_t *config_chachapoly,
                                                  const uint8_t *key_buffer)
{
#if defined(SLI_SI917B0)
  /* Fetch key type from attributes */
  psa_key_location_t location_chachapoly = PSA_KEY_LIFETIME_GET_LOCATION(psa_get_key_lifetime(attributes));
  if (location_chachapoly == 0) {
    config_chachapoly->key_config.b0.key_type = SL_SI91X_TRANSPARENT_KEY;
  } else {
    config_chachapoly->key_config.b0.key_type = SL_SI91X_WRAPPED_KEY;
  }

  config_chachapoly->key_config.b0.key_size     = SL_SI91X_CHACHAPOLY_KEY_SIZE_256;
  config_chachapoly->key_config.b0.key_slot     = 0;
  config_chachapoly->key_config.b0.wrap_iv_mode = SL_SI91X_WRAP_IV_CBC_MODE;
  if (config_chachapoly->key_config.b0.wrap_iv_mode == SL_SI91X_WRAP_IV_CBC_MODE) {
    memcpy(config_chachapoly->key_config.b0.wrap_iv, WRAP_IV, SL_SI91X_IV_SIZE);
  }
  memcpy(config_chachapoly->key_config.b0.key_buffer, key_buffer, config_chachapoly->key_config.b0.key_size);
#else
  memcpy(config_chachapoly->key_config.a0.key_chacha, key_buffer, SL_SI91X_CHACHAPOLY_KEY_SIZE_256);
#endif
}
#endif

/*****************************************************************************
 * Encrypto a message using AEAD algorithms.
******************************************************************************/
psa_status_t sli_si91x_crypto_aead_encrypt(const psa_key_attributes_t *attributes,
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
  psa_status_t status = PSA_ERROR_GENERIC_ERROR;

  // Input check
  if ((key_buffer == NULL) || (key_buffer_size <= 0) || (attributes == NULL) || (nonce == NULL)
      || (additional_data == NULL && additional_data_length > 0) || (plaintext == NULL && plaintext_length > 0)
      || (plaintext_length > 0 && (ciphertext == NULL || ciphertext_size == 0)) || (ciphertext_length == NULL)) {
    return PSA_ERROR_INVALID_ARGUMENT;
  }

  // Verify that the driver supports the given parameters
  status = sli_si91x_check_aead_parameters(attributes, alg, nonce_length, additional_data_length);
  if (status != PSA_SUCCESS) {
    return status;
  }

  psa_key_type_t key_type = psa_get_key_type(attributes);
  size_t key_bits         = psa_get_key_bits(attributes);
  uint8_t tag_length      = PSA_AEAD_TAG_LENGTH(key_type, key_bits, alg);

  // Check sufficient output buffer size.
  if (ciphertext_size < (plaintext_length + tag_length)) {
    return PSA_ERROR_BUFFER_TOO_SMALL;
  }
  *ciphertext_length = 0;

  sl_status_t si91x_status;
  switch (PSA_ALG_AEAD_WITH_SHORTENED_TAG(alg, 0)) {
#if defined(SLI_PSA_DRIVER_FEATURE_CCM)
    case PSA_ALG_AEAD_WITH_SHORTENED_TAG(PSA_ALG_CCM, 0):
      sl_si91x_ccm_config_t config_ccm;
      config_ccm.encrypt_decrypt = SL_SI91X_CCM_ENCRYPT;
      config_ccm.msg             = plaintext;
      config_ccm.msg_length      = plaintext_length;
      config_ccm.nonce           = nonce;
      config_ccm.nonce_length    = nonce_length;
      config_ccm.ad              = additional_data;
      config_ccm.ad_length       = additional_data_length;
      config_ccm.tag             = ciphertext + plaintext_length;
      config_ccm.tag_length      = tag_length;

      sli_si91x_set_input_config_ccm(attributes, &config_ccm, key_buffer, key_buffer_size);

      /* Calling sl_si91x_ccm() for CCM encryption */
      si91x_status = sl_si91x_ccm(&config_ccm, ciphertext);

#if !defined(SLI_SI917B0)
      free(config_ccm.key_config.a0.key);
#endif
      /* gets the si91x error codes and returns its equivalent psa_status codes */
      status = convert_si91x_error_code_to_psa_status(si91x_status);

      break;
#endif /* SLI_PSA_DRIVER_FEATURE_CCM */
#if defined(SLI_PSA_DRIVER_FEATURE_GCM)
    case PSA_ALG_AEAD_WITH_SHORTENED_TAG(PSA_ALG_GCM, 0):
      sl_si91x_gcm_config_t config_gcm;
      config_gcm.encrypt_decrypt = SL_SI91X_GCM_ENCRYPT;
      config_gcm.dma_use         = SL_SI91X_GCM_DMA_ENABLE;
      config_gcm.msg             = plaintext;
      config_gcm.msg_length      = plaintext_length;
      config_gcm.nonce           = nonce;
      config_gcm.nonce_length    = nonce_length;
      config_gcm.ad              = additional_data;
      config_gcm.ad_length       = additional_data_length;

      sli_si91x_set_input_config_gcm(attributes, &config_gcm, key_buffer, key_buffer_size);

      /* Calling sl_si91x_gcm() for GCM encryption */
      si91x_status = sl_si91x_gcm(&config_gcm, ciphertext);

#if !defined(SLI_SI917B0)
      free(config_gcm.key_config.a0.key);
#endif
      /* gets the si91x error codes and returns its equivalent psa_status codes */
      status = convert_si91x_error_code_to_psa_status(si91x_status);

      break;
#endif /* SLI_PSA_DRIVER_FEATURE_GCM */
#if defined(SLI_PSA_DRIVER_FEATURE_CHACHAPOLY)
    case PSA_ALG_AEAD_WITH_SHORTENED_TAG(PSA_ALG_CHACHA20_POLY1305, 0):
      uint8_t temp_nonce[16] = { 0 };
      temp_nonce[0]          = 0x01;
      memcpy(temp_nonce + 4, nonce, 12);
      sl_si91x_chachapoly_config_t config_chachapoly;
      config_chachapoly.encrypt_decrypt = SL_SI91X_CHACHAPOLY_ENCRYPT;
      config_chachapoly.dma_use         = SL_SI91X_CHACHAPOLY_DMA_ENABLE;
      config_chachapoly.chachapoly_mode = SL_SI91X_CHACHA20POLY1305_MODE;
      config_chachapoly.msg             = plaintext;
      config_chachapoly.msg_length      = plaintext_length;
      config_chachapoly.nonce           = temp_nonce;
      config_chachapoly.ad              = additional_data;
      config_chachapoly.ad_length       = additional_data_length;

      sli_si91x_set_input_config_chachapoly(attributes, &config_chachapoly, key_buffer);

      /* Calling sl_si91x_chachapoly() for CHACHAPOLY encryption */
      si91x_status = sl_si91x_chachapoly(&config_chachapoly, ciphertext);

      /* gets the si91x error codes and returns its equivalent psa_status codes */
      status = convert_si91x_error_code_to_psa_status(si91x_status);

      break;
#endif /* SLI_PSA_DRIVER_FEATURE_CHACHAPOLY */
    default:
      (void)status;
      (void)key_buffer;
#if !defined(SLI_PSA_DRIVER_FEATURE_CCM) && !defined(SLI_PSA_DRIVER_FEATURE_GCM) \
  && !defined(SLI_PSA_DRIVER_FEATURE_CHACHAPOLY)
      UNUSED_VARIABLE(si91x_status);
#endif
      status = PSA_ERROR_NOT_SUPPORTED;
      break;
  }
  if (status == PSA_SUCCESS) {
    *ciphertext_length = plaintext_length + tag_length;
  }
  return status;
}

/*****************************************************************************
 * Decrypt a message using AEAD algorithms.
******************************************************************************/
psa_status_t sli_si91x_crypto_aead_decrypt(const psa_key_attributes_t *attributes,
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
  psa_status_t status = PSA_ERROR_GENERIC_ERROR;

  //! Input check
  if ((attributes == NULL) || (key_buffer == NULL) || (key_buffer_size <= 0) || (nonce == NULL)
      || (additional_data == NULL && additional_data_length > 0) || (ciphertext == NULL && ciphertext_length > 0)
      || (plaintext == NULL && plaintext_size > 0) || (plaintext_length == NULL)) {
    return PSA_ERROR_INVALID_ARGUMENT;
  }

  // Verify that the driver supports the given parameters
  status = sli_si91x_check_aead_parameters(attributes, alg, nonce_length, additional_data_length);
  if (status != PSA_SUCCESS) {
    return status;
  }
  *plaintext_length = 0;

  psa_key_type_t key_type = psa_get_key_type(attributes);
  size_t key_bits         = psa_get_key_bits(attributes);
  uint8_t tag_length      = PSA_AEAD_TAG_LENGTH(key_type, key_bits, alg);

  // Check sufficient output buffer size.
  if (plaintext_size < (ciphertext_length - tag_length)) {
    return PSA_ERROR_BUFFER_TOO_SMALL;
  }

  int32_t si91x_status;
  switch (PSA_ALG_AEAD_WITH_SHORTENED_TAG(alg, 0)) {
#if defined(SLI_PSA_DRIVER_FEATURE_CCM)
    case PSA_ALG_AEAD_WITH_SHORTENED_TAG(PSA_ALG_CCM, 0):
      sl_si91x_ccm_config_t config_ccm = { 0 };
      config_ccm.encrypt_decrypt       = SL_SI91X_CCM_DECRYPT;
      config_ccm.msg                   = ciphertext;
      config_ccm.msg_length            = ciphertext_length - tag_length;
      config_ccm.nonce                 = nonce;
      config_ccm.nonce_length          = nonce_length;
      config_ccm.ad                    = additional_data;
      config_ccm.ad_length             = additional_data_length;
      config_ccm.tag                   = (uint8_t *)ciphertext + (ciphertext_length - tag_length);
      config_ccm.tag_length            = tag_length;

      sli_si91x_set_input_config_ccm(attributes, &config_ccm, key_buffer, key_buffer_size);

      /* Calling sl_si91x_ccm() for CCM decryption */
      si91x_status = sl_si91x_ccm(&config_ccm, plaintext);

#if !defined(SLI_SI917B0)
      free(config_ccm.key_config.a0.key);
#endif
      status = convert_si91x_error_code_to_psa_status(si91x_status);
      break;
#endif /* SLI_PSA_DRIVER_FEATURE_CCM */
#if defined(SLI_PSA_DRIVER_FEATURE_GCM)
    case PSA_ALG_AEAD_WITH_SHORTENED_TAG(PSA_ALG_GCM, 0):
      sl_si91x_gcm_config_t config_gcm = { 0 };
      config_gcm.encrypt_decrypt       = SL_SI91X_GCM_DECRYPT;
      config_gcm.dma_use               = SL_SI91X_GCM_DMA_ENABLE;
      config_gcm.msg                   = ciphertext;
      config_gcm.msg_length            = ciphertext_length - tag_length;
      config_gcm.nonce                 = nonce;
      config_gcm.nonce_length          = nonce_length;
      config_gcm.ad                    = additional_data;
      config_gcm.ad_length             = additional_data_length;

      sli_si91x_set_input_config_gcm(attributes, &config_gcm, key_buffer, key_buffer_size);

      /* Calling sl_si91x_gcm() for GCM decryption */
      si91x_status = sl_si91x_gcm(&config_gcm, plaintext);

#if !defined(SLI_SI917B0)
      free(config_gcm.key_config.a0.key);
#endif
      status = convert_si91x_error_code_to_psa_status(si91x_status);
      break;
#endif /* SLI_PSA_DRIVER_FEATURE_GCM */
#if defined(SLI_PSA_DRIVER_FEATURE_CHACHAPOLY)
    case PSA_ALG_AEAD_WITH_SHORTENED_TAG(PSA_ALG_CHACHA20_POLY1305, 0):
      uint8_t temp_nonce[16] = { 0 };
      temp_nonce[0]          = 0x01;
      memcpy(temp_nonce + 4, nonce, 12);
      sl_si91x_chachapoly_config_t config_chachapoly;
      config_chachapoly.encrypt_decrypt = SL_SI91X_CHACHAPOLY_DECRYPT;
      config_chachapoly.dma_use         = SL_SI91X_CHACHAPOLY_DMA_ENABLE;
      config_chachapoly.chachapoly_mode = SL_SI91X_CHACHA20POLY1305_MODE;
      config_chachapoly.msg             = ciphertext;
      config_chachapoly.msg_length      = ciphertext_length - tag_length;
      config_chachapoly.nonce           = temp_nonce;
      config_chachapoly.ad              = additional_data;
      config_chachapoly.ad_length       = additional_data_length;

      sli_si91x_set_input_config_chachapoly(attributes, &config_chachapoly, key_buffer);

      /* Calling sl_si91x_chachapoly() for CHACHAPOLY decryption */
      si91x_status = sl_si91x_chachapoly(&config_chachapoly, plaintext);

      /* gets the si91x error codes and returns its equivalent psa_status codes */
      status = convert_si91x_error_code_to_psa_status(si91x_status);

      break;
#endif /* SLI_PSA_DRIVER_FEATURE_CHACHAPOLY */
    default:
      (void)status;
      (void)key_buffer;
#if !defined(SLI_PSA_DRIVER_FEATURE_CCM) && !defined(SLI_PSA_DRIVER_FEATURE_GCM) \
  && !defined(SLI_PSA_DRIVER_FEATURE_CHACHAPOLY)
      UNUSED_VARIABLE(si91x_status);
#endif
      status = PSA_ERROR_NOT_SUPPORTED;
      break;
  }
  if (status == PSA_SUCCESS) {
    *plaintext_length = ciphertext_length - tag_length;
  }
  return status;
}
