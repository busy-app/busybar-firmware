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
#include "sxsymcrypt/aead.h"
#if !defined(SLI_MBEDTLS_DEVICE_HC_LPW)
#include "sxsymcrypt/chachapoly.h"
#endif
#include "sxsymcrypt/keyref.h"
#include "sxsymcrypt/statuscodes.h"

#include "string.h"

#define SLI_HOSTCRYPTO_AEAD_CTX_SAVE            (1u << 5)
#define SLI_HOSTCRYPTO_CHACHAPOLY_CTX_SAVE      (1u << 6)
#define SLI_HOSTCRYPTO_CCM_STAR_TAG_LENGTH      0
#define SLI_HOSTCRYPTO_AEAD_BLOCK_SIZE          16

// -----------------------------------------------------------------------------
// Static Helper Functions

#if defined(SLI_PSA_DRIVER_FEATURE_CCM) || defined(SLI_PSA_DRIVER_FEATURE_GCM) \
  || defined(SLI_PSA_DRIVER_FEATURE_CHACHAPOLY)

static uint32_t get_aead_ctx_save_flag(psa_algorithm_t alg)
{
 #if defined(SLI_PSA_DRIVER_FEATURE_CHACHAPOLY)
  if (PSA_ALG_AEAD_WITH_SHORTENED_TAG(alg,
                                      0)
      == PSA_ALG_AEAD_WITH_SHORTENED_TAG(PSA_ALG_CHACHA20_POLY1305, 0)) {
    return SLI_HOSTCRYPTO_CHACHAPOLY_CTX_SAVE;
  }
 #endif
  (void) alg;
  return SLI_HOSTCRYPTO_AEAD_CTX_SAVE;
}

static psa_status_t check_aead_key_type(const psa_key_attributes_t *attributes,
                                        const psa_algorithm_t alg)
{
  if (attributes == NULL) {
    return PSA_ERROR_INVALID_ARGUMENT;
  }

  switch (PSA_ALG_AEAD_WITH_SHORTENED_TAG(alg, 0)) {
#if defined(SLI_PSA_DRIVER_FEATURE_CCM)
    case PSA_ALG_AEAD_WITH_SHORTENED_TAG(PSA_ALG_CCM, 0):
      if (psa_get_key_type(attributes) != PSA_KEY_TYPE_AES) {
        return PSA_ERROR_INVALID_ARGUMENT;
      }
      break;
#endif // SLI_PSA_DRIVER_FEATURE_CCM
#if defined(SLI_PSA_DRIVER_FEATURE_GCM)
    case PSA_ALG_AEAD_WITH_SHORTENED_TAG(PSA_ALG_GCM, 0):
      if (psa_get_key_type(attributes) != PSA_KEY_TYPE_AES) {
        return PSA_ERROR_INVALID_ARGUMENT;
      }
      break;
#endif // SLI_PSA_DRIVER_FEATURE_GCM
#if defined(SLI_PSA_DRIVER_FEATURE_CHACHAPOLY)
    case PSA_ALG_AEAD_WITH_SHORTENED_TAG(PSA_ALG_CHACHA20_POLY1305, 0):
      // Chachapoly only supports 32 Byte (256 bits) key.
      if (psa_get_key_type(attributes) != PSA_KEY_TYPE_CHACHA20
          || psa_get_key_bits(attributes) != 256) {
        return PSA_ERROR_INVALID_ARGUMENT;
      }
      break;
#endif
    default:
      return PSA_ERROR_NOT_SUPPORTED;
      break;
  }
  return PSA_SUCCESS;
}

static psa_status_t driver_can_handle(const psa_key_attributes_t *attributes,
                                      psa_algorithm_t alg,
                                      size_t key_buffer_size)
{
  psa_status_t status = check_aead_key_type(attributes, alg);
  if (status != PSA_SUCCESS) {
    return status;
  }

  psa_key_location_t location =
    PSA_KEY_LIFETIME_GET_LOCATION(psa_get_key_lifetime(attributes));

  if (location == PSA_KEY_LOCATION_LOCAL_STORAGE) {
    size_t key_bits = psa_get_key_bits(attributes);
    if (key_buffer_size < PSA_BITS_TO_BYTES(key_bits)) {
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

  return PSA_SUCCESS;
}
static uint8_t* get_aead_block(sli_hostcrypto_transparent_aead_operation_t *operation)
{
 #if defined(SLI_PSA_DRIVER_FEATURE_CHACHAPOLY)
  if (PSA_ALG_AEAD_WITH_SHORTENED_TAG(operation->alg,
                                      0)
      == PSA_ALG_AEAD_WITH_SHORTENED_TAG(PSA_ALG_CHACHA20_POLY1305, 0)) {
    return operation->block.chacha_block;
  } else {
    return operation->block.aes_block;
  }
 #else
  return operation->block.aes_block;
 #endif
}

static size_t get_aead_block_size(psa_algorithm_t alg)
{
#if defined(SLI_PSA_DRIVER_FEATURE_CHACHAPOLY)
  if (PSA_ALG_AEAD_WITH_SHORTENED_TAG(alg,
                                      0)
      == PSA_ALG_AEAD_WITH_SHORTENED_TAG(PSA_ALG_CHACHA20_POLY1305, 0)) {
    return 64;     // Chachapoly block size
  }
#endif
  (void) alg;
  return 16;   // AES block size
}
static psa_status_t check_aead_parameters(const psa_key_attributes_t *attributes,
                                          psa_algorithm_t alg,
                                          size_t nonce_length,
                                          size_t additional_data_length)
{
  size_t tag_length = PSA_AEAD_TAG_LENGTH(psa_get_key_type(attributes),
                                          psa_get_key_bits(attributes),
                                          alg);

  switch (PSA_ALG_AEAD_WITH_SHORTENED_TAG(alg, 0)) {
#if defined(SLI_PSA_DRIVER_FEATURE_CCM)
    case PSA_ALG_AEAD_WITH_SHORTENED_TAG(PSA_ALG_CCM, 0):
      if (psa_get_key_type(attributes) != PSA_KEY_TYPE_AES) {
        return PSA_ERROR_INVALID_ARGUMENT;
      }
      if (tag_length < 4
          || tag_length > 16
          || tag_length % 2 != 0
          || nonce_length < 7
          || nonce_length > 13) {
        return PSA_ERROR_INVALID_ARGUMENT;
      }
      break;
#endif // SLI_PSA_DRIVER_FEATURE_CCM
#if defined(SLI_PSA_DRIVER_FEATURE_GCM)
    case PSA_ALG_AEAD_WITH_SHORTENED_TAG(PSA_ALG_GCM, 0):
      if (psa_get_key_type(attributes) != PSA_KEY_TYPE_AES) {
        return PSA_ERROR_INVALID_ARGUMENT;
      }
      // AD are limited to 2^64 bits, so 2^61 bytes.
      // We need not check if SIZE_MAX (max of size_t) is less than 2^61 (0x2000000000000000)
#if SIZE_MAX > 0x2000000000000000ull
      if (additional_data_length >> 61 != 0) {
        return PSA_ERROR_INVALID_ARGUMENT;
      }
#else // SIZE_MAX > 0x2000000000000000ull
      (void) additional_data_length;
#endif // SIZE_MAX > 0x2000000000000000ull
      if ((tag_length < 4) || (tag_length > 16)) {
        return PSA_ERROR_INVALID_ARGUMENT;
      }
      if (nonce_length == 0) {
        return PSA_ERROR_INVALID_ARGUMENT;
      }
#if !defined(SLI_PSA_SUPPORT_GCM_IV_CALCULATION)
      if (nonce_length != 12) {
        // sxsymcrypt only supports 12 bytes long IVs.
        return PSA_ERROR_NOT_SUPPORTED;
      }
#endif // ! SLI_PSA_SUPPORT_GCM_IV_CALCULATION
      break;
#endif // SLI_PSA_DRIVER_FEATURE_GCM
#if defined(SLI_PSA_DRIVER_FEATURE_CHACHAPOLY)
    case PSA_ALG_AEAD_WITH_SHORTENED_TAG(PSA_ALG_CHACHA20_POLY1305, 0):
      // Chachapoly only supports 32 Byte (256 bits) key.
      if (psa_get_key_type(attributes) != PSA_KEY_TYPE_CHACHA20
          || psa_get_key_bits(attributes) != 256) {
        return PSA_ERROR_INVALID_ARGUMENT;
      }
      // verify nonce and tag lengths
      if ((tag_length != 16) || nonce_length != 12) {
        return PSA_ERROR_INVALID_ARGUMENT;
      }

      // AD are limited to 2^64 bits, so 2^61 bytes.
      // We need not check if SIZE_MAX (max of size_t) is less than 2^61 (0x2000000000000000)
#if SIZE_MAX > 0x2000000000000000ull
      if (additional_data_length >> 61 != 0) {
        return PSA_ERROR_INVALID_ARGUMENT;
      }
#else // SIZE_MAX > 0x2000000000000000ull
      (void) additional_data_length;
#endif // SIZE_MAX > 0x2000000000000000ull
      break;
#endif // SLI_PSA_DRIVER_FEATURE_CHACHAPOLY
    default:
      return PSA_ERROR_NOT_SUPPORTED;
      break;
  }

#if !defined(SLI_PSA_DRIVER_FEATURE_GCM)
  (void) additional_data_length;
#endif // !SLI_PSA_DRIVER_FEATURE_GCM

  switch (psa_get_key_bits(attributes)) {
    case 128:     // fallthrough
    case 192:     // fallthrough
    case 256:
      break;
    default:
      return PSA_ERROR_INVALID_ARGUMENT;
  }

  return PSA_SUCCESS;
}
#endif //SLI_PSA_DRIVER_FEATURE_CCM || SLI_PSA_DRIVER_FEATURE_GCM || SLI_PSA_DRIVER_FEATURE_CHACHAPOLY

#if defined(SLI_PSA_SUPPORT_GCM_IV_CALCULATION) && defined(SLI_PSA_DRIVER_FEATURE_GCM)
/* Do GCM in software in case the IV isn't 12 bytes, since that's the only
 * thing the accelerator supports. */
static psa_status_t sli_hostcrypto_software_gcm(struct sxkeyref* key_ref,
                                                const uint8_t* nonce,
                                                size_t nonce_length,
                                                const uint8_t *additional_data,
                                                size_t additional_data_length,
                                                const uint8_t *input,
                                                uint8_t *output,
                                                size_t plaintext_length,
                                                size_t tag_length,
                                                uint8_t *tag,
                                                bool encrypt_ndecrypt)
{
  // Step 1: calculate H = Ek(0)
  uint8_t Ek[16] = { 0 };
  int sx_status = SX_ERR_UNITIALIZED_OBJ;

  struct sxblkcipher cipher;

  if (sli_sxsymcrypt_lock_cryptomaster_selection(
        SLI_SXSYMCRYPT_CRYPTOMASTER_HOSTSYMCRYPTO, false)) {
    return PSA_ERROR_SERVICE_FAILURE;
  }
  sx_status = sx_blkcipher_create_aesecb_enc(&cipher, key_ref);
  if (sli_sxsymcrypt_unlock_cryptomaster_selection()) {
    return PSA_ERROR_SERVICE_FAILURE;
  }
  if (sx_status != SX_OK) {
    return PSA_ERROR_HARDWARE_FAILURE;
  }
  sx_status = sx_blkcipher_crypt(&cipher,
                                 (const char *) Ek,
                                 sizeof(Ek),
                                 (char *) Ek);
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

  // Step 2: calculate IV = GHASH(H, {}, IV)
  uint8_t iv[16] = { 0 };
  uint64_t HL[16], HH[16];

  sli_psa_software_ghash_setup(Ek, HL, HH);

  for (size_t i = 0; i < nonce_length; i += 16) {
    // Mix in IV
    for (size_t j = 0; j < (nonce_length - i > 16 ? 16 : nonce_length - i); j++) {
      iv[j] ^= nonce[i + j];
    }
    // Update result
    sli_psa_software_ghash_multiply(HL, HH, iv, iv);
  }

  iv[12] ^= (nonce_length * 8) >> 24;
  iv[13] ^= (nonce_length * 8) >> 16;
  iv[14] ^= (nonce_length * 8) >>  8;
  iv[15] ^= (nonce_length * 8) >>  0;

  sli_psa_software_ghash_multiply(HL, HH, iv, iv);

  // Step 3: Calculate first counter block for tag generation
  uint8_t tagbuf[16] = { 0 };

  if (sli_sxsymcrypt_lock_cryptomaster_selection(
        SLI_SXSYMCRYPT_CRYPTOMASTER_HOSTSYMCRYPTO, false)) {
    return PSA_ERROR_SERVICE_FAILURE;
  }
  sx_status = sx_blkcipher_create_aesecb_enc(&cipher, key_ref);
  if (sli_sxsymcrypt_unlock_cryptomaster_selection()) {
    return PSA_ERROR_SERVICE_FAILURE;
  }
  if (sx_status != SX_OK) {
    return PSA_ERROR_HARDWARE_FAILURE;
  }
  sx_status = sx_blkcipher_crypt(&cipher,
                                 (const char *) iv,
                                 sizeof(iv),
                                 (char *) tagbuf);
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

  // If we're decrypting, mix in the to-be-checked tag value before transforming
  if (!encrypt_ndecrypt) {
    for (size_t i = 0; i < tag_length; i++) {
      tagbuf[i] ^= tag[i];
    }
  }

  // Step 4: increment IV (ripple increment)
  for (size_t i = 0; i < 16; i++) {
    iv[15 - i]++;

    if (iv[15 - i] != 0) {
      break;
    }
  }

  // Step 5: Accumulate additional data
  memset(Ek, 0, sizeof(Ek));
  for (size_t i = 0; i < additional_data_length; i += 16) {
    // Mix in additional data as much as we have
    for (size_t j = 0;
         j < (additional_data_length - i > 16 ? 16 : additional_data_length - i);
         j++) {
      Ek[j] ^= additional_data[i + j];
    }

    sli_psa_software_ghash_multiply(HL, HH, Ek, Ek);
  }

  // Step 6: If we're decrypting, accumulate the ciphertext before it gets transformed
  if (!encrypt_ndecrypt) {
    for (size_t i = 0; i < plaintext_length; i += 16) {
      // Mix in ciphertext
      for (size_t j = 0;
           j < (plaintext_length - i > 16 ? 16 : plaintext_length - i);
           j++) {
        Ek[j] ^= input[i + j];
      }

      sli_psa_software_ghash_multiply(HL, HH, Ek, Ek);
    }
  }

  // Step 7: transform data using AES-CTR
  if (plaintext_length) {
    if (sli_sxsymcrypt_lock_cryptomaster_selection(
          SLI_SXSYMCRYPT_CRYPTOMASTER_HOSTSYMCRYPTO, false)) {
      return PSA_ERROR_SERVICE_FAILURE;
    }
    sx_status = sx_blkcipher_create_aesctr_enc(&cipher,
                                               key_ref,
                                               (const char *)iv);
    if (sli_sxsymcrypt_unlock_cryptomaster_selection()) {
      return PSA_ERROR_SERVICE_FAILURE;
    }
    if (sx_status != SX_OK) {
      return PSA_ERROR_HARDWARE_FAILURE;
    }
    sx_status = sx_blkcipher_crypt(&cipher,
                                   (const char *) input,
                                   plaintext_length,
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
  }

  // Step 8: If we're encrypting, accumulate the ciphertext now
  if (encrypt_ndecrypt) {
    for (size_t i = 0; i < plaintext_length; i += 16) {
      // Mix in ciphertext
      for (size_t j = 0;
           j < (plaintext_length - i > 16 ? 16 : plaintext_length - i);
           j++) {
        Ek[j] ^= output[i + j];
      }

      sli_psa_software_ghash_multiply(HL, HH, Ek, Ek);
    }
  }

  // Step 9: add len(A) || len(C) block to tag calculation
  uint64_t bitlen = additional_data_length * 8;
  Ek[0]  ^= bitlen >> 56;
  Ek[1]  ^= bitlen >> 48;
  Ek[2]  ^= bitlen >> 40;
  Ek[3]  ^= bitlen >> 32;
  Ek[4]  ^= bitlen >> 24;
  Ek[5]  ^= bitlen >> 16;
  Ek[6]  ^= bitlen >>  8;
  Ek[7]  ^= bitlen >>  0;

  bitlen = plaintext_length * 8;
  Ek[8]  ^= bitlen >> 56;
  Ek[9]  ^= bitlen >> 48;
  Ek[10] ^= bitlen >> 40;
  Ek[11] ^= bitlen >> 32;
  Ek[12] ^= bitlen >> 24;
  Ek[13] ^= bitlen >> 16;
  Ek[14] ^= bitlen >>  8;
  Ek[15] ^= bitlen >>  0;

  sli_psa_software_ghash_multiply(HL, HH, Ek, Ek);

  // Step 10: calculate tag value
  for (size_t i = 0; i < tag_length; i++) {
    tagbuf[i] ^= Ek[i];
  }

  // Step 11: output tag for encrypt operation, check tag for decrypt
  if (encrypt_ndecrypt) {
    memcpy(tag, tagbuf, tag_length);
  } else {
    uint8_t accumulator = 0;
    for (size_t i = 0; i < tag_length; i++) {
      accumulator |= tagbuf[i];
    }
    if (accumulator != 0) {
      return PSA_ERROR_INVALID_SIGNATURE;
    }
  }

  return PSA_SUCCESS;
}
#endif // SLI_PSA_SUPPORT_GCM_IV_CALCULATION && SLI_PSA_DRIVER_FEATURE_GCM

psa_status_t sli_hostcrypto_transparent_aead_encrypt(
  const psa_key_attributes_t *attributes,
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
#if defined(SLI_PSA_DRIVER_FEATURE_CCM) || defined(SLI_PSA_DRIVER_FEATURE_GCM) \
  || defined(SLI_PSA_DRIVER_FEATURE_CHACHAPOLY)
  if (ciphertext_size <= plaintext_length) {
    return PSA_ERROR_BUFFER_TOO_SMALL;
  }

  size_t tag_length = 0;
  psa_status_t psa_status = sli_hostcrypto_transparent_aead_encrypt_tag(
    attributes, key_buffer, key_buffer_size, alg,
    nonce, nonce_length,
    additional_data, additional_data_length,
    plaintext, plaintext_length,
    ciphertext, plaintext_length, ciphertext_length,
    &ciphertext[plaintext_length], ciphertext_size - plaintext_length, &tag_length);

  if (psa_status == PSA_SUCCESS) {
    *ciphertext_length += tag_length;
  }

  return psa_status;

#else // SLI_PSA_DRIVER_FEATURE_CCM || SLI_PSA_DRIVER_FEATURE_GCM || SLI_PSA_DRIVER_FEATURE_CHACHAPOLY
  (void) attributes;
  (void) key_buffer;
  (void) key_buffer_size;
  (void) alg;
  (void) nonce;
  (void) nonce_length;
  (void) additional_data;
  (void) additional_data_length;
  (void) plaintext;
  (void) plaintext_length;
  (void) ciphertext;
  (void) ciphertext_size;
  (void) ciphertext_length;

  return PSA_ERROR_NOT_SUPPORTED;
#endif // SLI_PSA_DRIVER_FEATURE_CCM || SLI_PSA_DRIVER_FEATURE_GCM || SLI_PSA_DRIVER_FEATURE_CHACHAPOLY
}

psa_status_t sli_hostcrypto_transparent_aead_decrypt(
  const psa_key_attributes_t *attributes,
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
#if defined(SLI_PSA_DRIVER_FEATURE_CCM) || defined(SLI_PSA_DRIVER_FEATURE_GCM) \
  || defined(SLI_PSA_DRIVER_FEATURE_CHACHAPOLY)
  if (attributes == NULL) {
    return PSA_ERROR_INVALID_ARGUMENT;
  }

  size_t tag_length = PSA_AEAD_TAG_LENGTH(psa_get_key_type(attributes),
                                          psa_get_key_bits(attributes),
                                          alg);

  if (ciphertext_length < tag_length
      || ciphertext == NULL
      || (tag_length > 16)) {
    return PSA_ERROR_INVALID_ARGUMENT;
  }

  // Split the tag in its own buffer to avoid potential issues when the
  // plaintext buffer extends into the tag area
  uint8_t check_tag[16];
  memcpy(check_tag, &ciphertext[ciphertext_length - tag_length], tag_length);

  return sli_hostcrypto_transparent_aead_decrypt_tag(
    attributes, key_buffer, key_buffer_size, alg,
    nonce, nonce_length,
    additional_data, additional_data_length,
    ciphertext, ciphertext_length - tag_length,
    check_tag, tag_length,
    plaintext, plaintext_size, plaintext_length);
#else // SLI_PSA_DRIVER_FEATURE_CCM || SLI_PSA_DRIVER_FEATURE_GCM || SLI_PSA_DRIVER_FEATURE_CHACHAPOLY

  (void) attributes;
  (void) key_buffer;
  (void) key_buffer_size;
  (void) alg;
  (void) nonce;
  (void) nonce_length;
  (void) additional_data;
  (void) additional_data_length;
  (void) plaintext;
  (void) plaintext_size;
  (void) plaintext_length;
  (void) ciphertext;
  (void) ciphertext_length;

  return PSA_ERROR_NOT_SUPPORTED;

#endif // SLI_PSA_DRIVER_FEATURE_CCM || SLI_PSA_DRIVER_FEATURE_GCM || SLI_PSA_DRIVER_FEATURE_CHACHAPOLY
}

psa_status_t sli_hostcrypto_transparent_aead_encrypt_tag(
  const psa_key_attributes_t *attributes,
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
  size_t *ciphertext_length,
  uint8_t *tag,
  size_t tag_size,
  size_t *tag_length)
{
  #if defined(SLI_PSA_DRIVER_FEATURE_CCM) || defined(SLI_PSA_DRIVER_FEATURE_GCM) \
  || defined(SLI_PSA_DRIVER_FEATURE_CHACHAPOLY)

  if (key_buffer == NULL
      || attributes == NULL
      || nonce == NULL
      || (additional_data == NULL && additional_data_length > 0)
      || (plaintext == NULL && plaintext_length > 0)
      || (plaintext_length > 0 && (ciphertext == NULL || ciphertext_size == 0))
      || ciphertext_length == NULL) {
    return PSA_ERROR_INVALID_ARGUMENT;
  }

  if (tag_size == 0 || tag == NULL || tag_length == NULL) {
    return PSA_ERROR_INVALID_ARGUMENT;
  }

  *tag_length = PSA_AEAD_TAG_LENGTH(psa_get_key_type(attributes),
                                    psa_get_key_bits(attributes),
                                    alg);

  // Verify that the driver supports the given parameters.
  psa_status_t psa_status = check_aead_parameters(attributes,
                                                  alg,
                                                  nonce_length,
                                                  additional_data_length);
  if (psa_status != PSA_SUCCESS) {
    return psa_status;
  }

  // Check key type and buffer size.
  psa_status = driver_can_handle(attributes, alg, key_buffer_size);
  if (psa_status != PSA_SUCCESS) {
    return psa_status;
  }

  // Check sufficient output buffer size.
  if ((ciphertext_size < plaintext_length)
      || (tag_size < *tag_length)) {
    return PSA_ERROR_BUFFER_TOO_SMALL;
  }

  // Our drivers only support full or no overlap between input and output
  // buffers. So in the case of partial overlap, copy the input buffer into
  // the output buffer and process it in place as if the buffers fully
  // overlapped.
  if ((ciphertext > plaintext) && (ciphertext < (plaintext + plaintext_length))) {
    memmove(ciphertext, plaintext, plaintext_length);
    plaintext = ciphertext;
  }

  int sx_status = SX_ERR_UNITIALIZED_OBJ;

  struct sxaead aead;
  struct sxkeyref key_ref;
  psa_status = sli_hostcrypto_load_key(&key_ref, attributes, key_buffer);
  if (psa_status != PSA_SUCCESS) {
    return psa_status;
  }

#if defined(SLI_PSA_DRIVER_FEATURE_CCM)
  if (PSA_ALG_AEAD_WITH_SHORTENED_TAG(alg,
                                      0) == PSA_ALG_AEAD_WITH_SHORTENED_TAG(PSA_ALG_CCM, 0)) {
    // Check length of plaintext.
    unsigned char q = 16 - 1 - (unsigned char) nonce_length;
    if (q < sizeof(plaintext_length)
        && plaintext_length >= (1UL << (q * 8))) {
      return PSA_ERROR_INVALID_ARGUMENT;
    }

    if (sli_sxsymcrypt_lock_cryptomaster_selection(
          SLI_SXSYMCRYPT_CRYPTOMASTER_HOSTSYMCRYPTO, false)) {
      return PSA_ERROR_SERVICE_FAILURE;
    }
    sx_status = sx_aead_create_aesccm_enc(&aead, &key_ref,
                                          (const char *) nonce,
                                          nonce_length,
                                          *tag_length,
                                          additional_data_length,
                                          plaintext_length);
    if (sli_sxsymcrypt_unlock_cryptomaster_selection()) {
      psa_status = PSA_ERROR_SERVICE_FAILURE;
    }
  } else
#endif // SLI_PSA_DRIVER_FEATURE_CCM
#if defined(SLI_PSA_DRIVER_FEATURE_GCM)
  if (PSA_ALG_AEAD_WITH_SHORTENED_TAG(alg,
                                      0) == PSA_ALG_AEAD_WITH_SHORTENED_TAG(PSA_ALG_GCM, 0)) {
    if (nonce_length == 12) {
      if (sli_sxsymcrypt_lock_cryptomaster_selection(
            SLI_SXSYMCRYPT_CRYPTOMASTER_HOSTSYMCRYPTO, false)) {
        return PSA_ERROR_SERVICE_FAILURE;
      }
      sx_status = sx_aead_create_aesgcm_enc(&aead, &key_ref,
                                            (const char *) nonce);
      if (sli_sxsymcrypt_unlock_cryptomaster_selection()) {
        psa_status = PSA_ERROR_SERVICE_FAILURE;
      }
    }
#if defined(SLI_PSA_SUPPORT_GCM_IV_CALCULATION)
    else {
      psa_status = sli_hostcrypto_software_gcm(&key_ref,
                                               nonce, nonce_length,
                                               additional_data, additional_data_length,
                                               plaintext,
                                               ciphertext,
                                               plaintext_length,
                                               *tag_length,
                                               tag,
                                               true);
      if (psa_status == PSA_SUCCESS) {
        *ciphertext_length = plaintext_length;
      } else {
        *ciphertext_length = 0;
        *tag_length = 0;
      }
      return psa_status;
    }
#else // SLI_PSA_SUPPORT_GCM_IV_CALCULATION
    else {
      psa_status = PSA_ERROR_NOT_SUPPORTED;
    }
#endif // SLI_PSA_SUPPORT_GCM_IV_CALCULATION
  } else
#endif // SLI_PSA_DRIVER_FEATURE_GCM
#if defined(SLI_PSA_DRIVER_FEATURE_CHACHAPOLY)
  if (PSA_ALG_AEAD_WITH_SHORTENED_TAG(alg,
                                      0)
      == PSA_ALG_AEAD_WITH_SHORTENED_TAG(PSA_ALG_CHACHA20_POLY1305, 0)) {
    if (sli_sxsymcrypt_lock_cryptomaster_selection(
          SLI_SXSYMCRYPT_CRYPTOMASTER_HOSTSYMCRYPTO, false) != SL_STATUS_OK) {
      return PSA_ERROR_SERVICE_FAILURE;
    }
    sx_status = sx_aead_create_chacha20poly1305_enc(&aead, &key_ref,
                                                    (const char *) nonce);
    if (sli_sxsymcrypt_unlock_cryptomaster_selection()) {
      psa_status = PSA_ERROR_SERVICE_FAILURE;
    }
  } else
#endif
  {
    return PSA_ERROR_NOT_SUPPORTED;
  }

  if (psa_status != PSA_SUCCESS) {
    return psa_status;
  }
  if (sx_status != SX_OK) {
    return PSA_ERROR_HARDWARE_FAILURE;
  }

 #if defined(SLI_PSA_DRIVER_FEATURE_GCM)
  if (PSA_ALG_AEAD_WITH_SHORTENED_TAG(alg,
                                      0) == PSA_ALG_AEAD_WITH_SHORTENED_TAG(PSA_ALG_GCM, 0)) {
    sx_status = sx_aead_truncate_tag(&aead, *tag_length);
    if (sx_status != SX_OK) {
      return PSA_ERROR_HARDWARE_FAILURE;
    }
  }
 #endif
  if (additional_data_length) {
    sx_status = sx_aead_feed_aad(&aead,
                                 (const char *) additional_data,
                                 additional_data_length);
    if (sx_status != SX_OK) {
      return PSA_ERROR_HARDWARE_FAILURE;
    }
  }

  sx_status = sx_aead_crypt(&aead,
                            (const char *) plaintext,
                            plaintext_length,
                            (char *) ciphertext);
  if (sx_status != SX_OK) {
    return PSA_ERROR_HARDWARE_FAILURE;
  }

  sx_status = sx_aead_produce_tag(&aead, (char *) tag);
  if (sx_status != SX_OK) {
    return PSA_ERROR_HARDWARE_FAILURE;
  }

  sx_status = sx_aead_wait(&aead);
  if (sx_status != SX_OK) {
    return PSA_ERROR_HARDWARE_FAILURE;
  }

  *ciphertext_length = plaintext_length;

  return PSA_SUCCESS;

#else // SLI_PSA_DRIVER_FEATURE_CCM || SLI_PSA_DRIVER_FEATURE_GCM || SLI_PSA_DRIVER_FEATURE_CHACHAPOLY

  (void) attributes;
  (void) key_buffer;
  (void) key_buffer_size;
  (void) alg;
  (void) nonce;
  (void) nonce_length;
  (void) additional_data;
  (void) additional_data_length;
  (void) plaintext;
  (void) plaintext_length;
  (void) ciphertext;
  (void) ciphertext_size;
  (void) ciphertext_length;
  (void) tag;
  (void) tag_size;
  (void) tag_length;

  return PSA_ERROR_NOT_SUPPORTED;

#endif // SLI_PSA_DRIVER_FEATURE_CCM || SLI_PSA_DRIVER_FEATURE_GCM || SLI_PSA_DRIVER_FEATURE_CHACHAPOLY
}

psa_status_t sli_hostcrypto_transparent_aead_decrypt_tag(
  const psa_key_attributes_t *attributes,
  const uint8_t *key_buffer,
  size_t key_buffer_size,
  psa_algorithm_t alg,
  const uint8_t *nonce,
  size_t nonce_length,
  const uint8_t *additional_data,
  size_t additional_data_length,
  const uint8_t *ciphertext,
  size_t ciphertext_length,
  const uint8_t *tag,
  size_t tag_length,
  uint8_t *plaintext,
  size_t plaintext_size,
  size_t *plaintext_length)
{
#if defined(SLI_PSA_DRIVER_FEATURE_CCM) || defined(SLI_PSA_DRIVER_FEATURE_GCM) \
  || defined(SLI_PSA_DRIVER_FEATURE_CHACHAPOLY)
  if (attributes == NULL
      || key_buffer == NULL
      || nonce == NULL
      || (additional_data == NULL && additional_data_length > 0)
      || (ciphertext == NULL && ciphertext_length > 0)
      || (plaintext == NULL && plaintext_size > 0)
      || plaintext_length == NULL) {
    return PSA_ERROR_INVALID_ARGUMENT;
  }

  if (tag == NULL) {
    return PSA_ERROR_INVALID_ARGUMENT;
  }

  // Verify that the driver supports the given parameters.
  psa_status_t psa_status = check_aead_parameters(attributes,
                                                  alg,
                                                  nonce_length,
                                                  additional_data_length);
  if (psa_status != PSA_SUCCESS) {
    return psa_status;
  }

  // Check key type and buffer size.
  psa_status = driver_can_handle(attributes, alg, key_buffer_size);
  if (psa_status != PSA_SUCCESS) {
    return psa_status;
  }

  // Check sufficient output buffer size.
  if (plaintext_size < ciphertext_length) {
    return PSA_ERROR_BUFFER_TOO_SMALL;
  }

  // Our drivers only support full or no overlap between input and output
  // buffers. So in the case of partial overlap, copy the input buffer into
  // the output buffer and process it in place as if the buffers fully
  // overlapped.
  if ((plaintext > ciphertext) && (plaintext < (ciphertext + ciphertext_length))) {
    memmove(plaintext, ciphertext, ciphertext_length);
    ciphertext = plaintext;
  }

  int sx_status = SX_ERR_UNITIALIZED_OBJ;

  struct sxaead aead;
  struct sxkeyref key_ref;
  psa_status = sli_hostcrypto_load_key(&key_ref, attributes, key_buffer);
  if (psa_status != PSA_SUCCESS) {
    return psa_status;
  }

#if defined(SLI_PSA_DRIVER_FEATURE_CCM)
  if (PSA_ALG_AEAD_WITH_SHORTENED_TAG(alg,
                                      0) == PSA_ALG_AEAD_WITH_SHORTENED_TAG(PSA_ALG_CCM, 0)) {
    // Check length of ciphertext.
    unsigned char q = 16 - 1 - (unsigned char) nonce_length;
    if (q < sizeof(ciphertext_length)
        && ciphertext_length >= (1UL << (q * 8))) {
      return PSA_ERROR_INVALID_ARGUMENT;
    }

    if (sli_sxsymcrypt_lock_cryptomaster_selection(
          SLI_SXSYMCRYPT_CRYPTOMASTER_HOSTSYMCRYPTO, false)) {
      return PSA_ERROR_SERVICE_FAILURE;
    }
    sx_status = sx_aead_create_aesccm_dec(&aead, &key_ref,
                                          (const char *) nonce,
                                          nonce_length,
                                          tag_length,
                                          additional_data_length,
                                          ciphertext_length);
    if (sli_sxsymcrypt_unlock_cryptomaster_selection()) {
      psa_status = PSA_ERROR_SERVICE_FAILURE;
    }
  } else
#endif // SLI_PSA_DRIVER_FEATURE_CCM
#if defined(SLI_PSA_DRIVER_FEATURE_GCM)
  if (PSA_ALG_AEAD_WITH_SHORTENED_TAG(alg,
                                      0) == PSA_ALG_AEAD_WITH_SHORTENED_TAG(PSA_ALG_GCM, 0)) {
    if (nonce_length == 12) {
      if (sli_sxsymcrypt_lock_cryptomaster_selection(
            SLI_SXSYMCRYPT_CRYPTOMASTER_HOSTSYMCRYPTO, false)) {
        return PSA_ERROR_SERVICE_FAILURE;
      }
      sx_status = sx_aead_create_aesgcm_dec(&aead, &key_ref,
                                            (const char *) nonce);
      if (sli_sxsymcrypt_unlock_cryptomaster_selection()) {
        psa_status = PSA_ERROR_SERVICE_FAILURE;
      }
    }
#if defined(SLI_PSA_SUPPORT_GCM_IV_CALCULATION)
    else {
      psa_status = sli_hostcrypto_software_gcm(&key_ref,
                                               nonce, nonce_length,
                                               additional_data, additional_data_length,
                                               ciphertext,
                                               plaintext,
                                               ciphertext_length,
                                               tag_length,
                                               (uint8_t *) tag,
                                               false);
      if (psa_status == PSA_SUCCESS) {
        *plaintext_length = ciphertext_length;
      } else {
        *plaintext_length = 0;
      }
      return psa_status;
    }
#else // SLI_PSA_SUPPORT_GCM_IV_CALCULATION
    else {
      psa_status = PSA_ERROR_NOT_SUPPORTED;
    }
#endif // SLI_PSA_SUPPORT_GCM_IV_CALCULATION
  } else
#endif // SLI_PSA_DRIVER_FEATURE_GCM
#if defined(SLI_PSA_DRIVER_FEATURE_CHACHAPOLY)
  if (PSA_ALG_AEAD_WITH_SHORTENED_TAG(alg,
                                      0)
      == PSA_ALG_AEAD_WITH_SHORTENED_TAG(PSA_ALG_CHACHA20_POLY1305, 0)) {
    if (sli_sxsymcrypt_lock_cryptomaster_selection(
          SLI_SXSYMCRYPT_CRYPTOMASTER_HOSTSYMCRYPTO, false) != SL_STATUS_OK) {
      return PSA_ERROR_SERVICE_FAILURE;
    }
    sx_status = sx_aead_create_chacha20poly1305_dec(&aead,
                                                    &key_ref,
                                                    (const char *) nonce);
    if (sli_sxsymcrypt_unlock_cryptomaster_selection()) {
      psa_status = PSA_ERROR_SERVICE_FAILURE;
    }
  } else
#endif // SLI_PSA_DRIVER_FEATURE_CHACHAPOLY
  {
    return PSA_ERROR_NOT_SUPPORTED;
  }

  if (psa_status != PSA_SUCCESS) {
    return psa_status;
  }
  if (sx_status != SX_OK) {
    return PSA_ERROR_HARDWARE_FAILURE;
  }

#if defined(SLI_PSA_DRIVER_FEATURE_GCM)
  if (PSA_ALG_AEAD_WITH_SHORTENED_TAG(alg,
                                      0) == PSA_ALG_AEAD_WITH_SHORTENED_TAG(PSA_ALG_GCM, 0)) {
    sx_status = sx_aead_truncate_tag(&aead, tag_length);
    if (sx_status != SX_OK) {
      return PSA_ERROR_HARDWARE_FAILURE;
    }
  }
#endif

  if (additional_data_length) {
    sx_status = sx_aead_feed_aad(&aead,
                                 (const char *) additional_data,
                                 additional_data_length);
    if (sx_status != SX_OK) {
      return PSA_ERROR_HARDWARE_FAILURE;
    }
  }

  sx_status = sx_aead_crypt(&aead,
                            (const char *) ciphertext,
                            ciphertext_length,
                            (char *) plaintext);
  if (sx_status != SX_OK) {
    *plaintext_length = 0;
    sli_psa_zeroize(plaintext, plaintext_size);
    return PSA_ERROR_HARDWARE_FAILURE;
  }

  sx_status = sx_aead_verify_tag(&aead, (const char *) tag);
  if (sx_status != SX_OK) {
    *plaintext_length = 0;
    sli_psa_zeroize(plaintext, plaintext_size);
    return PSA_ERROR_HARDWARE_FAILURE;
  }

  sx_status = sx_aead_wait(&aead);
  if (sx_status != SX_OK) {
    *plaintext_length = 0;
    sli_psa_zeroize(plaintext, plaintext_size);
    if (sx_status == SX_ERR_INVALID_TAG) {
      return PSA_ERROR_INVALID_SIGNATURE;
    } else {
      return PSA_ERROR_HARDWARE_FAILURE;
    }
  }

  *plaintext_length = ciphertext_length;

  return PSA_SUCCESS;

#else // SLI_PSA_DRIVER_FEATURE_CCM || SLI_PSA_DRIVER_FEATURE_GCM || SLI_PSA_DRIVER_FEATURE_CHACHAPOLY

  (void) attributes;
  (void) key_buffer;
  (void) key_buffer_size;
  (void) alg;
  (void) nonce;
  (void) nonce_length;
  (void) additional_data;
  (void) additional_data_length;
  (void) plaintext;
  (void) plaintext_size;
  (void) plaintext_length;
  (void) ciphertext;
  (void) ciphertext_length;
  (void) tag;
  (void) tag_length;

  return PSA_ERROR_NOT_SUPPORTED;

#endif // SLI_PSA_DRIVER_FEATURE_CCM || SLI_PSA_DRIVER_FEATURE_GCM || SLI_PSA_DRIVER_FEATURE_CHACHAPOLY
}

#if defined(SLI_PSA_DRIVER_FEATURE_CCM) || defined(SLI_PSA_DRIVER_FEATURE_GCM) \
  || defined(SLI_PSA_DRIVER_FEATURE_CHACHAPOLY)
static psa_status_t transparent_aead_encrypt_decrypt_setup(
  sli_hostcrypto_transparent_aead_operation_t *operation,
  const psa_key_attributes_t *attributes,
  const uint8_t *key_buffer,
  size_t key_buffer_size,
  psa_algorithm_t alg,
  sli_encrypt_direction_t operation_direction)
{
  if (operation == NULL
      || attributes == NULL
      || key_buffer == NULL) {
    return PSA_ERROR_INVALID_ARGUMENT;
  }

  memset(operation, 0, sizeof(*operation));

  // Check key type and buffer size.
  psa_status_t status = driver_can_handle(attributes, alg, key_buffer_size);
  if (status != PSA_SUCCESS) {
    return status;
  }

  size_t key_bits = psa_get_key_bits(attributes);

  // Validate operation.
#if defined(SLI_PSA_DRIVER_FEATURE_CHACHAPOLY)
  if (PSA_ALG_AEAD_WITH_SHORTENED_TAG(alg,
                                      0)
      == PSA_ALG_AEAD_WITH_SHORTENED_TAG(PSA_ALG_CHACHA20_POLY1305, 0)) {
    if (key_size != 32
        && (PSA_AEAD_TAG_LENGTH(psa_get_key_type(attributes), key_bits, alg) != 16)) {
      return PSA_ERROR_INVALID_ARGUMENT;
    }
  }
#endif

  // Validate tag length.
  if (PSA_AEAD_TAG_LENGTH(psa_get_key_type(attributes), key_bits, alg) > 16) {
    return PSA_ERROR_INVALID_ARGUMENT;
  }

  // Validate operation.
  switch (PSA_ALG_AEAD_WITH_SHORTENED_TAG(alg, 0)) {
  #if defined(SLI_PSA_DRIVER_FEATURE_GCM)
    case PSA_ALG_AEAD_WITH_SHORTENED_TAG(PSA_ALG_GCM, 0):
    {
      operation->alg = alg;
      break;
    }
  #endif
  #if defined(SLI_PSA_DRIVER_FEATURE_CCM)
    case PSA_ALG_AEAD_WITH_SHORTENED_TAG(PSA_ALG_CCM, 0):
    {
      operation->alg = alg;
      break;
    }
  #endif
  #if defined(SLI_PSA_DRIVER_FEATURE_CHACHAPOLY)
    case PSA_ALG_AEAD_WITH_SHORTENED_TAG(PSA_ALG_CHACHA20_POLY1305, 0):
    {
      operation->alg = alg;
      break;
    }
  #endif
    default:
      return PSA_ERROR_NOT_SUPPORTED;
  }

  psa_status_t psa_status = sli_hostcrypto_load_key(&operation->key_ref, attributes, key_buffer);
  if (psa_status != PSA_SUCCESS) {
    return psa_status;
  }

  operation->direction = operation_direction;

  operation->lens_set = false;

  return PSA_SUCCESS;
}
#endif // SLI_PSA_DRIVER_FEATURE_CCM || SLI_PSA_DRIVER_FEATURE_GCM || SLI_PSA_DRIVER_FEATURE_CHACHAPOLY

psa_status_t sli_hostcrypto_transparent_aead_encrypt_setup(
  sli_hostcrypto_transparent_aead_operation_t *operation,
  const psa_key_attributes_t *attributes,
  const uint8_t *key_buffer,
  size_t key_buffer_size,
  psa_algorithm_t alg)
{
#if defined(SLI_PSA_DRIVER_FEATURE_CCM) || defined(SLI_PSA_DRIVER_FEATURE_GCM) \
  || defined(SLI_PSA_DRIVER_FEATURE_CHACHAPOLY)

  return transparent_aead_encrypt_decrypt_setup(operation,
                                                attributes,
                                                key_buffer,
                                                key_buffer_size,
                                                alg,
                                                SLI_HOSTCRYPTO_ENCRYPT);

#else // SLI_PSA_DRIVER_FEATURE_CCM || SLI_PSA_DRIVER_FEATURE_GCM || SLI_PSA_DRIVER_FEATURE_CHACHAPOLY

  (void) operation;
  (void) attributes;
  (void) key_buffer;
  (void) key_buffer_size;
  (void) alg;

  return PSA_ERROR_NOT_SUPPORTED;
#endif // SLI_PSA_DRIVER_FEATURE_CCM || SLI_PSA_DRIVER_FEATURE_GCM || SLI_PSA_DRIVER_FEATURE_CHACHAPOLY
}

psa_status_t sli_hostcrypto_transparent_aead_decrypt_setup(
  sli_hostcrypto_transparent_aead_operation_t *operation,
  const psa_key_attributes_t *attributes,
  const uint8_t *key_buffer,
  size_t key_buffer_size,
  psa_algorithm_t alg)
{
  #if defined(SLI_PSA_DRIVER_FEATURE_CCM) || defined(SLI_PSA_DRIVER_FEATURE_GCM) \
  || defined(SLI_PSA_DRIVER_FEATURE_CHACHAPOLY)

  return transparent_aead_encrypt_decrypt_setup(operation,
                                                attributes,
                                                key_buffer,
                                                key_buffer_size,
                                                alg,
                                                SLI_HOSTCRYPTO_DECRYPT);

  #else // SLI_PSA_DRIVER_FEATURE_CCM || SLI_PSA_DRIVER_FEATURE_GCM || SLI_PSA_DRIVER_FEATURE_CHACHAPOLY
  (void) operation;
  (void) attributes;
  (void) key_buffer;
  (void) key_buffer_size;
  (void) alg;
  return PSA_ERROR_NOT_SUPPORTED;
  #endif // SLI_PSA_DRIVER_FEATURE_CCM || SLI_PSA_DRIVER_FEATURE_GCM || SLI_PSA_DRIVER_FEATURE_CHACHAPOLY
}

psa_status_t sli_hostcrypto_transparent_aead_set_lengths(
  sli_hostcrypto_transparent_aead_operation_t *operation,
  size_t ad_length,
  size_t plaintext_length)
{
#if defined(SLI_PSA_DRIVER_FEATURE_CCM) || defined(SLI_PSA_DRIVER_FEATURE_GCM) \
  || defined(SLI_PSA_DRIVER_FEATURE_CHACHAPOLY)
  if (operation == NULL) {
    return PSA_ERROR_INVALID_ARGUMENT;
  }

  // Check if operation has already started
  if (operation->ad_len > 0 || operation->processed_len != 0) {
    return PSA_ERROR_BAD_STATE;
  }

  size_t tag_len = 0;
  // To pass current PSA Crypto test suite, tag length encoded in the
  // algorithm needs to be checked at this point.
  switch (PSA_ALG_AEAD_WITH_SHORTENED_TAG(operation->alg, 0)) {
#if defined(SLI_PSA_DRIVER_FEATURE_CCM)
    case PSA_ALG_AEAD_WITH_SHORTENED_TAG(PSA_ALG_CCM, 0):
      tag_len = PSA_ALG_AEAD_GET_TAG_LENGTH(operation->alg);
      if ((tag_len % 2 != 0) || tag_len < 4 || tag_len > 16) {
        return PSA_ERROR_INVALID_ARGUMENT;
      }
      operation->ad_len = ad_length;
      operation->plaintext_len = plaintext_length;
      operation->lens_set = true;
      break;
#endif
#if defined(SLI_PSA_DRIVER_FEATURE_GCM)
    case PSA_ALG_AEAD_WITH_SHORTENED_TAG(PSA_ALG_GCM, 0):
      tag_len = PSA_ALG_AEAD_GET_TAG_LENGTH(operation->alg);
      if (tag_len < 4 || tag_len > 16) {
        return PSA_ERROR_INVALID_ARGUMENT;
      }
      operation->ad_len = ad_length;
      operation->plaintext_len = plaintext_length;
      operation->lens_set = true;
      break;
#endif
#if defined(SLI_PSA_DRIVER_FEATURE_CHACHAPOLY)
    case PSA_ALG_AEAD_WITH_SHORTENED_TAG(PSA_ALG_CHACHA20_POLY1305, 0):
      tag_len = PSA_ALG_AEAD_GET_TAG_LENGTH(operation->alg);
      if (tag_len != 16) {
        return PSA_ERROR_INVALID_ARGUMENT;
      }
      operation->ad_len = ad_length;
      operation->plaintext_len = plaintext_length;
      operation->lens_set = true;
      break;
#endif // SLI_PSA_DRIVER_FEATURE_CHACHAPOLY
    default:
      return PSA_ERROR_BAD_STATE;
  }

  return PSA_SUCCESS;
#else //SLI_PSA_DRIVER_FEATURE_CCM || SLI_PSA_DRIVER_FEATURE_GCM || SLI_PSA_DRIVER_FEATURE_CHACHAPOLY
  (void) operation;
  (void) ad_length;
  (void) plaintext_length;
  return PSA_ERROR_NOT_SUPPORTED;
#endif //SLI_PSA_DRIVER_FEATURE_CCM || SLI_PSA_DRIVER_FEATURE_GCM || SLI_PSA_DRIVER_FEATURE_CHACHAPOLY
}

psa_status_t sli_hostcrypto_transparent_aead_set_nonce(
  sli_hostcrypto_transparent_aead_operation_t *operation,
  const uint8_t *nonce,
  size_t nonce_size)
{
#if defined(SLI_PSA_DRIVER_FEATURE_CCM) || defined(SLI_PSA_DRIVER_FEATURE_GCM) \
  || defined(SLI_PSA_DRIVER_FEATURE_CHACHAPOLY)

  if (operation == NULL || nonce == NULL) {
    return PSA_ERROR_INVALID_ARGUMENT;
  }

  // aead context was set previously
  if (operation->aead_ctx.dma.regs != 0) {
    return PSA_ERROR_BAD_STATE;
  }

  if (operation->iv_length != 0) {
    return PSA_ERROR_BAD_STATE;
  }

  // Validate operation.
  switch (PSA_ALG_AEAD_WITH_SHORTENED_TAG(operation->alg, 0)) {
#if defined(SLI_PSA_DRIVER_FEATURE_GCM)
    case PSA_ALG_AEAD_WITH_SHORTENED_TAG(PSA_ALG_GCM, 0):
    {
      if (nonce_size != 12) {
        return PSA_ERROR_NOT_SUPPORTED;
      }
      break;
    }
#endif
#if defined(SLI_PSA_DRIVER_FEATURE_CCM)
    case PSA_ALG_AEAD_WITH_SHORTENED_TAG(PSA_ALG_CCM, 0):
    {
      if (nonce_size < 7 || nonce_size > 13) {
        return PSA_ERROR_INVALID_ARGUMENT;
      }
      break;
    }
#endif
#if defined(SLI_PSA_DRIVER_FEATURE_CHACHAPOLY)
    case PSA_ALG_AEAD_WITH_SHORTENED_TAG(PSA_ALG_CHACHA20_POLY1305, 0):
    {
      // Chachapoly only supports Nonce size of 12
      if (nonce_size != 12) {
        return PSA_ERROR_INVALID_ARGUMENT;
      }
      break;
    }
#endif
    default:
      return PSA_ERROR_NOT_SUPPORTED;
  }

  memcpy(operation->iv, nonce, nonce_size);
  operation->iv_length = nonce_size;

  return PSA_SUCCESS;

#else //SLI_PSA_DRIVER_FEATURE_CCM || SLI_PSA_DRIVER_FEATURE_GCM || SLI_PSA_DRIVER_FEATURE_CHACHAPOLY
  (void) operation;
  (void) nonce;
  (void) nonce_size;
  return PSA_ERROR_NOT_SUPPORTED;
#endif
}

#if defined(SLI_PSA_DRIVER_FEATURE_CCM) || defined(SLI_PSA_DRIVER_FEATURE_GCM) \
  || defined(SLI_PSA_DRIVER_FEATURE_CHACHAPOLY)
static psa_status_t sli_hostcrypto_transparent_aead_create(
  sli_hostcrypto_transparent_aead_operation_t *operation)
{
  int sx_status = SX_ERR_UNITIALIZED_OBJ;

  // aead context was set previously
  if (operation->aead_ctx.dma.regs
      && (operation->aead_ctx.dma.dmamem.cfg & get_aead_ctx_save_flag(operation->alg))) {
    return PSA_ERROR_BAD_STATE;
  }

  switch (PSA_ALG_AEAD_WITH_SHORTENED_TAG(operation->alg, 0)) {
#if defined(SLI_PSA_DRIVER_FEATURE_GCM)
    case PSA_ALG_AEAD_WITH_SHORTENED_TAG(PSA_ALG_GCM, 0):
    {
      if (operation->iv_length != 12) {
        return PSA_ERROR_NOT_SUPPORTED;
      }
      if (sli_sxsymcrypt_lock_cryptomaster_selection(
            SLI_SXSYMCRYPT_CRYPTOMASTER_HOSTSYMCRYPTO, false)) {
        return PSA_ERROR_SERVICE_FAILURE;
      }
      if (operation->direction == SLI_HOSTCRYPTO_ENCRYPT) {
        sx_status = sx_aead_create_aesgcm_enc(&operation->aead_ctx,
                                              &operation->key_ref,
                                              (const char *) operation->iv);
      } else if (operation->direction == SLI_HOSTCRYPTO_DECRYPT) {
        sx_status = sx_aead_create_aesgcm_dec(&operation->aead_ctx,
                                              &operation->key_ref,
                                              (const char *) operation->iv);
      }
      if (sli_sxsymcrypt_unlock_cryptomaster_selection()) {
        return PSA_ERROR_SERVICE_FAILURE;
      }
      if (sx_status != SX_OK) {
        return PSA_ERROR_HARDWARE_FAILURE;
      }
      break;
    }
#endif
#if defined(SLI_PSA_DRIVER_FEATURE_CCM)
    case PSA_ALG_AEAD_WITH_SHORTENED_TAG(PSA_ALG_CCM, 0):
    {
      if (operation->iv_length < 7 || operation->iv_length > 13) {
        return PSA_ERROR_INVALID_ARGUMENT;
      }
      if (!operation->lens_set) {
        return PSA_ERROR_BAD_STATE;
      }
      if (sli_sxsymcrypt_lock_cryptomaster_selection(
            SLI_SXSYMCRYPT_CRYPTOMASTER_HOSTSYMCRYPTO, false)) {
        return PSA_ERROR_SERVICE_FAILURE;
      }
      if (operation->direction == SLI_HOSTCRYPTO_ENCRYPT) {
        sx_status = sx_aead_create_aesccm_enc(&operation->aead_ctx,
                                              &operation->key_ref,
                                              (const char *) operation->iv,
                                              operation->iv_length,
                                              PSA_ALG_AEAD_GET_TAG_LENGTH(operation->alg),
                                              operation->ad_len,
                                              operation->plaintext_len);
      } else if (operation->direction == SLI_HOSTCRYPTO_DECRYPT) {
        sx_status = sx_aead_create_aesccm_dec(&operation->aead_ctx,
                                              &operation->key_ref,
                                              (const char *) operation->iv,
                                              operation->iv_length,
                                              PSA_ALG_AEAD_GET_TAG_LENGTH(operation->alg),
                                              operation->ad_len,
                                              operation->plaintext_len);
      }
      if (sli_sxsymcrypt_unlock_cryptomaster_selection()) {
        return PSA_ERROR_SERVICE_FAILURE;
      }
      if (sx_status != SX_OK) {
        return PSA_ERROR_HARDWARE_FAILURE;
      }
      break;
    }
#endif
#if defined(SLI_PSA_DRIVER_FEATURE_CHACHAPOLY)
    case PSA_ALG_AEAD_WITH_SHORTENED_TAG(PSA_ALG_CHACHA20_POLY1305, 0):
    {
      if (operation->iv_length != 12) {
        return PSA_ERROR_NOT_SUPPORTED;
      }
      if (sli_sxsymcrypt_lock_cryptomaster_selection(
            SLI_SXSYMCRYPT_CRYPTOMASTER_HOSTSYMCRYPTO, false) != SL_STATUS_OK) {
        return PSA_ERROR_SERVICE_FAILURE;
      }
      if (operation->direction == SLI_HOSTCRYPTO_ENCRYPT) {
        sx_status = sx_aead_create_chacha20poly1305_enc(&operation->aead_ctx,
                                                        &operation->key_ref,
                                                        (const char *) operation->iv);
      } else if (operation->direction == SLI_HOSTCRYPTO_DECRYPT) {
        sx_status = sx_aead_create_chacha20poly1305_dec(&operation->aead_ctx,
                                                        &operation->key_ref,
                                                        (const char *) operation->iv);
      }
      if (sli_sxsymcrypt_unlock_cryptomaster_selection()) {
        return PSA_ERROR_SERVICE_FAILURE;
      }
      if (sx_status != SX_OK) {
        return PSA_ERROR_HARDWARE_FAILURE;
      }
      break;
    }
#endif
    default:
      return PSA_ERROR_NOT_SUPPORTED;
  }
  return PSA_SUCCESS;
}
#endif // SLI_PSA_DRIVER_FEATURE_CCM || SLI_PSA_DRIVER_FEATURE_GCM || SLI_PSA_DRIVER_FEATURE_CHACHAPOLY

psa_status_t sli_hostcrypto_transparent_aead_update_ad(
  sli_hostcrypto_transparent_aead_operation_t *operation,
  const uint8_t *input,
  size_t input_length)
{
#if defined(SLI_PSA_DRIVER_FEATURE_CCM) || defined(SLI_PSA_DRIVER_FEATURE_GCM) \
  || defined(SLI_PSA_DRIVER_FEATURE_CHACHAPOLY)
  if (operation == NULL
      || (input == NULL && input_length > 0)) {
    return PSA_ERROR_INVALID_ARGUMENT;
  }

  if (operation->alg == 0) {
    return PSA_ERROR_BAD_STATE;
  }

  if (operation->processed_len > 0) {
    return PSA_ERROR_BAD_STATE;
  }

  // No additional data.
  if (input_length == 0) {
    return PSA_SUCCESS;
  }

  if (operation->lens_set) {
    if ((input_length + operation->processed_ad) > operation->ad_len) {
      return PSA_ERROR_INVALID_ARGUMENT;
    }
  }

  if (operation->processed_ad == 0) {
    psa_status_t psa_status = sli_hostcrypto_transparent_aead_create(operation);
    if (psa_status != PSA_SUCCESS) {
      return psa_status;
    }
  }
  int sx_status;

  if (operation->aead_ctx.dma.dmamem.cfg & get_aead_ctx_save_flag(operation->alg)) {
    if (sli_sxsymcrypt_lock_cryptomaster_selection(
          SLI_SXSYMCRYPT_CRYPTOMASTER_HOSTSYMCRYPTO, false)) {
      return PSA_ERROR_SERVICE_FAILURE;
    }
    sx_status = sx_aead_resume_state(&operation->aead_ctx);
    if (sli_sxsymcrypt_unlock_cryptomaster_selection()) {
      return PSA_ERROR_SERVICE_FAILURE;
    }
    if (sx_status != SX_OK) {
      return PSA_ERROR_HARDWARE_FAILURE;
    }
  }

  sx_status = sx_aead_feed_aad(&operation->aead_ctx,
                               (const char *) input,
                               input_length);
  if (sx_status != SX_OK) {
    return PSA_ERROR_HARDWARE_FAILURE;
  }

  if (operation->plaintext_len == 0) {
    if ((input_length + operation->processed_ad) == operation->ad_len) {
      // Can't use context saving if plaintext length is zero and this is the
      // last AAD input
      operation->processed_ad += input_length;
      return PSA_SUCCESS;
    }
  }

  if (input_length + operation->processed_ad < SLI_HOSTCRYPTO_AEAD_BLOCK_SIZE) {
    // Data will be processed only once we have a full block. sxsymcrypt
    // stores the intermediate data in aead context.
    operation->processed_ad += input_length;
    return PSA_SUCCESS;
  }

  sx_status = sx_aead_save_state(&operation->aead_ctx);
  if (sx_status != SX_OK) {
    return PSA_ERROR_HARDWARE_FAILURE;
  }

  sx_status = sx_aead_wait(&operation->aead_ctx);
  if (sx_status != SX_OK) {
    return PSA_ERROR_HARDWARE_FAILURE;
  }

  operation->processed_ad += input_length;

  return PSA_SUCCESS;
#else //SLI_PSA_DRIVER_FEATURE_CCM || SLI_PSA_DRIVER_FEATURE_GCM || SLI_PSA_DRIVER_FEATURE_CHACHAPOLY

  (void) operation;
  (void) input;
  (void) input_length;
  return PSA_ERROR_NOT_SUPPORTED;
#endif //SLI_PSA_DRIVER_FEATURE_CCM || SLI_PSA_DRIVER_FEATURE_GCM || SLI_PSA_DRIVER_FEATURE_CHACHAPOLY
}

psa_status_t sli_hostcrypto_transparent_aead_update(
  sli_hostcrypto_transparent_aead_operation_t *operation,
  const uint8_t *input,
  size_t input_length,
  uint8_t *output,
  size_t output_size,
  size_t *output_length)
{
#if defined(SLI_PSA_DRIVER_FEATURE_CCM) || defined(SLI_PSA_DRIVER_FEATURE_GCM) \
  || defined(SLI_PSA_DRIVER_FEATURE_CHACHAPOLY)

  if (output_size < input_length) {
    return PSA_ERROR_BUFFER_TOO_SMALL;
  }

  if (operation == NULL
      || ((input == NULL || output == NULL) && input_length > 0)
      || output_length == NULL) {
    return PSA_ERROR_INVALID_ARGUMENT;
  }

  if (operation->alg == 0) {
    return PSA_ERROR_BAD_STATE;
  }

  size_t aead_block_size = get_aead_block_size(operation->alg);

  if (operation->lens_set) {
    if (operation->processed_ad != operation->ad_len) {
      return PSA_ERROR_INVALID_ARGUMENT;
    }
    if ((operation->processed_len + input_length) > operation->plaintext_len) {
      return PSA_ERROR_INVALID_ARGUMENT;
    }
  }

  // Check variable overflow
  if (operation->processed_len > 0xFFFFFFFF - input_length) {
    return PSA_ERROR_INVALID_ARGUMENT;
  }

  *output_length = 0;

  if (input_length == 0) {
    return PSA_SUCCESS;
  }

  if (operation->processed_ad == 0 && operation->processed_len == 0) {
    psa_status_t psa_status = sli_hostcrypto_transparent_aead_create(operation);
    if (psa_status != PSA_SUCCESS) {
      return psa_status;
    }
  }
  int sx_status = SX_ERR_UNITIALIZED_OBJ;

  size_t bytes_left_in_block = aead_block_size - (operation->processed_len % aead_block_size);
  size_t actual_output_length = 0;

  uint8_t *block = get_aead_block(operation);

  // If input can fit in the block then just copy it to the block
  if (input_length < bytes_left_in_block) {
    memcpy(&block[operation->processed_len % aead_block_size],
           input,
           input_length);
    operation->processed_len += input_length;
    *output_length = actual_output_length;
    return PSA_SUCCESS;
  }

  // We have atleast the block we saved plus the input left
  size_t output_blocks = 1;
  if (input_length > bytes_left_in_block) {
    // plus however many full blocks are left over after filling the block
    output_blocks += (input_length - bytes_left_in_block) / aead_block_size;
  }
  if (output_size < (output_blocks * aead_block_size)) {
    return PSA_ERROR_BUFFER_TOO_SMALL;
  }

  // Our drivers only support full or no overlap between input and output
  // buffers. So in the case of partial overlap, copy the input buffer into
  // the output buffer and process it in place as if the buffers fully
  // overlapped.
  if ((output > input) && (output < (input + input_length))) {
    if (output_size < input_length) {
      return PSA_ERROR_INVALID_ARGUMENT;
    }

    memmove(output, input, input_length);
    input = output;
  }

  if (bytes_left_in_block != aead_block_size) {
    // Read in up to full streaming input block.
    memcpy(&block[operation->processed_len % aead_block_size],
           input,
           bytes_left_in_block);

#if defined(SLI_PSA_DRIVER_FEATURE_CCM)
    if (PSA_ALG_AEAD_WITH_SHORTENED_TAG(operation->alg,
                                        0) == PSA_ALG_AEAD_WITH_SHORTENED_TAG(PSA_ALG_CCM, 0)) {
      // If this is the last input and we have a complete block then save it
      // for future processing
      if ((operation->processed_len + bytes_left_in_block) == operation->plaintext_len) {
        operation->processed_len += bytes_left_in_block;
        return PSA_SUCCESS;
      }
    }
#endif

    // Resume operation if context was saved
    if (operation->aead_ctx.dma.dmamem.cfg & get_aead_ctx_save_flag(operation->alg)) {
      if (sli_sxsymcrypt_lock_cryptomaster_selection(
            SLI_SXSYMCRYPT_CRYPTOMASTER_HOSTSYMCRYPTO, false)) {
        return PSA_ERROR_SERVICE_FAILURE;
      }
      sx_status = sx_aead_resume_state(&operation->aead_ctx);
      if (sli_sxsymcrypt_unlock_cryptomaster_selection()) {
        return PSA_ERROR_SERVICE_FAILURE;
      }
      if (sx_status != SX_OK) {
        return PSA_ERROR_HARDWARE_FAILURE;
      }
    }

    // Process block first
    sx_status = sx_aead_crypt(&operation->aead_ctx,
                              (const char *) block,
                              aead_block_size,
                              (char *) output);
    if (sx_status != SX_OK) {
      return PSA_ERROR_HARDWARE_FAILURE;
    }
    sx_status = sx_aead_save_state(&operation->aead_ctx);
    if (sx_status != SX_OK) {
      return PSA_ERROR_HARDWARE_FAILURE;
    }
    sx_status = sx_aead_wait(&operation->aead_ctx);
    if (sx_status != SX_OK) {
      return PSA_ERROR_HARDWARE_FAILURE;
    }
    output += aead_block_size;
    actual_output_length += aead_block_size;
    input += bytes_left_in_block;
    input_length -= bytes_left_in_block;
    operation->processed_len += bytes_left_in_block;
  }

  // Input is not final input
  if (input_length >= aead_block_size) {
    // CCM Multipart finish will hardfault without input data, so if remaining
    // input is the final input, then store the last block
#if defined(SLI_PSA_DRIVER_FEATURE_CCM)
    if (PSA_ALG_AEAD_WITH_SHORTENED_TAG(operation->alg,
                                        0) == PSA_ALG_AEAD_WITH_SHORTENED_TAG(PSA_ALG_CCM, 0)) {
      if ((operation->processed_len + input_length) == operation->plaintext_len
          && operation->plaintext_len % aead_block_size == 0) {
        memcpy(block, input + (input_length - aead_block_size), aead_block_size);
        operation->processed_len += aead_block_size;
        input_length -= aead_block_size;
        if (input_length == 0) {
          return PSA_SUCCESS;
        }
      }
    }
#endif
    size_t operation_size = (input_length / aead_block_size) * aead_block_size;

    if (operation_size > 0) {
      if (operation->aead_ctx.dma.dmamem.cfg & get_aead_ctx_save_flag(operation->alg)) {
        if (sli_sxsymcrypt_lock_cryptomaster_selection(
              SLI_SXSYMCRYPT_CRYPTOMASTER_HOSTSYMCRYPTO, false)) {
          return PSA_ERROR_SERVICE_FAILURE;
        }
        sx_status = sx_aead_resume_state(&operation->aead_ctx);
        if (sli_sxsymcrypt_unlock_cryptomaster_selection()) {
          return PSA_ERROR_SERVICE_FAILURE;
        }
        if (sx_status != SX_OK) {
          return PSA_ERROR_HARDWARE_FAILURE;
        }
      }

      sx_status = sx_aead_crypt(&operation->aead_ctx,
                                (const char *) input,
                                operation_size,
                                (char *) output);
      if (sx_status != SX_OK) {
        return PSA_ERROR_HARDWARE_FAILURE;
      }
      sx_status = sx_aead_save_state(&operation->aead_ctx);
      if (sx_status != SX_OK) {
        return PSA_ERROR_HARDWARE_FAILURE;
      }
      sx_status = sx_aead_wait(&operation->aead_ctx);
      if (sx_status != SX_OK) {
        return PSA_ERROR_HARDWARE_FAILURE;
      }
    }

    input += operation_size;
    input_length -= operation_size;
    operation->processed_len += operation_size;
    actual_output_length += operation_size;
    output += operation_size;
  }

  // Move remaining input to block
  if (input_length > 0) {
    memcpy(&block[operation->processed_len % aead_block_size],
           input,
           input_length);

    operation->processed_len += input_length;
  }

  *output_length = actual_output_length;

  return PSA_SUCCESS;
#else //SLI_PSA_DRIVER_FEATURE_CCM || SLI_PSA_DRIVER_FEATURE_GCM || SLI_PSA_DRIVER_FEATURE_CHACHAPOLY
  (void) operation;
  (void) input;
  (void) input_length;
  (void) output;
  (void) output_size;
  (void) output_length;
  return PSA_ERROR_NOT_SUPPORTED;
#endif //SLI_PSA_DRIVER_FEATURE_CCM || SLI_PSA_DRIVER_FEATURE_GCM || SLI_PSA_DRIVER_FEATURE_CHACHAPOLY
}

psa_status_t sli_hostcrypto_transparent_aead_finish(
  sli_hostcrypto_transparent_aead_operation_t *operation,
  uint8_t *ciphertext,
  size_t ciphertext_size,
  size_t *ciphertext_length,
  uint8_t *tag,
  size_t tag_size,
  size_t *tag_length)
{
#if defined(SLI_PSA_DRIVER_FEATURE_CCM) || defined(SLI_PSA_DRIVER_FEATURE_GCM) \
  || defined(SLI_PSA_DRIVER_FEATURE_CHACHAPOLY)

  if (operation == NULL) {
    return PSA_ERROR_INVALID_ARGUMENT;
  }

  uint32_t tag_len;

  tag_len = PSA_ALG_AEAD_GET_TAG_LENGTH(operation->alg);

  size_t aead_block_size = get_aead_block_size(operation->alg);

  if (tag_size < tag_len) {
    return PSA_ERROR_BUFFER_TOO_SMALL;
  }

  if (operation->direction != SLI_HOSTCRYPTO_ENCRYPT) {
    return PSA_ERROR_INVALID_ARGUMENT;
  }

  if (ciphertext_size < operation->processed_len % aead_block_size) {
    return PSA_ERROR_BUFFER_TOO_SMALL;
  }

  if (operation->lens_set) {
    if (operation->processed_ad != operation->ad_len) {
      return PSA_ERROR_INVALID_ARGUMENT;
    }
    if (operation->processed_len != operation->plaintext_len) {
      return PSA_ERROR_INVALID_ARGUMENT;
    }
  }

  if (operation->processed_ad == 0 && operation->processed_len == 0) {
#if defined(SLI_PSA_DRIVER_FEATURE_CCM)
    if (PSA_ALG_AEAD_WITH_SHORTENED_TAG(operation->alg,
                                        0) == PSA_ALG_AEAD_WITH_SHORTENED_TAG(PSA_ALG_CCM, 0)) {
      // Since no AAD and plaintext has been processed, operation has not been
      // created. So set plaintext and AAD lengths to zero and create operation.
      operation->plaintext_len = 0;
      operation->ad_len = 0;
      operation->lens_set = true;
    }
#endif
    psa_status_t psa_status = sli_hostcrypto_transparent_aead_create(operation);
    if (psa_status != PSA_SUCCESS) {
      return psa_status;
    }
  }

  int sx_status = SX_ERR_UNITIALIZED_OBJ;

  uint8_t *block = get_aead_block(operation);

  if (operation->aead_ctx.dma.dmamem.cfg & get_aead_ctx_save_flag(operation->alg)) {
    if (sli_sxsymcrypt_lock_cryptomaster_selection(
          SLI_SXSYMCRYPT_CRYPTOMASTER_HOSTSYMCRYPTO, false)) {
      return PSA_ERROR_SERVICE_FAILURE;
    }
    sx_status = sx_aead_resume_state(&operation->aead_ctx);
    if (sli_sxsymcrypt_unlock_cryptomaster_selection()) {
      return PSA_ERROR_SERVICE_FAILURE;
    }
    if (sx_status != SX_OK) {
      return PSA_ERROR_HARDWARE_FAILURE;
    }
  }
  if (operation->processed_len % aead_block_size != 0) {
    sx_status = sx_aead_crypt(&operation->aead_ctx,
                              (const char *) block,
                              operation->processed_len % aead_block_size,
                              (char *) ciphertext);
    if (sx_status != SX_OK) {
      return PSA_ERROR_HARDWARE_FAILURE;
    }
  }
#if defined(SLI_PSA_DRIVER_FEATURE_CCM)
  else {
    if (PSA_ALG_AEAD_WITH_SHORTENED_TAG(operation->alg,
                                        0) == PSA_ALG_AEAD_WITH_SHORTENED_TAG(PSA_ALG_CCM, 0)) {
      if (operation->processed_len) {
        // If processed length is multiple of 16 then we have saved the last block
        // so process that now
        if (ciphertext_size < 16) {
          return PSA_ERROR_BUFFER_TOO_SMALL;
        }
        sx_status = sx_aead_crypt(&operation->aead_ctx,
                                  (const char *) block,
                                  16,
                                  (char *) ciphertext);
        if (sx_status != SX_OK) {
          return PSA_ERROR_HARDWARE_FAILURE;
        }
      }
    }
  }
#endif

#if defined(SLI_PSA_DRIVER_FEATURE_GCM)
  if (PSA_ALG_AEAD_WITH_SHORTENED_TAG(operation->alg,
                                      0) == PSA_ALG_AEAD_WITH_SHORTENED_TAG(PSA_ALG_GCM, 0)) {
    sx_status = sx_aead_truncate_tag(&operation->aead_ctx, tag_len);
    if (sx_status != SX_OK) {
      return PSA_ERROR_HARDWARE_FAILURE;
    }
  }
#endif

  sx_status = sx_aead_produce_tag(&operation->aead_ctx, (char *) tag);
  if (sx_status != SX_OK) {
    return PSA_ERROR_HARDWARE_FAILURE;
  }

  sx_status = sx_aead_wait(&operation->aead_ctx);
  if (sx_status != SX_OK) {
    return PSA_ERROR_HARDWARE_FAILURE;
  }

#if defined(SLI_PSA_DRIVER_FEATURE_CCM)
  if ((operation->processed_len % 16 == 0)
      && (PSA_ALG_AEAD_WITH_SHORTENED_TAG(operation->alg,
                                          0) == PSA_ALG_AEAD_WITH_SHORTENED_TAG(PSA_ALG_CCM, 0))) {
    *ciphertext_length = 16;
  } else
#endif
  {
    *ciphertext_length = operation->processed_len % aead_block_size;
  }
  *tag_length = tag_len;

  return PSA_SUCCESS;

#else //SLI_PSA_DRIVER_FEATURE_CCM || SLI_PSA_DRIVER_FEATURE_GCM || SLI_PSA_DRIVER_FEATURE_CHACHAPOLY
  (void) operation;
  (void) ciphertext;
  (void) ciphertext_size;
  (void) ciphertext_length;
  (void) tag;
  (void) tag_size;
  (void) tag_length;
  return PSA_ERROR_NOT_SUPPORTED;
#endif //SLI_PSA_DRIVER_FEATURE_CCM || SLI_PSA_DRIVER_FEATURE_GCM) || SLI_PSA_DRIVER_FEATURE_CHACHAPOLY
}

psa_status_t sli_hostcrypto_transparent_aead_verify(
  sli_hostcrypto_transparent_aead_operation_t *operation,
  uint8_t *plaintext,
  size_t plaintext_size,
  size_t *plaintext_length,
  const uint8_t *tag,
  size_t tag_length)
{
#if defined(SLI_PSA_DRIVER_FEATURE_CCM) || defined(SLI_PSA_DRIVER_FEATURE_GCM) \
  || defined(SLI_PSA_DRIVER_FEATURE_CHACHAPOLY)

  if (operation == NULL) {
    return PSA_ERROR_INVALID_ARGUMENT;
  }

  if (tag == NULL || tag_length == 0) {
    return PSA_ERROR_INVALID_SIGNATURE;
  }

  size_t aead_block_size = get_aead_block_size(operation->alg);

  if (plaintext_size < operation->processed_len % aead_block_size) {
    return PSA_ERROR_BUFFER_TOO_SMALL;
  }

  if (operation->lens_set) {
    if (operation->processed_ad != operation->ad_len) {
      return PSA_ERROR_INVALID_ARGUMENT;
    }
    if (operation->processed_len != operation->plaintext_len) {
      return PSA_ERROR_INVALID_ARGUMENT;
    }
  }

  if (operation->direction != SLI_HOSTCRYPTO_DECRYPT) {
    return PSA_ERROR_INVALID_ARGUMENT;
  }

  if (operation->processed_ad == 0 && operation->processed_len == 0) {
#if defined(SLI_PSA_DRIVER_FEATURE_CCM)
    if (PSA_ALG_AEAD_WITH_SHORTENED_TAG(operation->alg,
                                        0) == PSA_ALG_AEAD_WITH_SHORTENED_TAG(PSA_ALG_CCM, 0)) {
      // Since no AAD and plaintext has been processed, operation has not been
      // created. So set plaintext and AAD lengths to zero and create operation.
      operation->plaintext_len = 0;
      operation->ad_len = 0;
      operation->lens_set = true;
    }
#endif
    psa_status_t psa_status = sli_hostcrypto_transparent_aead_create(operation);
    if (psa_status != PSA_SUCCESS) {
      return psa_status;
    }
  }

  int sx_status = SX_ERR_UNITIALIZED_OBJ;

  uint8_t *block = get_aead_block(operation);

  if (operation->aead_ctx.dma.dmamem.cfg & get_aead_ctx_save_flag(operation->alg)) {
    if (sli_sxsymcrypt_lock_cryptomaster_selection(
          SLI_SXSYMCRYPT_CRYPTOMASTER_HOSTSYMCRYPTO, false)) {
      return PSA_ERROR_SERVICE_FAILURE;
    }
    sx_status = sx_aead_resume_state(&operation->aead_ctx);
    if (sli_sxsymcrypt_unlock_cryptomaster_selection()) {
      return PSA_ERROR_SERVICE_FAILURE;
    }
    if (sx_status != SX_OK) {
      return PSA_ERROR_HARDWARE_FAILURE;
    }
  }
  if (operation->processed_len % aead_block_size != 0) {
    sx_status = sx_aead_crypt(&operation->aead_ctx,
                              (const char *) block,
                              operation->processed_len % aead_block_size,
                              (char *) plaintext);
    if (sx_status != SX_OK) {
      return PSA_ERROR_HARDWARE_FAILURE;
    }
  }
#if defined(SLI_PSA_DRIVER_FEATURE_CCM)
  else {
    if (PSA_ALG_AEAD_WITH_SHORTENED_TAG(operation->alg,
                                        0) == PSA_ALG_AEAD_WITH_SHORTENED_TAG(PSA_ALG_CCM, 0)) {
      if (operation->processed_len) {
        // If processed length is multiple of 16 then we have saved the last block
        // so process that now
        if (plaintext_size < 16) {
          return PSA_ERROR_BUFFER_TOO_SMALL;
        }
        sx_status = sx_aead_crypt(&operation->aead_ctx,
                                  (const char *) block,
                                  16,
                                  (char *) plaintext);
        if (sx_status != SX_OK) {
          return PSA_ERROR_HARDWARE_FAILURE;
        }
      }
    }
  }
#endif

#if defined(SLI_PSA_DRIVER_FEATURE_GCM)
  if (PSA_ALG_AEAD_WITH_SHORTENED_TAG(operation->alg,
                                      0) == PSA_ALG_AEAD_WITH_SHORTENED_TAG(PSA_ALG_GCM, 0)) {
    sx_status = sx_aead_truncate_tag(&operation->aead_ctx, tag_length);
    if (sx_status != SX_OK) {
      return PSA_ERROR_HARDWARE_FAILURE;
    }
  }
#endif

  sx_status = sx_aead_verify_tag(&operation->aead_ctx, (const char *) tag);
  if (sx_status != SX_OK) {
    return PSA_ERROR_HARDWARE_FAILURE;
  }
  sx_status = sx_aead_wait(&operation->aead_ctx);
  if (sx_status == SX_ERR_INVALID_TAG) {
    return PSA_ERROR_INVALID_SIGNATURE;
  }
  if (sx_status != SX_OK) {
    return PSA_ERROR_HARDWARE_FAILURE;
  }

#if defined(SLI_PSA_DRIVER_FEATURE_CCM)
  if ((operation->processed_len % 16 == 0)
      && (PSA_ALG_AEAD_WITH_SHORTENED_TAG(operation->alg,
                                          0) == PSA_ALG_AEAD_WITH_SHORTENED_TAG(PSA_ALG_CCM, 0))) {
    *plaintext_length = 16;
  } else
#endif
  {
    *plaintext_length = operation->processed_len % aead_block_size;
  }

  return PSA_SUCCESS;
#else //SLI_PSA_DRIVER_FEATURE_CCM || SLI_PSA_DRIVER_FEATURE_GCM || SLI_PSA_DRIVER_FEATURE_CHACHAPOLY
  (void) operation;
  (void) plaintext;
  (void) plaintext_size;
  (void) plaintext_length;
  (void) tag;
  (void) tag_length;
  return PSA_ERROR_NOT_SUPPORTED;
#endif //SLI_PSA_DRIVER_FEATURE_CCM || SLI_PSA_DRIVER_FEATURE_GCM || SLI_PSA_DRIVER_FEATURE_CHACHAPOLY
}

psa_status_t sli_hostcrypto_transparent_aead_abort(
  sli_hostcrypto_transparent_aead_operation_t *operation)
{
  if (operation == NULL) {
    return PSA_ERROR_INVALID_ARGUMENT;
  }

  memset(operation, 0, sizeof(*operation));
  return PSA_SUCCESS;
}

#endif
