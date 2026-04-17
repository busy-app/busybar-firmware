/***************************************************************************/ /**
 * @file
 * @brief SL SI91X Crypto common utility file
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

/******************************************************
 *                    Constants
 ******************************************************/
/**
 * @addtogroup CRYPTO_CONSTANTS
 * @{ 
 */

/**
 * @brief Maximum length of the message in bytes that can be handled in one go.
 */
#define SL_SI91X_MAX_DATA_SIZE_IN_BYTES 1400

/**
 * @brief Maximum length of the CCM message in bytes that can be handled in one go.
 */
#define SL_SI91X_MAX_DATA_SIZE_IN_BYTES_FOR_CCM 1200

/**
 * @brief Maximum length of the CHACHAPOLY message in bytes that can be handled in one go.
 */
#define SL_SI91X_MAX_DATA_SIZE_IN_BYTES_FOR_CHACHAPOLY 1200

/**
 * @brief Length of the initialization vector in bytes.
 */
#define SL_SI91X_IV_SIZE 16

/**
 * @brief Key size in bytes.
 */
#define SL_SI91X_KEY_BUFFER_SIZE 32

/**
 * @brief Maximum length of the ECDSA message in bytes that can be handled in one go.
 */
#define SL_SI91X_MAX_DATA_SIZE_IN_BYTES_FOR_ECDSA 1000

/**
 * @brief Indicator to pinpoint the first data segment undergoing processing within the API.
 */
#define FIRST_CHUNK BIT(0)

/**
 * @brief Indicator to pinpoint the middle data segment undergoing processing within the API.
 */
#define MIDDLE_CHUNK BIT(1)

/**
 * @brief Indicator to pinpoint the last data segment undergoing processing within the API.
 */
#define LAST_CHUNK BIT(2)

/**
 * @brief Enumeration defining different key slots for built-in keys supported by the SI91X device.
 *
 * This enumeration defines different key slots for built-in keys supported by the SI91X device.
 */
typedef enum {
  SL_SI91X_KEY_SLOT_1 = 1, ///< M4 OTA key slot
  SL_SI91X_KEY_SLOT_2      ///< M4 Public key slot
} sl_si91x_crypto_key_slot_t;

/**
 * @brief Enumeration defining different key sizes for built-in keys supported by the SI91X device.
 *
 * This enumeration defines different key sizes for built-in keys supported by the SI91X device,
 * such as 32, and 96 bytes.
 */
typedef enum {
  SL_SI91X_KEY_SIZE_1 = 32, ///< M4 OTA key size
  SL_SI91X_KEY_SIZE_2 = 96  ///< M4 Public key size
} sl_si91x_crypto_key_size_t;

/**
 * @brief Enumeration defining keys supported by the SI91X device.
 *
 * This enumeration defines different keys supported by the SI91X device,
 * such as transparent key, wrapped key, and built-in keys.
 */
typedef enum {
  SL_SI91X_TRANSPARENT_KEY = 0, ///< Plain key
  SL_SI91X_WRAPPED_KEY, ///< Wrapped version of the plain key. To use the wrapped keys, security should be enabled in the module.
  SL_SI91X_BUILT_IN_KEY ///< Firmware built-in keys
} sl_si91x_crypto_key_type_t;

/**
 * @brief Enumeration defining Wrap IV modes supported by the SI91X device.
 *
 * This enumeration defines different modes supported by the SI91X device
 * during the WRAP process, such as ECB and CBC modes.
 */
typedef enum {
  SL_SI91X_WRAP_IV_ECB_MODE = 0, ///< Wrap IV using ECB mode
  SL_SI91X_WRAP_IV_CBC_MODE      ///< Wrap IV using CBC mode
} sl_si91x_crypto_wrap_mode_t;

/**
 * @brief Enumeration defining crypto algorithms supported by the SI91X device.
 *
 * This enumeration defines different crypto algorithms supported by the SI91X device,
 * including AES, SHA, HMAC_SHA, ECDH, and so on.
 */
typedef enum {
  AES        = 2,  ///< AES
  SHA        = 4,  ///< SHA
  HMAC_SHA   = 5,  ///< HMAC
  DH         = 14, ///< DH
  ECDH       = 15, ///< ECDH
  SHA3       = 16, ///< SHA3
  CHACHAPOLY = 17, ///< CHACHAPOLY
  GCM        = 18, ///< GCM
  WRAP       = 20, ///< WRAP
  TRNG       = 21, ///< TRNG
  CCM        = 31, ///< CCM
  ECDSA      = 32  ///< ECDSA
} sl_si91x_crypto_algorithm_type_t;

/** @} */
