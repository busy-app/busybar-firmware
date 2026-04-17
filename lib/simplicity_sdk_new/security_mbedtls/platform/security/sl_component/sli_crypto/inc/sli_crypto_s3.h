/***************************************************************************//**
 * @file
 * @brief Hardware accelerated cryptographic defintions specific to series-3
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
#ifndef SLI_CRYPTO_S3_H
#define SLI_CRYPTO_S3_H

#include "sl_status.h"
#include "sl_code_classification.h"
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/// Standard buffer size in bytes
#define SLI_CRYPTO_AES_BLOCK_SIZE  16
/// Location value for keys stored in plaintext
#define SLI_CRYPTO_KEY_LOCATION_PLAINTEXT ((sli_crypto_key_location_t)0x00000000UL)
/// Location value for keys stored in KSU
#define SLI_CRYPTO_KEY_LOCATION_KSU       ((sli_crypto_key_location_t)0x00000001UL)
/// Crypto engine selection value for HostSymCrypt
/**
 * @note
 * Using the HOSTSYMCRYPTO engine from an ISR (Interrupt Service Routine) is not supported.
 * Attempting to use this engine in interrupt context will return SL_STATUS_NOT_SUPPORTED.
 */
#define SLI_CRYPTO_HOSTSYMCRYPTO ((sli_crypto_engine_t)0x00000000UL)
/// Crypto engine selection value for LPWAES
#define SLI_CRYPTO_LPWAES        ((sli_crypto_engine_t)0x00000001UL)
/// The default crypto engine selection is LPWAES
#define SLI_CRYPTO_ENGINE_DEFAULT (SLI_CRYPTO_LPWAES)

/// Primary used for KSU key slot ID
typedef uint32_t sli_crypto_key_slot_t;

/// Used to choose a crypto engine.
/// @ref SLI_CRYPTO_HOSTSYMCRYPTO or
/// @ref SLI_CRYPTO_LPWAES.
typedef uint32_t sli_crypto_engine_t;

/// Key storage location. Can either be
/// @ref SLI_CRYPTO_KEY_LOCATION_PLAINTEXT or
/// @ref SLI_CRYPTO_KEY_LOCATION_KSU.
typedef uint32_t sli_crypto_key_location_t;

/// Describes where the plaintext key is stored
typedef struct {
  uint8_t* pointer; ///< Pointer to a key buffer.
  uint32_t size;    ///< Size of buffer.
} sli_crypto_key_buffer_t;

/// Describes the plaintext key
typedef struct {
  sli_crypto_key_buffer_t buffer; ///< Key buffer.
  uint32_t key_size;              ///< Key size.
} sli_crypto_plaintext_key_t;

typedef struct {
  /// Key storage location.
  sli_crypto_key_location_t location;
  /// Crypto engine.
  sli_crypto_engine_t engine;
  /// Describes key storage location.
  /// @ref sli_crypto_plaintext_key_t is used if @ref sli_crypto_key_location_t.method is @ref SLI_CRYPTO_KEY_LOCATION_PLAINTEXT
  /// while @ref sli_crypto_key_slot_t is used for @ref SLI_CRYPTO_KEY_LOCATION_KSU.
  union {
    sli_crypto_plaintext_key_t plaintext_key;
    sli_crypto_key_slot_t key_slot;
  } key;
  /// Request the SLI Crypto API to yield the executing core while waiting for the crypto engine to complete.
  bool yield;
} sli_crypto_descriptor_t;

/// SLI Crypto descriptor reset values. Some of the values are not initialized.
/// The user will need to initialize the descriptor members for further use in
/// the SLI Crypto API. The purpose of these initialisation values is to set
/// the descriptor objects to a known default device specific state initially
/// when the context object is declared.
#define SLI_CRYPTO_DESCRIPTOR_INIT SLI_CRYPTO_DESCRIPTOR_INIT_PLAINTEXT_KEY(NULL, 0)

#define SLI_CRYPTO_DESCRIPTOR_INIT_PLAINTEXT_KEY(ptr, sz) \
  {                                                       \
    .location = SLI_CRYPTO_KEY_LOCATION_PLAINTEXT,        \
    .engine = SLI_CRYPTO_ENGINE_DEFAULT,                  \
    .key = {                                              \
      .plaintext_key = {                                  \
        .buffer = {                                       \
          .pointer = ptr,                                 \
          .size = sz,                                     \
        },                                                \
        .key_size = sz,                                   \
      },                                                  \
    },                                                    \
    .yield = false,                                       \
  }

/***************************************************************************//**
 * @brief                Generic AES GCM operation
 *
 * @param key_descriptor AES key descriptor
 * @param encrypt        Encrypt operation
 * @param data_in        Input data buffer
 * @param data_len       length of input data
 * @param data_out       Output data buffer
 * @param iv             nonce (initialization vector). Must be 12 bytes.
 * @param aad            Additional authenticated data buffer
 * @param aad_len        length of additional authenticated data
 * @param tag            Tag buffer
 * @param tag_len        length of tag
 *
 * @return               SL_STATUS_OK if successful, relevant status code on error
 ******************************************************************************/
sl_status_t sli_crypto_gcm(sli_crypto_descriptor_t  *key_descriptor,
                           bool                     encrypt,
                           const unsigned char      *data_in,
                           size_t                   data_len,
                           unsigned char            *data_out,
                           const unsigned char      *iv,
                           const unsigned char      *aad,
                           size_t                   aad_len,
                           unsigned char            *tag,
                           size_t                   tag_len);

/***************************************************************************//**
 * @brief                AES GCM encrypt operation
 *
 * @param key_descriptor AES key descriptor
 * @param data_in        Input data buffer
 * @param data_len       length of input data
 * @param data_out       Output data buffer
 * @param iv             nonce (initialization vector). Must be 12 bytes.
 * @param aad            Additional authenticated data buffer
 * @param aad_len        length of additional authenticated data
 * @param tag            Tag buffer
 * @param tag_len        length of tag
 *
 * @return               SL_STATUS_OK if successful, relevant status code on error
 ******************************************************************************/
sl_status_t sli_crypto_gcm_encrypt(sli_crypto_descriptor_t  *key_descriptor,
                                   const unsigned char      *data_in,
                                   size_t                   data_len,
                                   unsigned char            *data_out,
                                   const unsigned char      *iv,
                                   const unsigned char      *aad,
                                   size_t                   aad_len,
                                   unsigned char            *tag,
                                   size_t                   tag_len);

/***************************************************************************//**
 * @brief                AES GCM decrypt operation
 *
 * @param key_descriptor AES key descriptor
 * @param data_in        Input data buffer
 * @param data_len       length of input data
 * @param data_out       Output data buffer
 * @param iv             nonce (initialization vector). Must be 12 bytes.
 * @param aad            Additional authenticated data buffer
 * @param aad_len        length of additional authenticated data
 * @param tag            Tag buffer
 * @param tag_len        length of tag
 *
 * @return               SL_STATUS_OK if successful, relevant status code on error
 ******************************************************************************/
sl_status_t sli_crypto_gcm_decrypt(sli_crypto_descriptor_t  *key_descriptor,
                                   const unsigned char      *data_in,
                                   size_t                   data_len,
                                   unsigned char            *data_out,
                                   const unsigned char      *iv,
                                   const unsigned char      *aad,
                                   size_t                   aad_len,
                                   const unsigned char      *tag,
                                   size_t                   tag_len);

/***************************************************************************//**
 * @brief                Generic AES CCM operation
 *
 * @param key_descriptor AES key descriptor
 * @param encrypt        Encrypt operation
 * @param data_in        Input data buffer
 * @param data_len       length of input data
 * @param data_out       Output data buffer
 * @param iv             nonce (initialization vector)
 * @param iv_len         length of initialization vector
 * @param aad            Additional authenticated data buffer
 * @param aad_len        length of additional authenticated data
 * @param tag            Tag buffer
 * @param tag_len        length of tag
 *
 * @return               SL_STATUS_OK if successful, relevant status code on error
 ******************************************************************************/
SL_CODE_CLASSIFY(SL_CODE_COMPONENT_SLI_CRYPTO, SL_CODE_CLASS_TIME_CRITICAL)
sl_status_t sli_crypto_ccm(sli_crypto_descriptor_t  *key_descriptor,
                           bool                     encrypt,
                           unsigned char            *data_in,
                           size_t                   data_len,
                           unsigned char            *data_out,
                           const unsigned char      *iv,
                           size_t                   iv_len,
                           unsigned char            *aad,
                           size_t                   aad_len,
                           unsigned char            *tag,
                           size_t                   tag_len);

/***************************************************************************//**
 * @brief                AES-CTR block encryption/decryption
 *
 * @param key_descriptor AES key descriptor
 * @param input          16-byte input block
 * @param iv_in          16-byte counter/IV starting value
 * @param iv_out         16-byte counter/IV output after block round
 * @param output         16-byte output block
 *
 * @return               SL_STATUS_OK if successful, relevant status code on error
 ******************************************************************************/
SL_CODE_CLASSIFY(SL_CODE_COMPONENT_SLI_CRYPTO, SL_CODE_CLASS_TIME_CRITICAL)
sl_status_t sli_crypto_ctr (sli_crypto_descriptor_t *key_descriptor,
                            const unsigned char     input[SLI_CRYPTO_AES_BLOCK_SIZE],
                            const unsigned char     iv_in[SLI_CRYPTO_AES_BLOCK_SIZE],
                            volatile unsigned char  iv_out[SLI_CRYPTO_AES_BLOCK_SIZE],
                            volatile unsigned char  output[SLI_CRYPTO_AES_BLOCK_SIZE]);

/***************************************************************************//**
 * @brief                AES-ECB block encryption/decryption
 *
 * @param key_descriptor AES key descriptor
 * @param encrypt        true for encryption, false for decryption
 * @param input          16-byte input block
 * @param output         16-byte output block
 *
 * @return               SL_STATUS_OK if successful, relevant status code on error
 ******************************************************************************/
SL_CODE_CLASSIFY(SL_CODE_COMPONENT_SLI_CRYPTO, SL_CODE_CLASS_TIME_CRITICAL)
sl_status_t sli_crypto_ecb(sli_crypto_descriptor_t *key_descriptor,
                           bool                    encrypt,
                           const unsigned char     input[SLI_CRYPTO_AES_BLOCK_SIZE],
                           volatile unsigned char  output[SLI_CRYPTO_AES_BLOCK_SIZE]);

/***************************************************************************//**
 * @brief                AES-CMAC calculation

 * @param key_descriptor AES key descriptor
 * @param input          Input buffer containing the message to be signed
 * @param length         Amount of bytes in the input buffer
 * @param output         16-byte output block for calculated CMAC
 *
 * @return               SL_STATUS_OK if successful, relevant status code on error
 ******************************************************************************/
SL_CODE_CLASSIFY(SL_CODE_COMPONENT_SLI_CRYPTO, SL_CODE_CLASS_TIME_CRITICAL)
sl_status_t sli_crypto_cmac(sli_crypto_descriptor_t *key_descriptor,
                            const unsigned char     *input,
                            unsigned int            length,
                            volatile unsigned char  output[SLI_CRYPTO_AES_BLOCK_SIZE]);

#ifdef __cplusplus
}
#endif

#endif // SLI_CRYPTO_S3_H
