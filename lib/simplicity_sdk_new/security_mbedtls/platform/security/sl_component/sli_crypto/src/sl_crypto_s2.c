/***************************************************************************//**
 * @file
 * @brief Provides hardware accelerated cryptographic primitives for series-2.
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
#include "em_device.h"
#include "sli_crypto.h"
#include "sl_assert.h"
#include "sli_protocol_crypto.h"

sl_status_t sli_crypto_init(void)
{
  #if defined(SLI_RADIOAES_REQUIRES_MASKING)
  sli_aes_seed_mask();
  #endif

  return SL_STATUS_OK;
}
/***************************************************************************//**
 * @brief          CCM buffer authenticated decryption optimized for BLE
 ******************************************************************************/
sl_status_t sli_crypto_ccm_auth_decrypt_ble(sli_crypto_descriptor_t     *key_descriptor,
                                            unsigned char               *data,
                                            size_t                      length,
                                            const unsigned char         *iv,
                                            unsigned char               header,
                                            unsigned char               *tag)
{
  EFM_ASSERT(key_descriptor != NULL);
  EFM_ASSERT(data != NULL);
  EFM_ASSERT(iv != NULL);
  EFM_ASSERT(tag != NULL);
  EFM_ASSERT(key_descriptor->location == SLI_CRYPTO_KEY_LOCATION_PLAINTEXT);
  EFM_ASSERT(key_descriptor->key.plaintext_key.buffer.pointer != NULL);

  return sli_ccm_auth_decrypt_ble(data,
                                  length,
                                  (const unsigned char *)key_descriptor->key.plaintext_key.buffer.pointer,
                                  iv,
                                  header,
                                  tag);
}

/***************************************************************************//**
 * @brief          CCM buffer encryption optimized for BLE
 ******************************************************************************/
sl_status_t sli_crypto_ccm_encrypt_and_tag_ble(sli_crypto_descriptor_t     *key_descriptor,
                                               unsigned char               *data,
                                               size_t                      length,
                                               const unsigned char         *iv,
                                               unsigned char               header,
                                               unsigned char               *tag)
{
  EFM_ASSERT(key_descriptor != NULL);
  EFM_ASSERT(data != NULL);
  EFM_ASSERT(iv != NULL);
  EFM_ASSERT(tag != NULL);
  EFM_ASSERT(key_descriptor->location == SLI_CRYPTO_KEY_LOCATION_PLAINTEXT);
  EFM_ASSERT(key_descriptor->key.plaintext_key.buffer.pointer != NULL);

  return sli_ccm_encrypt_and_tag_ble(data,
                                     length,
                                     (const unsigned char *)key_descriptor->key.plaintext_key.buffer.pointer,
                                     iv,
                                     header,
                                     tag);
}

/***************************************************************************//**
 * @brief          CCM buffer authenticated decryption optimized for Zigbee
 ******************************************************************************/
sl_status_t sli_crypto_ccm_zigbee(sli_crypto_descriptor_t     *key_descriptor,
                                  bool                        encrypt,
                                  const unsigned char         *data_in,
                                  unsigned char               *data_out,
                                  size_t                      length,
                                  const unsigned char         *iv,
                                  const unsigned char         *aad,
                                  size_t                      aad_len,
                                  unsigned char               *tag,
                                  size_t                      tag_len)
{
  EFM_ASSERT(key_descriptor != NULL);
  EFM_ASSERT(data_in != NULL);
  EFM_ASSERT(iv != NULL);
  EFM_ASSERT(key_descriptor->location == SLI_CRYPTO_KEY_LOCATION_PLAINTEXT);
  EFM_ASSERT(key_descriptor->key.plaintext_key.buffer.pointer != NULL);

  return sli_ccm_zigbee(encrypt,
                        data_in,
                        data_out,
                        length,
                        (const unsigned char *)key_descriptor->key.plaintext_key.buffer.pointer,
                        iv,
                        aad,
                        aad_len,
                        tag,
                        tag_len);
}

/***************************************************************************//**
 * @brief          Process a table of BLE RPA device keys and look for a
 *                 match against the supplied hash
 ******************************************************************************/
sl_status_t sli_crypto_process_rpa(sli_crypto_descriptor_t     *key_descriptor,
                                   size_t                      irk_len,
                                   uint64_t                    keymask,
                                   uint32_t                    prand,
                                   uint32_t                    hash,
                                   int                         *irk_index)
{
  EFM_ASSERT(key_descriptor != NULL);
  EFM_ASSERT(irk_index != NULL);
  EFM_ASSERT(key_descriptor->location == SLI_CRYPTO_KEY_LOCATION_PLAINTEXT);
  EFM_ASSERT(key_descriptor->key.plaintext_key.buffer.pointer != NULL);
  (void)irk_len;
  const unsigned char *keytable
    = (const unsigned char *)key_descriptor->key.plaintext_key.buffer.pointer;
  *irk_index = sli_process_ble_rpa(keytable,
                                   (uint32_t)keymask,
                                   prand,
                                   hash);
  if (*irk_index == -1) {
    return SL_STATUS_FAIL;
  }

  return SL_STATUS_OK;
}

// /***************************************************************************//**
// * @brief          AES-CTR block encryption/decryption optimized for radio
// *******************************************************************************/
sl_status_t sli_crypto_aes_ctr_radio(sli_crypto_descriptor_t  *key_descriptor,
                                     unsigned int             keybits,
                                     const unsigned char      input[SLI_CRYPTO_AES_BLOCK_SIZE],
                                     const unsigned char      iv_in[SLI_CRYPTO_AES_BLOCK_SIZE],
                                     volatile unsigned char   iv_out[SLI_CRYPTO_AES_BLOCK_SIZE],
                                     volatile unsigned char   output[SLI_CRYPTO_AES_BLOCK_SIZE])
{
  EFM_ASSERT(key_descriptor != NULL);
  EFM_ASSERT(keybits == 128 || keybits == 192 || keybits == 256);
  EFM_ASSERT(input != NULL);
  EFM_ASSERT(iv_in != NULL);
  EFM_ASSERT(output != NULL);
  EFM_ASSERT(key_descriptor->location == SLI_CRYPTO_KEY_LOCATION_PLAINTEXT);
  EFM_ASSERT(key_descriptor->key.plaintext_key.key_size == keybits / 8);
  EFM_ASSERT(key_descriptor->key.plaintext_key.buffer.pointer != NULL);

  return sli_aes_crypt_ctr_radio( (const unsigned char *)key_descriptor->key.plaintext_key.buffer.pointer,
                                  keybits,
                                  input,
                                  iv_in,
                                  iv_out,
                                  output);
}

/***************************************************************************//**
 * @brief          AES-ECB block encryption/decryption optimized for radio
 ********************************************************************************/
sl_status_t sli_crypto_aes_ecb_radio(bool                    encrypt,
                                     sli_crypto_descriptor_t *key_descriptor,
                                     unsigned int            keybits,
                                     const unsigned char     input[SLI_CRYPTO_AES_BLOCK_SIZE],
                                     volatile unsigned char  output[SLI_CRYPTO_AES_BLOCK_SIZE])
{
  EFM_ASSERT(key_descriptor != NULL);
  EFM_ASSERT(keybits == 128 || keybits == 192 || keybits == 256);
  EFM_ASSERT(input != NULL);
  EFM_ASSERT(output != NULL);
  EFM_ASSERT(key_descriptor->location == SLI_CRYPTO_KEY_LOCATION_PLAINTEXT);
  EFM_ASSERT(key_descriptor->key.plaintext_key.key_size == keybits / 8);
  EFM_ASSERT(key_descriptor->key.plaintext_key.buffer.pointer != NULL);

  return sli_aes_crypt_ecb_radio(encrypt,
                                 (const unsigned char *)key_descriptor->key.plaintext_key.buffer.pointer,
                                 keybits,
                                 input,
                                 output);
}

/***************************************************************************//**
 * @brief          AES-CMAC calculation optimized for radio
 ********************************************************************************/
sl_status_t sli_crypto_aes_cmac_radio(sli_crypto_descriptor_t *key_descriptor,
                                      unsigned int            keybits,
                                      const unsigned char     *input,
                                      unsigned int            length,
                                      volatile unsigned char  output[SLI_CRYPTO_AES_BLOCK_SIZE])
{
  EFM_ASSERT(key_descriptor != NULL);
  EFM_ASSERT(keybits == 128 || keybits == 192 || keybits == 256);
  EFM_ASSERT(input != NULL);
  EFM_ASSERT(length == SLI_CRYPTO_AES_BLOCK_SIZE);
  EFM_ASSERT(output != NULL);
  EFM_ASSERT(key_descriptor->location == SLI_CRYPTO_KEY_LOCATION_PLAINTEXT);
  EFM_ASSERT(key_descriptor->key.plaintext_key.key_size == keybits / 8);
  EFM_ASSERT(key_descriptor->key.plaintext_key.buffer.pointer != NULL);

  return sli_aes_cmac_radio((const unsigned char *)key_descriptor->key.plaintext_key.buffer.pointer,
                            keybits,
                            input,
                            length,
                            output);
}
