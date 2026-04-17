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
/**
 * @file    app.c
 * @version 0.1
 *
 *  @brief : This file contains example application for WLAN BLE coex power save.
 *
 *  @section Description  This file contains example application for WLAN BLE coex power save. 
 */

/**
 * Include files
 **/
//! SL Wi-Fi SDK includes
#include "sl_board_configuration.h"
#include "sl_wifi.h"
#include "sl_wifi_callback_framework.h"
#include "cmsis_os2.h"

// BLE Specific inclusions
#include "ble_config.h"
#include <rsi_ble_apis.h>
#include "rsi_ble_common_config.h"
#include <rsi_common_apis.h>
#ifdef SLI_SI91X_MCU_INTERFACE
#include "sl_si91x_power_manager.h"
#include "sl_si91x_m4_ps.h"
#include "sl_si91x_driver_gpio.h"
#endif
#include "wifi_app.h"
#include "sl_utility.h"

static const sl_wifi_device_configuration_t config = {
  .boot_option = LOAD_NWP_FW,
  .mac_address = NULL,
  .band        = SL_SI91X_WIFI_BAND_2_4GHZ,
  .region_code = US,
  .boot_config = { .oper_mode              = SL_SI91X_CLIENT_MODE,
                   .coex_mode              = SL_SI91X_WLAN_BLE_MODE,
                   .feature_bit_map        = (SL_WIFI_FEAT_WPS_DISABLE | SL_SI91X_FEAT_ULP_GPIO_BASED_HANDSHAKE
                                       | SL_SI91X_FEAT_DEV_TO_HOST_ULP_GPIO_1),
                   .tcp_ip_feature_bit_map = (SL_SI91X_TCP_IP_FEAT_DHCPV4_CLIENT | SL_SI91X_TCP_IP_FEAT_EXTENSION_VALID
                                              | SL_SI91X_TCP_IP_FEAT_DNS_CLIENT | SL_SI91X_TCP_IP_FEAT_SSL),
                   .custom_feature_bit_map = (SL_WIFI_SYSTEM_CUSTOM_FEAT_EXTENSION_VALID),
                   .ext_custom_feature_bit_map =
                     (SL_WIFI_SYSTEM_EXT_FEAT_LOW_POWER_MODE | SL_SI91X_EXT_FEAT_XTAL_CLK | MEMORY_CONFIG
#ifdef SLI_SI917
                      | SL_SI91X_EXT_FEAT_FRONT_END_SWITCH_PINS_ULP_GPIO_4_5_0
#endif
                      | SL_SI91X_EXT_FEAT_BT_CUSTOM_FEAT_ENABLE // Enable BT feature
                      ),
                   .ext_tcp_ip_feature_bit_map =
                     (SL_SI91X_CONFIG_FEAT_EXTENTION_VALID | SL_SI91X_EXT_TCP_IP_TOTAL_SELECTS(10)),
                   .bt_feature_bit_map = (SL_SI91X_BT_RF_TYPE | SL_SI91X_ENABLE_BLE_PROTOCOL),
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

uint8_t volatile ble_connection_done = 0, wlan_socket_connection_done = 0;
int32_t no_ble_events = 0;
extern wifi_app_cb_t wifi_app_cb;
bool button_is_pressed = 0;

#if SL_SI91X_TICKLESS_MODE
osSemaphoreId_t data_received_semaphore;
#endif

//! Function prototypes
extern void rsi_wlan_app_task(void);
extern void rsi_ble_app_init(void);
extern void rsi_ble_app_task(void);
sl_status_t wifi_app_event_handler(sl_wifi_event_t event, sl_wifi_buffer_t *buffer);
extern int32_t rsi_ble_app_get_event(void);

void rsi_wlan_ble_app(void *argument)
{
  UNUSED_PARAMETER(argument);
  int32_t status                     = SL_STATUS_OK;
  sl_wifi_firmware_version_t version = { 0 };
#if SL_SI91X_TICKLESS_MODE
  data_received_semaphore = osSemaphoreNew(1, 0, NULL);
  osStatus_t rc           = 0;
#endif

  //! Wi-Fi initialization
  status = sl_wifi_init(&config, NULL, sl_wifi_default_event_handler);
  if (status != SL_STATUS_OK) {
    LOG_PRINT("\r\nWi-Fi Initialization Failed, Error Code : 0x%lX\r\n", status);
    return;
  }
  LOG_PRINT("\r\n Wi-Fi initialization is successful\n");

  //! Firmware version Prints
  status = sl_wifi_get_firmware_version(&version);
  if (status != SL_STATUS_OK) {
    LOG_PRINT("\r\nFailed to fetch firmware version: 0x%lx\r\n", status);
  } else {
    print_firmware_version(&version);
  }

  //! BLE initialization
  rsi_ble_app_init();

  while (1) {
    //! WLAN application tasks
    rsi_wlan_app_task();

    //! BLE application tasks
    rsi_ble_app_task();

#ifdef SLI_SI91X_MCU_INTERFACE
#if ENABLE_NWP_POWER_SAVE
#ifndef BLE_ADVERTISE_ONLY
    if ((wlan_socket_connection_done == 1) && (ble_connection_done == 1)) {
#else
    if (wlan_socket_connection_done == 1) {
#endif
      no_ble_events = rsi_ble_app_get_event();
      if (!(wifi_app_cb.event_map & RSI_SEND_EVENT) && (no_ble_events == -1)) {
#if (SL_SI91X_TICKLESS_MODE == 0)
        if ((!(P2P_STATUS_REG & TA_wakeup_M4))) {
          P2P_STATUS_REG &= ~M4_wakeup_TA;
          LOG_PRINT("M4 in Sleep\r\n");
          sl_si91x_power_manager_sleep();
          LOG_PRINT("M4 wake up\r\n");
#if (ENABLE_ALARM || ENABLE_NPSS_GPIO_2)
          wifi_ble_send_data();
#endif
        }
#else
        LOG_PRINT("M4 in Sleep\r\n");
        rc = osSemaphoreAcquire(data_received_semaphore, osWaitForever);
        LOG_PRINT("M4 wake up\r\n");
        if (button_is_pressed == true) {
          wifi_ble_send_data();
          button_is_pressed = 0;
        }
        if (rc == osErrorTimeout) {
          wifi_ble_send_data();
        }
#endif
      }
    }
#endif
#endif
  }

  return;
}

void app_init(void)
{
  osThreadNew((osThreadFunc_t)rsi_wlan_ble_app, NULL, &thread_attributes);
  return;
}
