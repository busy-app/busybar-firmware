/***************************************************************************//**
 * @file sl_ddp_common.h
 * @brief Common framework for DDP
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

#ifndef SL_DDP_COMMON_H
#define SL_DDP_COMMON_H

#include <stdint.h>
#include "sl_ddp_types.h"

/**************************************************************************//**
 * @addtogroup SL_DDP_COMMON_API DDP Common API
 *
 * @{
 *****************************************************************************/

/// Helper macro for jumping to the error handler label if the condition
/// is untrue.
#define SL_DDP_ERROR_CHECK(__condition) \
  do {                                  \
    if (!(__condition)) {               \
      goto error_handler;               \
    }                                   \
  } while (0)

/// Helper macro for setting the status variable and jumping to
/// the handler label if the condition is untrue.
#define SL_DDP_ERROR_CHECK_SET_STATUS(__condition, __value) \
  do {                                                      \
    if (!(__condition)) {                                   \
      status = __value;                                     \
      goto error_handler;                                   \
    }                                                       \
  } while (0)

/// Helper macro setting the status variable if the condition
/// is untrue.
#define SL_DDP_ERROR_SET_STATUS(__condition) \
  do {                                       \
    status = __condition;                    \
    goto error_handler;                      \
  } while (0)

/***************************************************************************//**
 * Handle a DDP command.
 *
 * @param[in] id DDP command ID
 * @param[in] input Command specific input structure
 * @param[in] input_len Length of command specific input structure in bytes
 * @param[out] output Command specific output structure
 * @param[in] output_size Maximum size allocated to command specific output
 *                        structure by the caller
 * @param[out] output_len Length of command specific output structure in
 *                        bytes
 * @return SL_DDP_STATUS_SUCCESS on success, an error code otherwise
 ******************************************************************************/
sl_ddp_status_t sl_ddp_handle_command(uint32_t id,
                                      const uint8_t *input,
                                      uint16_t input_len,
                                      uint8_t *output,
                                      uint16_t output_size,
                                      uint16_t *output_len);

/** @} (end SL_DDP_COMMON_API) */

#endif  // SL_DDP_COMMON_H
