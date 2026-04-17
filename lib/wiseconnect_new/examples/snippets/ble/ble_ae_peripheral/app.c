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
 * @brief : This file contains example application for BLE Heart Rate Profile
 * @section Description :
 * This application demonstrates how to configure Heart rate as GATT server in
 * BLE peripheral mode and explains how to do indicate operation with GATT server
 * from connected remote device using GATT client.
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

//! BLE include file to refer BLE APIs
#include <string.h>
#include <stdio.h>

#include "ble_config.h"
#include "rsi_ble.h"
#include "rsi_ble_apis.h"
#include "rsi_ble_common_config.h"
#include "rsi_bt_common.h"
#include "rsi_bt_common_apis.h"
#include "rsi_common_apis.h"

#define RSI_BLE_LOCAL_NAME  "AE_PERIPHERAL"
#define BLE_AE_ADV_DATA     "AE_PERIPHERAL_DATA_1"
#define BLE_AE_ADV_DATA_LEN 0x19
#define RSI_SEL_ANTENNA     RSI_SEL_INTERNAL_ANTENNA //RSI_SEL_EXTERNAL_ANTENNA
#define RSI_SCAN_RESP_DATA  "REDPINE_AE_DEVICE"
#define ADV_SET2            1

#define REM_DEV_ADDR "00:00:00:00:00:00"

#define PERIODIC_ADV_EN 0
#define BLE_ADV_HNDL1   0x00

#define LOCAL_DEV_ADDR_LEN 18 // Length of the local device address

uint8_t adv[BLE_AE_ADV_DATA_LEN] = { 2, 1, 6 };

//! Address type of the device to connect
#define RSI_BLE_ACCEPTLIST_DEV_ADDR1_TYPE LE_RANDOM_ADDRESS

//! Address of the device to add in accept list
uint8_t ble_acceptlist_addr1[BLE_ACCEPT_LIST_ADDR_LEN] = { 0xDA, 0x81, 0xE0, 0xB8, 0xB9, 0x42 };

//! application events list
#define RSI_APP_EVENT_CONNECTED          1
#define RSI_APP_EVENT_DISCONNECTED       2
#define RSI_APP_EVENT_EXT_ADV_TERMINATED 3
#define RSI_APP_EVENT_SCAN_REQ_RECEIVED  4
/*=======================================================================*/
//!    Application powersave configurations
/*=======================================================================*/
#define ENABLE_NWP_POWER_SAVE 1 //! Set to 1 for powersave mode

#if ENABLE_NWP_POWER_SAVE
//! Power Save Profile Mode
#define PSP_MODE RSI_SLEEP_MODE_2
//! Power Save Profile type
#define PSP_TYPE RSI_MAX_PSP
sl_wifi_performance_profile_v2_t wifi_profile = { .profile = ASSOCIATED_POWER_SAVE_LOW_LATENCY };
#endif

//! Application global parameters.
static rsi_bt_resp_get_local_name_t rsi_app_resp_get_local_name = { 0 };
static uint8_t rsi_app_resp_get_dev_addr[RSI_DEV_ADDR_LEN]      = { 0 };
static uint32_t rsi_app_async_event_map                         = 0;
static rsi_ble_event_conn_status_t rsi_app_connected_device     = { 0 };
static rsi_ble_event_disconnect_t rsi_app_disconnected_device   = { 0 };
static rsi_ble_adv_set_terminated_t adv_set_terminated          = { 0 };
static rsi_ble_scan_req_recvd_t scan_req_recvd                  = { 0 };

osSemaphoreId_t ble_main_task_sem;
int8_t rsi_app_resp_tx_power                  = 0;
uint32_t rsi_app_resp_max_no_of_supp_adv_sets = 0;
static uint16_t rsi_app_resp_max_adv_data_len = 0;

static const sl_wifi_device_configuration_t config = {
  .boot_option = LOAD_NWP_FW,
  .mac_address = NULL,
  .band        = SL_SI91X_WIFI_BAND_2_4GHZ,
  .region_code = US,
  .boot_config = { .oper_mode = SL_SI91X_CLIENT_MODE,
                   .coex_mode = SL_SI91X_WLAN_BLE_MODE,
#ifdef SLI_SI91X_MCU_INTERFACE
                   .feature_bit_map = (SL_WIFI_FEAT_WPS_DISABLE | SL_SI91X_FEAT_ULP_GPIO_BASED_HANDSHAKE
                                       | SL_SI91X_FEAT_DEV_TO_HOST_ULP_GPIO_1),
#else
                   .feature_bit_map = SL_SI91X_FEAT_ULP_GPIO_BASED_HANDSHAKE | SL_SI91X_FEAT_DEV_TO_HOST_ULP_GPIO_1,
#endif
#if RSI_DISABLE
                   .tcp_ip_feature_bit_map = SL_SI91X_TCP_IP_FEAT_DHCPV4_CLIENT
#else
                   .tcp_ip_feature_bit_map = (SL_SI91X_TCP_IP_FEAT_DHCPV4_CLIENT | SL_SI91X_TCP_IP_FEAT_EXTENSION_VALID)
#endif
                                             | SL_SI91X_TCP_IP_FEAT_DNS_CLIENT | SL_SI91X_TCP_IP_FEAT_SSL,
                   .custom_feature_bit_map =
                     (SL_WIFI_SYSTEM_CUSTOM_FEAT_EXTENSION_VALID | SL_WIFI_SYSTEM_CUSTOM_FEAT_EXTENSION_VALID),
                   .ext_custom_feature_bit_map =
                     (SL_SI91X_EXT_FEAT_XTAL_CLK | SL_WIFI_SYSTEM_EXT_FEAT_LOW_POWER_MODE | MEMORY_CONFIG
#ifdef SLI_SI917
                      | SL_SI91X_EXT_FEAT_FRONT_END_SWITCH_PINS_ULP_GPIO_4_5_0
#endif
                      | SL_SI91X_EXT_FEAT_BT_CUSTOM_FEAT_ENABLE),
#ifdef RSI_PROCESS_MAX_RX_DATA
                   .ext_tcp_ip_feature_bit_map =
                     (SL_SI91X_CONFIG_FEAT_EXTENTION_VALID | SL_SI91X_EXT_TCP_MAX_RECV_LENGTH),
#else
                   .ext_tcp_ip_feature_bit_map = SL_SI91X_CONFIG_FEAT_EXTENTION_VALID,
#endif
                   .bt_feature_bit_map = ((SL_SI91X_BT_RF_TYPE | SL_SI91X_ENABLE_BLE_PROTOCOL)),
                   /*Enable BLE custom feature bitmap*/
                   .ble_feature_bit_map =
                     (SL_SI91X_BLE_MAX_NBR_PERIPHERALS(RSI_BLE_MAX_NBR_PERIPHERALS)
                      | SL_SI91X_BLE_MAX_NBR_CENTRALS(RSI_BLE_MAX_NBR_CENTRALS)
                      | SL_SI91X_BLE_MAX_NBR_ATT_SERV(RSI_BLE_MAX_NBR_ATT_SERV)
                      | SL_SI91X_BLE_MAX_NBR_ATT_REC(RSI_BLE_MAX_NBR_ATT_REC)
                      | SL_SI91X_FEAT_BLE_CUSTOM_FEAT_EXTENTION_VALID | SL_SI91X_BLE_PWR_INX(RSI_BLE_PWR_INX)
                      | SL_SI91X_BLE_PWR_SAVE_OPTIONS(RSI_BLE_PWR_SAVE_OPTIONS)
                      | SL_SI91X_916_BLE_COMPATIBLE_FEAT_ENABLE
#if RSI_BLE_GATT_ASYNC_ENABLE
                      | SL_SI91X_BLE_GATT_ASYNC_ENABLE
#endif
                      ),
                   .ble_ext_feature_bit_map =
                     (SL_SI91X_BLE_NUM_CONN_EVENTS(RSI_BLE_NUM_CONN_EVENTS)
                      | SL_SI91X_BLE_NUM_REC_BYTES(RSI_BLE_NUM_REC_BYTES)
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
#if RSI_BLE_ENABLE_ADV_EXTN
                      | SL_SI91X_BLE_ENABLE_ADV_EXTN
#endif
#if RSI_BLE_AE_MAX_ADV_SETS
                      | SL_SI91X_BLE_AE_MAX_ADV_SETS(RSI_BLE_AE_MAX_ADV_SETS)
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
  .priority   = 0,
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
  rsi_app_async_event_map = 0;
  return;
}

/*==============================================*/
/**
 * @fn         rsi_ble_app_set_event
 * @brief      sets the specific event.
 * @param[in]  event_num, specific event number.
 * @return     none.
 * @section description
 * This function is used to set/raise the specific event.
 */
static void rsi_ble_app_set_event(uint32_t event_num)
{
  rsi_app_async_event_map |= BIT(event_num);
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
  rsi_app_async_event_map &= ~BIT(event_num);
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

  for (ix = 0; ix < 32; ix++) {
    if (rsi_app_async_event_map & (1 << ix)) {
      return ix;
    }
  }

  return (RSI_FAILURE);
}

/*==============================================*/
/**
 * @fn         rsi_ble_simple_peripheral_on_conn_status_event
 * @brief      invoked when connection complete event is received
 * @param[out] resp_conn, connected remote device information
 * @return     none.
 * @section description
 * This callback function indicates the status of the connection
 */
void rsi_ble_simple_peripheral_on_conn_status_event(rsi_ble_event_conn_status_t *resp_conn)
{
  memcpy(&rsi_app_connected_device, resp_conn, sizeof(rsi_ble_event_conn_status_t));
  rsi_ble_app_set_event(RSI_APP_EVENT_CONNECTED);
}

/*==============================================*/
/**
 * @fn         rsi_ble_simple_peripheral_on_disconnect_event
 * @brief      invoked when disconnection event is received
 * @param[in]  resp_disconnect, disconnected remote device information
 * @param[in]  reason, reason for disconnection.
 * @return     none.
 * @section description
 * This callback function indicates disconnected device information and status
 */
void rsi_ble_simple_peripheral_on_disconnect_event(rsi_ble_event_disconnect_t *resp_disconnect, uint16_t reason)
{
  LOG_PRINT("\n Disconnect callback received with status 0x%X \n", reason);
  memcpy(&rsi_app_disconnected_device, resp_disconnect, sizeof(rsi_ble_event_disconnect_t));
  rsi_ble_app_set_event(RSI_APP_EVENT_DISCONNECTED);
}

/*==============================================*/
/**
 * @fn         rsi_ble_simple_peripheral_on_enhance_conn_status_event
 * @brief      invoked when enhanced connection complete event is received
 * @param[out] resp_conn, connected remote device information
 * @return     none.
 * @section description
 * This callback function indicates the status of the connection
 */
void rsi_ble_simple_peripheral_on_enhance_conn_status_event(rsi_ble_event_enhance_conn_status_t *resp_enh_conn)
{
  rsi_app_connected_device.dev_addr_type = resp_enh_conn->dev_addr_type;
  memcpy(rsi_app_connected_device.dev_addr, resp_enh_conn->dev_addr, RSI_DEV_ADDR_LEN);
  rsi_app_connected_device.status = resp_enh_conn->status;
  rsi_ble_app_set_event(RSI_APP_EVENT_CONNECTED);
}

/*==============================================*/
/**
 * @fn         ble_extended_advertise_terminated_callback 
 * @brief      invoked when enhanced connection complete event is received
 * @param[out] resp_conn, connected remote device information
 * @return     none.
 * @section description
 * This callback function indicates the status of the connection
 */
void ble_extended_advertise_terminated_callback(uint16_t status, rsi_ble_adv_set_terminated_t *resp_terminated)
{
  LOG_PRINT("\n Terminated callback received with status 0x%X \n", status);
  adv_set_terminated.adv_handle              = resp_terminated->adv_handle;
  adv_set_terminated.status                  = resp_terminated->status;
  adv_set_terminated.num_completed_ae_events = resp_terminated->num_completed_ae_events;
  adv_set_terminated.conn_handle             = resp_terminated->conn_handle;
  rsi_ble_app_set_event(RSI_APP_EVENT_EXT_ADV_TERMINATED);
}

/*==============================================*/
/**
 * @fn         ble_extended_scan_request_received_callback
 * @brief      invoked when enhanced connection complete event is received
 * @param[out] resp_conn, connected remote device information
 * @return     none.
 * @section description
 * This callback function indicates the status of the connection
 */
void ble_extended_scan_request_received_callback(uint16_t status, rsi_ble_scan_req_recvd_t *resp_scan_received)
{
  LOG_PRINT("\n Scan request callback received with status 0x%X \n", status);
  scan_req_recvd.adv_handle        = resp_scan_received->adv_handle;
  scan_req_recvd.scanner_addr_type = resp_scan_received->scanner_addr_type;
  memcpy(scan_req_recvd.scanner_addr, resp_scan_received->scanner_addr, RSI_DEV_ADDR_LEN);
  rsi_ble_app_set_event(RSI_APP_EVENT_SCAN_REQ_RECEIVED);
}

/*==============================================*/
/**
 * @fn         rsi_ble_peripheral
 * @brief      Tests the BLE GAP peripheral role.
 * @param[in]  none
 * @return    none.
 * @section description
 * This function is used to test the BLE peripheral role and simple GAP API's.
 */
void rsi_ble_peripheral(void)
{
  int32_t status                             = 0;
  int32_t temp_event_map                     = 0;
  uint8_t remote_dev_addr[18]                = { 0 };
  uint8_t rand_addr[RSI_DEV_ADDR_LEN]        = { 0 };
  uint8_t local_dev_addr[LOCAL_DEV_ADDR_LEN] = { 0 };
  rsi_ascii_dev_address_to_6bytes_rev((uint8_t *)rand_addr, (int8_t *)RSI_BLE_SET_RAND_ADDR);
  uint8_t data[BLE_AE_ADV_DATA_LEN] = { 0 };
  strncpy((char *)data, BLE_AE_ADV_DATA, BLE_AE_ADV_DATA_LEN);
  status = sl_wifi_init(&config, NULL, sl_wifi_default_event_handler);
  if (status != SL_STATUS_OK) {
    LOG_PRINT("\r\nWi-Fi Initialization Failed, Error Code : 0x%lX\r\n", status);
    return;
  } else {
    LOG_PRINT("\r\n Wi-Fi Initialization Success\n");
  }
  //! BLE register GAP callbacks
  rsi_ble_gap_register_callbacks(NULL,
                                 rsi_ble_simple_peripheral_on_conn_status_event,
                                 rsi_ble_simple_peripheral_on_disconnect_event,
                                 NULL,
                                 NULL,
                                 NULL,
                                 rsi_ble_simple_peripheral_on_enhance_conn_status_event,
                                 NULL,
                                 NULL,
                                 NULL);
  //! create ble main task if ble protocol is selected
  status = rsi_ble_adv_ext_events_register_callbacks(RSI_BLE_ON_ADV_EXT_ADVERTISE_SET_TERMINATED_EVENT,
                                                     (void *)ble_extended_advertise_terminated_callback);

  if (status != RSI_SUCCESS) {
    LOG_PRINT("\n RSI_BLE_ON_ADV_EXT_ADVERTISE_SET_TERMINATED_EVENT callback registering failed \n");
  } else {
    LOG_PRINT("\n RSI_BLE_ON_ADV_EXT_ADVERTISE_SET_TERMINATED_EVENT callback registering success \n");
  }
  status = rsi_ble_adv_ext_events_register_callbacks(RSI_BLE_ON_ADV_EXT_SCAN_REQUEST_RECEIVED_EVENT,
                                                     (void *)ble_extended_scan_request_received_callback);
  if (status != RSI_SUCCESS) {
    LOG_PRINT("\n RSI_BLE_ON_ADV_EXT_SCAN_REQUEST_RECEIVED_EVENT callback registering failed \n");
  } else {
    LOG_PRINT("\n RSI_BLE_ON_ADV_EXT_SCAN_REQUEST_RECEIVED_EVENT callback registering success \n");
  }
  //! initialize the event map
  //ble_main_task_sem = osSemaphoreNew(1, 0, NULL);
  //! initialize the event map
  rsi_ble_app_init_events();

  status = rsi_ble_addto_acceptlist((int8_t *)ble_acceptlist_addr1, RSI_BLE_ACCEPTLIST_DEV_ADDR1_TYPE);
  if (status != RSI_SUCCESS) {
    return;
  }

  //! get the local device MAC address.
  status = rsi_bt_get_local_device_address(rsi_app_resp_get_dev_addr);
  if (status != RSI_SUCCESS) {
    LOG_PRINT("\r\n rsi_bt_get_local_device_address failed with 0x%lX \r\n", status);
    return;
  } else {
    rsi_6byte_dev_address_to_ascii(local_dev_addr, rsi_app_resp_get_dev_addr);
    LOG_PRINT("\r\n Local device address %s \r\n", local_dev_addr);
  }

  //! set the local device name
  status = rsi_bt_set_local_name((uint8_t *)RSI_BLE_LOCAL_NAME);
  if (status != RSI_SUCCESS) {
    LOG_PRINT("\n rsi_bt_set_local_name failed with 0x%lX \n", status);
  }

  //! get the local device name
  status = rsi_bt_get_local_name(&rsi_app_resp_get_local_name);
  if (status != RSI_SUCCESS) {
    LOG_PRINT("\n rsi_bt_get_local_name failed with 0x%lX \n", status);
  } else {
    LOG_PRINT(" \n get_local_name done \n");
  }

  adv[3] = strlen(RSI_BLE_LOCAL_NAME) + 1;
  adv[4] = 9;
  strcpy((char *)&adv[5], RSI_BLE_LOCAL_NAME);

  //! get the supported max adv data len
  status = rsi_ble_get_max_adv_data_len((uint8_t *)&rsi_app_resp_max_adv_data_len);

  if (status != RSI_SUCCESS) {
    LOG_PRINT("get max adv data length failed with 0x%lX \n", status);
    return;
  } else {
    LOG_PRINT("Max supported Adv Data length is %d \n", (uint8_t)rsi_app_resp_max_adv_data_len);
  }

  //! get the Max no.of supported adv sets
  status = rsi_ble_get_max_no_of_supp_adv_sets((uint8_t *)&rsi_app_resp_max_no_of_supp_adv_sets);
  if (status != RSI_SUCCESS) {
    LOG_PRINT("get max supported adv sets failed with 0x%lX\n", status);
    return;
  } else {
    LOG_PRINT("Max number of supported Adv sets are %d  \n", (uint8_t)rsi_app_resp_max_no_of_supp_adv_sets);
  }

  //! set ae adv params
  rsi_ble_ae_adv_params_t ble_ae_params = { 0 };
  uint8_t peer_addr[RSI_DEV_ADDR_LEN]   = { 0 };

  ble_ae_params.adv_handle                = BLE_ADV_HNDL1;
  ble_ae_params.adv_event_prop            = BLE_AE_ADV_EVNT_PROP;
  ble_ae_params.primary_adv_intterval_min = RSI_BLE_ADV_INT_MIN; //140 only 3bytes are valid;
  ble_ae_params.primary_adv_intterval_max = RSI_BLE_ADV_INT_MAX; //160 only 3bytes are valid;
  ble_ae_params.primary_adv_chnl_map      = RSI_BLE_ADV_CHANNEL_MAP;
  ble_ae_params.own_addr_type             = LE_RANDOM_ADDRESS;
  ble_ae_params.peer_addr_type            = LE_RANDOM_ADDRESS;
  rsi_ascii_dev_address_to_6bytes_rev((uint8_t *)peer_addr, (int8_t *)REM_DEV_ADDR);
  memcpy(ble_ae_params.peer_dev_addr, peer_addr, RSI_DEV_ADDR_LEN);
  ble_ae_params.adv_filter_policy      = BLE_AE_ADV_FILTER_POLICY;
  ble_ae_params.adv_tx_power           = BLE_AE_ADV_TX_PWR;
  ble_ae_params.primary_adv_phy        = PRIMARY_ADV_PHY;
  ble_ae_params.sec_adv_max_skip       = BLE_AE_SEC_ADV_MAX_SKIP;
  ble_ae_params.sec_adv_phy            = SECONDARY_ADV_PHY;
  ble_ae_params.adv_sid                = BLE_AE_ADV_SID;
  ble_ae_params.scan_req_notify_enable = SCAN_REQ_NOTIF_EN;

  LOG_PRINT("\nBLE_AE_ADV_EVNT_PROP = %x \n", BLE_AE_ADV_EVNT_PROP);

  status = rsi_ble_set_ae_params(&ble_ae_params, &rsi_app_resp_tx_power);
  if (status != RSI_SUCCESS) {
    LOG_PRINT("set ae params failed with 0x%lX \n", status);
    return;
  } else {
    LOG_PRINT("Setting AE params successful and selected TX Power is %d dbm \n", rsi_app_resp_tx_power);
  }

#if PERIODIC_ADV_EN
  rsi_ble_ae_periodic_adv_params_t ae_periodic_param = { 0 };
  ae_periodic_param.max_interval                     = 0x190;
  ae_periodic_param.min_interval                     = 0x90;
  ae_periodic_param.properties                       = 0x40;
  ae_periodic_param.adv_handle                       = BLE_ADV_HNDL1;
  status                                             = rsi_ble_app_set_periodic_ae_params(&ae_periodic_param);

  if (status != RSI_SUCCESS) {
    LOG_PRINT("set ae Periodic adv params failed with 0x%lX\n", status);
    return status;
  } else {
    LOG_PRINT("set ae periodic adv params success \n");
  }
#endif

#if ADV_SET2
  ble_ae_params.adv_handle                = 0x01;
  ble_ae_params.adv_event_prop            = 0x01; //Connectable;
  ble_ae_params.primary_adv_intterval_min = 0x30; //140 only 3bytes are valid;
  ble_ae_params.primary_adv_intterval_max = 0x30; //160 only 3bytes are valid;
  ble_ae_params.primary_adv_chnl_map      = RSI_BLE_ADV_CHANNEL_MAP;
  ble_ae_params.own_addr_type             = LE_PUBLIC_ADDRESS;
  ble_ae_params.peer_addr_type            = LE_RANDOM_ADDRESS;
  rsi_ascii_dev_address_to_6bytes_rev((uint8_t *)peer_addr, (int8_t *)REM_DEV_ADDR);
  memcpy(ble_ae_params.peer_dev_addr, peer_addr, RSI_DEV_ADDR_LEN);
  ble_ae_params.adv_filter_policy      = BLE_AE_ADV_FILTER_POLICY;
  ble_ae_params.adv_tx_power           = BLE_AE_ADV_TX_PWR;
  ble_ae_params.primary_adv_phy        = PRIMARY_ADV_PHY;
  ble_ae_params.sec_adv_max_skip       = BLE_AE_SEC_ADV_MAX_SKIP;
  ble_ae_params.sec_adv_phy            = SECONDARY_ADV_PHY;
  ble_ae_params.adv_sid                = 0x01;
  ble_ae_params.scan_req_notify_enable = SCAN_REQ_NOTIF_EN;

  LOG_PRINT("\nBLE_AE_ADV_EVNT_PROP = %x \n", BLE_AE_ADV_EVNT_PROP);

  status = rsi_ble_set_ae_params(&ble_ae_params, &rsi_app_resp_tx_power);
  if (status != RSI_SUCCESS) {
    LOG_PRINT("set ae params failed with 0x%lX \n", status);
    return;
  } else {
    LOG_PRINT("Setting AE params of set 2 successful and selected TX Power is %d dbm \n", rsi_app_resp_tx_power);
  }
#endif

  //! set AE set random address
  status = rsi_ble_set_ae_set_random_address(BLE_ADV_HNDL1, rand_addr);
  if (status != RSI_SUCCESS) {
    LOG_PRINT("set ae set random address failed with 0x%lX \n", status);
    return;
  } else {
    LOG_PRINT("set ae set random address successful \n");
  }

#if PERIODIC_ADV_EN
  //setting periodic advertising data
  rsi_ble_ae_data_t ble_ae_data = { 0 };
  ble_ae_data.type              = BLE_AE_PERIODIC_ADV_DATA_TYPE;
  ble_ae_data.adv_handle        = 0x00;
  ble_ae_data.operation         = 0x03;
  ble_ae_data.frag_pref         = 0x00;
  ble_ae_data.data_len          = BLE_AE_ADV_DATA_LEN;
  memcpy(ble_ae_data.data, data, ble_ae_data.data_len);

  status = rsi_ble_set_ae_data(&ble_ae_data);

  if (status != RSI_SUCCESS) {
    LOG_PRINT("set periodic adv data failed with 0x%lX\n", status);
    return status;
  } else {
    LOG_PRINT("set periodic adv data success \n");
  }
#else
  //setting extended advertising data for set 1
  rsi_ble_ae_data_t ble_ae_data = { 0 };

  ble_ae_data.type = BLE_AE_ADV_DATA_TYPE;
  ble_ae_data.adv_handle = BLE_ADV_HNDL1;
  ble_ae_data.operation = 0x00;
  ble_ae_data.frag_pref = 0x00;
  ble_ae_data.data_len = BLE_AE_ADV_DATA_LEN;
  memcpy(ble_ae_data.data, data, ble_ae_data.data_len);

  status = rsi_ble_set_ae_data(&ble_ae_data);

  if (status != RSI_SUCCESS) {
    LOG_PRINT("set ae data failed with 0x%lX\n", status);
    return;
  } else {
    LOG_PRINT("set ae data success \n");
  }

  //setting ae scan response data for set 1
  ble_ae_data.type = BLE_AE_SCAN_RSP_DATA_TYPE;
  ble_ae_data.adv_handle = BLE_ADV_HNDL1;
  ble_ae_data.operation = 0x00;
  ble_ae_data.frag_pref = 0x00;
  ble_ae_data.data_len = BLE_AE_ADV_DATA_LEN;
  memcpy(ble_ae_data.data, data, ble_ae_data.data_len);

  status = rsi_ble_set_ae_data(&ble_ae_data);

  if (status != RSI_SUCCESS) {
    LOG_PRINT("set ae data failed with 0x%lX\n", status);
    return;
  } else {
    LOG_PRINT("set ae data success \n");
  }
#endif
  // Setting ae advertising enable for set 1
  rsi_ble_ae_adv_enable_t ble_ae_adv = { 0 };

  ble_ae_adv.enable        = RSI_BLE_START_ADV;
  ble_ae_adv.no_of_sets    = 1;
  ble_ae_adv.adv_handle    = BLE_ADV_HNDL1;
  ble_ae_adv.duration      = 0;
  ble_ae_adv.max_ae_events = BLE_MAX_AE_EVNTS;

  status = rsi_ble_start_ae_advertising(&ble_ae_adv);
  if (status != RSI_SUCCESS) {
    LOG_PRINT("set ae adv enable failed with status 0x%lX\n", status);
    return;
  } else {
    LOG_PRINT("set ae adv enable success \n");
  }
#if PERIODIC_ADV_EN
  status = rsi_ble_app_set_periodic_ae_enable(1, 0);
  if (status != RSI_SUCCESS) {
    LOG_PRINT("set ae Periodic adv enable failed with 0x%lX\n", status);
    return status;
  } else {
    LOG_PRINT("set ae periodic adv enable success \n");
  }
#endif
#if ADV_SET2
  //setting extended advertising data for set 2

  ble_ae_data.type       = BLE_AE_ADV_DATA_TYPE;
  ble_ae_data.adv_handle = 0x01;
  ble_ae_data.operation  = 0x03;
  ble_ae_data.frag_pref  = 0x00;
  ble_ae_data.data_len   = BLE_AE_ADV_DATA_LEN;
  memcpy(ble_ae_data.data, data, ble_ae_data.data_len);

  status = rsi_ble_set_ae_data(&ble_ae_data);

  if (status != RSI_SUCCESS) {
    LOG_PRINT("set ae data failed with 0x%lX\n", status);
    return;
  } else {
    LOG_PRINT("set ae data success \n");
  }

  //setting ae scan response data
  ble_ae_data.type       = BLE_AE_SCAN_RSP_DATA_TYPE;
  ble_ae_data.adv_handle = 0x01;
  ble_ae_data.operation  = 0x03;
  ble_ae_data.frag_pref  = 0x00;
  ble_ae_data.data_len   = BLE_AE_ADV_DATA_LEN;
  memcpy(ble_ae_data.data, data, ble_ae_data.data_len);

  status = rsi_ble_set_ae_data(&ble_ae_data);

  if (status != RSI_SUCCESS) {
    LOG_PRINT("set ae data failed with 0x%lX\n", status);
    return;
  } else {
    LOG_PRINT("set ae data success \n");
  }
  // ae advertising enable for set 2
  ble_ae_adv.enable        = RSI_BLE_START_ADV;
  ble_ae_adv.no_of_sets    = 1;
  ble_ae_adv.adv_handle    = 0x01;
  ble_ae_adv.duration      = 0;
  ble_ae_adv.max_ae_events = BLE_MAX_AE_EVNTS;

  status = rsi_ble_start_ae_advertising(&ble_ae_adv);
  if (status != RSI_SUCCESS) {
    LOG_PRINT("set ae adv enable failed with status 0x%lX\n", status);
    return;
  } else {
    LOG_PRINT("set ae adv enable success \n");
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
    LOG_PRINT("\r\n Failed to initiate power save in Wi-Fi mode :%ld\r\n", status);
    return;
  }

  LOG_PRINT("\r\n Module is in power save \r\n");
#endif
  while (1) {
    //! Application main loop
    //! checking for received events
    temp_event_map = rsi_ble_app_get_event();
    if (temp_event_map == RSI_FAILURE) {
      //! if events are not received loop will be continued.
      if (ble_main_task_sem) {
        osSemaphoreAcquire(ble_main_task_sem, osWaitForever);
      }
      continue;
    }
    //! if any event is received, it will be served.
    switch (temp_event_map) {
      case RSI_APP_EVENT_CONNECTED: {
        //! remote device connected event

        //! clear the connected event.
        rsi_ble_app_clear_event(RSI_APP_EVENT_CONNECTED);

        //! converts the 6 byte MAC address to string format
        rsi_6byte_dev_address_to_ascii((uint8_t *)remote_dev_addr, rsi_app_connected_device.dev_addr);

        LOG_PRINT("\n Remote device with %s address connected \n", remote_dev_addr);

      } break;

      case RSI_APP_EVENT_DISCONNECTED: {
        //! remote device connected event

        //! clear the disconnected event.
        rsi_ble_app_clear_event(RSI_APP_EVENT_DISCONNECTED);

        rsi_6byte_dev_address_to_ascii((uint8_t *)remote_dev_addr, rsi_app_disconnected_device.dev_addr);

        LOG_PRINT("\n Remote device with %s address disconnected \n", remote_dev_addr);

        //retriggering ae advertiser enable for set 1
        rsi_ble_ae_adv_enable_t ble_ae_adv = { 0 };

        ble_ae_adv.enable        = RSI_BLE_START_ADV;
        ble_ae_adv.no_of_sets    = 1;
        ble_ae_adv.adv_handle    = BLE_ADV_HNDL1;
        ble_ae_adv.duration      = 0;
        ble_ae_adv.max_ae_events = BLE_MAX_AE_EVNTS;

        status = rsi_ble_start_ae_advertising(&ble_ae_adv);
        if (status != RSI_SUCCESS) {
          LOG_PRINT("set ae adv enable failed with 0x%lX \n", status);
        } else {
          LOG_PRINT("set ae adv enable success \n");
        }

      } break;
      case RSI_APP_EVENT_EXT_ADV_TERMINATED: {
        rsi_ble_app_clear_event(RSI_APP_EVENT_EXT_ADV_TERMINATED);
        LOG_PRINT("\n Extended Advertsing terminated event received for adv handle %d , conn handle %d \n",
                  adv_set_terminated.adv_handle,
                  adv_set_terminated.conn_handle);

        //retriggering ae advertiser enable for set 1
        rsi_ble_ae_adv_enable_t ble_ae_adv = { 0 };

        ble_ae_adv.enable        = RSI_BLE_START_ADV;
        ble_ae_adv.no_of_sets    = 1;
        ble_ae_adv.adv_handle    = adv_set_terminated.adv_handle;
        ble_ae_adv.duration      = 0;
        ble_ae_adv.max_ae_events = BLE_MAX_AE_EVNTS;

        status = rsi_ble_start_ae_advertising(&ble_ae_adv);
        if (status != RSI_SUCCESS) {
          LOG_PRINT("set ae adv enable failed with 0x%lX \n", status);
        } else {
          LOG_PRINT("set ae adv enable success \n");
        }
      } break;
      case RSI_APP_EVENT_SCAN_REQ_RECEIVED: {
        rsi_ble_app_clear_event(RSI_APP_EVENT_SCAN_REQ_RECEIVED);
        rsi_6byte_dev_address_to_ascii((uint8_t *)remote_dev_addr, scan_req_recvd.scanner_addr);
        LOG_PRINT("\n Scan request received for handle %d with addr %s \n", scan_req_recvd.adv_handle, remote_dev_addr);
      } break;
    }
  }
}
void app_init(void)
{
  osThreadNew((osThreadFunc_t)rsi_ble_peripheral, NULL, &thread_attributes);
}
