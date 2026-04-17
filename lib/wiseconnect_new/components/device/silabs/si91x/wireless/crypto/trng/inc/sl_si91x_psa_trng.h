/*******************************************************************************
 * @file  sl_si91x_psa_trng.h
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

#ifndef SL_SI91X_PSA_TRNG_H
#define SL_SI91X_PSA_TRNG_H

#include "string.h"
/* Include file for error code conversion */
#include "sl_si91x_crypto.h"
#include "psa/crypto.h"

/***************************************************************************/ /**
 * @fn          psa_status_t sl_si91x_psa_get_random()
 * @brief
 *   This API attempts to read the requested amount of conditioned entropy from the TRNG peripheral.
 * @param[in]   len
 *   Amount of bytes to read from the FIFO into the output
 * @param[out]  output
 *   Output buffer to contain the entropy
 * @param[out]  out_len
 *   Amount of bytes actually written into the output buffer
 * @return
 *   returns psa_status_t PSA_SUCCESS  on success, on failure return error code
******************************************************************************/
psa_status_t sl_si91x_psa_get_random(uint8_t *output, size_t len, size_t *out_len);

/***************************************************************************/ /**
 * @fn          psa_status_t sli_si91x_crypto_trng_init()
 * @brief
 *   This API Checks entropy and initializes key which needs to be programmed to TRNG hardware engine.
 * @return
 *   returns psa_status_t PSA_SUCCESS  on success, on failure return error code
******************************************************************************/
psa_status_t sli_si91x_crypto_trng_init();

#endif /* SL_SI91X_PSA_TRNG_H */
