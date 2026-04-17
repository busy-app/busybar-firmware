/*******************************************************************************
* @file  app.c
* @brief 
*******************************************************************************
* # License
* <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
*******************************************************************************
*
* The licensor of this software is Silicon Laboratories Inc. Your use of this
* software is governed by the terms of Silicon Labs Master Software License
* Agreement (MSLA) available at
* www.silabs.com/about-us/legal/master-software-license-agreement. This
* software is distributed to you in Source Code format and is governed by the
* sections of the MSLA applicable to Source Code.
*
******************************************************************************/
/*************************************************************************
 *
 */

/*================================================================================
 * @brief : This file contains example application for Bluetooth LE Privacy
 * @section Description :
 * This application demonstrates how to configure device with privacy feature by
 * organizing resolving list and resolution process and how to connect to remote
 * Peripheral device.
 =================================================================================*/

/**
 * Include files
 * */

//! SL Wi-Fi SDK includes
#include "sl_board_configuration.h"
#include "sl_constants.h"
#include "sl_wifi.h"
#include "sl_wifi_callback_framework.h"
#include "cmsis_os2.h"
#include "sl_utility.h"
#include "FreeRTOSConfig.h"
//! BLE include file to refer BLE APIs
#include "rsi_ble_apis.h"
#include "ble_config.h"
#include "rsi_ble_common_config.h"
#include "rsi_bt_common_apis.h"
#include "rsi_bt_common.h"
#include "rsi_ble.h"

//! Common include file
#include "rsi_common_apis.h"
#include <stdio.h>
#include <string.h>
#if SL_SI91X_TICKLESS_MODE == 0 && defined(SLI_SI91X_MCU_INTERFACE)
#include "sl_si91x_m4_ps.h"
#include "sl_si91x_power_manager.h"
#endif

//! Remote Device Name to connect
#define RSI_REMOTE_DEVICE_NAME "BLE_SIMPLE_PRIVACY"

//! local device name
#define RSI_BLE_DEVICE_NAME "SIMPLE_PRIVACY"

//! local device IO capability
#define RSI_BLE_SMP_IO_CAPABILITY 0x00
#define RSI_BLE_SMP_PASSKEY       0

//! application event list
#define RSI_APP_EVENT_ADV_REPORT            0x00
#define RSI_BLE_CONN_EVENT                  0x01
#define RSI_BLE_DISCONN_EVENT               0x02
#define RSI_BLE_SMP_REQ_EVENT               0x03
#define RSI_BLE_SMP_RESP_EVENT              0x04
#define RSI_BLE_SMP_PASSKEY_EVENT           0x05
#define RSI_BLE_SMP_FAILED_EVENT            0x06
#define RSI_BLE_ENCRYPT_STARTED_EVENT       0x07
#define RSI_BLE_SMP_PASSKEY_DISPLAY_EVENT   0x08
#define RSI_BLE_SC_PASSKEY_EVENT            0x09
#define RSI_BLE_LTK_REQ_EVENT               0x0A
#define RSI_BLE_SECURITY_KEYS_EVENT         0x0B
#define RSI_BLE_ENHANCE_CONNECTED_EVENT     0x0C
#define RSI_APP_EVENT_DATA_LENGTH_CHANGE    0x0D
#define RSI_APP_EVENT_PHY_UPDATE_COMPLETE   0x0E
#define RSI_APP_EVENT_CONN_UPDATE_COMPLETE  0x0F
#define RSI_BLE_RECEIVE_REMOTE_FEATURES     0x10
#define RSI_APP_EVENT_REMOTE_CONN_PARAM_REQ 0x11

//! Address type of the device to connect
#define RSI_BLE_REMOTE_ADDR_TYPE LE_PUBLIC_ADDRESS

//! Address of the device to connect
#define RSI_BLE_REMOTE_ADDR "00:23:A7:56:77:77"

//!address resolution enable
#define RSI_BLE_SET_RESOLVABLE_PRIV_ADDR_TOUT 120

//! process type 1-add device to resolvlist, 2-remove device from resolvlist, 3-clear the resolvlist
#define RSI_BLE_ADD_TO_RESOLVE_LIST      1
#define RSI_BLE_REMOVE_FROM_RESOLVE_LIST 2
#define RSI_BLE_CLEAR_RESOLVE_LIST       3

//! privacy mode 0-Network privacy mode 1-Device privacy mode
#define RSI_BLE_NETWORK_PRIVACY_MODE 0
#define RSI_BLE_DEVICE_PRIVACY_MODE  1

#define RSI_BLE_PRIVACY_MODE RSI_BLE_DEVICE_PRIVACY_MODE

#define RSI_BLE_RESOLVING_LIST_SIZE 5

#define MITM_REQ          0x01
#define RSI_MAX_LIST_SIZE 0x05

#define LOCAL_DEV_ADDR_LEN 18 // Length of the local device address

/*=======================================================================*/
//!    Powersave configurations
/*=======================================================================*/
#define ENABLE_NWP_POWER_SAVE 0 //! Set to 1 for powersave mode

#if ENABLE_NWP_POWER_SAVE
//! Power Save Profile Mode
#define PSP_MODE RSI_SLEEP_MODE_2
//! Power Save Profile type
#define PSP_TYPE RSI_MAX_PSP

sl_wifi_performance_profile_v2_t wifi_profile = { .profile = ASSOCIATED_POWER_SAVE_LOW_LATENCY };
#endif
//! user defined structure
//LE resolvlist group.
typedef struct rsi_ble_resolvlist_group_s {
  uint8_t used;
  uint8_t remote_dev_addr_type;
  uint8_t remote_dev_addr[18];
  uint8_t peer_irk[16];
  uint8_t local_irk[16];
  uint8_t Identity_addr_type;
  uint8_t Identity_addr[18];
} rsi_ble_resolvlist_group_t;

//LE resolvlist.
typedef struct rsi_ble_resolve_key_s {
  uint8_t remote_dev_addr_type;
  uint8_t remote_dev_addr[18];
  uint8_t peer_irk[16];
  uint8_t local_irk[16];
  uint16_t remote_ediv;
  uint8_t remote_rand[16];
  uint8_t remote_ltk[16];
  uint8_t Identity_addr_type;
  uint8_t Identity_addr[18];
} rsi_ble_resolve_key_t;

//LE resolvlist.
typedef struct rsi_ble_dev_ltk_list_s {
  uint8_t enc_enable;
  uint8_t sc_enable;
  uint8_t remote_dev_addr_type;
  uint8_t remote_dev_addr[6];
  uint8_t peer_irk[16];
  uint8_t local_irk[16];
  uint16_t remote_ediv;
  uint16_t local_ediv;
  uint8_t remote_rand[8];
  uint8_t localrand[8];
  uint8_t remote_ltk[16];
  uint8_t localltk[16];
  uint8_t Identity_addr_type;
  uint8_t Identity_addr[6];
  uint8_t used;
} rsi_ble_dev_ltk_list_t;

//! global parameters list
static uint32_t passkey;
static uint8_t dev_address_type;
static uint8_t str_remote_address[18];
static uint8_t remote_addr_type = 0;
static uint8_t remote_name[31];
static uint8_t remote_dev_addr[18]          = { 0 };
static uint8_t remote_dev_bd_addr[6]        = { 0 };
static uint8_t device_found                 = 0;
static uint8_t rsi_app_resp_resolvlist_size = 0;

static rsi_bt_event_le_ltk_request_t temp_le_ltk_req;
static rsi_bt_event_le_security_keys_t temp_le_sec_keys;
static rsi_ble_event_enhance_conn_status_t rsi_app_enhance_connected_device = { 0 };
static rsi_ble_resolvlist_group_t resolvlist[5];
static rsi_ble_resolve_key_t resolve_key;
rsi_ble_dev_ltk_list_t ble_dev_ltk_list[RSI_MAX_LIST_SIZE];
uint8_t smp_passkey[BLE_PASSKEY_SIZE];

static rsi_ble_event_remote_features_t remote_dev_feature;
static rsi_ble_event_phy_update_t rsi_app_phy_update_complete;
static rsi_ble_event_remote_conn_param_req_t rsi_app_remote_device_conn_params;
static rsi_ble_event_conn_status_t rsi_app_connected_device;
static rsi_ble_event_data_length_update_t data_length_update;

uint8_t str_remote_dev_address[18] = { '\0' };
osSemaphoreId_t ble_main_task_sem;
static volatile uint32_t ble_app_event_map;
static volatile uint32_t ble_app_event_map1;

int8_t add_device_to_ltk_key_list(rsi_ble_dev_ltk_list_t *ble_dev_ltk_list,
                                  rsi_bt_event_encryption_enabled_t *enc_enabled);
int8_t add_security_keys_to_device_list(rsi_ble_dev_ltk_list_t *ble_dev_ltk_list,
                                        rsi_bt_event_le_security_keys_t *le_sec_keys);
int8_t rsi_get_ltk_list(rsi_ble_dev_ltk_list_t *ble_dev_ltk_list, rsi_bt_event_le_ltk_request_t *le_ltk_req);

static const sl_wifi_device_configuration_t config = {
  .boot_option = LOAD_NWP_FW,
  .mac_address = NULL,
  .band        = SL_SI91X_WIFI_BAND_2_4GHZ,
  .region_code = US,
  .boot_config = { .oper_mode       = SL_SI91X_CLIENT_MODE,
                   .coex_mode       = SL_SI91X_WLAN_BLE_MODE,
                   .feature_bit_map = (SL_WIFI_FEAT_WPS_DISABLE | SL_SI91X_FEAT_ULP_GPIO_BASED_HANDSHAKE
                                       | SL_SI91X_FEAT_DEV_TO_HOST_ULP_GPIO_1),
                   .tcp_ip_feature_bit_map =
                     (SL_SI91X_TCP_IP_FEAT_DHCPV4_CLIENT | SL_SI91X_TCP_IP_FEAT_EXTENSION_VALID),
                   .custom_feature_bit_map = (SL_WIFI_SYSTEM_CUSTOM_FEAT_EXTENSION_VALID),
                   .ext_custom_feature_bit_map =
                     (SL_WIFI_SYSTEM_EXT_FEAT_LOW_POWER_MODE | SL_SI91X_EXT_FEAT_XTAL_CLK | MEMORY_CONFIG
#ifdef SLI_SI917
                      | SL_SI91X_EXT_FEAT_FRONT_END_SWITCH_PINS_ULP_GPIO_4_5_0
#endif
                      | SL_SI91X_EXT_FEAT_BT_CUSTOM_FEAT_ENABLE),
                   .bt_feature_bit_map         = (SL_SI91X_BT_RF_TYPE | SL_SI91X_ENABLE_BLE_PROTOCOL),
                   .ext_tcp_ip_feature_bit_map = (SL_SI91X_CONFIG_FEAT_EXTENTION_VALID),
                   //!ENABLE_BLE_PROTOCOL in bt_feature_bit_map
                   .ble_feature_bit_map =
                     ((SL_SI91X_BLE_MAX_NBR_PERIPHERALS(RSI_BLE_MAX_NBR_PERIPHERALS)
                       | SL_SI91X_BLE_MAX_NBR_CENTRALS(RSI_BLE_MAX_NBR_CENTRALS)
                       | SL_SI91X_BLE_MAX_NBR_ATT_SERV(RSI_BLE_MAX_NBR_ATT_SERV)
                       | SL_SI91X_BLE_MAX_NBR_ATT_REC(RSI_BLE_MAX_NBR_ATT_REC))
                      | SL_SI91X_FEAT_BLE_CUSTOM_FEAT_EXTENTION_VALID | SL_SI91X_BLE_PWR_INX(RSI_BLE_PWR_INX)
                      | SL_SI91X_BLE_PWR_SAVE_OPTIONS(RSI_BLE_PWR_SAVE_OPTIONS)
                      | SL_SI91X_916_BLE_COMPATIBLE_FEAT_ENABLE
#if RSI_BLE_GATT_ASYNC_ENABLE
                      | SL_SI91X_BLE_GATT_ASYNC_ENABLE
#endif
                      ),
                   .ble_ext_feature_bit_map =
                     ((SL_SI91X_BLE_NUM_CONN_EVENTS(RSI_BLE_NUM_CONN_EVENTS)
                       | SL_SI91X_BLE_NUM_REC_BYTES(RSI_BLE_NUM_REC_BYTES))
#if RSI_BLE_INDICATE_CONFIRMATION_FROM_HOST
                      | SL_SI91X_BLE_INDICATE_CONFIRMATION_FROM_HOST //indication response from app
#endif
#if RSI_BLE_MTU_EXCHANGE_FROM_HOST
                      | SL_SI91X_BLE_MTU_EXCHANGE_FROM_HOST //MTU Exchange request initiation from app
#endif
#if RSI_BLE_SET_SCAN_RESP_DATA_FROM_HOST
                      | (SL_SI91X_BLE_SET_SCAN_RESP_DATA_FROM_HOST) //Set SCAN Resp Data from app
#endif
#if RSI_BLE_DISABLE_CODED_PHY_FROM_HOST
                      | (SL_SI91X_BLE_DISABLE_CODED_PHY_FROM_HOST) //Disable Coded PHY from app
#endif
#if BLE_SIMPLE_GATT
                      | SL_SI91X_BLE_GATT_INIT
#endif
                      ),
                   .config_feature_bit_map = (SL_SI91X_FEAT_SLEEP_GPIO_SEL_BITMAP | SL_WIFI_ENABLE_ENHANCED_MAX_PSP) }
};

const osThreadAttr_t thread_attributes = {
  .name       = "application_thread",
  .attr_bits  = 0,
  .cb_mem     = 0,
  .cb_size    = 0,
  .stack_mem  = 0,
  .stack_size = 3072,
  .priority   = osPriorityNormal,
  .tz_module  = 0,
  .reserved   = 0,
};

/*==============================================*/
/**
 * @fn         rsi_ble_app_init_events
 * @brief      initializes the event parameter.
 * @param[in]  none.
 * @return     none.
 * @section description
 * This function is used during BLE initialization.
 */
static void rsi_ble_app_init_events()
{
  ble_app_event_map  = 0;
  ble_app_event_map1 = 0;
  return;
}

/*==============================================*/
/**
 * @fn         rsi_ble_app_set_event
 * @brief      set the specific event.
 * @param[in]  event_num, specific event number.
 * @return     none.
 * @section description
 * This function is used to set/raise the specific event.
 */
void rsi_ble_app_set_event(uint32_t event_num)
{
  if (event_num < 32) {
    ble_app_event_map |= BIT(event_num);
  } else {
    ble_app_event_map1 |= BIT((event_num - 32));
  }

  osSemaphoreRelease(ble_main_task_sem);
  return;
}

/*==============================================*/
/**
 * @fn         rsi_ble_app_clear_event
 * @brief      clears the specific event.
 * @param[in]  event_num, specific event number.
 * @return     none.
 * @section description
 * This function is used to clear the specific event.
 */
static void rsi_ble_app_clear_event(uint32_t event_num)
{
  if (event_num < 32) {
    ble_app_event_map &= ~BIT(event_num);
  } else {
    ble_app_event_map1 &= ~BIT((event_num - 32));
  }

  return;
}

/*==============================================*/
/**
 * @fn         rsi_ble_app_get_event
 * @brief      returns the first set event based on priority
 * @param[in]  none.
 * @return     int32_t
 *             > 0  = event number
 *             -1   = not received any event
 * @section description
 * This function returns the highest priority event among all the set events
 */
static int32_t rsi_ble_app_get_event(void)
{
  uint32_t ix;

  for (ix = 0; ix < 64; ix++) {
    if (ix < 32) {
      if (ble_app_event_map & (1 << ix)) {
        return ix;
      }
    } else {
      if (ble_app_event_map1 & (1 << (ix - 32))) {
        return ix;
      }
    }
  }

  return (-1);
}

/*==============================================*/
/**
 * @fn         rsi_ble_simple_central_on_adv_report_event
 * @brief      invoked when advertise report event is received
 * @param[in]  adv_report, pointer to the received advertising report
 * @return     none.
 * @section description
 * This callback function updates the scanned remote devices list
 */
void rsi_ble_simple_central_on_adv_report_event(rsi_ble_event_adv_report_t *adv_report)
{
  if (device_found == 1) {
    return;
  }

  memset(remote_name, 0, 31);
  BT_LE_ADPacketExtract(remote_name, adv_report->adv_data, adv_report->adv_data_len);

  remote_addr_type = adv_report->dev_addr_type;
  rsi_6byte_dev_address_to_ascii(remote_dev_addr, (uint8_t *)adv_report->dev_addr);
  memcpy(remote_dev_bd_addr, (uint8_t *)adv_report->dev_addr, 6);
  if (((device_found == 0) && ((strcmp((const char *)remote_name, RSI_REMOTE_DEVICE_NAME)) == 0))
      || ((remote_addr_type == RSI_BLE_REMOTE_ADDR_TYPE)
          && ((strcmp((const char *)remote_dev_addr, RSI_BLE_REMOTE_ADDR) == 0)))) {
    device_found = 1;
    rsi_ble_app_set_event(RSI_APP_EVENT_ADV_REPORT);
  }
}

/*==============================================*/
/**
 * @fn         rsi_ble_on_connect_event
 * @brief      invoked when connection complete event is received
 * @param[out] resp_conn, connected remote device information
 * @return     none.
 * @section description
 * This callback function indicates the status of the connection
 */
static void rsi_ble_on_connect_event(rsi_ble_event_conn_status_t *resp_conn)
{
  dev_address_type = resp_conn->dev_addr_type;
  memcpy(remote_dev_bd_addr, resp_conn->dev_addr, 6);
  rsi_6byte_dev_address_to_ascii(str_remote_address, resp_conn->dev_addr);

  rsi_ble_app_set_event(RSI_BLE_CONN_EVENT);
}

/*==============================================*/
/**
 * @fn         rsi_ble_on_disconnect_event
 * @brief      invoked when disconnection event is received
 * @param[in]  resp_disconnect, disconnected remote device information
 * @param[in]  reason, reason for disconnection.
 * @return     none.
 * @section description
 * This callback function indicates disconnected device information and status
 */
static void rsi_ble_on_disconnect_event(rsi_ble_event_disconnect_t *resp_disconnect, uint16_t reason)
{
  UNUSED_PARAMETER(reason); //This statement is added only to resolve compilation warning, value is unchanged
  memcpy(remote_dev_bd_addr, resp_disconnect->dev_addr, 6);

  rsi_ble_app_set_event(RSI_BLE_DISCONN_EVENT);
}

/*==============================================*/
/**
 * @fn         rsi_ble_on_smp_request 
 * @brief      its invoked when smp request event is received.
 * @param[in]  remote_dev_address, it indicates remote bd address.
 * @return     none.
 * @section description
 * This callback function is invoked when an SMP request event is received (we are in central mode)
 * Note: peripheral requested to start SMP request, we have to send SMP request command
 */
void rsi_ble_on_smp_request(rsi_bt_event_smp_req_t *remote_dev_address)
{
  memcpy(remote_dev_bd_addr, remote_dev_address->dev_addr, 6);
  rsi_6byte_dev_address_to_ascii(str_remote_address, remote_dev_address->dev_addr);

  rsi_ble_app_set_event(RSI_BLE_SMP_REQ_EVENT);
}

/*==============================================*/
/**
 * @fn         rsi_ble_on_smp_response 
 * @brief      its invoked when smp response event is received.
 * @param[in]  remote_dev_address, it indicates remote bd address.
 * @return     none.
 * @section description
 * This callback function is invoked when an SMP response event is received(we are in peripheral mode)
 * Note: Central initiated SMP protocol, we have to send SMP response command
 */
void rsi_ble_on_smp_response(rsi_bt_event_smp_resp_t *remote_dev_address)
{
  memcpy(remote_dev_bd_addr, remote_dev_address->dev_addr, 6);
  rsi_6byte_dev_address_to_ascii(str_remote_address, remote_dev_address->dev_addr);

  rsi_ble_app_set_event(RSI_BLE_SMP_RESP_EVENT);
}

/*==============================================*/
/**
 * @fn         rsi_ble_on_smp_passkey 
 * @brief      its invoked when smp passkey event is received.
 * @param[in]  remote_dev_address, it indicates remote bd address.
 * @return     none.
 * @section description
 * This callback function is invoked when SMP passkey events is received
 * Note: We have to send SMP passkey command
 */
void rsi_ble_on_smp_passkey(rsi_bt_event_smp_passkey_t *remote_dev_address)
{
  memcpy(remote_dev_bd_addr, remote_dev_address->dev_addr, 6);
  rsi_6byte_dev_address_to_ascii(str_remote_address, remote_dev_address->dev_addr);

  rsi_ble_app_set_event(RSI_BLE_SMP_PASSKEY_EVENT);
}

/*==============================================*/
/**
 * @fn         rsi_ble_on_smp_passkey_display 
 * @brief      its invoked when smp passkey event is received.
 * @param[in]  remote_dev_address, it indicates remote bd address.
 * @return     none.
 * @section description
 * This callback function is invoked when SMP passkey events is received
 * Note: We have to send SMP passkey command
 */
void rsi_ble_on_smp_passkey_display(rsi_bt_event_smp_passkey_display_t *smp_passkey_display)
{
  memcpy(remote_dev_bd_addr, smp_passkey_display->dev_addr, 6);
  rsi_6byte_dev_address_to_ascii(str_remote_address, smp_passkey_display->dev_addr);
  memcpy(smp_passkey, smp_passkey_display->passkey, sizeof(smp_passkey_display->passkey));

  rsi_ble_app_set_event(RSI_BLE_SMP_PASSKEY_DISPLAY_EVENT);
}
/*==============================================*/
/**
 * @fn         rsi_ble_on_sc_passkey 
 * @brief      its invoked when smp passkey event is received.
 * @param[in]  remote_dev_address, it indicates remote bd address.
 * @return     none.
 * @section description
 * This callback function is invoked when SMP passkey events is received
 * Note: We have to send SMP passkey command
 */
void rsi_ble_on_sc_passkey(rsi_bt_event_sc_passkey_t *sc_passkey)
{
  memcpy(remote_dev_bd_addr, sc_passkey->dev_addr, 6);
  rsi_6byte_dev_address_to_ascii(str_remote_address, sc_passkey->dev_addr);
  passkey = sc_passkey->passkey;

  rsi_ble_app_set_event(RSI_BLE_SC_PASSKEY_EVENT);
}

/*==============================================*/
/**
 * @fn         rsi_ble_on_smp_failed 
 * @brief      its invoked when smp failed event is received.
 * @param[in]  remote_dev_address, it indicates remote bd address.
 * @return     none.
 * @section description
 * This callback function is invoked when SMP failed events is received
 */
void rsi_ble_on_smp_failed(uint16_t status, rsi_bt_event_smp_failed_t *remote_dev_address)
{
  UNUSED_PARAMETER(status);
  memcpy(remote_dev_bd_addr, remote_dev_address->dev_addr, 6);
  rsi_6byte_dev_address_to_ascii(str_remote_address, remote_dev_address->dev_addr);
  rsi_ble_app_set_event(RSI_BLE_SMP_FAILED_EVENT);
}

/*==============================================*/
/**
 * @fn         rsi_ble_on_le_ltk_req_event
 * @brief      invoked when disconnection event is received
 * @param[in]  resp_disconnect, disconnected remote device information
 * @param[in]  reason, reason for disconnection.
 * @return     none.
 * @section description
 * This callback function indicates disconnected device information and status
 */
static void rsi_ble_on_le_ltk_req_event(rsi_bt_event_le_ltk_request_t *le_ltk_req)
{
  LOG_PRINT(" \r\n rsi_ble_on_le_ltk_req_event \r\n");
  memcpy(&temp_le_ltk_req, le_ltk_req, sizeof(rsi_bt_event_le_ltk_request_t));
  rsi_ble_app_set_event(RSI_BLE_LTK_REQ_EVENT);
}

/*==============================================*/
/**
 * @fn         rsi_ble_on_le_security_keys_event
 * @brief      invoked when security keys event is received
 * @param[in]  rsi_bt_event_le_security_keys_t, security keys information
 * @return     none.
 * @section description
 * This callback function indicates security keys information
 */
static void rsi_ble_on_le_security_keys_event(rsi_bt_event_le_security_keys_t *le_sec_keys)
{
  LOG_PRINT(" \r\n rsi_ble_on_le_security_keys_event \r\n");
  add_security_keys_to_device_list(ble_dev_ltk_list, le_sec_keys);
  memcpy(&temp_le_sec_keys, le_sec_keys, sizeof(rsi_bt_event_le_security_keys_t));
  rsi_ble_app_set_event(RSI_BLE_SECURITY_KEYS_EVENT);
}

/*==============================================*/
/**
 * @fn         rsi_ble_on_enhance_conn_status_event
 * @brief      invoked when enhanced connection complete event is received
 * @param[out] resp_enh_conn, connected remote device information
 * @return     none.
 * @section description
 * This callback function indicates the status of the enhanced connection
 */
void rsi_ble_on_enhance_conn_status_event(rsi_ble_event_enhance_conn_status_t *resp_enh_conn)
{
  memcpy(&rsi_app_enhance_connected_device, resp_enh_conn, sizeof(rsi_ble_event_enhance_conn_status_t));
  dev_address_type = resp_enh_conn->dev_addr_type;
  memcpy(remote_dev_bd_addr, resp_enh_conn->dev_addr, 6);

  rsi_ble_app_set_event(RSI_BLE_ENHANCE_CONNECTED_EVENT);
}
/*==============================================*/
/**
 * @fn         rsi_ble_data_length_change_event
 * @brief      invoked when data length is set
  * @section description
 * This Callback function indicates data length is set
 */
void rsi_ble_data_length_change_event(rsi_ble_event_data_length_update_t *rsi_ble_data_length_update)
{
  memcpy(&data_length_update, rsi_ble_data_length_update, sizeof(rsi_ble_event_data_length_update_t));

  rsi_ble_app_set_event(RSI_APP_EVENT_DATA_LENGTH_CHANGE);
}
/*==============================================*/
/**
 * @fn         rsi_ble_phy_update_complete_event
 * @brief      invoked when disconnection event is received
 * @param[in]  resp_disconnect, disconnected remote device information
 * @param[in]  reason, reason for disconnection.
 * @return     none.
 * @section description
 * This Callback function indicates disconnected device information and status
 */
void rsi_ble_phy_update_complete_event(rsi_ble_event_phy_update_t *rsi_ble_event_phy_update_complete)
{
  LOG_PRINT("\n PHY UPDATE COMPLETE\n");
  memcpy(&rsi_app_phy_update_complete, rsi_ble_event_phy_update_complete, sizeof(rsi_ble_event_phy_update_t));
  rsi_ble_app_set_event(RSI_APP_EVENT_PHY_UPDATE_COMPLETE);
}

void rsi_ble_on_conn_update_complete_event(rsi_ble_event_conn_update_t *rsi_ble_event_conn_update_complete,
                                           uint16_t resp_status)
{
  UNUSED_PARAMETER(resp_status); //This statement is added only to resolve compilation warning, value is unchanged
  UNUSED_PARAMETER(
    rsi_ble_event_conn_update_complete); //This statement is added only to resolve compilation warning, value is unchanged
  LOG_PRINT("\n CONN UPDATE COMPLETE\n");
  rsi_ble_app_set_event(RSI_APP_EVENT_CONN_UPDATE_COMPLETE);
}
/*==============================================*/
/**
* @fn         rsi_ble_on_remote_features_event
* @brief      invoked when LE remote features event is received.
* @param[in] resp_conn, connected remote device information
* @return     none.
* @section description
* This callback function indicates the status of the connection
*/
void rsi_ble_on_remote_features_event(rsi_ble_event_remote_features_t *rsi_ble_event_remote_features)
{
  //  LOG_PRINT("\nFeature received is %s\n", rsi_ble_event_remote_features->remote_features);
  memcpy(&remote_dev_feature, rsi_ble_event_remote_features, sizeof(rsi_ble_event_remote_features_t));
  rsi_ble_app_set_event(RSI_BLE_RECEIVE_REMOTE_FEATURES);
}
/*==============================================*/
/**
* @fn         rsi_ble_on_remote_conn_params_request_event
* @brief      invoked when remote conn params request is received
* @param[out] remote_conn_param, emote conn params request information
* @return     none.
* @section description
* This callback function indicates the status of the connection
*/
static void rsi_ble_on_remote_conn_params_request_event(rsi_ble_event_remote_conn_param_req_t *remote_conn_param,
                                                        uint16_t status)
{
  UNUSED_PARAMETER(status); //This statement is added only to resolve compilation warning, value is unchanged
  memcpy(&rsi_app_remote_device_conn_params, remote_conn_param, sizeof(rsi_ble_event_remote_conn_param_req_t));

  rsi_ble_app_set_event(RSI_APP_EVENT_REMOTE_CONN_PARAM_REQ);
}
/*==============================================*/
/**
 * @fn         rsi_ble_on_encrypt_started 
 * @brief      its invoked when encryption started event is received.
 * @param[in]  remote_dev_address, it indicates remote bd address.
 * @return     none.
 * @section description
 * This callback function is invoked when encryption started events is received
 */
void rsi_ble_on_encrypt_started(uint16_t status, rsi_bt_event_encryption_enabled_t *enc_enabled)
{
  UNUSED_PARAMETER(status);
  add_device_to_ltk_key_list(ble_dev_ltk_list, enc_enabled);
  rsi_ble_app_set_event(RSI_BLE_ENCRYPT_STARTED_EVENT);
}

/*==============================================*/
/**
 * @fn         add_device_to_ltk_key_list
 * @brief      this function will add device to the ltk key list
 * @param[in]  ltk device list pointer
 * @param[in]  encrypt start event pointer
 * @param[out] status
 * @section description
 * add device to ltk key list
 * */
int8_t add_device_to_ltk_key_list(rsi_ble_dev_ltk_list_t *ble_dev_ltk_list,
                                  rsi_bt_event_encryption_enabled_t *enc_enabled)
{
  int8_t status = 0;
  uint8_t ix;

  for (ix = 0; ix < RSI_MAX_LIST_SIZE; ix++) {

    if (ble_dev_ltk_list[ix].used == 1) {
      if ((enc_enabled->dev_addr_type > 1) && (!memcmp(enc_enabled->dev_addr, ble_dev_ltk_list[ix].Identity_addr, 6))) {
        break;
      }

      if ((enc_enabled->dev_addr_type <= 1)
          && (!memcmp(enc_enabled->dev_addr, ble_dev_ltk_list[ix].remote_dev_addr, 6))) {
        break;
      }
    }

    if (ble_dev_ltk_list[ix].used == 0) {
      ble_dev_ltk_list[ix].used       = 1;
      ble_dev_ltk_list[ix].enc_enable = enc_enabled->enabled;
      ble_dev_ltk_list[ix].sc_enable  = enc_enabled->sc_enable;
      memcpy(ble_dev_ltk_list[ix].remote_dev_addr, enc_enabled->dev_addr, 6);
      memcpy(ble_dev_ltk_list[ix].localltk, enc_enabled->localltk, 16);
      memcpy(ble_dev_ltk_list[ix].localrand, enc_enabled->localrand, 8);
      ble_dev_ltk_list[ix].local_ediv = enc_enabled->localediv;
      break;
    }
  }
  if (ix >= RSI_MAX_LIST_SIZE) {
    return -1;
  }
  return status;
}

/*==============================================*/
/**
 * @fn         add_device_to_resolvlist
 * @brief      this function will add device to resolvlistwith updated irks 
 * @param[out] ix, index of resolvlist
 * @return     none.
 * @section description
 * add device to resolvlistwith updated irks 
 * */
int8_t add_security_keys_to_device_list(rsi_ble_dev_ltk_list_t *ble_dev_ltk_list,
                                        rsi_bt_event_le_security_keys_t *le_sec_keys)
{
  int8_t status = 0;
  uint8_t ix;
  for (ix = 0; ix < RSI_MAX_LIST_SIZE; ix++) {

    if ((ble_dev_ltk_list[ix].used == 1) && (!memcmp(ble_dev_ltk_list[ix].remote_dev_addr, le_sec_keys->dev_addr, 6))
        && (ble_dev_ltk_list[ix].remote_dev_addr_type == le_sec_keys->dev_addr_type)) {
      memcpy(ble_dev_ltk_list[ix].local_irk, le_sec_keys->local_irk, 16);
      memcpy(ble_dev_ltk_list[ix].peer_irk, le_sec_keys->remote_irk, 16);
      memcpy(ble_dev_ltk_list[ix].remote_rand, le_sec_keys->remote_rand, 8);
      memcpy(ble_dev_ltk_list[ix].remote_ltk, le_sec_keys->remote_ltk, 16);
      memcpy(ble_dev_ltk_list[ix].Identity_addr, le_sec_keys->Identity_addr, 6);
      ble_dev_ltk_list[ix].remote_ediv        = le_sec_keys->remote_ediv;
      ble_dev_ltk_list[ix].Identity_addr_type = le_sec_keys->Identity_addr_type;
      break;
    }
  }
  if (ix >= RSI_MAX_LIST_SIZE) {
    return -1;
  }

  return status;
}

/*==============================================*/
/**
 * @fn         add_device_to_resolvlist
 * @brief      this function will add device to resolvlistwith updated irks 
 * @param[out] ix, index of resolvlist
 * @return     none.
 * @section description
 * add device to resolvlistwith updated irks 
 * */
int32_t add_device_to_resolvlist(rsi_ble_resolvlist_group_t *resolvlist_p, rsi_ble_resolve_key_t *resolve_key_p)
{
  uint32_t status = 0;
  uint8_t ix;

  for (ix = 0; ix < RSI_BLE_RESOLVING_LIST_SIZE; ix++) {
    if (resolvlist_p[ix].used == 0) {
      resolvlist_p[ix].used               = 1;
      resolvlist_p[ix].Identity_addr_type = resolve_key_p->Identity_addr_type;
      memcpy(resolvlist_p[ix].Identity_addr, resolve_key_p->Identity_addr, sizeof(resolve_key_p->Identity_addr));
      memcpy(resolvlist_p[ix].peer_irk, resolve_key_p->peer_irk, sizeof(resolve_key_p->peer_irk));
      memcpy(resolvlist_p[ix].local_irk, resolve_key_p->local_irk, sizeof(resolve_key_p->local_irk));
      break;
    }
  }
  if (ix >= RSI_BLE_RESOLVING_LIST_SIZE) {
    return -1;
  }

  //add device to resolvlist or remove from resolvlist or clear the resolvlist
  status = rsi_ble_resolvlist(RSI_BLE_ADD_TO_RESOLVE_LIST,
                              resolve_key_p->Identity_addr_type,
                              temp_le_sec_keys.Identity_addr,
                              resolve_key_p->peer_irk,
                              resolve_key_p->local_irk);
  if (status != RSI_SUCCESS) {
    return status;
  }
  return status;
}

int8_t rsi_get_ltk_list(rsi_ble_dev_ltk_list_t *ble_dev_ltk_list, rsi_bt_event_le_ltk_request_t *le_ltk_req)
{
  uint8_t ix;

  for (ix = 0; ix < RSI_MAX_LIST_SIZE; ix++) {

    if (ble_dev_ltk_list[ix].used == 1) {
      if ((le_ltk_req->dev_addr_type > 1) && (!(memcmp(le_ltk_req->dev_addr, ble_dev_ltk_list[ix].Identity_addr, 6)))) {
        return ix;
      }

      if ((le_ltk_req->dev_addr_type <= 1)
          && (!(memcmp(le_ltk_req->dev_addr, ble_dev_ltk_list[ix].remote_dev_addr, 6)))) {
        return ix;
      }
    }
  }
  return -1;
}

/*==============================================*/
/**
 * @fn         update_resolvlist
 * @brief      this function will update the resolvlist if different remote address or different irks for same address received
 * @param[out] none.
 * @return     none.
 * @section description
 * update the resolvlist if different remote address or different irks for same address received
 * */
void update_resolvlist(rsi_ble_resolvlist_group_t *resolvlist_p, rsi_ble_resolve_key_t *resolve_key_p)
{
  uint8_t ix = 0;
  while (ix < RSI_BLE_RESOLVING_LIST_SIZE) {
    if (!strcmp((const char *)resolvlist_p[ix].Identity_addr, (const char *)resolve_key_p->Identity_addr)) {
      if (memcmp(resolvlist_p[ix].peer_irk, resolve_key_p->peer_irk, sizeof(resolve_key_p->peer_irk))) {
        add_device_to_resolvlist(resolvlist_p, resolve_key_p);
        break;
      } else {
        //LOG_PRINT("\nPeer IRK is not modified\n");
        break;
      }
    } else {
      strcpy((char *)resolvlist_p[ix].Identity_addr, (char *)resolve_key_p->Identity_addr);
      add_device_to_resolvlist(resolvlist_p, resolve_key_p);
      break;
    }
    ix++;
  }
}

/*==============================================*/
/**
 * @fn         ble_privacy_app
 * @brief      This application is used for testing the Privacy feature.
 * @param[in]  none. 
 * @return     none.
 * @section description
 * This function is used to test the LL Privacy Protocol.
 */
void ble_privacy_app(void *unused)
{
  UNUSED_PARAMETER(unused);
  int32_t ix;
  int32_t event_id;
  uint8_t first_connect                                      = 0;
  rsi_ble_dev_ltk_list_t *ble_dev_ltk                        = NULL;
  sl_wifi_firmware_version_t version                         = { 0 };
  static uint8_t rsi_app_resp_get_dev_addr[RSI_DEV_ADDR_LEN] = { 0 };
  uint8_t local_dev_addr[LOCAL_DEV_ADDR_LEN]                 = { 0 };

#if (RSI_DEVICE_ROLE == PERIPHERAL_ROLE)
  uint8_t adv[31] = { 2, 1, 6 };
#endif
  sl_status_t status;

  //! Wi-Fi initialization
  status = sl_wifi_init(&config, NULL, sl_wifi_default_event_handler);
  if (status != SL_STATUS_OK) {
    LOG_PRINT("\r\n Wi-Fi Initialization Failed, Error Code : 0x%lX\r\n", status);
    return;
  }
  printf("\r\n Wi-Fi initialization is successful\n");

  //! Firmware version Prints
  status = sl_wifi_get_firmware_version(&version);
  if (status != SL_STATUS_OK) {
    LOG_PRINT("\r\nFirmware version Failed, Error Code : 0x%lX\r\n", status);
  } else {
    print_firmware_version(&version);
  }

  //! get the local device MAC address.
  status = rsi_bt_get_local_device_address(rsi_app_resp_get_dev_addr);
  if (status != RSI_SUCCESS) {
    LOG_PRINT("\r\n Get local device address failed = %lx\r\n", status);
    return;
  } else {
    rsi_6byte_dev_address_to_ascii(local_dev_addr, rsi_app_resp_get_dev_addr);
    LOG_PRINT("\r\n Local device address %s \r\n", local_dev_addr);
  }

  //! registering the GAP callback functions
  rsi_ble_gap_register_callbacks(rsi_ble_simple_central_on_adv_report_event,
                                 rsi_ble_on_connect_event,
                                 rsi_ble_on_disconnect_event,
                                 NULL,
                                 rsi_ble_phy_update_complete_event,
                                 rsi_ble_data_length_change_event,
                                 rsi_ble_on_enhance_conn_status_event,
                                 NULL,
                                 rsi_ble_on_conn_update_complete_event,
                                 rsi_ble_on_remote_conn_params_request_event);

  rsi_ble_gap_extended_register_callbacks(rsi_ble_on_remote_features_event, NULL);

  //! registering the SMP callback functions
  rsi_ble_smp_register_callbacks(rsi_ble_on_smp_request,
                                 rsi_ble_on_smp_response,
                                 rsi_ble_on_smp_passkey,
                                 rsi_ble_on_smp_failed,
                                 rsi_ble_on_encrypt_started,
                                 rsi_ble_on_smp_passkey_display,
                                 rsi_ble_on_sc_passkey,
                                 rsi_ble_on_le_ltk_req_event,
                                 rsi_ble_on_le_security_keys_event,
                                 NULL,
                                 NULL);
  //! create ble main task if ble protocol is selected
  ble_main_task_sem = osSemaphoreNew(1, 0, NULL);

  //!  initializing the application events map
  rsi_ble_app_init_events();

  //! Set local name
  status = rsi_bt_set_local_name((uint8_t *)RSI_BLE_DEVICE_NAME);
  if (status != RSI_SUCCESS) {
    LOG_PRINT("\r\n ble set local name cmd failed with reason code : %lX \n", status);
    return;
  }
  LOG_PRINT("\n Local name set to: %s\n", RSI_BLE_DEVICE_NAME);

#if (RSI_DEVICE_ROLE == PERIPHERAL_ROLE)
  //!preparing scan response data
  adv[3] = strlen(RSI_BLE_DEVICE_NAME) + 1;
  adv[4] = 9;
  strcpy((char *)&adv[5], RSI_BLE_DEVICE_NAME);

  //!set scan response data
  status = rsi_ble_set_scan_response_data(adv, strlen(RSI_BLE_DEVICE_NAME) + 5);
  if (status != RSI_SUCCESS) {
    LOG_PRINT("\r\n Set Scan Response Data Failed, Error Code : 0x%lX\r\n", status);
    return;
  } else {
    LOG_PRINT("\r\n Set Scan Response Data Success\r\n");
  }

  //! set advertise data
  status = rsi_ble_set_advertise_data(adv, strlen(RSI_BLE_DEVICE_NAME) + 5);
  if (status != RSI_SUCCESS) {
    LOG_PRINT("\r\n Set Advertise Data Failed, Error Code : 0x%lX\r\n", status);
    return;
  } else {
    LOG_PRINT("\r\n Set Advertise Data Success\r\n");
  }

  //! start the advertising
  status = rsi_ble_start_advertising();
  if (status != RSI_SUCCESS) {
    LOG_PRINT("\r\n Start Advertising Failed, Error Code : 0x%lX\r\n", status);
    return;
  } else {
    LOG_PRINT("\r\n Start Advertising Success\r\n");
  }
#else
  uint8_t Resolve_list_updated = 0; // Variable to check if reslove_key is updated or not.
  status                       = rsi_ble_start_scanning();
  if (status != RSI_SUCCESS) {
    LOG_PRINT("\r\nStart Scanning Failed, Error Code : 0x%lX\r\n", status);
    return;
  } else {
    LOG_PRINT("\r\nStart Scanning Success\r\n");
  }

#endif
#if ENABLE_NWP_POWER_SAVE
  LOG_PRINT("\r\n keep module in to power save \r\n");
  //! initiating power save in BLE mode
  status = rsi_bt_power_save_profile(PSP_MODE, PSP_TYPE);
  if (status != RSI_SUCCESS) {
    LOG_PRINT("\r\n Failed to initiate power save in BLE mode \r\n");
    return;
  }

  //! initiating power save in wlan mode
  status = sl_wifi_set_performance_profile_v2(&wifi_profile);
  if (status != SL_STATUS_OK) {
    LOG_PRINT("\r\n Failed to initiate power save in Wi-Fi mode :%lx\r\n", status);
    return;
  }

  LOG_PRINT("\r\n Module is in power save \r\n");
#endif
  //! waiting for events from controller.
  while (1) {
    //! checking for events list
    event_id = rsi_ble_app_get_event();

    if (event_id == -1) {
#if ((SL_SI91X_TICKLESS_MODE == 0) && SLI_SI91X_MCU_INTERFACE && ENABLE_NWP_POWER_SAVE)
      //! if events are not received loop will be continued.
      if ((!(P2P_STATUS_REG & TA_wakeup_M4))) {
        P2P_STATUS_REG &= ~M4_wakeup_TA;
        sl_si91x_power_manager_sleep();
      }
#else
      osSemaphoreAcquire(ble_main_task_sem, osWaitForever);
#endif
      continue;
    }

    switch (event_id) {
      case RSI_APP_EVENT_ADV_REPORT: {
        //! advertise report event.
        //! clear the advertise report event.
        rsi_ble_app_clear_event(RSI_APP_EVENT_ADV_REPORT);
        LOG_PRINT("\r\n In Advertising Event\r\n");

        status = rsi_ble_connect(remote_addr_type, (int8_t *)remote_dev_bd_addr);
        if (status != RSI_SUCCESS) {
          LOG_PRINT("\n Connect status: 0x%lX\r\n", status);
        }
      } break;
      case RSI_BLE_CONN_EVENT: {
        //! event invokes when connection was completed

        //! clear the served event
        rsi_ble_app_clear_event(RSI_BLE_CONN_EVENT);
        rsi_6byte_dev_address_to_ascii(str_remote_dev_address, rsi_app_connected_device.dev_addr);
        LOG_PRINT("\r\n Module connected to address : %s \r\n", str_remote_dev_address);

        status = rsi_ble_smp_pair_request(remote_dev_bd_addr, RSI_BLE_SMP_IO_CAPABILITY, MITM_REQ);
      } break;

      case RSI_BLE_DISCONN_EVENT: {
        //! event invokes when disconnection was completed

        //! clear the served event
        rsi_ble_app_clear_event(RSI_BLE_DISCONN_EVENT);
        LOG_PRINT("\r\n Module got disconnected\r\n");
#if ENABLE_NWP_POWER_SAVE
        LOG_PRINT("\r\n keep module in to active state \r\n");
        //! initiating Active mode in BT mode
        status = rsi_bt_power_save_profile(RSI_ACTIVE, PSP_TYPE);
        if (status != RSI_SUCCESS) {
          LOG_PRINT("\r\n Failed to keep Module in ACTIVE mode \r\n");
          return;
        }

        //! initiating power save in wlan mode
        wifi_profile.profile = HIGH_PERFORMANCE;
        status               = sl_wifi_set_performance_profile_v2(&wifi_profile);
        if (status != SL_STATUS_OK) {
          LOG_PRINT("\r\n Failed to keep module in HIGH_PERFORMANCE mode \r\n");
          return;
        }
#endif
        device_found  = 0;
        first_connect = 0;
#if (RSI_DEVICE_ROLE == CENTRAL_ROLE)

        if (Resolve_list_updated == 0) {
          status = rsi_ble_start_scanning();
          if (status != RSI_SUCCESS) {
            LOG_PRINT("\r\nStart Scanning Failed, Error Code : 0x%lX\r\n", status);
            return;
          } else {
            LOG_PRINT("\r\n Scanning Success\r\n");
          }

        } else {
          status = rsi_ble_connect(
            resolve_key.Identity_addr_type + 2,
            (int8_t *)rsi_ascii_dev_address_to_6bytes_rev(remote_dev_bd_addr, (int8_t *)resolve_key.Identity_addr));
          if (status != RSI_SUCCESS) {
            LOG_PRINT("\n Connect status: 0x%lX\r\n", status);
          }
        }
#else
        //! start the advertising
        LOG_PRINT("\n Start Advertising\n");
        status = rsi_ble_start_advertising();
        if (status != RSI_SUCCESS) {
          LOG_PRINT("\n Failed to start advertising, error code : %lx\n", status);
          return;
        }
#endif
#if ENABLE_NWP_POWER_SAVE
        LOG_PRINT("\r\n keep module in to power save \r\n");
        status = rsi_bt_power_save_profile(PSP_MODE, PSP_TYPE);
        if (status != RSI_SUCCESS) {
          return;
        }

        //! initiating power save in wlan mode
        wifi_profile.profile = ASSOCIATED_POWER_SAVE;
        status               = sl_wifi_set_performance_profile_v2(&wifi_profile);
        if (status != SL_STATUS_OK) {
          LOG_PRINT("\r\n Failed to keep module in power save \r\n");
          return;
        }
        LOG_PRINT("\r\n Module is in power save \r\n");
#endif
      } break;

      case RSI_BLE_SMP_REQ_EVENT: {
        LOG_PRINT("\n In SMP request event, str_remote_address : %s\r\n", str_remote_address);

        //! clear the served event
        rsi_ble_app_clear_event(RSI_BLE_SMP_REQ_EVENT);
        if (first_connect == 0) {
          //! initiating the SMP pairing process
          status        = rsi_ble_smp_pair_request(remote_dev_bd_addr, RSI_BLE_SMP_IO_CAPABILITY, MITM_REQ);
          first_connect = 1;
        }
      } break;

      case RSI_BLE_SMP_RESP_EVENT: {
        LOG_PRINT("\n In SMP response event, str_remote_address : %s\r\n", str_remote_address);

        //! clear the served event
        rsi_ble_app_clear_event(RSI_BLE_SMP_RESP_EVENT);

        //! initiating the SMP pairing process
        status = rsi_ble_smp_pair_response(remote_dev_bd_addr, RSI_BLE_SMP_IO_CAPABILITY, MITM_REQ);
      } break;

      case RSI_BLE_SMP_PASSKEY_EVENT: {
        LOG_PRINT("\n In SMP passkey event, str_remote_address : %s\r\n", str_remote_address);

        //! clear the served event
        rsi_ble_app_clear_event(RSI_BLE_SMP_PASSKEY_EVENT);

        //! initiating the SMP pairing process
        status = rsi_ble_smp_passkey(remote_dev_bd_addr, RSI_BLE_SMP_PASSKEY);
      } break;
      case RSI_BLE_SMP_PASSKEY_DISPLAY_EVENT: {
        LOG_PRINT("\r\nIn SMP passkey display event\r\n");
        LOG_PRINT("\n Remote addr: %s, passkey: %s\r\n", str_remote_address, smp_passkey);

        //! clear the served event
        rsi_ble_app_clear_event(RSI_BLE_SMP_PASSKEY_DISPLAY_EVENT);
      } break;
      case RSI_BLE_LTK_REQ_EVENT: {
        LOG_PRINT("\r\n In LTK request event\r\n");
        //! event invokes when disconnection was completed

        //! clear the served event
        rsi_ble_app_clear_event(RSI_BLE_LTK_REQ_EVENT);
        ix = rsi_get_ltk_list(ble_dev_ltk_list, &temp_le_ltk_req);

        ble_dev_ltk = &ble_dev_ltk_list[ix];
        if ((ix != -1) && (ble_dev_ltk != NULL)) {
          LOG_PRINT("\n positive reply\n");
          //!  give le ltk req reply cmd with positive reply
          status = rsi_ble_ltk_req_reply(temp_le_ltk_req.dev_addr,
                                         (1 | (ble_dev_ltk->enc_enable) | (ble_dev_ltk->sc_enable << 7)),
                                         ble_dev_ltk->localltk);
          if (status != RSI_SUCCESS) {
            LOG_PRINT("\r\n Failed to restart smp pairing with status: 0x%lx \r\n", status);
          }
        }

        else {
          LOG_PRINT("\n Negative reply\n");
          //! give le ltk req reply cmd with negative reply
          status = rsi_ble_ltk_req_reply(temp_le_ltk_req.dev_addr, 0, NULL);

          LOG_PRINT("\n Re-initiating SMP\n");
          status = rsi_ble_smp_pair_request(remote_dev_bd_addr, RSI_BLE_SMP_IO_CAPABILITY, MITM_REQ);
        }
      } break;

      case RSI_BLE_SC_PASSKEY_EVENT: {
        LOG_PRINT("\r\n In SC passkey event\r\n");
        LOG_PRINT("\n Remote addr: %s, passkey: %06ld \r\n", str_remote_address, passkey);

        //! clear the served event
        rsi_ble_app_clear_event(RSI_BLE_SC_PASSKEY_EVENT);
        rsi_ble_smp_passkey(remote_dev_bd_addr, passkey);
      } break;

      case RSI_BLE_SECURITY_KEYS_EVENT: {
        LOG_PRINT("\r\n In Sequrity keys event\r\n");
        //! event invokes when security keys are received
        //! clear the served event
        rsi_ble_app_clear_event(RSI_BLE_SECURITY_KEYS_EVENT);
        resolve_key.remote_dev_addr_type = dev_address_type;

        resolve_key.remote_ediv = temp_le_sec_keys.remote_ediv;
        memcpy(resolve_key.remote_rand, temp_le_sec_keys.remote_rand, sizeof(temp_le_sec_keys.remote_rand));
        memcpy(resolve_key.remote_ltk, temp_le_sec_keys.remote_ltk, sizeof(temp_le_sec_keys.remote_ltk));

        rsi_6byte_dev_address_to_ascii(resolve_key.remote_dev_addr, temp_le_sec_keys.dev_addr);
        memcpy(resolve_key.peer_irk, temp_le_sec_keys.remote_irk, sizeof(temp_le_sec_keys.remote_irk));
        memcpy(resolve_key.local_irk, temp_le_sec_keys.local_irk, sizeof(temp_le_sec_keys.local_irk));

        resolve_key.Identity_addr_type = temp_le_sec_keys.Identity_addr_type;
        rsi_6byte_dev_address_to_ascii(resolve_key.Identity_addr, temp_le_sec_keys.Identity_addr);

        update_resolvlist((rsi_ble_resolvlist_group_t *)&resolvlist, &resolve_key);
#if (RSI_DEVICE_ROLE == CENTRAL_ROLE)

        if (Resolve_list_updated == 0) {
          Resolve_list_updated = 1;
        }
#endif

        //get resolvlist size
        status = rsi_ble_get_resolving_list_size(&rsi_app_resp_resolvlist_size);
        if (status != RSI_SUCCESS) {
          LOG_PRINT("\n line %d -> status: 0x%lx\r\n", __LINE__, status);
          return;
        }

        //set address resolution enable and resolvable private address timeout
        status =
          rsi_ble_set_addr_resolution_enable(RSI_BLE_DEV_ADDR_RESOLUTION_ENABLE, RSI_BLE_SET_RESOLVABLE_PRIV_ADDR_TOUT);
        if (status != RSI_SUCCESS) {
          LOG_PRINT("\n line %d -> status: 0x%lx\r\n", __LINE__, status);
          return;
        }

        //set privacy mode
        status = rsi_ble_set_privacy_mode(
          resolve_key.Identity_addr_type,
          rsi_ascii_dev_address_to_6bytes_rev(remote_dev_bd_addr, (int8_t *)resolve_key.Identity_addr),
          RSI_BLE_PRIVACY_MODE);
        if (status != RSI_SUCCESS) {
          LOG_PRINT("line %d -> status: 0x%lx\r\n", __LINE__, status);
          return;
        }
      } break;

      case RSI_BLE_SMP_FAILED_EVENT: {
        LOG_PRINT("\n In SMP failed event, str_remote_address: %s\r\n", str_remote_address);

        //! initiate SMP protocol as a Central

        //! clear the served event
        rsi_ble_app_clear_event(RSI_BLE_SMP_FAILED_EVENT);
      } break;

      case RSI_BLE_ENCRYPT_STARTED_EVENT: {
        LOG_PRINT("\r\n In encrypt start event\r\n");
        //! start the encrypt event

        //! clear the served event
        rsi_ble_app_clear_event(RSI_BLE_ENCRYPT_STARTED_EVENT);
      } break;
      case RSI_BLE_ENHANCE_CONNECTED_EVENT: {
        //! remote device enhance connected event
        LOG_PRINT("\r\n In enhance connect event\r\n");

        //! clear the enhance connected event.
        rsi_ble_app_clear_event(RSI_BLE_ENHANCE_CONNECTED_EVENT);

      } break;
      case RSI_APP_EVENT_DATA_LENGTH_CHANGE: {
        LOG_PRINT("\r\n In data length change event\r\n");

        LOG_PRINT("Max_tx_octets: %d \r\n", data_length_update.MaxTxOctets);
        LOG_PRINT("Max_tx_time: %d \r\n", data_length_update.MaxTxTime);
        LOG_PRINT("Max_rx_octets: %d \r\n", data_length_update.MaxRxOctets);
        LOG_PRINT("Max_rx_time: %d \r\n", data_length_update.MaxRxTime);

        rsi_ble_app_clear_event(RSI_APP_EVENT_DATA_LENGTH_CHANGE);
      } break;
      case RSI_APP_EVENT_PHY_UPDATE_COMPLETE: {
        LOG_PRINT("\r\n In physical update complete event\r\n");
        //! phy update complete event

        //! clear the phy updare complete event.
        rsi_ble_app_clear_event(RSI_APP_EVENT_PHY_UPDATE_COMPLETE);
      } break;
      case RSI_APP_EVENT_CONN_UPDATE_COMPLETE: {
        LOG_PRINT("\r\n In connection update complete event\r\n");
        //! conn update complete event

        //! clear the conn updare complete event.
        rsi_ble_app_clear_event(RSI_APP_EVENT_CONN_UPDATE_COMPLETE);
      } break;
      case RSI_BLE_RECEIVE_REMOTE_FEATURES: {
        LOG_PRINT("\r\n In BLE receive remote features event\r\n");
        rsi_ble_app_clear_event(RSI_BLE_RECEIVE_REMOTE_FEATURES);

        if (first_connect == 0) {
          //! initiating the SMP pairing process
          status        = rsi_ble_smp_pair_request(remote_dev_bd_addr, RSI_BLE_SMP_IO_CAPABILITY, MITM_REQ);
          first_connect = 1;
        }
      } break;
      case RSI_APP_EVENT_REMOTE_CONN_PARAM_REQ: {
        LOG_PRINT("\r\n In Remote connection parameters request event\r\n");
        //! clear the remote conn params event.
        rsi_ble_app_clear_event(RSI_APP_EVENT_REMOTE_CONN_PARAM_REQ);

        LOG_PRINT("\tREMOTE DEVICE ADDRESS:%s\n",
                  rsi_6byte_dev_address_to_ascii(str_remote_address, rsi_app_remote_device_conn_params.dev_addr));
        LOG_PRINT("\tCONN PARAMS INFORMATION:\n");
        LOG_PRINT("\tCONN INTERVAL MIN:0x%04x\n", rsi_app_remote_device_conn_params.conn_interval_min);
        LOG_PRINT("\tCONN INTERVAL MAX:0x%04x\n", rsi_app_remote_device_conn_params.conn_interval_max);
        LOG_PRINT("\tCONN LATENCY:0x%04x\n", rsi_app_remote_device_conn_params.conn_latency);
        LOG_PRINT("\tSUPERVISION TIMEOUT:0x%04x\n", rsi_app_remote_device_conn_params.timeout);

        //! Default ACCEPT the remote conn params request(0-ACCEPT, 1-REJECT)
        status = rsi_ble_conn_param_resp(rsi_app_remote_device_conn_params.dev_addr, 0);
        if (status != RSI_SUCCESS) {
          LOG_PRINT("\n Conn param resp status: 0x%lx\r\n", status);
        }
      } break;
      default: {
      }
    }
  }
}

void app_init(void)
{
  osThreadNew((osThreadFunc_t)ble_privacy_app, NULL, &thread_attributes);
}
