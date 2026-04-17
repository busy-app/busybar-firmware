/***************************************************************************//**
 * @file sl_ddp_types.h
 * @brief Common type declarations for DDP
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

#ifndef SL_DDP_TYPES_H
#define SL_DDP_TYPES_H

#include <stdint.h>

/**************************************************************************//**
 * @addtogroup SL_DDP_COMMON_API DDP Common API
 *
 * @{
 *****************************************************************************/

/// Possible DDP command handler return values
typedef enum {
  SL_DDP_STATUS_SUCCESS = 0,
  SL_DDP_STATUS_NVM3_OPEN_ERR,
  SL_DDP_STATUS_NVM3_WRITE_ERR,
  SL_DDP_STATUS_NVM3_REPACK_ERR,
  SL_DDP_STATUS_NVM3_CLOSE_ERR,
  SL_DDP_STATUS_PSA_CRYPTO_INIT_ERR,
  SL_DDP_STATUS_PSA_IMPORT_KEY_ERR,
  SL_DDP_STATUS_PSA_GENERATE_KEY_ERR,
  SL_DDP_STATUS_PSA_GENERATED_KEY_ID_ERR,
  SL_DDP_STATUS_PSA_EXPORT_KEY_ERR,
  SL_DDP_STATUS_PSA_SIGN_MESSAGE_ERR,
  SL_DDP_STATUS_PSA_RAND_NUM_GEN_ERR,
  SL_DDP_STATUS_PKT_LEN_TOO_SMALL_ERR,
  SL_DDP_STATUS_REQ_LEN_TOO_SMALL_ERR,
  SL_DDP_STATUS_IN_ARGS_NOT_VALID_ERR,
  SL_DDP_STATUS_OUT_ARGS_NOT_VALID_ERR,
  SL_DDP_STATUS_DATA_NOT_EXISTS_ERR,
  SL_DDP_STATUS_CMD_UNKNOWN_ERR,
  SL_DDP_STATUS_UNKNOWN,
  SL_DDP_STATUS_PSA_ALREADY_EXISTS_ERR
} sl_ddp_status_t;

/**************************************************************************//**
 * DDP command handler prototype.
 *
 * @param[in] input Command specific input structure
 * @param[in] input_len Length of command specific input structure in bytes
 * @param[out] output Command specific output structure
 * @param[in] output_size Maximum size allocated to command specific output
 *                        structure by the caller
 * @param[out] output_len Length of command specific output structure in
 *                        bytes
 * @return SL_DDP_STATUS_SUCCESS on success, an error code otherwise
 *
 * All DDP commands must expose a function implementing this prototype
 * in their component template contribution.
 *****************************************************************************/
typedef sl_ddp_status_t (*sl_ddp_command_func_t)(const uint8_t *input,
                                                 uint16_t input_len,
                                                 uint8_t *output,
                                                 uint16_t output_size,
                                                 uint16_t *output_len);

/// DDP command table entry
typedef struct {
  /// DDP command ID
  uint32_t id;
  /// Pointer to DDP command handler
  sl_ddp_command_func_t handler;
} sl_ddp_command_entry_t;

/** @} (end SL_DDP_COMMON_API) */

#endif  // SL_DDP_TYPES_H
