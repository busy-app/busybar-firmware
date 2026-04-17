/*******************************************************************************
 * @file  sl_si91x_psa_sha.h
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

#ifndef SL_SI91X_PSA_SHA_H
#define SL_SI91X_PSA_SHA_H

#include "psa/crypto.h"

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                          Public Function Declarations
// -----------------------------------------------------------------------------

/**
 * \brief Process an Hashing operation.
 *
 * \note The signature of this function is that of a PSA driver
 *       hash_compute entry point. This function behaves as an hash_compute
 *       entry point as defined in the PSA driver interface specification for
 *       transparent drivers.
 *
 * \param[in]  alg                The SHA algorithm to compute.
 * \param[in]  input              Input data.
 * \param[in]  input_length       Size of given input.
 * \param[out] hash               Output buffer for the hash value.
 * \param[in]  hash_size          Size of the ciphertext buffer in bytes. This
 *                                must be appropriate for the selected algorithm
 * \param[out] hash_length        On success, the size of the output in the
 *                                ciphertext buffer.
 *
 * \retval PSA_SUCCESS Success.
 * \retval PSA_ERROR_NOT_SUPPORTED
 *         \p alg is not supported.
 * \retval PSA_ERROR_INSUFFICIENT_MEMORY
 * \retval PSA_ERROR_BUFFER_TOO_SMALL
 *         ciphertext_size is too small.
 * \retval PSA_ERROR_CORRUPTION_DETECTED
 */
psa_status_t sli_si91x_crypto_hash_compute(psa_algorithm_t alg,
                                           const uint8_t *input,
                                           size_t input_length,
                                           uint8_t *hash,
                                           size_t hash_size,
                                           size_t *hash_length);

#endif /* SL_SI91X_PSA_SHA_H */
