/***************************************************************************//**
 * @file sl_wisun_app_setting_br.h
 * @brief Wi-SUN Application Border Router Settings
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

#ifndef SL_WISUN_APP_SETTING_BR_H
#define SL_WISUN_APP_SETTING_BR_H

#ifdef __cplusplus
extern "C" {
#endif

// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------
#include <stdint.h>

#include "sl_status.h"
#include "sl_wisun_api.h"
#include "sl_wisun_config.h"
#include "sl_component_catalog.h"

#include "sl_wisun_types.h"

#include "sl_wisun_app_setting_common.h"

/**************************************************************************//**
 * @addtogroup SL_WISUN_APP_SETTING
 * @{
 *****************************************************************************/

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

///  Wi-SUN default network name size
#define APP_SETTING_NETWORK_NAME_MAX_SIZE   (SL_WISUN_NETWORK_NAME_SIZE + 1)

/// Size of the printable data buffer
#define APP_UTIL_PRINTABLE_DATA_MAX_LENGTH  (64)

/// Size of the IPv6 prefix
#define APP_IPV6_PREFIX_SIZE                (43)

/// Wifi SSID size
#define SL_WISUN_WIFI_SSID_SIZE             (32)

/// Wifi Passphrase size
#define SL_WISUN_WIFI_PASSPHRASE_SIZE       (32)

/**************************************************************************//**
 * @addtogroup APP_SETTING_TYPES Type definitions
 * @ingroup SL_WISUN_APP_SETTING
 * @{
 *****************************************************************************/

/// Wisun BR setting structure
SL_PACK_START(1)
typedef struct app_setting_br {
  /// Network Name
  char network_name[APP_SETTING_NETWORK_NAME_MAX_SIZE];
  /// Network size
  uint8_t network_size;
  /// TX Power
  int16_t tx_power_ddbm;
  /// UC Dwell interval in ms
  uint8_t uc_dwell_interval_ms;
  /// BC interval in ms
  uint32_t bc_interval_ms;
  /// BC Dwell interval in ms
  uint8_t bc_dwell_interval_ms;
  /// State
  uint8_t state;
  /// Allowed channels
  char allowed_channels[APP_UTIL_PRINTABLE_DATA_MAX_LENGTH + 1];
  /// IPv6 prefix
  char ipv6_prefix[APP_IPV6_PREFIX_SIZE + 1];
  /// Regulation
  uint8_t regulation;
  /// FEC
  uint8_t fec;
  /// RX PHY mode IDs
  uint8_t rx_phy_mode_ids[SL_WISUN_MAX_PHY_MODE_ID_COUNT];
  /// RX PHY mode IDs count
  uint8_t rx_phy_mode_ids_count;
  /// LFN profile
  uint8_t lfn_profile;
  /// Maximum neighbor count
  uint8_t max_neighbor_count;
  /// Maximum child count
  uint8_t max_child_count;
  /// Maximum security neighbor count
  uint16_t max_security_neighbor_count;
  /// Key chain
  uint8_t keychain;
  /// Key chain index
  uint8_t keychain_index;
  /// Socket RX buffer size
  uint16_t socket_rx_buffer_size;
  /// PHY configuration type
  sl_wisun_phy_config_t phy;
  /// Default PHY
  bool is_default_phy;
  /// PAN ID
  uint16_t pan_id;
} SL_ATTRIBUTE_PACKED app_setting_br_t;
SL_PACK_END()

typedef struct app_setting_wifi {
  uint8_t ssid[SL_WISUN_WIFI_SSID_SIZE + 1]; ///< SSID value
  uint8_t passphrase[SL_WISUN_WIFI_PASSPHRASE_SIZE + 1];  ///< PSK credential
  uint8_t security_type;
} app_setting_wifi_t;

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
void app_wisun_setting_init(void);

/**************************************************************************//**
 * @brief Get the Wi-SUN BR settings.
 * @param[out] wisun_setting is the obtained Wi-SUN BR setting
 * @return sl_status_t if the getting is successful it returns SL_STATUS_OK, otherwise
 * error code.
 *****************************************************************************/
sl_status_t app_wisun_setting_br_get(app_setting_br_t *const wisun_setting);

/**************************************************************************//**
 * @brief Set the Wi-SUN BR settings.
 * @param[in] settings is the Wi-SUN BR setting to be set
 * @return sl_status_t if the setting is successful it returns SL_STATUS_OK, otherwise
 * error code.
 *****************************************************************************/
sl_status_t app_wisun_setting_br_set(const app_setting_br_t *const settings);

#if defined(SL_CATALOG_WISUN_BR_WIFI_PRESENT)
/**************************************************************************//**
 * @brief Get the Wi-SUN Wifi settings.
 * @param[out] wisun_setting is the obtained Wi-SUN Wifi setting
 * @return sl_status_t if the getting is successful it returns SL_STATUS_OK, otherwise
 * error code.
 *****************************************************************************/
sl_status_t app_wisun_setting_wifi_get(app_setting_wifi_t *const wisun_setting);

/**************************************************************************//**
 * @brief Set the Wi-SUN Wifi SSID.
 * @param[out] ssid is the SSID to be set.
 * @return sl_status_t if the setting is successful it returns SL_STATUS_OK, otherwise
 * error code.
 *****************************************************************************/
sl_status_t app_wisun_setting_set_ssid(const uint8_t *const ssid);

/**************************************************************************//**
 * @brief Get the Wi-SUN Wifi SSID.
 * @param[out] ssid is the SSID to be obtained.
 * @param[in] size is the size of the SSID buffer
 * @return sl_status_t if the getting is successful it returns SL_STATUS_OK, otherwise
 * error code.
 *****************************************************************************/
sl_status_t app_wisun_setting_get_ssid(uint8_t *const ssid, uint8_t size);

/**************************************************************************//**
 * @brief Set the Wi-SUN Wifi security.
 * @param[out] security is the security to be set.
 * @return sl_status_t if the setting is successful it returns SL_STATUS_OK, otherwise
 * error code.
 *****************************************************************************/
sl_status_t app_wisun_setting_set_security(const uint8_t *const security);

/**************************************************************************//**
 * @brief Get the Wi-SUN Wifi security.
 * @param[out] security is the obtained security setting
 * @return sl_status_t if the getting is successful it returns SL_STATUS_OK, otherwise
 * error code.
 *****************************************************************************/
sl_status_t app_wisun_setting_get_security(uint8_t *const security);

/**************************************************************************//**
 * @brief Set the Wi-SUN Wifi passphrase.
 * @param[out] passphrase is the passphrase to be set.
 * @return sl_status_t if the setting is successful it returns SL_STATUS_OK, otherwise
 * error code.
 *****************************************************************************/
sl_status_t app_wisun_setting_set_passphrase(const uint8_t *const passphrase);

/**************************************************************************//**
 * @brief Get the Wi-SUN Wifi passphrase.
 * @param[out] passphrase is the obtained passphrase
 * @param[in] size is the size of the passphrase buffer
 * @return sl_status_t if the getting is successful it returns SL_STATUS_OK, otherwise
 * error code.
 *****************************************************************************/
sl_status_t app_wisun_setting_get_passphrase(uint8_t *const passphrase, uint8_t size);
#endif

/**************************************************************************//**
 * @brief Set the Wi-SUN network name.
 * @param[out] name is the network name that will be set.
 * @return sl_status_t if the set is successful it returns SL_STATUS_OK, otherwise
 * error code.
 *****************************************************************************/
sl_status_t app_wisun_setting_set_network_name(const char *const name);

/**************************************************************************//**
 * @brief Set the Wi-SUN network size.
 * @param[in] size is the pointer to network size that will be set.
 * @return sl_status_t if the set is successful it returns SL_STATUS_OK, otherwise
 * error code.
 *****************************************************************************/
sl_status_t app_wisun_setting_set_network_size(const uint8_t *const size);

/**************************************************************************//**
 * @brief Set the Wi-SUN TX power.
 * @param[out] tx_power is the pointer to TX power that will be set.
 * @return sl_status_t if the set is successful it returns SL_STATUS_OK, otherwise
 * error code.
 *****************************************************************************/
sl_status_t app_wisun_setting_set_tx_power(const int16_t * const tx_power);

/**************************************************************************//**
 * @brief Set the Wi-SUN PAN ID.
 * @param[out] pan_id is the pointer to PAN ID that will be set.
 * @return sl_status_t if the set is successful it returns SL_STATUS_OK, otherwise
 * error code.
 *****************************************************************************/
sl_status_t app_wisun_setting_set_pan_id(const uint16_t *const pan_id);

/**************************************************************************//**
 * @brief Set the Wi-SUN PHY.
 * @param[out] phy is the PHY that will be set.
 * @return sl_status_t if the set is successful it returns SL_STATUS_OK, otherwise
 * error code.
 *****************************************************************************/
sl_status_t app_wisun_setting_set_phy(const sl_wisun_phy_config_t *const phy);

/**************************************************************************//**
 * @brief Get the Wi-SUN network name.
 * @param[out] name pointer where the name is copied onto
 * @param[in] size is the size of the name buffer
 * @return sl_status_t it is successful if it returns SL_STATUS_OK otherwise
 *         it is not.
 *****************************************************************************/
sl_status_t app_wisun_setting_get_network_name(char *const name, uint8_t size);

/**************************************************************************//**
 * @brief Get the Wi-SUN network size.
 * @param[out] size pointer where the network size is copied onto
 * @return sl_status_t it is successful if it returns SL_STATUS_OK otherwise
 *         it is not.
 *****************************************************************************/
sl_status_t app_wisun_setting_get_network_size(uint8_t *const size);

/**************************************************************************//**
 * @brief Get the Wi-SUN TX power.
 * @param[out] tx_power pointer where the TX power is copied onto
 * @return sl_status_t it is successful if it returns SL_STATUS_OK otherwise
 *         it is not.
 *****************************************************************************/
sl_status_t app_wisun_setting_get_tx_power(int16_t * const tx_power);

/**************************************************************************//**
 * @brief Get the Wi-SUN PHY.
 * @param[out] phy pointer where the PHY is copied onto
 * @return sl_status_t it is successful if it returns SL_STATUS_OK otherwise
 *         it is not.
 *****************************************************************************/
sl_status_t app_wisun_setting_get_phy(sl_wisun_phy_config_t *const phy);

/**************************************************************************//**
 * @brief Init internal PHY settings
 * @details Full radio config: data initialized with default HPY for appropriate board
 *          Simple PHY: get first element of PHY list by 'sl_wisun_util_get_rf_settings'
 * @return sl_status_t it is successful if it returns SL_STATUS_OK otherwise
 *         it is not.
 *****************************************************************************/
sl_status_t app_wisun_setting_init_phy_cfg(void);

/**************************************************************************//**
 * @brief Convert a string representation of channels to channel mask
 * @details This function converts a string containing channel information to
 *          a Wi-SUN channel mask structure
 * @param[in] str String representation of the channel list
 * @param[out] channel_mask Pointer to the channel mask to be populated
 * @return sl_status_t SL_STATUS_OK on success or appropriate error code
 *****************************************************************************/
sl_status_t app_settings_get_channel_mask(const char *str,
                                          sl_wisun_channel_mask_t *channel_mask);

/** @}*/

#ifdef __cplusplus
}
#endif

#endif // SL_WISUN_APP_SETTING_BR_H
