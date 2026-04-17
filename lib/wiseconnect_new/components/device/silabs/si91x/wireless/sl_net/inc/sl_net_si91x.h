/***************************************************************************/ /**
 * @file
 * @brief
 *******************************************************************************
 * # License
 * <b>Copyright 2019 Silicon Laboratories Inc. www.silabs.com</b>
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
#include "sl_net_constants.h"
#include "sl_wifi_host_interface.h"
#include "sli_net_constants.h"
#include "sli_net_common_utility.h"
#include "sl_utility.h"

/** \addtogroup SI91X_NETWORK_FUNCTIONS 
 * \ingroup SL_SI91X_API
 * @{ */
/***************************************************************************/
/**
 * @brief
 *   Configure the IP address for the specified virtual AP.
 * 
 * @details
 *   This function configures the IP address for the specified virtual Access Point (AP) on the Si91x device. The IP address details are provided in the `address` parameter.
 * 
 *   Virtual AP ID is used to differentiate between client and AP in concurrent mode.
 * 
 * @param[in] address
 *   Pointer to an [sl_net_ip_configuration_t](../wiseconnect-api-reference-guide-nwk-mgmt/sl-net-ip-configuration-t) structure where the assigned IP address details will be stored.
 *
 * @param[in] virtual_ap_id
 *   Virtual AP ID. One of the values from [sl_wifi_vap_id_t](../wiseconnect-api-reference-guide-wi-fi/sl-wifi-constants#sl-wifi-vap-id-t).
 * 
 * @return
 *   sl_status_t. See [Status Codes](https://docs.silabs.com/gecko-platform/latest/platform-common/status) and [WiSeConnect Status Codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) for details.
 * 
 * @note
 *   The function's wait time depends on the number of retries and intervals specified in the DHCP configuration contained in the `address` parameter. 
 *   Ensure that the DHCP retry intervals and maximum retries are configured appropriately to avoid unexpected delays.
 *   For details on calculating the wait time, refer to the [sl_net_dhcp_configuration_t](../wiseconnect-api-reference-guide-nwk-mgmt/sl_net_dhcp_configuration_t) structure
 *
 */
sl_status_t sl_si91x_configure_ip_address(sl_net_ip_configuration_t *address, uint8_t virtual_ap_id);
/** @} */

/***************************************************************************/ /**
 * @brief
 *   Register a event handler for network events.
 * @param[in] function
 *   Function pointer to callback.
 * @pre Pre-conditions:
 * - 
 *   @ref sl_si91x_driver_init should be called before this API.
 * @return
 *   sl_status_t. See https://docs.silabs.com/gecko-platform/latest/platform-common/status for details.
 ******************************************************************************/
static inline SL_DEPRECATED_API_WISECONNECT_4_0 sl_status_t
sl_si91x_register_event_handler(sl_net_event_handler_t function);

/***************************************************************************/ /**
 * @brief
 *   Default event handler for all events.
 * @param[in] event
 *   [sl_net_event_t](../wiseconnect-api-reference-guide-nwk-mgmt/sl-net-constants#sl-net-event-t) Asynchronous event received.
 * @param[in] buffer
 *   [sl_wifi_buffer_t](../wiseconnect-api-reference-guide-wi-fi/sl-wifi-buffer-t) Buffer containing data related to asynchronous event.
 * @pre Pre-conditions:
 * - 
 *   @ref sl_si91x_driver_init should be called before this API.
 * @return
 *   sl_status_t. See https://docs.silabs.com/gecko-platform/latest/platform-common/status for details.
 ******************************************************************************/
sl_status_t sl_si91x_default_handler(sl_net_event_t event, sl_wifi_buffer_t *buffer);

static inline SL_DEPRECATED_API_WISECONNECT_4_0 sl_status_t sl_si91x_set_credential(sl_net_credential_id_t id,
                                                                                    sl_net_credential_type_t type,
                                                                                    const void *credential,
                                                                                    uint32_t credential_length);

static inline SL_DEPRECATED_API_WISECONNECT_4_0 sl_status_t
sl_si91x_get_credential(sl_net_credential_id_t id,
                        const sl_net_credential_type_t *type,
                        const void *credential,
                        const uint32_t *credential_length);

static inline SL_DEPRECATED_API_WISECONNECT_4_0 sl_status_t sl_si91x_delete_credential(sl_net_credential_id_t id,
                                                                                       sl_net_credential_type_t type);

//! @cond Doxygen_Suppress
/// Enumerate multicast address command types
typedef enum {
  SL_WIFI_MULTICAST_LEAVE = 0, ///<  Wi-Fi multicast leave
  SL_WIFI_MULTICAST_JOIN  = 1, ///< Wi-Fi mulitcast join
} sl_si91x_multicast_address_command_type_t;

#define SL_SI91X_STATIC        SLI_NET_STATIC_IP
#define SL_SI91X_DHCP          SLI_NET_DHCP
#define SL_SI91X_DHCP_RESERVED SLI_NET_DHCP_RESERVED
#define SL_SI91X_DHCP_HOSTNAME SLI_NET_DHCP_HOSTNAME
#define SL_SI91X_DHCP_OPTION81 SLI_NET_DHCP_OPTION81
#define SL_SI91X_DHCP_OPTION77 SLI_NET_DHCP_OPTION77

typedef sli_net_ip_config_mode_t SL_DEPRECATED_API_WISECONNECT_4_0 sl_si91x_ip_config_mode_t;

/// Enumerate certificate types
typedef enum {
  SLI_SI91X_EAP_CLIENT             = 1,  ///< SLI_SI91X_EAP_CLIENT
  SLI_SI91X_FAST_PAC_FILE          = 2,  ///< SLI_SI91X_FAST_PAC_FILE
  SLI_SI91X_TLS_CLIENT             = 3,  ///< SLI_SI91X_TLS_CLIENT
  SLI_SI91X_TLS_CLIENT_PRIVATE_KEY = 4,  ///< SLI_SI91X_TLS_CLIENT_PRIVATE_KEY
  SLI_SI91X_TLS_CA_CERTIFICATE     = 5,  ///< SLI_SI91X_TLS_CA_CERTIFICATE
  SLI_SI91X_TLS_SERVER_CERTIFICATE = 6,  ///< SLI_SI91X_TLS_SERVER_CERTIFICATE
  SLI_SI91X_TLS_SERVER_PRIVATE_KEY = 7,  ///< SLI_SI91X_TLS_SERVER_PRIVATE_KEY
  SLI_SI91X_TLS_CBC_PRIVATE_KEY    = 8,  ///< SLI_SI91X_TLS_CBC_PRIVATE_KEY
  SLI_SI91X_TLS_ECB_PRIVATE_KEY    = 9,  ///< SLI_SI91X_TLS_ECB_PRIVATE_KEY
  SLI_SI91X_EAP_PRIVATE_KEY        = 17, ///< SLI_SI91X_EAP_PRIVATE_KEY
  SLI_SI91X_EAP_PUBLIC_KEY         = 33, ///< SLI_SI91X_EAP_PUBLIC_KEY
  SLI_SI91X_EAP_CA_CERTIFICATE     = 49, ///< SLI_SI91X_EAP_CA_CERTIFICATE
} sli_si91x_cert_type_t;

static inline sl_status_t sl_si91x_register_event_handler(sl_net_event_handler_t function)
{
  return sli_net_register_event_handler(function);
}

static inline sl_status_t sl_si91x_set_credential(sl_net_credential_id_t id,
                                                  sl_net_credential_type_t type,
                                                  const void *credential,
                                                  uint32_t credential_length)
{
  return sli_net_set_credential(id, type, credential, credential_length);
}

static inline sl_status_t sl_si91x_get_credential(sl_net_credential_id_t id,
                                                  const sl_net_credential_type_t *type,
                                                  const void *credential,
                                                  const uint32_t *credential_length)
{
  return sli_net_get_credential(id, type, credential, credential_length);
}

static inline sl_status_t sl_si91x_delete_credential(sl_net_credential_id_t id, sl_net_credential_type_t type)
{
  return sli_net_delete_credential(id, type);
}

//! @endcond
