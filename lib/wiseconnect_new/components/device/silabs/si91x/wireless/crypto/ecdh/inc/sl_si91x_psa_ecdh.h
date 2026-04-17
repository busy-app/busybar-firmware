/*******************************************************************************
 * @file  sl_si91x_psa_ecdh.h
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

#ifndef SL_SI91X_PSA_ECDH_H
#define SL_SI91X_PSA_ECDH_H

#include "psa/crypto.h"

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------
#define ECC_P256_LEN      32
#define ECC_P192_LEN      24
#define ECC_P256_BITS_LEN 256
#define ECC_P192_BITS_LEN 192
#define BUFFER_SIZE       32
// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
//                          Public Function Declarations
// -----------------------------------------------------------------------------

/** Generate a private key using random number generator.
 *
 * \note The signature of this function is that of a PSA driver
 *       generate_key entry point. This function behaves as a
 *       generate_key entry point as defined in the PSA driver
 *       interface specification for transparent drivers.
 *
 * \param[in]  attributes        The attributes of the key to use for the
 *                               operation.
 * \param[in,out]  key_buffer    The buffer containing the key context.
 * \param[in]  key_buffer_size   Size of the \p key_buffer buffer in bytes.
 * \param[out] key_buffer_length On success, the number of bytes that make up
 *                               the returned output. Initialized to zero
 *                               by the core.
 *
 * \retval PSA_SUCCESS
 * \retval PSA_ERROR_NOT_SUPPORTED
 * \retval PSA_ERROR_INSUFFICIENT_MEMORY
 * \retval PSA_ERROR_CORRUPTION_DETECTED
 */

psa_status_t sli_si91x_psa_generate_key_ecdh(const psa_key_attributes_t *attributes,
                                             const uint8_t *key_buffer,
                                             size_t key_buffer_size,
                                             size_t *key_buffer_length);

/** Generate a public key using ecdh multiplication.
 *
 * \note The signature of this function is that of a PSA driver
 *       export_public_key entry point. This function behaves as a
 *       export_public_key entry point as defined in the PSA driver
 *       interface specification for transparent drivers.
 *
 * \param[in]  attributes       The attributes of the key to use for the
 *                              operation.
 * \param[in]  key_buffer       The buffer containing the key context.
 * \param[in]  key_buffer_size  Size of the \p key_buffer buffer in bytes.
 * \param[in,out] data          Buffer where the output is to be written.
 * \param[in]  data_size        Size of the \p data buffer in bytes.
 * \param[out] data_length      On success, the number of bytes that make up
 *                              the returned output. Initialized to zero
 *                              by the core.
 *
 * \retval PSA_SUCCESS
 * \retval PSA_ERROR_NOT_SUPPORTED
 * \retval PSA_ERROR_INSUFFICIENT_MEMORY
 * \retval PSA_ERROR_CORRUPTION_DETECTED
 */

psa_status_t sli_si91x_psa_export_public_key_ecdh(const psa_key_attributes_t *attributes,
                                                  const uint8_t *key_buffer,
                                                  size_t key_buffer_size,
                                                  uint8_t *data,
                                                  size_t data_size,
                                                  size_t *data_length);

/** Generate a shared secret using private key and peer's public key.
 *
 * \note The signature of this function is that of a PSA driver
 *       key_agreement entry point. This function behaves as a
 *       key_agreement entry point as defined in the PSA driver
 *       interface specification for transparent drivers.
 *
 * \param[in] alg                       The ECDH algorithm to compute
 *                                      (\c PSA_ALG_XXX value such that
 *                                      PSA_ALG_ECDH(\p alg) is true).
 * \param[in]  attributes               The attributes of the key to use for the
 *                                      operation.
 * \param[in]  private_key              The buffer containing the private key context.
 * \param[in]  private_key_len          Size of the \p private_key buffer in bytes.
 * \param[in]  peer_key                 The buffer containing the peer public key context.
 * \param[in]  peer_key_length          Size of the \p peer_key buffer in bytes.
 * \param[in,out] shared_secret         Buffer where the output is to be written.
 * \param[in]  shared_secret_size       Size of the \p shared_secret buffer in bytes.
 * \param[out] shared_secret_length     On success, the number of bytes that make up
 *                                      the returned output. Initialized to zero
 *                                      by the core.
 *
 * \retval PSA_SUCCESS
 * \retval PSA_ERROR_NOT_SUPPORTED
 * \retval PSA_ERROR_INSUFFICIENT_MEMORY
 * \retval PSA_ERROR_CORRUPTION_DETECTED
 * \retval PSA_ERROR_BUFFER_TOO_SMALL
 *         The size of the \p shared_secret buffer is small.
 */

psa_status_t sli_si91x_psa_ecdh_key_agreement(psa_algorithm_t alg,
                                              const psa_key_attributes_t *attributes,
                                              const uint8_t *private_key,
                                              size_t private_key_len,
                                              const uint8_t *peer_key,
                                              size_t peer_key_length,
                                              uint8_t *shared_secret,
                                              size_t shared_secret_size,
                                              size_t *shared_secret_length);
#endif /* SLI_SI91X_PSA_ECDH_H */
