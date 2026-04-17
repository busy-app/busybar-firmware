/***************************************************************************/ /**
 * @file  sl_si91x_psa_mac.h
 * @brief SL SI91X PSA HMAC Header file
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

#ifndef SL_SI91X_PSA_MAC_H
#define SL_SI91X_PSA_MAC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "sli_si91x_crypto_driver_functions.h"
#include "string.h"
#include "sl_status.h"
#include "sl_si91x_crypto.h"

/***************************************************************************/ /**
 * @brief This API will calculate the MAC (message authentication code) of a message.
 * @param[in] attributes 
 *   The attributes of the key to use for the operation.
 * @param[in] key_buffer
 *   The buffer containing the key to use for computing the MAC.
 * @param[in] key_buffer_size
 *   Size of the \p key_buffer buffer in bytes.
 * @param[in] alg
 *   The MAC algorithm to use (\c PSA_ALG_XXX value such that #PSA_ALG_IS_MAC(\p alg) is true).
 * @param[in] input
 *   Buffer containing the input message.
 * @param[in] input_length
 *   Size of the \p input buffer in bytes.
 * @param[out] mac 
 *   Buffer where the MAC value is to be written.
 * @param[in] mac_size
 *   Size of the \p mac buffer in bytes.
 * @param[out] mac_length
 *   On success, the number of bytes that make up the MAC value.
 * @return 
 *   psa_status_t. See https://docs.silabs.com/gecko-platform/4.1/service/api/group-error for details.
******************************************************************************/
psa_status_t sli_si91x_crypto_mac_compute(const psa_key_attributes_t *attributes,
                                          const uint8_t *key_buffer,
                                          size_t key_buffer_size,
                                          psa_algorithm_t alg,
                                          const uint8_t *input,
                                          size_t input_length,
                                          uint8_t *mac,
                                          size_t mac_size,
                                          size_t *mac_length);

#ifdef __cplusplus
}
#endif

#endif /* SL_SI91X_PSA_MAC_H */