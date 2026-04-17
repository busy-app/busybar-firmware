/***************************************************************************//**
 * @file
 * @brief KSU Key Slot Manager for hardware-accelerated cryptography. Manages allocation, import, generation, and deletion of key slots in the Silicon Labs Key Storage Unit (KSU) for secure key handling and integration with SE hardware.
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

#include "em_device.h"
#include "sli_crypto_ksu_manager.h"
#include "sl_se_manager.h"

#if defined (SL_COMPONENT_CATALOG_PRESENT)
  #include "sl_component_catalog.h"
#endif

// -----------------------------------------------------------------------------
// Static variable Declarations

// Create KSU slots.
static sli_ksu_slot_t ksu_slots[SLI_KSU_MAX_KEY_SLOTS] = { 0 };

#if defined(SL_CATALOG_MICRIUMOS_KERNEL_PRESENT) || defined(SL_CATALOG_FREERTOS_KERNEL_PRESENT)
// Threading support (as opposed to API calls only from a single thread)
// is currently required in RTOS mode.
// Lock mutex for synchronizing multiple threads calling into the KSU Manager API.
#include "sli_psec_osal.h"
static sli_psec_osal_lock_t sli_ksu_lock = { 0 };
#endif

// -----------------------------------------------------------------------------
// Function Declarations

/**
 * @brief
 *   Initialize KSU Manager resources
 */
sl_status_t sli_ksu_init(void)
{
  sl_status_t sl_status = sl_se_init();
  if (sl_status != SL_STATUS_OK) {
    return sl_status;
  }
#if defined(SL_CATALOG_MICRIUMOS_KERNEL_PRESENT) || defined(SL_CATALOG_FREERTOS_KERNEL_PRESENT)
  // Initialize KSU lock
  sl_status = sli_psec_osal_init_lock(&sli_ksu_lock);
  if (sl_status != SL_STATUS_OK) {
    return sl_status;
  }
#endif

  // Initialize reserved KSU slots
  for (int i = 0; i < SLI_KSU_KEY_SLOT_USER_START; i++) {
    ksu_slots[i].user_ref = NULL;
    ksu_slots[i].state = SLI_KSU_SLOT_STATUS_RESERVED; // Reserved slots are not available for use
    ksu_slots[i].crypto_engine_id = 0; // Initialize engine ID to 0
  }

  // Initialize user KSU slots
  // Start from SLI_KSU_KEY_SLOT_USER_START to SLI_KSU_MAX_KEY_SLOTS
  for (size_t i = SLI_KSU_KEY_SLOT_USER_START; i < SLI_KSU_MAX_KEY_SLOTS; i++) {
    ksu_slots[i].user_ref = NULL;
    ksu_slots[i].state = SLI_KSU_SLOT_STATUS_AVAILABLE;
    ksu_slots[i].crypto_engine_id = 0; // Initialize engine ID to 0
  }

  return SL_STATUS_OK;
}

/**
 * @brief
 *   Acquire the KSU lock for exclusive access if necessary (thread mode).
 */
static inline sl_status_t sli_ksu_lock_acquire(void)
{
  sl_status_t sl_status = SL_STATUS_OK;
#if defined(SL_CATALOG_MICRIUMOS_KERNEL_PRESENT) || defined(SL_CATALOG_FREERTOS_KERNEL_PRESENT)
  // Acquire the KSU lock (mutex) to protect KSU slot operations
  sl_status = sli_psec_osal_take_lock(&sli_ksu_lock);
  if (sl_status != SL_STATUS_OK) {
    return sl_status;
  }
#endif
  return sl_status;
}

/**
 * @brief
 *   Release the KSU lock if necessary (thread mode).
 */
static inline sl_status_t sli_ksu_lock_release(void)
{
  sl_status_t sl_status = SL_STATUS_OK;
#if defined(SL_CATALOG_MICRIUMOS_KERNEL_PRESENT) || defined(SL_CATALOG_FREERTOS_KERNEL_PRESENT)
  // Acquire the KSU lock (mutex) to protect KSU slot operations
  sl_status = sli_psec_osal_give_lock(&sli_ksu_lock);
  if (sl_status != SL_STATUS_OK) {
    return sl_status;
  }
#endif
  return sl_status;
}

/**
 * @brief
 *   Find available key KSU slot in key descriptor
 */
static inline sl_status_t sli_ksu_find_free_user_slot(sl_se_key_descriptor_t *key_desc,
                                                 void *user_ref)
{
  if (key_desc == NULL || user_ref == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  uint8_t key_slot_id = 0;
  // Check if the user_ref is already associated with a slot
  if (sli_ksu_get_key_slot_id_from_user_ref(user_ref, &key_slot_id) == SL_STATUS_OK) {
    return SL_STATUS_ALREADY_EXISTS;
  }

  // Perform a linear search to find the first empty slot
  for (size_t i = SLI_KSU_KEY_SLOT_USER_START; i < SLI_KSU_MAX_KEY_SLOTS; i++) {
    if (ksu_slots[i].user_ref == NULL
        && ksu_slots[i].state == SLI_KSU_SLOT_STATUS_AVAILABLE) {
      key_desc->storage.location.ksu.keyslot = i;
      return SL_STATUS_OK;
    }
  }
  // No empty slot found
  return SL_STATUS_FULL;
}

sl_status_t sli_ksu_allocate_key_slot(sl_se_key_descriptor_t *key_desc,
                                      void *user_ref,
                                      uint8_t crypto_engine_id)
{
  if (key_desc == NULL || user_ref == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  sl_status_t sl_status = SL_STATUS_OK;

  // Acquire the KSU lock (mutex) to protect KSU slot operations
  sl_status = sli_ksu_lock_acquire();
  if (sl_status != SL_STATUS_OK) {
    return sl_status;
  }

  // Find available slot in KSU and set up descriptor
  sl_status = sli_ksu_find_free_user_slot(key_desc, user_ref);
  if (sl_status != SL_STATUS_OK) {
    sli_ksu_lock_release();
    return sl_status;
  }

  // Set the crypto engine ID
  key_desc->storage.location.ksu.crypto_engine_id = crypto_engine_id;

  // Mark the slot as in use and associate with user reference
  uint8_t slot_id = key_desc->storage.location.ksu.keyslot;
  ksu_slots[slot_id].user_ref = user_ref;
  ksu_slots[slot_id].state = SLI_KSU_SLOT_STATUS_IN_USE;
  ksu_slots[slot_id].crypto_engine_id = crypto_engine_id;

  // Update available RAM

  // Release the KSU manager lock (mutex)
  sli_ksu_lock_release();
  return SL_STATUS_OK;
}

sl_status_t sli_ksu_key_slot_import(sl_se_key_descriptor_t *key_desc,
                                    const uint8_t *key_data,
                                    size_t key_data_length,
                                    void *user_ref)
{
  if (key_desc == NULL || key_data == NULL || key_data_length == 0
      || user_ref == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  // Acquire the KSU manager lock (mutex) to protect KSU slot operations
  sl_status_t sl_status = sli_ksu_lock_acquire();
  if (sl_status != SL_STATUS_OK) {
    return sl_status;
  }

  // Create a key desc representing the plaintext input key
  sl_se_key_descriptor_t plaintext_key_desc = *key_desc;

  plaintext_key_desc.storage.method = SL_SE_KEY_STORAGE_EXTERNAL_PLAINTEXT;
  plaintext_key_desc.storage.location.buffer.pointer = (uint8_t *)key_data;
  plaintext_key_desc.storage.location.buffer.size = (key_data_length + 3) & ~3;

  // Initialize to an invalid value.
  uint8_t key_slot_id = ((uint8_t)SLI_KSU_MAX_KEY_SLOTS);
  sl_se_command_context_t cmd_ctx = SL_SE_COMMAND_CONTEXT_INIT;

  // Find available slot in KSU
  sl_status = sli_ksu_find_free_user_slot(key_desc, user_ref);
  if (sl_status != SL_STATUS_OK) {
    goto exit;
  }

  sl_status = sl_se_init_command_context(&cmd_ctx);
  if (sl_status != SL_STATUS_OK) {
    goto exit;
  }

  // Call SE manager to import the key
  sl_status = sl_se_import_key(&cmd_ctx,
                               &plaintext_key_desc,
                               key_desc);
  if (sl_status != SL_STATUS_OK) {
    goto exit;
  }

  key_slot_id = key_desc->storage.location.ksu.keyslot;

  // Update the KSU slot with user reference and key descriptor
  ksu_slots[key_slot_id].user_ref = user_ref;
  ksu_slots[key_slot_id].state = SLI_KSU_SLOT_STATUS_IN_USE;
  ksu_slots[key_slot_id].crypto_engine_id = key_desc->storage.location.ksu.crypto_engine_id;

  sl_status = sl_se_deinit_command_context(&cmd_ctx);

  exit:
  if (sl_status != SL_STATUS_OK) {
    // Cleanup on failure: if key was imported but something else failed,
    // we need to clean up both the slot and the hardware key
    if (key_desc->storage.location.ksu.keyslot != 0) {
      uint8_t failed_slot_id = key_desc->storage.location.ksu.keyslot;

      // Reset the slot if it was allocated
      if (failed_slot_id < SLI_KSU_MAX_KEY_SLOTS
          && ksu_slots[failed_slot_id].state == SLI_KSU_SLOT_STATUS_IN_USE) {
        ksu_slots[failed_slot_id].user_ref = NULL;
        ksu_slots[failed_slot_id].state = SLI_KSU_SLOT_STATUS_AVAILABLE;
        ksu_slots[failed_slot_id].crypto_engine_id = 0;
      }

      // Clear the keyslot in the descriptor
      key_desc->storage.location.ksu.keyslot = 0;
    }
  }

  // Release the KSU manager lock (mutex)
  sli_ksu_lock_release();
  return sl_status;
}

sl_status_t sli_ksu_key_slot_generate(sl_se_key_descriptor_t *key_desc,
                                      void *user_ref)
{
  if (key_desc == NULL || user_ref == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  sl_status_t sl_status = SL_STATUS_OK;

  // Acquire the KSU manager lock (mutex) to protect KSU slot operations
  sl_status = sli_ksu_lock_acquire();
  if (sl_status != SL_STATUS_OK) {
    return sl_status;
  }

  sl_se_command_context_t cmd_ctx = SL_SE_COMMAND_CONTEXT_INIT;

  // Find available slot in KSU
  sl_status = sli_ksu_find_free_user_slot(key_desc, user_ref);
  if (sl_status != SL_STATUS_OK) {
    goto exit;
  }

  sl_status = sl_se_init_command_context(&cmd_ctx);
  if (sl_status != SL_STATUS_OK) {
    goto exit;
  }

  // Call SE manager to generate the key directly in KSU
  sl_status = sl_se_generate_key(&cmd_ctx, key_desc);
  if (sl_status != SL_STATUS_OK) {
    goto exit;
  }

  uint8_t key_slot_id = key_desc->storage.location.ksu.keyslot;

  // Update the KSU slot with user reference and key descriptor
  ksu_slots[key_slot_id].user_ref = user_ref;
  ksu_slots[key_slot_id].state = SLI_KSU_SLOT_STATUS_IN_USE;
  ksu_slots[key_slot_id].crypto_engine_id = key_desc->storage.location.ksu.crypto_engine_id;

  sl_status = sl_se_deinit_command_context(&cmd_ctx);

  exit:
  if (sl_status != SL_STATUS_OK) {
    // Cleanup on failure: if key was generated but something else failed,
    // we need to clean up both the slot and the hardware key
    if (key_desc->storage.location.ksu.keyslot != 0) {
      uint8_t failed_slot_id = key_desc->storage.location.ksu.keyslot;

      // Reset the slot if it was allocated
      if (failed_slot_id < SLI_KSU_MAX_KEY_SLOTS
          && ksu_slots[failed_slot_id].state == SLI_KSU_SLOT_STATUS_IN_USE) {
        ksu_slots[failed_slot_id].user_ref = NULL;
        ksu_slots[failed_slot_id].state = SLI_KSU_SLOT_STATUS_AVAILABLE;
        ksu_slots[failed_slot_id].crypto_engine_id = 0;
      }

      // Clear the keyslot in the descriptor
      key_desc->storage.location.ksu.keyslot = 0;
    }
  }

  // Release the KSU manager lock (mutex)
  sli_ksu_lock_release();
  return sl_status;
}

sl_status_t sli_ksu_delete_key(sl_se_key_descriptor_t *key_desc)
{
  uint8_t key_slot_id = key_desc->storage.location.ksu.keyslot;
  if (key_slot_id >= SLI_KSU_MAX_KEY_SLOTS) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  if (ksu_slots[key_slot_id].state == SLI_KSU_SLOT_STATUS_AVAILABLE) {
    return SL_STATUS_DELETED;
  }

  if (ksu_slots[key_slot_id].state == SLI_KSU_SLOT_STATUS_RESERVED) {
    return SL_STATUS_PERMISSION;
  }

  // Acquire the KSU manager lock (mutex) to protect KSU slot operations
  sl_status_t sl_status = sli_ksu_lock_acquire();
  if (sl_status != SL_STATUS_OK) {
    return sl_status;
  }

  // Mark the KSU slot as free regardless of whether the subsequent call to
  // sl_se_delete_key fails or not, because by now we know the key has been
  // loaded to the KSU and all error conditions returned by sl_se_delete_key
  // should indicate we should assume the key is removed or lost,
  // e.g. the KSU may have been disabled.
  ksu_slots[key_slot_id].user_ref = NULL;
  ksu_slots[key_slot_id].state = SLI_KSU_SLOT_STATUS_AVAILABLE;
  ksu_slots[key_slot_id].crypto_engine_id = 0;

  sl_se_command_context_t cmd_ctx = SL_SE_COMMAND_CONTEXT_INIT;

  sl_status = sl_se_init_command_context(&cmd_ctx);
  if (sl_status != SL_STATUS_OK) {
    goto exit;
  }

  // Call SE manager to destroy the key
  sl_status = sl_se_delete_key(&cmd_ctx, key_desc);
  if (sl_status != SL_STATUS_OK) {
    goto exit;
  }

  sl_status = sl_se_deinit_command_context(&cmd_ctx);
  if (sl_status != SL_STATUS_OK) {
    goto exit;
  }

  exit:
  // Release the KSU manager lock (mutex)
  sli_ksu_lock_release();

  if (sl_status != SL_STATUS_OK) {
    return sl_status;
  }

  return SL_STATUS_OK;
}

sl_status_t sli_ksu_get_key_slot_id_from_user_ref(void *user_ref, uint8_t *key_slot_id)
{
  if (user_ref == NULL || key_slot_id == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  for (size_t i = SLI_KSU_KEY_SLOT_USER_START; i < SLI_KSU_MAX_KEY_SLOTS; i++) {
    if (ksu_slots[i].user_ref == user_ref) {
      *key_slot_id = i;
      return SL_STATUS_OK;
    }
  }
  return SL_STATUS_NOT_FOUND;
}

sl_status_t sli_ksu_get_crypto_engine_id_from_user_ref(void *user_ref, uint8_t *crypto_engine_id)
{
  if (user_ref == NULL || crypto_engine_id == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  for (size_t i = SLI_KSU_KEY_SLOT_USER_START; i < SLI_KSU_MAX_KEY_SLOTS; i++) {
    if (ksu_slots[i].user_ref == user_ref) {
      *crypto_engine_id = ksu_slots[i].crypto_engine_id;
      return SL_STATUS_OK;
    }
  }
  return SL_STATUS_NOT_FOUND;
}

sl_status_t sli_ksu_key_slot_copy(sl_se_key_descriptor_t *source_key_desc,
                                  sl_se_key_descriptor_t *target_key_desc,
                                  void *user_ref)
{
  if (source_key_desc == NULL || target_key_desc == NULL || user_ref == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  sl_status_t sl_status = SL_STATUS_OK;

  // Acquire the KSU manager lock (mutex) to protect KSU slot operations
  sl_status = sli_ksu_lock_acquire();
  if (sl_status != SL_STATUS_OK) {
    return sl_status;
  }

  // Check if space is available in the KSU (must be inside mutex for thread safety)

  // Find available slot in KSU
  sl_status = sli_ksu_find_free_user_slot(target_key_desc, user_ref);
  if (sl_status != SL_STATUS_OK) {
    goto exit;
  }

  sl_se_command_context_t cmd_ctx = SL_SE_COMMAND_CONTEXT_INIT;

  sl_status = sl_se_init_command_context(&cmd_ctx);
  if (sl_status != SL_STATUS_OK) {
    goto exit;
  }

  // Call SE manager to transfer the key
  sl_status = sl_se_transfer_key(&cmd_ctx, source_key_desc, target_key_desc);
  if (sl_status != SL_STATUS_OK) {
    goto exit;
  }

  uint8_t key_slot_id = target_key_desc->storage.location.ksu.keyslot;

  // Update the KSU slot with user reference and key descriptor
  ksu_slots[key_slot_id].user_ref = user_ref;
  ksu_slots[key_slot_id].state = SLI_KSU_SLOT_STATUS_IN_USE;
  ksu_slots[key_slot_id].crypto_engine_id = target_key_desc->storage.location.ksu.crypto_engine_id;

  sl_status = sl_se_deinit_command_context(&cmd_ctx);

  exit:
  if (sl_status != SL_STATUS_OK) {
    // Cleanup on failure: if key was transferred but something else failed,
    // we need to clean up both the slot and the hardware key
    if (target_key_desc->storage.location.ksu.keyslot != 0) {
      uint8_t failed_slot_id = target_key_desc->storage.location.ksu.keyslot;

      // Reset the slot if it was allocated
      if (failed_slot_id < SLI_KSU_MAX_KEY_SLOTS
          && ksu_slots[failed_slot_id].state == SLI_KSU_SLOT_STATUS_IN_USE) {
        ksu_slots[failed_slot_id].user_ref = NULL;
        ksu_slots[failed_slot_id].state = SLI_KSU_SLOT_STATUS_AVAILABLE;
        ksu_slots[failed_slot_id].crypto_engine_id = 0;
      }

      // Clear the keyslot in the descriptor
      target_key_desc->storage.location.ksu.keyslot = 0;
    }
  }

  // Release the KSU manager lock (mutex)
  sli_ksu_lock_release();
  return sl_status;
}