/*******************************************************************************
* @file  app.c
* @brief
*******************************************************************************
* # License
* <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
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
 * @brief : This file contains example application for Wlan Station BLE
 * Provisioning
 * @section Description :
 * This application explains how to get the WLAN connection functionality using
 * BLE provisioning.
 * Silicon Labs Module starts advertising and with BLE Provisioning the Access Point
 * details are fetched.
 * Silicon Labs device is configured as a WiFi station and connects to an Access Point.
 =================================================================================*/

/**
 * Include files
 **/
//! SL Wi-Fi SDK includes
#include "sl_si91x_driver.h"
#include "wifi_config.h"
#include "rsi_ble_common_config.h"
#include <rsi_common_apis.h>
#include "glib.h"
#include "logo_bitmaps.h"
#include "sl_si91x_power_manager.h"

// TCP IP BYPASS feature check
#define RSI_TCP_IP_BYPASS RSI_DISABLE

// Out of box demo application binary version
#define OOB_APP_VERSION "v2.0"

// Function prototypes
extern void wifi_app_task(void);
extern void rsi_ble_configurator_task(void *argument);
void rsi_ble_configurator_init(void);
extern void memlcd_app_init(void);
extern void sensor_event_thread(void *argument);

uint8_t magic_word;

osSemaphoreId_t wlan_thread_sem;
osSemaphoreId_t ble_thread_sem;
osSemaphoreId_t i2c_sem;
extern volatile int currentLine;
extern GLIB_Context_t glibContext;
// The following condition enables the front-end internal switch control
#if (SL_SI91X_ACX_MODULE == 1)
#define FRONT_END_SWITCH_CTRL SL_SI91X_EXT_FEAT_FRONT_END_INTERNAL_SWITCH
#define REGION_CODE           IGNORE_REGION
#else
#define FRONT_END_SWITCH_CTRL SL_SI91X_EXT_FEAT_FRONT_END_SWITCH_PINS_ULP_GPIO_4_5_0
#define REGION_CODE           WORLD_DOMAIN
#endif
static const sl_wifi_device_configuration_t config = {
  .boot_option = LOAD_NWP_FW,
  .mac_address = NULL,
  .band        = SL_SI91X_WIFI_BAND_2_4GHZ,
  .region_code = REGION_CODE,
  .boot_config = { .oper_mode       = SL_SI91X_CLIENT_MODE,
                   .coex_mode       = SL_SI91X_WLAN_BLE_MODE,
                   .feature_bit_map = (SL_SI91X_FEAT_ULP_GPIO_BASED_HANDSHAKE | SL_SI91X_FEAT_DEV_TO_HOST_ULP_GPIO_1
#ifdef SLI_SI91X_MCU_INTERFACE
                                       | SL_WIFI_FEAT_WPS_DISABLE
#endif
                                       ),
                   .tcp_ip_feature_bit_map = (SL_SI91X_TCP_IP_FEAT_DHCPV4_CLIENT
#if (!RSI_TCP_IP_BYPASS)
                                              | SL_SI91X_TCP_IP_FEAT_EXTENSION_VALID | SL_SI91X_TCP_IP_FEAT_DNS_CLIENT
                                              | SL_SI91X_TCP_IP_FEAT_SSL | SL_SI91X_TCP_IP_FEAT_ICMP
#endif
                                              ),
                   .custom_feature_bit_map = (SL_WIFI_SYSTEM_CUSTOM_FEAT_EXTENSION_VALID),
                   .ext_custom_feature_bit_map =
                     (SL_WIFI_SYSTEM_EXT_FEAT_LOW_POWER_MODE | SL_SI91X_EXT_FEAT_XTAL_CLK | MEMORY_CONFIG
#ifdef SLI_SI917
                      | FRONT_END_SWITCH_CTRL
#endif
                      | SL_SI91X_EXT_FEAT_BT_CUSTOM_FEAT_ENABLE),
                   .bt_feature_bit_map         = (SL_SI91X_BT_RF_TYPE | SL_SI91X_ENABLE_BLE_PROTOCOL),
                   .ext_tcp_ip_feature_bit_map = (SL_SI91X_CONFIG_FEAT_EXTENTION_VALID
                                                  | SL_SI91X_EXT_TCP_IP_TOTAL_SELECTS(1) | SL_SI91X_EXT_EMB_MQTT_ENABLE
#ifdef RSI_PROCESS_MAX_RX_DATA
                                                  | SL_SI91X_EXT_TCP_MAX_RECV_LENGTH
#endif
                                                  ),
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

const osThreadAttr_t ble_thread_attributes = {
  .name       = "ble_thread",
  .attr_bits  = 0,
  .cb_mem     = 0,
  .cb_size    = 0,
  .stack_mem  = 0,
  .stack_size = 2048,
  .priority   = osPriorityNormal,
  .tz_module  = 0,
  .reserved   = 0,
};

const osThreadAttr_t sensor_event_thread_attributes = {
  .name       = "sensor_event_thread",
  .attr_bits  = 0,
  .cb_mem     = 0,
  .cb_size    = 0,
  .stack_mem  = 0,
  .stack_size = 2048,
  .priority   = osPriorityNormal1,
  .tz_module  = 0,
  .reserved   = 0,
};

void rsi_wlan_ble_app_init(void *argument)
{
  UNUSED_PARAMETER(argument);
  int32_t status = RSI_SUCCESS;

  sl_si91x_power_manager_add_ps_requirement(SL_SI91X_POWER_MANAGER_PS3);
  memlcd_app_init();
  GLIB_clear(&glibContext);
  GLIB_drawBitmap(&glibContext,
                  SILABS_LOGO_POSITION_X,
                  SILABS_LOGO_POSITION_Y,
                  SILABS_LOGO_WIDTH,
                  SILABS_LOGO_HEIGHT,
                  silabsLogo);
  currentLine = 2;
  GLIB_drawStringOnLine(&glibContext, "Demo Started", currentLine++, GLIB_ALIGN_LEFT, 5, 5, true);
  DMD_updateDisplay();

  status = sl_net_init(SL_NET_WIFI_CLIENT_INTERFACE, &config, NULL, network_event_handler);
  if (status != SL_STATUS_OK && status != SL_STATUS_ALREADY_INITIALIZED) {
    LOG_PRINT("\r\nWireless Initialization Failed, Error Code : 0x%lX\r\n", status);
    GLIB_drawStringOnLine(&glibContext, "Wireless Initialization Failed", currentLine++, GLIB_ALIGN_LEFT, 5, 5, true);
    DMD_updateDisplay();
    return;
  }
  LOG_PRINT("\r\nSi917 OOB Demo " OOB_APP_VERSION "\r\n");
  LOG_PRINT("\r\nWireless Initialization Success\r\n");
  currentLine = 0;
  GLIB_clear(&glibContext);
  GLIB_drawBitmap(&glibContext,
                  SILABS_LOGO_POSITION_X,
                  SILABS_LOGO_POSITION_Y,
                  SILABS_LOGO_WIDTH,
                  SILABS_LOGO_HEIGHT,
                  silabsLogo);
  GLIB_drawBitmap(&glibContext,
                  BLE_ICON_POSITION_X,
                  BLE_ICON_POSITION_Y,
                  BLUETOOTH_ICON_SIZE,
                  BLUETOOTH_ICON_SIZE,
                  bleLogo);
  currentLine += 2;
  GLIB_drawStringOnLine(&glibContext, "Si917 OOB Demo " OOB_APP_VERSION, currentLine++, GLIB_ALIGN_LEFT, 5, 5, true);
  DMD_updateDisplay();

  wlan_thread_sem = osSemaphoreNew(1, 0, NULL);
  if (wlan_thread_sem == NULL) {
    LOG_PRINT("\r\nFailed to create wlan_thread_sem\r\n");
    return;
  }

  ble_thread_sem = osSemaphoreNew(1, 0, NULL);
  if (ble_thread_sem == NULL) {
    LOG_PRINT("\r\nFailed to create ble_thread_sem\r\n");
    return;
  }

#ifdef SLI_SI91X_MCU_INTERFACE
  uint8_t xtal_enable = 1;
  status              = sl_si91x_m4_ta_secure_handshake(SL_SI91X_ENABLE_XTAL, 1, &xtal_enable, 0, NULL);
  if (status != SL_STATUS_OK) {
    printf("\r\nFailed to bring m4_ta_secure_handshake: 0x%lx\r\n", status);
    return;
  }
#endif

  if (osThreadNew((osThreadFunc_t)rsi_ble_configurator_task, NULL, &ble_thread_attributes) == NULL) {
    LOG_PRINT("\r\nFailed to create BLE thread\r\n");
  }

  // BLE initialization
  rsi_ble_configurator_init();

  wifi_app_task();

  return;
}

void app_init(void)
{
  osThreadNew((osThreadFunc_t)rsi_wlan_ble_app_init, NULL, &thread_attributes);
  osThreadNew((osThreadFunc_t)sensor_event_thread, NULL, &sensor_event_thread_attributes);
}
