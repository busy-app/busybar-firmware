/***************************************************************************/ /**
 * @file  sli_si91x_crypto_driver_functions.h
 * @brief SLI SI91X PSA Crypto common utility file
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

#include "psa/crypto.h"
#include "sl_status.h"
#include "sl_si91x_crypto.h"
#include "sli_psa_driver_features.h"

#ifdef SLI_CIPHER_DEVICE_SI91X
#include "sl_si91x_psa_aes.h"
#endif

#ifdef SLI_AEAD_DEVICE_SI91X
#include "sl_si91x_psa_aead.h"
#endif

#ifdef SLI_TRNG_DEVICE_SI91X
#include "sl_si91x_psa_trng.h"
#endif

#ifdef SLI_ECDH_DEVICE_SI91X
#include "sl_si91x_psa_ecdh.h"
#endif

#ifdef SLI_ECDSA_DEVICE_SI91X
#include "sl_si91x_psa_ecdsa.h"
#endif

#ifdef SLI_MAC_DEVICE_SI91X
#include "sl_si91x_psa_mac.h"
#endif

#ifdef SLI_SHA_DEVICE_SI91X
#include "sl_si91x_psa_sha.h"
#endif

/******************************************************
 *                Function Declarations
*******************************************************/
/**
 * @addtogroup CRYPTO_FUNCTIONS
 * @{ 
 */

/***************************************************************************/
/**
 * @brief 
 *   To convert the SI91X error codes to PSA status codes.
 * @param[in] si91x_status 
 *   Status of type sl_status_t
 * @return
 *   psa_status_t
******************************************************************************/
psa_status_t convert_si91x_error_code_to_psa_status(sl_status_t si91x_status);

/** @} */
