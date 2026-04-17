/*******************************************************************************
 * @file  sl_si91x_psa_sha.c
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
#include "sl_si91x_sha.h"
#include "sl_si91x_psa_sha.h"
#include "sli_si91x_crypto_driver_functions.h"

psa_status_t sli_si91x_crypto_hash_compute(psa_algorithm_t alg,
                                           const uint8_t *input,
                                           size_t input_length,
                                           uint8_t *hash,
                                           size_t hash_size,
                                           size_t *hash_length)
{
  psa_status_t status = PSA_ERROR_GENERIC_ERROR;

  uint8_t sha_algo;

#if defined(PSA_WANT_ALG_SHA_1) || defined(PSA_WANT_ALG_SHA_224) || defined(PSA_WANT_ALG_SHA_256) \
  || defined(PSA_WANT_ALG_SHA_384) || defined(PSA_WANT_ALG_SHA_512)

  if (((input == NULL) && (input_length > 0)) || ((hash == NULL) && (hash_size > 0))
      || ((hash_length == NULL) && (hash_size > 0))) {
    return PSA_ERROR_INVALID_ARGUMENT;
  }

  switch (alg) {
#if defined(PSA_WANT_ALG_SHA_1)
    case PSA_ALG_SHA_1:
      sha_algo     = SL_SI91X_SHA_1;
      *hash_length = SL_SI91X_SHA_1_DIGEST_LEN;
      break;
#endif // PSA_WANT_ALG_SHA_1
#if defined(PSA_WANT_ALG_SHA_224)
    case PSA_ALG_SHA_224:
      sha_algo     = SL_SI91X_SHA_224;
      *hash_length = SL_SI91X_SHA_224_DIGEST_LEN;
      break;
#endif // PSA_WANT_ALG_SHA_224
#if defined(PSA_WANT_ALG_SHA_256)
    case PSA_ALG_SHA_256:
      sha_algo     = SL_SI91X_SHA_256;
      *hash_length = SL_SI91X_SHA_256_DIGEST_LEN;
      break;
#endif // PSA_WANT_ALG_SHA_256
#if defined(PSA_WANT_ALG_SHA_384)
    case PSA_ALG_SHA_384:
      sha_algo     = SL_SI91X_SHA_384;
      *hash_length = SL_SI91X_SHA_384_DIGEST_LEN;
      break;
#endif // PSA_WANT_ALG_SHA_384
#if defined(PSA_WANT_ALG_SHA_512)
    case PSA_ALG_SHA_512:
      sha_algo     = SL_SI91X_SHA_512;
      *hash_length = SL_SI91X_SHA_512_DIGEST_LEN;
      break;
#endif // PSA_WANT_ALG_SHA_256
    default:
      *hash_length = SL_SI91X_SHA_LEN_INVALID;
      return PSA_ERROR_BAD_STATE;
  }

  status = convert_si91x_error_code_to_psa_status(sl_si91x_sha(sha_algo, (uint8_t *)input, input_length, hash));
  if (status != PSA_SUCCESS) {
    *hash_length = SL_SI91X_SHA_LEN_INVALID;
  }

#else
  // Suppress unused parameter warnings when no SHA algorithms are enabled
  (void)alg;
  (void)input;
  (void)input_length;
  (void)hash;
  (void)hash_size;
  (void)hash_length;
  (void)sha_algo;
#endif
  return status;
}
