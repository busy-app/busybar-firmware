/***************************************************************************//**
 * @file sli_token_manager_hal_crypto_se.c
 * @brief Crypto HAL implementation for SE subsystem
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

#include <string.h>
#include "sli_token_manager_hal_crypto.h"
#include "sl_se_manager.h"
#include "sl_se_manager_entropy.h"
#include "sl_se_manager_cipher.h"

/** @cond DO_NOT_INCLUDE_WITH_DOXYGEN */

#define SE_CRYPTO_GCM_TAG_SIZE   4
#define SE_CRYPTO_NONCE_SIZE     12
#define SE_CRYPTO_KEY_SIZE       16
/*******************************************************************************
 **************************   GLOBAL VARIABLES   *******************************
 ******************************************************************************/

static sl_se_key_descriptor_t se_hal_gcm_key_desc;

/*******************************************************************************
 **************************   LOCAL FUNCTIONS   ********************************
 ******************************************************************************/

/***************************************************************************//**
 * Initialize key descriptor for crypto operations.
 ******************************************************************************/
static void hal_key_desc_init(void)
{
  se_hal_gcm_key_desc.type = SL_SE_KEY_TYPE_AES_128;
  se_hal_gcm_key_desc.size = SE_CRYPTO_KEY_SIZE;
  se_hal_gcm_key_desc.storage.method = SL_SE_KEY_STORAGE_INTERNAL_IMMUTABLE;
  se_hal_gcm_key_desc.flags = SL_SE_KEY_FLAG_IS_DEVICE_GENERATED;
  // The common token manager and NVM3 share the same key.
  se_hal_gcm_key_desc.storage.location.slot = SL_SE_KEY_SLOT_NVM3_KEY;
}

/***************************************************************************//**
 * Validate crypto parameters.
 ******************************************************************************/
static sl_status_t hal_validate_crypto_params(const uint8_t *nonce,
                                              const uint8_t *aad,
                                              size_t aad_len,
                                              const uint8_t *in_data,
                                              size_t in_data_len,
                                              uint8_t *out_data,
                                              const uint8_t *tag,
                                              hal_crypto_algo_t crypto_algo)
{
  if (crypto_algo != HAL_CRYPTO_ALGO_AEAD) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  if ((nonce == NULL)
      || (aad == NULL && aad_len > 0)
      || (in_data == NULL && in_data_len > 0)
      || (out_data == NULL)
      || (tag == NULL)) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  return SL_STATUS_OK;
}

/** @endcond */

/***************************************************************************//**
 * SE manager initialization.
 ******************************************************************************/
static sl_status_t hal_crypto_se_init(void)
{
  // Initialize the SE manager
  if (sl_se_init() != SL_STATUS_OK) {
    return SL_STATUS_INITIALIZATION;
  }
  // Initialize key descriptor for crypto operations
  hal_key_desc_init();
  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Generate true random number.
 ******************************************************************************/
static sl_status_t hal_crypto_se_gen_rand_num(uint8_t *output, size_t output_size)
{
  sl_se_command_context_t cmd_ctx;
  sl_se_init_command_context(&cmd_ctx);
  if (sl_se_get_random(&cmd_ctx, output, output_size) != SL_STATUS_OK) {
    return SL_STATUS_SECURITY_RANDOM_NUM_GEN_ERROR;
  }
  return SL_STATUS_OK;
}

/***************************************************************************//**
 * SE crypto encryption operation.
 ******************************************************************************/
static sl_status_t hal_crypto_se_encrypt(const uint8_t *nonce,
                                         const uint8_t *aad,
                                         size_t aad_len,
                                         const uint8_t *plain_data,
                                         size_t plain_data_len,
                                         uint8_t *cipher_data,
                                         uint8_t *tag,
                                         hal_crypto_algo_t crypto_algo)
{
  if (hal_validate_crypto_params(nonce,
                                 aad,
                                 aad_len,
                                 plain_data,
                                 plain_data_len,
                                 cipher_data,
                                 tag,
                                 crypto_algo) != SL_STATUS_OK) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  sl_se_command_context_t cmd_ctx;
  sl_se_init_command_context(&cmd_ctx);
  if (sl_se_gcm_crypt_and_tag(&cmd_ctx,
                              &se_hal_gcm_key_desc,
                              SL_SE_ENCRYPT,
                              plain_data_len,
                              nonce,
                              SE_CRYPTO_NONCE_SIZE,
                              aad,
                              aad_len,
                              plain_data,
                              cipher_data,
                              SE_CRYPTO_GCM_TAG_SIZE,
                              tag) != SL_STATUS_OK) {
    return SL_STATUS_SECURITY_ENCRYPT_ERROR;
  }
  return SL_STATUS_OK;
}

/***************************************************************************//**
 * SE crypto decryption operation.
 ******************************************************************************/
static sl_status_t  hal_crypto_se_decrypt(const uint8_t *nonce,
                                          const uint8_t *aad,
                                          size_t aad_len,
                                          const uint8_t *cipher_data,
                                          size_t cipher_data_len,
                                          uint8_t *plain_data,
                                          const uint8_t *tag,
                                          hal_crypto_algo_t crypto_algo)
{
  if (hal_validate_crypto_params(nonce,
                                 aad,
                                 aad_len,
                                 cipher_data,
                                 cipher_data_len,
                                 plain_data,
                                 tag,
                                 crypto_algo) != SL_STATUS_OK) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  sl_se_command_context_t cmd_ctx;
  sl_se_init_command_context(&cmd_ctx);
  if (sl_se_gcm_auth_decrypt(&cmd_ctx,
                             &se_hal_gcm_key_desc,
                             cipher_data_len,
                             nonce,
                             SE_CRYPTO_NONCE_SIZE,
                             aad,
                             aad_len,
                             cipher_data,
                             plain_data,
                             SE_CRYPTO_GCM_TAG_SIZE,
                             tag) != SL_STATUS_OK) {
    return SL_STATUS_SECURITY_DECRYPT_ERROR;
  }
  return SL_STATUS_OK;
}

/*******************************************************************************
 ***************************   GLOBAL VARIABLES   ******************************
 ******************************************************************************/

const hal_crypto_handle_t hal_crypto_handle = {
  .init           = hal_crypto_se_init,                ///< Set the init function
  .gen_rand_num   = hal_crypto_se_gen_rand_num,        ///< Set the random num generator function
  .encrypt        = hal_crypto_se_encrypt,             ///< Set the encryption function
  .decrypt        = hal_crypto_se_decrypt,             ///< Set the decryption function
};
