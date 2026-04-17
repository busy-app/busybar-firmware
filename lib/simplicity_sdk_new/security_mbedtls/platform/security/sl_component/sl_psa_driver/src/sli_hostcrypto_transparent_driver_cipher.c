/***************************************************************************//**
 * @file
 * @brief PSA Crypto cipher drivers for the HostCryptoSubSystem.
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
#include "sxsymcrypt/aes.h"
#include "sxsymcrypt/blkcipher.h"
#include "sxsymcrypt/keyref.h"
#include "sxsymcrypt/statuscodes.h"

#include <string.h>

// Register value for context saving
#define SLI_HOSTCRYPTO_BLKCIPHER_CTX_SAVE              (1u << 5)

// AES IV size
#define SLI_HOSTCRYPTO_AES_IV_SIZE                     16
// -----------------------------------------------------------------------------
// Static Helper Functions

#if (defined(SLI_PSA_DRIVER_FEATURE_AES)                \
  && (defined(SLI_PSA_DRIVER_FEATURE_AES_ECB)           \
  || defined(SLI_PSA_DRIVER_FEATURE_AES_CTR)            \
  || defined(SLI_PSA_DRIVER_FEATURE_AES_CFB)            \
  || defined(SLI_PSA_DRIVER_FEATURE_AES_OFB)            \
  || defined(SLI_PSA_DRIVER_FEATURE_AES_CBC_NO_PADDING) \
  || defined(SLI_PSA_DRIVER_FEATURE_AES_CBC_PKCS7)))    \
  || defined(SLI_PSA_DRIVER_FEATURE_AES_CCM_STAR_NO_TAG)
static psa_status_t driver_can_handle(const psa_key_attributes_t *attributes,
                                      psa_algorithm_t alg,
                                      size_t key_buffer_size)
{
  switch (alg) {
#if defined(SLI_PSA_DRIVER_FEATURE_AES_ECB)
    case PSA_ALG_ECB_NO_PADDING:
#endif
#if defined(MBEDTLS_PSA_CRYPTO_C)
#if defined(SLI_PSA_DRIVER_FEATURE_AES_CTR)
    case PSA_ALG_CTR:
#endif
#if defined(SLI_PSA_DRIVER_FEATURE_AES_CFB)
    case PSA_ALG_CFB:
#endif
#if defined(SLI_PSA_DRIVER_FEATURE_AES_OFB)
    case PSA_ALG_OFB:
#endif
#if defined(SLI_PSA_DRIVER_FEATURE_AES_CBC_NO_PADDING)
    case PSA_ALG_CBC_NO_PADDING:
#endif
#if defined(SLI_PSA_DRIVER_FEATURE_AES_CBC_PKCS7)
    case PSA_ALG_CBC_PKCS7:
#endif
#if defined(SLI_PSA_DRIVER_FEATURE_AES_CCM_STAR_NO_TAG)
    case PSA_ALG_CCM_STAR_NO_TAG:
#endif
#endif // MBEDTLS_PSA_CRYPTO_C
    if (psa_get_key_type(attributes) != PSA_KEY_TYPE_AES) {
      return PSA_ERROR_NOT_SUPPORTED;
    }
    psa_key_location_t location =
      PSA_KEY_LIFETIME_GET_LOCATION(psa_get_key_lifetime(attributes));

    if (location == PSA_KEY_LOCATION_LOCAL_STORAGE) {
      size_t key_bits = psa_get_key_bits(attributes);
      if (key_buffer_size < (key_bits / 8)
          || (key_bits != 128 && key_bits != 192 && key_bits != 256)) {
        return PSA_ERROR_INVALID_ARGUMENT;
      }
#if defined(SLI_PSA_DRIVER_FEATURE_KSU)
    } else if (location == SL_PSA_KEY_LOCATION_KSU_0) {
      if (key_buffer_size < sizeof(uint8_t)) {
        return PSA_ERROR_INVALID_ARGUMENT;
      }
#endif // SLI_PSA_DRIVER_FEATURE_KSU
    } else {
      return PSA_ERROR_INVALID_ARGUMENT;
    }
    break;
    default:
      (void) attributes;
      return PSA_ERROR_NOT_SUPPORTED;
  }

  return PSA_SUCCESS;
}
#endif

// -----------------------------------------------------------------------------
// Entry Point Definitions

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
  size_t *output_length)
{
#if (defined(SLI_PSA_DRIVER_FEATURE_AES)                \
  && (defined(SLI_PSA_DRIVER_FEATURE_AES_ECB)           \
  || defined(SLI_PSA_DRIVER_FEATURE_AES_CTR)            \
  || defined(SLI_PSA_DRIVER_FEATURE_AES_CFB)            \
  || defined(SLI_PSA_DRIVER_FEATURE_AES_OFB)            \
  || defined(SLI_PSA_DRIVER_FEATURE_AES_CBC_NO_PADDING) \
  || defined(SLI_PSA_DRIVER_FEATURE_AES_CBC_PKCS7)))    \
  || defined(SLI_PSA_DRIVER_FEATURE_AES_CCM_STAR_NO_TAG)

  // Input validation
  if (attributes == NULL
      || key_buffer == NULL
      || key_buffer_size == 0
      || (input == NULL && input_length > 0)
      || (iv == NULL && iv_length > 0)
      || (output == NULL && output_size > 0)
      || output_length == NULL) {
    return PSA_ERROR_INVALID_ARGUMENT;
  }

  psa_status_t psa_status = driver_can_handle(attributes, alg, key_buffer_size);
  if (psa_status != PSA_SUCCESS) {
    return psa_status;
  }

  // 0-length encrypt/decrypt is allowed according to the unit tests in PSA
  if (input_length == 0) {
    *output_length = 0;
    return PSA_SUCCESS;
  }

  // Our drivers only support full or no overlap between input and output
  // buffers. So in the case of partial overlap, copy the input buffer into
  // the output buffer and process it in place as if the buffers fully
  // overlapped.
  if ((output > input) && (output < (input + input_length))) {
    // Sanity check before copying. Some ciphers have a stricter requirement
    // than this (if an IV is included), but no ciphers will have an output
    // smaller than the input.
    if (output_size < input_length) {
      return PSA_ERROR_INVALID_ARGUMENT;
    }

    memmove(output, input, input_length);
    input = output;
  }

  // Load key
  struct sxkeyref key_ref;
  psa_status = sli_hostcrypto_load_key(&key_ref, attributes, key_buffer);
  if (psa_status != PSA_SUCCESS) {
    return psa_status;
  }

  // Create and configure operation context
  struct sxblkcipher cipher;
  int sx_status = SX_ERR_UNITIALIZED_OBJ;
  if (sli_sxsymcrypt_lock_cryptomaster_selection(
        SLI_SXSYMCRYPT_CRYPTOMASTER_HOSTSYMCRYPTO, false)) {
    return PSA_ERROR_SERVICE_FAILURE;
  }
  switch (alg) {
#if defined(SLI_PSA_DRIVER_FEATURE_AES_ECB)
    case PSA_ALG_ECB_NO_PADDING:
      if (input_length > output_size) {
        psa_status = PSA_ERROR_BUFFER_TOO_SMALL;
        break;
      }
      if (input_length % 16 != 0) {
        psa_status = PSA_ERROR_INVALID_ARGUMENT;
        break;
      }
      sx_status = sx_blkcipher_create_aesecb_enc(&cipher, &key_ref);
      break;
#endif
#if defined(MBEDTLS_PSA_CRYPTO_C)
#if defined(SLI_PSA_DRIVER_FEATURE_AES_CCM_STAR_NO_TAG)
    case PSA_ALG_CCM_STAR_NO_TAG:     // Explicit Fallthrough
#endif
#if defined(SLI_PSA_DRIVER_FEATURE_AES_CTR) \
    || defined(SLI_PSA_DRIVER_FEATURE_AES_CCM_STAR_NO_TAG)
    case PSA_ALG_CTR:
      uint8_t iv_buf[16] = { 0 };
#if defined(SLI_PSA_DRIVER_FEATURE_AES_CCM_STAR_NO_TAG)
      if (alg == PSA_ALG_CCM_STAR_NO_TAG) {
        if (iv_length != 13) {
          psa_status = PSA_ERROR_INVALID_ARGUMENT;
          break;
        }
        // AES-CCM*-no-tag is basically AES-CTR with preformatted IV
        iv_buf[0] = 1;
        memcpy(&iv_buf[1], iv, 13);
        iv_buf[14] = 0;
        iv_buf[15] = 1;
      } else
#endif // SLI_PSA_DRIVER_FEATURE_AES_CCM_STAR_NO_TAG
      {
        if (iv_length != SLI_HOSTCRYPTO_AES_IV_SIZE) {
          psa_status = PSA_ERROR_INVALID_ARGUMENT;
          break;
        }
        memcpy(iv_buf, iv, SLI_HOSTCRYPTO_AES_IV_SIZE);
      }
      if (input_length > output_size) {
        psa_status = PSA_ERROR_BUFFER_TOO_SMALL;
        break;
      }
      sx_status = sx_blkcipher_create_aesctr_enc(&cipher,
                                                 &key_ref,
                                                 (const char *) iv_buf);
      break;
#endif // SLI_PSA_DRIVER_FEATURE_AES_CTR || SLI_PSA_DRIVER_FEATURE_AES_CCM_STAR_NO_TAG

#if defined(SLI_PSA_DRIVER_FEATURE_AES_CBC_NO_PADDING) \
    || defined(SLI_PSA_DRIVER_FEATURE_AES_CBC_PKCS7)
    case PSA_ALG_CBC_NO_PADDING:
      if (input_length % 16 != 0) {
        psa_status = PSA_ERROR_INVALID_ARGUMENT;
        break;
      }
    // Fall through
    case PSA_ALG_CBC_PKCS7:
      if (iv_length != SLI_HOSTCRYPTO_AES_IV_SIZE) {
        psa_status = PSA_ERROR_INVALID_ARGUMENT;
        break;
      }
      if (alg == PSA_ALG_CBC_NO_PADDING) {
        if (output_size < input_length) {
          psa_status = PSA_ERROR_BUFFER_TOO_SMALL;
        }
      } else {
        if (output_size < (input_length & ~0xF) + 16) {
          psa_status = PSA_ERROR_BUFFER_TOO_SMALL;
        }
      }
      sx_status = sx_blkcipher_create_aescbc_enc(&cipher,
                                                 &key_ref,
                                                 (const char *) iv);
      break;
#endif
#if defined(SLI_PSA_DRIVER_FEATURE_AES_CFB)
    case PSA_ALG_CFB:
      if (iv_length != SLI_HOSTCRYPTO_AES_IV_SIZE) {
        psa_status = PSA_ERROR_INVALID_ARGUMENT;
        break;
      }
      if (input_length > output_size) {
        psa_status = PSA_ERROR_BUFFER_TOO_SMALL;
        break;
      }
      sx_status = sx_blkcipher_create_aescfb_enc(&cipher,
                                                 &key_ref,
                                                 (const char *) iv);
      break;
#endif
#if defined(SLI_PSA_DRIVER_FEATURE_AES_OFB)
    case PSA_ALG_OFB:
      if (iv_length != SLI_HOSTCRYPTO_AES_IV_SIZE) {
        psa_status = PSA_ERROR_INVALID_ARGUMENT;
        break;
      }
      if (input_length > output_size) {
        psa_status = PSA_ERROR_BUFFER_TOO_SMALL;
        break;
      }
      sx_status = sx_blkcipher_create_aesofb_enc(&cipher,
                                                 &key_ref,
                                                 (const char *) iv);
      break;
#endif
#endif //MBEDTLS_PSA_CRYPTO_C
    default:
      psa_status = PSA_ERROR_NOT_SUPPORTED;
  }
  if (sli_sxsymcrypt_unlock_cryptomaster_selection()) {
    psa_status = PSA_ERROR_SERVICE_FAILURE;
  }
  if (psa_status != PSA_SUCCESS) {
    return psa_status;
  }
  if (sx_status != SX_OK) {
    return PSA_ERROR_HARDWARE_FAILURE;
  }
#if defined(SLI_PSA_DRIVER_FEATURE_AES_CBC_PKCS7)
  if (alg == PSA_ALG_CBC_PKCS7) {
    uint8_t pad_block[16];
    // copy last non-block input to padding block
    memcpy(pad_block, &input[input_length & ~0xF], input_length & 0xF);
    // Add PKCS7 padding.
    memset(&pad_block[input_length & 0xF],
           16 - (input_length & 0xF),
           16 - (input_length & 0xF));
    if ((input_length & ~0xF) > 0) {
      sx_status = sx_blkcipher_crypt(&cipher,
                                     (const char *) input,
                                     input_length & ~0xF,
                                     (char *) output);
      if (sx_status != SX_OK) {
        return PSA_ERROR_HARDWARE_FAILURE;
      }
    }
    sx_status = sx_blkcipher_crypt(&cipher,
                                   (const char *) pad_block,
                                   16,
                                   (char *) &output[input_length & ~0xF]);
    if (sx_status != SX_OK) {
      return PSA_ERROR_HARDWARE_FAILURE;
    }
    sx_status = sx_blkcipher_run(&cipher);
    if (sx_status != SX_OK) {
      return PSA_ERROR_HARDWARE_FAILURE;
    }
    sx_status = sx_blkcipher_wait(&cipher);
    if (sx_status != SX_OK) {
      return PSA_ERROR_HARDWARE_FAILURE;
    }
    *output_length = (input_length & ~0xF) + 16;
  } else
#endif
  {
    sx_status = sx_blkcipher_crypt(&cipher,
                                   (const char *) input,
                                   input_length,
                                   (char *) output);
    if (sx_status != SX_OK) {
      return PSA_ERROR_HARDWARE_FAILURE;
    }
    sx_status = sx_blkcipher_run(&cipher);
    if (sx_status != SX_OK) {
      return PSA_ERROR_HARDWARE_FAILURE;
    }
    sx_status = sx_blkcipher_wait(&cipher);
    if (sx_status != SX_OK) {
      return PSA_ERROR_HARDWARE_FAILURE;
    }
    *output_length = input_length;
  }

  return PSA_SUCCESS;
#else // SLI_PSA_DRIVER_FEATURE_AES_* && SLI_PSA_DRIVER_FEATURE_AES

  (void) attributes;
  (void) key_buffer;
  (void) key_buffer_size;
  (void) alg;
  (void) iv;
  (void) iv_length;
  (void) input;
  (void) input_length;
  (void) output;
  (void) output_size;
  (void) output_length;

  return PSA_ERROR_NOT_SUPPORTED;

#endif // SLI_PSA_DRIVER_FEATURE_AES_* && SLI_PSA_DRIVER_FEATURE_AES
}

psa_status_t sli_hostcrypto_transparent_cipher_decrypt(
  const psa_key_attributes_t *attributes,
  const uint8_t *key_buffer,
  size_t key_buffer_size,
  psa_algorithm_t alg,
  const uint8_t *input,
  size_t input_length,
  uint8_t *output,
  size_t output_size,
  size_t *output_length)
{
#if (defined(SLI_PSA_DRIVER_FEATURE_AES)                \
  && (defined(SLI_PSA_DRIVER_FEATURE_AES_ECB)           \
  || defined(SLI_PSA_DRIVER_FEATURE_AES_CTR)            \
  || defined(SLI_PSA_DRIVER_FEATURE_AES_CFB)            \
  || defined(SLI_PSA_DRIVER_FEATURE_AES_OFB)            \
  || defined(SLI_PSA_DRIVER_FEATURE_AES_CBC_NO_PADDING) \
  || defined(SLI_PSA_DRIVER_FEATURE_AES_CBC_PKCS7)))    \
  || defined(SLI_PSA_DRIVER_FEATURE_AES_CCM_STAR_NO_TAG)

  // Input validation
  if (attributes == NULL
      || key_buffer == NULL
      || key_buffer_size == 0
      || (input == NULL && input_length > 0)
      || (output == NULL && output_size > 0)
      || output_length == NULL) {
    return PSA_ERROR_INVALID_ARGUMENT;
  }

  psa_status_t psa_status = driver_can_handle(attributes, alg, key_buffer_size);
  if (psa_status != PSA_SUCCESS) {
    return psa_status;
  }

  // 0-length encrypt/decrypt is allowed according to the unit tests in PSA
  if (input_length == 0) {
    *output_length = 0;
    return PSA_SUCCESS;
  }

  // Only passing an IV should also be OK (all modes use an IV except ECB).
  if ((input_length == SLI_HOSTCRYPTO_AES_IV_SIZE)
      && (alg != PSA_ALG_ECB_NO_PADDING)) {
    *output_length = 0;
    return PSA_SUCCESS;
  }

  // Our drivers only support full or no overlap between input and output
  // buffers. So in the case of partial overlap, copy the input buffer into
  // the output buffer and process it in place as if the buffers fully
  // overlapped.
  if ((output > input) && (output < (input + input_length))) {
    // Sanity check before copying. Some ciphers have a stricter requirement
    // than this (if an IV is included), but no ciphers will have an output
    // smaller than the input.
    if (output_size < input_length) {
      return PSA_ERROR_INVALID_ARGUMENT;
    }

    memmove(output, input, input_length);
    input = output;
  }

#if defined(SLI_PSA_DRIVER_FEATURE_AES_CTR)             \
  || defined(SLI_PSA_DRIVER_FEATURE_AES_CFB)            \
  || defined(SLI_PSA_DRIVER_FEATURE_AES_OFB)            \
  || defined(SLI_PSA_DRIVER_FEATURE_AES_CBC_NO_PADDING) \
  || defined(SLI_PSA_DRIVER_FEATURE_AES_CBC_PKCS7)      \
  || defined(SLI_PSA_DRIVER_FEATURE_AES_CCM_STAR_NO_TAG)
  uint8_t iv[SLI_HOSTCRYPTO_AES_IV_SIZE];
#if defined(SLI_PSA_DRIVER_FEATURE_AES_CCM_STAR_NO_TAG)
  if (alg == PSA_ALG_CCM_STAR_NO_TAG) {
    // AES-CCM*-no-tag is basically AES-CTR with preformatted IV
    iv[0] = 1;
    memcpy(&iv[1], input, 13);
    iv[14] = 0;
    iv[15] = 1;
    input += 13;
    input_length -= 13;
  } else
#endif   // SLI_PSA_DRIVER_FEATURE_AES_CCM_STAR_NO_TAG
  if (alg != PSA_ALG_ECB_NO_PADDING) {
    memcpy(iv, input, SLI_HOSTCRYPTO_AES_IV_SIZE);
    input += SLI_HOSTCRYPTO_AES_IV_SIZE;
    input_length -= SLI_HOSTCRYPTO_AES_IV_SIZE;
  }
#endif

  // Load key
  struct sxkeyref key_ref;
  psa_status = sli_hostcrypto_load_key(&key_ref, attributes, key_buffer);
  if (psa_status != PSA_SUCCESS) {
    return psa_status;
  }

  // Create and configure operation context
  struct sxblkcipher cipher;
  int sx_status = SX_ERR_UNITIALIZED_OBJ;
  if (sli_sxsymcrypt_lock_cryptomaster_selection(
        SLI_SXSYMCRYPT_CRYPTOMASTER_HOSTSYMCRYPTO, false)) {
    return PSA_ERROR_SERVICE_FAILURE;
  }
  switch (alg) {
#if defined(SLI_PSA_DRIVER_FEATURE_AES_ECB)
    case PSA_ALG_ECB_NO_PADDING:
      if (input_length % 16 != 0) {
        psa_status = PSA_ERROR_INVALID_ARGUMENT;
        break;
      }
      if (input_length > output_size) {
        psa_status = PSA_ERROR_BUFFER_TOO_SMALL;
        break;
      }
      sx_status = sx_blkcipher_create_aesecb_dec(&cipher, &key_ref);
      break;
#endif
#if defined(SLI_PSA_DRIVER_FEATURE_AES_CCM_STAR_NO_TAG)
    case PSA_ALG_CCM_STAR_NO_TAG:     // Explicit fallthrough
#endif   // SLI_PSA_DRIVER_FEATURE_AES_CCM_STAR_NO_TAG
#if defined(SLI_PSA_DRIVER_FEATURE_AES_CTR) \
    || defined(SLI_PSA_DRIVER_FEATURE_AES_CCM_STAR_NO_TAG)
    case PSA_ALG_CTR:
  #if defined(SLI_PSA_DRIVER_FEATURE_AES_CCM_STAR_NO_TAG)
      if (alg == PSA_ALG_CCM_STAR_NO_TAG) {
        if (output_size < input_length - 13) {
          return PSA_ERROR_BUFFER_TOO_SMALL;
        }
      } else
  #endif
      {
        if (input_length > output_size) {
          psa_status = PSA_ERROR_BUFFER_TOO_SMALL;
          break;
        }
      }
      sx_status = sx_blkcipher_create_aesctr_dec(&cipher,
                                                 &key_ref,
                                                 (const char *) iv);
      break;
#endif
#if defined(SLI_PSA_DRIVER_FEATURE_AES_CBC_NO_PADDING) \
    || defined(SLI_PSA_DRIVER_FEATURE_AES_CBC_PKCS7)
    case PSA_ALG_CBC_NO_PADDING:
      if (input_length > output_size) {
        psa_status = PSA_ERROR_BUFFER_TOO_SMALL;
        break;
      }
    // Fall through
    case PSA_ALG_CBC_PKCS7:
      if (input_length % 16 != 0) {
        psa_status = PSA_ERROR_INVALID_ARGUMENT;
        break;
      }
      if (alg == PSA_ALG_CBC_PKCS7) {
        // Check output has enough room for at least n-1 blocks.
        if (input_length < 16
            || output_size < (input_length - 16)) {
          psa_status = PSA_ERROR_BUFFER_TOO_SMALL;
          break;
        }
      }
      sx_status = sx_blkcipher_create_aescbc_dec(&cipher,
                                                 &key_ref,
                                                 (const char *) iv);
      break;
#endif
#if defined(SLI_PSA_DRIVER_FEATURE_AES_CFB)
    case PSA_ALG_CFB:
      if (input_length > output_size) {
        psa_status = PSA_ERROR_BUFFER_TOO_SMALL;
        break;
      }
      sx_status = sx_blkcipher_create_aescfb_dec(&cipher,
                                                 &key_ref,
                                                 (const char *) iv);
      break;
#endif
#if defined(SLI_PSA_DRIVER_FEATURE_AES_OFB)
    case PSA_ALG_OFB:
      if (input_length > output_size) {
        psa_status = PSA_ERROR_BUFFER_TOO_SMALL;
        break;
      }
      sx_status = sx_blkcipher_create_aesofb_dec(&cipher,
                                                 &key_ref,
                                                 (const char *) iv);
      break;
#endif
    default:
      psa_status = PSA_ERROR_NOT_SUPPORTED;
  }
  if (sli_sxsymcrypt_unlock_cryptomaster_selection()) {
    return PSA_ERROR_SERVICE_FAILURE;
  }
  if (psa_status != PSA_SUCCESS) {
    return psa_status;
  }
  if (sx_status != SX_OK) {
    return PSA_ERROR_HARDWARE_FAILURE;
  }

#if defined(SLI_PSA_DRIVER_FEATURE_AES_CBC_PKCS7)
  if (alg == PSA_ALG_CBC_PKCS7) {
    // Decrypt input except the last block
    sx_status = sx_blkcipher_crypt(&cipher,
                                   (const char *) input,
                                   input_length - 16,
                                   (char *) output);
    if (sx_status != SX_OK) {
      return PSA_ERROR_HARDWARE_FAILURE;
    }
    uint8_t last_block[16];
    // Store last block to temporary buffer to be used in removing the padding.
    memcpy(last_block, &input[input_length - 16], 16);
    sx_status = sx_blkcipher_crypt(&cipher,
                                   (const char *) last_block,
                                   16,
                                   (char *) last_block);
    if (sx_status != SX_OK) {
      return PSA_ERROR_HARDWARE_FAILURE;
    }
    sx_status = sx_blkcipher_run(&cipher);
    if (sx_status != SX_OK) {
      return PSA_ERROR_HARDWARE_FAILURE;
    }
    sx_status = sx_blkcipher_wait(&cipher);
    if (sx_status != SX_OK) {
      return PSA_ERROR_HARDWARE_FAILURE;
    }
    // Check all padding bytes.
    size_t pad_bytes = 0;
    psa_status = sli_psa_validate_pkcs7_padding(last_block,
                                                16,
                                                &pad_bytes);
    if (psa_status != PSA_SUCCESS) {
      return psa_status;
    }

    if (output_size < (input_length - pad_bytes)) {
      return PSA_ERROR_BUFFER_TOO_SMALL;
    }

    // Copy non-padding bytes.
    memcpy(&output[input_length - 16], last_block, 16 - pad_bytes);

    *output_length = input_length - pad_bytes;
  } else
#endif
  {
    sx_status = sx_blkcipher_crypt(&cipher,
                                   (const char *) input,
                                   input_length,
                                   (char *) output);
    if (sx_status != SX_OK) {
      return PSA_ERROR_HARDWARE_FAILURE;
    }
    sx_status = sx_blkcipher_run(&cipher);
    if (sx_status != SX_OK) {
      return PSA_ERROR_HARDWARE_FAILURE;
    }
    sx_status = sx_blkcipher_wait(&cipher);
    if (sx_status != SX_OK) {
      return PSA_ERROR_HARDWARE_FAILURE;
    }

    *output_length = input_length;
  }
  return PSA_SUCCESS;
#else // SLI_PSA_DRIVER_FEATURE_AES_* && SLI_PSA_DRIVER_FEATURE_AES

  (void) attributes;
  (void) key_buffer;
  (void) key_buffer_size;
  (void) alg;
  (void) input;
  (void) input_length;
  (void) output;
  (void) output_size;
  (void) output_length;

  return PSA_ERROR_NOT_SUPPORTED;

#endif // SLI_PSA_DRIVER_FEATURE_AES_* && SLI_PSA_DRIVER_FEATURE_AES
}

psa_status_t sli_hostcrypto_transparent_cipher_encrypt_setup(
  sli_hostcrypto_transparent_cipher_operation_t *operation,
  const psa_key_attributes_t *attributes,
  const uint8_t *key_buffer,
  size_t key_buffer_size,
  psa_algorithm_t alg)
{
#if (defined(SLI_PSA_DRIVER_FEATURE_AES)                \
  && (defined(SLI_PSA_DRIVER_FEATURE_AES_ECB)           \
  || defined(SLI_PSA_DRIVER_FEATURE_AES_CTR)            \
  || defined(SLI_PSA_DRIVER_FEATURE_AES_CFB)            \
  || defined(SLI_PSA_DRIVER_FEATURE_AES_OFB)            \
  || defined(SLI_PSA_DRIVER_FEATURE_AES_CBC_NO_PADDING) \
  || defined(SLI_PSA_DRIVER_FEATURE_AES_CBC_PKCS7)))    \
  || defined(SLI_PSA_DRIVER_FEATURE_AES_CCM_STAR_NO_TAG)

  if (operation == NULL || attributes == NULL || key_buffer == NULL) {
    return PSA_ERROR_INVALID_ARGUMENT;
  }

  // Reset context.
  memset(operation, 0, sizeof(sli_hostcrypto_transparent_cipher_operation_t));

  // Set up context.
  memcpy(&operation->alg, &alg, sizeof(alg));
  operation->direction = SLI_HOSTCRYPTO_ENCRYPT;

  psa_status_t psa_status = driver_can_handle(attributes, alg, key_buffer_size);

  if (psa_status != PSA_SUCCESS) {
    return psa_status;
  }

  return sli_hostcrypto_load_key(&operation->key_ref, attributes, key_buffer);

#else // PSA_WANT_ALG_AES && PSA_WANT_KEY_TYPE_AES

  (void) operation;
  (void) attributes;
  (void) key_buffer;
  (void) key_buffer_size;
  (void) alg;

  return PSA_ERROR_NOT_SUPPORTED;

#endif // SLI_PSA_DRIVER_FEATURE_AES_* && SLI_PSA_DRIVER_FEATURE_AES
}

psa_status_t sli_hostcrypto_transparent_cipher_decrypt_setup(
  sli_hostcrypto_transparent_cipher_operation_t *operation,
  const psa_key_attributes_t *attributes,
  const uint8_t *key_buffer,
  size_t key_buffer_size,
  psa_algorithm_t alg)
{
#if (defined(SLI_PSA_DRIVER_FEATURE_AES)                \
  && (defined(SLI_PSA_DRIVER_FEATURE_AES_ECB)           \
  || defined(SLI_PSA_DRIVER_FEATURE_AES_CTR)            \
  || defined(SLI_PSA_DRIVER_FEATURE_AES_CFB)            \
  || defined(SLI_PSA_DRIVER_FEATURE_AES_OFB)            \
  || defined(SLI_PSA_DRIVER_FEATURE_AES_CBC_NO_PADDING) \
  || defined(SLI_PSA_DRIVER_FEATURE_AES_CBC_PKCS7)))    \
  || defined(SLI_PSA_DRIVER_FEATURE_AES_CCM_STAR_NO_TAG)

  if (operation == NULL || attributes == NULL || key_buffer == NULL) {
    return PSA_ERROR_INVALID_ARGUMENT;
  }

  // Reset context.
  memset(operation, 0, sizeof(sli_hostcrypto_transparent_cipher_operation_t));

  // Set up context.
  memcpy(&operation->alg, &alg, sizeof(alg));
  operation->direction = SLI_HOSTCRYPTO_DECRYPT;

  psa_status_t psa_status = driver_can_handle(attributes, alg, key_buffer_size);

  if (psa_status != PSA_SUCCESS) {
    return psa_status;
  }

  return sli_hostcrypto_load_key(&operation->key_ref, attributes, key_buffer);

#else // PSA_WANT_ALG_AES && PSA_WANT_KEY_TYPE_AES

  (void) operation;
  (void) attributes;
  (void) key_buffer;
  (void) key_buffer_size;
  (void) alg;

  return PSA_ERROR_NOT_SUPPORTED;

#endif // SLI_PSA_DRIVER_FEATURE_AES_* && SLI_PSA_DRIVER_FEATURE_AES
}

psa_status_t sli_hostcrypto_transparent_cipher_set_iv(
  sli_hostcrypto_transparent_cipher_operation_t *operation,
  const uint8_t *iv,
  size_t iv_length)
{
#if (defined(SLI_PSA_DRIVER_FEATURE_AES)                \
  && (defined(SLI_PSA_DRIVER_FEATURE_AES_CTR)           \
  || defined(SLI_PSA_DRIVER_FEATURE_AES_CFB)            \
  || defined(SLI_PSA_DRIVER_FEATURE_AES_OFB)            \
  || defined(SLI_PSA_DRIVER_FEATURE_AES_CBC_NO_PADDING) \
  || defined(SLI_PSA_DRIVER_FEATURE_AES_CBC_PKCS7)))    \
  || defined(SLI_PSA_DRIVER_FEATURE_AES_CCM_STAR_NO_TAG)

  int sx_status = SX_OK;
  psa_status_t psa_status = PSA_SUCCESS;

  if (operation == NULL || iv == NULL) {
    return PSA_ERROR_INVALID_ARGUMENT;
  }

  if ((operation->key_ref.key == NULL) && (operation->key_ref.cfg == 0)) {
    // Key reference is not set up.
    return PSA_ERROR_BAD_STATE;
  }

  if (operation->cipher.dma.regs != 0) {
    // cipher context was set previously
    return PSA_ERROR_BAD_STATE;
  }

  if (operation->alg == PSA_ALG_ECB_NO_PADDING) {
    if (iv_length > 0) {
      return PSA_ERROR_INVALID_ARGUMENT;
    }
    return PSA_SUCCESS;
  }

  uint8_t *iv_buf = operation->iv;
#if defined(SLI_PSA_DRIVER_FEATURE_AES_CCM_STAR_NO_TAG)
  if (operation->alg == PSA_ALG_CCM_STAR_NO_TAG) {
    if (iv_length != 13) {
      return PSA_ERROR_INVALID_ARGUMENT;
    }
    // AES-CCM*-no-tag is basically AES-CTR with preformatted IV
    iv_buf[0] = 1;
    memcpy(&iv_buf[1], iv, 13);
    iv_buf[14] = 0;
    iv_buf[15] = 1;
  } else
#endif   // SLI_PSA_DRIVER_FEATURE_AES_CCM_STAR_NO_TAG
  {
    if (iv_length > SLI_HOSTCRYPTO_AES_IV_SIZE) {
      return PSA_ERROR_INVALID_ARGUMENT;
    }
    memcpy(iv_buf, iv, SLI_HOSTCRYPTO_AES_IV_SIZE);
  }

  if (sli_sxsymcrypt_lock_cryptomaster_selection(
        SLI_SXSYMCRYPT_CRYPTOMASTER_HOSTSYMCRYPTO, false)) {
    return PSA_ERROR_SERVICE_FAILURE;
  }

  switch (operation->alg) {
#if defined(SLI_PSA_DRIVER_FEATURE_AES_CCM_STAR_NO_TAG)
    case PSA_ALG_CCM_STAR_NO_TAG:       // Explicit fallthrough
#endif
#if defined(SLI_PSA_DRIVER_FEATURE_AES_CTR) \
    || defined(SLI_PSA_DRIVER_FEATURE_AES_CCM_STAR_NO_TAG)
    case PSA_ALG_CTR:
      if (operation->direction == SLI_HOSTCRYPTO_ENCRYPT) {
        sx_status = sx_blkcipher_create_aesctr_enc(
          &operation->cipher, &operation->key_ref, (const char *) operation->iv);
      } else {
        sx_status = sx_blkcipher_create_aesctr_dec(
          &operation->cipher, &operation->key_ref, (const char *) operation->iv);
      }
      break;
#endif
#if defined(SLI_PSA_DRIVER_FEATURE_AES_CFB)
    case PSA_ALG_CFB:
      if (operation->direction == SLI_HOSTCRYPTO_ENCRYPT) {
        sx_status = sx_blkcipher_create_aescfb_enc(
          &operation->cipher, &operation->key_ref, (const char *) operation->iv);
      } else {
        sx_status = sx_blkcipher_create_aescfb_dec(
          &operation->cipher, &operation->key_ref, (const char *) operation->iv);
      }
      break;
#endif
#if defined(SLI_PSA_DRIVER_FEATURE_AES_OFB)
    case PSA_ALG_OFB:
      if (operation->direction == SLI_HOSTCRYPTO_ENCRYPT) {
        sx_status = sx_blkcipher_create_aesofb_enc(
          &operation->cipher, &operation->key_ref, (const char *) operation->iv);
      } else {
        sx_status = sx_blkcipher_create_aesofb_dec(
          &operation->cipher, &operation->key_ref, (const char *) operation->iv);
      }
      break;
#endif
#if defined(SLI_PSA_DRIVER_FEATURE_AES_CBC_NO_PADDING) \
    || defined(SLI_PSA_DRIVER_FEATURE_AES_CBC_PKCS7)
    case PSA_ALG_CBC_NO_PADDING:
    case PSA_ALG_CBC_PKCS7:
      if (operation->direction == SLI_HOSTCRYPTO_ENCRYPT) {
        sx_status = sx_blkcipher_create_aescbc_enc(
          &operation->cipher, &operation->key_ref, (const char *) operation->iv);
      } else {
        sx_status = sx_blkcipher_create_aescbc_dec(
          &operation->cipher, &operation->key_ref, (const char *) operation->iv);
      }
      break;
#endif
    default:
      psa_status = PSA_ERROR_NOT_SUPPORTED;
  }

  if (sx_status != SX_OK) {
    psa_status = PSA_ERROR_HARDWARE_FAILURE;
  }

  if (sli_sxsymcrypt_unlock_cryptomaster_selection()) {
    if (psa_status == PSA_SUCCESS) {
      psa_status = PSA_ERROR_SERVICE_FAILURE;
    }
  }

  return psa_status;

#else // SLI_PSA_DRIVER_FEATURE_AES_* && SLI_PSA_DRIVER_FEATURE_AES

  (void) operation;
  (void) iv;
  (void) iv_length;

  return PSA_ERROR_NOT_SUPPORTED;

#endif // SLI_PSA_DRIVER_FEATURE_AES_* && SLI_PSA_DRIVER_FEATURE_AES
}

psa_status_t sli_hostcrypto_transparent_cipher_update(
  sli_hostcrypto_transparent_cipher_operation_t *operation,
  const uint8_t *input,
  size_t input_length,
  uint8_t *output,
  size_t output_size,
  size_t *output_length)
{
#if (defined(SLI_PSA_DRIVER_FEATURE_AES)                \
  && (defined(SLI_PSA_DRIVER_FEATURE_AES_ECB)           \
  || defined(SLI_PSA_DRIVER_FEATURE_AES_CTR)            \
  || defined(SLI_PSA_DRIVER_FEATURE_AES_CFB)            \
  || defined(SLI_PSA_DRIVER_FEATURE_AES_OFB)            \
  || defined(SLI_PSA_DRIVER_FEATURE_AES_CBC_NO_PADDING) \
  || defined(SLI_PSA_DRIVER_FEATURE_AES_CBC_PKCS7)))    \
  || defined(SLI_PSA_DRIVER_FEATURE_AES_CCM_STAR_NO_TAG)

  int sx_status = SX_ERR_UNITIALIZED_OBJ;

  // Argument check.
  if (operation == NULL
      || (input == NULL && input_length > 0)
      || (output == NULL && output_size > 0)
      || output_length == NULL) {
    return PSA_ERROR_INVALID_ARGUMENT;
  }

  if (input_length == 0) {
    return PSA_SUCCESS;
  }

  if (!operation->cipher.dma.regs
      && !(operation->cipher.dma.dmamem.cfg & SLI_HOSTCRYPTO_BLKCIPHER_CTX_SAVE)) {
#if defined(SLI_PSA_DRIVER_FEATURE_AES_ECB)
    if (operation->alg == PSA_ALG_ECB_NO_PADDING) {
      // ECB mode does not have context saving so we need to create a new
      // operation everytime the update function is called
      if (sli_sxsymcrypt_lock_cryptomaster_selection(
            SLI_SXSYMCRYPT_CRYPTOMASTER_HOSTSYMCRYPTO, false)) {
        return PSA_ERROR_SERVICE_FAILURE;
      }
      if (operation->direction == SLI_HOSTCRYPTO_ENCRYPT) {
        sx_status = sx_blkcipher_create_aesecb_enc(&operation->cipher,
                                                   &operation->key_ref);
      } else if (operation->direction == SLI_HOSTCRYPTO_DECRYPT) {
        sx_status = sx_blkcipher_create_aesecb_dec(&operation->cipher,
                                                   &operation->key_ref);
      }
      if (sli_sxsymcrypt_unlock_cryptomaster_selection()) {
        return PSA_ERROR_SERVICE_FAILURE;
      }
      if (sx_status != SX_OK) {
        return PSA_ERROR_HARDWARE_FAILURE;
      }
    } else
#endif // SLI_PSA_DRIVER_FEATURE_AES_ECB
    {
      return PSA_ERROR_BAD_STATE;
    }
  }

  bool lagging = true;
  switch (operation->alg) {
    case PSA_ALG_ECB_NO_PADDING:
    case PSA_ALG_CBC_NO_PADDING:
    case PSA_ALG_CBC_PKCS7:
      lagging = true;
      break;
#if defined(SLI_PSA_DRIVER_FEATURE_AES_CCM_STAR_NO_TAG)
    case PSA_ALG_CCM_STAR_NO_TAG:
#endif
    case PSA_ALG_CTR:
    case PSA_ALG_CFB:
    case PSA_ALG_OFB:
      lagging = false;
      break;
    default:
      return PSA_ERROR_BAD_STATE;
  }

  size_t bytes_left_in_block = 16 - (operation->processed_length % 16);
  size_t actual_output_length = 0;
  *output_length = 0;
  bool cache_full_block = (operation->alg == PSA_ALG_CBC_PKCS7
                           && operation->direction == SLI_HOSTCRYPTO_DECRYPT);

  if (lagging) {
    // If input can fit in the block then just copy it to the block
    if (cache_full_block && operation->processed_length > 0
        && bytes_left_in_block == 16) {
      // Don't fill the cached block until it's processed
    } else if (input_length < bytes_left_in_block) {
      memcpy(&operation->block[operation->processed_length % 16],
             input,
             input_length);
      operation->processed_length += input_length;
      *output_length = actual_output_length;
      return PSA_SUCCESS;
    }
    // We have atleast the block we saved plus the input left
    size_t output_blocks = 1;
    if (input_length > bytes_left_in_block) {
      // plus however many full blocks are left over after filling the block
      output_blocks += (input_length - bytes_left_in_block) / 16;
      // If we're caching and the sum of already-input and to-be-input data
      // ends up at a block boundary, we won't be outputting the last block
      if (cache_full_block && ((input_length - bytes_left_in_block) % 16 == 0)) {
        output_blocks -= 1;
      }
    }
    if (output_size < (output_blocks * 16)) {
      return PSA_ERROR_BUFFER_TOO_SMALL;
    }
  } else {
    if (output_size < input_length) {
      return PSA_ERROR_BUFFER_TOO_SMALL;
    }
  }

  // Our drivers only support full or no overlap between input and output
  // buffers. So in the case of partial overlap, copy the input buffer into
  // the output buffer and process it in place as if the buffers fully
  // overlapped.
  if ((output > input) && (output < (input + input_length))) {
    // Sanity check before copying. Some ciphers have a stricter requirement
    // than this (if an IV is included), but no ciphers will have an output
    // smaller than the input.
    if (output_size < input_length) {
      return PSA_ERROR_INVALID_ARGUMENT;
    }

    memmove(output, input, input_length);
    input = output;
  }

  if (lagging) {
    if (bytes_left_in_block != 16) {
      // Read in up to full streaming input block.
      memcpy(&operation->block[operation->processed_length % 16],
             input,
             bytes_left_in_block);
#if defined(SLI_PSA_DRIVER_FEATURE_AES_CBC_PKCS7)
      if (operation->alg == PSA_ALG_CBC_PKCS7
          && (cache_full_block && (bytes_left_in_block == input_length))) {
        // Don't process the streaming block if there is no more input data
        sx_status = SX_OK;
      } else
#endif
      {
        if (operation->cipher.dma.dmamem.cfg & SLI_HOSTCRYPTO_BLKCIPHER_CTX_SAVE) {
          if (sli_sxsymcrypt_lock_cryptomaster_selection(
                SLI_SXSYMCRYPT_CRYPTOMASTER_HOSTSYMCRYPTO, false)) {
            return PSA_ERROR_SERVICE_FAILURE;
          }
          sx_status = sx_blkcipher_resume_state(&operation->cipher);
          if (sli_sxsymcrypt_unlock_cryptomaster_selection()) {
            return PSA_ERROR_SERVICE_FAILURE;
          }
          if (sx_status != SX_OK) {
            return PSA_ERROR_HARDWARE_FAILURE;
          }
        }
        sx_status = sx_blkcipher_crypt(&operation->cipher,
                                       (const char *) operation->block,
                                       16,
                                       (char *) output);
        if (sx_status != SX_OK) {
          return PSA_ERROR_HARDWARE_FAILURE;
        }
#if defined(SLI_PSA_DRIVER_FEATURE_AES_ECB)
        if (operation->alg == PSA_ALG_ECB_NO_PADDING) {
          sx_status = sx_blkcipher_run(&operation->cipher);
        } else
#endif
        {
          sx_status = sx_blkcipher_save_state(&operation->cipher);
        }
        if (sx_status != SX_OK) {
          return PSA_ERROR_HARDWARE_FAILURE;
        }
        sx_status = sx_blkcipher_wait(&operation->cipher);
      }
      if (sx_status != SX_OK) {
        return PSA_ERROR_HARDWARE_FAILURE;
      }

      if (!(cache_full_block && (bytes_left_in_block == input_length))) {
        output += 16;
        actual_output_length += 16;
      }
      input += bytes_left_in_block;
      input_length -= bytes_left_in_block;
      operation->processed_length += bytes_left_in_block;
    }
#if defined(SLI_PSA_DRIVER_FEATURE_AES_CBC_PKCS7)
    else if (cache_full_block && operation->processed_length > 0) {
      // We know there's processing to be done, and that we haven't processed
      // the full block in the streaming buffer yet. Process it now.
      if (operation->cipher.dma.dmamem.cfg & SLI_HOSTCRYPTO_BLKCIPHER_CTX_SAVE) {
        if (sli_sxsymcrypt_lock_cryptomaster_selection(
              SLI_SXSYMCRYPT_CRYPTOMASTER_HOSTSYMCRYPTO, false)) {
          return PSA_ERROR_SERVICE_FAILURE;
        }
        sx_status = sx_blkcipher_resume_state(&operation->cipher);
        if (sli_sxsymcrypt_unlock_cryptomaster_selection()) {
          return PSA_ERROR_SERVICE_FAILURE;
        }
        if (sx_status != SX_OK) {
          return PSA_ERROR_HARDWARE_FAILURE;
        }
      }
      sx_status = sx_blkcipher_crypt(&operation->cipher,
                                     (const char *) operation->block,
                                     16,
                                     (char *) output);
      if (sx_status != SX_OK) {
        return PSA_ERROR_HARDWARE_FAILURE;
      }
      sx_status = sx_blkcipher_save_state(&operation->cipher);
      if (sx_status != SX_OK) {
        return PSA_ERROR_HARDWARE_FAILURE;
      }
      sx_status = sx_blkcipher_wait(&operation->cipher);
      if (sx_status != SX_OK) {
        return PSA_ERROR_HARDWARE_FAILURE;
      }
      output += 16;
      actual_output_length += 16;
    }
#endif
  }

  if (input_length >= 16) {
    size_t operation_size = 0;
    operation_size = (input_length / 16) * 16;

    if (cache_full_block && (input_length % 16 == 0)) {
      // Don't decrypt the last block until finish is called, so that we
      // can properly remove the padding before returning it.
      operation_size -= 16;
    }
    if (operation_size > 0) {
      if ((operation->cipher.dma.dmamem.cfg & SLI_HOSTCRYPTO_BLKCIPHER_CTX_SAVE)
          || !operation->cipher.dma.regs) {
        if (sli_sxsymcrypt_lock_cryptomaster_selection(
              SLI_SXSYMCRYPT_CRYPTOMASTER_HOSTSYMCRYPTO, false)) {
          return PSA_ERROR_SERVICE_FAILURE;
        }
        if (operation->alg == PSA_ALG_ECB_NO_PADDING) {
          // ECB mode does not have context saving so we need to create a new
          // operation everytime the update function is called
          if (operation->direction == SLI_HOSTCRYPTO_ENCRYPT) {
            sx_status = sx_blkcipher_create_aesecb_enc(&operation->cipher,
                                                       &operation->key_ref);
          } else if (operation->direction == SLI_HOSTCRYPTO_DECRYPT) {
            sx_status = sx_blkcipher_create_aesecb_dec(&operation->cipher,
                                                       &operation->key_ref);
          }
        } else {
          sx_status = sx_blkcipher_resume_state(&operation->cipher);
        }
        if (sli_sxsymcrypt_unlock_cryptomaster_selection()) {
          return PSA_ERROR_SERVICE_FAILURE;
        }
        if (sx_status != SX_OK) {
          return PSA_ERROR_HARDWARE_FAILURE;
        }
      }
      sx_status = sx_blkcipher_crypt(&operation->cipher,
                                     (const char *) input,
                                     operation_size,
                                     (char *) output);
      if (sx_status != SX_OK) {
        return PSA_ERROR_HARDWARE_FAILURE;
      }
      if (operation->alg == PSA_ALG_ECB_NO_PADDING) {
        sx_status = sx_blkcipher_run(&operation->cipher);
      } else {
        sx_status = sx_blkcipher_save_state(&operation->cipher);
      }
      if (sx_status != SX_OK) {
        return PSA_ERROR_HARDWARE_FAILURE;
      }
      sx_status = sx_blkcipher_wait(&operation->cipher);
      if (sx_status != SX_OK) {
        return PSA_ERROR_HARDWARE_FAILURE;
      }
    }

    input += operation_size;
    input_length -= operation_size;
    operation->processed_length += operation_size;
    actual_output_length += operation_size;
    output += operation_size;
  }

  // Move remaining input to block
  if (input_length > 0) {
    if (!lagging) {
      if (operation->cipher.dma.dmamem.cfg & SLI_HOSTCRYPTO_BLKCIPHER_CTX_SAVE) {
        if (sli_sxsymcrypt_lock_cryptomaster_selection(
              SLI_SXSYMCRYPT_CRYPTOMASTER_HOSTSYMCRYPTO, false)) {
          return PSA_ERROR_SERVICE_FAILURE;
        }
        sx_status = sx_blkcipher_resume_state(&operation->cipher);
        if (sli_sxsymcrypt_unlock_cryptomaster_selection()) {
          return PSA_ERROR_SERVICE_FAILURE;
        }
        if (sx_status != SX_OK) {
          return PSA_ERROR_HARDWARE_FAILURE;
        }
      }
      sx_status = sx_blkcipher_crypt(&operation->cipher,
                                     (const char *) input,
                                     input_length,
                                     (char *) operation->block);
      if (sx_status != SX_OK) {
        return PSA_ERROR_HARDWARE_FAILURE;
      }
      sx_status = sx_blkcipher_save_state(&operation->cipher);
      if (sx_status != SX_OK) {
        return PSA_ERROR_HARDWARE_FAILURE;
      }
      sx_status = sx_blkcipher_wait(&operation->cipher);
      if (sx_status != SX_OK) {
        return PSA_ERROR_HARDWARE_FAILURE;
      }
      memcpy(output, operation->block, input_length);

      actual_output_length += input_length;
      operation->processed_length += input_length;
    } else {
      if ((input_length >= 16 && !cache_full_block)
          || (input_length > 16 && cache_full_block)) {
        *output_length = 0;
        return PSA_ERROR_BAD_STATE;
      }
      memcpy(&operation->block[operation->processed_length % 16],
             input,
             input_length);

      operation->processed_length += input_length;
    }
  }

  *output_length = actual_output_length;

  return PSA_SUCCESS;

#else // SLI_PSA_DRIVER_FEATURE_AES_* && SLI_PSA_DRIVER_FEATURE_AES

  (void) operation;
  (void) input;
  (void) input_length;
  (void) output;
  (void) output_size;
  (void) output_length;

  return PSA_ERROR_NOT_SUPPORTED;

#endif // SLI_PSA_DRIVER_FEATURE_AES_* && SLI_PSA_DRIVER_FEATURE_AES
}

psa_status_t sli_hostcrypto_transparent_cipher_finish(
  sli_hostcrypto_transparent_cipher_operation_t *operation,
  uint8_t *output,
  size_t output_size,
  size_t *output_length)
{
#if (defined(SLI_PSA_DRIVER_FEATURE_AES)                \
  && (defined(SLI_PSA_DRIVER_FEATURE_AES_ECB)           \
  || defined(SLI_PSA_DRIVER_FEATURE_AES_CTR)            \
  || defined(SLI_PSA_DRIVER_FEATURE_AES_CFB)            \
  || defined(SLI_PSA_DRIVER_FEATURE_AES_OFB)            \
  || defined(SLI_PSA_DRIVER_FEATURE_AES_CBC_NO_PADDING) \
  || defined(SLI_PSA_DRIVER_FEATURE_AES_CBC_PKCS7)))    \
  || defined(SLI_PSA_DRIVER_FEATURE_AES_CCM_STAR_NO_TAG)

  // Argument check.
  if (operation == NULL) {
    return PSA_ERROR_INVALID_ARGUMENT;
  }
  psa_status_t psa_status = PSA_ERROR_GENERIC_ERROR;
  int sx_status;

  switch (operation->alg) {
#if defined(SLI_PSA_DRIVER_FEATURE_AES_ECB) || defined(SLI_PSA_DRIVER_FEATURE_AES_CBC_NO_PADDING)
    // Blocksize-only modes without padding.
    case PSA_ALG_CBC_NO_PADDING:
    case PSA_ALG_ECB_NO_PADDING:
      (void) sx_status;
      (void) output;
      (void) output_size;
      // Can't finish if they haven't processed block-size input.
      if (operation->processed_length % 16 != 0) {
        psa_status = PSA_ERROR_INVALID_ARGUMENT;
      } else {
        if (operation->cipher.dma.regs) {
          sli_cmdma_release_hw(operation->cipher.dma.regs);
          operation->cipher.dma.regs = NULL;
        }
        psa_status = PSA_SUCCESS;
      }
      *output_length = 0;
      break;
#endif // SLI_PSA_DRIVER_FEATURE_AES_ECB || SLI_PSA_DRIVER_FEATURE_AES_CBC_NO_PADDING
#if defined(SLI_PSA_DRIVER_FEATURE_AES_CTR) || defined(SLI_PSA_DRIVER_FEATURE_AES_CFB) \
    || defined(SLI_PSA_DRIVER_FEATURE_AES_OFB)                                         \
    || defined(SLI_PSA_DRIVER_FEATURE_AES_CCM_STAR_NO_TAG)
    // Stream cipher modes.
    case PSA_ALG_CCM_STAR_NO_TAG:
    case PSA_ALG_CTR:
    case PSA_ALG_CFB:
    case PSA_ALG_OFB:
      (void) sx_status;
      (void) output;
      (void) output_size;
      *output_length = 0;
      psa_status = PSA_SUCCESS;
      break;
#endif // SLI_PSA_DRIVER_FEATURE_AES_CTR || SLI_PSA_DRIVER_FEATURE_AES_CFB || SLI_PSA_DRIVER_FEATURE_AES_OFB || SLI_PSA_DRIVER_FEATURE_AES_CCM_STAR_NO_TAG
#if defined(SLI_PSA_DRIVER_FEATURE_AES_CBC_PKCS7)
    // Padding mode.
    case PSA_ALG_CBC_PKCS7:
      if ((output == NULL && output_size > 0)
          || output_length == NULL) {
        psa_status = PSA_ERROR_INVALID_ARGUMENT;
        break;
      }
      if (operation->direction == SLI_HOSTCRYPTO_ENCRYPT) {
        if (output_size < 16) {
          psa_status = PSA_ERROR_BUFFER_TOO_SMALL;
          break;
        }
        size_t padding_bytes = 16 - (operation->processed_length % 16);
        memset(&operation->block[16 - padding_bytes],
               padding_bytes,
               padding_bytes);
        if (operation->cipher.dma.dmamem.cfg & SLI_HOSTCRYPTO_BLKCIPHER_CTX_SAVE) {
          if (sli_sxsymcrypt_lock_cryptomaster_selection(
                SLI_SXSYMCRYPT_CRYPTOMASTER_HOSTSYMCRYPTO, false)) {
            psa_status = PSA_ERROR_SERVICE_FAILURE;
            break;
          }
          sx_status = sx_blkcipher_resume_state(&operation->cipher);
          if (sli_sxsymcrypt_unlock_cryptomaster_selection()) {
            psa_status = PSA_ERROR_SERVICE_FAILURE;
            break;
          }
          if (sx_status != SX_OK) {
            psa_status =  PSA_ERROR_HARDWARE_FAILURE;
            break;
          }
        }
        sx_status = sx_blkcipher_crypt(&operation->cipher,
                                       (const char *) operation->block,
                                       16,
                                       (char *) output);
        if (sx_status != SX_OK) {
          psa_status = PSA_ERROR_HARDWARE_FAILURE;
          break;
        }
        sx_status = sx_blkcipher_run(&operation->cipher);
        if (sx_status != SX_OK) {
          psa_status = PSA_ERROR_HARDWARE_FAILURE;
          break;
        }
        sx_status = sx_blkcipher_wait(&operation->cipher);
        if (sx_status != SX_OK) {
          psa_status = PSA_ERROR_HARDWARE_FAILURE;
          break;
        }
        *output_length = 16;
        psa_status = PSA_SUCCESS;
      } else if (operation->direction == SLI_HOSTCRYPTO_DECRYPT) {
        // Expect full-block input.
        if (operation->processed_length % 16 != 0
            || operation->processed_length < 16) {
          psa_status = PSA_ERROR_INVALID_ARGUMENT;
          break;
        }

        uint8_t out_buf[16];

        // Decrypt the last block
        if (operation->cipher.dma.dmamem.cfg & SLI_HOSTCRYPTO_BLKCIPHER_CTX_SAVE) {
          if (sli_sxsymcrypt_lock_cryptomaster_selection(
                SLI_SXSYMCRYPT_CRYPTOMASTER_HOSTSYMCRYPTO, false)) {
            psa_status = PSA_ERROR_SERVICE_FAILURE;
            break;
          }
          sx_status = sx_blkcipher_resume_state(&operation->cipher);
          if (sli_sxsymcrypt_unlock_cryptomaster_selection()) {
            psa_status = PSA_ERROR_SERVICE_FAILURE;
            break;
          }
          if (sx_status != SX_OK) {
            psa_status = PSA_ERROR_HARDWARE_FAILURE;
            break;
          }
        }
        sx_status = sx_blkcipher_crypt(&operation->cipher,
                                       (const char *) operation->block,
                                       16,
                                       (char *) out_buf);
        if (sx_status != SX_OK) {
          psa_status = PSA_ERROR_HARDWARE_FAILURE;
          break;
        }
        sx_status = sx_blkcipher_run(&operation->cipher);
        if (sx_status != SX_OK) {
          psa_status = PSA_ERROR_HARDWARE_FAILURE;
          break;
        }
        sx_status = sx_blkcipher_wait(&operation->cipher);
        if (sx_status != SX_OK) {
          psa_status = PSA_ERROR_HARDWARE_FAILURE;
          break;
        }

        size_t padding_bytes = 0;
        psa_status = sli_psa_validate_pkcs7_padding(out_buf,
                                                    16,
                                                    &padding_bytes);

        if (psa_status == PSA_SUCCESS) {
          // The padding was valid.
          if (output_size < 16 - padding_bytes) {
            psa_status = PSA_ERROR_BUFFER_TOO_SMALL;
            break;
          }
          memcpy(output, out_buf, 16 - padding_bytes);
          *output_length = 16 - padding_bytes;
        }
        psa_status = PSA_SUCCESS;
      }
      break;
#endif // SLI_PSA_DRIVER_FEATURE_AES_CBC_PKCS7
    default:
      psa_status = PSA_ERROR_BAD_STATE;
  }

  if (psa_status != PSA_SUCCESS && output_length != NULL) {
    *output_length = 0;
  }

  // Wipe context.
  memset(operation, 0, sizeof(sli_hostcrypto_transparent_cipher_operation_t));

  return psa_status;

#else // SLI_PSA_DRIVER_FEATURE_AES_* && PSA_WANT_KEY_TYPE_*

  (void) operation;
  (void) output;
  (void) output_size;
  (void) output_length;

  return PSA_ERROR_NOT_SUPPORTED;

#endif // SLI_PSA_DRIVER_FEATURE_AES_* && PSA_WANT_KEY_TYPE_*
}

psa_status_t sli_hostcrypto_transparent_cipher_abort(
  sli_hostcrypto_transparent_cipher_operation_t *operation)
{
#if (defined(SLI_PSA_DRIVER_FEATURE_AES)                \
  && (defined(SLI_PSA_DRIVER_FEATURE_AES_ECB)           \
  || defined(SLI_PSA_DRIVER_FEATURE_AES_CTR)            \
  || defined(SLI_PSA_DRIVER_FEATURE_AES_CFB)            \
  || defined(SLI_PSA_DRIVER_FEATURE_AES_OFB)            \
  || defined(SLI_PSA_DRIVER_FEATURE_AES_CBC_NO_PADDING) \
  || defined(SLI_PSA_DRIVER_FEATURE_AES_CBC_PKCS7)))    \
  || defined(SLI_PSA_DRIVER_FEATURE_AES_CCM_STAR_NO_TAG)

  if (operation != NULL) {
    // Wipe context.
    memset(operation, 0, sizeof(*operation));
  }

  return PSA_SUCCESS;

#else // SLI_PSA_DRIVER_FEATURE_AES_* && SLI_PSA_DRIVER_FEATURE_AES

  (void) operation;

  return PSA_ERROR_NOT_SUPPORTED;

#endif // SLI_PSA_DRIVER_FEATURE_AES_* && SLI_PSA_DRIVER_FEATURE_AES
}

#endif // SLI_MBEDTLS_DEVICE_HC
