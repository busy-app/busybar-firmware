/***************************************************************************//**
 * @file
 * @brief Key Slot Unit (KSU) manager API for secure key storage, allocation, and lifecycle operations in Silicon Labs hardware.
 *        Provides functions to manage key slots, import, generate, copy, and delete cryptographic keys, and map user references to hardware key slots and crypto engines.
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

#ifndef SLI_CRYPTO_KSU_MANAGER_H
#define SLI_CRYPTO_KSU_MANAGER_H

#include "sl_se_manager_internal_keys.h"
#include "sl_se_manager_util.h"
#include "sl_se_manager_types.h"
#include "sli_se_manager_internal.h"
#include "sl_status.h"
#include "sl_component_catalog.h"
#include "sli_ksu_keyslots_config.h"

#include <stddef.h>

typedef enum {
  SLI_KSU_SLOT_STATUS_AVAILABLE = 0,
  SLI_KSU_SLOT_STATUS_IN_USE = 1,
  SLI_KSU_SLOT_STATUS_RESERVED = 2
} sli_ksu_key_slot_status_t;

typedef struct sli_ksu_slot{
  void *user_ref;
  sli_ksu_key_slot_status_t state;
  uint8_t crypto_engine_id;
} sli_ksu_slot_t;

enum crypto_engines {
  SLI_CRYPTOMASTER_AES = 0,
  SLI_CRYPTOMASTER_HASH = 1
};

/***************************************************************************//**
 * @brief                Import plaintext key into KSU
 *
 * @param key_desc       Output key descriptor
 * @param key_data       Input key data buffer
 * @param key_data_len   length of input key data
 * @param user_ref       User provided reference to be  mapped to KSU key slot used
 * @param key_slot_id    KSU keyslot ID of imported key
 *
 * @return               SL_STATUS_OK if successful, relevant status code on error
 ******************************************************************************/
sl_status_t sli_ksu_key_slot_import(sl_se_key_descriptor_t *key_desc,
                                    const uint8_t *key_data,
                                    size_t key_data_len,
                                    void *user_ref);

/***************************************************************************//**
 * @brief                Initialize the KSU manager
 * @return               SL_STATUS_OK if successful, relevant status code on error
 ******************************************************************************/
sl_status_t sli_ksu_init(void);

/***************************************************************************//**
 * @brief
 *   Retrieve the KSU key slot ID associated with a user reference.
 *
 * @param[in]  user_ref
 *   User-provided reference mapped to a KSU key slot.
 * @param[out] key_slot_id
 *   Pointer to store the resulting key slot ID.
 *
 * @return
 *   SL_STATUS_OK if successful, or a relevant error status code.
 ******************************************************************************/
sl_status_t sli_ksu_get_key_slot_id_from_user_ref(void *user_ref, uint8_t *key_slot_id);

/***************************************************************************//**
 * @brief
 *   Retrieve the crypto engine ID associated with a user reference.
 *
 * @param[in]  user_ref
 *   User-provided reference mapped to a crypto engine.
 * @param[out] crypto_engine_id
 *   Pointer to store the resulting crypto engine ID.
 *
 * @return
 *   SL_STATUS_OK if successful, or a relevant error status code.
 ******************************************************************************/
sl_status_t sli_ksu_get_crypto_engine_id_from_user_ref(void *user_ref, uint8_t *crypto_engine_id);

/***************************************************************************//**
 * @brief
 *   Delete a key from the Key Storage Unit (KSU).
 *
 * @param[in] key_desc
 *   Pointer to the key descriptor of the key to delete.
 *
 * @return
 *   SL_STATUS_OK if successful, or a relevant error status code.
 ******************************************************************************/
sl_status_t sli_ksu_delete_key(sl_se_key_descriptor_t *key_desc);

/***************************************************************************//**
 * @brief                Copy key from external location to KSU
 *
 * @param source_key_desc Source key descriptor
 * @param target_key_desc Target KSU key descriptor
 * @param user_ref       User provided reference to be mapped to KSU key slot
 *
 * @return               SL_STATUS_OK if successful, relevant status code on error
 ******************************************************************************/
sl_status_t sli_ksu_key_slot_copy(sl_se_key_descriptor_t *source_key_desc,
                                  sl_se_key_descriptor_t *target_key_desc,
                                  void *user_ref);

/***************************************************************************//**
 * @brief                Allocate KSU slot for a key
 *
 * @param key_desc       Key descriptor to set slot information in
 * @param user_ref       User provided reference to be mapped to KSU key slot
 * @param crypto_engine_id Crypto engine ID for the key
 *
 * @return               SL_STATUS_OK if successful, relevant status code on error
 ******************************************************************************/
sl_status_t sli_ksu_allocate_key_slot(sl_se_key_descriptor_t *key_desc,
                                      void *user_ref,
                                      uint8_t crypto_engine_id);

/***************************************************************************//**
 * @brief                Generate a random key directly into KSU
 *
 * @param key_desc       Key descriptor with KSU storage method
 * @param user_ref       User provided reference to be mapped to KSU key slot
 *
 * @return               SL_STATUS_OK if successful, relevant status code on error
 ******************************************************************************/
sl_status_t sli_ksu_key_slot_generate(sl_se_key_descriptor_t *key_desc,
                                      void *user_ref);

#endif // SLI_CRYPTO_KSU_MANAGER_H
