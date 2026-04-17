/***************************************************************************//**
 * @file
 * @brief Crypto HAL for NVM3 driver
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

#ifndef NVM3_HAL_CRYPTO_H
#define NVM3_HAL_CRYPTO_H

#include "sl_status.h"

#ifdef NVM3_HOST_BUILD
#include "nvm3_hal_host.h"
#else
#include "sl_assert.h"
#include "sl_common.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************//**
 * @addtogroup nvm3
 * @{
 ******************************************************************************/

/***************************************************************************//**
 * @addtogroup nvm3halcrypto NVM3 HAL CRYPTO
 * @brief NVM3 Crypto Hardware Abstraction Layer
 * @{
 * @details
 * This module provides the interface to the NVM. By having all NVM access
 * functions in a separate file, it is possible to support different hardware
 * by substituting the functions in this module.
 *
 * @note These functions are used by the NVM3 and should not be used by
 * any applications.
 ******************************************************************************/

/******************************************************************************
 ******************************    MACROS    **********************************
 *****************************************************************************/

/// @cond DO_NOT_INCLUDE_WITH_DOXYGEN

#define nvm3_halCryptoInit(halCrypto)                              ((halCrypto)->init())
#define nvm3_halCryptoGenRandNum(halCrypto, a, b)                  ((halCrypto)->genRandNum((a), (b)))
#define nvm3_halCryptoEncrypt(halCrypto, a, b, c, d, e, f, g, h)   ((halCrypto)->encrypt((a), (b), (c), (d), (e), (f), (g), (h)))
#define nvm3_halCryptoDecrypt(halCrypto, a, b, c, d, e, f, g, h)   ((halCrypto)->decrypt((a), (b), (c), (d), (e), (f), (g), (h)))

/// @endcond

/******************************************************************************
 ******************************   TYPEDEFS   **********************************
 *****************************************************************************/

typedef enum {
  NVM3_HAL_CRYPTO_ALGO_NONE = 0,
  NVM3_HAL_CRYPTO_ALGO_AEAD = 1,
  NVM3_HAL_CRYPTO_ALGO_ENC = 2
} nvm3_halCryptoAlgo_t;

/*******************************************************************************
 *****************************   PROTOTYPES   **********************************
 ******************************************************************************/

/***************************************************************************//**
 * @brief
 *  Init the NVM3 HAL Crypto for usage.
 *
 * @details
 *   This function must be run at initialization, before any other functions
 *   are called. It is used to initialize the crypto before any crypto
 *   functionality can be accessed.
 *
 * @return
 *   The result of the crypto init call.
 *   @ref SL_STATUS_OK on success or a NVM3 @ref sl_status_t on failure.
 ******************************************************************************/
typedef sl_status_t (*nvm3_HalCryptoInit_t)(void);

/***************************************************************************//**
 * @brief
 *   This function is used to generate random number.
 *
 * @param[in] outputSize
 *   The length of the random number to be generated in number of bytes.
 *
 * @param[out] output
 *   A pointer to the random number generated.
 *
 * @return
 *   The result of the random number generator operation.
 *   @ref SL_STATUS_OK on success or a NVM3 @ref sl_status_t on failure.
 ******************************************************************************/
typedef sl_status_t (*nvm3_HalCryptoGenRandNum_t)(uint8_t *output, size_t outputSize);

/***************************************************************************//**
 * @brief
 *   This function is used to perform encryption/authenticated encryption on
 *   NVM data.
 *
 * @param[in] nonce
 *   A pointer to the nonce value.
 *
 * @param[in] aad
 *   A pointer to the additional data to authenticate.
 *
 * @param[in] aadLen
 *   The length of the additional data to authenticate in number of bytes.
 *
 * @param[in] plainData
 *   A pointer to plain data to encrypt.
 *
 * @param[in] plainDataLen
 *   The length of the plain data in number of bytes.
 *
 * @param[out] cipherData
 *   A pointer to encrypted data.
 *
 * @param[out] tag
 *   A pointer to the tag.
 *
 * @param[in] cryptoAlgo
 *   Crypto algorithm to use.
 *
 * @return
 *   The result of the encryption operation.
 *   @ref SL_STATUS_OK on success or a NVM3 @ref sl_status_t on failure.
 ******************************************************************************/
typedef sl_status_t (*nvm3_HalCryptoEncrypt_t)(const uint8_t *nonce, const uint8_t *aad, size_t aadLen, const uint8_t *plainData, size_t plainDataLen, uint8_t *cipherData, uint8_t *tag, nvm3_halCryptoAlgo_t cryptoAlgo);

/***************************************************************************//**
 * @brief
 *   This function is used to perform decryption/authenticated decryption
 *   of NVM data.
 *
 * @param[in] nonce
 *   A pointer to the nonce value.
 *
 * @param[in] aad
 *   A pointer to the additional data to authenticate.
 *
 * @param[in] aadLen
 *   The length of the additional data to authenticate in number of bytes.
 *
 * @param[in] cipherData
 *   A pointer to encrypted data to decrypt.
 *
 * @param[in] cipherDataLen
 *   The length of the encrypted data in number of bytes.
 *
 * @param[out] plainData
 *   A pointer to decrypted data.
 *
 * @param[in] tag
 *   A pointer to the tag to verify.
 *
 * @param[in] cryptoAlgo
 *   Crypto algorithm to use.
 *
 * @return
 *   The result of the decryption operation.
 *   @ref SL_STATUS_OK on success or a NVM3 @ref sl_status_t on failure.
 ******************************************************************************/
typedef sl_status_t (*nvm3_HalCryptoDecrypt_t)(const uint8_t *nonce, const uint8_t *aad, size_t aadLen, const uint8_t *cipherData, size_t cipherDataLen, uint8_t *plainData, const uint8_t *tag, nvm3_halCryptoAlgo_t cryptoAlgo);

/// @brief The HAL Crypto handle definition.
typedef struct {
  nvm3_HalCryptoInit_t          init;          ///< Pointer to the init function
  nvm3_HalCryptoGenRandNum_t    genRandNum;    ///< Pointer to the random number generator function
  nvm3_HalCryptoEncrypt_t       encrypt;       ///< Pointer to the encryption function
  nvm3_HalCryptoDecrypt_t       decrypt;       ///< Pointer to the decryption function
} nvm3_HalCryptoHandle_t;

/** @} (end addtogroup nvm3halcrypto) */
/** @} (end addtogroup nvm3) */

#ifdef __cplusplus
}
#endif

#endif /* NVM_CRYPTO_HAL_H */
