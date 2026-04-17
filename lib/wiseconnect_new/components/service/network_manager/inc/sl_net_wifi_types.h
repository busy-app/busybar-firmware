/***************************************************************************/ /**
 * @file  sl_net_wifi_types.h
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

#pragma once
#include "sl_net_types.h"
#include "sl_net_ip_types.h"
#include "sl_wifi_device.h" // The device specific Wi-Fi structures and definitions
#include "sl_wifi_types.h"

/** \addtogroup SL_NET_TYPES
 * @{ */
/**
 * @brief Network Wi-Fi client profile.
 * 
 * @details
 * This structure holds the configuration for a Wi-Fi client profile, including the Wi-Fi client settings and the network IP configuration.
 */
typedef struct {
  sl_wifi_client_configuration_t
    config; ///< Wi-Fi client configuration of type [sl_wifi_client_configuration_t](../wiseconnect-api-reference-guide-wi-fi/sl-wifi-client-configuration-t)
  sl_net_ip_configuration_t ip; ///< Network IP configuration of type @ref sl_net_ip_configuration_t
  uint8_t priority;             ///< Priority for auto-join (0 = highest, 255 = lowest)
} sl_net_wifi_client_profile_t;

/**
 * @brief Network Wi-Fi AP profile.
 * 
 * @details
 * This structure holds the configuration for a Wi-Fi Access Point (AP) profile, including the Wi-Fi AP settings and the network IP configuration. 
 */
typedef struct {
  sl_wifi_ap_configuration_t
    config; ///< Wi-Fi AP configuration of type [sl_wifi_ap_configuration_t](../wiseconnect-api-reference-guide-wi-fi/sl-wifi-ap-configuration-t)
  sl_net_ip_configuration_t ip; ///< Network IP configuration of type @ref sl_net_ip_configuration_t
} sl_net_wifi_ap_profile_t;

/**
 * @brief Network Wi-Fi PSK credential entry.
 * 
 * @details
 * This structure holds the Pre-Shared Key (PSK) credentials for a Wi-Fi network, including the credential type, data length, and the actual credential data.
 */
typedef struct {
  sl_net_credential_type_t type; ///< Network credential type of @ref sl_net_credential_type_t
  uint16_t data_length;          ///< Data length
  uint8_t data[196];             ///< Data
} sl_net_wifi_psk_credential_entry_t;

/**
 * @brief Network Wi-Fi EAP credential entry.
 * 
 * @details
 * This structure holds the Extensible Authentication Protocol (EAP) credentials for a Wi-Fi network, including the credential type, data length, and the actual EAP credential data.
 */
typedef struct {
  sl_net_credential_type_t type; ///< Network credential type of @ref sl_net_credential_type_t
  uint16_t data_length;          ///< Data length
  sl_wifi_eap_credential_t
    data; ///< Data of type [sl_wifi_eap_credential_t](../wiseconnect-api-reference-guide-wi-fi/sl-wifi-eap-credential-t)
} sl_net_wifi_eap_credential_entry_t;
/** @} */
