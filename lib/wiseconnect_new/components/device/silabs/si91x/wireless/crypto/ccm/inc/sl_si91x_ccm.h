/***************************************************************************/ /**
 * @file
 * @brief SL SI91X CCM Header file
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
 * @addtogroup CRYPTO_CCM_CONSTANTS
 * @{ 
 */

/**
 * @brief Enumeration defines the CCM encryption and decryption types.
 *
 * This enumeration defines the types of CCM operations supported, which includes
 * encryption, and decryption.
 */
typedef enum {
  SL_SI91X_CCM_ENCRYPT = 0, ///< CCM Encryption
  SL_SI91X_CCM_DECRYPT      ///< CCM Decryption
} sl_si91x_ccm_type_t;

/**
 * @brief Enumeration defines the CCM key sizes supported by the SI91X device.
 *
 * This enumeration defines the key sizes supported by the CCM encryption
 * algorithm on the SI91X device, which includes 128-bit, 192-bit, and 256-bit key sizes.
 */
typedef enum {
  SL_SI91X_CCM_KEY_SIZE_128 = 16, ///< Key size of 128 bits
  SL_SI91X_CCM_KEY_SIZE_192 = 24, ///< Key size of 192 bits
  SL_SI91X_CCM_KEY_SIZE_256 = 32  ///< Key size of 256 bits
} sl_si91x_ccm_key_size_t;

/** @} */

/******************************************************
 *                   Type Definitions
 ******************************************************/
/**
 * @addtogroup CRYPTO_CCM_TYPES 
 * @{ 
 */

/**
 * @brief Structure defines the CCM key configuration for the non-B0 chip versions.
 */
typedef struct {
  uint8_t *key;        ///< Pointer to the key
  uint16_t key_length; ///< Length of the key
} sl_si91x_ccm_key_config_a0_t;

/**
 * @brief Structure defines the CCM key configuration for B0 chip versions.
 */
typedef struct {
  sl_si91x_crypto_key_type_t key_type;          ///< Key type
  sl_si91x_ccm_key_size_t key_size;             ///< Key size
  sl_si91x_crypto_key_slot_t key_slot;          ///< Key slot
  sl_si91x_crypto_wrap_mode_t wrap_iv_mode;     ///< Wrap mode
  uint8_t wrap_iv[SL_SI91X_IV_SIZE];            ///< IV used in SL_SI91X_AES_CBC and SL_SI91X_AES_CTR modes
  uint8_t key_buffer[SL_SI91X_KEY_BUFFER_SIZE]; ///< Buffer to store the key
  uint32_t reserved;                            ///< Reserved for future use
} sl_si91x_ccm_key_config_b0_t;

/**
 * @brief Union holds the CCM key configuration structures of B0, and non-B0 chip versions.
 */
typedef union {
  sl_si91x_ccm_key_config_a0_t a0; ///< Key configuration for non-B0 chip versions
  sl_si91x_ccm_key_config_b0_t b0; ///< Key configuration for B0 chip versions
} sl_si91x_ccm_key_config_t;

/**
 * @brief Structure defines the CCM configuration.
 * 
 * This structure defines the CCM configuration required for the operation which includes 
 * type of operation, input message, length of input message, key configuration structure, and so on.
 */
typedef struct {
  sl_si91x_ccm_type_t encrypt_decrypt;  ///< Encryption or decryption
  const uint8_t *msg;                   ///< Pointer to the input message
  uint16_t msg_length;                  ///< Length of the message
  const uint8_t *nonce;                 ///< Pointer to the Initialization vector
  uint8_t *tag;                         ///< Pointer to the tag
  const uint8_t *ad;                    ///< Pointer to the additional data
  uint16_t nonce_length;                ///< Length of the Initialization vector
  uint16_t tag_length;                  ///< Length of the tag
  uint16_t ad_length;                   ///< Length of the additional data
  sl_si91x_ccm_key_config_t key_config; ///< Key configuration
} sl_si91x_ccm_config_t;

/** @} */

/******************************************************
 *                Function Declarations
*******************************************************/
/**
 * @addtogroup CRYPTO_CCM_FUNCTIONS
 * @{ 
 */

/***************************************************************************/
/**
 * @brief 
 *   To encrypt or decrypt the message according to the given configuration. This is a blocking API.
 * @param[in] config 
 *   Configuration object of type @ref sl_si91x_ccm_config_t.
 * @param[out] output 
 *   Buffer to store the output.
 * @return
 *   sl_status_t.
 * For more information on status codes, refer to 
 * [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_ccm(sl_si91x_ccm_config_t *config, uint8_t *output);

/** @} */
