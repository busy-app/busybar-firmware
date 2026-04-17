/***************************************************************************//**
 * @file sl_ddp_initialize_nvm.c
 * @brief DDP command for initializing NVM
 *******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

#include <string.h>
#include "nvm3_default.h"
#include "sl_ddp_initialize_nvm.h"

/**************************************************************************//**
 * Write data to NVM.
 *****************************************************************************/
sl_ddp_status_t sl_ddp_initialize_nvm(const uint8_t *input,
                                      uint16_t input_len,
                                      uint8_t *output,
                                      uint16_t output_size,
                                      uint16_t *output_len)
{
  (void)output;
  (void)output_len;
  (void)output_size;

  if (input == NULL || input_len < sizeof(sl_ddp_initialize_nvm_req_t)) {
    return SL_DDP_STATUS_IN_ARGS_NOT_VALID_ERR;
  }

  Ecode_t ret;
  const sl_ddp_initialize_nvm_req_t *req = (const sl_ddp_initialize_nvm_req_t *)input;

  // Update NVM3 information to the default
  nvm3_defaultInit->nvmAdr = (nvm3_HalPtr_t)req->base_addr;
  nvm3_defaultInit->nvmSize = (size_t)req->size;

  // Refresh NVM3 default handle
  nvm3_deinitDefault();
  ret = nvm3_initDefault();
  if (ret != ECODE_NVM3_OK) {
    return SL_DDP_STATUS_NVM3_OPEN_ERR;
  }

  return SL_DDP_STATUS_SUCCESS;
}
