/***************************************************************************//**
 * @file
 * @brief AES decryption functionality for Silicon Labs bootloader
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

#include "btl_security_aes.h"
#include "btl_security_types.h"

#include <string.h> // For memory copy functions

#include <stdio.h>

// Initialize AES-CTR algorithm in CCM mode as used by GBL
void btl_initAesCcm(void          *ctx,
                    uint8_t       flags,
                    uint8_t       *nonce,
                    uint32_t      counter,
                    const uint8_t *key,
                    unsigned int  keySize)
{
#if defined(_CMU_CLKEN1_SEMAILBOXHOST_MASK)
  CMU->CLKEN1_SET = CMU_CLKEN1_SEMAILBOXHOST;
#endif

  AesCtrContext_t *context = (AesCtrContext_t *)ctx;

  (void)key;
  context->aesKeyDesc.type = SL_SE_KEY_TYPE_SYMMETRIC;
  context->aesKeyDesc.size = keySize / 8UL; // keySize in bytes
  context->aesKeyDesc.flags = SL_SE_KEY_FLAG_NON_EXPORTABLE; // Non exportable key
  context->aesKeyDesc.storage.method = SL_SE_KEY_STORAGE_INTERNAL_IMMUTABLE;
  context->aesKeyDesc.storage.location.slot = SL_SE_KEY_SLOT_APPLICATION_AES_128_KEY;
  // Indicate start of stream by setting offset to 0
  context->offsetInBlock = 0;

  // CCM uses counter mode with the following concatenated:
  //  * flags   (1  byte)
  //  * nonce   (12 bytes)
  //  * counter (3  bytes)
  context->counter[0] = flags;
  memcpy(&(context->counter[1]), nonce, 12);
  context->counter[13] = (counter & 0x00FF0000) >> 16;
  context->counter[14] = (counter & 0x0000FF00) >> 8;
  context->counter[15] = (counter & 0x000000FF);
}

// Process AES-CTR data.
void btl_processAesCtrData(void          *ctx,
                           const uint8_t *input,
                           uint8_t       *output,
                           size_t        length)
{
  AesCtrContext_t *context = (AesCtrContext_t *)ctx;
  sl_se_command_context_t cmd_ctx;
  sl_se_init_command_context(&cmd_ctx);
  sl_se_aes_crypt_ctr(&cmd_ctx,
                      &(context->aesKeyDesc),
                      length,
                      (uint32_t *)&(context->offsetInBlock),
                      context->counter,
                      context->streamBlock,
                      input,
                      output);
}
