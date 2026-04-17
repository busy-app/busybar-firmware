/*******************************************************************************
 * @file  sl_si91x_psa_wrap.h
 * @brief
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

#ifndef SL_SI91X_PSA_WRAP_H
#define SL_SI91X_PSA_WRAP_H

#include "sli_si91x_crypto_driver_functions.h"
#include "string.h"
#include "sl_status.h"
#include "sl_si91x_crypto.h"

extern unsigned char WRAP_IV[];
#define PSA_KEY_VOLATILE_PERSISTENT_WRAPPED ((psa_key_location_t)0x000001)
// The PSA key is given in wrapped format to psa_import_key and other APIs

#define PSA_KEY_VOLATILE_PERSISTENT_WRAP_IMPORT ((psa_key_location_t)0x800000)
// The PSA key is given in plain format, to be wrapped in psa_import_key API and used in wrapped format with other APIs

/**
 * @brief
 *   This API is used to wrap a key
 * @param[in]  key_buffer
 *   Input key buffer and buffer to store the wrapped key
 * @param[in]  key_buffer_size
 *   Key size
 * @param[in]  wrap_mode
 *   Wrapping method(ECB/CBC)
 * @param[in]  wrap_iv_var
 *   IV for AES CBC mode
 * @return
 *   psa_status_t PSA_SUCCESS on success, return error code on failure
 */
psa_status_t sli_si91x_crypto_wrap_key(uint8_t *key_buffer,
                                       size_t key_buffer_size,
                                       uint32_t wrap_mode,
                                       uint8_t *wrap_iv_var);

/***************************************************************************/ /**
 * @brief
 *   This API is used to generate and wrap a symmetric key
 * @param[in]  key_buffer
 *   Input key buffer and buffer to store the generated wrapped key
 * @param[in]  key_buffer_size
 *   Key size in bytes
 * @param[in]  wrap_mode
 *   Wrapping method(ECB/CBC)
 * @param[in]  wrap_iv_var
 *   IV for AES CBC mode
 * @return
 *   psa_status_t PSA_SUCCESS on success, return error code on failure
******************************************************************************/
psa_status_t sli_si91x_crypto_generate_symm_key(uint8_t *key_buffer,
                                                size_t key_buffer_size,
                                                uint32_t wrap_mode,
                                                uint8_t *wrap_iv_var);

#endif /* SL_SI91X_PSA_WRAP_H */
