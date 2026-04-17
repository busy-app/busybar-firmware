/***************************************************************************//**
 * @file
 * @brief DDP commands for NVM
 *******************************************************************************
 * # License
 * <b>Copyright 2026 Silicon Laboratories Inc. www.silabs.com</b>
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
#include "sl_status.h"
#include "nvm3.h"
#include "sl_ddp_types.h"

// -----------------------------------------------------------------------------
// Definitions
#define NVM3_HANDLE   (nvm3_defaultHandle)

// Input structure of DDP NVM set command
SL_PACK_START(1)
typedef struct {
  uint32_t object_key; // NVM object key as nvm3_ObjectKey_t
  uint16_t data_len; // Amount of data to write in bytes
  uint8_t data[]; // Data to save
} SL_ATTRIBUTE_PACKED sl_ddp_cmd_nvm_set_t;
SL_PACK_END()

// Input structure of DDP NVM get command
SL_PACK_START(1)
typedef struct {
  uint32_t object_key; // NVM object key as nvm3_ObjectKey_t
} SL_ATTRIBUTE_PACKED sl_ddp_cmd_nvm_get_t;
SL_PACK_END()

// -----------------------------------------------------------------------------
// Public functions

/******************************************************************************
 * DDP Command for NVM set.
 *****************************************************************************/
int sl_ddp_cmd_nvm_set(const uint8_t *input,
                       uint16_t input_len,
                       uint8_t *output,
                       uint16_t output_size,
                       uint16_t *output_len)
{
  (void)output;
  (void)output_size;
  (void)output_len;

  if ((input == NULL) || (input_len < sizeof(sl_ddp_cmd_nvm_set_t))) {
    return SL_DDP_ERROR_ARG;
  }

  const sl_ddp_cmd_nvm_set_t *req = (const sl_ddp_cmd_nvm_set_t *)input;
  sl_status_t sc;
  sc = nvm3_writeData(NVM3_HANDLE,
                      (nvm3_ObjectKey_t)(req->object_key),
                      req->data,
                      (size_t)req->data_len);
  if (sc != SL_STATUS_OK) {
    return (int)sc;
  }

  // Do repacking if needed
  if (nvm3_repackNeeded(NVM3_HANDLE)) {
    sc = nvm3_repack(NVM3_HANDLE);
  }

  return (int)sc;
}

/******************************************************************************
 * DDP Command for NVM get.
 *****************************************************************************/
int sl_ddp_cmd_nvm_get(const uint8_t *input,
                       uint16_t input_len,
                       uint8_t *output,
                       uint16_t output_size,
                       uint16_t *output_len)
{
  if ((input == NULL)
      || (input_len < sizeof(sl_ddp_cmd_nvm_get_t))
      || (output_len == NULL)) {
    return SL_DDP_ERROR_ARG;
  }

  const sl_ddp_cmd_nvm_get_t *req = (const sl_ddp_cmd_nvm_get_t *)input;
  sl_status_t sc;

  // Get NVM object size
  uint32_t type;
  size_t len;
  sc = nvm3_getObjectInfo(NVM3_HANDLE,
                          (nvm3_ObjectKey_t)(req->object_key),
                          &type,
                          &len);
  (void)type;
  if (sc != SL_STATUS_OK) {
    return (int)sc;
  }

  if (len > output_size) {
    return SL_DDP_ERROR_LENGTH;
  }

  // Get NVM object
  *output_len = len;
  sc = nvm3_readData(NVM3_HANDLE,
                     (nvm3_ObjectKey_t)(req->object_key),
                     output,
                     (size_t)*output_len);
  return (int)sc;
}
