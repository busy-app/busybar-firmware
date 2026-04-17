/***************************************************************************//**
 * @file
 * @brief PSA Crypto MAC drivers for the HostCryptoSubSystem.
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
#include "sl_psa_values.h"

#include "sli_sxsymcrypt.h"
#include "sxsymcrypt/sha1.h"
#include "sxsymcrypt/sha2.h"
#include "sxsymcrypt/hmac.h"
#include "sxsymcrypt/cmac.h"
#include "sxsymcrypt/mac.h"
#include "sxsymcrypt/keyref.h"
#include "sxsymcrypt/statuscodes.h"

#include "string.h"

#define SLI_HOSTCRYPTO_CMAC_CTX_SAVE            (1u << 5)
#define SLI_HOSTCRYPTO_CMAC_MAC_SIZE            16

// -----------------------------------------------------------------------------
// Static Helper Functions

#if defined(SLI_PSA_DRIVER_FEATURE_HMAC) || defined(SLI_PSA_DRIVER_FEATURE_CMAC)
static psa_status_t driver_can_handle(const psa_key_attributes_t *attributes,
                                      psa_algorithm_t alg,
                                      size_t key_buffer_size)
{
  psa_key_location_t location =
    PSA_KEY_LIFETIME_GET_LOCATION(psa_get_key_lifetime(attributes));

#if defined(SLI_PSA_DRIVER_FEATURE_HMAC)
  if (PSA_ALG_IS_HMAC(alg)) {
    psa_key_type_t key_type = psa_get_key_type(attributes);

    if (location == PSA_KEY_LOCATION_LOCAL_STORAGE) {
      // For local storage, require HMAC key type
      if (key_type != PSA_KEY_TYPE_HMAC) {
        return PSA_ERROR_INVALID_ARGUMENT;
      }
      size_t key_bits = psa_get_key_bits(attributes);
      if (key_buffer_size < PSA_BITS_TO_BYTES(key_bits)) {
        return PSA_ERROR_INVALID_ARGUMENT;
      }
#if defined(SLI_PSA_DRIVER_FEATURE_KSU)
    } else if (location == SL_PSA_KEY_LOCATION_KSU_0) {
      // For KSU, allow raw data key type
      if (key_type != PSA_KEY_TYPE_RAW_DATA && key_type != PSA_KEY_TYPE_HMAC) {
        return PSA_ERROR_INVALID_ARGUMENT;
      }
      size_t key_bits = psa_get_key_bits(attributes);
      // KSU & PSA crypto only supports 512 and keys for MAC operations
      if (key_bits != 512) {
        return PSA_ERROR_NOT_SUPPORTED;
      }
      if (key_buffer_size < sizeof(uint8_t)) {
        return PSA_ERROR_INVALID_ARGUMENT;
      }
#endif // SLI_PSA_DRIVER_FEATURE_KSU
    } else {
      return PSA_ERROR_INVALID_ARGUMENT;
    }

    // Validate HMAC hash algorithm
    switch (PSA_ALG_HMAC_GET_HASH(alg)) {
      case PSA_ALG_SHA_1:
      case PSA_ALG_SHA_224:
      case PSA_ALG_SHA_256:
      case PSA_ALG_SHA_384:
      case PSA_ALG_SHA_512:
        break;
      default:
        return PSA_ERROR_NOT_SUPPORTED;
    }
  } else
#endif // SLI_PSA_DRIVER_FEATURE_HMAC

#if defined(SLI_PSA_DRIVER_FEATURE_CMAC)
  if (PSA_ALG_FULL_LENGTH_MAC(alg) == PSA_ALG_CMAC) {
    if (psa_get_key_type(attributes) != PSA_KEY_TYPE_AES) {
      return PSA_ERROR_NOT_SUPPORTED;
    }

    if (location == PSA_KEY_LOCATION_LOCAL_STORAGE) {
      size_t key_bits = psa_get_key_bits(attributes);
      if (key_buffer_size < PSA_BITS_TO_BYTES(key_bits)
          || (key_bits != 128 && key_bits != 192 && key_bits != 256)) {
        return PSA_ERROR_INVALID_ARGUMENT;
      }
#if defined(SLI_PSA_DRIVER_FEATURE_KSU)
    } else if (location == SL_PSA_KEY_LOCATION_KSU_0) {
      size_t key_bits = psa_get_key_bits(attributes);
      // KSU supports standard AES key sizes for CMAC (AES engine, not hash engine)
      if (key_bits != 128 && key_bits != 192 && key_bits != 256) {
        return PSA_ERROR_INVALID_ARGUMENT;
      }
      if (key_buffer_size < sizeof(uint8_t)) {
        return PSA_ERROR_INVALID_ARGUMENT;
      }
#endif // SLI_PSA_DRIVER_FEATURE_KSU
    } else {
      return PSA_ERROR_INVALID_ARGUMENT;
    }
  } else
#endif // SLI_PSA_DRIVER_FEATURE_CMAC
  {
    return PSA_ERROR_NOT_SUPPORTED;
  }

  return PSA_SUCCESS;
}
#endif

#if defined(SLI_PSA_DRIVER_FEATURE_HMAC)
static psa_status_t sli_hostcrypto_hmac_validate_key(
  const psa_key_attributes_t *attributes,
  psa_algorithm_t alg,
  size_t *digest_size)
{
  // Check key type and output size
  psa_key_type_t key_type = psa_get_key_type(attributes);
  psa_key_location_t location =
    PSA_KEY_LIFETIME_GET_LOCATION(psa_get_key_lifetime(attributes));

#if defined(SLI_PSA_DRIVER_FEATURE_KSU)
  if (location == SL_PSA_KEY_LOCATION_KSU_0) {
    // For KSU, allow raw data key type for HMAC
    if (key_type != PSA_KEY_TYPE_RAW_DATA && key_type != PSA_KEY_TYPE_HMAC) {
      return PSA_ERROR_INVALID_ARGUMENT;
    }
  } else
#endif // SLI_PSA_DRIVER_FEATURE_KSU
  {
    // For local storage, require HMAC key type
    if (key_type != PSA_KEY_TYPE_HMAC) {
      return PSA_ERROR_INVALID_ARGUMENT;
    }
  }

  switch (PSA_ALG_HMAC_GET_HASH(alg)) {
    case PSA_ALG_SHA_1:
      *digest_size = SX_HASH_DIGESTSZ_SHA1;
      break;
    case PSA_ALG_SHA_224:
      *digest_size = SX_HASH_DIGESTSZ_SHA2_224;
      break;
    case PSA_ALG_SHA_256:
      *digest_size = SX_HASH_DIGESTSZ_SHA2_256;
      break;
    case PSA_ALG_SHA_384:
      *digest_size = SX_HASH_DIGESTSZ_SHA2_384;
      break;
    case PSA_ALG_SHA_512:
      *digest_size = SX_HASH_DIGESTSZ_SHA2_512;
      break;
    default:
      return PSA_ERROR_NOT_SUPPORTED;
  }

  return PSA_SUCCESS;
}
#endif // SLI_PSA_DRIVER_FEATURE_HMAC

#if defined(SLI_PSA_DRIVER_FEATURE_CMAC)
static psa_status_t sli_hostcrypto_cmac_validate_key(
  const psa_key_attributes_t *attributes)
{
  // Check key type and size
  if (psa_get_key_type(attributes) != PSA_KEY_TYPE_AES) {
    // CMAC could be used with DES
    return PSA_ERROR_NOT_SUPPORTED;
  }

  switch (psa_get_key_bits(attributes) / 8) {
    case 16:
    case 24:
    case 32:
      break;
    default:
      // There's no other AES algorithm in existence
      return PSA_ERROR_INVALID_ARGUMENT;
  }

  return PSA_SUCCESS;
}
#endif // SLI_PSA_DRIVER_FEATURE_CMAC

psa_status_t sli_hostcrypto_transparent_mac_compute(
  const psa_key_attributes_t *attributes,
  const uint8_t *key_buffer,
  size_t key_buffer_size,
  psa_algorithm_t alg,
  const uint8_t *input,
  size_t input_length,
  uint8_t *mac,
  size_t mac_size,
  size_t *mac_length)
{
#if defined(SLI_PSA_DRIVER_FEATURE_HMAC) || defined(SLI_PSA_DRIVER_FEATURE_CMAC)

  if (key_buffer == NULL
      || attributes == NULL
      || mac == NULL
      || mac_length == NULL
      || ((input == NULL) && (input_length > 0))) {
    return PSA_ERROR_INVALID_ARGUMENT;
  }

  psa_status_t psa_status = driver_can_handle(attributes, alg, key_buffer_size);
  if (psa_status != PSA_SUCCESS) {
    return psa_status;
  }

  int sx_status = SX_ERR_UNITIALIZED_OBJ;
  struct sxmac mac_ctx;
  size_t output_len = 0;

  struct sxkeyref key_ref;
  psa_status = sli_hostcrypto_load_key(&key_ref, attributes, key_buffer);
  if (psa_status != PSA_SUCCESS) {
    return psa_status;
  }

  *mac_length = 0;
#if defined(SLI_PSA_DRIVER_FEATURE_HMAC)
  if (PSA_ALG_IS_HMAC(alg)) {
    psa_status = sli_hostcrypto_hmac_validate_key(attributes, alg, &output_len);
    if (psa_status != PSA_SUCCESS) {
      return psa_status;
    }

    if ((PSA_MAC_TRUNCATED_LENGTH(alg) > 0)
        && (PSA_MAC_TRUNCATED_LENGTH(alg) < output_len)) {
      output_len = PSA_MAC_TRUNCATED_LENGTH(alg);
    }

    if (mac_size < output_len) {
      return PSA_ERROR_BUFFER_TOO_SMALL;
    }

    if (sli_sxsymcrypt_lock_cryptomaster_selection(
          SLI_SXSYMCRYPT_CRYPTOMASTER_HOSTSYMCRYPTO, false)) {
      return PSA_ERROR_SERVICE_FAILURE;
    }

    switch (PSA_ALG_HMAC_GET_HASH(alg)) {
      case PSA_ALG_SHA_1:
        sx_status = sx_mac_create_hmac_sha1(&mac_ctx, &key_ref);
        break;
      case PSA_ALG_SHA_224:
        sx_status = sx_mac_create_hmac_sha2_224(&mac_ctx, &key_ref);
        break;
      case PSA_ALG_SHA_256:
        sx_status = sx_mac_create_hmac_sha2_256(&mac_ctx, &key_ref);
        break;
      case PSA_ALG_SHA_384:
        sx_status = sx_mac_create_hmac_sha2_384(&mac_ctx, &key_ref);
        break;
      case PSA_ALG_SHA_512:
        sx_status = sx_mac_create_hmac_sha2_512(&mac_ctx, &key_ref);
        break;
      default:
        return PSA_ERROR_NOT_SUPPORTED;
    }

    if (sli_sxsymcrypt_unlock_cryptomaster_selection()) {
      return PSA_ERROR_SERVICE_FAILURE;
    }
    if (sx_status != SX_OK) {
      return PSA_ERROR_HARDWARE_FAILURE;
    }
  } else
#endif // SLI_PSA_DRIVER_FEATURE_HMAC

#if defined(SLI_PSA_DRIVER_FEATURE_CMAC)
  // If not HMAC, continue with the regular MAC algos
  if (PSA_ALG_FULL_LENGTH_MAC(alg) == PSA_ALG_CMAC) {
    psa_status = sli_hostcrypto_cmac_validate_key(attributes);
    if (psa_status != PSA_SUCCESS) {
      return psa_status;
    }

    output_len = PSA_MAC_TRUNCATED_LENGTH(alg);
    if (output_len == 0) {
      output_len = 16;
    } else if (output_len > 16) {
      return PSA_ERROR_INVALID_ARGUMENT;
    }

    if (mac_size < output_len) {
      return PSA_ERROR_BUFFER_TOO_SMALL;
    }

    if (sli_sxsymcrypt_lock_cryptomaster_selection(
          SLI_SXSYMCRYPT_CRYPTOMASTER_HOSTSYMCRYPTO, false)) {
      return PSA_ERROR_SERVICE_FAILURE;
    }

    sx_status = sx_mac_create_aescmac(&mac_ctx, &key_ref);

    if (sli_sxsymcrypt_unlock_cryptomaster_selection()) {
      return PSA_ERROR_SERVICE_FAILURE;
    }

    if (sx_status != SX_OK) {
      return PSA_ERROR_HARDWARE_FAILURE;
    }
  } else
#endif // SLI_PSA_DRIVER_FEATURE_CMAC
  {
    return PSA_ERROR_NOT_SUPPORTED;
  }

  sx_status = sx_mac_feed(&mac_ctx, (const char *)input, input_length);
  if (sx_status != SX_OK) {
    return PSA_ERROR_HARDWARE_FAILURE;
  }

  uint8_t output[64] = { 0 };
  sx_status = sx_mac_generate(&mac_ctx, (char *)output);
  if (sx_status != SX_OK) {
    return PSA_ERROR_HARDWARE_FAILURE;
  }

  sx_status = sx_mac_wait(&mac_ctx);
  if (sx_status != SX_OK) {
    return PSA_ERROR_HARDWARE_FAILURE;
  }

  memcpy(mac, output, output_len);
  *mac_length = output_len;

  memset(output, 0, sizeof(output));

  return PSA_SUCCESS;

#else // SLI_PSA_DRIVER_FEATURE_HMAC || SLI_PSA_DRIVER_FEATURE_CMAC

  (void)attributes;
  (void)key_buffer;
  (void)key_buffer_size;
  (void)alg;
  (void)input;
  (void)input_length;
  (void)mac;
  (void)mac_size;
  (void)mac_length;

  return PSA_ERROR_NOT_SUPPORTED;

#endif // SLI_PSA_DRIVER_FEATURE_HMAC || SLI_PSA_DRIVER_FEATURE_CMAC
}

// Make sure that the two locations of 'alg' are in the same place, since we access them
// interchangeably.
#if defined(SLI_PSA_DRIVER_FEATURE_HMAC)
_Static_assert(offsetof(sli_hostcrypto_transparent_mac_operation_t, hmac.alg)
               == offsetof(sli_hostcrypto_transparent_mac_operation_t, cmac.alg),
               "hmac.alg and cmac.alg are not aliases");
#endif // SLI_PSA_DRIVER_FEATURE_HMAC

psa_status_t sli_hostcrypto_transparent_mac_sign_setup(
  sli_hostcrypto_transparent_mac_operation_t *operation,
  const psa_key_attributes_t *attributes,
  const uint8_t *key_buffer,
  size_t key_buffer_size,
  psa_algorithm_t alg)
{
#if defined(SLI_PSA_DRIVER_FEATURE_HMAC) || defined(SLI_PSA_DRIVER_FEATURE_CMAC)

  if (operation == NULL
      || attributes == NULL
      || (key_buffer == NULL && key_buffer_size > 0)) {
    return PSA_ERROR_INVALID_ARGUMENT;
  }

  psa_status_t status = driver_can_handle(attributes, alg, key_buffer_size);
  if (status != PSA_SUCCESS) {
    return status;
  }

  // start by resetting context
  memset(operation, 0, sizeof(*operation));

#if defined(SLI_PSA_DRIVER_FEATURE_HMAC)
  if (PSA_ALG_IS_HMAC(alg)) {
    size_t output_len = 0;
    status = sli_hostcrypto_hmac_validate_key(attributes, alg, &output_len);
    if (status != PSA_SUCCESS) {
      return status;
    }

    if (PSA_MAC_TRUNCATED_LENGTH(alg) > output_len) {
      return PSA_ERROR_INVALID_ARGUMENT;
    }

    // HOSTCRYPTO does not support multipart HMAC. Construct it from hashing instead.
    psa_algorithm_t hash_alg = PSA_ALG_HMAC_GET_HASH(alg);

    // Get key size from attributes or buffer size based on location
    psa_key_location_t location =
      PSA_KEY_LIFETIME_GET_LOCATION(psa_get_key_lifetime(attributes));

#if defined(SLI_PSA_DRIVER_FEATURE_KSU)
    // KSU keys cannot be used for multipart HMAC because:
    // 1. The key material is stored in hardware and not accessible to software
    // 2. Multipart HMAC requires constructing IPAD/OPAD from the actual key bytes
    // 3. For KSU keys, key_buffer contains only a small slot reference, not key data
    if (location == SL_PSA_KEY_LOCATION_KSU_0) {
      return PSA_ERROR_NOT_SUPPORTED;
    }
#endif // SLI_PSA_DRIVER_FEATURE_KSU

    size_t key_size = 0;

    if (location == PSA_KEY_LOCATION_LOCAL_STORAGE) {
      key_size = PSA_BITS_TO_BYTES(psa_get_key_bits(attributes));
      if (key_size > key_buffer_size) {
        return PSA_ERROR_INVALID_ARGUMENT;
      }
    } else {
      return PSA_ERROR_INVALID_ARGUMENT;
    }

    // Reduce the key if larger than a block
    if (key_size > PSA_HASH_BLOCK_LENGTH(hash_alg)) {
      status = sli_hostcrypto_transparent_hash_compute(
        hash_alg,
        key_buffer,
        key_size,
        operation->hmac.opad,
        sizeof(operation->hmac.opad),
        &key_size);
      if (status != PSA_SUCCESS) {
        return status;
      }
    } else if (key_size > 0) {
      memcpy(operation->hmac.opad, key_buffer, key_size);
    }

    // Calculate inner padding in opad buffer and start a multipart hash with it
    for (size_t i = 0; i < key_size; i++) {
      operation->hmac.opad[i] ^= 0x36;
    }
    memset(&operation->hmac.opad[key_size], 0x36, PSA_HASH_BLOCK_LENGTH(hash_alg) - key_size);

    status = sli_hostcrypto_transparent_hash_setup(&operation->hmac.hash_ctx,
                                                   hash_alg);
    if (status != PSA_SUCCESS) {
      return status;
    }

    status = sli_hostcrypto_transparent_hash_update(&operation->hmac.hash_ctx,
                                                    operation->hmac.opad,
                                                    PSA_HASH_BLOCK_LENGTH(hash_alg));
    if (status != PSA_SUCCESS) {
      return status;
    }

    // Calculate outer padding and store it for finalisation
    for (size_t i = 0; i < PSA_HASH_BLOCK_LENGTH(hash_alg); i++) {
      operation->hmac.opad[i] ^= 0x36 ^ 0x5C;
    }

    operation->hmac.alg = alg;
    return PSA_SUCCESS;
  } else
#endif // SLI_PSA_DRIVER_FEATURE_HMAC

  // If not HMAC, check other algos
#if defined(SLI_PSA_DRIVER_FEATURE_CMAC)
  if (PSA_ALG_FULL_LENGTH_MAC(alg) == PSA_ALG_CMAC) {
    status = sli_hostcrypto_cmac_validate_key(attributes);
    if (status != PSA_SUCCESS) {
      return status;
    }

    operation->cmac.alg = alg;
    status = sli_hostcrypto_load_key(&operation->cmac.key_ref, attributes, key_buffer);
  } else
#endif // SLI_PSA_DRIVER_FEATURE_CMAC
  {
#if !defined(SLI_PSA_DRIVER_FEATURE_CMAC)
    (void)key_buffer_size;
#endif // !SLI_PSA_DRIVER_FEATURE_CMAC
    status = PSA_ERROR_NOT_SUPPORTED;
  }

  return status;

#else // SLI_PSA_DRIVER_FEATURE_HMAC || SLI_PSA_DRIVER_FEATURE_CMAC

  (void)operation;
  (void)attributes;
  (void)key_buffer;
  (void)key_buffer_size;
  (void)alg;

  return PSA_ERROR_NOT_SUPPORTED;

#endif // SLI_PSA_DRIVER_FEATURE_HMAC || SLI_PSA_DRIVER_FEATURE_CMAC
}

psa_status_t sli_hostcrypto_transparent_mac_verify_setup(sli_hostcrypto_transparent_mac_operation_t *operation,
                                                         const psa_key_attributes_t *attributes,
                                                         const uint8_t *key_buffer,
                                                         size_t key_buffer_size,
                                                         psa_algorithm_t alg)
{
  // Since the PSA Crypto core exposes the verify functionality of the drivers without
  // actually implementing the fallback to 'sign' when the driver doesn't support verify,
  // we need to do this ourselves for the time being.
  return sli_hostcrypto_transparent_mac_sign_setup(operation,
                                                   attributes,
                                                   key_buffer,
                                                   key_buffer_size,
                                                   alg);
}

#if defined(SLI_PSA_DRIVER_FEATURE_CMAC)
static psa_status_t sli_hostcrypto_cmac_create(
  sli_hostcrypto_transparent_mac_operation_t *operation)
{
  if (operation == NULL) {
    return PSA_ERROR_INVALID_ARGUMENT;
  }

  int sx_status = SX_ERR_UNITIALIZED_OBJ;
  if (sli_sxsymcrypt_lock_cryptomaster_selection(
        SLI_SXSYMCRYPT_CRYPTOMASTER_HOSTSYMCRYPTO, false)) {
    return PSA_ERROR_SERVICE_FAILURE;
  }

  sx_status = sx_mac_create_aescmac(&operation->cmac.cmac_ctx, &operation->cmac.key_ref);

  if (sli_sxsymcrypt_unlock_cryptomaster_selection()) {
    return PSA_ERROR_SERVICE_FAILURE;
  }

  if (sx_status != SX_OK) {
    return PSA_ERROR_HARDWARE_FAILURE;
  }

  return PSA_SUCCESS;
}
#endif // SLI_PSA_DRIVER_FEATURE_CMAC

psa_status_t sli_hostcrypto_transparent_mac_update(
  sli_hostcrypto_transparent_mac_operation_t *operation,
  const uint8_t *input,
  size_t input_length)
{
#if defined(SLI_PSA_DRIVER_FEATURE_HMAC) || defined(SLI_PSA_DRIVER_FEATURE_CMAC)

  if (operation == NULL
      || input == NULL) {
    return PSA_ERROR_INVALID_ARGUMENT;
  }

  if (input_length == 0 ) {
    return PSA_SUCCESS;
  }

#if defined(SLI_PSA_DRIVER_FEATURE_HMAC)
  if (PSA_ALG_IS_HMAC(operation->hmac.alg)) {
    return sli_hostcrypto_transparent_hash_update(
      &operation->hmac.hash_ctx,
      input,
      input_length);
  } else
#endif // SLI_PSA_DRIVER_FEATURE_HMAC

#if defined(SLI_PSA_DRIVER_FEATURE_CMAC)
  if (PSA_ALG_FULL_LENGTH_MAC(operation->cmac.alg) == PSA_ALG_CMAC) {
    psa_status_t psa_status = PSA_ERROR_CORRUPTION_DETECTED;
    int sx_status = SX_ERR_UNITIALIZED_OBJ;
    size_t current_block_len = 0;

    // if the potential last block include data
    // we need to process them first
    current_block_len = operation->cmac.current_block_len;
    if (current_block_len) {
      size_t bytes_to_boundary = 16 - current_block_len;
      // if the total of bytes is smaller than a block, just copy and return
      // else fill up the potential last block
      if (input_length < bytes_to_boundary) {
        memcpy(&operation->cmac.current_block[current_block_len], input, input_length);
        operation->cmac.current_block_len = current_block_len + input_length;
        return PSA_SUCCESS;
      } else {
        // fill up the potential last block
        memcpy(&operation->cmac.current_block[current_block_len], input, bytes_to_boundary);
        operation->cmac.current_block_len = 16;
        input_length -= bytes_to_boundary;
        input += bytes_to_boundary;
      }

      // if there is more input data, the potential last block is not
      // the last block, which means we can process it now,
      if (input_length) {
        // Execute CMAC operation on the single context block
        if (operation->cmac.cmac_ctx.dma.dmamem.cfg & SLI_HOSTCRYPTO_CMAC_CTX_SAVE) {
          if (sli_sxsymcrypt_lock_cryptomaster_selection(
                SLI_SXSYMCRYPT_CRYPTOMASTER_HOSTSYMCRYPTO, false)) {
            return PSA_ERROR_SERVICE_FAILURE;
          }
          sx_status = sx_mac_resume_state(&operation->cmac.cmac_ctx);
          if (sli_sxsymcrypt_unlock_cryptomaster_selection()) {
            return PSA_ERROR_SERVICE_FAILURE;
          }
          if (sx_status != SX_OK) {
            return PSA_ERROR_HARDWARE_FAILURE;
          }
        }
        if (!operation->cmac.cmac_ctx.dma.regs
            && !(operation->cmac.cmac_ctx.dma.dmamem.cfg & SLI_HOSTCRYPTO_CMAC_CTX_SAVE)) {
          psa_status = sli_hostcrypto_cmac_create(operation);
          if (psa_status != PSA_SUCCESS) {
            return psa_status;
          }
        }
        sx_status = sx_mac_feed(&operation->cmac.cmac_ctx,
                                (const char *)operation->cmac.current_block,
                                operation->cmac.current_block_len);
        if (sx_status != SX_OK) {
          return PSA_ERROR_HARDWARE_FAILURE;
        }
        sx_status = sx_mac_save_state(&operation->cmac.cmac_ctx);
        if (sx_status != SX_OK) {
          return PSA_ERROR_HARDWARE_FAILURE;
        }
        sx_status = sx_mac_wait(&operation->cmac.cmac_ctx);
        if (sx_status != SX_OK) {
          return PSA_ERROR_HARDWARE_FAILURE;
        }
        operation->cmac.current_block_len = 0;
      }
    }

    // Process complete input blocks
    if (input_length > 16) {
      // Calculate the number of bytes in complete blocks to process.
      // If the last block is complete we need to postpone processing it
      // since it may be the last block which should go to sx_mac_generate.
      size_t bytes_to_copy = (input_length - 1) & ~0xFUL;

      if (bytes_to_copy) {
        // Execute CMAC operation on the blocks
        if (operation->cmac.cmac_ctx.dma.dmamem.cfg & SLI_HOSTCRYPTO_CMAC_CTX_SAVE) {
          if (sli_sxsymcrypt_lock_cryptomaster_selection(
                SLI_SXSYMCRYPT_CRYPTOMASTER_HOSTSYMCRYPTO, false)) {
            return PSA_ERROR_SERVICE_FAILURE;
          }
          sx_status = sx_mac_resume_state(&operation->cmac.cmac_ctx);
          if (sli_sxsymcrypt_unlock_cryptomaster_selection()) {
            return PSA_ERROR_SERVICE_FAILURE;
          }
          if (sx_status != SX_OK) {
            return PSA_ERROR_HARDWARE_FAILURE;
          }
        }
        if (!operation->cmac.cmac_ctx.dma.regs
            && !(operation->cmac.cmac_ctx.dma.dmamem.cfg & SLI_HOSTCRYPTO_CMAC_CTX_SAVE)) {
          psa_status = sli_hostcrypto_cmac_create(operation);
          if (psa_status != PSA_SUCCESS) {
            return psa_status;
          }
        }
        sx_status = sx_mac_feed(&operation->cmac.cmac_ctx,
                                (const char *)input,
                                bytes_to_copy);
        if (sx_status != SX_OK) {
          return PSA_ERROR_HARDWARE_FAILURE;
        }
        sx_status = sx_mac_save_state(&operation->cmac.cmac_ctx);
        if (sx_status != SX_OK) {
          return PSA_ERROR_HARDWARE_FAILURE;
        }
        sx_status = sx_mac_wait(&operation->cmac.cmac_ctx);
        if (sx_status != SX_OK) {
          return PSA_ERROR_HARDWARE_FAILURE;
        }

        input_length -= bytes_to_copy;
        input += bytes_to_copy;
      }
    }

    // Store remaining input bytes
    if (input_length) {
      memcpy(operation->cmac.current_block, input, input_length);
      operation->cmac.current_block_len = input_length;
    }

    return PSA_SUCCESS;
  } else
#endif // SLI_PSA_DRIVER_FEATURE_CMAC
  {
    return PSA_ERROR_BAD_STATE;
  }

#else // SLI_PSA_DRIVER_FEATURE_HMAC || SLI_PSA_DRIVER_FEATURE_CMAC

  (void)operation;
  (void)input;
  (void)input_length;

  return PSA_ERROR_NOT_SUPPORTED;

#endif // SLI_PSA_DRIVER_FEATURE_HMAC || SLI_PSA_DRIVER_FEATURE_CMAC
}

psa_status_t sli_hostcrypto_transparent_mac_sign_finish(
  sli_hostcrypto_transparent_mac_operation_t *operation,
  uint8_t *mac,
  size_t mac_size,
  size_t *mac_length)
{
#if defined(SLI_PSA_DRIVER_FEATURE_HMAC) || defined(SLI_PSA_DRIVER_FEATURE_CMAC)

  if (operation == NULL
      || mac == NULL
      || mac_length == NULL) {
    return PSA_ERROR_INVALID_ARGUMENT;
  }

  psa_status_t psa_status = PSA_ERROR_CORRUPTION_DETECTED;

  *mac_length = 0;

#if defined(SLI_PSA_DRIVER_FEATURE_HMAC)
  if (PSA_ALG_IS_HMAC(operation->hmac.alg)) {
    uint8_t buffer[128 + 64];
    size_t olen;
    size_t block_len = PSA_HASH_BLOCK_LENGTH(operation->hmac.alg);

    // Return error if hash alg is SHA3
    if (PSA_ALG_HMAC_GET_HASH(operation->hmac.alg) & 0x00000010) {
      return PSA_ERROR_NOT_SUPPORTED;
    }

    // Construct outer hash input from opad and hash result
    memcpy(buffer, operation->hmac.opad, block_len);
    memset(operation->hmac.opad, 0, block_len);

    psa_status = sli_hostcrypto_transparent_hash_finish(
      &operation->hmac.hash_ctx,
      &buffer[block_len],
      sizeof(buffer) - block_len,
      &olen);

    if (psa_status != PSA_SUCCESS) {
      return psa_status;
    }

    // Calculate HMAC
    psa_status = sli_hostcrypto_transparent_hash_compute(
      PSA_ALG_HMAC_GET_HASH(operation->hmac.alg),
      buffer,
      block_len + olen,
      buffer,
      sizeof(buffer),
      &olen);
    if (psa_status != PSA_SUCCESS) {
      return psa_status;
    }

    // Copy out a potentially truncated HMAC
    size_t requested_length = PSA_MAC_TRUNCATED_LENGTH(operation->hmac.alg);
    if (requested_length == 0) {
      requested_length = olen;
    }

    if (requested_length > mac_size) {
      memset(buffer, 0, sizeof(buffer));
      return PSA_ERROR_BUFFER_TOO_SMALL;
    }

    memcpy(mac, buffer, requested_length);
    *mac_length = requested_length;
    memset(buffer, 0, sizeof(buffer));
    return PSA_SUCCESS;
  } else
#endif // SLI_PSA_DRIVER_FEATURE_HMAC

  // Check algorithm and store if supported
#if defined(SLI_PSA_DRIVER_FEATURE_CMAC)
  if (PSA_ALG_FULL_LENGTH_MAC(operation->cmac.alg) == PSA_ALG_CMAC) {
    // Check output size
    int sx_status = SX_ERR_UNITIALIZED_OBJ;
    size_t requested_length = PSA_MAC_TRUNCATED_LENGTH(operation->cmac.alg);
    if (requested_length == 0) {
      requested_length = SLI_HOSTCRYPTO_CMAC_MAC_SIZE;
    } else if (requested_length > SLI_HOSTCRYPTO_CMAC_MAC_SIZE) {
      return PSA_ERROR_INVALID_ARGUMENT;
    }

    if (mac_size < requested_length) {
      return PSA_ERROR_BUFFER_TOO_SMALL;
    }

    if (operation->cmac.cmac_ctx.dma.dmamem.cfg & SLI_HOSTCRYPTO_CMAC_CTX_SAVE) {
      if (sli_sxsymcrypt_lock_cryptomaster_selection(
            SLI_SXSYMCRYPT_CRYPTOMASTER_HOSTSYMCRYPTO, false)) {
        return PSA_ERROR_SERVICE_FAILURE;
      }
      sx_status = sx_mac_resume_state(&operation->cmac.cmac_ctx);
      if (sli_sxsymcrypt_unlock_cryptomaster_selection()) {
        return PSA_ERROR_SERVICE_FAILURE;
      }
      if (sx_status != SX_OK) {
        return PSA_ERROR_HARDWARE_FAILURE;
      }
    }
    if (!operation->cmac.cmac_ctx.dma.regs
        && !(operation->cmac.cmac_ctx.dma.dmamem.cfg & SLI_HOSTCRYPTO_CMAC_CTX_SAVE)) {
      psa_status = sli_hostcrypto_cmac_create(operation);
      if (psa_status != PSA_SUCCESS) {
        return psa_status;
      }
    }

    if (operation->cmac.current_block_len) {
      sx_status = sx_mac_feed(&operation->cmac.cmac_ctx,
                              (const char *)operation->cmac.current_block,
                              operation->cmac.current_block_len);
      if (sx_status != SX_OK) {
        return PSA_ERROR_HARDWARE_FAILURE;
      }
    }

    uint8_t output_buffer[SLI_HOSTCRYPTO_CMAC_MAC_SIZE];
    sx_status = sx_mac_generate(&operation->cmac.cmac_ctx, (char *)output_buffer);
    if (sx_status != SX_OK) {
      return PSA_ERROR_HARDWARE_FAILURE;
    }
    sx_status = sx_mac_wait(&operation->cmac.cmac_ctx);
    if (sx_status != SX_OK) {
      return PSA_ERROR_HARDWARE_FAILURE;
    }
    // Copy the requested number of bytes (max 16 for CMAC) to the user buffer.
    memcpy(mac, output_buffer, requested_length);
    memset(output_buffer, 0, sizeof(output_buffer));
    *mac_length = requested_length;

    return PSA_SUCCESS;
  } else
#endif // SLI_PSA_DRIVER_FEATURE_CMAC
  {
    return PSA_ERROR_NOT_SUPPORTED;
  }

#else // SLI_PSA_DRIVER_FEATURE_HMAC || SLI_PSA_DRIVER_FEATURE_CMAC

  (void)operation;
  (void)mac;
  (void)mac_size;
  (void)mac_length;

  return PSA_ERROR_NOT_SUPPORTED;

#endif // SLI_PSA_DRIVER_FEATURE_HMAC || SLI_PSA_DRIVER_FEATURE_CMAC
}

psa_status_t sli_hostcrypto_transparent_mac_verify_finish(
  sli_hostcrypto_transparent_mac_operation_t *operation,
  const uint8_t *mac,
  size_t mac_length)
{
  // Since the PSA Crypto core exposes the verify functionality of the drivers without
  // actually implementing the fallback to 'sign' when the driver doesn't support verify,
  // we need to do this ourselves for the time being.
  uint8_t calculated_mac[PSA_MAC_MAX_SIZE] = { 0 };
  size_t calculated_length = PSA_MAC_MAX_SIZE;

  psa_status_t status = sli_hostcrypto_transparent_mac_sign_finish(
    operation,
    calculated_mac, sizeof(calculated_mac), &calculated_length);
  if (status != PSA_SUCCESS) {
    return status;
  }

  if (mac_length > sizeof(calculated_mac)) {
    return PSA_ERROR_INVALID_ARGUMENT;
  }

  if (sli_psa_safer_memcmp(mac, calculated_mac, mac_length) != 0) {
    status = PSA_ERROR_INVALID_SIGNATURE;
  } else {
    status = PSA_SUCCESS;
  }

  memset(calculated_mac, 0, sizeof(calculated_mac));
  return status;
}

psa_status_t sli_hostcrypto_transparent_mac_abort(
  sli_hostcrypto_transparent_mac_operation_t *operation)
{
#if defined(SLI_PSA_DRIVER_FEATURE_HMAC) || defined(SLI_PSA_DRIVER_FEATURE_CMAC)

  // There's no state in hardware that we need to preserve, so zeroing out the context suffices.
  if (operation == NULL) {
    return PSA_ERROR_INVALID_ARGUMENT;
  }

  memset(operation, 0, sizeof(*operation));
  return PSA_SUCCESS;

#else // SLI_PSA_DRIVER_FEATURE_HMAC || SLI_PSA_DRIVER_FEATURE_CMAC

  (void)operation;

  return PSA_ERROR_NOT_SUPPORTED;

#endif // SLI_PSA_DRIVER_FEATURE_HMAC || SLI_PSA_DRIVER_FEATURE_CMAC
}

#endif // defined(SLI_MBEDTLS_DEVICE_HC)
