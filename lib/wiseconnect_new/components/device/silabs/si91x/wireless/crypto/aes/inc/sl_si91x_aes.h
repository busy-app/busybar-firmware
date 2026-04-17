/***************************************************************************/ /**
 * @file
 * @brief SL SI91X AES Header file
 *******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
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

#pragma once
#include "sl_si91x_crypto.h"
#include "sl_status.h"

/******************************************************
 *                    Constants
 ******************************************************/
/**
 * @addtogroup CRYPTO_AES_CONSTANTS
 * @{ 
 */

/**
 * @brief AES BLOCK SIZE of 16 bytes or 128 bits.
 */
#define SL_SI91X_AES_BLOCK_SIZE 16

/**
 * @brief First chunk of data.
 */
#define SL_SI91X_AES_FIRST_CHUNK BIT(0)

/**
 * @brief Middle chunk of data.
 */
#define SL_SI91X_AES_MIDDLE_CHUNK BIT(1)

/**
 * @brief Last chunk of data.
 */
#define SL_SI91X_AES_LAST_CHUNK BIT(2)

/**
 * @brief Enumeration defines the AES modes supported by the SI91X device.
 *
 * This enumeration defines different AES modes supported by the SI91X device,
 * such as AES CBC, AES ECB, and AES CTR modes.
 */
typedef enum {
  SL_SI91X_AES_CBC = 1, ///< AES CBC mode
  SL_SI91X_AES_ECB,     ///< AES ECB mode
  SL_SI91X_AES_CTR      ///< AES CTR mode
} sl_si91x_aes_mode_t;

/**
 * @brief Enumeration defines the AES encryption and decryption types.
 *
 * This enumeration defines the types of AES operations supported, which includes
 * encryption and decryption.
 */
typedef enum {
  SL_SI91X_AES_ENCRYPT = 1, ///< AES encryption
  SL_SI91X_AES_DECRYPT      ///< AES decryption
} sl_si91x_aes_type_t;

/**
 * @brief Enumeration defines the key sizes supported by the AES encryption
 * algorithm on the SI91X device, which includes 128-bit, 192-bit, and 256-bit key sizes.
 */
typedef enum {
  SL_SI91X_AES_KEY_SIZE_128 = 16, ///< Key size of 128 bits
  SL_SI91X_AES_KEY_SIZE_192 = 24, ///< Key size of 192 bits
  SL_SI91X_AES_KEY_SIZE_256 = 32  ///< Key size of 256 bits
} sl_si91x_aes_key_size_t;

/** @} */

/******************************************************
 *                   Type Definitions
 ******************************************************/
/**
 * @addtogroup CRYPTO_AES_TYPES 
 * @{ 
 */

/**
 * @brief Structure defines the AES key configuration for the non-B0 chip versions.
 */
typedef struct {
  uint8_t *key;        ///< Pointer to the key
  uint16_t key_length; ///< Length of the key
} sl_si91x_aes_key_config_a0_t;

/**
 * @brief Structure defines the AES key configuration for B0 chip versions.
 */
typedef struct {
  sl_si91x_crypto_key_type_t key_type;          ///< Key type
  sl_si91x_aes_key_size_t key_size;             ///< Key size
  sl_si91x_crypto_key_slot_t key_slot;          ///< Key slot
  sl_si91x_crypto_wrap_mode_t wrap_iv_mode;     ///< Wrap mode
  uint8_t wrap_iv[SL_SI91X_IV_SIZE];            ///< IV used in SL_SI91X_AES_CBC and SL_SI91X_AES_CTR modes
  uint8_t key_buffer[SL_SI91X_KEY_BUFFER_SIZE]; ///< Buffer to store the key
  uint32_t reserved;                            ///< Reserved for future use
} sl_si91x_aes_key_config_b0_t;

/**
 * @brief Union holds the AES key configuration structures of B0 and non-B0 chip versions.
 */
typedef union {
  sl_si91x_aes_key_config_a0_t a0; ///< Key configuration for non-B0 chip versions
  sl_si91x_aes_key_config_b0_t b0; ///< Key configuration for B0 chip versions
} sl_si91x_aes_key_config_t;

/**
 * @brief Structure defines the AES configuration.
 * 
 * This structure defines AES configuration required for the operation which includes 
 * type of operation, input message, length of input message, key configuration structure, and so on. 
 */
typedef struct {
  sl_si91x_aes_mode_t aes_mode;         ///< AES Mode
  sl_si91x_aes_type_t encrypt_decrypt;  ///< Encryption or decryption
  const uint8_t *msg;                   ///< Pointer to the input message
  uint16_t msg_length;                  ///< Length of the message
  const uint8_t *iv;                    ///< Pointer to the Initialization vector
  sl_si91x_aes_key_config_t key_config; ///< Key configuration
} sl_si91x_aes_config_t;

/**
 * @brief Structure defines the PSA AES Multipart configuration.
 * 
 * This structure defines PSA AES Multipart configuration required for the operation which includes 
 * type of operation, input message, length of input message, key configuration structure, and so on. 
 */
typedef struct {
  sl_si91x_aes_mode_t aes_mode;         ///< AES Mode
  sl_si91x_aes_type_t encrypt_decrypt;  ///< Encryption or decryption
  const uint8_t *msg;                   ///< Pointer to the input message
  uint16_t msg_length;                  ///< Length of the message
  uint8_t iv[SL_SI91X_IV_SIZE];         ///< Initialization vector
  uint8_t iv_flag;                      ///< Flag to indicate whether iv is set or not
  sl_si91x_aes_key_config_t key_config; ///< Key configuration
  uint8_t chunk_flag;                   ///< Flag to indicate chunk number
} sli_si91x_psa_aes_multipart_config_t;

/** @} */

/******************************************************
 *                Function Declarations
 ******************************************************/

/**
 * @addtogroup CRYPTO_AES_FUNCTIONS
 * @{
 */

/***************************************************************************/
/**
 * @brief 
 *   To encrypt or decrypt the message according to the given configuration. This is a blocking API.
 * @param[in] config 
 *   Configuration object of type @ref sl_si91x_aes_config_t
 * @param[out] output 
 *   Buffer to store the output.
 * @note
 *   The maximum length of the message in bytes that can be handled in one go is determined by SLI_SI91X_MAX_DATA_SIZE_IN_BYTES_FOR_AES.
 *   If the message length is not a multiple of SL_SI91X_AES_BLOCK_SIZE, the function returns an error.
 * 
 * @return
 *   sl_status_t.
 * For more information on status codes, refer to 
 * [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_aes(sl_si91x_aes_config_t *config, uint8_t *output);

/***************************************************************************/
/**
 * @brief 
 *   Encrypts or decrypts a chunk of a message (CBC and ECB modes are not supported for multipart operations) according to the given configuration. This is a blocking API.
 * @param[in] config 
 *   Configuration object of type @ref sl_si91x_aes_config_t containing the AES operation parameters.
 * @param[in] chunk_length 
 *   Length of the current chunk to be processed in bytes.
 * @param[in] aes_flags 
 *   Flags indicating the position of the current chunk. Valid values are:- @ref SL_SI91X_AES_FIRST_CHUNK, @ref SL_SI91X_AES_MIDDLE_CHUNK, @ref SL_SI91X_AES_LAST_CHUNK.
 *   Buffer to store the processed output data.
 * @return
 *   sl_status_t.
 * For more information on status codes, refer to 
 * [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 * @note
 *   - The maximum length of the message in bytes that can be handled in one go is determined by SLI_SI91X_MAX_DATA_SIZE_IN_BYTES_FOR_AES.
 *   - If the message length exceeds this limit, the function returns SL_STATUS_INVALID_PARAMETER.
 *   - If the message length is not a multiple of SL_SI91X_AES_BLOCK_SIZE, the function returns an error.
 ******************************************************************************/
sl_status_t sl_si91x_aes_multipart(const sl_si91x_aes_config_t *config,
                                   uint16_t chunk_length,
                                   uint8_t aes_flags,
                                   uint8_t *output);
/** @} */
