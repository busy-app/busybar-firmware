/***************************************************************************//**
 * @file sl_wisun_app_br_core.c
 * @brief Wi-SUN Application Border Router Core
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
#include <stdio.h>
#include <string.h>

#include "sl_assert.h"
#include "sl_status.h"
#include "sl_wisun_app_core.h"
#include "sl_wisun_app_br_core.h"
#include "sl_wisun_app_core_config.h"
#include "cmsis_os2.h"
#include "sl_cmsis_os2_common.h"
#include "sl_wisun_types.h"
#include "sl_wisun_api.h"
#include "sl_wisun_config.h"
#include "sl_wisun_event_mgr.h"
#include "sl_wisun_trace_util.h"
#include "sl_sleeptimer.h"
#include "sl_memory_manager.h"
#include "sl_component_catalog.h"
#include "border_router/sl_wisun_br_api.h"
#include "sl_wisun_br_dhcpv6_server.h"
#include "sl_wisun_keychain.h"

#if defined(SL_CATALOG_WISUN_BR_AGENT_SERVICE_PRESENT)
#include "sl_wisun_br_agent_service.h"
#endif

#if defined(SL_CATALOG_WISUN_APP_SETTING_PRESENT)
#include "sl_wisun_app_setting_br.h"
#endif

#if defined(SL_CATALOG_WISUN_BR_LWIP_PRESENT)
#include "sl_wisun_br_lwip.h"
#endif
// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

/// Eventflag error mask
#define APP_WISUN_EVTFLAG_ERROR_MSK                       (0x00000001UL << 31UL)

/// Wifi SSID size
#define SL_WISUN_APP_BR_CORE_WIFI_SSID_SIZE               (32)

/// Wifi Passphrase size
#define SL_WISUN_APP_BR_CORE_WIFI_PASSPHRASE_SIZE         (32)

/// Size of the printable data buffer
#define SL_WISUN_APP_BR_CORE_PRINTABLE_DATA_MAX_LENGTH    (64)

/// Size of the IPv6 prefix
#define SL_WISUN_APP_BR_CORE_IPV6_PREFIX_SIZE             (43)

/// Release mutex and return
#define _return_and_mtx_release() \
  do {                            \
    _app_wisun_mutex_release();   \
    return;                       \
  } while (0)

/// Release mutex and return value
#define _return_val_and_mtx_release(retval) \
  do {                                      \
    _app_wisun_mutex_release();             \
    return (retval);                        \
  } while (0)

/// Synthetize br settings if it is not available
#if !defined(SL_CATALOG_WISUN_APP_SETTING_PRESENT)
SL_PACK_START(1)
typedef struct app_setting_br {
  /// Network Name
  char network_name[SL_WISUN_NETWORK_NAME_SIZE + 1];
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
  char allowed_channels[SL_WISUN_APP_BR_CORE_PRINTABLE_DATA_MAX_LENGTH + 1];
  /// IPv6 prefix
  char ipv6_prefix[SL_WISUN_APP_BR_CORE_IPV6_PREFIX_SIZE + 1];
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
#endif

/// Synthetize wifi settings if it is not available
#if !defined(SL_CATALOG_WISUN_APP_SETTING_PRESENT) \
  && defined(SL_CATALOG_WISUN_BR_WIFI_PRESENT)
typedef struct app_setting_wifi {
  /// SSID
  uint8_t ssid[SL_WISUN_APP_BR_CORE_WIFI_SSID_SIZE + 1];
  /// Passphrase
  uint8_t passphrase[SL_WISUN_APP_BR_CORE_WIFI_PASSPHRASE_SIZE + 1];
  /// Security type
  uint8_t security_type;
} app_setting_wifi_t;
#endif

// -----------------------------------------------------------------------------
//                          Static Function Declarations
// -----------------------------------------------------------------------------

/**************************************************************************//**
 * @brief Acquire application mutex
 * @details Internal mutex lock
 *****************************************************************************/
__STATIC_INLINE void _app_wisun_mutex_acquire(void);

/**************************************************************************//**
 * @brief Release application mutex
 * @details Internal mutex release
 *****************************************************************************/
__STATIC_INLINE void _app_wisun_mutex_release(void);

/**************************************************************************//**
 * @brief Setting state flag
 * @details It sets the state by a flag
 * @param[in] flag is a flag bit
 *****************************************************************************/
__STATIC_INLINE void _app_wisun_core_set_state(const sl_wisun_app_core_state_t flag);

// -----------------------------------------------------------------------------
//                                Static Variables
// -----------------------------------------------------------------------------

/// Create default setting if br settings are not available
#if !defined(SL_CATALOG_WISUN_APP_SETTING_PRESENT)
static const app_setting_br_t _br_default_settings = {
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
#endif

/// Create default setting if wifi settings are not available
#if !defined(SL_CATALOG_WISUN_APP_SETTING_PRESENT) \
  && defined(SL_CATALOG_WISUN_BR_WIFI_PRESENT)
static const app_setting_wifi_t _wifi_default_settings = {
  .ssid = SL_WISUN_BR_CONFIG_WIFI_DEFAULT_SSID,
  .passphrase = SL_WISUN_BR_CONFIG_WIFI_DEFAULT_PASSPHRASE
                .security_type = SL_WISUN_BR_CONFIG_WIFI_DEFAULT_SECURITY_TYPE
};
#endif

/// App framework mutex
static osMutexId_t _app_core_mtx = NULL;

/// App framework mutex attribute
static const osMutexAttr_t _app_wisun_mtx_attr = {
  .name      = "AppWisunMutex",
  .attr_bits = osMutexRecursive,
  .cb_mem    = NULL,
  .cb_size   = 0
};

/// Error flag for errors
static osEventFlagsId_t _app_core_state = NULL;

/// State event flags attributes
static const osEventFlagsAttr_t _app_wisun_evt_attr = {
  .name      = "AppWisunEvtFlags",
  .attr_bits = 0,
  .cb_mem    = NULL,
  .cb_size   = 0
};

/// Internal br setting storage
static app_setting_br_t _br_setting = { 0U };

/// DHCPv6 server socket
SL_WEAK int app_dhcpv6_socket = SOCKET_INVALID_ID;

// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------

/* Network update event handler */
void sl_wisun_network_update_event_hnd(sl_wisun_evt_t *evt)
{
  __CHECK_FOR_STATUS(evt->evt.error.status);
}

/* Connected event handler */
void sl_wisun_connected_event_hnd(sl_wisun_evt_t *evt)
{
  __CHECK_FOR_STATUS(evt->evt.error.status);
}

/* Disconnected event handler */
void sl_wisun_disconnected_event_hnd(sl_wisun_evt_t *evt)
{
  __CHECK_FOR_STATUS(evt->evt.error.status);
}

/* Connection lost event handler */
void sl_wisun_connection_lost_event_hnd(sl_wisun_evt_t *evt)
{
  __CHECK_FOR_STATUS(evt->evt.error.status);
}

/* Error event handler */
void sl_wisun_error_event_hnd(sl_wisun_evt_t *evt)
{
  printf("[Wi-SUN network error occurred. Status: %lu\n",
         evt->evt.error.status);
}

/* Join state event handler */
void sl_wisun_join_state_event_hnd(sl_wisun_evt_t *evt)
{
#if defined(SL_CATALOG_WISUN_BR_LWIP_PRESENT)
  sl_wisun_ip_address_t ll_addr = { 0 };
  sl_wisun_ip_address_t gua_addr = { 0 };
  sl_wisun_ip_address_t dodagid_addr = { 0 };

  if (evt->evt.join_state.join_state == SL_WISUN_JOIN_STATE_OPERATIONAL) {
    if (sl_wisun_br_get_ip_addresses(ll_addr.address, gua_addr.address, dodagid_addr.address) != SL_STATUS_OK) {
      printf("[app_handle_join_state_ind: sl_wisun_br_get_ip_addresses failed]\n");
      return;
    }
    sl_wisun_br_lwip_pan_up(ll_addr.address, gua_addr.address);
  } else {
    sl_wisun_br_lwip_pan_down();
  }
#endif
  __CHECK_FOR_STATUS(evt->evt.error.status);
}

void sl_wisun_lfn_wake_up_hnd(sl_wisun_evt_t *evt)
{
  __CHECK_FOR_STATUS(evt->evt.error.status);
}

void sl_wisun_multicast_reg_finish_hnd(sl_wisun_evt_t *evt)
{
  __CHECK_FOR_STATUS(evt->evt.error.status);
}

void sl_wisun_dhcp_vendor_data_hnd(sl_wisun_evt_t *evt)
{
  __CHECK_FOR_STATUS(evt->evt.error.status);
}

void sl_wisun_pan_defect_hnd(sl_wisun_evt_t *evt)
{
  __CHECK_FOR_STATUS(evt->evt.error.status);
}

void sl_wisun_direct_connect_link_available_hnd(sl_wisun_evt_t *evt)
{
  __CHECK_FOR_STATUS(evt->evt.error.status);
}

void sl_wisun_direct_connect_status_hnd(sl_wisun_evt_t *evt)
{
  __CHECK_FOR_STATUS(evt->evt.error.status);
}

void sl_wisun_br_stopped_hnd(sl_wisun_evt_t *evt)
{
  __CHECK_FOR_STATUS(evt->evt.error.status);
}

void sl_wisun_mode_switch_fallback_hnd(sl_wisun_evt_t *evt)
{
  __CHECK_FOR_STATUS(evt->evt.error.status);
}

void sl_wisun_regulation_tx_level_hnd(sl_wisun_evt_t *evt)
{
  __CHECK_FOR_STATUS(evt->evt.error.status);
}

void sl_wisun_br_routing_table_update_hnd(sl_wisun_evt_t *evt)
{
#if defined(SL_CATALOG_WISUN_BR_AGENT_SERVICE_PRESENT)
  if (evt->evt.br_routing_table_update.event
      == SL_WISUN_ROUTING_TABLE_UPDATE_ROUTE_CHANGED) {
    (void) sl_wisun_br_agent_service_send_graph_info();
  }
#endif
  printf("[Routing table update: route changed]\n");
  __CHECK_FOR_STATUS(evt->evt.error.status);
}

/* Wi-SUN app core init */
void sl_wisun_app_core_init(void)
{
  // init wisun network mutex
  _app_core_mtx = osMutexNew(&_app_wisun_mtx_attr);
  EFM_ASSERT(_app_core_mtx != NULL);

  // init wisun event flags
  _app_core_state = osEventFlagsNew(&_app_wisun_evt_attr);
  EFM_ASSERT(_app_core_state != NULL);
}

/* App core get state */
sl_status_t sl_wisun_app_core_get_state(uint32_t * const state)
{
  *state = osEventFlagsGet(_app_core_state);
  // Check error flag
  if (*state & APP_WISUN_EVTFLAG_ERROR_MSK) {
    return SL_STATUS_FAIL;
  }
  return SL_STATUS_OK;
}

/* App core wait state */
sl_status_t sl_wisun_app_core_wait_state(const uint32_t state, const uint32_t timeout)
{
  uint32_t ret = 0UL;
  ret = osEventFlagsWait(_app_core_state, state, osFlagsWaitAll | osFlagsNoClear, timeout);
  return (ret & APP_WISUN_EVTFLAG_ERROR_MSK) ? SL_STATUS_FAIL : SL_STATUS_OK;
}

void sl_wisun_app_br_core_start(void)
{
  sl_wisun_channel_mask_t channel_mask = { 0 };
  sl_wisun_phy_config_t phy_config = { 0 };
  sl_wisun_br_connection_params_t params = { 0 };
  sl_wisun_br_lfn_params_t lfn_params = { 0 };
  uint8_t ipv6_prefix[IPV6_ADDR_SIZE] = { 0U };
  int16_t ipv6_prefix_length = 0;
  uint8_t trustedca_count = 0U;
  sl_wisun_keychain_entry_t *trustedca = NULL;
  uint16_t certificate_options = 0U;
  sl_wisun_keychain_credential_t *credential = NULL;
  sl_wisun_mac_address_t address = { 0 };
  sl_wisun_network_info_t app_network_info;
#if defined(SL_CATALOG_WISUN_APP_SETTING_PRESENT)
  uint16_t pan_id = 0U;
#endif

  _app_wisun_mutex_acquire();
#if defined(SL_CATALOG_WISUN_APP_SETTING_PRESENT)
  // Init app PHY config
  if (app_wisun_setting_init_phy_cfg() != SL_STATUS_OK) {
    printf("[Failed: unable to init PHY config\n");
    _app_wisun_core_set_state(SL_WISUN_APP_CORE_STATE_SETTING_ERROR);
    _return_and_mtx_release();
  }

  // get full settings (PHY, network name, network size and TX power)
  if (app_wisun_setting_br_get(&_br_setting) != SL_STATUS_OK) {
    printf("[Failed: unable to get settings\n");
    _app_wisun_core_set_state(SL_WISUN_APP_CORE_STATE_SETTING_ERROR);
    _return_and_mtx_release();
  }
#else
  memcpy(&_br_setting, &_br_default_settings, sizeof(app_setting_br_t));
#endif

  // Set Device Type
  EFM_ASSERT(sl_wisun_set_device_type(SL_WISUN_BORDER_ROUTER) == SL_STATUS_OK);

  // Set TX Power
  EFM_ASSERT(sl_wisun_set_tx_power_ddbm(_br_setting.tx_power_ddbm) == SL_STATUS_OK);

  // Set Connection Parameters
  switch (_br_setting.network_size) {
    case SL_WISUN_NETWORK_SIZE_SMALL:
      params = SL_WISUN_BR_PARAMS_PROFILE_SMALL;
      break;
    case SL_WISUN_NETWORK_SIZE_MEDIUM:
      params = SL_WISUN_BR_PARAMS_PROFILE_MEDIUM;
      break;
    case SL_WISUN_NETWORK_SIZE_LARGE:
      params = SL_WISUN_BR_PARAMS_PROFILE_LARGE;
      break;
    case SL_WISUN_NETWORK_SIZE_TEST:
      params = SL_WISUN_BR_PARAMS_PROFILE_TEST;
      break;
    default:
      EFM_ASSERT(0);
      break;
  }
  EFM_ASSERT(sl_wisun_br_set_connection_parameters(&params) == SL_STATUS_OK);

  // Set Neighbor Table
  EFM_ASSERT(sl_wisun_config_neighbor_table(_br_setting.max_child_count,
                                            _br_setting.max_neighbor_count,
                                            _br_setting.max_security_neighbor_count) == SL_STATUS_OK);

  EFM_ASSERT((sl_wisun_stoip6_prefix(_br_setting.ipv6_prefix, ipv6_prefix, &ipv6_prefix_length) >= 0)
             && (ipv6_prefix_length >= 0) && (ipv6_prefix_length <= 64));

  // Set IPv6 Prefix
  EFM_ASSERT(sl_wisun_br_set_ipv6_prefix(ipv6_prefix, (uint8_t)ipv6_prefix_length) == SL_STATUS_OK);

  // Set LFN Parameters
  switch (_br_setting.lfn_profile) {
    case SL_WISUN_LFN_PROFILE_TEST:
      lfn_params = SL_WISUN_BR_PARAMS_LFN_TEST;
      break;
    case SL_WISUN_LFN_PROFILE_BALANCED:
      lfn_params = SL_WISUN_BR_PARAMS_LFN_BALANCED;
      break;
    case SL_WISUN_LFN_PROFILE_ECO:
      lfn_params = SL_WISUN_BR_PARAMS_LFN_ECO;
      break;
    default:
      EFM_ASSERT(0);
      break;
  }

  EFM_ASSERT(sl_wisun_br_set_lfn_parameters(&lfn_params) == SL_STATUS_OK);

  // Get channel mask
  EFM_ASSERT(app_settings_get_channel_mask(_br_setting.allowed_channels,
                                           &channel_mask) == SL_STATUS_OK);

  // Set Channel Mask
  EFM_ASSERT(sl_wisun_set_channel_mask(&channel_mask) == SL_STATUS_OK);

  // Set Unicast Settings
  EFM_ASSERT(sl_wisun_set_unicast_settings(_br_setting.uc_dwell_interval_ms) == SL_STATUS_OK);

  // Set Broadcast Settings
  EFM_ASSERT(sl_wisun_br_set_broadcast_settings(_br_setting.bc_interval_ms,
                                                _br_setting.bc_dwell_interval_ms) == SL_STATUS_OK);

  // Get trusted CA count
  trustedca_count = sl_wisun_keychain_get_trustedca_count();
  EFM_ASSERT(trustedca_count > 0U);

  certificate_options = SL_WISUN_CERTIFICATE_OPTION_IS_REF;
  for (uint8_t idx = 0U; idx < trustedca_count; ++idx) {
    trustedca = sl_wisun_keychain_get_trustedca(idx);
    EFM_ASSERT(trustedca != NULL);
    if (trustedca->keychain == SL_WISUN_KEYCHAIN_NVM) {
      printf("[Using NVM trusted CA #%u]\n", idx);
    } else if (trustedca->keychain == SL_WISUN_KEYCHAIN_BUILTIN) {
      printf("[Using built-in trusted CA #%u]\n", idx);
    }
    // Set Trusted CA
    EFM_ASSERT(sl_wisun_set_trusted_certificate(certificate_options,
                                                trustedca->data_length,
                                                trustedca->data) == SL_STATUS_OK);
    sl_free(trustedca);
    trustedca = NULL;
    certificate_options |= SL_WISUN_CERTIFICATE_OPTION_APPEND;
  }

  // Retrieve a device credential
  credential = sl_wisun_keychain_get_credential((sl_wisun_keychain_t)_br_setting.keychain,
                                                _br_setting.keychain_index);
  EFM_ASSERT(credential != NULL);
  if (credential->certificate.keychain == SL_WISUN_KEYCHAIN_NVM) {
    printf("[Using NVM device credentials]\n");
  } else if (credential->certificate.keychain == SL_WISUN_KEYCHAIN_BUILTIN) {
    printf("[Using built-in device credentials]\n");
  }

  // Set Device Certificate
  EFM_ASSERT(sl_wisun_set_br_device_certificate(SL_WISUN_CERTIFICATE_OPTION_IS_REF | SL_WISUN_CERTIFICATE_OPTION_HAS_KEY,
                                                credential->certificate.data_length,
                                                credential->certificate.data) == SL_STATUS_OK);
  if (credential->pk.type == SL_WISUN_KEYCHAIN_KEY_TYPE_PLAINTEXT) {
    // Set Device Private Key
    EFM_ASSERT(sl_wisun_set_device_private_key(SL_WISUN_PRIVATE_KEY_OPTION_IS_REF,
                                               credential->pk.u.plaintext.data_length,
                                               credential->pk.u.plaintext.data) == SL_STATUS_OK);
  } else {
    // Set Device Private Key ID
    EFM_ASSERT(sl_wisun_set_device_private_key_id(credential->pk.u.key_id) == SL_STATUS_OK);
  }

  sl_free(credential);
  credential = NULL;

  phy_config.config.fan11.reg_domain = _br_setting.phy.config.fan11.reg_domain;
  phy_config.config.fan11.chan_plan_id = _br_setting.phy.config.fan11.chan_plan_id;
  phy_config.config.fan11.phy_mode_id = _br_setting.phy.config.fan11.phy_mode_id;
  phy_config.type = _br_setting.phy.type;

  EFM_ASSERT(sl_wisun_get_mac_address(&address) == SL_STATUS_OK);
  EFM_ASSERT(sl_wisun_br_dhcpv6_server_start(app_dhcpv6_socket,
                                             ipv6_prefix,
                                             address.address,
                                             LIFETIME_INFINITE) == SL_STATUS_OK);

  // Start Border Router
  EFM_ASSERT(sl_wisun_br_start((const uint8_t *)_br_setting.network_name, &phy_config) == SL_STATUS_OK);

  // Set PAN ID
  EFM_ASSERT(sl_wisun_get_network_info(&app_network_info) == SL_STATUS_OK);
  _br_setting.pan_id = app_network_info.pan_id;
  printf("[Border router PAN ID: 0x%04X]\n", _br_setting.pan_id);
#if defined(SL_CATALOG_WISUN_APP_SETTING_PRESENT)
  pan_id = app_network_info.pan_id;
  app_wisun_setting_set_pan_id(&pan_id);
#endif

#if defined(SL_CATALOG_WISUN_BR_AGENT_SERVICE_PRESENT)
  if (sl_wisun_br_agent_service_send_reg() != SL_STATUS_OK) {
    printf("[Failed: unable to send registration to the Agent Service]\n");
  }
#endif

  _app_wisun_mutex_release();
  printf("[Border router started]\n");
}

// -----------------------------------------------------------------------------
//                          Static Function Definitions
// -----------------------------------------------------------------------------

/* Mutex acquire */
__STATIC_INLINE void _app_wisun_mutex_acquire(void)
{
  EFM_ASSERT(osMutexAcquire(_app_core_mtx, osWaitForever) == osOK);
}

/* Mutex release */
__STATIC_INLINE void _app_wisun_mutex_release(void)
{
  EFM_ASSERT(osMutexRelease(_app_core_mtx) == osOK);
}

__STATIC_INLINE void _app_wisun_core_set_state(const sl_wisun_app_core_state_t flag)
{
  (void) osEventFlagsSet(_app_core_state, 1UL << flag);
}
