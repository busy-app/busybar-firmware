/***************************************************************************//**
 * @file sl_ddp_write_nvm.h
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

#ifndef SL_DDP_WRITE_NVM_H
#define SL_DDP_WRITE_NVM_H

#include <stdint.h>
#include "sl_common.h"
#include "sl_ddp_types.h"

/**************************************************************************//**
 * @addtogroup SL_DDP_COMMAND_API DDP Command API
 *
 * @{
 *****************************************************************************/

/// Input structure of DDP command for writing an NVM object
SL_PACK_START(1)
typedef struct {
  /// NVM Object Key
  uint32_t key;
  /// Amount of data to write in bytes
  uint16_t data_len;
  /// Data to write
  uint8_t data[];
} SL_ATTRIBUTE_PACKED sl_ddp_write_nvm_req_t;
SL_PACK_END()

/***************************************************************************//**
 * Write data to NVM.
 *
 * @param[in] input Command specific input structure
 * @param[in] input_len Length of command specific input structure in bytes
 * @param[out] output Command specific output structure
 * @param[in] output_size Maximum size allocated to command specific output
 *                        structure by the caller
 * @param[out] output_len Length of command specific output structure in
 *                        bytes
 * @return SL_DDP_STATUS_SUCCESS on success, an error code otherwise
 ******************************************************************************/
sl_ddp_status_t sl_ddp_write_nvm(const uint8_t *input,
                                 uint16_t input_len,
                                 uint8_t *output,
                                 uint16_t output_size,
                                 uint16_t *output_len);

/** @} (end SL_DDP_COMMAND_API) */

#endif  // SL_DDP_WRITE_NVM3_H
