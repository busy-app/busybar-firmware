/***************************************************************************//**
 * @file
 * @brief Common framework for DDP
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

#ifndef SL_DDP_COMMON_H
#define SL_DDP_COMMON_H

#include <stdint.h>
#include "sl_ddp_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * @addtogroup SL_DDP_COMMON_API DDP Common API
 *
 * @{
 *****************************************************************************/

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
 * @return 0 on success, an error code otherwise
 ******************************************************************************/
int sl_ddp_handle_command(uint16_t id,
                          const uint8_t *input,
                          uint16_t input_len,
                          uint8_t *output,
                          uint16_t output_size,
                          uint16_t *output_len);

/** @} (end SL_DDP_COMMON_API) */

#ifdef __cplusplus
}
#endif

#endif  // SL_DDP_COMMON_H
