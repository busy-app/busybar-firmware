/***************************************************************************//**
 * @file sli_token_manager_hal_crypto_host.c
 * @brief Crypto HAL implementation for Host subsystem
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
#include "sli_crypto.h"
#include "sl_se_manager.h"
#include "sl_se_manager_entropy.h"
#include "sli_token_manager_hal_crypto.h"

/** @cond DO_NOT_INCLUDE_WITH_DOXYGEN */

/******************************************************************************
 ******************************    MACROS    **********************************
 *****************************************************************************/
// The common token manager and NVM3 share the same key.
#define NVM3_KSU_KEY_SLOT  0           ///< KSU key slot
#define HOST_CRYPTO_GCM_TAG_SIZE   4
#define HOST_CRYPTO_NONCE_SIZE     12
#define HOST_CRYPTO_KEY_SIZE       16
/*******************************************************************************
 **************************   GLOBAL VARIABLES   *******************************
 ******************************************************************************/

static sli_crypto_descriptor_t host_hal_gcm_key_desc;
static bool key_desc_init_done = false;

/*******************************************************************************
 **************************   LOCAL FUNCTIONS   ********************************
 ******************************************************************************/

/***************************************************************************//**
 * Initialize key descriptor for host crypto operations.
 ******************************************************************************/
static sl_status_t hal_crypto_host_key_desc_init()
{
  sl_se_command_context_t cmd_ctx;

  sl_se_key_descriptor_t host_key_desc = {
    .type = SL_SE_KEY_TYPE_AES_128,
    .size = 16,
    .storage.method = SL_SE_KEY_STORAGE_INTERNAL_IMMUTABLE,
    .storage.location.slot = SL_SE_KEY_SLOT_NVM3_KEY,
    .flags = SL_SE_KEY_FLAG_IS_DEVICE_GENERATED,
  };
  // Setup KSU key descriptor, CTM uses NVM3 key.
  sl_se_key_descriptor_t host_ksu_key_slot = {
    .type = SL_SE_KEY_TYPE_AES_128,
    .storage.method = SL_SE_KEY_STORAGE_INTERNAL_KSU,
    .storage.location.ksu.keyslot = NVM3_KSU_KEY_SLOT,
  };
  sl_se_init_command_context(&cmd_ctx);

  if (sl_se_transfer_key(&cmd_ctx, &host_key_desc, &host_ksu_key_slot) != SL_STATUS_OK) {
    return SL_STATUS_SECURITY_KEY_ERROR;
  }
  host_hal_gcm_key_desc.location = SLI_CRYPTO_KEY_LOCATION_KSU;
  host_hal_gcm_key_desc.engine = SLI_CRYPTO_HOSTSYMCRYPTO;
  host_hal_gcm_key_desc.key.key_slot = NVM3_KSU_KEY_SLOT;
  host_hal_gcm_key_desc.yield = false;
  key_desc_init_done = true;

  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Validate crypto parameters.
 ******************************************************************************/
static sl_status_t hal_validate_host_crypto_params(const uint8_t *nonce,
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

/***************************************************************************//**
 * Host crypto initialization.
 ******************************************************************************/
static sl_status_t hal_crypto_host_init(void)
{
  if (key_desc_init_done == true) {
    return SL_STATUS_OK;
  }
  // Initialize key descriptor for host crypto operations
  if (hal_crypto_host_key_desc_init() != SL_STATUS_OK) {
    return SL_STATUS_SECURITY_KEY_ERROR;
  }
  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Generate true random number.
 ******************************************************************************/
static sl_status_t hal_crypto_host_gen_rand_num(uint8_t *output,
                                                size_t output_size)
{
  sl_se_command_context_t cmd_ctx;
  sl_se_init_command_context(&cmd_ctx);
  if (sl_se_get_random(&cmd_ctx, output, output_size) != SL_STATUS_OK) {
    return SL_STATUS_SECURITY_RANDOM_NUM_GEN_ERROR;
  }
  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Host symmetric crypto encryption operation.
 ******************************************************************************/
static sl_status_t hal_crypto_host_encrypt(const uint8_t *nonce,
                                           const uint8_t *aad,
                                           size_t aad_len,
                                           const uint8_t *plain_data,
                                           size_t plain_data_len,
                                           uint8_t *cipher_data,
                                           uint8_t *tag,
                                           hal_crypto_algo_t crypto_algo)
{
  if (hal_validate_host_crypto_params(nonce,
                                      aad,
                                      aad_len,
                                      plain_data,
                                      plain_data_len,
                                      cipher_data,
                                      tag,
                                      crypto_algo) != SL_STATUS_OK) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  if (sli_crypto_gcm_encrypt(&host_hal_gcm_key_desc,
                             plain_data,
                             plain_data_len,
                             cipher_data,
                             nonce,
                             aad,
                             aad_len,
                             tag,
                             HOST_CRYPTO_GCM_TAG_SIZE) != SL_STATUS_OK) {
    return SL_STATUS_SECURITY_ENCRYPT_ERROR;
  }
  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Host symmetric crypto decryption operation.
 ******************************************************************************/
static sl_status_t hal_crypto_host_decrypt(const uint8_t *nonce,
                                           const uint8_t *aad,
                                           size_t aad_len,
                                           const uint8_t *cipher_data,
                                           size_t cipher_data_len,
                                           uint8_t *plain_data,
                                           const uint8_t *tag,
                                           hal_crypto_algo_t crypto_algo)
{
  if (hal_validate_host_crypto_params(nonce,
                                      aad,
                                      aad_len,
                                      cipher_data,
                                      cipher_data_len,
                                      plain_data,
                                      tag,
                                      crypto_algo) != SL_STATUS_OK) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  if (sli_crypto_gcm_decrypt(&host_hal_gcm_key_desc,
                             cipher_data,
                             cipher_data_len,
                             plain_data,
                             nonce,
                             aad,
                             aad_len,
                             tag,
                             HOST_CRYPTO_GCM_TAG_SIZE) != SL_STATUS_OK) {
    return SL_STATUS_SECURITY_DECRYPT_ERROR;
  }
  return SL_STATUS_OK;
}

/*******************************************************************************
 ***************************   GLOBAL VARIABLES   ******************************
 ******************************************************************************/

const hal_crypto_handle_t hal_crypto_handle = {
  .init           = hal_crypto_host_init,                ///< Set the init function
  .gen_rand_num   = hal_crypto_host_gen_rand_num,        ///< Set the random num generator function
  .encrypt        = hal_crypto_host_encrypt,             ///< Set the encryption function
  .decrypt        = hal_crypto_host_decrypt,             ///< Set the decryption function
};

/** @endcond */
