/***************************************************************************//**
 * @file sl_wisun_br_wifi.h
 * @brief Provide Wi-FI connectivity to Wi-SUN Border Router
 *******************************************************************************
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifndef __SL_WISUN_BR_WIFI_H__
#define __SL_WISUN_BR_WIFI_H__

typedef enum {
  WIFI_SECURITY_TYPE_NONE = 0,
  WIFI_SECURITY_TYPE_WPA_PSK = 1,
  WIFI_SECURITY_TYPE_WPA2_TKIP = 2,
  WIFI_SECURITY_TYPE_WPA2_CCMP = 3,
} wifi_security_type_t;

/***************************************************************************//**
 * Connect to the given Wi-Fi Access-Point.
 *
 * @param[in] ssid SSID (Service Set Identifier)
 * @param[in] ssid_length SSID length in bytes
 * @param[in] security_type Security type (WPA-TKIP, WPA2-CCMP)
 * @param[in] passphrase PSK credential
 * @return SL_STATUS_OK if success, an error code otherwise
 ******************************************************************************/
sl_status_t sl_wisun_br_wifi_connect(const uint8_t *ssid,
                                     uint8_t ssid_length,
                                     wifi_security_type_t security_type,
                                     const uint8_t *passphrase);

/***************************************************************************//**
 * Disconnect from Wi-Fi Access-Point.
 *
 * @return SL_STATUS_OK if success, an error code otherwise
 ******************************************************************************/
sl_status_t sl_wisun_br_wifi_disconnect(void);

/***************************************************************************//**
 * Get wlan informations.
 *
 * @param[out] connected true if connected
 * @param[out] channel_number Channel number of connected AP
 * @param[out] mac_address Pointer to MAC address
 * @param[out] ipv6_address Pointer to IPv6 address
 * @return SL_STATUS_OK if success, an error code otherwise
 ******************************************************************************/
sl_status_t sl_wisun_br_wifi_get_info(bool *connected,
                                      uint16_t *channel_number,
                                      uint8_t mac_address[6],
                                      uint8_t ipv6_address[16]);

/***************************************************************************//**
 * Handler called on Wi-Fi join state changes.
 *
 * @param[in] connected true if Wi-Fi is connected successfully, false otherwise
 ******************************************************************************/
typedef void (*sl_wisun_br_wifi_join_handler_t)(bool connected);

/***************************************************************************//**
 * Register a function to handle Wi-Fi join state changes.
 *
 * @param[in] handler Handler function
 * @return SL_STATUS_OK if successful, an error code otherwise
 ******************************************************************************/
sl_status_t sl_wisun_br_wifi_set_join_handler(sl_wisun_br_wifi_join_handler_t handler);

/***************************************************************************//**
 * Initialize the Wi-Fi client.
 * @return SL_STATUS_OK if success, an error code otherwise
 ******************************************************************************/
sl_status_t sl_wisun_br_wifi_init(void);

#endif  /* __SL_WISUN_BR_WIFI_H__ */
