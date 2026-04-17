/***************************************************************************/ /**
 * @file
 * @brief SL SI91X GCM Header file
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
 * @addtogroup CRYPTO_GCM_CONSTANTS
 * @{ 
 */

/**
 * @brief Enumeration defining GCM encryption and decryption types.
 *
 * This enumeration defines the types of GCM operations supported, which includes
 * encryption, and decryption.
 */
typedef enum {
  SL_SI91X_GCM_ENCRYPT = 0, ///< GCM Encryption
  SL_SI91X_GCM_DECRYPT      ///< GCM Decryption
} sl_si91x_gcm_type_t;

/**
 * @brief Enumeration defining GCM modes supported by the SI91X device.
 *
 * This enumeration defines different GCM modes supported by the SI91X device,
 * such as GCM, and CMAC modes.
 */
typedef enum {
  SL_SI91X_GCM_MODE = 0, ///< GCM mode
  SL_SI91X_CMAC_MODE     ///< CMAC mode
} sl_si91x_gcm_mode_t;

/**
 * @brief Enumeration defining the GCM DMA enable, and disable options.
 *
 * This enumeration defines whether DMA is enabled or disabled for GCM operations.
 */
typedef enum {
  SL_SI91X_GCM_DMA_DISABLE = 0, ///< Disable DMA
  SL_SI91X_GCM_DMA_ENABLE       ///< Enable DMA
} sl_si91x_gcm_dma_use_t;

/**
 * @brief Enumeration defining GCM key sizes supported by the SI91X device.
 *
 * This enumeration defines the key sizes supported by the GCM encryption
 * algorithm on the SI91X device, which includes 128, 192, and 256-bit key sizes.
 */
typedef enum {
  SL_SI91X_GCM_KEY_SIZE_128 = 16, ///< Key size of 128 bits
  SL_SI91X_GCM_KEY_SIZE_192 = 24, ///< Key size of 192 bits
  SL_SI91X_GCM_KEY_SIZE_256 = 32  ///< Key size of 256 bits
} sl_si91x_gcm_key_size_t;

/** @} */

/******************************************************
 *                   Type Definitions
 ******************************************************/
/**
 * @addtogroup CRYPTO_GCM_TYPES 
 * @{ 
 */

/**
 * @brief Structure defining GCM key configuration for non-B0 chip versions.
 *
 * This structure defines the key configuration for non-B0 chip versions, which includes
 * a pointer to the key, and the length of the key.
 */
typedef struct {
  uint8_t *key;        ///< Pointer to the key
  uint16_t key_length; ///< Length of the key
} sl_si91x_gcm_key_config_a0_t;

/**
 * @brief Structure defining GCM key configuration for B0 chip versions.
 *
 * This structure defines the key configuration for B0 chip versions, including
 * the key type, key size, key slot, wrap mode, IV, key buffer, and reserved field.
 */
typedef struct {
  sl_si91x_crypto_key_type_t key_type;          ///< Key type - wrapped or plain
  sl_si91x_gcm_key_size_t key_size;             ///< Key size
  sl_si91x_crypto_key_slot_t key_slot;          ///< Key slot
  sl_si91x_crypto_wrap_mode_t wrap_iv_mode;     ///< Wrap mode - ECB or CBC
  uint8_t wrap_iv[SL_SI91X_IV_SIZE];            ///< IV used for wrapping in SL_SI91X_AES_CBC and SL_SI91X_AES_CTR modes
  uint8_t key_buffer[SL_SI91X_KEY_BUFFER_SIZE]; ///< Buffer to store the key
  uint32_t reserved;                            ///< Reserved for future use
} sl_si91x_gcm_key_config_b0_t;

/**
 * @brief Union holding GCM key configuration structures for B0, and non-B0 chip versions.
 *
 * This union holds the key configuration structures for both B0, and non-B0 chip versions.
 */
typedef union {
  sl_si91x_gcm_key_config_a0_t a0; ///< Key configuration for non-B0 chip versions
  sl_si91x_gcm_key_config_b0_t b0; ///< Key configuration for B0 chip versions
} sl_si91x_gcm_key_config_t;

/**
 * @brief Structure defining GCM configuration.
 *
 * This structure defines the GCM configuration required for the operation, which includes 
 * the type of operation, input message, length of the input message, key configuration structure, and so on.
 */
typedef struct {
  sl_si91x_gcm_type_t encrypt_decrypt; ///< Encryption or decryption
#if defined(SLI_SI917B0)
  sl_si91x_gcm_mode_t gcm_mode; ///< GCM or CMAC mode
#endif
  sl_si91x_gcm_dma_use_t dma_use;       ///< DMA Disable or Enable
  const uint8_t *msg;                   ///< Pointer to the input message
  uint16_t msg_length;                  ///< Length of the message
  const uint8_t *nonce;                 ///< Pointer to the Initialization vector
  const uint8_t *ad;                    ///< Pointer to the additional data
  uint16_t nonce_length;                ///< Length of the Initialization vector (Supports fixed 12 Bytes of IV)
  uint16_t ad_length;                   ///< Length of the additional data
  sl_si91x_gcm_key_config_t key_config; ///< Key configuration
} sl_si91x_gcm_config_t;

/** @} */

/******************************************************
 *                Function Declarations
*******************************************************/
/**
 * @addtogroup CRYPTO_GCM_FUNCTIONS
 * @{ 
 */

/***************************************************************************/
/**
 * @brief 
 *   To encrypt or decrypt the message according to the given configuration. This is a blocking API.
 * @param[in] config 
 *   Configuration object of type @ref sl_si91x_gcm_config_t
 * @param[out] output 
 *   Buffer to store the output.
 * @note
 *   The maximum message length supported is 1400 bytes.
 *   The M4 receive buffer is limited to 1400 bytes, so both the packet length and expected output must stay within this limit. If the output exceeds 1400 bytes, the NWP will return an error.
 * @return
 *   sl_status_t.
 * For more information on status codes, refer to 
 * [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
******************************************************************************/
sl_status_t sl_si91x_gcm(sl_si91x_gcm_config_t *config, uint8_t *output);

/** @} */
