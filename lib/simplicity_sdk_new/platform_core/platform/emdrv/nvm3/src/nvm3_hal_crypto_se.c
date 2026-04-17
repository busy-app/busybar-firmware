/***************************************************************************//**
 * @file
 * @brief HAL SE Crypto implementation for Non-Volatile Memory driver
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
#include "sl_se_manager.h"
#include "sl_se_manager_entropy.h"
#include "sl_se_manager_cipher.h"

/***************************************************************************//**
 * @addtogroup nvm3
 * @{
 ******************************************************************************/

/***************************************************************************//**
 * @addtogroup nvm3halcrypto
 * @{
 ******************************************************************************/

/** @cond DO_NOT_INCLUDE_WITH_DOXYGEN */

/*******************************************************************************
 **************************   GLOBAL VARIABLES   *******************************
 ******************************************************************************/

static sl_se_key_descriptor_t nvm3_gcm_key_desc;

/*******************************************************************************
 **************************   LOCAL FUNCTIONS   ********************************
 ******************************************************************************/

/***************************************************************************//**
 * Initialize key descriptor for NVM3 crypto operations.
 ******************************************************************************/
static void nvm3_keyDescInit(void)
{
  nvm3_gcm_key_desc.type = SL_SE_KEY_TYPE_AES_128;
  nvm3_gcm_key_desc.size = 16;
  nvm3_gcm_key_desc.storage.method = SL_SE_KEY_STORAGE_INTERNAL_IMMUTABLE;
  nvm3_gcm_key_desc.flags = SL_SE_KEY_FLAG_IS_DEVICE_GENERATED;
  nvm3_gcm_key_desc.storage.location.slot = SL_SE_KEY_SLOT_NVM3_KEY;
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
 * SE manager initialization.
 ******************************************************************************/
static sl_status_t nvm3_halCryptoSeInit(void)
{
  // Initialize the SE manager
  if (sl_se_init() != SL_STATUS_OK) {
    return SL_STATUS_INITIALIZATION;
  }
  // Initialize key descriptor for NVM3 crypto operations
  nvm3_keyDescInit();
  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Generate true random number.
 ******************************************************************************/
static sl_status_t nvm3_halCryptoSeGenRandNum(uint8_t *output, size_t outputSize)
{
  sl_se_command_context_t cmd_ctx;
  sl_se_init_command_context(&cmd_ctx);
  if (sl_se_get_random(&cmd_ctx, output, outputSize) != SL_STATUS_OK) {
    return SL_STATUS_SECURITY_RANDOM_NUM_GEN_ERROR;
  }
  return SL_STATUS_OK;
}

/***************************************************************************//**
 * SE crypto encryption operation.
 ******************************************************************************/
static sl_status_t nvm3_halCryptoSeEncrypt(const uint8_t *nonce,
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
  sl_se_command_context_t cmd_ctx;
  sl_se_init_command_context(&cmd_ctx);
  if (sl_se_gcm_crypt_and_tag(&cmd_ctx,
                              &nvm3_gcm_key_desc,
                              SL_SE_ENCRYPT,
                              plainDataLen,
                              nonce,
                              NVM3_NONCE_SIZE,
                              aad,
                              aadLen,
                              plainData,
                              cipherData,
                              NVM3_GCM_TAG_SIZE,
                              tag) != SL_STATUS_OK) {
    return SL_STATUS_SECURITY_ENCRYPT_ERROR;
  }
  return SL_STATUS_OK;
}

/***************************************************************************//**
 * SE crypto decryption operation.
 ******************************************************************************/
static sl_status_t  nvm3_halCryptoSeDecrypt(const uint8_t *nonce,
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
  sl_se_command_context_t cmd_ctx;
  sl_se_init_command_context(&cmd_ctx);
  if (sl_se_gcm_auth_decrypt(&cmd_ctx,
                             &nvm3_gcm_key_desc,
                             cipherDataLen,
                             nonce,
                             NVM3_NONCE_SIZE,
                             aad,
                             aadLen,
                             cipherData,
                             plainData,
                             NVM3_GCM_TAG_SIZE,
                             tag) != SL_STATUS_OK) {
    return SL_STATUS_SECURITY_DECRYPT_ERROR;
  }
  return SL_STATUS_OK;
}

/*******************************************************************************
 ***************************   GLOBAL VARIABLES   ******************************
 ******************************************************************************/

const nvm3_HalCryptoHandle_t nvm3_halCryptoHandle  = {
  .init = nvm3_halCryptoSeInit,                  ///< Set the init function
  .genRandNum = nvm3_halCryptoSeGenRandNum,      ///< Set the random num generator function
  .encrypt = nvm3_halCryptoSeEncrypt,            ///< Set the encryption function
  .decrypt = nvm3_halCryptoSeDecrypt,            ///< Set the decryption function
};

/** @} (end addtogroup nvm3halcrypto) */
/** @} (end addtogroup nvm3) */
