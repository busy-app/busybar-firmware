/***************************************************************************//**
 * @file sl_wisun_ddp_generate_csr.h
 * @brief DDP command for generating a Wi-SUN CSR
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

#ifndef SL_WISUN_DDP_GENERATE_CSR_H
#define SL_WISUN_DDP_GENERATE_CSR_H

#include <stdint.h>
#include "sl_common.h"
#include "sl_ddp_types.h"

/**************************************************************************//**
 * @addtogroup SL_DDP_COMMAND_API DDP Command API
 *
 * @{
 *****************************************************************************/

/// Input structure of DDP command for generating a Wi-SUN CSR
SL_PACK_START(1)
typedef struct {
  /// PSA Key ID
  uint32_t key_id;
} SL_ATTRIBUTE_PACKED sl_wisun_ddp_generate_csr_req_t;
SL_PACK_END()

/// Output structure of DDP command for generating a Wi-SUN CSR
SL_PACK_START(1)
typedef struct {
  /// Length of Certificate Signing Request in bytes
  uint32_t csr_len;
  /// Certificate Signing Request
  uint8_t csr[];
} SL_ATTRIBUTE_PACKED sl_wisun_ddp_generate_csr_rsp_t;
SL_PACK_END()

/***************************************************************************//**
 * Generate a Wi-SUN CSR.
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
sl_ddp_status_t sl_wisun_ddp_generate_csr(const uint8_t *input,
                                          uint16_t input_len,
                                          uint8_t *output,
                                          uint16_t output_size,
                                          uint16_t *output_len);

/** @} (end SL_DDP_COMMAND_API) */

#endif  // SL_WISUN_DDP_GENERATE_CSR_H
