/*******************************************************************************
 * @file  sli_si91x_crypto_driver_functions.c
 * @brief SLI SI91X PSA Crypto common utility file
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
#include "sl_status.h"
#include "sl_additional_status.h"

/*==============================================*/
/**
 * @fn          psa_status_t convert_si91x_error_code_to_psa_status(sl_status_t si91x_status)
 * @brief       This API gets the si91x error codes and returns its equivalent psa_status codes
 * @param[in]   si91x_status, si91x error code
 * @return      returns psa_status error code.
 *
 * @section description
 *       This API gets the si91x error codes and returns its equivalent psa_status codes.
 */
psa_status_t convert_si91x_error_code_to_psa_status(sl_status_t si91x_status)
{
  psa_status_t status = PSA_SUCCESS;
  switch (si91x_status) {
    case SL_STATUS_OK:
      break;
    case SL_STATUS_INVALID_PARAMETER:
      status = PSA_ERROR_INVALID_ARGUMENT;
      break;
    case SL_STATUS_SI91X_COMMAND_GIVEN_IN_INVALID_STATE:
      status = PSA_ERROR_BAD_STATE;
      break;
    case SL_STATUS_ALLOCATION_FAILED:
      status = PSA_ERROR_COMMUNICATION_FAILURE;
      break;
    case SL_STATUS_NOT_SUPPORTED:
      status = PSA_ERROR_NOT_SUPPORTED;
      break;
    case SL_STATUS_TIMEOUT:
      status = PSA_ERROR_COMMUNICATION_FAILURE;
      break;
    case SL_STATUS_CRYPTO_INVALID_PARAMETER:
      status = PSA_ERROR_INVALID_ARGUMENT;
      break;
    case SL_STATUS_CRYPTO_INVALID_SIGNATURE:
      status = PSA_ERROR_INVALID_SIGNATURE;
      break;
    case SL_STATUS_TRANSMIT:
      status = PSA_ERROR_COMMUNICATION_FAILURE;
      break;
    default:
      status = PSA_ERROR_GENERIC_ERROR;
      break;
  }
  return status;
}
