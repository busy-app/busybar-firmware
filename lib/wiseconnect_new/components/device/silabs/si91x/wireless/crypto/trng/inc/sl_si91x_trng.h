/***************************************************************************/ /**
 * @file
 * @brief SL SI91X TRNG API
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

#pragma once
#include "sl_si91x_protocol_types.h"
#include "sl_si91x_crypto.h"
#include "sl_status.h"

#ifdef SLI_TRNG_DEVICE_SI91X
#include "sl_si91x_protocol_types.h"
#endif //SLI_TRNG_DEVICE_SI91X

#ifndef SLI_SI91X_TRNG_DUPLICATE_CHECK
/// @brief Enables duplicate check for TRNG output.
#define SLI_SI91X_TRNG_DUPLICATE_CHECK 1
#endif // SLI_SI91X_TRNG_DUPLICATE_CHECK

/******************************************************
 *                   Type Definitions
 ******************************************************/
/**
 * @addtogroup CRYPTO_TRNG_TYPES 
 * @{ 
 */

/**
 * @brief Structure defining TRNG configuration
 * 
 * This structure defines TRNG configuration required for the operation which includes
 * input message, length of input message, key, and so on. 
 */
typedef struct {
  uint32_t *trng_test_data; ///< Test input data for TRNG
  uint16_t input_length;    ///< Input length
  uint32_t *trng_key;       ///< TRNG key
} sl_si91x_trng_config_t;

/** @} */

/******************************************************
 *                Function Declarations
*******************************************************/

/**
 * @addtogroup CRYPTO_TRNG_FUNCTIONS
 * @{ 
 */

/***************************************************************************/
/**
 * @brief 
 *   To initialize the TRNG hardware engine.
 * @param[in] config 
 *   Configuration object of type @ref sl_si91x_trng_config_t
 * @param[out] output 
 *   Buffer to store the output.
 * @return
 *   sl_status_t.
 * For more information on status codes, see 
 * [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
******************************************************************************/
sl_status_t sl_si91x_trng_init(const sl_si91x_trng_config_t *config, uint32_t *output);

/***************************************************************************/
/**
 * @brief 
 *   To check the entropy of TRNG and verify its functioning.
 * @return
 *   sl_status_t.
 * For more information on status codes, see 
 * [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
******************************************************************************/
sl_status_t sl_si91x_trng_entropy(void);

/***************************************************************************/
/**
 * @brief 
 *   To initialize and program the key required for the TRNG hardware engine.
 * @param[in] trng_key 
 *   Pointer to the TRNG key.
 * @param[in] key_length 
 *   Length of the key in Dwords (uint32_t).
 * @return
 *   sl_status_t.
 * For more information on status codes, see 
 * [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
******************************************************************************/
sl_status_t sl_si91x_trng_program_key(uint32_t *trng_key, uint16_t key_length);

/***************************************************************************/
/**
 * @brief 
 *   To generate a random number of the desired length.
 * @param[in] random_number 
 *   Address for the random number.
 * @param[in] length 
 *   Length of the random number in bytes.
 * @return
 *   sl_status_t.
 * For more information on status codes, see 
 * [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
******************************************************************************/
sl_status_t sl_si91x_trng_get_random_num(uint32_t *random_number, uint16_t length);

#if SLI_SI91X_TRNG_DUPLICATE_CHECK
/***************************************************************************/
/**
 * @brief 
 *   To check if there are any repeating elements in the array.
 * @param[in,out] dword 
 *   Pointer to the array of elements.
 * @param[in] length_in_dwords 
 *   Length of the array in dwords.
 * @return
 *   sl_status_t.
 * For more information on status codes, see 
 * [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
******************************************************************************/
sl_status_t sl_si91x_duplicate_element(const uint32_t *dword, uint32_t length_in_dwords);
#endif // SLI_SI91X_TRNG_DUPLICATE_CHECK

/** @} */
