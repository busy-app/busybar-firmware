/***************************************************************************//**
 * @file
 * @brief ECDSA signing functionality for Silicon Labs bootloader
 *******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc.  Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement.  This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/
#include "btl_security_ecdsa.h"
#include "btl_security_types.h"
#include "em_device.h"

#include <stddef.h>
#include <string.h> // For memset

#include "sli_se_manager_mailbox.h"

/** Verify the ECDSA signature of the SHA hash, using
 *  the public key in the relevant token, with the signature contained in
 *  the byte arrays pointed to.
 */
int32_t btl_verifyEcdsaP256r1(const uint8_t *sha256,
                              const uint8_t *signatureR,
                              const uint8_t *signatureS,
                              const uint8_t *keyX,
                              const uint8_t *keyY)
{
  if ((sha256 == NULL) || (signatureR == NULL) || (signatureS == NULL)) {
    return BOOTLOADER_ERROR_SECURITY_INVALID_PARAM;
  }

  int32_t retVal;
  uint8_t pub_key[64] = { 0 };
  uint8_t signature[64] = { 0 };

  sl_se_key_descriptor_t key = { 0 };
  if ((keyX == NULL) || (keyY == NULL)) {
    key.type = SL_SE_KEY_TYPE_ECC_P256;
    key.flags = SL_SE_KEY_FLAG_ASYMMETRIC_BUFFER_HAS_PUBLIC_KEY
                | SL_SE_KEY_FLAG_ASYMMETRIC_SIGNING_ONLY;
    key.storage.method = SL_SE_KEY_STORAGE_INTERNAL_IMMUTABLE;
    key.storage.location.slot = SL_SE_KEY_SLOT_APPLICATION_SECURE_BOOT_KEY;
  } else {
    key.type = SL_SE_KEY_TYPE_ECC_P256;
    key.flags = SL_SE_KEY_FLAG_ASYMMETRIC_BUFFER_HAS_PUBLIC_KEY
                | SL_SE_KEY_FLAG_ASYMMETRIC_SIGNING_ONLY;
    key.storage.method = SL_SE_KEY_STORAGE_EXTERNAL_PLAINTEXT;
    key.storage.location.buffer.pointer = pub_key;
    key.storage.location.buffer.size = sizeof(pub_key);
  };

  if (keyX != NULL) {
    memcpy(pub_key, keyX, 32);
  }

  if (keyY != NULL) {
    memcpy(&pub_key[32], keyY, 32);
  }

  memcpy(signature, signatureR, 32);
  memcpy(&signature[32], signatureS, 32);

  sl_se_command_context_t cmd_ctx = { 0 };
  sl_se_init_command_context(&cmd_ctx);

  retVal = sl_se_ecc_verify(&cmd_ctx,
                            &key,
                            SL_SE_HASH_NONE,
                            true,
                            sha256,
                            32,
                            signature,
                            64);
  sl_se_deinit_command_context(&cmd_ctx);

  if (retVal == SL_STATUS_OK) {
    return BOOTLOADER_OK;
  }

  return BOOTLOADER_ERROR_SECURITY_REJECTED;
}
