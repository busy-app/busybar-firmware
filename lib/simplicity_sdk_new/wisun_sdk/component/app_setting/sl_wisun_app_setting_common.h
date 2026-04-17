/***************************************************************************//**
 * @file sl_wisun_app_setting_common.h
 * @brief Wi-SUN Application Common Settings
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

#ifndef SL_WISUN_APP_SETTING_COMMON_H
#define SL_WISUN_APP_SETTING_COMMON_H

#ifdef __cplusplus
extern "C" {
#endif

// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------
#include <stdint.h>

#include "sl_status.h"
#include "sl_wisun_api.h"

/**************************************************************************//**
 * @addtogroup SL_WISUN_APP_SETTING
 * @{
 *****************************************************************************/

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

///  Wi-SUN default network name size
#define APP_SETTING_NETWORK_NAME_MAX_SIZE   (SL_WISUN_NETWORK_NAME_SIZE + 1)

/// Wi-SUN settings default subscription channel
#define APP_SETTING_DEFAULT_SUBSCRIPT_CH    (0U)

/**************************************************************************//**
 * @addtogroup APP_SETTING_TYPES Type definitions
 * @ingroup SL_WISUN_APP_SETTING
 * @{
 *****************************************************************************/
/// App settings notification channels
typedef enum app_setting_notification {
  /// Set Network Name notification
  APP_SETTING_NOTIFICATION_SET_NETWORK_NAME = 0UL,
  /// Set Network Size notification
  APP_SETTING_NOTIFICATION_SET_NETWORK_SIZE,
  /// Set TX Power notification
  APP_SETTING_NOTIFICATION_SET_TX_POWER,
  /// Set TX Power notification
  APP_SETTING_NOTIFICATION_SET_PHY_CFG,
  /// Set Wifi SSID
  APP_SETTING_NOTIFICATION_SET_SSID,
  /// Set Wifi Passphrase
  APP_SETTING_NOTIFICATION_SET_PASSPHRASE,
  /// Set Wifi Security
  APP_SETTING_NOTIFICATION_SET_SECURITY,
  /// Set BR settings
  APP_SETTING_NOTIFICATION_SET_BR_SETTINGS
} app_setting_notification_t;

/// Wisun setting structure
typedef struct app_setting_wisun {
  /// Network Name
  char network_name[APP_SETTING_NETWORK_NAME_MAX_SIZE];
  /// Network size
  uint8_t network_size;
  /// TX Power
  int16_t tx_power_ddbm;
  /// Device type
  uint8_t device_type;
  /// LFN profile
  uint8_t lfn_profile;
  /// default flag
  bool is_default_phy;
  /// PHY settings
  sl_wisun_phy_config_t phy;
  /// Key chain
  uint8_t keychain;
  /// Key chain index
  uint8_t keychain_index;
} app_setting_wisun_t;

/** @} (end APP_SETTING_TYPES) */

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                          Public Function Declarations
// -----------------------------------------------------------------------------

/**************************************************************************//**
 * @brief Initialize Wi-SUN setting
 *****************************************************************************/
void app_wisun_setting_common_init(void);

/**************************************************************************//**
 * @brief Subscribe to setting notification channel
 * @details Notifications are received on the channel (output) about notification type
 * @param[in] notif Type of the notification 'app_setting_notification_t'
 * @param[out] channel Provided channel number, this is the output of the subscription
 * @return sl_status_t SL_STATUS_OK on success, otherwise SL_STATUS_FAIL
 *****************************************************************************/
sl_status_t app_wisun_setting_subscribe_notification(const app_setting_notification_t notif,
                                                     uint8_t * const channel);

/**************************************************************************//**
 * @brief Is setting notified getter
 * @details Polling notification flag
 * @param[in] notif Type of the notification 'app_setting_notification_t'
 * @param[in] channel channel number, this is the output of the subscription
 * @return bool true if channel is notified by setting, otherwise false
 *****************************************************************************/
bool app_wisun_setting_is_notified(const app_setting_notification_t notif,
                                   const uint8_t channel);

/**************************************************************************//**
 * @brief Unsubscribe from setting notification
 * @details Delete notification channel flag from subscripted channel flags
 * @param[in] notif Type of the notification 'app_setting_notification_t'
 * @param[in] channel channel number, this is the output of the subscription
 *****************************************************************************/
void app_wisun_setting_unsubscribe(const app_setting_notification_t notif,
                                   const uint8_t channel);

/**************************************************************************//**
 * @brief Acknowledge notification
 * @details Delete notification channel flag
 * @param[in] notif ype of the notification 'app_setting_notification_t'
 * @param[in] channel channel number, this is the output of the subscription
 *****************************************************************************/
void app_wisun_setting_ack_notification(const app_setting_notification_t notif,
                                        const uint8_t channel);

/**************************************************************************//**
 * @brief App setting notify
 * @details Notify subscribed channels about settings calls
 * @param[in] notif Notification type
 * @return sl_status_t SL_STATUS_OK on success, otherwise SL_STATUS_FAIL
 *****************************************************************************/
sl_status_t app_wisun_setting_notify(app_setting_notification_t notif);

/** @}*/

#ifdef __cplusplus
}
#endif

#endif // SL_WISUN_APP_SETTING_COMMON_H
