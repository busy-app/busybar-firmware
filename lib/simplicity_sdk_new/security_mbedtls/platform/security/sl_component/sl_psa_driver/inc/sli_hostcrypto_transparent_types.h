/***************************************************************************//**
 * @file
 * @brief PSA Crypto driver types for the HostCryptoSubSystem.
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

#ifndef SLI_HOSTCRYPTO_TRANSPARENT_TYPES_H
#define SLI_HOSTCRYPTO_TRANSPARENT_TYPES_H

#include "sli_psa_driver_features.h"

#if defined(SLI_MBEDTLS_DEVICE_HC)

#include "psa/crypto_driver_common.h"
#include "sxsymcrypt/keyref.h"
#include "sxsymcrypt/blkcipher.h"
#include "sxsymcrypt/hash.h"
#include <stdbool.h>

// -----------------------------------------------------------------------------
// Type Definitions
#define PSA_KEY_LOCATION_SLI_HOSTCRYPTO_TRANSPARENT  (PSA_KEY_LOCATION_VENDOR_FLAG | (psa_key_location_t) 0x3UL)

typedef enum {
  SLI_HOSTCRYPTO_ENCRYPT = 0,
  SLI_HOSTCRYPTO_DECRYPT = 1
} sli_encrypt_direction_t;

typedef struct {
  sli_encrypt_direction_t direction;  ///< Cipher direction (encrypt/decrypt)
  psa_algorithm_t alg;                ///< Algorithm (cipher and mode of operation)
  struct sxkeyref key_ref;            ///< Key reference structure
  struct sxblkcipher cipher;          ///< Cipher operation
  uint8_t block[16];                  ///< Block for input caching
  uint8_t iv[16];                     ///< IV
  size_t processed_length;            ///< Number of bytes processed
} sli_hostcrypto_transparent_cipher_operation_t;

typedef struct {
  sli_encrypt_direction_t direction;        ///< Encrypt/Decrypt
  psa_algorithm_t alg;                      ///< Algorithm
  struct sxkeyref key_ref;                  ///< Key buffer
  struct sxaead aead_ctx;                   ///< AEAD operation
  bool lens_set;                            ///< Plaintext and AAD lengths are set
  size_t ad_len;                            ///< Total length of additional data
  size_t plaintext_len;                     ///< Total length of plaintext data
  uint8_t iv[16];                           ///< IV buffer
  uint8_t iv_length;                        ///< Length of IV
  size_t processed_ad;                      ///< Current additional data length
  size_t processed_len;                     ///< Current encrypted/decrypted message length
  union {
#if defined(SLI_PSA_DRIVER_FEATURE_CCM) || defined(SLI_PSA_DRIVER_FEATURE_GCM)
    uint8_t aes_block[16];                  ///< Input data saved for aes.
#endif
  #if defined(SLI_PSA_DRIVER_FEATURE_CHACHAPOLY)
    uint8_t chacha_block[64];               ///< Input data saved for Chachapoly.
  #endif
  } block;
} sli_hostcrypto_transparent_aead_operation_t;

typedef struct {
  struct sxhash hash_ctx;             ///< Hash operation
  size_t processed_len;               ///< Number of bytes processed
  uint8_t block[128];                 ///< Data block being processed
  size_t digest_size;                 ///< Size of hash digest
  size_t hash_block_size;             ///< Size of hash block
} sli_hostcrypto_transparent_hash_operation_t;

typedef union {
  struct {
    psa_algorithm_t alg;                    ///< MAC type
    struct sxkeyref key_ref;                ///< key buffer
    uint8_t current_block[16];              ///< current and potentially last block
    size_t  current_block_len;              ///< current number of bytes in current block
    struct sxmac cmac_ctx;                  ///< CMAC state context
  } cmac;
  #if defined(SLI_PSA_DRIVER_FEATURE_HMAC)
  struct {
    psa_algorithm_t alg;                    ///< HMAC type
    sli_hostcrypto_transparent_hash_operation_t hash_ctx;  ///< Hash context for multipart HMAC
    uint8_t opad[128];                       ///< opad for use during finalisation
  } hmac;
  #endif
} sli_hostcrypto_transparent_mac_operation_t;

#endif // SLI_MBEDTLS_DEVICE_HC

#endif // SLI_HOSTCRYPTO_TRANSPARENT_TYPES_H
