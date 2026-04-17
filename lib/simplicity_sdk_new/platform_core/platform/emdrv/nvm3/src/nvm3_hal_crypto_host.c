/***************************************************************************//**
 * @file
 * @brief HAL Host Crypto implementation for Non-Volatile Memory driver
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

#include <string.h>
#include "nvm3.h"
#include "nvm3_hal_crypto_handle.h"
#include "sli_crypto.h"
#include "sl_se_manager.h"
#include "sl_se_manager_entropy.h"

/***************************************************************************//**
 * @addtogroup nvm3
 * @{
 ******************************************************************************/

/***************************************************************************//**
 * @addtogroup nvm3halcrypto
 * @{
 ******************************************************************************/

/******************************************************************************
 ******************************    MACROS    **********************************
 *****************************************************************************/

#define NVM3_KSU_KEY_SLOT  0           ///< KSU key slot used by NVM3

/** @cond DO_NOT_INCLUDE_WITH_DOXYGEN */

/*******************************************************************************
 **************************   GLOBAL VARIABLES   *******************************
 ******************************************************************************/

static sli_crypto_descriptor_t nvm3_gcm_key_desc;
static bool key_desc_init_done = false;

/*******************************************************************************
 **************************   LOCAL FUNCTIONS   ********************************
 ******************************************************************************/

/***************************************************************************//**
 * Initialize key descriptor for NVM3 crypto operations.
 ******************************************************************************/
static sl_status_t nvm3_keyDescInit()
{
  sl_se_command_context_t cmd_ctx;

  sl_se_key_descriptor_t nvm3_se_key_desc = {
    .type = SL_SE_KEY_TYPE_AES_128,
    .size = 16,
    .storage.method = SL_SE_KEY_STORAGE_INTERNAL_IMMUTABLE,
    .storage.location.slot = SL_SE_KEY_SLOT_NVM3_KEY,
    .flags = SL_SE_KEY_FLAG_IS_DEVICE_GENERATED,
  };
  // Setup KSU key descriptor
  sl_se_key_descriptor_t nvm3_ksu_key_slot = {
    .type = SL_SE_KEY_TYPE_AES_128,
    .storage.method = SL_SE_KEY_STORAGE_INTERNAL_KSU,
    .storage.location.ksu.keyslot = NVM3_KSU_KEY_SLOT,
  };
  sl_se_init_command_context(&cmd_ctx);
  // Transfer NVM3 crypto key to KSU slot
  if (sl_se_transfer_key(&cmd_ctx, &nvm3_se_key_desc, &nvm3_ksu_key_slot) != SL_STATUS_OK) {
    return SL_STATUS_SECURITY_KEY_ERROR;
  }
  nvm3_gcm_key_desc.location = SLI_CRYPTO_KEY_LOCATION_KSU;
  nvm3_gcm_key_desc.engine = SLI_CRYPTO_HOSTSYMCRYPTO;
  nvm3_gcm_key_desc.key.key_slot = NVM3_KSU_KEY_SLOT;
  nvm3_gcm_key_desc.yield = false;
  key_desc_init_done = true;

  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Validate crypto parameters.
 ******************************************************************************/
static sl_status_t nvm3_validateCryptoParams(const uint8_t *nonce,
                                             const uint8_t *aad,
                                             size_t aadLen,
                                             const uint8_t *inData,
                                             size_t inDataLen,
                                             uint8_t *outData,
                                             const uint8_t *tag,
                                             nvm3_halCryptoAlgo_t cryptoAlgo)
{
  if (cryptoAlgo != NVM3_HAL_CRYPTO_ALGO_AEAD) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  if ((nonce == NULL)
      || (aad == NULL && aadLen > 0)
      || (inData == NULL && inDataLen > 0)
      || (outData == NULL)
      || (tag == NULL)) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  return SL_STATUS_OK;
}

/** @endcond */

/***************************************************************************//**
 * Host crypto initialization.
 ******************************************************************************/
static sl_status_t nvm3_halCryptoHostInit(void)
{
  if (key_desc_init_done == true) {
    return SL_STATUS_OK;
  }
  // Initialize key descriptor for NVM3 crypto operations
  if (nvm3_keyDescInit() != SL_STATUS_OK) {
    return SL_STATUS_SECURITY_KEY_ERROR;
  }
  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Generate true random number.
 ******************************************************************************/
static sl_status_t nvm3_halCryptoSeGenRandNum(uint8_t *output,
                                              size_t outputSize)
{
  sl_se_command_context_t cmd_ctx;
  sl_se_init_command_context(&cmd_ctx);
  if (sl_se_get_random(&cmd_ctx, output, outputSize) != SL_STATUS_OK) {
    return SL_STATUS_SECURITY_RANDOM_NUM_GEN_ERROR;
  }
  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Host symmetric crypto encryption operation.
 ******************************************************************************/
static sl_status_t nvm3_halCryptoHostEncrypt(const uint8_t *nonce,
                                             const uint8_t *aad,
                                             size_t aadLen,
                                             const uint8_t *plainData,
                                             size_t plainDataLen,
                                             uint8_t *cipherData,
                                             uint8_t *tag,
                                             nvm3_halCryptoAlgo_t cryptoAlgo)
{
  if (nvm3_validateCryptoParams(nonce,
                                aad,
                                aadLen,
                                plainData,
                                plainDataLen,
                                cipherData,
                                tag,
                                cryptoAlgo) != SL_STATUS_OK) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  if (sli_crypto_gcm_encrypt(&nvm3_gcm_key_desc,
                             plainData,
                             plainDataLen,
                             cipherData,
                             nonce,
                             aad,
                             aadLen,
                             tag,
                             NVM3_GCM_TAG_SIZE) != SL_STATUS_OK) {
    return SL_STATUS_SECURITY_ENCRYPT_ERROR;
  }
  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Host symmetric crypto decryption operation.
 ******************************************************************************/
static sl_status_t nvm3_halCryptoHostDecrypt(const uint8_t *nonce,
                                             const uint8_t *aad,
                                             size_t aadLen,
                                             const uint8_t *cipherData,
                                             size_t cipherDataLen,
                                             uint8_t *plainData,
                                             const uint8_t *tag,
                                             nvm3_halCryptoAlgo_t cryptoAlgo)
{
  if (nvm3_validateCryptoParams(nonce,
                                aad,
                                aadLen,
                                cipherData,
                                cipherDataLen,
                                plainData,
                                tag,
                                cryptoAlgo) != SL_STATUS_OK) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  if (sli_crypto_gcm_decrypt(&nvm3_gcm_key_desc,
                             cipherData,
                             cipherDataLen,
                             plainData,
                             nonce,
                             aad,
                             aadLen,
                             tag,
                             NVM3_GCM_TAG_SIZE) != SL_STATUS_OK) {
    return SL_STATUS_SECURITY_DECRYPT_ERROR;
  }
  return SL_STATUS_OK;
}

/*******************************************************************************
 ***************************   GLOBAL VARIABLES   ******************************
 ******************************************************************************/

const nvm3_HalCryptoHandle_t nvm3_halCryptoHandle  = {
  .init = nvm3_halCryptoHostInit,                ///< Set the init function
  .genRandNum = nvm3_halCryptoSeGenRandNum,      ///< Set the random num generator function
  .encrypt = nvm3_halCryptoHostEncrypt,          ///< Set the encryption function
  .decrypt = nvm3_halCryptoHostDecrypt,          ///< Set the decryption function
};

/** @} (end addtogroup nvm3halcrypto) */
/** @} (end addtogroup nvm3) */
