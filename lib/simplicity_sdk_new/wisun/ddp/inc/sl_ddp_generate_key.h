/***************************************************************************//**
 * @file sl_ddp_generate_key.h
 * @brief DDP command for generating a PSA Crypto key
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

#ifndef SL_DDP_GENERATE_KEY_H
#define SL_DDP_GENERATE_KEY_H

#include <stdint.h>
#include "sl_common.h"
#include "sl_ddp_types.h"

/**************************************************************************//**
 * @addtogroup SL_DDP_COMMAND_API DDP Command API
 *
 * @{
 *****************************************************************************/

/// Input structure of DDP command for generating a PSA Crypto key
SL_PACK_START(1)
typedef struct {
  /// Lifetime of the key as psa_key_type_t
  uint32_t lifetime;
  /// Location of the key as psa_key_location_t
  uint32_t location;
  /// Permitted usage of the key as psa_key_usage_t
  uint32_t usage_flags;
  /// Length of key in bits
  uint32_t bits;
  /// Permitted algorithms of the key as psa_algorithm_t
  uint32_t algo;
  /// Type of the key as psa_key_type_t
  uint16_t type;
  /// PSA Key ID
  uint32_t key_id;
} SL_ATTRIBUTE_PACKED sl_ddp_generate_key_req_t;
SL_PACK_END()

/// Output structure of DDP command for generating a PSA Crypto key
SL_PACK_START(1)
typedef struct {
  /// Length of the generated public key in bytes. This field may be
  /// zero if no corresponding public key has been generated.
  uint32_t key_len;
  /// Generated public key
  uint8_t key[];
} SL_ATTRIBUTE_PACKED sl_ddp_generate_key_rsp_t;
SL_PACK_END()

/***************************************************************************//**
 * Generate a PSA Crypto key.
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
sl_ddp_status_t sl_ddp_generate_key(const uint8_t *input,
                                    uint16_t input_len,
                                    uint8_t *output,
                                    uint16_t output_size,
                                    uint16_t *output_len);

/** @} (end SL_DDP_COMMAND_API) */

#endif  // SL_DDP_GENERATE_KEY_H
