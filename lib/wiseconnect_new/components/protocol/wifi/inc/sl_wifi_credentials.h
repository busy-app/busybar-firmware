/********************************************************************************
 * @file  sl_wifi_credentials.h
 *******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifndef SL_WIFI_CREDENTIALS_H
#define SL_WIFI_CREDENTIALS_H

#include "sl_wifi_types.h"

/** 
 * \addtogroup WIFI_CREDENTIAL_FUNCTIONS Wi-Fi Credential
 * \ingroup SL_WIFI_FUNCTIONS
 * @{ */

/***************************************************************************/ /**
 * @brief
 *   Store a credential in specified credential identifier.
 * 
 * @details
 *   This function stores the credential type and data for the specified credential ID.
 *   The credential data can include client credentials, access point credentials and user credentials (user credential is a generic credential identifier that can be configured to represent a specific credential type, such as TLS, MQTT, or HTTP, based on the chosen protocol).
 * 
 *   Repeatedly calling this API with the same ID will overwrite the existing credential type and data.
 *  
 * @pre Pre-conditions:
 * - @ref sl_wifi_init should be called before this API.
 * 
 * @param[in] id
 *   Credential identifier as identified by @ref sl_wifi_credential_id_t.
 * 
 * @param[in] type
 *   Credential type as identified by @ref sl_wifi_credential_type_t.
 * 
 * @param[in] credential
 *   Pointer to the credential data object.
 * 
 * @param[in] credential_length
 *   Length of the credential data object.
 * 
 * @return
 *   sl_status_t. See [Status Codes](https://docs.silabs.com/gecko-platform/latest/platform-common/status) and [WiSeConnect Status Codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) for details.
 ******************************************************************************/
sl_status_t sl_wifi_set_credential(sl_wifi_credential_id_t id,
                                   sl_wifi_credential_type_t type,
                                   const void *credential,
                                   uint32_t credential_length);

/***************************************************************************/ /**
 * @brief
 *   Retrieve a stored credential.
 * 
 * @details
 *   This function retrieves the credential data for the specified credential ID.
 *   The retrieved credential data is stored in the provided credential object.
 * 
 * @pre Pre-conditions:
 * - @ref sl_wifi_init should be called before this API.
 * 
 * @param[in] id
 *   Credential identifier as identified by @ref sl_wifi_credential_id_t.
 * 
 * @param[out] type
 *   Credential type as identified by @ref sl_wifi_credential_type_t.
 * 
 * @param[out] credential
 *   Pointer to location where credential data is stored.
 * 
 * @param[in,out] credential_length
 *   in: Number of bytes available at credential.
 *   out: Number of bytes written.
 * 
 * @return
 *   sl_status_t. See [Status Codes](https://docs.silabs.com/gecko-platform/latest/platform-common/status) and [WiSeConnect Status Codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) for details.
 ******************************************************************************/
sl_status_t sl_wifi_get_credential(sl_wifi_credential_id_t id,
                                   sl_wifi_credential_type_t *type,
                                   void *credential,
                                   uint32_t *credential_length);

/***************************************************************************/ /**
 * @brief
 *   Delete a stored credential.
 * 
 * @details
 *   This function deletes the credential data for the specific credential ID and type.
 *   Once deleted, the credential cannot be used further for any operations.
 * 
 * @pre Pre-conditions:
 * -
 *   @ref sl_wifi_init should be called before this API.
 * 
 * @param[in] id
 *   Credential identifier as identified by @ref sl_wifi_credential_id_t.
 * 
 * @return
 *   sl_status_t. See [Status Codes](https://docs.silabs.com/gecko-platform/latest/platform-common/status) and [WiSeConnect Status Codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) for details.
 ******************************************************************************/
sl_status_t sl_wifi_delete_credential(sl_wifi_credential_id_t id);

/** @} */

#endif //SL_WIFI_CREDENTIALS_H
