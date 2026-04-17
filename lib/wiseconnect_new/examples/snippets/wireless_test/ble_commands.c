/*******************************************************************************
* @file  ble_commands.c
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

#include "sl_wifi.h"
#include "sl_constants.h"
#include "ble_config.h"
#include "rsi_ble_common_config.h"
#include <rsi_ble_apis.h>
#include "rsi_bt_common_apis.h"
#include "console.h"
#include "sl_si91x_driver.h"
#include <stdio.h>
#include <string.h>
#include "sl_si91x_driver.h"
#include "console_argument_types.h"
/*
// APP version
#define APP_FW_VERSION "0.4"

// Function prototypes
extern void sl_wifi_app_task(void);
extern void rsi_ble_configurator_task(void *argument);
void rsi_ble_configurator_init(void);
uint8_t magic_word;

osSemaphoreId_t wlan_thread_sem;
osSemaphoreId_t ble_thread_sem;
*/
#define DISABLE 0
#define ENABLE  1

#define BLE_NODE                                    0
#define UPDATE_GAIN_TABLE_MAX_POWER                 0
#define UPDATE_GAIN_TABLE_OFFSET                    1
#define BLE_GAIN_TABLE_LP_CHAIN_0DBM_OFFSET_UPDATE  2
#define BLE_GAIN_TABLE_LP_CHAIN_10DBM_OFFSET_UPDATE 3

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

#define DUTY_CYCLING_DISABLE 0
#define DUTY_CYCLING_ENABLE  1

#define PRBS9_SEQ            0x0
#define FOUR_ONES_FOUR_ZEROS 0x1
#define ALT_ONES_AND_ZEROS   0x2
#define PRBS15_SEQ           0x3
#define ALL_ONES             0x4
#define ALL_ZEROS            0x5
#define FOUR_ZEROS_FOUR_ONES 0x6
#define ALT_ZERO_ALT_ONE     0x7

//Direct Test Modes
#define RSI_BLE_TX_PAYLOAD_LEN      0x20
#define RSI_BLE_TX_PAYLOAD_TYPE     PRBS9_SEQ
#define RSI_BLE_STANDARD_MODULATION 0x00

const sl_wifi_device_configuration_t sl_wifi_ble_configuration_cli = {
  .boot_option = LOAD_NWP_FW,
  .mac_address = NULL,
  .band        = SL_SI91X_WIFI_BAND_2_4GHZ,
  .region_code = WORLD_DOMAIN,
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
                   .ext_custom_feature_bit_map = (SL_SI91X_EXT_FEAT_XTAL_CLK | MEMORY_CONFIG
#if ENABLE_POWERSAVE_CLI
                                                  | SL_WIFI_SYSTEM_EXT_FEAT_LOW_POWER_MODE
#endif
#ifdef SLI_SI917
                                                  | SL_SI91X_EXT_FEAT_FRONT_END_SWITCH_PINS_ULP_GPIO_4_5_0
#endif
                                                  | SL_SI91X_EXT_FEAT_BT_CUSTOM_FEAT_ENABLE),
                   .bt_feature_bit_map = (RSI_BT_FEATURE_BITMAP
#if (RSI_BT_GATT_ON_CLASSIC)
                                          | SL_SI91X_BT_ATT_OVER_CLASSIC_ACL /* to support att over classic acl link */
#endif
                                          ),
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
                   .config_feature_bit_map = 0 }
};

static rsi_ble_per_transmit_t rsi_ble_per_tx;
static rsi_ble_per_receive_t rsi_ble_per_rx;

static rsi_bt_per_stats_t per_stats;
static uint16_t testmodes_num_of_packets;
static rsi_bt_resp_get_local_name_t rsi_app_resp_get_local_name = { 0 };
static uint8_t rsi_app_resp_get_dev_addr[RSI_DEV_ADDR_LEN]      = { 0 };
uint8_t local_dev_addr[18]                                      = { 0 };
uint8_t adv[31]                                                 = { 2, 1, 6 };

static uint8_t wifi_deinit_var;

sl_status_t ble_user_gain_table_max_power_handler(console_args_t *arguments)
{
  sl_status_t status                        = SL_STATUS_OK;
  uint8_t ble_user_gain_table_max_power[10] = { 0 };
  ble_user_gain_table_max_power[0]          = (sl_si91x_user_gain_table_region_code_t)GET_COMMAND_ARG(arguments, 0);
  ble_user_gain_table_max_power[1]          = (uint8_t)GET_COMMAND_ARG(arguments, 1);
  ble_user_gain_table_max_power[2]          = (sl_si91x_user_gain_table_region_code_t)GET_COMMAND_ARG(arguments, 2);
  ble_user_gain_table_max_power[3]          = (uint8_t)GET_COMMAND_ARG(arguments, 3);
  ble_user_gain_table_max_power[4]          = (sl_si91x_user_gain_table_region_code_t)GET_COMMAND_ARG(arguments, 4);
  ble_user_gain_table_max_power[5]          = (uint8_t)GET_COMMAND_ARG(arguments, 5);
  ble_user_gain_table_max_power[6]          = (sl_si91x_user_gain_table_region_code_t)GET_COMMAND_ARG(arguments, 6);
  ble_user_gain_table_max_power[7]          = (uint8_t)GET_COMMAND_ARG(arguments, 7);
  ble_user_gain_table_max_power[8]          = (sl_si91x_user_gain_table_region_code_t)GET_COMMAND_ARG(arguments, 8);
  ble_user_gain_table_max_power[9]          = (uint8_t)GET_COMMAND_ARG(arguments, 9);

  status = rsi_bt_cmd_update_gain_table_offset_or_max_pwr(BLE_NODE,
                                                          sizeof(ble_user_gain_table_max_power),
                                                          ble_user_gain_table_max_power,
                                                          UPDATE_GAIN_TABLE_MAX_POWER);

  VERIFY_STATUS_AND_RETURN(status);
  return SL_STATUS_OK;
}

sl_status_t ble_user_gain_table_max_power_offset_handler(void) //console_args_t *arguments)
{
  sl_status_t status = SL_STATUS_OK;
  //! structure for the MAXPOWER OFFSET
  uint8_t Si917_BLE_REGION_BASED_MAXPOWER_VS_OFFSET[128] = { 5, //NUM_OF_REGIONS
                                                             FCC,
                                                             4, //NUM_OF_CHANNELS
                                                             //chan_num     1M   2M   500kbps 125kbps
                                                             255,
                                                             0,
                                                             0,
                                                             5,
                                                             0,
                                                             0,
                                                             0,
                                                             0,
                                                             5,
                                                             0,
                                                             38,
                                                             0,
                                                             2,
                                                             5,
                                                             0,
                                                             39,
                                                             0,
                                                             16,
                                                             5,
                                                             0,
                                                             ETSI,
                                                             4, //NUM_OF_CHANNELS
                                                             //chan_num     1M   2M   500kbps 125kbps
                                                             255,
                                                             0,
                                                             0,
                                                             0,
                                                             0,
                                                             0,
                                                             0,
                                                             0,
                                                             0,
                                                             0,
                                                             19,
                                                             0,
                                                             0,
                                                             0,
                                                             0,
                                                             39,
                                                             0,
                                                             0,
                                                             0,
                                                             0,
                                                             TELEC,
                                                             4, //NUM_OF_CHANNELS
                                                             //chan_num     1M   2M   500kbps 125kbps
                                                             255,
                                                             0,
                                                             0,
                                                             0,
                                                             0,
                                                             0,
                                                             2,
                                                             2,
                                                             2,
                                                             2,
                                                             19,
                                                             0,
                                                             0,
                                                             0,
                                                             0,
                                                             39,
                                                             0,
                                                             0,
                                                             0,
                                                             0,
                                                             WORLD_WIDE,
                                                             4, //NUM_OF_CHANNELS
                                                             //chan_num     1M   2M   500kbps 125kbps
                                                             255,
                                                             0,
                                                             0,
                                                             0,
                                                             0,
                                                             0,
                                                             0,
                                                             0,
                                                             0,
                                                             0,
                                                             19,
                                                             0,
                                                             0,
                                                             0,
                                                             0,
                                                             39,
                                                             0,
                                                             0,
                                                             0,
                                                             0,
                                                             KCC,
                                                             4, //NUM_OF_CHANNELS
                                                             //chan_num     1M   2M   500kbps 125kbps
                                                             255,
                                                             0,
                                                             0,
                                                             0,
                                                             0,
                                                             0,
                                                             2,
                                                             2,
                                                             2,
                                                             2,
                                                             20,
                                                             0,
                                                             0,
                                                             0,
                                                             0,
                                                             39,
                                                             0,
                                                             0,
                                                             0,
                                                             0 };
  status                                                 = rsi_bt_cmd_update_gain_table_offset_or_max_pwr(BLE_NODE,
                                                          sizeof(Si917_BLE_REGION_BASED_MAXPOWER_VS_OFFSET),
                                                          Si917_BLE_REGION_BASED_MAXPOWER_VS_OFFSET,
                                                          UPDATE_GAIN_TABLE_OFFSET);

  VERIFY_STATUS_AND_RETURN(status);
  return SL_STATUS_OK;
}

sl_status_t ble_user_gain_table_lp_chain_0dBm_offset_handler(void)
{ //console_args_t *arguments){
  sl_status_t status = SL_STATUS_OK;

  //! structure for the LP 0dBm POWER OFFSET
  uint8_t Si917_BLE_REGION_BASED_LP_CHAIN_0DBM_OFFSET_XX[128] = {
    5, //NUM_OF_REGIONS
    FCC,
    4, //NUM_OF_CHANNELS
    //chan_num    1M   2M   125kbps 500kbps
    255,
    31,
    31,
    31,
    31,
    0,
    31,
    31,
    31,
    31,
    19,
    31,
    31,
    31,
    31,
    39,
    31,
    8,
    31,
    31,
    ETSI,
    4, //NUM_OF_CHANNELS
    255,
    31,
    31,
    31,
    31,
    0,
    31,
    31,
    31,
    31,
    19,
    31,
    31,
    31,
    31,
    39,
    31,
    31,
    31,
    31,
    TELEC,
    4, //NUM_OF_CHANNELS
    255,
    31,
    31,
    31,
    31,
    0,
    31,
    31,
    31,
    31,
    19,
    31,
    31,
    31,
    31,
    39,
    31,
    31,
    31,
    31,
    WORLD_WIDE,
    4, //NUM_OF_CHANNELS
    255,
    31,
    31,
    31,
    31,
    0,
    31,
    31,
    31,
    31,
    19,
    31,
    31,
    31,
    31,
    39,
    31,
    31,
    31,
    31,
    KCC,
    4, //NUM_OF_CHANNELS
    255,
    31,
    31,
    31,
    31,
    0,
    31,
    31,
    31,
    31,
    19,
    31,
    31,
    31,
    31,
    39,
    31,
    31,
    31,
    31,
  };
  //! structure update for the LP_CHAIN 0dBm OFFSET
  status = rsi_bt_cmd_update_gain_table_offset_or_max_pwr(0,
                                                          sizeof(Si917_BLE_REGION_BASED_LP_CHAIN_0DBM_OFFSET_XX),
                                                          Si917_BLE_REGION_BASED_LP_CHAIN_0DBM_OFFSET_XX,
                                                          BLE_GAIN_TABLE_LP_CHAIN_0DBM_OFFSET_UPDATE);
  VERIFY_STATUS_AND_RETURN(status);
  return SL_STATUS_OK;
}

//ble_user_gain_table_lp_chain_8dBm_offset_handler
sl_status_t ble_user_gain_table_lp_chain_8dBm_offset_handler(void)
{ //console_args_t *arguments){
  sl_status_t status = SL_STATUS_OK;

  //! structure for the LP 0dBm POWER OFFSET
  uint8_t Si917_BLE_REGION_BASED_LP_CHAIN_10DBM_OFFSET_XX[128] = {
    5, //NUM_OF_REGIONS
    FCC,
    4, //NUM_OF_CHANNELS
    //chan_num    1M   2M   125kbps 500kbps
    255,
    63,
    63,
    63,
    63,
    0,
    63,
    63,
    63,
    63,
    19,
    63,
    63,
    63,
    63,
    39,
    63,
    35,
    63,
    63,
    ETSI,
    4, //NUM_OF_CHANNELS
    255,
    63,
    63,
    63,
    63,
    0,
    63,
    63,
    63,
    63,
    19,
    63,
    63,
    63,
    63,
    39,
    63,
    63,
    63,
    63,
    TELEC,
    4, //NUM_OF_CHANNELS
    255,
    63,
    63,
    63,
    63,
    0,
    63,
    63,
    63,
    63,
    19,
    63,
    63,
    63,
    63,
    39,
    63,
    63,
    63,
    63,
    WORLD_WIDE,
    4, //NUM_OF_CHANNELS
    255,
    63,
    63,
    63,
    63,
    0,
    63,
    63,
    63,
    63,
    19,
    63,
    63,
    63,
    63,
    39,
    63,
    63,
    63,
    63,
    KCC,
    4, //NUM_OF_CHANNELS
    255,
    63,
    63,
    63,
    63,
    0,
    63,
    63,
    63,
    63,
    19,
    63,
    63,
    63,
    63,
    39,
    63,
    63,
    63,
    63,
  };

  //! structure update for the LP_CHAIN 10dBm OFFSET
  status = rsi_bt_cmd_update_gain_table_offset_or_max_pwr(0,
                                                          sizeof(Si917_BLE_REGION_BASED_LP_CHAIN_10DBM_OFFSET_XX),
                                                          Si917_BLE_REGION_BASED_LP_CHAIN_10DBM_OFFSET_XX,
                                                          BLE_GAIN_TABLE_LP_CHAIN_10DBM_OFFSET_UPDATE);
  VERIFY_STATUS_AND_RETURN(status);
  return SL_STATUS_OK;
}
//_ble_tx_test_mode_handler
sl_status_t ble_tx_test_mode_handler(console_args_t *arguments)
{
  sl_status_t status = SL_STATUS_OK;

  uint8_t ble_tx_channel      = (sl_wifi_region_code_t)GET_COMMAND_ARG(arguments, 0);
  uint8_t ble_tx_phy          = (sl_wifi_region_code_t)GET_COMMAND_ARG(arguments, 1);
  uint8_t ble_tx_payload      = GET_OPTIONAL_COMMAND_ARG(arguments, 2, RSI_BLE_TX_PAYLOAD_LEN, const uint8_t);
  uint8_t ble_tx_payload_type = GET_OPTIONAL_COMMAND_ARG(arguments, 3, RSI_BLE_TX_PAYLOAD_TYPE, const uint8_t);
  if (wifi_deinit_var != ENABLE) {
    status = sl_si91x_disable_radio();
    VERIFY_STATUS_AND_RETURN(status);
    if (status == SL_STATUS_OK) {
      wifi_deinit_var = ENABLE;
      ;
    }
  }
  status = rsi_ble_tx_test_mode(ble_tx_channel, ble_tx_phy, ble_tx_payload, ble_tx_payload_type);

  VERIFY_STATUS_AND_RETURN(status);
  if (status == SL_STATUS_OK) {
    wifi_deinit_var = ENABLE;
    ;
  }
  return SL_STATUS_OK;
}

//ble_rx_test_mode_handler
sl_status_t ble_rx_test_mode_handler(console_args_t *arguments)
{
  sl_status_t status = SL_STATUS_OK;

  uint8_t ble_rx_channel = (sl_wifi_region_code_t)GET_COMMAND_ARG(arguments, 0);
  uint8_t ble_rx_phy     = (sl_wifi_region_code_t)GET_COMMAND_ARG(arguments, 1);
  if (wifi_deinit_var != ENABLE) {
    status = sl_si91x_disable_radio();
    VERIFY_STATUS_AND_RETURN(status);
    if (status == SL_STATUS_OK) {
      wifi_deinit_var = ENABLE;
      ;
    }
  }
  status = rsi_ble_rx_test_mode(ble_rx_channel, ble_rx_phy, RSI_BLE_STANDARD_MODULATION);

  VERIFY_STATUS_AND_RETURN(status);
  return SL_STATUS_OK;
}

sl_status_t wifi_radio_deinit_command_handler()
{
  sl_status_t status = SL_STATUS_OK;
  status             = sl_si91x_disable_radio();
  VERIFY_STATUS_AND_RETURN(status);
  wifi_deinit_var = ENABLE;
  return SL_STATUS_OK;
}

sl_status_t rsi_ble_per_transmit_command_handler(console_args_t *arguments)
{
  sl_status_t status                          = SL_STATUS_OK;
  rsi_ble_per_tx.cmd_ix                       = BLE_TRANSMIT_CMD_ID;
  rsi_ble_per_tx.transmit_enable              = (uint8_t)GET_COMMAND_ARG(arguments, 0);
  *(uint32_t *)&rsi_ble_per_tx.access_addr[0] = BLE_ACCESS_ADDR;
  *(uint16_t *)&rsi_ble_per_tx.pkt_len[0]     = (uint16_t)GET_COMMAND_ARG(arguments, 1);
  rsi_ble_per_tx.phy_rate                     = (uint8_t)GET_COMMAND_ARG(arguments, 2);
  rsi_ble_per_tx.rx_chnl_num                  = (uint8_t)GET_COMMAND_ARG(arguments, 3);
  rsi_ble_per_tx.tx_chnl_num                  = (uint8_t)GET_COMMAND_ARG(arguments, 3);
  rsi_ble_per_tx.scrambler_seed               = GET_OPTIONAL_COMMAND_ARG(arguments, 11, SCRAMBLER_SEED, const uint8_t);
  rsi_ble_per_tx.payload_type  = GET_OPTIONAL_COMMAND_ARG(arguments, 10, DATA_PRBS9, const uint8_t); //DATA_PRBS9;
  rsi_ble_per_tx.le_chnl_type  = LE_DATA_CHNL_TYPE;
  rsi_ble_per_tx.tx_power      = (uint8_t)GET_COMMAND_ARG(arguments, 4);
  rsi_ble_per_tx.transmit_mode = (uint8_t)GET_COMMAND_ARG(arguments, 5);
  rsi_ble_per_tx.freq_hop_en   = NO_HOPPING;
  rsi_ble_per_tx.ant_sel       = GET_OPTIONAL_COMMAND_ARG(arguments, 6, 2, const uint8_t);
  rsi_ble_per_tx.inter_pkt_gap = GET_OPTIONAL_COMMAND_ARG(arguments, 7, 0, const uint8_t);
  rsi_ble_per_tx.pll_mode      = PLL_MODE_0;
  rsi_ble_per_tx.rf_type       = BLE_INTERNAL_RF;
  rsi_ble_per_tx.rf_chain      = GET_OPTIONAL_COMMAND_ARG(arguments, 8, 2, const uint8_t);
  *(uint32_t *)&rsi_ble_per_tx.num_pkts[0] = GET_OPTIONAL_COMMAND_ARG(arguments, 9, 0, const uint32_t);
  if (wifi_deinit_var != ENABLE) {
    status = sl_si91x_disable_radio();
    VERIFY_STATUS_AND_RETURN(status);
    if (status == SL_STATUS_OK) {
      wifi_deinit_var = ENABLE;
      ;
    }
  }
  status = rsi_ble_per_transmit(&rsi_ble_per_tx);

  VERIFY_STATUS_AND_RETURN(status);
  return SL_STATUS_OK;
}

sl_status_t rsi_ble_per_receive_command_handler(console_args_t *arguments)
{
  sl_status_t status = SL_STATUS_OK;

  rsi_ble_per_rx.cmd_ix                       = BLE_RECEIVE_CMD_ID;
  rsi_ble_per_rx.receive_enable               = (uint8_t)GET_COMMAND_ARG(arguments, 0);
  *(uint32_t *)&rsi_ble_per_rx.access_addr[0] = BLE_ACCESS_ADDR;
  rsi_ble_per_rx.ext_data_len_indication      = EXT_DATA_LEN_IND;
  rsi_ble_per_rx.phy_rate                     = (uint8_t)GET_COMMAND_ARG(arguments, 1);
  rsi_ble_per_rx.rx_chnl_num                  = (uint8_t)GET_COMMAND_ARG(arguments, 2);
  rsi_ble_per_rx.tx_chnl_num                  = (uint8_t)GET_COMMAND_ARG(arguments, 2);
  rsi_ble_per_rx.scrambler_seed               = SCRAMBLER_SEED;
  rsi_ble_per_rx.le_chnl_type                 = LE_DATA_CHNL_TYPE;
  rsi_ble_per_rx.loop_back_mode               = LOOP_BACK_MODE_DISABLE;
  rsi_ble_per_rx.freq_hop_en                  = NO_HOPPING;
  rsi_ble_per_rx.ant_sel                      = GET_OPTIONAL_COMMAND_ARG(arguments, 3, 2, const uint8_t);
  rsi_ble_per_rx.duty_cycling_en              = DUTY_CYCLING_DISABLE;
  rsi_ble_per_rx.pll_mode                     = PLL_MODE_0;
  rsi_ble_per_rx.rf_type                      = BLE_INTERNAL_RF;
  rsi_ble_per_rx.rf_chain                     = GET_OPTIONAL_COMMAND_ARG(arguments, 4, 2, const uint8_t); //16
  if (wifi_deinit_var != ENABLE) {
    status = sl_si91x_disable_radio();
    VERIFY_STATUS_AND_RETURN(status);
    if (status == SL_STATUS_OK) {
      wifi_deinit_var = ENABLE;
      ;
    }
  }
  //! start the Receive PER functionality
  status = rsi_ble_per_receive(&rsi_ble_per_rx);

  VERIFY_STATUS_AND_RETURN(status);
  if (status == SL_STATUS_OK) {
    wifi_deinit_var = ENABLE;
    ;
  }
  return SL_STATUS_OK;
}

sl_status_t rsi_bt_per_stats_command_handler(console_args_t *arguments)
{
  UNUSED_PARAMETER(arguments);
  sl_status_t status    = SL_STATUS_OK;
  uint8_t time_duration = GET_OPTIONAL_COMMAND_ARG(arguments, 0, 1, const uint8_t);
  for (int z = 1; z <= time_duration; z++) {
    status = rsi_bt_per_stats(BT_PER_STATS_CMD_ID, &per_stats);
    VERIFY_STATUS_AND_RETURN(status);
    osDelay(1000);

    printf("\r\n{\r\n"
           "\t\"crc_fail_cnt\": %u\r\n"
           "\t\"crc_pass_cnt\": %u\r\n"
           "\t\"tx_dones\": %u\r\n"
           "\t\"rssi\": %d\r\n"
           "\t\"id_pkts_rcvd\":%u\r\n"
           "}\r\n",
           per_stats.crc_fail_cnt,
           per_stats.crc_pass_cnt,
           per_stats.tx_dones,
           per_stats.rssi,
           per_stats.id_pkts_rcvd);
  }
  return SL_STATUS_OK;
}

sl_status_t rsi_bt_set_local_name_command_handler(console_args_t *arguments)
{
  UNUSED_PARAMETER(arguments);
  sl_status_t status = SL_STATUS_OK;
  status             = rsi_bt_set_local_name((uint8_t *)RSI_BLE_LOCAL_NAME);
  VERIFY_STATUS_AND_RETURN(status);
  return status;
}

sl_status_t rsi_bt_get_local_name_command_handler(console_args_t *arguments)
{
  UNUSED_PARAMETER(arguments);
  sl_status_t status = SL_STATUS_OK;
  status             = rsi_bt_get_local_name(&rsi_app_resp_get_local_name);
  VERIFY_STATUS_AND_RETURN(status);

  printf("\r\n%s\r\n", rsi_app_resp_get_local_name.name);
  return status;
}

sl_status_t rsi_bt_get_local_device_address_command_handler(console_args_t *arguments)
{
  UNUSED_PARAMETER(arguments);
  sl_status_t status = SL_STATUS_OK;
  status             = rsi_bt_get_local_device_address(rsi_app_resp_get_dev_addr);
  VERIFY_STATUS_AND_RETURN(status);

  rsi_6byte_dev_address_to_ascii(local_dev_addr, (uint8_t *)rsi_app_resp_get_dev_addr);
  printf("\r\n%s\r\n", local_dev_addr);
  return status;
}

sl_status_t rsi_ble_set_advertise_data_command_handler(console_args_t *arguments)
{
  UNUSED_PARAMETER(arguments);
  sl_status_t status = SL_STATUS_OK;

  //! prepare advertise data //local/device name
  adv[3] = strlen(RSI_BLE_LOCAL_NAME) + 1;
  adv[4] = 9;
  strcpy((char *)&adv[5], RSI_BLE_LOCAL_NAME);

  status = rsi_ble_set_advertise_data(adv, strlen(RSI_BLE_LOCAL_NAME) + 5);
  VERIFY_STATUS_AND_RETURN(status);
  return status;
}
void rsi_ble_simple_peripheral_on_conn_status_event(rsi_ble_event_conn_status_t *resp_conn)
{
  static rsi_ble_event_conn_status_t rsi_app_connected_device = { 0 };
  static uint8_t str_remote_address[18]                       = { '\0' };
  memcpy(&rsi_app_connected_device, resp_conn, sizeof(rsi_ble_event_conn_status_t));
  rsi_6byte_dev_address_to_ascii(str_remote_address, rsi_app_connected_device.dev_addr);
  printf("\r\n Module connected to address : %s \r\n", str_remote_address);
}

void rsi_ble_simple_peripheral_on_disconnect_event(rsi_ble_event_disconnect_t *resp_disconnect, uint16_t reason)
{
  UNUSED_PARAMETER(reason); //This statement is added only to resolve compilation warning, value is unchanged
  static rsi_ble_event_disconnect_t rsi_app_disconnected_device = { 0 };
  static uint8_t str_remote_address[18]                         = { '\0' };
  memcpy(&rsi_app_disconnected_device, resp_disconnect, sizeof(rsi_ble_event_disconnect_t));
  rsi_6byte_dev_address_to_ascii(str_remote_address, rsi_app_disconnected_device.dev_addr);
  printf("\r\n Module disconnected to address : %s \r\n", str_remote_address);
}
void rsi_ble_simple_peripheral_on_enhance_conn_status_event(rsi_ble_event_enhance_conn_status_t *resp_enh_conn)
{
  static rsi_ble_event_enhance_conn_status_t rsi_app_connected_device;
  static uint8_t str_remote_address[18] = { '\0' };
  memcpy(&rsi_app_connected_device, resp_enh_conn, sizeof(rsi_ble_event_enhance_conn_status_t));
  rsi_6byte_dev_address_to_ascii(str_remote_address, rsi_app_connected_device.dev_addr);
  printf("\r\n Module connected to address : %s \r\n", str_remote_address);
}
sl_status_t rsi_ble_start_advertising_command_handler(console_args_t *arguments)
{
  UNUSED_PARAMETER(arguments);
  sl_status_t status = SL_STATUS_OK;
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
  status = rsi_ble_start_advertising();
  VERIFY_STATUS_AND_RETURN(status);
  return status;
}

sl_status_t rsi_ble_stop_advertising_command_handler(console_args_t *arguments)
{
  UNUSED_PARAMETER(arguments);
  sl_status_t status = SL_STATUS_OK;
  status             = rsi_ble_stop_advertising();
  VERIFY_STATUS_AND_RETURN(status);
  return status;
}

sl_status_t ble_end_test_command_handler(console_args_t *arguments)
{
  UNUSED_PARAMETER(arguments);
  sl_status_t status = SL_STATUS_OK;
  status             = rsi_ble_end_test_mode(&testmodes_num_of_packets);
  VERIFY_STATUS_AND_RETURN(status);
  printf("\r\nNum of packets:%d\r\n", testmodes_num_of_packets);
  return SL_STATUS_OK;
}
