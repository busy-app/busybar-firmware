/***************************************************************************//**
 * @file
 * @brief Common type declarations for DDP
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

#ifndef SL_DDP_TYPES_H
#define SL_DDP_TYPES_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * @addtogroup SL_DDP_COMMON_API DDP Common API
 *
 * @{
 *****************************************************************************/

// -----------------------------------------------------------------------------
// Definitions
#define SL_DDP_ERROR_COMMAND          ((int)1)
#define SL_DDP_ERROR_ARG              ((int)2)
#define SL_DDP_ERROR_LENGTH           ((int)3)
#define SL_DDP_ERROR_NOT_INITIALIZED  ((int)4)

/******************************************************************************
 * DDP command handler prototype.
 *
 * @param[in] input Command specific input structure
 * @param[in] input_len Length of command specific input structure in bytes
 * @param[out] output Command specific output structure
 * @param[in] output_size Maximum size allocated to command specific output
 *                        structure by the caller
 * @param[out] output_len Length of command specific output structure in
 *                        bytes
 * @return 0 on success, an error code otherwise
 *
 * All DDP commands must expose a function implementing this prototype
 * in their component template contribution.
 *****************************************************************************/
typedef int (*sl_ddp_command_func_t)(const uint8_t *input,
                                     uint16_t input_len,
                                     uint8_t *output,
                                     uint16_t output_size,
                                     uint16_t *output_len);

// DDP command table entry
typedef struct {
  uint16_t id; // DDP command ID
  sl_ddp_command_func_t handler; // Pointer to DDP command handler
} sl_ddp_command_entry_t;

/** @} (end SL_DDP_COMMON_API) */

#ifdef __cplusplus
}
#endif

#endif  // SL_DDP_TYPES_H
