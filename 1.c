/***************************************************************************/ /**
 * @file
 * @brief Powersave Deep Sleep Example Application
 *******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
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

#include "cmsis_os2.h"
#include "sl_net.h"
#include "sl_board_configuration.h"
#include "sl_wifi_types.h"
#include "sl_wifi_device.h"
#include "sl_wifi.h"
#include "sl_si91x_driver.h"

#ifdef SLI_SI91X_MCU_INTERFACE
#include "sl_si91x_power_manager.h"
#endif // SLI_SI91X_MCU_INTERFACE

/******************************************************
*                    Constants
******************************************************/
#define POWER_SAVE_PROFILE DEEP_SLEEP_WITHOUT_RAM_RETENTION

/******************************************************
 *               Function Declarations
 ******************************************************/
static void application_start(void *argument);
static void enable_standby(void);

#ifndef SLI_SI91X_MCU_INTERFACE
static void enable_high_performance(void);
#endif

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

static const sl_wifi_device_configuration_t station_init_configuration = {
  .boot_option = LOAD_NWP_FW,
  .mac_address = NULL,
  .band        = SL_SI91X_WIFI_BAND_2_4GHZ,
  .boot_config = { .oper_mode       = SL_SI91X_CLIENT_MODE,
                   .coex_mode       = SL_SI91X_WLAN_ONLY_MODE,
                   .feature_bit_map = (SL_SI91X_FEAT_SECURITY_OPEN | SL_SI91X_FEAT_ULP_GPIO_BASED_HANDSHAKE
#ifdef SLI_SI91X_MCU_INTERFACE
                                       | SL_SI91X_FEAT_WPS_DISABLE
#endif
                                       ),
                   .tcp_ip_feature_bit_map =
                     (SL_SI91X_TCP_IP_FEAT_DHCPV4_CLIENT | SL_SI91X_TCP_IP_FEAT_EXTENSION_VALID),
                   .custom_feature_bit_map     = (SL_SI91X_CUSTOM_FEAT_EXTENTION_VALID),
                   .ext_custom_feature_bit_map = (SL_SI91X_EXT_FEAT_LOW_POWER_MODE | MEMORY_CONFIG
#if defined(SLI_SI917) || defined(SLI_SI915)
                                                  | SL_SI91X_EXT_FEAT_FRONT_END_SWITCH_PINS_ULP_GPIO_4_5_0
#endif
                                                  ),
                   .bt_feature_bit_map         = 0,
                   .ext_tcp_ip_feature_bit_map = SL_SI91X_CONFIG_FEAT_EXTENTION_VALID,
                   .ble_feature_bit_map        = 0,
                   .ble_ext_feature_bit_map    = 0,
                   .config_feature_bit_map     = SL_SI91X_FEAT_SLEEP_GPIO_SEL_BITMAP }
};

/******************************************************
 *               Function Definitions
 ******************************************************/

void app_init(const void *unused)
{
  UNUSED_PARAMETER(unused);
  osThreadNew((osThreadFunc_t)application_start, NULL, &thread_attributes);
}

static void application_start(void *argument)
{
  UNUSED_PARAMETER(argument);
  sl_status_t status;
  // Initialize the Wi-Fi client interface
  status = sl_net_init(SL_NET_WIFI_CLIENT_INTERFACE, &station_init_configuration, NULL, NULL);
  if (status != SL_STATUS_OK) {
    printf("\r\nFailed to bring Wi-Fi client interface up: 0x%lX\r\n", status);
    return;
  }
  printf("\r\nWi-Fi client interface init success\r\n");

#ifdef SLI_SI91X_MCU_INTERFACE
  uint8_t xtal_enable = 1;
  // Establish a secure handshake between the M4 core and the NWP
  status = sl_si91x_m4_ta_secure_handshake(SL_SI91X_ENABLE_XTAL, 1, &xtal_enable, 0, NULL);
  if (status != SL_STATUS_OK) {
    printf("\r\nFailed to bring m4_ta_secure_handshake: 0x%lX\r\n", status);
    return;
  }
  printf("\r\nm4_ta_secure_handshake Success\r\n");
#endif

  // Enabling low-power standby mode
  enable_standby();
  printf("\r\nNWP is in power save mode\r\n");

#ifdef SLI_SI91X_MCU_INTERFACE
  printf("\r\nM4 in Sleep\r\n");
  status = sl_si91x_power_manager_add_ps_requirement(SL_SI91X_POWER_MANAGER_PS0);
  if (status != SL_STATUS_OK) {
    // If status is not OK, return with the error code.
    printf("\r\nsl_si91x_power_manager_add_ps_requirement failed, Error Code: 0x%lX\r\n", status);
  } else {
    printf("\r\nM4 wake up\r\n");
  }

#else
  osDelay(30000);

  if (POWER_SAVE_PROFILE == DEEP_SLEEP_WITH_RAM_RETENTION) {

    // Brings the NWP out of power save mode
    enable_high_performance();
    printf("\r\nNWP comes out of power save mode\r\n");

  } else if (POWER_SAVE_PROFILE == DEEP_SLEEP_WITHOUT_RAM_RETENTION) {

    // Initialize the Wi-Fi client interface
    status = sl_net_init(SL_NET_WIFI_CLIENT_INTERFACE, &station_init_configuration, NULL, NULL);
    if (status != SL_STATUS_OK) {
      printf("\r\nFailed to bring Wi-Fi client interface up: 0x%lX\r\n", status);
      return;
    }
    printf("\r\nWi-Fi client interface init success\r\n");
  }

  printf("\r\nExample Demonstration Completed\r\n");

#endif
}

static void enable_standby(void)
{
  sl_wifi_performance_profile_v2_t performance_profile = { .profile = POWER_SAVE_PROFILE };

  sl_status_t status = sl_wifi_set_performance_profile_v2(&performance_profile);
  if (status != SL_STATUS_OK) {
    printf("\r\nPower save profile with deep sleep Failed, Error Code : 0x%lX\r\n", status);
    return;
  }
  printf("\r\nPower save profile with deep sleep Success \r\n");
}

#ifndef SLI_SI91X_MCU_INTERFACE
static void enable_high_performance(void)
{
  sl_wifi_performance_profile_v2_t performance_profile = { .profile = HIGH_PERFORMANCE };

  sl_status_t status = sl_wifi_set_performance_profile_v2(&performance_profile);
  if (status != SL_STATUS_OK) {
    printf("\r\nFailed to keep module in HIGH_PERFORMANCE mode, Error Code : 0x%lX\r\n", status);
    return;
  }
  printf("\r\nModule is in HIGH_PERFORMANCE mode\r\n");
}
#endif