/*******************************************************************************
 * @file  sl_si91x_psa_ecdh.c
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
#include "sl_si91x_ecdh.h"
#include "sl_si91x_trng.h"
#include "sl_si91x_psa_ecdh.h"
#include "sli_si91x_crypto_driver_functions.h"
#include <stdio.h>

psa_status_t sli_si91x_psa_generate_key_ecdh(const psa_key_attributes_t *attributes,
                                             const uint8_t *key_buffer,
                                             size_t key_buffer_size,
                                             size_t *key_buffer_length)
{

#if defined(SLI_PSA_DRIVER_FEATURE_ECDH) || defined(SLI_PSA_DRIVER_FEATURE_ECDSA)

  // Argument check.
  if (attributes == NULL || key_buffer == NULL || key_buffer_size == 0 || key_buffer_length == NULL) {
    return PSA_ERROR_INVALID_ARGUMENT;
  }

  psa_key_type_t key_type     = psa_get_key_type(attributes);
  psa_ecc_family_t curve_type = PSA_KEY_TYPE_ECC_GET_FAMILY(key_type);
  size_t key_bits             = psa_get_key_bits(attributes);
  psa_status_t status         = PSA_ERROR_GENERIC_ERROR;
  *key_buffer_length          = 0;

  // Check key type. PSA Crypto defines generate_key to be an invalid call with a key type
  // of public key.
  if (!PSA_KEY_TYPE_IS_ECC_KEY_PAIR(key_type)) {
    return PSA_ERROR_NOT_SUPPORTED;
  }

  // We currently only support R1
  if (curve_type != PSA_ECC_FAMILY_SECP_R1) {
    return PSA_ERROR_NOT_SUPPORTED;
  }

  switch (key_bits) {

#if defined(SLI_PSA_DRIVER_FEATURE_P192R1)
    case ECC_P192_BITS_LEN:
      if (key_buffer_size != ECC_P192_LEN)
        return PSA_ERROR_INVALID_ARGUMENT;
      break;
#endif //SLI_PSA_DRIVER_FEATURE_P192R1

#if defined(SLI_PSA_DRIVER_FEATURE_P256R1)
    case ECC_P256_BITS_LEN:
      if (key_buffer_size != ECC_P256_LEN)
        return PSA_ERROR_INVALID_ARGUMENT;
      break;
#endif //SLI_PSA_DRIVER_FEATURE_P256R1

    default:
      return PSA_ERROR_NOT_SUPPORTED;
  }

  status =
    convert_si91x_error_code_to_psa_status(sl_si91x_trng_get_random_num((uint32_t *)key_buffer, key_buffer_size));

  if (status != PSA_SUCCESS) {
    return status;
  }

  *key_buffer_length = key_buffer_size;
  return status;

#else // SLI_PSA_DRIVER_FEATURE_ECDH || SLI_PSA_DRIVER_FEATURE_ECDSA

  (void)attributes;
  (void)key_buffer;
  (void)key_buffer_size;
  (void)key_buffer_length;
  return (PSA_ERROR_NOT_SUPPORTED);

#endif // SLI_PSA_DRIVER_FEATURE_ECDH || SLI_PSA_DRIVER_FEATURE_ECDSA
}

psa_status_t sli_si91x_psa_export_public_key_ecdh(const psa_key_attributes_t *attributes,
                                                  const uint8_t *key_buffer,
                                                  size_t key_buffer_size,
                                                  uint8_t *data,
                                                  size_t data_size,
                                                  size_t *data_length)
{

#if defined(SLI_PSA_DRIVER_FEATURE_ECDH) || defined(SLI_PSA_DRIVER_FEATURE_ECDSA)

  // Argument check.
  if (attributes == NULL || key_buffer == NULL || key_buffer_size == 0 || data == NULL || data_size == 0
      || data_length == NULL) {
    return PSA_ERROR_INVALID_ARGUMENT;
  }

  size_t key_bits             = psa_get_key_bits(attributes);
  psa_status_t status         = PSA_ERROR_GENERIC_ERROR;
  psa_key_type_t key_type     = psa_get_key_type(attributes);
  psa_ecc_family_t curve_type = PSA_KEY_TYPE_ECC_GET_FAMILY(key_type);

  // If the key is stored transparently and is already a public key,
  // let the core handle it.
  if (PSA_KEY_TYPE_IS_ECC_PUBLIC_KEY(key_type)) {
    return PSA_ERROR_NOT_SUPPORTED;
  }

  if (!PSA_KEY_TYPE_IS_ECC(key_type)) {
    return PSA_ERROR_NOT_SUPPORTED;
  }

  // We currently only support R1
  if (curve_type != PSA_ECC_FAMILY_SECP_R1) {
    return PSA_ERROR_NOT_SUPPORTED;
  }

  if (key_buffer_size < PSA_BITS_TO_BYTES(key_bits)) {
    return PSA_ERROR_INVALID_ARGUMENT;
  }

  // Check sufficient output buffer size.
  if (data_size < PSA_BITS_TO_BYTES(key_bits) * 2 + 1) {
    return PSA_ERROR_BUFFER_TOO_SMALL;
  }

  switch (key_bits) {
#if defined(SLI_PSA_DRIVER_FEATURE_P192R1)
    case ECC_P192_BITS_LEN:

      uint8_t ecc_p192_gx[ECC_P192_LEN] = { 0x18, 0x8D, 0xA8, 0x0E, 0xB0, 0x30, 0x90, 0xF6, 0x7C, 0xBF, 0x20, 0xEB,
                                            0x43, 0xA1, 0x88, 0x00, 0xF4, 0xFF, 0x0A, 0xFD, 0x82, 0xFF, 0x10, 0x12 };
      uint8_t ecc_p192_gy[ECC_P192_LEN] = { 0x07, 0x19, 0x2B, 0x95, 0xFF, 0xC8, 0xDA, 0x78, 0x63, 0x10, 0x11, 0xED,
                                            0x6B, 0x24, 0xCD, 0xD5, 0x73, 0xF9, 0x77, 0xA1, 0x1E, 0x79, 0x48, 0x11 };
      uint8_t ecc_p192_z[ECC_P192_LEN]  = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01 };

      //! Buffers to store responses
      uint8_t ecc_p192_rx[ECC_P192_LEN] = { 0 };
      uint8_t ecc_p192_ry[ECC_P192_LEN] = { 0 };
      uint8_t ecc_p192_rz[ECC_P192_LEN] = { 0 };

      uint8_t ecc_p192_d[ECC_P192_LEN] = { 0 };

      memcpy(ecc_p192_d, key_buffer, key_buffer_size);

      //!ECDH point multiplication
      status = convert_si91x_error_code_to_psa_status(sl_si91x_ecdh_point_multiplication(SL_SI91X_ECDH_192,
                                                                                         ecc_p192_d,
                                                                                         ecc_p192_gx,
                                                                                         ecc_p192_gy,
                                                                                         ecc_p192_z,
                                                                                         2,
                                                                                         ecc_p192_rx,
                                                                                         ecc_p192_ry,
                                                                                         ecc_p192_rz,
                                                                                         1));
      if (status != PSA_SUCCESS) {
        return status;
      }

      *data_length = PSA_BITS_TO_BYTES(key_bits) * 2 + 1;
      data[0]      = 0x04;
      memcpy(data + 1, ecc_p192_rx, ECC_P192_LEN);
      memcpy(data + 1 + ECC_P192_LEN, ecc_p192_ry, ECC_P192_LEN);

      break;
#endif //SLI_PSA_DRIVER_FEATURE_P192R1

#if defined(SLI_PSA_DRIVER_FEATURE_P256R1)
    case ECC_P256_BITS_LEN:

      uint8_t ecc_p256_gx[ECC_P256_LEN] = { 0x6B, 0x17, 0xD1, 0xF2, 0xE1, 0x2C, 0x42, 0x47, 0xF8, 0xBC, 0xE6,
                                            0xE5, 0x63, 0xA4, 0x40, 0xF2, 0x77, 0x03, 0x7D, 0x81, 0x2D, 0xEB,
                                            0x33, 0xA0, 0xF4, 0xA1, 0x39, 0x45, 0xD8, 0x98, 0xC2, 0x96 };
      uint8_t ecc_p256_gy[ECC_P256_LEN] = { 0x4F, 0xE3, 0x42, 0xE2, 0xFE, 0x1A, 0x7F, 0x9B, 0x8E, 0xE7, 0xEB,
                                            0x4A, 0x7C, 0x0F, 0x9E, 0x16, 0x2B, 0xCE, 0x33, 0x57, 0x6B, 0x31,
                                            0x5E, 0xCE, 0xCB, 0xB6, 0x40, 0x68, 0x37, 0xBF, 0x51, 0xF5 };
      uint8_t ecc_p256_z[ECC_P256_LEN]  = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1 };

      //! Buffers to store responses
      uint8_t ecc_p256_rx[ECC_P256_LEN] = { 0 };
      uint8_t ecc_p256_ry[ECC_P256_LEN] = { 0 };
      uint8_t ecc_p256_rz[ECC_P256_LEN] = { 0 };

      uint8_t ecc_p256_d[ECC_P256_LEN] = { 0 };

      memcpy(ecc_p256_d, key_buffer, key_buffer_size);
      //!ECDH point multiplication
      status = convert_si91x_error_code_to_psa_status(sl_si91x_ecdh_point_multiplication(SL_SI91X_ECDH_256,
                                                                                         ecc_p256_d,
                                                                                         ecc_p256_gx,
                                                                                         ecc_p256_gy,
                                                                                         ecc_p256_z,
                                                                                         2,
                                                                                         ecc_p256_rx,
                                                                                         ecc_p256_ry,
                                                                                         ecc_p256_rz,
                                                                                         1));
      if (status != PSA_SUCCESS) {
        return status;
      }

      *data_length = PSA_BITS_TO_BYTES(key_bits) * 2 + 1;
      data[0]      = 0x04;

      memcpy(data + 1, ecc_p256_rx, ECC_P256_LEN);
      memcpy(data + 1 + ECC_P256_LEN, ecc_p256_ry, ECC_P256_LEN);
      break;
#endif //SLI_PSA_DRIVER_FEATURE_P256R1

    default:
      return PSA_ERROR_NOT_SUPPORTED;
  }

  return status;

#else //SLI_PSA_DRIVER_FEATURE_ECDH || SLI_PSA_DRIVER_FEATURE_ECDSA

  (void)attributes;
  (void)key_buffer;
  (void)key_buffer_size;
  (void)data;
  (void)data_size;
  (void)data_length;
  return (PSA_ERROR_NOT_SUPPORTED);

#endif //SLI_PSA_DRIVER_FEATURE_ECDH || SLI_PSA_DRIVER_FEATURE_ECDSA
}

psa_status_t sli_si91x_psa_ecdh_key_agreement(psa_algorithm_t alg,
                                              const psa_key_attributes_t *attributes,
                                              const uint8_t *private_key,
                                              size_t private_key_len,
                                              const uint8_t *peer_key,
                                              size_t peer_key_length,
                                              uint8_t *shared_secret,
                                              size_t shared_secret_size,
                                              size_t *shared_secret_length)
{

#if defined(SLI_PSA_DRIVER_FEATURE_ECDH)

  // Argument check.
  if (attributes == NULL || private_key == NULL || peer_key == NULL || shared_secret == NULL
      || shared_secret_length == NULL) {
    return PSA_ERROR_INVALID_ARGUMENT;
  }

  (void)alg;
  (void)peer_key_length;
  psa_key_type_t key_type = psa_get_key_type(attributes);
  size_t key_bits         = psa_get_key_bits(attributes);
  psa_status_t status     = PSA_ERROR_GENERIC_ERROR;
  *shared_secret_length   = 0;

  // Only accelerate ECDH.
  if (!PSA_ALG_IS_ECDH(alg)) {
    return PSA_ERROR_NOT_SUPPORTED;
  }

  // Check that key_buffer contains private key.
  if (PSA_KEY_TYPE_IS_PUBLIC_KEY(key_type)) {
    return PSA_ERROR_INVALID_ARGUMENT;
  }

  if (key_type != PSA_KEY_TYPE_ECC_KEY_PAIR(PSA_ECC_FAMILY_SECP_R1)) {
    return PSA_ERROR_NOT_SUPPORTED;
  }

  // Check input-keys sizes.
  if (private_key_len < PSA_BITS_TO_BYTES(key_bits) || peer_key_length < PSA_BITS_TO_BYTES(key_bits) * 2 + 1) {
    return PSA_ERROR_INVALID_ARGUMENT;
  }

  // Check sufficient output buffer size.
  if (shared_secret_size < PSA_BITS_TO_BYTES(key_bits)) {
    return PSA_ERROR_BUFFER_TOO_SMALL;
  }

  // Check peer_key is a public key of correct format.
  if (peer_key[0] != 0x04) {
    return PSA_ERROR_INVALID_ARGUMENT;
  }

  switch (key_bits) {
    case ECC_P192_BITS_LEN:

      if (private_key_len > ECC_P192_LEN)
        return PSA_ERROR_INVALID_ARGUMENT;

      uint8_t ecc_p192_z[ECC_P192_LEN] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                           0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01 };

      //! Buffers to store responses
      uint8_t ecc_p192_rx[ECC_P192_LEN] = { 0 };
      uint8_t ecc_p192_ry[ECC_P192_LEN] = { 0 };
      uint8_t ecc_p192_rz[ECC_P192_LEN] = { 0 };

      uint8_t ecc_p192_d[ECC_P192_LEN]  = { 0 };
      uint8_t ecc_p192_px[ECC_P192_LEN] = { 0 };
      uint8_t ecc_p192_py[ECC_P192_LEN] = { 0 };

      memcpy(ecc_p192_d, private_key, private_key_len);

      memcpy(ecc_p192_px, peer_key + 1, ECC_P192_LEN);
      memcpy(ecc_p192_py, peer_key + 1 + ECC_P192_LEN, ECC_P192_LEN);

      //!ECDH point multiplication
      status = convert_si91x_error_code_to_psa_status(sl_si91x_ecdh_point_multiplication(SL_SI91X_ECDH_192,
                                                                                         ecc_p192_d,
                                                                                         ecc_p192_px,
                                                                                         ecc_p192_py,
                                                                                         ecc_p192_z,
                                                                                         2,
                                                                                         ecc_p192_rx,
                                                                                         ecc_p192_ry,
                                                                                         ecc_p192_rz,
                                                                                         1));

      if (status != PSA_SUCCESS) {
        return status;
      }

      memcpy(shared_secret, ecc_p192_rx, ECC_P192_LEN);
      *shared_secret_length = ECC_P192_LEN;

      break;

    case ECC_P256_BITS_LEN:

      if (private_key_len > ECC_P256_LEN)
        return PSA_ERROR_INVALID_ARGUMENT;

      uint8_t ecc_p256_z[ECC_P256_LEN] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                           0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                           0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1 };

      //! Buffers to store responses
      uint8_t ecc_p256_rx[ECC_P256_LEN] = { 0 };
      uint8_t ecc_p256_ry[ECC_P256_LEN] = { 0 };
      uint8_t ecc_p256_rz[ECC_P256_LEN] = { 0 };

      uint8_t ecc_p256_d[ECC_P256_LEN]  = { 0 };
      uint8_t ecc_p256_px[ECC_P256_LEN] = { 0 };
      uint8_t ecc_p256_py[ECC_P256_LEN] = { 0 };

      memcpy(ecc_p256_d, private_key, private_key_len);

      memcpy(ecc_p256_px, peer_key + 1, ECC_P256_LEN);
      memcpy(ecc_p256_py, peer_key + 1 + ECC_P256_LEN, ECC_P256_LEN);

      //!ECDH point multiplication
      status = convert_si91x_error_code_to_psa_status(sl_si91x_ecdh_point_multiplication(SL_SI91X_ECDH_256,
                                                                                         ecc_p256_d,
                                                                                         ecc_p256_px,
                                                                                         ecc_p256_py,
                                                                                         ecc_p256_z,
                                                                                         2,
                                                                                         ecc_p256_rx,
                                                                                         ecc_p256_ry,
                                                                                         ecc_p256_rz,
                                                                                         1));

      if (status != PSA_SUCCESS) {
        return status;
      }
      memcpy(shared_secret, ecc_p256_rx, ECC_P256_LEN);
      *shared_secret_length = ECC_P256_LEN;
      break;

    default:
      return PSA_ERROR_NOT_SUPPORTED;
  }

  return status;

#else //SLI_PSA_DRIVER_FEATURE_ECDH

  (void)alg;
  (void)attributes;
  (void)private_key;
  (void)private_key_len;
  (void)peer_key;
  (void)peer_key_length;
  (void)shared_secret;
  (void)shared_secret_size;
  (void)shared_secret_length;
  return (PSA_ERROR_NOT_SUPPORTED);

#endif //SLI_PSA_DRIVER_FEATURE_ECDH
}
