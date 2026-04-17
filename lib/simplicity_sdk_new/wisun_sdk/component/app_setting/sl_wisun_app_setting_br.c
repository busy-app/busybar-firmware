/***************************************************************************//**
 * @file sl_wisun_app_setting_br.c
 * @brief Wi-SUN Application Border Router settings
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

// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------
#include <stddef.h>
#include <string.h>
#include <stdlib.h>

#include "sl_assert.h"
#include "sl_status.h"
#include "sl_string.h"
#include "cmsis_os2.h"
#include "sl_cmsis_os2_common.h"
#include "sl_wisun_api.h"
#include "sl_wisun_cli_settings.h"
#include "sl_wisun_app_setting_br.h"
#include "sl_component_catalog.h"
#include "sl_wisun_config.h"

#include "sl_wisun_types.h"
#include "sl_wisun_keychain.h"

#if defined(SL_CATALOG_WISUN_BR_WIFI_PRESENT)
#include "sl_wisun_br_wifi.h"
#endif

#include "sl_wisun_br_config.h"

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

/// Default Network name for initialization
#define APP_SETTINGS_DEFAULT_NETWORK_NAME           "Wi-SUN Network"

/// Default Wifi SSID for initialization
#define APP_SETTINGS_DEFAULT_WIFI_SSID              "DEFAULT_SSID"

/// Default Wifi Passphrase for initialization
#define APP_SETTINGS_DEFAULT_WIFI_PASSPHRASE        "DEFAULT_PASSPHRASE"

// -----------------------------------------------------------------------------
//                          Static Function Declarations
// -----------------------------------------------------------------------------

/**************************************************************************//**
 * @brief Acquire application BR mutex
 * @details Internal BR mutex lock
 *****************************************************************************/
__STATIC_INLINE void _wisun_mutex_br_acquire(void);

/**************************************************************************//**
 * @brief Release application BR mutex
 * @details Internal BR mutex release
 *****************************************************************************/
__STATIC_INLINE void _wisun_mutex_br_release(void);

/**************************************************************************//**
 * @brief Helper function for checking the name of the Wi-SUN network
 * @param [in] *name is the network name that are wanted to check
 * @param [out] *name_len is the length of the name that calculated by the function.
 * @return char* const char pointer that points to checked name or to the default
 *          one if the checked one is incorrect.
 *****************************************************************************/
static const char *_app_check_nw_name(const char *name, size_t *const name_len);

#if defined(SL_CATALOG_WISUN_BR_WIFI_PRESENT)
/**************************************************************************//**
 * @brief Helper function for checking the SSID
 * @param [in] *ssid is the SSID that are wanted to check
 * @param [out] *ssid_len is the length of the SSID that calculated by the function.
 * @return char* const char pointer that points to checked SSID or to the default
 *          one if the checked one is incorrect.
 *****************************************************************************/
static const uint8_t *_app_check_ssid(const uint8_t *ssid, size_t *const ssid_len);

/**************************************************************************//**
 * @brief Helper function for checking the Passphrase
 * @param [in] *passphrase is the Passphrase that are wanted to check
 * @param [out] *passphrase_len is the length of the Passphrase that calculated by the function.
 * @return char* const char pointer that points to checked Passphrase or to the default
 *          one if the checked one is incorrect.
 *****************************************************************************/
static const uint8_t *_app_check_passphrase(const uint8_t *passphrase, size_t *const passphrase_len);
#endif

/**************************************************************************//**
 * @brief Convert ranges to mask
 * @details Convert ranges to mask
 * @param[in] str String containing ranges
 * @param[out] mask Pointer to the mask to be filled
 * @param[in] size Size of the mask
 * @return sl_status_t SL_STATUS_OK on success, otherwise SL_STATUS_FAIL
 *****************************************************************************/
static sl_status_t _app_ranges_to_mask(const char *str, uint8_t *mask, uint32_t size);

// -----------------------------------------------------------------------------
//                                Static Variables
// -----------------------------------------------------------------------------

/// App BR settings mutex
static osMutexId_t _wisun_setting_br_mtx;

/// App BR settings mutex attribute
static const osMutexAttr_t _wisun_setting_br_mtx_attr = {
  .name      = "WisunSettingBrMutex",
  .attr_bits = osMutexRecursive,
  .cb_mem    = NULL,
  .cb_size   = 0
};

/// Wi-SUN border router settings
static app_setting_br_t _wisun_br_settings = { 0 };

#if defined(SL_CATALOG_WISUN_BR_WIFI_PRESENT)
/// Wi-SUN border router WiFi settings
static app_setting_wifi_t _wisun_wifi_settings = { 0 };
#endif

/// Default br settings structure
static const app_setting_br_t _wisun_br_settings_default = {
#if defined(WISUN_CONFIG_NETWORK_NAME)
  .network_name = WISUN_CONFIG_NETWORK_NAME,
#else
  .network_name = APP_SETTINGS_DEFAULT_NETWORK_NAME,
#endif
#if defined(WISUN_CONFIG_DEFAULT_PHY_FAN10)
  .phy = {
    .type = SL_WISUN_PHY_CONFIG_FAN10,
    .config.fan10.reg_domain = WISUN_CONFIG_REGULATORY_DOMAIN,
    .config.fan10.op_class = WISUN_CONFIG_OPERATING_CLASS,
    .config.fan10.op_mode = WISUN_CONFIG_OPERATING_MODE,
  },
#elif defined(WISUN_CONFIG_DEFAULT_PHY_FAN11)
  .phy = {
    .type = SL_WISUN_PHY_CONFIG_FAN11,
    .config.fan11.reg_domain = WISUN_CONFIG_REGULATORY_DOMAIN,
    .config.fan11.chan_plan_id = WISUN_CONFIG_CHANNEL_PLAN_ID,
    .config.fan11.phy_mode_id = WISUN_CONFIG_PHY_MODE_ID,
  },
#else
  .phy = { 0 },
#endif
#if defined(WISUN_CONFIG_NETWORK_SIZE)
  .network_size = WISUN_CONFIG_NETWORK_SIZE,
#else
  .network_size = SL_WISUN_NETWORK_SIZE_SMALL,
#endif
#if defined(WISUN_CONFIG_TX_POWER)
  .tx_power_ddbm = WISUN_CONFIG_TX_POWER,
#else
  .tx_power_ddbm = 200,
#endif
#if defined(WISUN_CONFIG_MODE_SWITCH_PHYS)
  .rx_phy_mode_ids = WISUN_CONFIG_MODE_SWITCH_PHYS,
#else
  .rx_phy_mode_ids = { 0 },
#endif
#if defined(WISUN_CONFIG_MODE_SWITCH_PHYS_NUMBER)
  .rx_phy_mode_ids_count = WISUN_CONFIG_MODE_SWITCH_PHYS_NUMBER,
#else
  .rx_phy_mode_ids_count = 0,
#endif
#if defined(WISUN_CONFIG_DEVICE_PROFILE)
  .lfn_profile = WISUN_CONFIG_DEVICE_PROFILE,
#else
  .lfn_profile = SL_WISUN_LFN_PROFILE_TEST,
#endif
#if defined(WISUN_CONFIG_ALLOWED_CHANNELS)
  .allowed_channels = WISUN_CONFIG_ALLOWED_CHANNELS,
#else
  .allowed_channels = "0-255",
#endif
  .uc_dwell_interval_ms = SL_WISUN_BR_CONFIG_UC_DWELL_INTERVAL,
  .bc_interval_ms = SL_WISUN_BR_CONFIG_BC_INTERVAL,
  .bc_dwell_interval_ms = SL_WISUN_BR_CONFIG_BC_DWELL_INTERVAL,
  .ipv6_prefix = SL_WISUN_BR_CONFIG_IPV6_PREFIX,
  .max_neighbor_count = SL_WISUN_BR_CONFIG_MAX_NEIGHBOR_COUNT,
  .max_child_count = SL_WISUN_BR_CONFIG_MAX_CHILD_COUNT,
  .max_security_neighbor_count = SL_WISUN_BR_CONFIG_MAX_SECURITY_NEIGHBOR_COUNT,
  .keychain = SL_WISUN_BR_CONFIG_KEYCHAIN,
  .keychain_index = SL_WISUN_BR_CONFIG_KEYCHAIN_INDEX,
  .socket_rx_buffer_size = SL_WISUN_BR_CONFIG_SOCKET_RX_BUFFER_SIZE,
  .fec = 0,
  .state = SL_WISUN_BR_STATE_INITIALIZED,
  .is_default_phy = true,
  .pan_id = 0U
};

#if defined(SL_CATALOG_WISUN_BR_WIFI_PRESENT)
static const app_setting_wifi_t _wisun_wifi_settings_default = {
  .ssid = SL_WISUN_BR_CONFIG_WIFI_DEFAULT_SSID,
  .passphrase = SL_WISUN_BR_CONFIG_WIFI_DEFAULT_PASSPHRASE,
  .security_type = SL_WISUN_BR_CONFIG_WIFI_DEFAULT_SECURITY_TYPE
};
#endif

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------
/// Dummy Wi-SUN application settings for keep nvm space when BR settings are used
app_setting_wisun_t dummy_wisun_app_settings = {
  #if defined(WISUN_CONFIG_NETWORK_NAME)
  .network_name = WISUN_CONFIG_NETWORK_NAME,
#else
  .network_name = APP_SETTINGS_DEFAULT_NETWORK_NAME,
#endif
#if defined(WISUN_CONFIG_NETWORK_SIZE)
  .network_size = WISUN_CONFIG_NETWORK_SIZE,
#else
  .network_size = SL_WISUN_NETWORK_SIZE_SMALL,
#endif
#if defined(WISUN_CONFIG_TX_POWER)
  .tx_power_ddbm = WISUN_CONFIG_TX_POWER,
#else
  .tx_power_ddbm = 200,
#endif
  .is_default_phy = true,
#if defined(WISUN_CONFIG_DEVICE_TYPE)
  .device_type = WISUN_CONFIG_DEVICE_TYPE,
#else
  .device_type = SL_WISUN_ROUTER,
#endif
#if defined(WISUN_CONFIG_DEVICE_PROFILE)
  .lfn_profile = WISUN_CONFIG_DEVICE_PROFILE,
#else
  .lfn_profile = SL_WISUN_LFN_PROFILE_TEST,
#endif
#if defined(WISUN_CONFIG_DEFAULT_PHY_FAN10)
  .phy = {
    .type = SL_WISUN_PHY_CONFIG_FAN10,
    .config.fan10.reg_domain = WISUN_CONFIG_REGULATORY_DOMAIN,
    .config.fan10.op_class = WISUN_CONFIG_OPERATING_CLASS,
    .config.fan10.op_mode = WISUN_CONFIG_OPERATING_MODE,
  },
#elif defined(WISUN_CONFIG_DEFAULT_PHY_FAN11)
  .phy = {
    .type = SL_WISUN_PHY_CONFIG_FAN11,
    .config.fan11.reg_domain = WISUN_CONFIG_REGULATORY_DOMAIN,
    .config.fan11.chan_plan_id = WISUN_CONFIG_CHANNEL_PLAN_ID,
    .config.fan11.phy_mode_id = WISUN_CONFIG_PHY_MODE_ID,
  },
#else
  .phy = { 0 },
#endif
};

/// Wi-SUN application network saving settings
const app_saving_item_t dummy_network_saving_settings = {
  .data = &dummy_wisun_app_settings,
  .data_size = sizeof(dummy_wisun_app_settings),
  .default_val = &dummy_wisun_app_settings
};

/// Wi-SUN border router network saving settings
const app_saving_item_t br_saving_settings = {
  .data = &_wisun_br_settings,
  .data_size = sizeof(_wisun_br_settings),
  .default_val = &_wisun_br_settings_default
};

#if defined(SL_CATALOG_WISUN_BR_WIFI_PRESENT)
/// Wi-SUN border router network saving settings
const app_saving_item_t wifi_saving_settings = {
  .data = &_wisun_wifi_settings,
  .data_size = sizeof(_wisun_wifi_settings),
  .default_val = &_wisun_wifi_settings_default
};
#endif

/// Wi-SUN application all saved data
const app_saving_item_t *saving_settings[] = {
  &dummy_network_saving_settings,
  &br_saving_settings,
#if defined(SL_CATALOG_WISUN_BR_WIFI_PRESENT)
  &wifi_saving_settings,
#endif
  NULL
};
// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------

/* Initialization of Wi-SUN setting */
void app_wisun_setting_init(void)
{
  // init wisun settings mutex
  _wisun_setting_br_mtx = osMutexNew(&_wisun_setting_br_mtx_attr);
  EFM_ASSERT(_wisun_setting_br_mtx != NULL);
}

/* Get br setting */
sl_status_t app_wisun_setting_br_get(app_setting_br_t *const wisun_setting)
{
  if (wisun_setting == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  // create a deep copy of setting
  _wisun_mutex_br_acquire();
  memcpy(wisun_setting, &_wisun_br_settings, sizeof(app_setting_br_t));
  _wisun_mutex_br_release();

  return SL_STATUS_OK;
}

#if defined(SL_CATALOG_WISUN_BR_WIFI_PRESENT)
/* Get wifi setting */
sl_status_t app_wisun_setting_wifi_get(app_setting_wifi_t *const wisun_setting)
{
  if (wisun_setting == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  // create a deep copy of setting
  _wisun_mutex_br_acquire();
  memcpy(wisun_setting, &_wisun_wifi_settings, sizeof(app_setting_wifi_t));
  _wisun_mutex_br_release();

  return SL_STATUS_OK;
}

sl_status_t app_wisun_setting_set_ssid(const uint8_t *const ssid)
{
  const uint8_t *wifi_ssid = NULL;
  size_t ssid_len = 0U;
  sl_status_t stat = SL_STATUS_FAIL;

  if (ssid == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  _wisun_mutex_br_acquire();
  wifi_ssid = _app_check_ssid(ssid, &ssid_len);

  if (ssid_len < SL_WISUN_WIFI_SSID_SIZE + 1U) {
    memcpy(_wisun_wifi_settings.ssid, wifi_ssid, ssid_len);
    _wisun_wifi_settings.ssid[ssid_len] = 0U;
  } else {
    _wisun_mutex_br_release();
    return SL_STATUS_FAIL;
  }

  stat = app_wisun_setting_notify(APP_SETTING_NOTIFICATION_SET_SSID);
  _wisun_mutex_br_release();

  return stat;
}

sl_status_t app_wisun_setting_get_ssid(uint8_t *const ssid, uint8_t size)
{
  uint8_t ssid_len = 0U;

  if ((ssid == NULL) || (size < SL_WISUN_WIFI_SSID_SIZE + 1U)) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  _wisun_mutex_br_acquire();

  ssid_len = (uint8_t)sl_strlen((char *)_wisun_wifi_settings.ssid);

  if (ssid_len < SL_WISUN_WIFI_SSID_SIZE + 1U) {
    memset(ssid, 0U, SL_WISUN_WIFI_SSID_SIZE + 1U);
    memcpy(ssid, _wisun_wifi_settings.ssid, ssid_len);
  } else {
    _wisun_mutex_br_release();
    return SL_STATUS_FAIL;
  }

  _wisun_mutex_br_release();
  return SL_STATUS_OK;
}

sl_status_t app_wisun_setting_set_passphrase(const uint8_t *const passphrase)
{
  const uint8_t *wifi_passphrase = NULL;
  size_t passphrase_len = 0U;
  sl_status_t stat = SL_STATUS_FAIL;

  if (passphrase == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  _wisun_mutex_br_acquire();
  wifi_passphrase = _app_check_passphrase(passphrase, &passphrase_len);

  if (passphrase_len < SL_WISUN_WIFI_PASSPHRASE_SIZE + 1U) {
    memcpy(_wisun_wifi_settings.passphrase, wifi_passphrase, passphrase_len);
    _wisun_wifi_settings.passphrase[passphrase_len] = 0U;
  } else {
    _wisun_mutex_br_release();
    return SL_STATUS_FAIL;
  }

  stat = app_wisun_setting_notify(APP_SETTING_NOTIFICATION_SET_PASSPHRASE);
  _wisun_mutex_br_release();

  return stat;
}

sl_status_t app_wisun_setting_get_passphrase(uint8_t *const passphrase, uint8_t size)
{
  uint8_t passphrase_len = 0U;

  if ((passphrase == NULL) || (size < SL_WISUN_WIFI_PASSPHRASE_SIZE + 1U)) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  _wisun_mutex_br_acquire();

  passphrase_len = (uint8_t)sl_strlen((char *)_wisun_wifi_settings.passphrase);

  if (passphrase_len < SL_WISUN_WIFI_PASSPHRASE_SIZE + 1U) {
    memset(passphrase, 0U, SL_WISUN_WIFI_PASSPHRASE_SIZE + 1U);
    memcpy(passphrase, _wisun_wifi_settings.passphrase, passphrase_len);
  } else {
    _wisun_mutex_br_release();
    return SL_STATUS_FAIL;
  }

  _wisun_mutex_br_release();
  return SL_STATUS_OK;
}

sl_status_t app_wisun_setting_set_security(const uint8_t *const security)
{
  sl_status_t stat = SL_STATUS_FAIL;

  if (security == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  _wisun_mutex_br_acquire();
  _wisun_wifi_settings.security_type = *security;
  stat = app_wisun_setting_notify(APP_SETTING_NOTIFICATION_SET_SECURITY);
  _wisun_mutex_br_release();

  return stat;
}

sl_status_t app_wisun_setting_get_security(uint8_t *const security)
{
  if (security == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  _wisun_mutex_br_acquire();
  *security = _wisun_wifi_settings.security_type;
  _wisun_mutex_br_release();

  return SL_STATUS_OK;
}
#endif

/* Setting Wi-SUN network name */
sl_status_t app_wisun_setting_set_network_name(const char *const name)
{
  const char *network_name = NULL;
  size_t name_len = 0U;
  sl_status_t stat = SL_STATUS_FAIL;

  if (name == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  _wisun_mutex_br_acquire();
  // check the network name, and return checked name.
  network_name = _app_check_nw_name(name, &name_len);

  if (name_len < APP_SETTING_NETWORK_NAME_MAX_SIZE) {
    memcpy(_wisun_br_settings.network_name, network_name, name_len);
    _wisun_br_settings.network_name[name_len] = 0U;
  } else {
    _wisun_mutex_br_release();
    return SL_STATUS_FAIL;
  }

  stat = app_wisun_setting_notify(APP_SETTING_NOTIFICATION_SET_NETWORK_NAME);
  _wisun_mutex_br_release();

  return stat;
}

/* Setting Wi-SUN network size */
sl_status_t app_wisun_setting_set_network_size(const uint8_t *const size)
{
  sl_status_t stat = SL_STATUS_FAIL;

  if (size == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  _wisun_mutex_br_acquire();
  _wisun_br_settings.network_size = *size;
  stat = app_wisun_setting_notify(APP_SETTING_NOTIFICATION_SET_NETWORK_SIZE);
  _wisun_mutex_br_release();

  return stat;
}

/* Setting BR settings */
sl_status_t app_wisun_setting_br_set(const app_setting_br_t *const settings)
{
  sl_status_t stat = SL_STATUS_FAIL;

  if (settings == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  _wisun_mutex_br_acquire();
  memcpy(&_wisun_br_settings, settings, sizeof(app_setting_br_t));
  stat = app_wisun_setting_notify(APP_SETTING_NOTIFICATION_SET_BR_SETTINGS);
  _wisun_mutex_br_release();

  return stat;
}

/* Setting Wi-SUN TX power */
sl_status_t app_wisun_setting_set_tx_power(const int16_t * const tx_power)
{
  sl_status_t stat = SL_STATUS_FAIL;

  if (tx_power == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  _wisun_mutex_br_acquire();
  _wisun_br_settings.tx_power_ddbm = *tx_power;
  stat = app_wisun_setting_notify(APP_SETTING_NOTIFICATION_SET_TX_POWER);
  _wisun_mutex_br_release();

  return stat;
}

/* Setting Wi-SUN PAN ID */
sl_status_t app_wisun_setting_set_pan_id(const uint16_t *const pan_id)
{
  if (pan_id == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  _wisun_mutex_br_acquire();
  _wisun_br_settings.pan_id = *pan_id;
  _wisun_mutex_br_release();

  return SL_STATUS_OK;
}

/* Setting Wi-SUN PHY */
sl_status_t app_wisun_setting_set_phy(const sl_wisun_phy_config_t *const phy)
{
  sl_status_t stat = SL_STATUS_OK;

  if (phy == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  _wisun_mutex_br_acquire();
  _wisun_br_settings.is_default_phy = false;
  memcpy(&_wisun_br_settings.phy, phy, sizeof(sl_wisun_phy_config_t));
  _wisun_mutex_br_release();

  if (app_wisun_setting_notify(APP_SETTING_NOTIFICATION_SET_PHY_CFG) != SL_STATUS_OK) {
    printf("[Failed: unable to set PHY config\n");
    stat = SL_STATUS_FAIL;
  }

  if (app_wisun_setting_notify(APP_SETTING_NOTIFICATION_SET_BR_SETTINGS) !=  SL_STATUS_OK) {
    printf("[Failed: unable to set BR settings\n");
    stat = SL_STATUS_FAIL;
  }

  return stat;
}

/* Getting network name */
sl_status_t app_wisun_setting_get_network_name(char *const name, uint8_t size)
{
  uint8_t name_len = 0U;

  if ((name == NULL) || (size < APP_SETTING_NETWORK_NAME_MAX_SIZE)) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  _wisun_mutex_br_acquire();

  name_len = (uint8_t)sl_strlen(_wisun_br_settings.network_name);

  if (name_len < APP_SETTING_NETWORK_NAME_MAX_SIZE) {
    memset(name, 0U, APP_SETTING_NETWORK_NAME_MAX_SIZE);
    memcpy(name, _wisun_br_settings.network_name, name_len);
  } else {
    _wisun_mutex_br_release();
    return SL_STATUS_FAIL;
  }

  _wisun_mutex_br_release();
  return SL_STATUS_OK;
}

/* Getting network size */
sl_status_t app_wisun_setting_get_network_size(uint8_t *const size)
{
  if (size == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  _wisun_mutex_br_acquire();
  *size = _wisun_br_settings.network_size;
  _wisun_mutex_br_release();

  return SL_STATUS_OK;
}

/* Getting TX power */
sl_status_t app_wisun_setting_get_tx_power(int16_t * const tx_power)
{
  if (tx_power == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  _wisun_mutex_br_acquire();
  *tx_power = _wisun_br_settings.tx_power_ddbm;
  _wisun_mutex_br_release();

  return SL_STATUS_OK;
}

/* Getting PHY */
sl_status_t app_wisun_setting_get_phy(sl_wisun_phy_config_t *const phy)
{
  if (phy == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  _wisun_mutex_br_acquire();
  memcpy(phy, &_wisun_br_settings.phy, sizeof(sl_wisun_phy_config_t));
  _wisun_mutex_br_release();

  return SL_STATUS_OK;
}

sl_status_t app_wisun_setting_init_phy_cfg(void)
{
  sl_status_t stat = SL_STATUS_OK;

  // Full radio config: create a copy of already prepared phy config with default settings
  if (_wisun_br_settings.is_default_phy) {
    memcpy(&_wisun_br_settings.phy, &_wisun_br_settings_default.phy, sizeof(sl_wisun_phy_config_t));
  }

  // Set notifications
  if (app_wisun_setting_notify(APP_SETTING_NOTIFICATION_SET_PHY_CFG) != SL_STATUS_OK) {
    printf("[Failed: unable to init PHY config\n");
    stat = SL_STATUS_FAIL;
  }

  if (app_wisun_setting_notify(APP_SETTING_NOTIFICATION_SET_BR_SETTINGS) !=  SL_STATUS_OK) {
    printf("[Failed: unable to init BR settings\n");
    stat = SL_STATUS_FAIL;
  }

  return stat;
}

sl_status_t app_settings_get_channel_mask(const char *str, sl_wisun_channel_mask_t *channel_mask)
{
  return _app_ranges_to_mask(str, channel_mask->mask, SL_WISUN_CHANNEL_MASK_SIZE);
}
// -----------------------------------------------------------------------------
//                          Static Function Definitions
// -----------------------------------------------------------------------------

/* Mutex acquire */
__STATIC_INLINE void _wisun_mutex_br_acquire(void)
{
  EFM_ASSERT(osMutexAcquire(_wisun_setting_br_mtx, osWaitForever) == osOK);
}

/* Mutex release */
__STATIC_INLINE void _wisun_mutex_br_release(void)
{
  EFM_ASSERT(osMutexRelease(_wisun_setting_br_mtx) == osOK);
}

static const char* _app_check_nw_name(const char *name, size_t *const name_len)
{
  const char* ret_name = APP_SETTINGS_DEFAULT_NETWORK_NAME;

  *name_len = sl_strnlen((char*)name, SL_WISUN_NETWORK_NAME_SIZE);
  if (!(*name_len < SL_WISUN_NETWORK_NAME_SIZE) || (*name_len == 0) ) {
    // set the default name size
    *name_len = sl_strnlen(APP_SETTINGS_DEFAULT_NETWORK_NAME, SL_WISUN_NETWORK_NAME_SIZE);
    printf("\r\n[Warning: The name of Wi-SUN network is incorrect, default name used, \"%s\" ]\r\n", ret_name);
  } else {
    ret_name = name;
  }

  return ret_name;
}

#if defined(SL_CATALOG_WISUN_BR_WIFI_PRESENT)
static const uint8_t* _app_check_ssid(const uint8_t *ssid, size_t *const ssid_len)
{
  const uint8_t *ret_ssid = (uint8_t *)APP_SETTINGS_DEFAULT_WIFI_SSID;

  *ssid_len = sl_strnlen((char*)ssid, SL_WISUN_WIFI_SSID_SIZE);
  if (!(*ssid_len < SL_WISUN_WIFI_SSID_SIZE) || (*ssid_len == 0) ) {
    // set the default SSID size
    *ssid_len = sl_strnlen((char *)APP_SETTINGS_DEFAULT_WIFI_SSID, SL_WISUN_WIFI_SSID_SIZE);
    printf("\n[Warning: The SSID of Wifi is incorrect, default SSID used, \"%s\" ]\n", ret_ssid);
  } else {
    ret_ssid = ssid;
  }

  return ret_ssid;
}

static const uint8_t* _app_check_passphrase(const uint8_t *passphrase, size_t *const passphrase_len)
{
  const uint8_t* ret_passphrase = (uint8_t *)APP_SETTINGS_DEFAULT_WIFI_PASSPHRASE;

  *passphrase_len = sl_strnlen((char*)passphrase, SL_WISUN_WIFI_PASSPHRASE_SIZE);
  if (!(*passphrase_len < SL_WISUN_WIFI_PASSPHRASE_SIZE) || (*passphrase_len == 0) ) {
    // set the default passphrase size
    *passphrase_len = sl_strnlen((char *)APP_SETTINGS_DEFAULT_WIFI_PASSPHRASE, SL_WISUN_WIFI_PASSPHRASE_SIZE);
    printf("\n[Warning: The passphrase of Wifi is incorrect, default passphrase used, \"%s\" ]\n", ret_passphrase);
  } else {
    ret_passphrase = passphrase;
  }

  return ret_passphrase;
}
#endif

static sl_status_t _app_ranges_to_mask(const char *str, uint8_t *mask, uint32_t size)
{
  char *endptr = NULL;
  uint32_t cur = 0U;
  uint32_t end = 0U;
  uint32_t index = 0U;

  memset(mask, 0U, size * sizeof(uint8_t));

  do {
    if (*str == '\0') {
      return SL_STATUS_FAIL;
    }
    cur = strtoul(str, &endptr, 0);
    if (*endptr == '-') {
      str = endptr + 1;
      end = strtoul(str, &endptr, 0);
    } else {
      end = cur;
    }
    if (*endptr != '\0' && *endptr != ',') {
      return SL_STATUS_FAIL;
    }
    if (cur > end) {
      return SL_STATUS_FAIL;
    }
    for (; cur <= end; cur++) {
      index = cur / 8;
      if (index < size) {
        mask[index] |= 1 << (cur % 8);
      } else {
        return SL_STATUS_FAIL;
      }
    }
    str = endptr + 1;
  } while (*endptr != '\0');

  return SL_STATUS_OK;
}
