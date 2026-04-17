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
 * Include files
 * */

//! SL Wi-Fi SDK includes
#include "sl_board_configuration.h"
#include "sl_wifi.h"
#include "sl_wifi_callback_framework.h"
#include "cmsis_os2.h"
#include "FreeRTOSConfig.h"
//! BLE include file to refer BLE APIs
#include "rsi_ble_apis.h"
#include "ble_config.h"
#include "rsi_ble_common_config.h"
#include "rsi_bt_common_apis.h"
#include "rsi_ble.h"
#include "rsi_bt_common.h"
#include "sl_si91x_driver.h"
#include "app.h"
#include "sl_utility.h"

//! Common include file
#include "rsi_common_apis.h"

#include <string.h>
#if SL_SI91X_TICKLESS_MODE == 0 && defined(SLI_SI91X_MCU_INTERFACE)
#include "sl_si91x_m4_ps.h"
#include "sl_si91x_power_manager.h"
#endif

#define RSI_BLE_LOCAL_NAME (void *)"BLE_PERIPHERAL"

#define DISABLE 0
#define ENABLE  1

#define RSI_BLE_PER_TRANSMIT_MODE 1
#define RSI_BLE_PER_RECEIVE_MODE  2
#define RSI_PER_STATS             3

#define RSI_CONFIG_PER_MODE RSI_BLE_PER_RECEIVE_MODE

#define LE_ONE_MBPS       1
#define LE_TWO_MBPS       2
#define LE_125_KBPS_CODED 4
#define LE_500_KBPS_CODED 8

#define DATA_PRBS9                 0x00
#define DATA_FOUR_ONES_FOUR_ZEROES 0x01
#define DATA_ALT_ONES_AND_ZEROES   0x02
#define DATA_PRSB15                0x03
#define DATA_ALL_ONES              0x04
#define DATA_ALL_ZEROES            0x05
#define DATA_FOUR_ZEROES_FOUR_ONES 0x06
#define DATA_ALT_ZEROES_AND_ONES   0x07

#define LE_ADV_CHNL_TYPE  0
#define LE_DATA_CHNL_TYPE 1

#define BURST_MODE     0
#define CONTIUOUS_MODE 1

#define NO_HOPPING     0
#define FIXED_HOPPING  1
#define RANDOM_HOPPING 2

#define BT_PER_STATS_CMD_ID 0x08
#define BLE_TRANSMIT_CMD_ID 0x13
#define BLE_RECEIVE_CMD_ID  0x14

#define BLE_ACCESS_ADDR    0x71764129
#define BLE_TX_PKT_LEN     32
#define BLE_PHY_RATE       LE_ONE_MBPS
#define BLE_RX_CHNL_NUM    10
#define BLE_TX_CHNL_NUM    10
#define BLE_TX_POWER_INDEX 8
#define SCRAMBLER_SEED     0
#define NUM_PKTS           0
#define RSI_INTER_PKT_GAP  0

#define ONBOARD_ANT_SEL 2
#define EXT_ANT_SEL     3

#define BLE_EXTERNAL_RF 0
#define BLE_INTERNAL_RF 1

#define NO_CHAIN_SEL      0
#define WLAN_HP_CHAIN_BIT 0
#define WLAN_LP_CHAIN_BIT 1
#define BT_HP_CHAIN_BIT   2
#define BT_LP_CHAIN_BIT   3

#define PLL_MODE_0 0
#define PLL_MODE_1 1

#define LOOP_BACK_MODE_DISABLE 0
#define LOOP_BACK_MODE_ENABLE  1

#define EXT_DATA_LEN_IND 1

#define DUTY_CYCLING_DISABLE  0
#define DUTY_CYCLING_ENABLE   1
#define ENABLE_NWP_POWER_SAVE 0  //! Set to 1 for powersave mode
#define LOCAL_DEV_ADDR_LEN    18 // Length of the local device address

#define GAIN_TABLE_AND_MAX_POWER_UPDATE_ENABLE 0 //! To update gain table and max tx power and offsets

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

// clang-format off
//! structure for the MAXPOWER
uint8_t Si917_BLE_REGION_BASED_MAXPOWER_XX[16] = {//{{{
	// BLE Max Power Index,
	FCC,        16,
	ETSI,       8,
	TELEC,      10,
	WORLDWIDE,  16,
	KCC,        10,
};//}}}

//! structure for the MAXPOWER OFFSET
uint8_t Si917_BLE_REGION_BASED_MAXPOWER_VS_OFFSET_XX[128] = {//{{{
	5,//NUM_OF_REGIONS
	FCC,
	4,//NUM_OF_CHANNELS
	//chan_num   1M   2M   125kbps 500kbps
	255,	    0,   0,      6,   0,
	0,          0,   0,      6,   0,
	38,         0,   2,      6,   0,
	39,         0,  16,      6,   0,
	ETSI,
	4,//NUM_OF_CHANNELS
	255,	    0,   0,      0,   0,
	0,          0,   0,      0,   0,
	19,         0,   0,      0,   0,
	39,         0,   0,      0,   0,
	TELEC,
	4,//NUM_OF_CHANNELS
	255,	    0,   0,      0,   0,
	0,	    0,   0,      0,   0,
	19,         0,   0,      0,   0,
	39,         0,   0,      0,   0,
	WORLDWIDE,
	4,//NUM_OF_CHANNELS
	255,	    0,   0,      0,   0,
	0,          0,   0,      0,   0,
	19,         0,   0,      0,   0,
	39,         0,   0,      0,   0,
	KCC,
	4,//NUM_OF_CHANNELS
	255,	    0,   0,      0,   0,
	0,	    0,   0,      0,   0,
	19,         0,   0,      0,   0,
	39,         0,   0,      0,   0
};//}}}

//! structure for the LP_CHAIN 0dBm OFFSET
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

//! structure for the LP_CHAIN 10dBm OFFSET
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

// clang-format on
#endif

//! Application global parameters.
static rsi_bt_resp_get_local_name_t rsi_app_resp_get_local_name = { 0 };
static uint8_t rsi_app_resp_get_dev_addr[RSI_DEV_ADDR_LEN]      = { 0 };
static rsi_ble_per_transmit_t rsi_ble_per_tx;
static rsi_ble_per_receive_t rsi_ble_per_rx;
static rsi_bt_per_stats_t per_stats;

static const sl_wifi_device_configuration_t config = {
  .boot_option = LOAD_NWP_FW,
  .mac_address = NULL,
  .band        = SL_SI91X_WIFI_BAND_2_4GHZ,
  .region_code = US,
  .boot_config = { .oper_mode = SL_SI91X_TRANSMIT_TEST_MODE,
                   .coex_mode = SL_SI91X_WLAN_BLE_MODE,
#ifdef SLI_SI91X_MCU_INTERFACE
                   .feature_bit_map = (SL_WIFI_FEAT_WPS_DISABLE | RSI_FEATURE_BIT_MAP),
#else
                   .feature_bit_map            = RSI_FEATURE_BIT_MAP,
#endif
#if RSI_TCP_IP_BYPASS
                   .tcp_ip_feature_bit_map = RSI_TCP_IP_FEATURE_BIT_MAP,
#else
                   .tcp_ip_feature_bit_map     = (RSI_TCP_IP_FEATURE_BIT_MAP | SL_SI91X_TCP_IP_FEAT_EXTENSION_VALID),
#endif
                   .custom_feature_bit_map = (SL_WIFI_SYSTEM_CUSTOM_FEAT_EXTENSION_VALID | RSI_CUSTOM_FEATURE_BIT_MAP),
                   .ext_custom_feature_bit_map =
                     (SL_WIFI_SYSTEM_EXT_FEAT_LOW_POWER_MODE | SL_SI91X_EXT_FEAT_XTAL_CLK | MEMORY_CONFIG
#ifdef SLI_SI917
                      | SL_SI91X_EXT_FEAT_FRONT_END_SWITCH_PINS_ULP_GPIO_4_5_0
#endif
                      | SL_SI91X_EXT_FEAT_BT_CUSTOM_FEAT_ENABLE),
                   .bt_feature_bit_map = (RSI_BT_FEATURE_BITMAP),
#ifdef RSI_PROCESS_MAX_RX_DATA
                   .ext_tcp_ip_feature_bit_map = (RSI_EXT_TCPIP_FEATURE_BITMAP | SL_SI91X_CONFIG_FEAT_EXTENTION_VALID
                                                  | SL_SI91X_EXT_TCP_MAX_RECV_LENGTH),
#else
                   .ext_tcp_ip_feature_bit_map = (RSI_EXT_TCPIP_FEATURE_BITMAP | SL_SI91X_CONFIG_FEAT_EXTENTION_VALID),
#endif
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
                   .config_feature_bit_map = (SL_SI91X_FEAT_SLEEP_GPIO_SEL_BITMAP | RSI_CONFIG_FEATURE_BITMAP) }
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
/**
 * @fn         ble_per
 * @brief      Tests the BLE PER Modes.
 * @param[in]  none
 * @return    none.
 * @section description
 * This function is used to test the different BLE PER Modes.
 */
void ble_per(void *unused)
{
  UNUSED_PARAMETER(unused);
  sl_status_t status;
  sl_wifi_firmware_version_t version         = { 0 };
  uint8_t local_dev_addr[LOCAL_DEV_ADDR_LEN] = { 0 };

  //! Wi-Fi initialization
  status = sl_wifi_init(&config, NULL, sl_wifi_default_event_handler);
  if (status != SL_STATUS_OK) {
    LOG_PRINT("\r\nWireless Initialization Failed, Error Code : 0x%lX\r\n", status);
    return;
  }
  LOG_PRINT("\r\nWireless Initialization Success\n");

#ifndef SL_SI91X_ACX_MODULE
  //! set region support
  status = sl_si91x_set_device_region(config.boot_config.oper_mode, config.band, config.region_code);
  if (status != SL_STATUS_OK) {
    LOG_PRINT("\r\nSet Region Failed, Error Code : %ld\r\n", status);
  } else {
    LOG_PRINT("\r\nSet Region Success\r\n");
  }
#endif

  //!  WLAN radio deinit
  status = sl_si91x_disable_radio();
  if (status != SL_STATUS_OK) {
    LOG_PRINT("\r\n  Failed to disable WLAN radio, Error Code : %ld\r\n", status);
  } else {
    LOG_PRINT("\r\n Disable WLAN radio success\r\n");
  }

  //! Firmware version Prints
  status = sl_wifi_get_firmware_version(&version);
  if (status != SL_STATUS_OK) {
    LOG_PRINT("\r\nFailed to fetch firmware version: 0x%lx\r\n", status);
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
    LOG_PRINT("\r\n Local device address : %s \r\n ", local_dev_addr);
  }

  //! set the local device name
  status = rsi_bt_set_local_name(RSI_BLE_LOCAL_NAME);
  if (status != RSI_SUCCESS) {
    LOG_PRINT("\r\n Set Local Name Failed = %lx\r\n", status);
  }

  //! get the local device name
  status = rsi_bt_get_local_name(&rsi_app_resp_get_local_name);
  if (status != RSI_SUCCESS) {
    LOG_PRINT("\r\n Get Local Name Failed = %lx\r\n", status);
  }
  LOG_PRINT("Local name set to: %s\n", rsi_app_resp_get_local_name.name);

#if GAIN_TABLE_AND_MAX_POWER_UPDATE_ENABLE

  //! structure update for the MAXPOWER
  status = rsi_bt_cmd_update_gain_table_offset_or_max_pwr(0,
                                                          sizeof(Si917_BLE_REGION_BASED_MAXPOWER_XX),
                                                          Si917_BLE_REGION_BASED_MAXPOWER_XX,
                                                          BLE_GAIN_TABLE_MAXPOWER_UPDATE);
  if (status != RSI_SUCCESS) {
    LOG_PRINT("\r\n Failed to update gain table for max power with status = %lx\r\n", status);
  } else {
    LOG_PRINT("\r\n Updation of gain table max tx power command is successful \r\n");
  }

  //! structure update for the MAXPOWER OFFSET
  status = rsi_bt_cmd_update_gain_table_offset_or_max_pwr(0,
                                                          sizeof(Si917_BLE_REGION_BASED_MAXPOWER_VS_OFFSET_XX),
                                                          Si917_BLE_REGION_BASED_MAXPOWER_VS_OFFSET_XX,
                                                          BLE_GAIN_TABLE_OFFSET_UPDATE);
  if (status != RSI_SUCCESS) {
    LOG_PRINT("\r\n Failed to update gain table offset with status = %lx\r\n", status);
  } else {
    LOG_PRINT("\r\n Updation of gain table offset command is successful \r\n");
  }

  //! structure update for the LP_CHAIN 0dBm OFFSET
  status = rsi_bt_cmd_update_gain_table_offset_or_max_pwr(0,
                                                          sizeof(Si917_BLE_REGION_BASED_LP_CHAIN_0DBM_OFFSET_XX),
                                                          Si917_BLE_REGION_BASED_LP_CHAIN_0DBM_OFFSET_XX,
                                                          BLE_GAIN_TABLE_LP_CHAIN_0DBM_OFFSET_UPDATE);
  if (status != RSI_SUCCESS) {
    LOG_PRINT("\r\n Failed to update gain table LP-Chain 0dBm offset with status = %lx\r\n", status);
  } else {
    LOG_PRINT("\r\n Updation of gain table LP-Chain 0dBm offset command is successful \r\n");
  }

  //! structure update for the LP_CHAIN 10dBm OFFSET
  status = rsi_bt_cmd_update_gain_table_offset_or_max_pwr(0,
                                                          sizeof(Si917_BLE_REGION_BASED_LP_CHAIN_10DBM_OFFSET_XX),
                                                          Si917_BLE_REGION_BASED_LP_CHAIN_10DBM_OFFSET_XX,
                                                          BLE_GAIN_TABLE_LP_CHAIN_10DBM_OFFSET_UPDATE);
  if (status != RSI_SUCCESS) {
    LOG_PRINT("\r\n Failed to update gain table LP-Chain 10dBm offset with status = %lx\r\n", status);
  } else {
    LOG_PRINT("\r\n Updation of gain table LP-Chain 10dBm offset command is successful \r\n");
  }

#endif

  if (RSI_CONFIG_PER_MODE == RSI_BLE_PER_TRANSMIT_MODE) {
    rsi_ble_per_tx.cmd_ix                       = BLE_TRANSMIT_CMD_ID;
    rsi_ble_per_tx.transmit_enable              = ENABLE;
    *(uint32_t *)&rsi_ble_per_tx.access_addr[0] = BLE_ACCESS_ADDR;
    *(uint16_t *)&rsi_ble_per_tx.pkt_len[0]     = BLE_TX_PKT_LEN;
    rsi_ble_per_tx.phy_rate                     = BLE_PHY_RATE;
    rsi_ble_per_tx.rx_chnl_num                  = BLE_RX_CHNL_NUM;
    rsi_ble_per_tx.tx_chnl_num                  = BLE_TX_CHNL_NUM;
    rsi_ble_per_tx.scrambler_seed               = SCRAMBLER_SEED;
    rsi_ble_per_tx.payload_type                 = DATA_FOUR_ZEROES_FOUR_ONES;
    rsi_ble_per_tx.le_chnl_type                 = LE_DATA_CHNL_TYPE;
    rsi_ble_per_tx.tx_power                     = BLE_TX_POWER_INDEX;
    rsi_ble_per_tx.transmit_mode                = BURST_MODE;
    rsi_ble_per_tx.freq_hop_en                  = NO_HOPPING;
    rsi_ble_per_tx.ant_sel                      = ONBOARD_ANT_SEL;
    rsi_ble_per_tx.inter_pkt_gap                = RSI_INTER_PKT_GAP;
    rsi_ble_per_tx.pll_mode                     = PLL_MODE_0;
    rsi_ble_per_tx.rf_type                      = BLE_INTERNAL_RF;
    rsi_ble_per_tx.rf_chain                     = BT_HP_CHAIN_BIT;

    rsi_uint32_to_4bytes(rsi_ble_per_tx.num_pkts, NUM_PKTS);

    //! start the Transmit PER functionality
    status = rsi_ble_per_transmit(&rsi_ble_per_tx);
    if (status != RSI_SUCCESS) {
      LOG_PRINT("\n per transmit cmd failed : 0x%lX \n", status);
      return;
    } else {
      LOG_PRINT("\nRSI_BLE_PER_TRANSMIT_MODE \n"
                "cmd id: 0x%X \n"
                "enable: %d \n"
                "access_addr: 0x%lX \n"
                "pkt_len: %d \n"
                "phy_rate: %d \n"
                "rx_chnl_num: %d \n"
                "tx_chnl_num: %d \n"
                "scrambler_seed: %d \n"
                "payload_type: %d \n"
                "le_chnl_type: %d \n"
                "tx_power: %d \n"
                "transmit_mode: %d \n"
                "freq_hop_en: %d \n"
                "ant_sel: %d \n"
                "inter_pkt_gap: %d \n"
                "pll_mode: %d \n"
                "rf_type: %d \n"
                "rf_chain: %d \n",
                rsi_ble_per_tx.cmd_ix,
                rsi_ble_per_tx.transmit_enable,
                *(uint32_t *)&rsi_ble_per_tx.access_addr[0],
                (*(uint16_t *)&rsi_ble_per_tx.pkt_len[0]),
                rsi_ble_per_tx.phy_rate,
                rsi_ble_per_tx.rx_chnl_num,
                rsi_ble_per_tx.tx_chnl_num,
                rsi_ble_per_tx.scrambler_seed,
                rsi_ble_per_tx.payload_type,
                rsi_ble_per_tx.le_chnl_type,
                rsi_ble_per_tx.tx_power,
                rsi_ble_per_tx.transmit_mode,
                rsi_ble_per_tx.freq_hop_en,
                rsi_ble_per_tx.ant_sel,
                rsi_ble_per_tx.inter_pkt_gap,
                rsi_ble_per_tx.pll_mode,
                rsi_ble_per_tx.rf_type,
                rsi_ble_per_tx.rf_chain);
    }
  } else if (RSI_CONFIG_PER_MODE == RSI_BLE_PER_RECEIVE_MODE) {
    rsi_ble_per_rx.cmd_ix                       = BLE_RECEIVE_CMD_ID;
    rsi_ble_per_rx.receive_enable               = ENABLE;
    *(uint32_t *)&rsi_ble_per_rx.access_addr[0] = BLE_ACCESS_ADDR;
    rsi_ble_per_rx.ext_data_len_indication      = EXT_DATA_LEN_IND;
    rsi_ble_per_rx.phy_rate                     = BLE_PHY_RATE;
    rsi_ble_per_rx.rx_chnl_num                  = BLE_RX_CHNL_NUM;
    rsi_ble_per_rx.tx_chnl_num                  = BLE_TX_CHNL_NUM;
    rsi_ble_per_rx.scrambler_seed               = SCRAMBLER_SEED;
    rsi_ble_per_rx.le_chnl_type                 = LE_DATA_CHNL_TYPE;
    rsi_ble_per_rx.loop_back_mode               = LOOP_BACK_MODE_DISABLE;
    rsi_ble_per_rx.freq_hop_en                  = NO_HOPPING;
    rsi_ble_per_rx.ant_sel                      = ONBOARD_ANT_SEL;
    rsi_ble_per_rx.duty_cycling_en              = DUTY_CYCLING_DISABLE;
    rsi_ble_per_rx.pll_mode                     = PLL_MODE_0;
    rsi_ble_per_rx.rf_type                      = BLE_INTERNAL_RF;
    rsi_ble_per_rx.rf_chain                     = BT_HP_CHAIN_BIT;
    //! start the Receive PER functionality
    status = rsi_ble_per_receive(&rsi_ble_per_rx);
    if (status != RSI_SUCCESS) {
      LOG_PRINT("\n per receive cmd failed : %lx \n", status);
      return;
    } else {
      LOG_PRINT("\nRSI_BLE_PER_RECEIVE_MODE \n"
                "cmd id: 0x%X \n"
                "enable: %d \n"
                "access_addr: 0x%lX \n"
                "ext_data_len_indication: %d \n"
                "phy_rate: %d \n"
                "rx_chnl_num: %d \n"
                "tx_chnl_num: %d \n"
                "scrambler_seed: %d \n"
                "le_chnl_type: %d \n"
                "loop_back_mode: %d \n"
                "freq_hop_en: %d \n"
                "ant_sel: %d \n"
                "duty_cycling_en: %d \n"
                "pll_mode: %d \n"
                "rf_type: %d \n"
                "rf_chain: %d \n",
                rsi_ble_per_rx.cmd_ix,
                rsi_ble_per_rx.receive_enable,
                *(uint32_t *)&rsi_ble_per_rx.access_addr[0],
                rsi_ble_per_rx.ext_data_len_indication,
                rsi_ble_per_rx.phy_rate,
                rsi_ble_per_rx.rx_chnl_num,
                rsi_ble_per_rx.tx_chnl_num,
                rsi_ble_per_rx.scrambler_seed,
                rsi_ble_per_rx.le_chnl_type,
                rsi_ble_per_rx.loop_back_mode,
                rsi_ble_per_rx.freq_hop_en,
                rsi_ble_per_rx.ant_sel,
                rsi_ble_per_rx.duty_cycling_en,
                rsi_ble_per_rx.pll_mode,
                rsi_ble_per_rx.rf_type,
                rsi_ble_per_rx.rf_chain);
    }
  }

  while (1) {
    status = rsi_bt_per_stats(BT_PER_STATS_CMD_ID, &per_stats);
    if (status != RSI_SUCCESS) {
      LOG_PRINT("\n per stats cmd failed : %lx \n", status);
    } else {
      LOG_PRINT("\nPER Stats \n"
                "crc_fail_cnt : %d \n"
                "crc_pass_cnt: %d \n"
                "tx_dones: %d \n"
                "rssi: %d \n"
                "id_pkts_rcvd :%d \n",
                per_stats.crc_fail_cnt,
                per_stats.crc_pass_cnt,
                per_stats.tx_dones,
                per_stats.rssi,
                per_stats.id_pkts_rcvd);
    }
#if ((SL_SI91X_TICKLESS_MODE == 0) && SLI_SI91X_MCU_INTERFACE && ENABLE_NWP_POWER_SAVE)
    if (!(P2P_STATUS_REG & TA_wakeup_M4)) {
      P2P_STATUS_REG &= ~M4_wakeup_TA;
      LOG_PRINT("\r\n M4 sleep");
      sl_si91x_power_manager_sleep();
    }
#else
    //To get tx_done logs properly and to avoid application hang issue due to continuous stats added 1sec delay.
    //It is applicable for both sdk 2.9 and 3.0
    osDelay(1000);
#endif
  }
  return;
}

void app_init(void)
{

  osThreadNew((osThreadFunc_t)ble_per, NULL, &thread_attributes);
}
