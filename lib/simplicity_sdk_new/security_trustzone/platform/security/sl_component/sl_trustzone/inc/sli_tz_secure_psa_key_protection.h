/***************************************************************************//**
 * @file
 * @brief Silicon Labs PSA Key ID Protection API.
 *******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 ******************************************************************************/

#ifndef SL_PSA_KEY_PROTECTION_H
#define SL_PSA_KEY_PROTECTION_H

#include "psa/crypto.h"
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif
/***************************************************************************//**
 * @brief
 *   Check if NonSecure access to a key ID should be denied.
 *   
 *   This is the main entry point for access control. It checks if the key ID
 *   is protected AND if the caller is NonSecure.
 *
 * @param[in] key_id
 *   The PSA key ID being accessed.
 *
 * @return
 *   PSA_SUCCESS if access is allowed.
 *   PSA_ERROR_NOT_PERMITTED if the key ID is protected and caller is NonSecure.
 ******************************************************************************/
psa_status_t sl_psa_key_check_access(psa_key_id_t key_id);

/// Caller ID value indicating NonSecure caller (for use with sl_psa_key_check_access)
#define SL_PSA_KEY_CALLER_NONSECURE  (0)

/// Macro to check if caller_id represents a NonSecure caller
#define SL_PSA_KEY_CALLER_IS_NS(caller_id)  ((caller_id) == SL_PSA_KEY_CALLER_NONSECURE)

#ifdef __cplusplus
}
#endif

#endif // SL_PSA_KEY_PROTECTION_H
