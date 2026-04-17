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
 * @brief : This file contains example application for BLE Gain table update
 * @section Description :
 * This application demonstrates how to update the Gain table.
 =================================================================================*/

/**
 * Include files
 * */
//! SL Wi-Fi SDK includes
#include "sl_board_configuration.h"
#include "sl_wifi.h"
#include "sl_wifi_callback_framework.h"
#include "cmsis_os2.h"
#include "sl_utility.h"

//! BLE include file to refer BLE APIs
#include <string.h>

#include "ble_config.h"
#include "rsi_ble_apis.h"
#include "rsi_ble_common_config.h"
#include "rsi_bt_common.h"
#include "rsi_bt_common_apis.h"

//! Common include file
#include "rsi_common_apis.h"
#ifdef SLI_SI91X_MCU_INTERFACE
#include "sl_si91x_m4_ps.h"
#endif

/*=======================================================================*/
//   ! MACROS
/*=======================================================================*/
#define RSI_BT_BLE_MODE             9
#define GLOBAL_BUFF_LEN             15000
#define UPDATE_GAIN_TABLE_MAX_POWER 0
#define UPDATE_GAIN_TABLE_OFFSET    1
#define BLE_NODE                    0

#define GAIN_TABLE_AND_MAX_POWER_UPDATE_ENABLE 1 //! To update gain table and max tx power and offsets

#if GAIN_TABLE_AND_MAX_POWER_UPDATE_ENABLE

#define FCC       0
#define ETSI      1
#define TELEC     2
#define WORLDWIDE 3
#define KCC       4

#define BLE_GAIN_TABLE_MAXPOWER_UPDATE              0
#define BLE_GAIN_TABLE_OFFSET_UPDATE                1
#define BLE_GAIN_TABLE_LP_CHAIN_0DBM_OFFSET_UPDATE  2
#define BLE_GAIN_TABLE_LP_CHAIN_10DBM_OFFSET_UPDATE 3

#define LOCAL_DEV_ADDR_LEN 18 // Length of the local device address

// clang-format off
//! structure for the MAXPOWER
uint8_t Si917_BLE_REGION_BASED_MAXPOWER[16] = {//{{{
    //BLE Max Power Index,
    FCC,        16,
    ETSI,       8,
    TELEC,      10,
    WORLDWIDE,  16,
    KCC,        10,
};//}}}


//! structure for the MAXPOWER OFFSET
uint8_t Si917_BLE_REGION_BASED_MAXPOWER_VS_OFFSET[128] = {//{{{
    5,//NUM_OF_REGIONS
    FCC,
    4,//NUM_OF_CHANNELS
    //chan_num     1M   2M   500kbps 125kbps
    255,     0,   0,      5,   0,
    0,      0,   0,      5,   0,
    38,      0,   2,      5,   0,
    39,      0,  16,      5,   0,
    ETSI,
    4,//NUM_OF_CHANNELS
    //chan_num     1M   2M   500kbps 125kbps
    255,     0,   0,      0,   0,
    0,      0,   0,      0,   0,
    19,      0,   0,      0,   0,
    39,      0,   0,      0,   0,
    TELEC,
    4,//NUM_OF_CHANNELS
    //chan_num     1M   2M   500kbps 125kbps
    255,     0,   0,      0,   0,
    0,      2,   2,      2,   2,
    19,      0,   0,      0,   0,
    39,      0,   0,      0,   0,
    WORLDWIDE,
    4,//NUM_OF_CHANNELS
    //chan_num     1M   2M   500kbps 125kbps
    255,     0,   0,      0,   0,
    0,      0,   0,      0,   0,
    19,      0,   0,      0,   0,
    39,      0,   0,      0,   0,
    KCC,
    4,//NUM_OF_CHANNELS
    //chan_num     1M   2M   500kbps 125kbps
    255,     0,   0,      0,   0,
    0,      2,   2,      2,   2,
    20,      0,   0,      0,   0,
    39,      0,   0,      0,   0
};

//! structure for the MAXPOWER OFFSET
uint8_t Si917_BLE_REGION_BASED_LP_CHAIN_0DBM_OFFSET_XX[128] = {//{{{
	5,//NUM_OF_REGIONS
	FCC,
	4,//NUM_OF_CHANNELS
	//chan_num    1M   2M   125kbps 500kbps
	255,	      31,  31,      31,   31,
	0,            31,  31,      31,   31,
	19,           31,  31,      31,   31,
	39,           31,   8,      31,   31,
	ETSI,
	4,//NUM_OF_CHANNELS
	255,	      31,  31,      31,   31,
	0,            31,  31,      31,   31,
	19,           31,  31,      31,   31,
	39,           31,  31,      31,   31,
	TELEC,
	4,//NUM_OF_CHANNELS
	255,	      31,  31,      31,   31,
	0,            31,  31,      31,   31,
	19,           31,  31,      31,   31,
	39,           31,  31,      31,   31,
	WORLDWIDE,
	4,//NUM_OF_CHANNELS
	255,	      31,  31,      31,   31,
	0,            31,  31,      31,   31,
	19,           31,  31,      31,   31,
	39,           31,  31,      31,   31,
	KCC,
	4,//NUM_OF_CHANNELS
	255,	      31,  31,      31,   31,
	0,            31,  31,      31,   31,
	19,           31,  31,      31,   31,
	39,           31,  31,      31,   31,
};//}}}

//! structure for the MAXPOWER OFFSET
uint8_t Si917_BLE_REGION_BASED_LP_CHAIN_10DBM_OFFSET_XX[128] = {//{{{
	5,//NUM_OF_REGIONS
	FCC,
	4,//NUM_OF_CHANNELS
	//chan_num    1M   2M   125kbps 500kbps
	255,	      63,  63,      63,   63,
	0,            63,  63,      63,   63,
	19,           63,  63,      63,   63,
	39,           63,  35,      63,   63,
	ETSI,
	4,//NUM_OF_CHANNELS
	255,	      63,  63,      63,   63,
	0,            63,  63,      63,   63,
	19,           63,  63,      63,   63,
	39,           63,  63,      63,   63,
	TELEC,
	4,//NUM_OF_CHANNELS
	255,	      63,  63,      63,   63,
	0,            63,  63,      63,   63,
	19,           63,  63,      63,   63,
	39,           63,  63,      63,   63,
	WORLDWIDE,
	4,//NUM_OF_CHANNELS
	255,	      63,  63,      63,   63,
	0,            63,  63,      63,   63,
	19,           63,  63,      63,   63,
	39,           63,  63,      63,   63,
	KCC,
	4,//NUM_OF_CHANNELS
	255,	      63,  63,      63,   63,
	0,            63,  63,      63,   63,
	19,           63,  63,      63,   63,
	39,           63,  63,      63,   63,
};//}}}
#endif
// clang-format on

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
                      | SL_SI91X_EXT_FEAT_BT_CUSTOM_FEAT_ENABLE // Enable BT feature
                      ),
                   .ext_tcp_ip_feature_bit_map = (SL_SI91X_CONFIG_FEAT_EXTENTION_VALID),
                   .bt_feature_bit_map         = (SL_SI91X_BT_RF_TYPE | SL_SI91X_ENABLE_BLE_PROTOCOL),
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
  .priority   = osPriorityNormal,
  .tz_module  = 0,
  .reserved   = 0,
};

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

/*=======================================================================*/
//   ! GLOBAL VARIABLES
/*=======================================================================*/
void update_gain_table(void *argument)
{
  UNUSED_PARAMETER(argument);
  int32_t status = RSI_SUCCESS;
  uint8_t _Si917_BLE_REGION_BASED_MAXPOWER_XX[16];
  uint8_t _Si917_BLE_REGION_BASED_MAXPOWER_VS_OFFSET_XX[128];

  sl_wifi_firmware_version_t version                         = { 0 };
  static uint8_t rsi_app_resp_get_dev_addr[RSI_DEV_ADDR_LEN] = { 0 };
  uint8_t local_dev_addr[LOCAL_DEV_ADDR_LEN]                 = { 0 };

  status = sl_wifi_init(&config, NULL, sl_wifi_default_event_handler);
  if (status != SL_STATUS_OK) {
    LOG_PRINT("\r\nWi-Fi Initialization Failed, Error Code : 0x%lX\r\n", status);
    return;
  } else {
    LOG_PRINT("\r\n Wi-Fi Initialization Success\n");
  }

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

  memcpy(_Si917_BLE_REGION_BASED_MAXPOWER_XX, Si917_BLE_REGION_BASED_MAXPOWER, sizeof(Si917_BLE_REGION_BASED_MAXPOWER));
  memcpy(_Si917_BLE_REGION_BASED_MAXPOWER_VS_OFFSET_XX,
         Si917_BLE_REGION_BASED_MAXPOWER_VS_OFFSET,
         sizeof(Si917_BLE_REGION_BASED_MAXPOWER_VS_OFFSET));

  status = rsi_bt_cmd_update_gain_table_offset_or_max_pwr(BLE_NODE,
                                                          sizeof(_Si917_BLE_REGION_BASED_MAXPOWER_XX),
                                                          _Si917_BLE_REGION_BASED_MAXPOWER_XX,
                                                          UPDATE_GAIN_TABLE_MAX_POWER);
  if (status != 0) {
    LOG_PRINT("update gain table max_pwr Failed, Error Code : 0x%lX\r\n", status);
  } else {
    LOG_PRINT("update gain table max_pwr successful\n")
  }

  status = rsi_bt_cmd_update_gain_table_offset_or_max_pwr(BLE_NODE,
                                                          sizeof(_Si917_BLE_REGION_BASED_MAXPOWER_VS_OFFSET_XX),
                                                          _Si917_BLE_REGION_BASED_MAXPOWER_VS_OFFSET_XX,
                                                          UPDATE_GAIN_TABLE_OFFSET);
  if (status != 0) {
    LOG_PRINT("update gain table_offset Failed, Error Code : 0x%lX\r\n", status);
  } else {
    LOG_PRINT("update gain table offset successful\n")
  }

  //! structure update for the LP_CHAIN 0dBm OFFSET
  status = rsi_bt_cmd_update_gain_table_offset_or_max_pwr(0,
                                                          sizeof(Si917_BLE_REGION_BASED_LP_CHAIN_0DBM_OFFSET_XX),
                                                          Si917_BLE_REGION_BASED_LP_CHAIN_0DBM_OFFSET_XX,
                                                          BLE_GAIN_TABLE_LP_CHAIN_0DBM_OFFSET_UPDATE);
  if (status != RSI_SUCCESS) {
    LOG_PRINT("\r\n update for the LP_CHAIN 0dBm OFFSET cmd failed with status = %lx\r\n", status);
  } else {
    LOG_PRINT("\r\n update for the LP_CHAIN 0dBm OFFSET successful \r\n");
  }

  //! structure update for the LP_CHAIN 10dBm OFFSET
  status = rsi_bt_cmd_update_gain_table_offset_or_max_pwr(0,
                                                          sizeof(Si917_BLE_REGION_BASED_LP_CHAIN_10DBM_OFFSET_XX),
                                                          Si917_BLE_REGION_BASED_LP_CHAIN_10DBM_OFFSET_XX,
                                                          BLE_GAIN_TABLE_LP_CHAIN_10DBM_OFFSET_UPDATE);
  if (status != RSI_SUCCESS) {
    LOG_PRINT("\r\n update for the LP_CHAIN 10dBm OFFSET cmd failed with status = %lx\r\n", status);
  } else {
    LOG_PRINT("\r\n update for the LP_CHAIN 10dBm OFFSET successful \r\n");
  }
}

void app_init(void)
{
  osThreadNew((osThreadFunc_t)update_gain_table, NULL, &thread_attributes);
}
