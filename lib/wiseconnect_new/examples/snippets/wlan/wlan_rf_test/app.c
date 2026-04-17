/***************************************************************************/ /**
 * @file
 * @brief Rf_Test Example Application
 *******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
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

#include <string.h>
#include "sl_status.h"
#include "cmsis_os2.h"
#include "sl_wifi.h"
#include "sl_net.h"
#include "socket.h"
#include "sl_ip_types.h"
#include "sl_utility.h"
#include "sl_net_si91x.h"
#include "errno.h"
#include "sl_net_wifi_types.h"
#include "sl_wifi_callback_framework.h"
#include "sl_si91x_driver.h"
#ifdef SLI_SI91X_MCU_INTERFACE
#include "rsi_rom_clks.h"
#endif

const sl_wifi_data_rate_t rate    = SL_WIFI_DATA_RATE_6;
const sl_wifi_tx_test_mode_t mode = SL_WIFI_TEST_BURST_MODE;

#define RECEIVE_STATS           0
#define MAX_RECEIVE_STATS_COUNT 5

static const sl_wifi_device_configuration_t transmit_test_configuration = {
  .boot_option = LOAD_NWP_FW,
  .mac_address = NULL,
  .band        = SL_SI91X_WIFI_BAND_2_4GHZ,
  .region_code = WORLD_DOMAIN,
  .boot_config = { .oper_mode = SL_SI91X_TRANSMIT_TEST_MODE,
                   .coex_mode = SL_SI91X_WLAN_ONLY_MODE,
                   .feature_bit_map =
#ifdef SLI_SI91X_MCU_INTERFACE
                     (SL_WIFI_FEAT_SECURITY_OPEN | SL_WIFI_FEAT_WPS_DISABLE),
#else
                     (SL_WIFI_FEAT_SECURITY_OPEN),
#endif
                   .tcp_ip_feature_bit_map =
                     (SL_SI91X_TCP_IP_FEAT_DHCPV4_CLIENT | SL_SI91X_TCP_IP_FEAT_EXTENSION_VALID),
                   .custom_feature_bit_map     = SL_WIFI_SYSTEM_CUSTOM_FEAT_EXTENSION_VALID,
                   .ext_custom_feature_bit_map = (MEMORY_CONFIG
#ifdef SLI_SI917
                                                  | SL_SI91X_EXT_FEAT_FRONT_END_SWITCH_PINS_ULP_GPIO_4_5_0
#endif
                                                  ),
                   .bt_feature_bit_map         = SL_SI91X_BT_RF_TYPE,
                   .ext_tcp_ip_feature_bit_map = SL_SI91X_CONFIG_FEAT_EXTENTION_VALID,
                   .ble_feature_bit_map        = 0,
                   .ble_ext_feature_bit_map    = 0,
                   .config_feature_bit_map     = SL_SI91X_FEAT_SLEEP_GPIO_SEL_BITMAP }
};

/******************************************************
 *               Variable Definitions
 ******************************************************/

const osThreadAttr_t thread_attributes = {
  .name       = "app",
  .attr_bits  = 0,
  .cb_mem     = 0,
  .cb_size    = 0,
  .stack_mem  = 0,
  .stack_size = 3072,
  .priority   = osPriorityLow,
  .tz_module  = 0,
  .reserved   = 0,
};

#if RECEIVE_STATS
static uint8_t stats_count = 0;
#endif
volatile sl_status_t callback_status = SL_STATUS_OK;

sl_wifi_request_tx_test_info_t tx_test_info = {
  .enable      = 1,
  .power       = 127,
  .rate        = rate,
  .length      = 100,
  .mode        = mode,
  .channel     = 1,
  .aggr_enable = 0,
  .no_of_pkts  = 0,
#ifdef SLI_SI917
  .enable_11ax            = 0,
  .coding_type            = 0,
  .nominal_pe             = 0,
  .ul_dl                  = 0,
  .he_ppdu_type           = 0,
  .beam_change            = 0,
  .bw                     = 0,
  .stbc                   = 0,
  .tx_bf                  = 0,
  .gi_ltf                 = 0,
  .dcm                    = 0,
  .nsts_midamble          = 0,
  .spatial_reuse          = 0,
  .bss_color              = 0,
  .he_siga2_reserved      = 0,
  .ru_allocation          = 0,
  .n_heltf_tot            = 0,
  .sigb_dcm               = 0,
  .sigb_mcs               = 0,
  .user_sta_id            = 0,
  .user_idx               = 0,
  .sigb_compression_field = 0,
#endif
};

float pass_avg          = 0;
float fail_avg          = 0;
uint32_t rssi_avg       = 0;
uint32_t crc_pass       = 0;
uint32_t crc_fail       = 0;
uint32_t cal_rssi       = 0;
uint16_t total_crc_pass = 0;
uint16_t total_crc_fail = 0;

/******************************************************
 *               Function Declarations
 ******************************************************/

static void application_start(void *argument);
#if RECEIVE_STATS
static sl_status_t wifi_stats_receive_handler(sl_wifi_event_t event,
                                              sl_status_t status_code,
                                              void *result,
                                              uint32_t result_length,
                                              void *arg);
#endif

/******************************************************
 *               Function Definitions
 ******************************************************/

void app_init(void)
{
  osThreadNew((osThreadFunc_t)application_start, NULL, &thread_attributes);
}

static void application_start(void *argument)
{
  UNUSED_PARAMETER(argument);
  sl_status_t status;

  status = sl_net_init(SL_NET_WIFI_CLIENT_INTERFACE, &transmit_test_configuration, NULL, NULL);
  if (status != SL_STATUS_OK) {
    printf("Failed to start Wi-Fi client interface: 0x%lx\r\n", status);
    return;
  }
  printf("\r\nWi-Fi Init Done \r\n");

#if RECEIVE_STATS
  // Register WLAN receive stats call back handler
  sl_wifi_set_stats_callback_v2(wifi_stats_receive_handler, NULL);
#endif

  status = sl_wifi_set_antenna(SL_WIFI_CLIENT_2_4GHZ_INTERFACE, SL_WIFI_ANTENNA_INTERNAL);
  if (status != SL_STATUS_OK) {
    printf("Failed to start set Antenna: 0x%lx\r\n", status);
    return;
  }
  printf("\r\nAntenna Command Frame Success \r\n");

  if ((mode == SL_WIFI_TEST_CONTINOUS_WAVE_MODE) || (mode == SL_WIFI_TEST_CONTINOUS_WAVE_MODE_OFF_CENTER_LOW)
      || (mode == SL_WIFI_TEST_CONTINOUS_WAVE_MODE_OFF_CENTER_HIGH)) {
    tx_test_info.mode = SL_WIFI_TEST_CONTINOUS_MODE;
    status            = sl_si91x_transmit_test_start(&tx_test_info);
    if (status != SL_STATUS_OK) {
      printf("\r\nTransmit test start Failed, Error Code : 0x%lX\r\n", status);
      return;
    }
    printf("\r\nTransmit test start Success\r\n");

    // Add delay here to see the TX packets on AIR
    osDelay(1000);

    status = sl_si91x_transmit_test_stop();
    if (status != SL_STATUS_OK) {
      printf("\r\nTransmit test stop Failed, Error Code : 0x%lX\r\n", status);
      return;
    }
    printf("\r\nTransmit test stop Success\r\n");

    osDelay(1000);
  }

  tx_test_info.mode = mode;
  status            = sl_si91x_transmit_test_start(&tx_test_info);
  if (status != SL_STATUS_OK) {
    printf("\r\nTransmit test start Failed, Error Code : 0x%lX\r\n", status);
    return;
  }
  printf("\r\nTransmit test start Success\r\n");

  // Add delay here to see the TX packets on AIR
  osDelay(1000);

  status = sl_si91x_transmit_test_stop();
  if (status != SL_STATUS_OK) {
    printf("\r\nTransmit test stop Failed, Error Code : 0x%lX\r\n", status);
    return;
  }
  printf("\r\nTransmit test stop Success\r\n");

#if RECEIVE_STATS
  ////////////////////////////////////////
  // Transmit data/TX from the peer//////
  ////////////////////////////////////////

  // Start/Receive publishing RX stats
  sl_wifi_channel_t channel = { 0 };
  channel.channel           = 1;
  status                    = sl_wifi_start_statistic_report(SL_WIFI_CLIENT_INTERFACE, channel);
  if (SL_STATUS_IN_PROGRESS == status) {
    callback_status = SL_STATUS_IN_PROGRESS;
    printf("\r\nReceive Statistics...\r\n");
    while (stats_count <= MAX_RECEIVE_STATS_COUNT) {
      osThreadYield();
      if (stats_count == MAX_RECEIVE_STATS_COUNT && callback_status != SL_STATUS_IN_PROGRESS) {
        printf("\r\n%s: Stop Statistics Report\n", __func__);
        sl_wifi_stop_statistic_report(SL_WIFI_CLIENT_INTERFACE);
        printf("\r\nStart Statistic Report Success\r\n");
        return;
      }
    }
    status = callback_status;
  }
  if (status != SL_STATUS_OK) {
    printf("\r\nStart Statistic Report Failed, Error Code : 0x%lX\r\n", status);
    return;
  }
#endif
}

#if RECEIVE_STATS
sl_status_t wifi_stats_receive_handler(sl_wifi_event_t event,
                                       sl_status_t status_code,
                                       void *reponse,
                                       uint32_t result_length,
                                       void *arg)
{
  UNUSED_PARAMETER(result_length);
  UNUSED_PARAMETER(arg);
  if (SL_WIFI_CHECK_IF_EVENT_FAILED(event)) {
    callback_status = status_code;
    return status_code;
  }

  if (event == SL_WIFI_STATS_ASYNC_EVENT) {
    sl_wifi_async_stats_response_t *result = (sl_wifi_async_stats_response_t *)reponse;

    printf("\r\n%s: WIFI STATS Recieved packet# %d\n", __func__, stats_count);
    printf("stats : crc_pass %d, crc_fail %d, cal_rssi :%d\n", result->crc_pass, result->crc_fail, result->cal_rssi);
    float p = result->crc_pass;
    float f = result->crc_fail;
    float t = p + f;

    float per_pass = (p * 100 / t);
    float per_fail = (f * 100 / t);

    pass_avg += per_pass;
    fail_avg += per_fail;

    total_crc_pass += result->crc_pass;
    total_crc_fail += result->crc_fail;

    if (stats_count == MAX_RECEIVE_STATS_COUNT - 1) {
      printf("\r\n CRC Average pass%% = %.6f,         CRC Average fail%% = %.6f \r\n",
             pass_avg / MAX_RECEIVE_STATS_COUNT,
             fail_avg / MAX_RECEIVE_STATS_COUNT);
      printf("Total : total_crc_pass %d, total_crc_fail %d \n", total_crc_pass, total_crc_fail);
      pass_avg = 0;
      fail_avg = 0;
    }
    stats_count++;
    callback_status = SL_STATUS_OK;
  }
  return SL_STATUS_OK;
}
#endif
