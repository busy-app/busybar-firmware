/***************************************************************************//**
 * @file
 * @brief Provides hardware accelerated cryptographic primitives for series-3.
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
#include "sl_clock_manager.h"
#include "sl_code_classification.h"
#include "sl_se_manager.h"
#include "sl_se_manager_entropy.h"
#include "sl_se_manager_types.h"
#include "string.h"

#include "sli_sxsymcrypt.h"
#include "sxsymcrypt/aes.h"
#include "sxsymcrypt/aead.h"
#include "sxsymcrypt/blkcipher.h"
#include "sxsymcrypt/cmac.h"
#include "sxsymcrypt/cmmask.h"
#include "sxsymcrypt/keyref.h"
#include "sxsymcrypt/statuscodes.h"

#define MASKBITS 128
typedef union _hostcrypto_seed {
  uint8_t  u8[MASKBITS / 8];
  uint32_t u32[MASKBITS / 32];
} sli_crypto_seed_t;

typedef enum _sli_engine_idx {
  SLI_CRYPTO_HOSTSYMCRYPTO_IDX = 0,
  SLI_CRYPTO_LPWAES_IDX,
  SLI_CRYPTO_ENGINE_COUNT
} sli_engine_id_t;

#define INVALID_ENGINE(engine) \
  (((engine) != SLI_CRYPTO_LPWAES) && ((engine) != SLI_CRYPTO_HOSTSYMCRYPTO))

sl_status_t sli_crypto_countermeasure_reseed(sli_crypto_engine_t engine, sli_crypto_seed_t *seed)
{
  if (INVALID_ENGINE(engine)) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  sl_status_t rc;
  struct sxcmmask ctx = { 0 };
  for (uint16_t i = 0; i < sizeof(seed->u32) / sizeof(seed->u32[0]); i++) {
    rc = sli_sxsymcrypt_lock_cryptomaster_selection(engine, false);
    if (rc != SL_STATUS_OK) {
      break;
    }
    if ((SX_OK != sx_cm_load_mask(&ctx, seed->u32[i]))) {
      rc =  SL_STATUS_FAIL;
      break;
    }

    if (SX_OK != sx_cm_load_mask_wait(&ctx)) {
      rc = SL_STATUS_FAIL;
      break;
    }
    sli_sxsymcrypt_unlock_cryptomaster_selection();
  }

  if(rc == SL_STATUS_FAIL) {
    sli_sxsymcrypt_unlock_cryptomaster_selection();
  }

  return rc;
}

sl_status_t sli_crypto_init(void)
{
  sl_se_command_context_t cmd_ctx = { 0 };
  sl_status_t rc;

  sli_crypto_seed_t seeds[SLI_CRYPTO_ENGINE_COUNT] = { 0 };

  struct {
    sli_engine_id_t index;
    sli_crypto_engine_t descriptor;
    sli_crypto_seed_t *seed;
  } engines[SLI_CRYPTO_ENGINE_COUNT] = {
    { SLI_CRYPTO_HOSTSYMCRYPTO_IDX,
      SLI_CRYPTO_HOSTSYMCRYPTO,
      &seeds[SLI_CRYPTO_HOSTSYMCRYPTO_IDX] },
    { SLI_CRYPTO_LPWAES_IDX,
      SLI_CRYPTO_LPWAES,
      &seeds[SLI_CRYPTO_LPWAES_IDX] }
  };

  rc = sl_se_init_command_context(&cmd_ctx);
  if (rc != SL_STATUS_OK) {
    return rc;
  }

  rc = sl_se_get_random(&cmd_ctx, seeds, sizeof(seeds));

  if (rc != SL_STATUS_OK) {
    return rc;
  }

  rc = sl_se_deinit_command_context(&cmd_ctx);

  if (rc != SL_STATUS_OK) {
    return rc;
  }

  for (int i = 0; i < SLI_CRYPTO_ENGINE_COUNT; i++) {
    rc = sli_crypto_countermeasure_reseed(engines[i].descriptor, engines[i].seed);
    if (rc != SL_STATUS_OK) {
      return rc;
    }
  }

  return rc;
}

SL_CODE_CLASSIFY(SL_CODE_COMPONENT_SLI_CRYPTO, SL_CODE_CLASS_TIME_CRITICAL)
static sl_status_t sli_crypto_exit(sl_status_t status,
                                   bool is_isr,
                                   sli_cryptomaster_state_t *lpwaes_state)
{
  if (is_isr) {
    sli_crypto_lpwaes_restore_state(lpwaes_state);
  } else {
    // If preempted status is set and current status is not fail/invalid signature
    // then set status to preempted status
    if (sli_crypto_preempted_status != SL_STATUS_NOT_INITIALIZED
        && status == SL_STATUS_OK) {
      status = sli_crypto_preempted_status;
      sli_crypto_preempted_status = SL_STATUS_NOT_INITIALIZED;
    }
  }

  return status;
}

sl_status_t sli_crypto_gcm(sli_crypto_descriptor_t  *key_descriptor,
                           bool                     encrypt,
                           const unsigned char      *data_in,
                           size_t                   data_len,
                           unsigned char            *data_out,
                           const unsigned char      *iv,
                           const unsigned char      *aad,
                           size_t                   aad_len,
                           unsigned char            *tag,
                           size_t                   tag_len)
{
  EFM_ASSERT(key_descriptor != NULL);
  EFM_ASSERT(data_in != NULL);
  EFM_ASSERT(iv != NULL);

  struct sxaead aead;
  struct sxkeyref key_ref;

  if (key_descriptor->location == SLI_CRYPTO_KEY_LOCATION_PLAINTEXT) {
    key_ref = sx_keyref_load_material(key_descriptor->key.plaintext_key.key_size,
                                      (const char *)key_descriptor->key.plaintext_key.buffer.pointer);
  } else if (key_descriptor->location == SLI_CRYPTO_KEY_LOCATION_KSU) {
    key_ref = sx_keyref_load_by_id(key_descriptor->key.key_slot);
  } else {
    return SL_STATUS_INVALID_PARAMETER;
  }
  sl_status_t status;
  int sx_status;
  status = sli_sxsymcrypt_lock_cryptomaster_selection(key_descriptor->engine, key_descriptor->yield);
  if (status != SL_STATUS_OK) {
    return status;
  }
  if (encrypt) {
    sx_status = sx_aead_create_aesgcm_enc(&aead,
                                          &key_ref,
                                          (const char *)iv);
  } else {
    sx_status = sx_aead_create_aesgcm_dec(&aead,
                                          &key_ref,
                                          (const char *)iv);
  }
  status = sli_sxsymcrypt_unlock_cryptomaster_selection();
  if (status != SL_STATUS_OK) {
    return status;
  }
  if (sx_status != SX_OK) {
    return SL_STATUS_FAIL;
  }
  sx_status = sx_aead_feed_aad(&aead, (const char *)aad, aad_len);
  if (sx_status != SX_OK) {
    return SL_STATUS_FAIL;
  }
  sx_status = sx_aead_crypt(&aead, (const char *)data_in, data_len, (char *)data_out);
  if (sx_status != SX_OK) {
    return SL_STATUS_FAIL;
  }
  sx_status = sx_aead_truncate_tag(&aead, tag_len);
  if (sx_status != SX_OK) {
    return SL_STATUS_FAIL;
  }
  if (encrypt) {
    sx_status = sx_aead_produce_tag(&aead, (char *)tag);
  } else {
    sx_status = sx_aead_verify_tag(&aead, (const char *)tag);
  }
  if (sx_status != SX_OK) {
    return SL_STATUS_FAIL;
  }
  sx_status = sx_aead_wait(&aead);
  if (sx_status != SX_OK) {
    if (encrypt) {
      return SL_STATUS_FAIL;
    } else {
      return SL_STATUS_INVALID_SIGNATURE;
    }
  }
  return (sx_status == SX_OK ? SL_STATUS_OK : SL_STATUS_FAIL);
}

sl_status_t sli_crypto_gcm_encrypt(sli_crypto_descriptor_t  *key_descriptor,
                                   const unsigned char      *data_in,
                                   size_t                   data_len,
                                   unsigned char            *data_out,
                                   const unsigned char      *iv,
                                   const unsigned char      *aad,
                                   size_t                   aad_len,
                                   unsigned char            *tag,
                                   size_t                   tag_len)
{
  EFM_ASSERT(key_descriptor != NULL);
  EFM_ASSERT(data_in != NULL);
  EFM_ASSERT(data_out != NULL);
  EFM_ASSERT(iv != NULL);
  EFM_ASSERT(tag != NULL);
  if (key_descriptor->location == SLI_CRYPTO_KEY_LOCATION_PLAINTEXT) {
    EFM_ASSERT(key_descriptor->key.plaintext_key.buffer.pointer != NULL);
  }
  return sli_crypto_gcm(key_descriptor,
                        true,
                        data_in,
                        data_len,
                        data_out,
                        iv,
                        aad,
                        aad_len,
                        tag,
                        tag_len);
}

sl_status_t sli_crypto_gcm_decrypt(sli_crypto_descriptor_t  *key_descriptor,
                                   const unsigned char      *data_in,
                                   size_t                   data_len,
                                   unsigned char            *data_out,
                                   const unsigned char      *iv,
                                   const unsigned char      *aad,
                                   size_t                   aad_len,
                                   const unsigned char      *tag,
                                   size_t                   tag_len)
{
  EFM_ASSERT(key_descriptor != NULL);
  EFM_ASSERT(data_in != NULL);
  EFM_ASSERT(data_out != NULL);
  EFM_ASSERT(iv != NULL);
  EFM_ASSERT(tag != NULL);
  if (key_descriptor->location == SLI_CRYPTO_KEY_LOCATION_PLAINTEXT) {
    EFM_ASSERT(key_descriptor->key.plaintext_key.buffer.pointer != NULL);
  }
  return sli_crypto_gcm(key_descriptor,
                        false,
                        data_in,
                        data_len,
                        data_out,
                        iv,
                        aad,
                        aad_len,
                        (unsigned char*)tag,
                        tag_len);
}

sl_status_t sli_crypto_ccm(sli_crypto_descriptor_t  *key_descriptor,
                           bool                     encrypt,
                           unsigned char            *data_in,
                           size_t                   data_len,
                           unsigned char            *data_out,
                           const unsigned char      *iv,
                           size_t                   iv_len,
                           unsigned char            *aad,
                           size_t                   aad_len,
                           unsigned char            *tag,
                           size_t                   tag_len)
{
  EFM_ASSERT(key_descriptor != NULL);
  EFM_ASSERT(data_in != NULL);
  EFM_ASSERT(iv != NULL);

  struct sxaead aead;
  struct sxkeyref key_ref;
  bool is_isr = false;
  sli_cryptomaster_state_t lpwaes_state;

  if (key_descriptor->location == SLI_CRYPTO_KEY_LOCATION_PLAINTEXT) {
    key_ref = sx_keyref_load_material(key_descriptor->key.plaintext_key.key_size,
                                      (const char *)key_descriptor->key.plaintext_key.buffer.pointer);
  } else if (key_descriptor->location == SLI_CRYPTO_KEY_LOCATION_KSU) {
    key_ref = sx_keyref_load_by_id(key_descriptor->key.key_slot);
  } else {
    return SL_STATUS_INVALID_PARAMETER;
  }

  sl_status_t status;
  status = sli_sxsymcrypt_lock_cryptomaster_selection(key_descriptor->engine, key_descriptor->yield);
  if (status != SL_STATUS_OK) {
    if (status == SL_STATUS_ISR && key_descriptor->engine == SLI_CRYPTO_LPWAES) {
      is_isr = true;
      sli_crypto_lpwaes_save_state(&lpwaes_state);
    } else {
      return status;
    }
  }

  int sx_status;
  if (encrypt) {
    sx_status = sx_aead_create_aesccm_enc(&aead,
                                          &key_ref,
                                          (const char *)iv,
                                          iv_len,
                                          tag_len,
                                          aad_len,
                                          data_len);
  } else {
    sx_status = sx_aead_create_aesccm_dec(&aead,
                                          &key_ref,
                                          (const char *)iv,
                                          iv_len,
                                          tag_len,
                                          aad_len,
                                          data_len);
  }
  status = sli_sxsymcrypt_unlock_cryptomaster_selection();
  if (status != SL_STATUS_OK) {
    return sli_crypto_exit(status, is_isr, &lpwaes_state);
  }
  if (sx_status != SX_OK) {
    return sli_crypto_exit(SL_STATUS_FAIL, is_isr, &lpwaes_state);
  }
  sx_status = sx_aead_feed_aad(&aead, (const char *)aad, aad_len);
  if (sx_status != SX_OK) {
    return sli_crypto_exit(SL_STATUS_FAIL, is_isr, &lpwaes_state);
  }
  if (data_out == NULL) {
    unsigned char output[128] = { 0 };
    EFM_ASSERT(data_len <= sizeof(output));
    sx_status = sx_aead_crypt(&aead, (const char *)data_in, data_len, (char *)output);
  } else {
    sx_status = sx_aead_crypt(&aead, (const char *)data_in, data_len, (char *)data_out);
  }
  if (sx_status != SX_OK) {
    return sli_crypto_exit(SL_STATUS_FAIL, is_isr, &lpwaes_state);
  }
  if (encrypt) {
    sx_status = sx_aead_produce_tag(&aead, (char *)tag);
  } else {
    sx_status = sx_aead_verify_tag(&aead, (const char *)tag);
  }
  if (sx_status != SX_OK) {
    return sli_crypto_exit(SL_STATUS_FAIL, is_isr, &lpwaes_state);
  }
  sx_status = sx_aead_wait(&aead);
  if (sx_status != SX_OK) {
    if (encrypt) {
      return sli_crypto_exit(SL_STATUS_FAIL, is_isr, &lpwaes_state);
    } else {
      return sli_crypto_exit(SL_STATUS_INVALID_SIGNATURE, is_isr, &lpwaes_state);
    }
  }

  return sli_crypto_exit(SL_STATUS_OK, is_isr, &lpwaes_state);
}

sl_status_t sli_crypto_ecb(sli_crypto_descriptor_t *key_descriptor,
                           bool                    encrypt,
                           const unsigned char     input[SLI_CRYPTO_AES_BLOCK_SIZE],
                           volatile unsigned char  output[SLI_CRYPTO_AES_BLOCK_SIZE])
{
  EFM_ASSERT(key_descriptor != NULL);
  EFM_ASSERT(input != NULL);
  EFM_ASSERT(output != NULL);

  struct sxblkcipher aes_ecb;
  struct sxkeyref key_ref;
  bool is_isr = false;
  sli_cryptomaster_state_t lpwaes_state;

  if (key_descriptor->location == SLI_CRYPTO_KEY_LOCATION_PLAINTEXT) {
    key_ref = sx_keyref_load_material(key_descriptor->key.plaintext_key.key_size,
                                      (const char *)key_descriptor->key.plaintext_key.buffer.pointer);
  } else if (key_descriptor->location == SLI_CRYPTO_KEY_LOCATION_KSU) {
    key_ref = sx_keyref_load_by_id(key_descriptor->key.key_slot);
  } else {
    return SL_STATUS_INVALID_PARAMETER;
  }

  sl_status_t status;
  status = sli_sxsymcrypt_lock_cryptomaster_selection(key_descriptor->engine, key_descriptor->yield);
  if (status != SL_STATUS_OK) {
    if (status == SL_STATUS_ISR && key_descriptor->engine == SLI_CRYPTO_LPWAES) {
      sli_crypto_lpwaes_save_state(&lpwaes_state);
      is_isr = true;
    } else {
      return status;
    }
  }

  int sx_status;
  if (encrypt) {
    sx_status = sx_blkcipher_create_aesecb_enc(&aes_ecb,
                                               &key_ref);
  } else {
    sx_status = sx_blkcipher_create_aesecb_dec(&aes_ecb,
                                               &key_ref);
  }
  status = sli_sxsymcrypt_unlock_cryptomaster_selection();
  if (status != SL_STATUS_OK) {
    return sli_crypto_exit(status, is_isr, &lpwaes_state);
  }
  if (sx_status != SX_OK) {
    return sli_crypto_exit(SL_STATUS_FAIL, is_isr, &lpwaes_state);
  }
  sx_status = sx_blkcipher_crypt(&aes_ecb, (const char *)input, SLI_CRYPTO_AES_BLOCK_SIZE, (char *)output);
  if (sx_status != SX_OK) {
    return sli_crypto_exit(SL_STATUS_FAIL, is_isr, &lpwaes_state);
  }
  sx_status = sx_blkcipher_run(&aes_ecb);
  if (sx_status != SX_OK) {
    return sli_crypto_exit(SL_STATUS_FAIL, is_isr, &lpwaes_state);
  }

  sx_status = sx_blkcipher_wait(&aes_ecb);
  if (sx_status != SX_OK) {
    return sli_crypto_exit(SL_STATUS_FAIL, is_isr, &lpwaes_state);
  }

  return sli_crypto_exit(SL_STATUS_OK, is_isr, &lpwaes_state);
}

sl_status_t sli_crypto_cmac(sli_crypto_descriptor_t    *key_descriptor,
                            const unsigned char *const input,
                            unsigned int               length,
                            volatile unsigned char     output[SLI_CRYPTO_AES_BLOCK_SIZE])
{
  EFM_ASSERT(key_descriptor != NULL);
  EFM_ASSERT(input != NULL);
  EFM_ASSERT(output != NULL);

  struct sxmac aes_cmac;
  struct sxkeyref key_ref;
  bool is_isr = false;
  sli_cryptomaster_state_t lpwaes_state;

  if (key_descriptor->location == SLI_CRYPTO_KEY_LOCATION_PLAINTEXT) {
    key_ref = sx_keyref_load_material(key_descriptor->key.plaintext_key.key_size,
                                      (const char *)key_descriptor->key.plaintext_key.buffer.pointer);
  } else if (key_descriptor->location == SLI_CRYPTO_KEY_LOCATION_KSU) {
    key_ref = sx_keyref_load_by_id(key_descriptor->key.key_slot);
  } else {
    return SL_STATUS_INVALID_PARAMETER;
  }

  sl_status_t status;
  status = sli_sxsymcrypt_lock_cryptomaster_selection(key_descriptor->engine, key_descriptor->yield);
  if (status != SL_STATUS_OK) {
    if (status == SL_STATUS_ISR && key_descriptor->engine == SLI_CRYPTO_LPWAES) {
      sli_crypto_lpwaes_save_state(&lpwaes_state);
      is_isr = true;
    } else {
      return status;
    }
  }

  int sx_status;

  sx_status = sx_mac_create_aescmac(&aes_cmac,
                                    &key_ref);

  status = sli_sxsymcrypt_unlock_cryptomaster_selection();
  if (status != SL_STATUS_OK) {
    return sli_crypto_exit(status, is_isr, &lpwaes_state);
  }
  if (sx_status != SX_OK) {
    return sli_crypto_exit(SL_STATUS_FAIL, is_isr, &lpwaes_state);
  }
  sx_status = sx_mac_feed(&aes_cmac, (const char *)input, length);
  if (sx_status != SX_OK) {
    return sli_crypto_exit(SL_STATUS_FAIL, is_isr, &lpwaes_state);
  }
  sx_status = sx_mac_generate(&aes_cmac, (char *)output);
  if (sx_status != SX_OK) {
    return sli_crypto_exit(SL_STATUS_FAIL, is_isr, &lpwaes_state);
  }
  sx_status = sx_mac_wait(&aes_cmac);
  if (sx_status != SX_OK) {
    return sli_crypto_exit(SL_STATUS_FAIL, is_isr, &lpwaes_state);
  }

  return sli_crypto_exit(SL_STATUS_OK, is_isr, &lpwaes_state);
}

sl_status_t sli_crypto_ctr(sli_crypto_descriptor_t *key_descriptor,
                           const unsigned char     input[SLI_CRYPTO_AES_BLOCK_SIZE],
                           const unsigned char     iv_in[SLI_CRYPTO_AES_BLOCK_SIZE],
                           volatile unsigned char  iv_out[SLI_CRYPTO_AES_BLOCK_SIZE],
                           volatile unsigned char  output[SLI_CRYPTO_AES_BLOCK_SIZE])
{
  EFM_ASSERT(key_descriptor != NULL);
  EFM_ASSERT(input != NULL);
  EFM_ASSERT(output != NULL);
  EFM_ASSERT(iv_in != NULL);

  struct sxblkcipher aes_ctr;
  struct sxkeyref key_ref;
  bool is_isr;
  sli_cryptomaster_state_t lpwaes_state;

  if (key_descriptor->location == SLI_CRYPTO_KEY_LOCATION_PLAINTEXT) {
    key_ref = sx_keyref_load_material(key_descriptor->key.plaintext_key.key_size,
                                      (const char *)key_descriptor->key.plaintext_key.buffer.pointer);
  } else if (key_descriptor->location == SLI_CRYPTO_KEY_LOCATION_KSU) {
    key_ref = sx_keyref_load_by_id(key_descriptor->key.key_slot);
  } else {
    return SL_STATUS_INVALID_PARAMETER;
  }

  sl_status_t status;
  status = sli_sxsymcrypt_lock_cryptomaster_selection(key_descriptor->engine, key_descriptor->yield);
  if (status != SL_STATUS_OK) {
    if (status == SL_STATUS_ISR && key_descriptor->engine == SLI_CRYPTO_LPWAES) {
      sli_crypto_lpwaes_save_state(&lpwaes_state);
      is_isr = true;
    } else {
      return status;
    }
  }

  int sx_status;
  sx_status = sx_blkcipher_create_aesctr_enc(&aes_ctr,
                                             &key_ref,
                                             (const char *)iv_in);

  status = sli_sxsymcrypt_unlock_cryptomaster_selection();
  if (status != SL_STATUS_OK) {
    return sli_crypto_exit(status, is_isr, &lpwaes_state);
  }
  if (sx_status != SX_OK) {
    return sli_crypto_exit(SL_STATUS_FAIL, is_isr, &lpwaes_state);
  }
  sx_status = sx_blkcipher_crypt(&aes_ctr, (const char *)input, SLI_CRYPTO_AES_BLOCK_SIZE, (char *)output);
  if (sx_status != SX_OK) {
    return sli_crypto_exit(SL_STATUS_FAIL, is_isr, &lpwaes_state);
  }
  sx_status = sx_blkcipher_save_state(&aes_ctr);
  if (sx_status != SX_OK) {
    return sli_crypto_exit(SL_STATUS_FAIL, is_isr, &lpwaes_state);
  }
  sx_status = sx_blkcipher_wait(&aes_ctr);
  if (sx_status != SX_OK) {
    return sli_crypto_exit(SL_STATUS_FAIL, is_isr, &lpwaes_state);
  }

  if (iv_out != NULL) {
    memcpy((void* )iv_out, aes_ctr.extramem, SLI_CRYPTO_AES_BLOCK_SIZE);
  }

  return sli_crypto_exit(SL_STATUS_OK, is_isr, &lpwaes_state);
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
  if (key_descriptor->location == SLI_CRYPTO_KEY_LOCATION_PLAINTEXT) {
    EFM_ASSERT(key_descriptor->key.plaintext_key.buffer.pointer != NULL);
  }

  return sli_crypto_ccm(key_descriptor,
                        false,
                        data,
                        length,
                        data,
                        iv,
                        13,
                        &header,
                        1,
                        tag,
                        4);
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
  if (key_descriptor->location == SLI_CRYPTO_KEY_LOCATION_PLAINTEXT) {
    EFM_ASSERT(key_descriptor->key.plaintext_key.buffer.pointer != NULL);
  }

  return sli_crypto_ccm(key_descriptor,
                        true,
                        data,
                        length,
                        data,
                        iv,
                        13,
                        &header,
                        1,
                        tag,
                        4);
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
  if (key_descriptor->location == SLI_CRYPTO_KEY_LOCATION_PLAINTEXT) {
    EFM_ASSERT(key_descriptor->key.plaintext_key.buffer.pointer != NULL);
  }

  return sli_crypto_ccm(key_descriptor,
                        encrypt,
                        (unsigned char *)data_in,
                        length,
                        data_out,
                        iv,
                        13,
                        (unsigned char *)aad,
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
  int irk_index_temp = -1;

  EFM_ASSERT(key_descriptor != NULL);
  EFM_ASSERT(irk_index != NULL);

  CMU->CLKEN0_SET = CMU_CLKEN0_LPWAES;
  CMU->CLKEN1_SET = CMU_CLKEN1_RPA;
  RPA->SWRST_SET = RPA_SWRST_SWRST;
  while ((RPA->SWRST & _RPA_SWRST_RESETTING_MASK) == RPA_SWRST_RESETTING) ;
  RPA->EN_SET = RPA_EN_EN;

  // Key mask is 64 bit to be future proof, but the current requirement need only 32 IRKs
  // and this should be the value used for verification purposes.
  RPA->IRKMASK = keymask & _RPA_IRKMASK_IRKMASK_MASK;

  if (key_descriptor->location == SLI_CRYPTO_KEY_LOCATION_KSU) {
    RPA->CTRL_CLR = RPA_CTRL_IRKDMEMSEL;
    RPA->IRKKEYSLOTID = key_descriptor->key.key_slot & _RPA_IRKKEYSLOTID_MASK;
  } else {
    RPA->CTRL_SET = RPA_CTRL_IRKDMEMSEL;
    EFM_ASSERT(key_descriptor->key.plaintext_key.buffer.pointer != NULL);
    RPA->IRKADDR = (uint32_t)key_descriptor->key.plaintext_key.buffer.pointer;
  }
  RPA->CTRL_SET = irk_len & _RPA_CTRL_NUMIRK_MASK;
  RPA->CTRL_SET = RPA_CTRL_RPAAESSELREQ;
  // Wait until the RPA gets the CryptoMaster AES control
  while ((RPA->STATUS & _RPA_STATUS_AESRDY_MASK) != RPA_STATUS_AESRDY) ;
  RPA->PRAND = prand & _RPA_PRAND_MASK;
  RPA->HASH = hash & _RPA_HASH_MASK;
  RPA->CMD_SET = RPA_CMD_START;
  // Wait for the interrupt flag
  while (RPA->IF == _RPA_IF_RESETVALUE) ;

  // Validate the result
  if ((RPA->IF & _RPA_IF_RPADONEIF_MASK) != RPA_IF_RPADONEIF) {
    RPA->EN_CLR = RPA_EN_EN;
    CMU->CLKEN1_CLR = CMU_CLKEN1_RPA;
    CMU->CLKEN0_CLR = CMU_CLKEN0_LPWAES;
    return SL_STATUS_INVALID_STATE;
  } else {
    // RPA computation has finished
    irk_index_temp = (int)((RPA->STATUS & _RPA_STATUS_IRKINDEX_MASK) >> _RPA_STATUS_IRKINDEX_SHIFT) - 1u;
    if ((RPA->STATUS & _RPA_STATUS_IRKNOTFOUND_MASK) != RPA_STATUS_IRKNOTFOUND) {
      if (irk_index_temp == -1) {
        // IRKNOTFOUND is not set and we did not get a match
        RPA->EN_CLR = RPA_EN_EN;
        CMU->CLKEN1_CLR = CMU_CLKEN1_RPA;
        CMU->CLKEN0_CLR = CMU_CLKEN0_LPWAES;
        return SL_STATUS_FAIL;
      }
    }
    *irk_index = irk_index_temp;
  }
  RPA->EN_CLR = RPA_EN_EN;
  CMU->CLKEN1_CLR = CMU_CLKEN1_RPA;
  CMU->CLKEN0_CLR = CMU_CLKEN0_LPWAES;

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

  if (key_descriptor->location == SLI_CRYPTO_KEY_LOCATION_PLAINTEXT) {
    EFM_ASSERT(key_descriptor->key.plaintext_key.key_size == keybits / 8);
    EFM_ASSERT(key_descriptor->key.plaintext_key.buffer.pointer != NULL);
  }

  return sli_crypto_ctr(key_descriptor,
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

  if (key_descriptor->location == SLI_CRYPTO_KEY_LOCATION_PLAINTEXT) {
    EFM_ASSERT(key_descriptor->key.plaintext_key.key_size == keybits / 8);
    EFM_ASSERT(key_descriptor->key.plaintext_key.buffer.pointer != NULL);
  }

  return sli_crypto_ecb(key_descriptor,
                        encrypt,
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

  if (key_descriptor->location == SLI_CRYPTO_KEY_LOCATION_PLAINTEXT) {
    EFM_ASSERT(key_descriptor->key.plaintext_key.key_size == keybits / 8);
    EFM_ASSERT(key_descriptor->key.plaintext_key.buffer.pointer != NULL);
  }

  return sli_crypto_cmac(key_descriptor,
                         input,
                         length,
                         output);
}
