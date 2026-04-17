/***************************************************************************/ /**
 * @file
 * @brief User Gain Table Example Application
 *******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
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
const sl_wifi_data_rate_t rate = SL_WIFI_DATA_RATE_6;

//! Bandwidth 0 = 20Mhz
#define SL_WIFI_BANDWIDTH_20MHz 0

//! Select bandwidth either 20Mhz
#define BANDWIDTH SL_WIFI_BANDWIDTH_20MHz

//! Select band
#define BAND SL_SI91X_WIFI_BAND_2_4GHZ

#define BAND_VALUE 1

//! Update Bump up offset for 52 Tone RU as per Region
#define X_BUMP_UP_OFFSET_52_TONE_RU 1

//! Update Bump up offset for 106 Tone RU as per Region
#define Y_BUMP_UP_OFFSET_106_TONE_RU 2

/* clang-format off */
  uint8_t su_gain_table_payload[]  = {
    4,//NUM_OF_REGIONS
        0, 0xB,//NUM_OF_CHANNELS
    //   rate,  11b, 11g, 11n, 11ax(SU) 11ax(TB)
            1,  32,  22,  22,  20,      14,
            2,  32,  26,  26,  24,      20,
            3,  34,  28,  28,  26,      30,
            4,  36,  34,  32,  30,      32,
            5,  36,  34,  34,  32,      32,
            6,  36,  36,  36,  34,      34,
            7,  36,  34,  34,  32,      28,
            8,  36,  32,  32,  30,      28,
            9,  36,  30,  30,  28,      28,
           10,  32,  28,  28,  24,      14,
           11,  32,  22,  22,  20,      14,
        1,0x11,
           255, 34,  36,  36,  36,      24,
        2,0x22,
           13,  34,  38,  36,  36,      24,
           14,  38,   0,   0,   0,       0,
        5, 0x25,
           1,   32,  22,  22,  20,      14,
           2,   32,  26,  26,  24,      20,
           11,  32,  22,  22,  20,      14,
           13,  34,  36,  36,  36,      24,
           14,  38,  0,   0,    0,      0,
    };
/* clang-format on */

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

sl_wifi_request_tx_test_info_t tx_test_info = {
  .enable      = 1,
  .power       = 127,
  .rate        = rate,
  .length      = 1000,
  .mode        = 0,
  .channel     = 1,
  .aggr_enable = 0,
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

/******************************************************
 *               Function Declarations
 ******************************************************/

static void application_start(void *argument);

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

  status = sl_net_init(SL_NET_WIFI_CLIENT_INTERFACE, &sl_wifi_default_transmit_test_configuration, NULL, NULL);
  if (status != SL_STATUS_OK) {
    printf("Failed to start Wi-Fi client interface: 0x%lx\r\n", status);
    return;
  }
  printf("\r\nWi-Fi Init Done \r\n");

  status = sl_wifi_set_antenna(SL_WIFI_CLIENT_2_4GHZ_INTERFACE, SL_WIFI_ANTENNA_INTERNAL);
  if (status != SL_STATUS_OK) {
    printf("Failed to start set Antenna, Error Code: 0x%lx\r\n", status);
    return;
  }
  printf("\r\nAntenna Command Frame Success \r\n");

  status = sl_wifi_update_su_gain_table(BAND_VALUE,
                                        BANDWIDTH,
                                        su_gain_table_payload,
                                        sizeof(su_gain_table_payload),
                                        X_BUMP_UP_OFFSET_52_TONE_RU,
                                        Y_BUMP_UP_OFFSET_106_TONE_RU);
  if (status != SL_STATUS_OK) {
    printf("Failed to update gain table, Error Code: 0x%lx\r\n", status);
    return;
  }
  printf("\r\nUpdate gain table Success \r\n");

  status = sl_si91x_transmit_test_start(&tx_test_info);

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

  return;
}