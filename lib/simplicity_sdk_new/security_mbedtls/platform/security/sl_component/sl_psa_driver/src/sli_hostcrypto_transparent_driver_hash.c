/***************************************************************************//**
 * @file
 * @brief Silicon Labs PSA Crypto Transparent Driver Hash functions.
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

#include "psa/crypto_driver_common.h"
#include "psa/crypto_struct.h"

#include "sli_psa_driver_common.h"
#include "sli_hostcrypto_transparent_types.h"
#include "sli_hostcrypto_transparent_functions.h"

#include "sli_sxsymcrypt.h"
#include "sxsymcrypt/hash.h"
#include "sxsymcrypt/sha1.h"
#include "sxsymcrypt/sha2.h"
#include "sxsymcrypt/statuscodes.h"

#include "string.h"

psa_status_t sli_hostcrypto_transparent_hash_compute(psa_algorithm_t alg,
                                                     const uint8_t *input,
                                                     size_t input_length,
                                                     uint8_t *hash,
                                                     size_t hash_size,
                                                     size_t *hash_length)
{
#if defined(SLI_PSA_DRIVER_FEATURE_SHA1)    \
  || defined(SLI_PSA_DRIVER_FEATURE_SHA224) \
  || defined(SLI_PSA_DRIVER_FEATURE_SHA256) \
  || defined(SLI_PSA_DRIVER_FEATURE_SHA384) \
  || defined(SLI_PSA_DRIVER_FEATURE_SHA512)

  if ((input == NULL && input_length > 0)
      || (hash == NULL && hash_size > 0)
      || (hash_length == NULL)) {
    return PSA_ERROR_INVALID_ARGUMENT;
  }

  int sx_status = SX_ERR_UNITIALIZED_OBJ;
  *hash_length = 0;

  switch (alg) {
#if defined(SLI_PSA_DRIVER_FEATURE_SHA1)
    case PSA_ALG_SHA_1:
      if (hash_size < SX_HASH_DIGESTSZ_SHA1) {
        return PSA_ERROR_BUFFER_TOO_SMALL;
      }
      break;
#endif // SLI_PSA_DRIVER_FEATURE_SHA1
#if defined(SLI_PSA_DRIVER_FEATURE_SHA224)
    case PSA_ALG_SHA_224:
      if (hash_size < SX_HASH_DIGESTSZ_SHA2_224) {
        return PSA_ERROR_BUFFER_TOO_SMALL;
      }
      break;
#endif // SLI_PSA_DRIVER_FEATURE_SHA224
#if defined(SLI_PSA_DRIVER_FEATURE_SHA256)
    case PSA_ALG_SHA_256:
      if (hash_size < SX_HASH_DIGESTSZ_SHA2_256) {
        return PSA_ERROR_BUFFER_TOO_SMALL;
      }
      break;
#endif // SLI_PSA_DRIVER_FEATURE_SHA256
#if defined(SLI_PSA_DRIVER_FEATURE_SHA384)
    case PSA_ALG_SHA_384:
      if (hash_size < SX_HASH_DIGESTSZ_SHA2_384) {
        return PSA_ERROR_BUFFER_TOO_SMALL;
      }
      break;
#endif // SLI_PSA_DRIVER_FEATURE_SHA384
#if defined(SLI_PSA_DRIVER_FEATURE_SHA512)
    case PSA_ALG_SHA_512:
      if (hash_size < SX_HASH_DIGESTSZ_SHA2_512) {
        return PSA_ERROR_BUFFER_TOO_SMALL;
      }
      break;
#endif // SLI_PSA_DRIVER_FEATURE_SHA512
    default:
      return PSA_ERROR_NOT_SUPPORTED;
  }

  struct sxhash hash_ctx;

  if (sli_sxsymcrypt_lock_cryptomaster_selection(
        SLI_SXSYMCRYPT_CRYPTOMASTER_HOSTSYMCRYPTO, false)) {
    return PSA_ERROR_SERVICE_FAILURE;
  }

  switch (alg) {
#if defined(SLI_PSA_DRIVER_FEATURE_SHA1)
    case PSA_ALG_SHA_1:
      sx_status = sx_hash_create_sha1(&hash_ctx, sizeof(hash_ctx));
      break;
#endif // SLI_PSA_DRIVER_FEATURE_SHA1
#if defined(SLI_PSA_DRIVER_FEATURE_SHA224)
    case PSA_ALG_SHA_224:
      sx_status = sx_hash_create_sha224(&hash_ctx, sizeof(hash_ctx));
      break;
#endif // SLI_PSA_DRIVER_FEATURE_SHA224
#if defined(SLI_PSA_DRIVER_FEATURE_SHA256)
    case PSA_ALG_SHA_256:
      sx_status = sx_hash_create_sha256(&hash_ctx, sizeof(hash_ctx));
      break;
#endif // SLI_PSA_DRIVER_FEATURE_SHA256
#if defined(SLI_PSA_DRIVER_FEATURE_SHA384)
    case PSA_ALG_SHA_384:
      sx_status = sx_hash_create_sha384(&hash_ctx, sizeof(hash_ctx));
      break;
#endif // SLI_PSA_DRIVER_FEATURE_SHA384
#if defined(SLI_PSA_DRIVER_FEATURE_SHA512)
    case PSA_ALG_SHA_512:
      sx_status = sx_hash_create_sha512(&hash_ctx, sizeof(hash_ctx));
      break;
#endif // SLI_PSA_DRIVER_FEATURE_SHA512
  }

  if (sli_sxsymcrypt_unlock_cryptomaster_selection()) {
    return PSA_ERROR_SERVICE_FAILURE;
  }

  if (sx_status != SX_OK) {
    return PSA_ERROR_HARDWARE_FAILURE;
  }

  sx_status = sx_hash_feed(&hash_ctx, (const char *)input, input_length);
  if (sx_status != SX_OK) {
    return PSA_ERROR_HARDWARE_FAILURE;
  }

  sx_status = sx_hash_digest(&hash_ctx, (char *)hash);
  if (sx_status != SX_OK) {
    return PSA_ERROR_HARDWARE_FAILURE;
  }

  sx_status = sx_hash_wait(&hash_ctx);
  if (sx_status != SX_OK) {
    return PSA_ERROR_HARDWARE_FAILURE;
  }

  *hash_length = sx_hash_get_digestsz(&hash_ctx);

  return PSA_SUCCESS;

#else // SLI_PSA_DRIVER_FEATURE_SHA*

  (void)alg;
  (void)input;
  (void)input_length;
  (void)hash;
  (void)hash_size;
  (void)hash_length;

  return PSA_ERROR_NOT_SUPPORTED;

#endif // SLI_PSA_DRIVER_FEATURE_SHA*
}

#if defined(SLI_PSA_DRIVER_FEATURE_SHA1)    \
  || defined(SLI_PSA_DRIVER_FEATURE_SHA224) \
  || defined(SLI_PSA_DRIVER_FEATURE_SHA256) \
  || defined(SLI_PSA_DRIVER_FEATURE_SHA384) \
  || defined(SLI_PSA_DRIVER_FEATURE_SHA512)
static psa_status_t sli_hostcrypto_transparent_hash_create(
  sli_hostcrypto_transparent_hash_operation_t *operation)
{
  // Hash operation already created
  if (operation->hash_ctx.algo) {
    return PSA_ERROR_BAD_STATE;
  }

  int sx_status = SX_ERR_UNITIALIZED_OBJ;
  if (sli_sxsymcrypt_lock_cryptomaster_selection(
        SLI_SXSYMCRYPT_CRYPTOMASTER_HOSTSYMCRYPTO, false)) {
    return PSA_ERROR_SERVICE_FAILURE;
  }

  switch (operation->digest_size) {
#if defined(SLI_PSA_DRIVER_FEATURE_SHA1)
    case SX_HASH_DIGESTSZ_SHA1:
      sx_status = sx_hash_create_sha1(&operation->hash_ctx, sizeof(operation->hash_ctx));
      break;
#endif // SLI_PSA_DRIVER_FEATURE_SHA1
#if defined(SLI_PSA_DRIVER_FEATURE_SHA224)
    case SX_HASH_DIGESTSZ_SHA2_224:
      sx_status = sx_hash_create_sha224(&operation->hash_ctx, sizeof(operation->hash_ctx));
      break;
#endif // SLI_PSA_DRIVER_FEATURE_SHA224
#if defined(SLI_PSA_DRIVER_FEATURE_SHA256)
    case SX_HASH_DIGESTSZ_SHA2_256:
      sx_status = sx_hash_create_sha256(&operation->hash_ctx, sizeof(operation->hash_ctx));
      break;
#endif // SLI_PSA_DRIVER_FEATURE_SHA256
#if defined(SLI_PSA_DRIVER_FEATURE_SHA384)
    case SX_HASH_DIGESTSZ_SHA2_384:
      sx_status = sx_hash_create_sha384(&operation->hash_ctx, sizeof(operation->hash_ctx));
      break;
#endif // SLI_PSA_DRIVER_FEATURE_SHA384
#if defined(SLI_PSA_DRIVER_FEATURE_SHA512)
    case SX_HASH_DIGESTSZ_SHA2_512:
      sx_status = sx_hash_create_sha512(&operation->hash_ctx, sizeof(operation->hash_ctx));
      break;
#endif // SLI_PSA_DRIVER_FEATURE_SHA512
    default:
      (void) sx_status;
      if (sli_sxsymcrypt_unlock_cryptomaster_selection()) {
        return PSA_ERROR_SERVICE_FAILURE;
      }
      return PSA_ERROR_NOT_SUPPORTED;
  }

  if (sli_sxsymcrypt_unlock_cryptomaster_selection()) {
    return PSA_ERROR_SERVICE_FAILURE;
  }

  if (sx_status != SX_OK) {
    return PSA_ERROR_HARDWARE_FAILURE;
  } else {
    return PSA_SUCCESS;
  }
}
#endif

psa_status_t sli_hostcrypto_transparent_hash_setup(
  sli_hostcrypto_transparent_hash_operation_t *operation,
  psa_algorithm_t alg)
{
#if defined(SLI_PSA_DRIVER_FEATURE_SHA1)    \
  || defined(SLI_PSA_DRIVER_FEATURE_SHA224) \
  || defined(SLI_PSA_DRIVER_FEATURE_SHA256) \
  || defined(SLI_PSA_DRIVER_FEATURE_SHA384) \
  || defined(SLI_PSA_DRIVER_FEATURE_SHA512)

  if (operation == NULL) {
    return PSA_ERROR_INVALID_ARGUMENT;
  }

  if (!PSA_ALG_IS_HASH(alg)) {
    return PSA_ERROR_INVALID_ARGUMENT;
  }

  // Reset context.
  memset(operation, 0, sizeof(sli_hostcrypto_transparent_hash_operation_t));

  switch (alg) {
#if defined(SLI_PSA_DRIVER_FEATURE_SHA1)
    case PSA_ALG_SHA_1:
      operation->digest_size = SX_HASH_DIGESTSZ_SHA1;
      operation->hash_block_size = SX_HASH_BLOCKSZ_SHA1;
      break;
#endif // SLI_PSA_DRIVER_FEATURE_SHA1
#if defined(SLI_PSA_DRIVER_FEATURE_SHA224)
    case PSA_ALG_SHA_224:
      operation->digest_size = SX_HASH_DIGESTSZ_SHA2_224;
      operation->hash_block_size = SX_HASH_BLOCKSZ_SHA2_224;
      break;
#endif // SLI_PSA_DRIVER_FEATURE_SHA224
#if defined(SLI_PSA_DRIVER_FEATURE_SHA256)
    case PSA_ALG_SHA_256:
      operation->digest_size = SX_HASH_DIGESTSZ_SHA2_256;
      operation->hash_block_size = SX_HASH_BLOCKSZ_SHA2_256;
      break;
#endif // SLI_PSA_DRIVER_FEATURE_SHA256
#if defined(SLI_PSA_DRIVER_FEATURE_SHA384)
    case PSA_ALG_SHA_384:
      operation->digest_size = SX_HASH_DIGESTSZ_SHA2_384;
      operation->hash_block_size = SX_HASH_BLOCKSZ_SHA2_384;
      break;
#endif // SLI_PSA_DRIVER_FEATURE_SHA384
#if defined(SLI_PSA_DRIVER_FEATURE_SHA512)
    case PSA_ALG_SHA_512:
      operation->digest_size = SX_HASH_DIGESTSZ_SHA2_512;
      operation->hash_block_size = SX_HASH_BLOCKSZ_SHA2_512;
      break;
#endif // SLI_PSA_DRIVER_FEATURE_SHA512
    default:
      return PSA_ERROR_NOT_SUPPORTED;
  }

  return PSA_SUCCESS;

#else // SLI_PSA_DRIVER_FEATURE_SHA*

  (void)operation;
  (void)alg;

  return PSA_ERROR_NOT_SUPPORTED;

#endif // SLI_PSA_DRIVER_FEATURE_SHA*
}

psa_status_t sli_hostcrypto_transparent_hash_update(
  sli_hostcrypto_transparent_hash_operation_t *operation,
  const uint8_t *input,
  size_t input_length)
{
#if defined(SLI_PSA_DRIVER_FEATURE_SHA1)    \
  || defined(SLI_PSA_DRIVER_FEATURE_SHA224) \
  || defined(SLI_PSA_DRIVER_FEATURE_SHA256) \
  || defined(SLI_PSA_DRIVER_FEATURE_SHA384) \
  || defined(SLI_PSA_DRIVER_FEATURE_SHA512)

  psa_status_t psa_status;
  int sx_status = SX_ERR_UNITIALIZED_OBJ;

  if (operation == NULL
      || (input == NULL && input_length > 0)) {
    return PSA_ERROR_INVALID_ARGUMENT;
  }

  switch (operation->digest_size) {
#if defined(SLI_PSA_DRIVER_FEATURE_SHA1)
    case SX_HASH_DIGESTSZ_SHA1:
#endif // SLI_PSA_DRIVER_FEATURE_SHA1
#if defined(SLI_PSA_DRIVER_FEATURE_SHA224)
    case SX_HASH_DIGESTSZ_SHA2_224:
#endif // SLI_PSA_DRIVER_FEATURE_SHA224
#if defined(SLI_PSA_DRIVER_FEATURE_SHA256)
    case SX_HASH_DIGESTSZ_SHA2_256:
#endif // SLI_PSA_DRIVER_FEATURE_SHA256
#if defined(SLI_PSA_DRIVER_FEATURE_SHA384)
    case SX_HASH_DIGESTSZ_SHA2_384:
#endif // SLI_PSA_DRIVER_FEATURE_SHA384
#if defined(SLI_PSA_DRIVER_FEATURE_SHA512)
    case SX_HASH_DIGESTSZ_SHA2_512:
#endif // SLI_PSA_DRIVER_FEATURE_SHA512
    break;
    default:
      // Digest size must have not been initialized by the setup function.
      return PSA_ERROR_BAD_STATE;
  }

  if (input_length == 0) {
    return PSA_SUCCESS;
  }

  uint8_t bytes_left_in_block = operation->hash_block_size
                                - (operation->processed_len % operation->hash_block_size);

  if ((bytes_left_in_block > 0) && (input_length >= bytes_left_in_block)) {
    memcpy(&operation->block[operation->processed_len % operation->hash_block_size],
           input,
           bytes_left_in_block);

    if (!operation->hash_ctx.dma.regs && operation->hash_ctx.algo) {
      if (sli_sxsymcrypt_lock_cryptomaster_selection(
            SLI_SXSYMCRYPT_CRYPTOMASTER_HOSTSYMCRYPTO, false)) {
        return PSA_ERROR_SERVICE_FAILURE;
      }
      sx_status = sx_hash_resume_state(&operation->hash_ctx);
      if (sli_sxsymcrypt_unlock_cryptomaster_selection()) {
        return PSA_ERROR_SERVICE_FAILURE;
      }
      if (sx_status != SX_OK) {
        return PSA_ERROR_HARDWARE_FAILURE;
      }
    }

    if ((operation->processed_len < operation->hash_block_size)
        && !operation->hash_ctx.algo) {
      // We haven't sent input to HW yet so create operation now
      psa_status = sli_hostcrypto_transparent_hash_create(operation);
      if (psa_status != PSA_SUCCESS) {
        return psa_status;
      }
    }

    sx_status = sx_hash_feed(&operation->hash_ctx,
                             (const char *)operation->block,
                             operation->hash_block_size);
    if (sx_status != SX_OK) {
      return PSA_ERROR_HARDWARE_FAILURE;
    }
    sx_status = sx_hash_save_state(&operation->hash_ctx);
    if (sx_status != SX_OK) {
      return PSA_ERROR_HARDWARE_FAILURE;
    }
    sx_status = sx_hash_wait(&operation->hash_ctx);
    if (sx_status != SX_OK) {
      return PSA_ERROR_HARDWARE_FAILURE;
    }
    input += bytes_left_in_block;
    input_length -= bytes_left_in_block;
    operation->processed_len += bytes_left_in_block;
  }

  if (input_length >= operation->hash_block_size) {
    size_t operation_size = (input_length / operation->hash_block_size) * operation->hash_block_size;

    if (!operation->hash_ctx.dma.regs && operation->hash_ctx.algo) {
      if (sli_sxsymcrypt_lock_cryptomaster_selection(
            SLI_SXSYMCRYPT_CRYPTOMASTER_HOSTSYMCRYPTO, false)) {
        return PSA_ERROR_SERVICE_FAILURE;
      }
      sx_status = sx_hash_resume_state(&operation->hash_ctx);
      if (sli_sxsymcrypt_unlock_cryptomaster_selection()) {
        return PSA_ERROR_HARDWARE_FAILURE;
      }
      if (sx_status != SX_OK) {
        return PSA_ERROR_HARDWARE_FAILURE;
      }
    }

    if ((operation->processed_len < operation->hash_block_size)
        && !operation->hash_ctx.algo) {
      // We haven't sent input to HW yet so create operation now
      psa_status = sli_hostcrypto_transparent_hash_create(operation);
      if (psa_status != PSA_SUCCESS) {
        return psa_status;
      }
    }
    sx_status = sx_hash_feed(&operation->hash_ctx,
                             (const char *)input,
                             operation_size);
    if (sx_status != SX_OK) {
      return PSA_ERROR_HARDWARE_FAILURE;
    }
    sx_status = sx_hash_save_state(&operation->hash_ctx);
    if (sx_status != SX_OK) {
      return PSA_ERROR_HARDWARE_FAILURE;
    }
    sx_status = sx_hash_wait(&operation->hash_ctx);
    if (sx_status != SX_OK) {
      return PSA_ERROR_HARDWARE_FAILURE;
    }

    input += operation_size;
    input_length -= operation_size;
    operation->processed_len += operation_size;
  }

  if (input_length > 0) {
    memcpy(&operation->block[operation->processed_len % operation->hash_block_size],
           input,
           input_length);
    operation->processed_len += input_length;
  }

  return PSA_SUCCESS;

#else // SLI_PSA_DRIVER_FEATURE_SHA*

  (void)operation;
  (void)input;
  (void)input_length;

  return PSA_ERROR_NOT_SUPPORTED;

#endif // SLI_PSA_DRIVER_FEATURE_SHA*
}

psa_status_t sli_hostcrypto_transparent_hash_finish(
  sli_hostcrypto_transparent_hash_operation_t *operation,
  uint8_t *hash,
  size_t hash_size,
  size_t *hash_length)
{
#if defined(SLI_PSA_DRIVER_FEATURE_SHA1)    \
  || defined(SLI_PSA_DRIVER_FEATURE_SHA224) \
  || defined(SLI_PSA_DRIVER_FEATURE_SHA256) \
  || defined(SLI_PSA_DRIVER_FEATURE_SHA384) \
  || defined(SLI_PSA_DRIVER_FEATURE_SHA512)

  psa_status_t psa_status;
  int sx_status = SX_ERR_UNITIALIZED_OBJ;

  if (operation == NULL
      || (hash_length == NULL && hash_size > 0)
      || (hash == NULL && hash_size > 0)) {
    return PSA_ERROR_INVALID_ARGUMENT;
  }

  switch (operation->digest_size) {
#if defined(SLI_PSA_DRIVER_FEATURE_SHA1)
    case SX_HASH_DIGESTSZ_SHA1:
#endif // SLI_PSA_DRIVER_FEATURE_SHA1
#if defined(SLI_PSA_DRIVER_FEATURE_SHA224)
    case SX_HASH_DIGESTSZ_SHA2_224:
#endif // SLI_PSA_DRIVER_FEATURE_SHA224
#if defined(SLI_PSA_DRIVER_FEATURE_SHA256)
    case SX_HASH_DIGESTSZ_SHA2_256:
#endif // SLI_PSA_DRIVER_FEATURE_SHA256
#if defined(SLI_PSA_DRIVER_FEATURE_SHA384)
    case SX_HASH_DIGESTSZ_SHA2_384:
#endif // SLI_PSA_DRIVER_FEATURE_SHA384
#if defined(SLI_PSA_DRIVER_FEATURE_SHA512)
    case SX_HASH_DIGESTSZ_SHA2_512:
#endif // SLI_PSA_DRIVER_FEATURE_SHA512
    break;
    default:
      // Digest size must have not been initialized by the setup function.
      return PSA_ERROR_BAD_STATE;
  }

  if (hash_size < operation->digest_size) {
    return PSA_ERROR_BUFFER_TOO_SMALL;
  }

  if (!operation->hash_ctx.dma.regs && operation->hash_ctx.algo) {
    if (sli_sxsymcrypt_lock_cryptomaster_selection(
          SLI_SXSYMCRYPT_CRYPTOMASTER_HOSTSYMCRYPTO, false)) {
      return PSA_ERROR_SERVICE_FAILURE;
    }
    sx_status = sx_hash_resume_state(&operation->hash_ctx);
    if (sli_sxsymcrypt_unlock_cryptomaster_selection()) {
      return PSA_ERROR_SERVICE_FAILURE;
    }
    if (sx_status != SX_OK) {
      return PSA_ERROR_HARDWARE_FAILURE;
    }
  }

  if ((operation->processed_len % operation->hash_block_size) != 0) {
    if (operation->processed_len < operation->hash_block_size
        && !operation->hash_ctx.algo) {
      // We haven't sent input to HW yet so create operation now
      psa_status = sli_hostcrypto_transparent_hash_create(operation);
      if (psa_status != PSA_SUCCESS) {
        return psa_status;
      }
    }
    sx_status = sx_hash_feed(&operation->hash_ctx,
                             (const char *)operation->block,
                             operation->processed_len % operation->hash_block_size);
    if (sx_status != SX_OK) {
      return PSA_ERROR_HARDWARE_FAILURE;
    }
  }

  if ((operation->processed_len < operation->hash_block_size)
      && !operation->hash_ctx.algo) {
    // We haven't sent input to HW yet so create operation now
    psa_status = sli_hostcrypto_transparent_hash_create(operation);
    if (psa_status != PSA_SUCCESS) {
      return psa_status;
    }
  }

  sx_status = sx_hash_digest(&operation->hash_ctx, (char *)hash);
  if (sx_status != SX_OK) {
    return PSA_ERROR_HARDWARE_FAILURE;
  }

  sx_status = sx_hash_wait(&operation->hash_ctx);
  if (sx_status != SX_OK) {
    return PSA_ERROR_HARDWARE_FAILURE;
  }

  *hash_length = operation->digest_size;
  memset(operation, 0, sizeof(sli_hostcrypto_transparent_hash_operation_t));

  return PSA_SUCCESS;

#else // SLI_PSA_DRIVER_FEATURE_SHA*

  (void)operation;
  (void)hash;
  (void)hash_size;
  (void)hash_length;

  return PSA_ERROR_NOT_SUPPORTED;

#endif // SLI_PSA_DRIVER_FEATURE_SHA*
}

psa_status_t sli_hostcrypto_transparent_hash_abort(
  sli_hostcrypto_transparent_hash_operation_t *operation)
{
#if defined(SLI_PSA_DRIVER_FEATURE_SHA1)    \
  || defined(SLI_PSA_DRIVER_FEATURE_SHA224) \
  || defined(SLI_PSA_DRIVER_FEATURE_SHA256) \
  || defined(SLI_PSA_DRIVER_FEATURE_SHA384) \
  || defined(SLI_PSA_DRIVER_FEATURE_SHA512)

  if (operation != NULL) {
    // Accelerator does not keep state, so just zero out the context and we're good.
    memset(operation, 0, sizeof(sli_hostcrypto_transparent_hash_operation_t));
  }

  return PSA_SUCCESS;

#else // SLI_PSA_DRIVER_FEATURE_SHA*

  (void)operation;

  return PSA_ERROR_NOT_SUPPORTED;

#endif // SLI_PSA_DRIVER_FEATURE_SHA*
}

psa_status_t sli_hostcrypto_transparent_hash_clone(
  const sli_hostcrypto_transparent_hash_operation_t *source_operation,
  sli_hostcrypto_transparent_hash_operation_t *target_operation)
{
#if defined(SLI_PSA_DRIVER_FEATURE_SHA1)    \
  || defined(SLI_PSA_DRIVER_FEATURE_SHA224) \
  || defined(SLI_PSA_DRIVER_FEATURE_SHA256) \
  || defined(SLI_PSA_DRIVER_FEATURE_SHA384) \
  || defined(SLI_PSA_DRIVER_FEATURE_SHA512)

  if (source_operation == NULL
      || target_operation == NULL) {
    return PSA_ERROR_BAD_STATE;
  }

  // Source operation must be active (setup has been called)
  if (!source_operation->digest_size) {
    return PSA_ERROR_BAD_STATE;
  }

  // Target operation must be inactive (setup has not been called)
  if (target_operation->digest_size) {
    return PSA_ERROR_BAD_STATE;
  }

  memset(target_operation, 0, sizeof(sli_hostcrypto_transparent_hash_operation_t));

  // Directly copying source to target as sxsymcrypt reserves HW when resuming
  // context or creating new operation
  *target_operation = *source_operation;

  return PSA_SUCCESS;

#else // SLI_PSA_DRIVER_FEATURE_SHA*

  (void)source_operation;
  (void)target_operation;

  return PSA_ERROR_NOT_SUPPORTED;

#endif // SLI_PSA_DRIVER_FEATURE_SHA*
}

#endif // SLI_MBEDTLS_DEVICE_HC
