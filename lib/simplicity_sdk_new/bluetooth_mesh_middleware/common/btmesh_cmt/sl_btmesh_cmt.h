/***************************************************************************//**
 * @file
 * @brief BT Mesh Common Manufacturing Token
 *******************************************************************************
 * # License
 * <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifndef SL_BTMESH_CMT_H
#define SL_BTMESH_CMT_H

#include "sl_bt_api.h"
#include "sl_status.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/***************************************************************************//**
 * @addtogroup btmesh_cmt
 * @{
 ******************************************************************************/

/***************************************************************************//**
 * Initialize the BT Mesh Common Manufacturing Token component.
 *
 * This function is called automatically after enabling the component.
 ******************************************************************************/
void sl_btmesh_cmt_on_init(void);

/***************************************************************************//**
 * Get the Mesh Manufacturing Token data
 *
 * @return Pointer to the Mesh Manufacturing Token data
 ******************************************************************************/
uint8_t *sl_btmesh_cmt_get_token_data(void);

/***************************************************************************//**
 * Get UUID from Common Manufacturing Token
 *
 * @return UUID found in the token, or all zeroes
 ******************************************************************************/
uuid_128 sl_btmesh_cmt_get_uuid(void);

/***************************************************************************//**
 * Get OOB Data from Common Manufacturing Token
 *
 * @param[out] output_size    Pointer to uint8_t. Maximum Output OOB size (0 = not supported, 8 = max).
 * @param[out] output_actions Pointer to uint16_t. Enum sl_btmesh_node_oob_output_action_flag_t. Allowed OOB Output Action types.
 * @param[out] input_size     Pointer to uint8_t. Maximum Input OOB size (0 = not supported, 8 = max).
 * @param[out] input_actions  Pointer to uint16_t. Enum sl_btmesh_node_oob_input_action_flag_t. Allowed OOB Input Action types.
 *
 * @return SL_STATUS_OK if OOB data was found and parsed successfully, otherwise SL_STATUS_FAIL
 ******************************************************************************/
sl_status_t sl_btmesh_cmt_get_oob_data(uint8_t  *output_size,
                                       uint16_t *output_actions,
                                       uint8_t  *input_size,
                                       uint16_t *input_actions);

/** @} end btmesh_cmt */

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // SL_BTMESH_CMT_H
