/*******************************************************************************
 * @file  sl_si91x_psa_trng.c
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
#include "sli_si91x_crypto_driver_functions.h"
#include "sl_si91x_psa_trng.h"
#include "sl_si91x_constants.h"
#include "sl_status.h"
#include "sl_si91x_trng.h"
#include <stdio.h>

/* TRNG key */
extern uint32_t trng_key[TRNG_KEY_SIZE];

psa_status_t sli_si91x_crypto_trng_init()
{
  psa_status_t status = PSA_ERROR_CORRUPTION_DETECTED;
  sl_status_t sl_status;

  /* This API checks the Entropy of TRNG i.e source for TRNG */
  sl_status = sl_si91x_trng_entropy();
  /* Convert the error code from si91x to psa */
  status = convert_si91x_error_code_to_psa_status(sl_status);

  if (status != PSA_SUCCESS) {
    return status;
  }

  /* This API Initializes key which needs to be programmed to TRNG hardware engine */
  sl_status = sl_si91x_trng_program_key(trng_key, TRNG_KEY_SIZE);
  /* Convert the error code from si91x to psa */
  status = convert_si91x_error_code_to_psa_status(sl_status);

  if (status != PSA_SUCCESS) {
    return status;
  }

  return status;
}

psa_status_t sl_si91x_psa_get_random(uint8_t *output, size_t len, size_t *out_len)
{
  if (output == NULL || len == 0 || out_len == NULL) {
    return PSA_ERROR_INVALID_ARGUMENT;
  }
  psa_status_t status = PSA_ERROR_CORRUPTION_DETECTED;
  sl_status_t sl_status;

  //! Memset the buffer to zero
  memset(output, 0, len);
  //! Get Random number of desired length
  sl_status = sl_si91x_trng_get_random_num((uint32_t *)output, len);

  /* Convert the error code from si91x to psa */
  status = convert_si91x_error_code_to_psa_status(sl_status);

  if (status != PSA_SUCCESS) {
    return status;
  }

  *out_len = len;
  return status;
}
