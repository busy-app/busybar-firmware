/***************************************************************************/ /**
 * @file
 * @brief SL SI91x ATTESTATION API
 *******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
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

#pragma once
#include "sl_si91x_protocol_types.h"
#include "sl_si91x_crypto.h"
#include "sl_status.h"

/******************************************************
 *                Function Declarations
 *******************************************************/

/**
 * @addtogroup CRYPTO_ATTESTATION_FUNCTIONS
 * @{
 */

/***************************************************************************/
/**
 * @fn sl_status_t sl_si91x_attestation_get_token(uint8_t *token, uint16_t length, uint32_t *nonce)
 * @brief To process the nonce for the attestation token.
 * @param[out] token
 *   Buffer to store the attestation token.
 * @param[in] length
 *   Length of the token buffer.
 * @param[in] nonce
 *   Nonce value used for generating the attestation token.
 * @return
 *   sl_status_t.
 * For more information on status codes, see 
 * [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status) for further details.
 ******************************************************************************/
sl_status_t sl_si91x_attestation_get_token(uint8_t *token, uint16_t length, uint32_t *nonce);

/** @} */
