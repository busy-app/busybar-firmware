/***************************************************************************//**
 * @file sli_token_manager_hal_crypto.h
 * @brief Crypto HAL for common token manager
 *******************************************************************************
 * # License
 * <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
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

 #ifndef SLI_TOKEN_MANAGER_HAL_CRYPTO_H
 #define SLI_TOKEN_MANAGER_HAL_CRYPTO_H

 #include "sl_status.h"
 #include "sl_common.h"
 #include "sl_enum.h"

 #ifdef __cplusplus
extern "C" {
 #endif

/******************************************************************************
 ******************************    MACROS    **********************************
 *****************************************************************************/

/******************************************************************************
 ******************************   TYPEDEFS   **********************************
 *****************************************************************************/

SL_ENUM(hal_crypto_algo_t){
  HAL_CRYPTO_ALGO_NONE = 0,
  HAL_CRYPTO_ALGO_AEAD = 1,
  HAL_CRYPTO_ALGO_ENC = 2
};

/*******************************************************************************
 *****************************   PROTOTYPES   **********************************
 ******************************************************************************/

/***************************************************************************//**
 * @brief
 *  Init the HAL Crypto for usage.
 *
 * @details
 *   This function must be run at initialization, before any other functions
 *   are called. It is used to initialize the crypto before any crypto
 *   functionality can be accessed.
 *
 * @return
 *   The result of the crypto init call.
 *   @ref SL_STATUS_OK on success or a @ref sl_status_t on failure.
 ******************************************************************************/
typedef sl_status_t (*hal_crypto_init_t)(void);

/***************************************************************************//**
 * @brief
 *   This function is used to generate random number.
 *
 * @param[in] output_size
 *   The length of the random number to be generated in number of bytes.
 *
 * @param[out] output
 *   A pointer to the random number generated.
 *
 * @return
 *   The result of the random number generator operation.
 *   @ref SL_STATUS_OK on success or a @ref sl_status_t on failure.
 ******************************************************************************/
typedef sl_status_t (*hal_crypto_gen_rand_num_t)(uint8_t *output, size_t output_size);

/***************************************************************************//**
 * @brief
 *   This function is used to perform encryption/authenticated encryption on
 *   the data.
 *
 * @param[in] nonce
 *   A pointer to the nonce value.
 *
 * @param[in] aad
 *   A pointer to the additional data to authenticate.
 *
 * @param[in] aad_len
 *   The length of the additional data to authenticate in number of bytes.
 *
 * @param[in] plain_data
 *   A pointer to plain data to encrypt.
 *
 * @param[in] plain_data_len
 *   The length of the plain data in number of bytes.
 *
 * @param[out] cipher_data
 *   A pointer to encrypted data.
 *
 * @param[out] tag
 *   A pointer to the tag.
 *
 * @param[in] crypto_algo
 *   Crypto algorithm to use.
 *
 * @return
 *   The result of the encryption operation.
 *   @ref SL_STATUS_OK on success or a @ref sl_status_t on failure.
 ******************************************************************************/
typedef sl_status_t (*hal_crypto_encrypt_t)(const uint8_t *nonce, const uint8_t *aad, size_t aad_len, const uint8_t *plain_data, size_t plain_data_len, uint8_t *cipher_data, uint8_t *tag, hal_crypto_algo_t crypto_algo);

/***************************************************************************//**
 * @brief
 *   This function is used to perform decryption/authenticated decryption
 *   of the data.
 *
 * @param[in] nonce
 *   A pointer to the nonce value.
 *
 * @param[in] aad
 *   A pointer to the additional data to authenticate.
 *
 * @param[in] aad_len
 *   The length of the additional data to authenticate in number of bytes.
 *
 * @param[in] cipher_data
 *   A pointer to encrypted data to decrypt.
 *
 * @param[in] cipher_data_len
 *   The length of the encrypted data in number of bytes.
 *
 * @param[out] plain_data
 *   A pointer to decrypted data.
 *
 * @param[in] tag
 *   A pointer to the tag to verify.
 *
 * @param[in] crypto_algo
 *   Crypto algorithm to use.
 *
 * @return
 *   The result of the decryption operation.
 *   @ref SL_STATUS_OK on success or a @ref sl_status_t on failure.
 ******************************************************************************/
typedef sl_status_t (*hal_crypto_decrypt_t)(const uint8_t *nonce, const uint8_t *aad, size_t aad_len, const uint8_t *cipher_data, size_t cipher_data_len, uint8_t *plain_data, const uint8_t *tag, hal_crypto_algo_t crypto_algo);

/// @brief The HAL Crypto handle definition.
typedef struct {
  hal_crypto_init_t          init;           ///< Pointer to the init function
  hal_crypto_gen_rand_num_t  gen_rand_num;   ///< Pointer to the random number generator function
  hal_crypto_encrypt_t       encrypt;        ///< Pointer to the encryption function
  hal_crypto_decrypt_t       decrypt;        ///< Pointer to the decryption function
} hal_crypto_handle_t;

extern const hal_crypto_handle_t hal_crypto_handle;    ///< The HAL crypto handle.

 #ifdef __cplusplus
}
 #endif

 #endif /* SLI_TOKEN_MANAGER_HAL_CRYPTO_H */
