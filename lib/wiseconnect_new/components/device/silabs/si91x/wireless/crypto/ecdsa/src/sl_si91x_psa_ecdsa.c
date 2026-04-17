/*******************************************************************************
 * @file  sl_si91x_psa_ecdsa.c
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
#if defined(SLI_SECURE_KEY_STORAGE_DEVICE_SI91X)
#include "sl_si91x_psa_wrap.h"
#endif
#include "sl_si91x_ecdsa.h"
#include "sl_si91x_crypto.h"
#include "sl_status.h"
#include "sl_constants.h"
#include "sl_si91x_protocol_types.h"
#include "sl_si91x_driver.h"
#include <string.h>

/**
 * \brief  Validate that the curve and algorithm combination is supported by hardware
 *
 * \param[in]  attributes         The attributes of the key to use for the
 *                                operation.
 * \param[in]      alg            The signature algorithm to compute.

 *
 * \retval #PSA_SUCCESS Success.
 * \retval #PSA_ERROR_NOT_SUPPORTED
 *         \p alg is not supported.
 * \retval #PSA_ERROR_INVALID_ARGUMENT
 */

static psa_status_t check_curve_availability(const psa_key_attributes_t *attributes, psa_algorithm_t alg)
{
  psa_key_type_t key_type = psa_get_key_type(attributes);

  if (PSA_ALG_IS_RSA_PSS(alg) || PSA_ALG_IS_RSA_PKCS1V15_SIGN(alg)) {
    // We shouldn't have a RSA-type alg for a ECC key.
    return PSA_ERROR_INVALID_ARGUMENT;
  }

#if defined(SLI_PSA_DRIVER_FEATURE_ECDSA)
  psa_ecc_family_t curvetype = PSA_KEY_TYPE_ECC_GET_FAMILY(key_type);

  if (curvetype == PSA_ECC_FAMILY_SECP_R1) {
    switch (psa_get_key_bits(attributes)) {
#if defined(SLI_PSA_DRIVER_FEATURE_P256R1)
      case 224: // Intentional
#endif
#if defined(SLI_PSA_DRIVER_FEATURE_P256R1)
      case 256: // Intentional
#endif
        // Only randomized ECDSA is supported on secp-r1 curves
        if (!PSA_ALG_IS_RANDOMIZED_ECDSA(alg)) {
          return PSA_ERROR_NOT_SUPPORTED;
        }
        break; // This break catches all the supported curves
      default:
        return PSA_ERROR_NOT_SUPPORTED;
    }
  } else if (curvetype == PSA_ECC_FAMILY_SECP_K1) {
    // Only randomized ECDSA is supported on secp-k1 curves
    if (!PSA_ALG_IS_RANDOMIZED_ECDSA(alg)) {
      return PSA_ERROR_NOT_SUPPORTED;
    }
    return PSA_ERROR_NOT_SUPPORTED;
  } else
#endif // SLI_PSA_DRIVER_FEATURE_ECDSA
  {
#if !defined(SLI_PSA_DRIVER_FEATURE_ECDSA)
    UNUSED_VARIABLE(key_type);
#endif
    return PSA_ERROR_NOT_SUPPORTED;
  }

  return PSA_SUCCESS;
}

/*****************************************************************************
* Generate a signature with input message using ECC private key.
*****************************************************************************/

psa_status_t sli_si91x_crypto_sign_message(const psa_key_attributes_t *attributes,
                                           const uint8_t *key_buffer,
                                           size_t key_buffer_size,
                                           psa_algorithm_t alg,
                                           const uint8_t *input,
                                           size_t input_length,
                                           uint8_t *signature,
                                           size_t signature_size,
                                           size_t *signature_length)
{
  psa_status_t status = PSA_ERROR_GENERIC_ERROR;

  // Input check
  if (attributes == NULL || key_buffer == NULL || key_buffer_size == 0 || (input == NULL && input_length > 0)
      || signature == NULL || signature_size == 0 || signature_length == NULL) {
    return PSA_ERROR_INVALID_ARGUMENT;
  }

  // Check the requested algorithm is supported
  if (PSA_KEY_TYPE_IS_ECC_KEY_PAIR(psa_get_key_type(attributes))) {
    status = check_curve_availability(attributes, alg);
    if (status != PSA_SUCCESS) {
      return status;
    }
  } else {
    // Not able to sign using non-ECC keys, or using public keys
    return PSA_ERROR_INVALID_ARGUMENT;
  }

  if (PSA_ALG_SIGN_GET_HASH(alg) != PSA_ALG_SHA_256 && PSA_ALG_SIGN_GET_HASH(alg) != PSA_ALG_SHA_384
      && PSA_ALG_SIGN_GET_HASH(alg) != PSA_ALG_SHA_512) {
    return PSA_ERROR_NOT_SUPPORTED;
  }

  // Validate that the output buffer can contain the full signature.
  if (signature_size < PSA_ECDSA_SIGNATURE_SIZE(psa_get_key_bits(attributes))) {
    return PSA_ERROR_BUFFER_TOO_SMALL;
  }

  if (key_buffer_size != SL_SI91X_ECDSA_PRIV_KEY_SIZE_224 && key_buffer_size != SL_SI91X_ECDSA_PRIV_KEY_SIZE_256) {
    return PSA_ERROR_NOT_SUPPORTED;
  }

  *signature_length = 0;

#if defined(SLI_PSA_DRIVER_FEATURE_ECDSA)
  sl_status_t si91x_status;

  sl_si91x_ecdsa_config_t config = { 0 };
  config.ecdsa_operation         = SL_SI91X_ECDSA_GENERATE_SIGN;

  if (key_buffer_size == SL_SI91X_ECDSA_PRIV_KEY_SIZE_224)
    config.curve_id = SL_SI91X_ECC_SECP224R1;
  else if (key_buffer_size == SL_SI91X_ECDSA_PRIV_KEY_SIZE_256)
    config.curve_id = SL_SI91X_ECC_SECP256R1;

  if (PSA_ALG_SIGN_GET_HASH(alg) == PSA_ALG_SHA_256)
    config.sha_mode = SL_SI91X_ECDSA_SHA_256;
  else if (PSA_ALG_SIGN_GET_HASH(alg) == PSA_ALG_SHA_384)
    config.sha_mode = SL_SI91X_ECDSA_SHA_384;
  else if (PSA_ALG_SIGN_GET_HASH(alg) == PSA_ALG_SHA_512)
    config.sha_mode = SL_SI91X_ECDSA_SHA_512;

  config.msg                = input;
  config.msg_length         = input_length;
  config.private_key        = key_buffer;
  config.private_key_length = key_buffer_size;
  config.public_key         = NULL;
  config.public_key_length  = 0;
  config.signature_length   = 0;

#if defined(SLI_SI917B0)
  /* Fetch key type from attributes */
  psa_key_location_t location = PSA_KEY_LIFETIME_GET_LOCATION(psa_get_key_lifetime(attributes));
  if (location == 0) {
    config.key_config.b0.key_type = SL_SI91X_TRANSPARENT_KEY;
  } else {
    config.key_config.b0.key_type = SL_SI91X_WRAPPED_KEY;
    config.private_key_length     = SL_SI91X_ECDSA_PRIV_KEY_SIZE_256; // wrapped key is of fixed output size 32
  }
  config.key_config.b0.key_slot     = 0;
  config.key_config.b0.key_size     = 0;
  config.key_config.b0.reserved     = 0;
  config.key_config.b0.wrap_iv_mode = SL_SI91X_WRAP_IV_CBC_MODE;
  if (config.key_config.b0.wrap_iv_mode == SL_SI91X_WRAP_IV_CBC_MODE) {
    memcpy(config.key_config.b0.wrap_iv, WRAP_IV, SL_SI91X_IV_SIZE);
  }
#else
  config.key_config.a0.key_length = key_buffer_size;
#endif

  uint8_t *sign_buf = (uint8_t *)malloc(SL_SI91X_ECDSA_SIGN_MAX_SIZE * sizeof(uint8_t));

  /* Calling sl_si91x_ecdsa() for signature generation */
  si91x_status = sl_si91x_ecdsa(&config, sign_buf);

  if (sign_buf[0] != 0x30)
    status = SL_STATUS_INVALID_SIGNATURE;

  /* Convert DER format signature to compact format */
  uint8_t r_size = sign_buf[3];
  uint8_t s_size = sign_buf[5 + r_size];

  uint8_t r_index   = 4;
  uint8_t s_index   = 6 + r_size;
  uint8_t r_bit     = 0;
  uint8_t s_bit     = 0;
  uint8_t r_leading = 0;
  uint8_t s_leading = 0;

  if (key_buffer_size == SL_SI91X_ECDSA_PRIV_KEY_SIZE_224) {
    if (r_size == SL_SI91X_ECDSA_SIGN_HALF_SIZE_P224R1 + 1)
      r_bit = 1;
    if (s_size == SL_SI91X_ECDSA_SIGN_HALF_SIZE_P224R1 + 1)
      s_bit = 1;
    if (r_size == SL_SI91X_ECDSA_SIGN_HALF_SIZE_P224R1 - 1)
      r_leading = 1;
    if (s_size == SL_SI91X_ECDSA_SIGN_HALF_SIZE_P224R1 - 1)
      s_leading = 1;

    signature[0]                                    = 0x00;
    signature[SL_SI91X_ECDSA_SIGN_HALF_SIZE_P224R1] = 0x00;
    memcpy(&signature[0 + r_leading], &sign_buf[r_index + r_bit], SL_SI91X_ECDSA_SIGN_HALF_SIZE_P224R1 - r_leading);
    memcpy(&signature[SL_SI91X_ECDSA_SIGN_HALF_SIZE_P224R1 + s_leading],
           &sign_buf[s_index + s_bit],
           SL_SI91X_ECDSA_SIGN_HALF_SIZE_P224R1 - s_leading);

  } else if (key_buffer_size == SL_SI91X_ECDSA_PRIV_KEY_SIZE_256) {
    if (r_size == SL_SI91X_ECDSA_SIGN_HALF_SIZE_P256R1 + 1)
      r_bit = 1;
    if (s_size == SL_SI91X_ECDSA_SIGN_HALF_SIZE_P256R1 + 1)
      s_bit = 1;
    if (r_size == SL_SI91X_ECDSA_SIGN_HALF_SIZE_P256R1 - 1)
      r_leading = 1;
    if (s_size == SL_SI91X_ECDSA_SIGN_HALF_SIZE_P256R1 - 1)
      s_leading = 1;

    signature[0]                                    = 0x00;
    signature[SL_SI91X_ECDSA_SIGN_HALF_SIZE_P256R1] = 0x00;
    memcpy(&signature[0 + r_leading], &sign_buf[r_index + r_bit], SL_SI91X_ECDSA_SIGN_HALF_SIZE_P256R1 - r_leading);
    memcpy(&signature[SL_SI91X_ECDSA_SIGN_HALF_SIZE_P256R1 + s_leading],
           &sign_buf[s_index + s_bit],
           SL_SI91X_ECDSA_SIGN_HALF_SIZE_P256R1 - s_leading);
  }

  /* gets the si91x error codes and returns its equivalent psa_status codes */
  status = convert_si91x_error_code_to_psa_status(si91x_status);

  free(sign_buf);
  *signature_length = signature_size;

  return status;

#else //SLI_PSA_DRIVER_FEATURE_ECDSA

  (void)attributes;
  (void)key_buffer;
  (void)key_buffer_size;
  (void)alg;
  (void)input;
  (void)input_length;
  (void)signature;
  (void)signature_size;
  (void)signature_length;

  return (PSA_ERROR_NOT_SUPPORTED);

#endif //SLI_PSA_DRIVER_FEATURE_ECDSA
};

/*****************************************************************************
* Verify a signature with input message using ECC public key.
*****************************************************************************/

psa_status_t sli_si91x_crypto_verify_message(const psa_key_attributes_t *attributes,
                                             const uint8_t *key_buffer,
                                             size_t key_buffer_size,
                                             psa_algorithm_t alg,
                                             const uint8_t *input,
                                             size_t input_length,
                                             const uint8_t *signature,
                                             size_t signature_length)
{

  psa_status_t status = PSA_ERROR_GENERIC_ERROR;

  // Input check.
  if (attributes == NULL || key_buffer == NULL || key_buffer_size == 0 || (input == NULL && input_length > 0)
      || signature == NULL) {
    return PSA_ERROR_INVALID_ARGUMENT;
  }

  // Verify can happen with only public key, private key verification not supported
  if (PSA_KEY_TYPE_IS_ECC_PUBLIC_KEY(psa_get_key_type(attributes))) {
    // Check the requested algorithm is supported and matches the key type
    status = check_curve_availability(attributes, alg);
    if (status != PSA_SUCCESS) {
      return status;
    }
  } else {
    return PSA_ERROR_NOT_SUPPORTED;
  }

  if (PSA_KEY_TYPE_IS_ECC_KEY_PAIR(psa_get_key_type(attributes))) {
    return PSA_ERROR_NOT_SUPPORTED;
  }

  if (signature_length == 0) {
    return PSA_ERROR_INVALID_SIGNATURE;
  }

  if (key_buffer_size != SL_SI91X_ECDSA_PUB_KEY_SIZE_224 && key_buffer_size != SL_SI91X_ECDSA_PUB_KEY_SIZE_256) {
    return PSA_ERROR_NOT_SUPPORTED;
  }

  if (key_buffer[0] != 0x04) /* To check uncompressed public keys format */
    return PSA_ERROR_INVALID_ARGUMENT;

  // Validate that the signature input is of the expected length.
  if (signature_length < PSA_ECDSA_SIGNATURE_SIZE(psa_get_key_bits(attributes))) {
    return PSA_ERROR_INVALID_SIGNATURE;
  }

#if defined(SLI_PSA_DRIVER_FEATURE_ECDSA)
  sl_status_t si91x_status;

  sl_si91x_ecdsa_config_t config = { 0 };
  config.ecdsa_operation         = SL_SI91X_ECDSA_VERIFY_SIGN;

  if (key_buffer_size == SL_SI91X_ECDSA_PUB_KEY_SIZE_224)
    config.curve_id = SL_SI91X_ECC_SECP224R1;
  else if (key_buffer_size == SL_SI91X_ECDSA_PUB_KEY_SIZE_256)
    config.curve_id = SL_SI91X_ECC_SECP256R1;

  if (PSA_ALG_SIGN_GET_HASH(alg) == PSA_ALG_SHA_256)
    config.sha_mode = SL_SI91X_ECDSA_SHA_256;
  else if (PSA_ALG_SIGN_GET_HASH(alg) == PSA_ALG_SHA_384)
    config.sha_mode = SL_SI91X_ECDSA_SHA_384;
  else if (PSA_ALG_SIGN_GET_HASH(alg) == PSA_ALG_SHA_512)
    config.sha_mode = SL_SI91X_ECDSA_SHA_512;

  config.msg                = input;
  config.msg_length         = input_length;
  config.private_key        = NULL;
  config.private_key_length = 0;
  config.public_key         = key_buffer;
  config.public_key_length  = key_buffer_size;

#if defined(SLI_SI917B0)
  /* Fetch key type from attributes */
  psa_key_location_t location = PSA_KEY_LIFETIME_GET_LOCATION(psa_get_key_lifetime(attributes));
  if (location == 0) {
    config.key_config.b0.key_type = SL_SI91X_TRANSPARENT_KEY;
  } else {
    config.key_config.b0.key_type = SL_SI91X_WRAPPED_KEY;
  }
  config.key_config.b0.key_slot     = 0;
  config.key_config.b0.key_size     = 0;
  config.key_config.b0.reserved     = 0;
  config.key_config.b0.wrap_iv_mode = SL_SI91X_WRAP_IV_CBC_MODE;
  if (config.key_config.b0.wrap_iv_mode == SL_SI91X_WRAP_IV_CBC_MODE) {
    memcpy(config.key_config.b0.wrap_iv, WRAP_IV, SL_SI91X_IV_SIZE);
  }
#else
  config.key_config.a0.key_length = key_buffer_size;
#endif

  uint8_t r_size    = 0;
  uint8_t s_size    = 0;
  uint8_t r_bit     = 0;
  uint8_t s_bit     = 0;
  uint8_t r_leading = 0;
  uint8_t s_leading = 0;

  /* Convert compact format signature to DER format */

  if (key_buffer_size == SL_SI91X_ECDSA_PUB_KEY_SIZE_224) {
    r_size = s_size = SL_SI91X_ECDSA_SIGN_HALF_SIZE_P224R1;
    if (signature[0] >= 0x80) {
      r_bit = 1;
      r_size++;
    }
    if (signature[SL_SI91X_ECDSA_SIGN_HALF_SIZE_P224R1] >= 0x80) {
      s_bit = 1;
      s_size++;
    }
    if (signature[0] == 0x00) {
      r_leading++;
      r_size--;
    }
    if (signature[SL_SI91X_ECDSA_SIGN_HALF_SIZE_P224R1] == 0x00) {
      s_leading++;
      s_size--;
    }
  }

  else if (key_buffer_size == SL_SI91X_ECDSA_PUB_KEY_SIZE_256) {
    r_size = s_size = SL_SI91X_ECDSA_SIGN_HALF_SIZE_P256R1;
    if (signature[0] >= 0x80) {
      r_bit = 1;
      r_size++;
    }
    if (signature[SL_SI91X_ECDSA_SIGN_HALF_SIZE_P256R1] >= 0x80) {
      s_bit = 1;
      s_size++;
    }
    if (signature[0] == 0x00) {
      r_leading++;
      r_size--;
    }
    if (signature[SL_SI91X_ECDSA_SIGN_HALF_SIZE_P256R1] == 0x00) {
      s_leading++;
      s_size--;
    }
  }

  uint8_t sign_size  = r_size + s_size + 4;
  uint8_t total_size = sign_size + 2;
  uint8_t s_index    = r_size + 6;
  uint8_t r_index    = 4;

  uint8_t *sign_buf = (uint8_t *)malloc(total_size * sizeof(uint8_t));

  sign_buf[0] = 0x30;
  sign_buf[1] = sign_size;

  sign_buf[2] = sign_buf[s_index - 2] = 0x02;
  sign_buf[3]                         = r_size;
  sign_buf[s_index - 1]               = s_size;

  sign_buf[r_index] = 0;
  sign_buf[s_index] = 0;

  if (key_buffer_size == SL_SI91X_ECDSA_PUB_KEY_SIZE_224) {
    memcpy(&sign_buf[r_index + r_bit], &signature[0 + r_leading], r_size - r_bit);
    memcpy(&sign_buf[s_index + s_bit], &signature[SL_SI91X_ECDSA_SIGN_HALF_SIZE_P224R1 + s_leading], s_size - s_bit);
  }

  else if (key_buffer_size == SL_SI91X_ECDSA_PUB_KEY_SIZE_256) {
    memcpy(&sign_buf[r_index + r_bit], &signature[0 + r_leading], r_size - r_bit);
    memcpy(&sign_buf[s_index + s_bit], &signature[SL_SI91X_ECDSA_SIGN_HALF_SIZE_P256R1 + s_leading], s_size - s_bit);
  }

  uint8_t *output = (uint8_t *)malloc(sizeof(uint8_t));

  config.signature        = sign_buf;
  config.signature_length = total_size;

  /* Calling sl_si91x_ecdsa() for signature verification */
  si91x_status = sl_si91x_ecdsa(&config, output);
  /* gets the si91x error codes and returns its equivalent psa_status codes */
  status = convert_si91x_error_code_to_psa_status(si91x_status);

  if (*output != 1 && status == 0) {
    return PSA_ERROR_INVALID_SIGNATURE;
  }

  free(sign_buf);
  free(output);
  return status;

#else //SLI_PSA_DRIVER_FEATURE_ECDSA

  (void)attributes;
  (void)key_buffer;
  (void)key_buffer_size;
  (void)alg;
  (void)input;
  (void)input_length;
  (void)signature;
  (void)signature_length;

  return PSA_ERROR_NOT_SUPPORTED;

#endif //SLI_PSA_DRIVER_FEATURE_ECDSA
};
