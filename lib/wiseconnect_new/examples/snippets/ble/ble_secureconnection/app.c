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
 * @brief : This file contains example application for BLE Secure Connections
 * @section Description :
 * This application demonstrates how to configure the SiLabs device in Central
 * mode and connects with remote peripheral device and how to enable SMP (Security
 * Manager Protocol) pairing and a Secured passkey pairing.
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
#include <string.h>

//! Common include file
#include "rsi_common_apis.h"
#if SL_SI91X_TICKLESS_MODE == 0 && defined(SLI_SI91X_MCU_INTERFACE)
#include "sl_si91x_m4_ps.h"
#include "sl_si91x_power_manager.h"
#endif

//! local device name
#define RSI_BLE_DEVICE_NAME "BLE_SMP_SC"

//! local device IO capability
#define RSI_BLE_SMP_IO_CAPABILITY 0x00
#define RSI_BLE_SMP_PASSKEY       0

#ifdef RSI_BLE_PING
//! Ping time out
#define RSI_BLE_PING_TIME_OUT 5000
#endif

#define LOCAL_DEV_ADDR_LEN 18 // Length of the local device address

//! application event list
#define RSI_BLE_CONN_EVENT                0x01
#define RSI_BLE_DISCONN_EVENT             0x02
#define RSI_BLE_SMP_REQ_EVENT             0x03
#define RSI_BLE_SMP_RESP_EVENT            0x04
#define RSI_BLE_SMP_PASSKEY_EVENT         0x05
#define RSI_BLE_SMP_FAILED_EVENT          0x06
#define RSI_BLE_ENCRYPT_STARTED_EVENT     0x07
#define RSI_BLE_SMP_PASSKEY_DISPLAY_EVENT 0x08
#define RSI_BLE_SC_PASSKEY_EVENT          0x09
#define RSI_BLE_LTK_REQ_EVENT             0x0A

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

//! global parameters list
static uint8_t str_remote_address[18];
static uint8_t remote_dev_address[6];
static uint32_t numeric_value;
static rsi_bt_event_encryption_enabled_t encrypt_keys;
static rsi_bt_event_le_ltk_request_t ble_ltk_req;
static rsi_bt_event_le_security_keys_t app_ble_sec_keys;
static rsi_bt_event_smp_passkey_display_t smp_passkey_display_event;
#ifdef RSI_BLE_PING
uint16_t timeout_resp;
#endif
osSemaphoreId_t ble_main_task_sem;
static volatile uint32_t ble_app_event_map;
static volatile uint32_t ble_app_event_map1;
#define MITM_REQ 0x01

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
 * @fn         rsi_ble_on_enhance_conn_status_event
 * @brief      invoked when enhanced connection complete event is received
 * @param[out] resp_conn, connected remote device information
 * @return     none.
 * @section description
 * This callback function indicates the status of the connection
 */
void rsi_ble_on_enhance_conn_status_event(rsi_ble_event_enhance_conn_status_t *resp_enh_conn)
{
  memcpy(remote_dev_address, resp_enh_conn->dev_addr, 6);
  rsi_6byte_dev_address_to_ascii(str_remote_address, resp_enh_conn->dev_addr);
  rsi_ble_app_set_event(RSI_BLE_CONN_EVENT);
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
  memcpy(remote_dev_address, resp_conn->dev_addr, 6);
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
  memcpy(remote_dev_address, resp_disconnect->dev_addr, 6);
  rsi_6byte_dev_address_to_ascii(str_remote_address, resp_disconnect->dev_addr);

  rsi_ble_app_set_event(RSI_BLE_DISCONN_EVENT);
}

/*==============================================*/
/**
 * @fn         rsi_ble_on_smp_request 
 * @brief      its invoked when smp request event is received.
 * @param[in]  remote_dev_address, it indicates remote bd address.
 * @return     none.
 * @section description
 * This callback function is invoked when SMP request events is received(we are in Central mode)
 * Note: Peripheral requested to start SMP request, we have to send SMP request command
 */
void rsi_ble_on_smp_request(rsi_bt_event_smp_req_t *event_smp_req)
{
  memcpy(remote_dev_address, event_smp_req->dev_addr, 6);
  rsi_6byte_dev_address_to_ascii(str_remote_address, event_smp_req->dev_addr);
  rsi_ble_app_set_event(RSI_BLE_SMP_REQ_EVENT);
}

/*==============================================*/
/**
 * @fn         rsi_ble_on_smp_response 
 * @brief      its invoked when smp response event is received.
 * @param[in]  remote_dev_address, it indicates remote bd address.
 * @return     none.
 * @section description
 * This callback function is invoked when SMP response events is received(we are in peripheral mode)
 * Note: Central initiated SMP protocol, we have to send SMP response command
 */
void rsi_ble_on_smp_response(rsi_bt_event_smp_resp_t *bt_event_smp)
{
  memcpy(remote_dev_address, bt_event_smp->dev_addr, 6);
  rsi_6byte_dev_address_to_ascii(str_remote_address, bt_event_smp->dev_addr);
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
void rsi_ble_on_smp_passkey(rsi_bt_event_smp_passkey_t *event_smp_passkey)
{
  memcpy(remote_dev_address, event_smp_passkey->dev_addr, 6);
  rsi_6byte_dev_address_to_ascii(str_remote_address, event_smp_passkey->dev_addr);
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
  memcpy(&smp_passkey_display_event, smp_passkey_display, sizeof(rsi_bt_event_smp_passkey_display_t));

  rsi_ble_app_set_event(RSI_BLE_SMP_PASSKEY_DISPLAY_EVENT);
}

void rsi_ble_on_sc_passkey(rsi_bt_event_sc_passkey_t *sc_passkey)
{
  memcpy(remote_dev_address, sc_passkey->dev_addr, 6);
  LOG_PRINT(" Pass_key event remote addr: %s, passkey: %06ld \r\n",
            rsi_6byte_dev_address_to_ascii(str_remote_address, sc_passkey->dev_addr),
            sc_passkey->passkey);
  numeric_value = sc_passkey->passkey;
  rsi_ble_app_set_event(RSI_BLE_SC_PASSKEY_EVENT);
}

/*==============================================*/
/**
 * @fn         rsi_ble_on_le_ltk_req_event
 * @brief      invoked when ltk request event is received
 * @param[in]  le_ltk_req, ltk request parameters.
 * @param[in]  reason, reason for disconnection.
 * @return     none.
 * @section description
 * This callback function indicates disconnected device information and status
 */
static void rsi_ble_on_le_ltk_req_event(rsi_bt_event_le_ltk_request_t *le_ltk_req)
{
  LOG_PRINT(" \r\n rsi_ble_on_le_ltk_req_event \r\n");
  memcpy(&ble_ltk_req, le_ltk_req, sizeof(rsi_bt_event_le_ltk_request_t));
  rsi_ble_app_set_event(RSI_BLE_LTK_REQ_EVENT);
}

/*==============================================*/
/**
 * @fn         rsi_ble_on_le_security_keys
 * @brief      its invoked when le secure keys events is received.
 * @param[in]  rsi_ble_event_le_security_keys, secure event parameters.
 * @return     none.
 * @section description
 * This callback function is invoked when security keys event is received after encription started. 
 */
void rsi_ble_on_le_security_keys(rsi_bt_event_le_security_keys_t *rsi_ble_event_le_security_keys)
{
  memcpy(&app_ble_sec_keys, rsi_ble_event_le_security_keys, sizeof(rsi_bt_event_le_security_keys_t));
  LOG_PRINT("security keys event remote_ediv: 0x%x\r\n", app_ble_sec_keys.remote_ediv);
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
void rsi_ble_on_smp_failed(uint16_t status, rsi_bt_event_smp_failed_t *event_smp_failed)
{
  memcpy(remote_dev_address, event_smp_failed->dev_addr, 6);
  LOG_PRINT("smp_failed status: 0x%x, str_remote_address: %s\r\n",
            status,
            rsi_6byte_dev_address_to_ascii(str_remote_address, event_smp_failed->dev_addr));
  rsi_ble_app_set_event(RSI_BLE_SMP_FAILED_EVENT);
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
  LOG_PRINT("start encrypt status: %d \r\n", status);
  memcpy(&encrypt_keys, enc_enabled, sizeof(rsi_bt_event_encryption_enabled_t));
  rsi_ble_app_set_event(RSI_BLE_ENCRYPT_STARTED_EVENT);
}

/*==============================================*/
/**
 * @fn         ble_smp_test_app
 * @brief      This application is used for testing the SMP protocol.
 * @param[in]  none. 
 * @return     none.
 * @section description
 * This function is used to test the SMP Protocol.
 */
void ble_smp_test_app(void *argument)
{
  UNUSED_PARAMETER(argument);

  int32_t event_id;
  uint8_t adv[31]                = { 2, 1, 6 };
  uint8_t pairing_info_available = 0;
  sl_status_t status;
  sl_wifi_firmware_version_t version                         = { 0 };
  static uint8_t rsi_app_resp_get_dev_addr[RSI_DEV_ADDR_LEN] = { 0 };
  uint8_t local_dev_addr[LOCAL_DEV_ADDR_LEN]                 = { 0 };

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
  rsi_ble_gap_register_callbacks(NULL,
                                 rsi_ble_on_connect_event,
                                 rsi_ble_on_disconnect_event,
                                 NULL,
                                 NULL,
                                 NULL,
                                 rsi_ble_on_enhance_conn_status_event,
                                 NULL,
                                 NULL,
                                 NULL);

  //! registering the SMP callback functions
  rsi_ble_smp_register_callbacks(rsi_ble_on_smp_request,
                                 rsi_ble_on_smp_response,
                                 rsi_ble_on_smp_passkey,
                                 rsi_ble_on_smp_failed,
                                 rsi_ble_on_encrypt_started,
                                 rsi_ble_on_smp_passkey_display,
                                 rsi_ble_on_sc_passkey,
                                 rsi_ble_on_le_ltk_req_event,
                                 rsi_ble_on_le_security_keys,
                                 NULL,
                                 NULL);
  //! create ble main task if ble protocol is selected
  ble_main_task_sem = osSemaphoreNew(1, 0, NULL);
  if (ble_main_task_sem == NULL) {
    LOG_PRINT("Failed to create ble_main_task_sem semaphore\n");
  }

  //!  initializing the application events map
  rsi_ble_app_init_events();

  //! Set local name
  status = rsi_bt_set_local_name((uint8_t *)RSI_BLE_DEVICE_NAME);
  if (status != RSI_SUCCESS) {
    LOG_PRINT("\r\n ble set local name cmd failed with reason code : %lX \n", status);
    return;
  }
  LOG_PRINT("\n Local name set to: %s\n", RSI_BLE_DEVICE_NAME);

  //! prepare advertise data //local/device name
  adv[3] = strlen(RSI_BLE_DEVICE_NAME) + 1;
  adv[4] = 9;
  strcpy((char *)&adv[5], RSI_BLE_DEVICE_NAME);

  //! set advertise data
  rsi_ble_set_advertise_data(adv, strlen(RSI_BLE_DEVICE_NAME) + 5);

  //! start advertising
  status = rsi_ble_start_advertising();
  if (status != RSI_SUCCESS) {
    LOG_PRINT("\r\n Start Advertising Failed, Error Code : 0x%lX\r\n", status);
    return;
  }
  LOG_PRINT("\r\n Start Advertising Success\r\n");

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
      case RSI_BLE_CONN_EVENT: {
        //! event invokes when connection was completed
        LOG_PRINT("\n Enhance connected, str_remote_address : %s\r\n", str_remote_address);

        //! clear the served event
        rsi_ble_app_clear_event(RSI_BLE_CONN_EVENT);
        rsi_6byte_dev_address_to_ascii(str_remote_address, remote_dev_address);
        LOG_PRINT("\r\n Module connected to address : %s \r\n", str_remote_address);

#ifdef RSI_BLE_PING
        //! get ping time out
        rsi_ble_get_le_ping_timeout(remote_dev_address, &timeout_resp);

        //! set le ping time out
        rsi_ble_set_le_ping_timeout(remote_dev_address, RSI_BLE_PING_TIME_OUT);

        //! get ping time out
        rsi_ble_get_le_ping_timeout(remote_dev_address, &timeout_resp);
#endif
        if (!pairing_info_available) {
          //! initiating the SMP pairing process
          status = rsi_ble_smp_pair_request(remote_dev_address, RSI_BLE_SMP_IO_CAPABILITY, MITM_REQ);
          if (status != RSI_SUCCESS) {
            LOG_PRINT("\n smp pair req failed with reason = %lx \n", status);
          }
        }
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
        //! start addvertising
        status = rsi_ble_start_advertising();
        if (status != RSI_SUCCESS) {
          LOG_PRINT("\n start adv cmd failed with reason = %lx \n", status);
        } else {
          LOG_PRINT("\n Start advertising ...\n");
        }

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
        LOG_PRINT("\n smp_req - str_remote_address : %s\r\n", str_remote_address);

        //! initiate SMP protocol as a Central

        pairing_info_available = 0;

        //! clear the served event
        rsi_ble_app_clear_event(RSI_BLE_SMP_REQ_EVENT);

        //! initiating the SMP pairing process
        status = rsi_ble_smp_pair_request(remote_dev_address, RSI_BLE_SMP_IO_CAPABILITY, MITM_REQ);
        if (status != RSI_SUCCESS) {
          LOG_PRINT("\n smp pair req failed with reason = %lx \n", status);
        }
      } break;

      case RSI_BLE_SMP_RESP_EVENT: {
        LOG_PRINT("\n smp_resp - str_remote_address : %s\r\n", str_remote_address);

        //! initiate SMP protocol as a Central

        //! clear the served event
        rsi_ble_app_clear_event(RSI_BLE_SMP_RESP_EVENT);

        pairing_info_available = 0;

        //! initiating the SMP pairing process
        status = rsi_ble_smp_pair_response(remote_dev_address, RSI_BLE_SMP_IO_CAPABILITY, MITM_REQ);
        if (status != RSI_SUCCESS) {
          LOG_PRINT("\n smp pair resp failed with reason = %lx \n", status);
        }
      } break;

      case RSI_BLE_SMP_PASSKEY_EVENT: {
        LOG_PRINT("\n smp_passkey event, str_remote_address : %s\r\n", str_remote_address);
        //! initiate SMP protocol as a Central

        //! clear the served event
        rsi_ble_app_clear_event(RSI_BLE_SMP_PASSKEY_EVENT);

        //! initiating the SMP pairing process
        status = rsi_ble_smp_passkey(remote_dev_address, RSI_BLE_SMP_PASSKEY);
        if (status != RSI_SUCCESS) {
          LOG_PRINT("\n smp passkey cmd failed with reason = %lx \n", status);
        }
      } break;
      case RSI_BLE_SMP_PASSKEY_DISPLAY_EVENT: {
        LOG_PRINT("\r\nIn SMP passkey display event\r\n");
        uint8_t passkey[BLE_PASSKEY_SIZE];

        memset(passkey, 0, BLE_PASSKEY_SIZE);

        rsi_6byte_dev_address_to_ascii(str_remote_address, smp_passkey_display_event.dev_addr);
        memcpy(passkey, smp_passkey_display_event.passkey, BLE_PASSKEY_SIZE);

        LOG_PRINT("\n Pass_key display event remote addr: %s, passkey: %s \r\n", str_remote_address, (char *)passkey);
        //! clear the served event
        rsi_ble_app_clear_event(RSI_BLE_SMP_PASSKEY_DISPLAY_EVENT);
      } break;

      case RSI_BLE_SC_PASSKEY_EVENT: {
        LOG_PRINT("\r\n In SC passkey event\r\n");
        rsi_ble_app_clear_event(RSI_BLE_SC_PASSKEY_EVENT);
        status = rsi_ble_smp_passkey(remote_dev_address, numeric_value);
        if (status != RSI_SUCCESS) {
          LOG_PRINT("\n smp passkey cmd failed with reason = %lx \n", status);
        }
      } break;

      case RSI_BLE_LTK_REQ_EVENT: {
        LOG_PRINT("\r\n In LTK request event\r\n");
        rsi_ble_app_clear_event(RSI_BLE_LTK_REQ_EVENT);
        if (pairing_info_available) {
          status = rsi_ble_ltk_req_reply(remote_dev_address,
                                         (1 | encrypt_keys.enabled | (encrypt_keys.sc_enable << 7)),
                                         encrypt_keys.localltk);
          if (status != RSI_SUCCESS) {
            LOG_PRINT("\n ltk req reply cmd failed with reason = %lx \n", status);
          }
        } else {
          rsi_ble_ltk_req_reply(remote_dev_address, 0, NULL);
          if (status != RSI_SUCCESS) {
            LOG_PRINT("\n ltk negative req reply cmd failed with reason = %lx \n", status);
          }

          /* restarting the SMP */
          status = rsi_ble_smp_pair_request(remote_dev_address, RSI_BLE_SMP_IO_CAPABILITY, MITM_REQ);
          if (status != RSI_SUCCESS) {
            LOG_PRINT("\n smp pair req failed with reason = %lx \n", status);
          } else {
            pairing_info_available = 0;
          }
        }
      } break;

      case RSI_BLE_SMP_FAILED_EVENT: {
        LOG_PRINT("\r\n In SMP failed event\r\n");
        //! initiate SMP protocol as a Central

        pairing_info_available = 0;
        //! clear the served event
        rsi_ble_app_clear_event(RSI_BLE_SMP_FAILED_EVENT);
      } break;

      case RSI_BLE_ENCRYPT_STARTED_EVENT: {
        LOG_PRINT("\r\n In encrypt started event\r\n");
        //! start the encrypt event

        pairing_info_available = 1;
        //! clear the served event
        rsi_ble_app_clear_event(RSI_BLE_ENCRYPT_STARTED_EVENT);
      } break;
      default: {
      }
    }
  }
}

void app_init(void)
{
  osThreadNew((osThreadFunc_t)ble_smp_test_app, NULL, &thread_attributes);
}
