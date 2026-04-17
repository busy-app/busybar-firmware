/***************************************************************************/ /**
 * @file
 * @brief SL SI91X HMAC Header file
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
 * @addtogroup CRYPTO_HMAC_CONSTANTS 
 * @{ 
 */

/**
 * @brief Enumeration defining HMAC-SHA modes supported by the SI91X device.
 *
 * This enumeration defines different HMAC-SHA modes supported by the SI91X device,
 * including SHA1, SHA256, SHA384, and SHA512 modes.
 */
typedef enum {
  SL_SI91X_HMAC_SHA_1 = 1, ///< HMAC SHA 1 mode
  SL_SI91X_HMAC_SHA_256,   ///< HMAC SHA 256 mode
  SL_SI91X_HMAC_SHA_384,   ///< HMAC SHA 384 mode
  SL_SI91X_HMAC_SHA_512    ///< HMAC SHA 512 mode
} sl_si91x_hmac_mode_t;

/**
 * @brief Enumeration defining digest lengths of HMAC-SHA modes supported by the SI91X device.
 *
 * This enumeration defines digest lengths of different HMAC-SHA modes supported by the SI91X device,
 * including SHA1, SHA256, SHA384, and SHA512 modes.
 */
typedef enum {
  SL_SI91X_HMAC_SHA_1_DIGEST_LEN   = 20, ///< Digest length for HMAC SHA 1
  SL_SI91X_HMAC_SHA_256_DIGEST_LEN = 32, ///< Digest length for HMAC SHA 256
  SL_SI91X_HMAC_SHA_384_DIGEST_LEN = 48, ///< Digest length for HMAC SHA 384
  SL_SI91X_HMAC_SHA_512_DIGEST_LEN = 64  ///< Digest length for HMAC SHA 512
} sl_si91x_hmac_digest_len_t;

/** @} */

/******************************************************
 *                   Type Definitions
 ******************************************************/
/**
 * @addtogroup CRYPTO_HMAC_TYPES 
 * @{ 
 */

/**
 * @brief Structure defining HMAC key configuration for non-B0 chip versions.
 *
 * The structure defines the key configuration for non-B0 chip versions, which includes
 * a pointer to the key, and the length of the key.
 */
typedef struct {
  uint8_t *key;        ///< Pointer to the key
  uint32_t key_length; ///< Length of the key
} sl_si91x_hmac_key_config_A0_t;

/**
 * @brief Structure defining HMAC key configuration for B0 chip versions.
 *
 * The structure defines the key configuration for B0 chip versions, which includes
 * the key type, key size, key slot, wrap mode, IV, key buffer, and reserved field.
 */
typedef struct {
  sl_si91x_crypto_key_type_t key_type;      ///< Key type - wrapped or plain
  uint32_t key_size;                        ///< Key size
  sl_si91x_crypto_key_slot_t key_slot;      ///< Key slot
  sl_si91x_crypto_wrap_mode_t wrap_iv_mode; ///< Wrap mode - ECB or CBC
  uint8_t wrap_iv[SL_SI91X_IV_SIZE];        ///< IV used for wrapping in SL_SI91X_AES_CBC and SL_SI91X_AES_CTR modes
  uint8_t *key;                             ///< Pointer to the key
  uint32_t reserved;                        ///< Reserved for future use
} sl_si91x_hmac_key_config_B0_t;

/**
 * @brief Union holding HMAC key configuration structures for B0, and non-B0 chip versions.
 *
 * This union holds the key configuration structures for both B0, and non-B0 chip versions.
 */
typedef union {
  sl_si91x_hmac_key_config_A0_t A0; ///< Key configuration for non-B0 chip versions
  sl_si91x_hmac_key_config_B0_t B0; ///< Key configuration for B0 chip versions
} sl_si91x_hmac_key_config_t;

/**
 * @brief Structure defining HMAC configuration.
 *
 * This structure defines the HMAC configuration required for the operation, which includes 
 * the type of operation, input message, length of the input message, key configuration structure, and so on.
 */
typedef struct {
  sl_si91x_hmac_mode_t hmac_mode;        ///< HMAC Mode
  const uint8_t *msg;                    ///< Pointer to the input message
  uint32_t msg_length;                   ///< Length of the message
  sl_si91x_hmac_key_config_t key_config; ///< Key configuration
} sl_si91x_hmac_config_t;

/** @} */

/******************************************************
 *                Function Declarations
*******************************************************/
/**
 * @addtogroup CRYPTO_HMAC_FUNCTIONS
 * @{ 
 */

/***************************************************************************/
/**
 * @brief 
 *   To compute the HMAC output for the given configuration. It is a blocking API.
 * @param[in] config 
 *   Configuration object of type @ref sl_si91x_hmac_config_t
 * @param[out] output 
 *   Buffer to store the output.
 * @return
 *   sl_status_t.
 * For more information on status codes, refer to 
 * [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
******************************************************************************/
sl_status_t sl_si91x_hmac(const sl_si91x_hmac_config_t *config, uint8_t *output);

/** @} */
