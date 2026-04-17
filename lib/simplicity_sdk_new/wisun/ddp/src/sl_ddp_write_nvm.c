/***************************************************************************//**
 * @file sl_ddp_write_nvm.c
 * @brief DDP command for writing an NVM object
 *******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
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
#include "sl_ddp_write_nvm.h"

/**************************************************************************//**
 * Write data to NVM.
 *****************************************************************************/
sl_ddp_status_t sl_ddp_write_nvm(const uint8_t *input,
                                 uint16_t input_len,
                                 uint8_t *output,
                                 uint16_t output_size,
                                 uint16_t *output_len)
{
  (void)output;
  (void)output_len;
  (void)output_size;

  if (input == NULL || input_len < sizeof(sl_ddp_write_nvm_req_t)) {
    return SL_DDP_STATUS_IN_ARGS_NOT_VALID_ERR;
  }

  Ecode_t ret;
  const sl_ddp_write_nvm_req_t *req = (const sl_ddp_write_nvm_req_t *)input;

  ret = nvm3_writeData(nvm3_defaultHandle, req->key, req->data, req->data_len);
  if (ret != ECODE_NVM3_OK) {
    return SL_DDP_STATUS_NVM3_WRITE_ERR;
  }

  return SL_DDP_STATUS_SUCCESS;
}
