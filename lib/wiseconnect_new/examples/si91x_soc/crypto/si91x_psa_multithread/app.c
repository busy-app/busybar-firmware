/***************************************************************************/ /**
 * @file
 * @brief  Example Application
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

#include "app.h"
#include "psa_multithread_app.h"
#include "sl_si91x_crypto_thread.h"

#include <string.h>
#include "cmsis_os2.h"
#include "sl_net.h"
#include "sl_wifi.h"
#include "sl_status.h"

// #define SAME_CRYPTO_ACCELERATORS
#define DIFFERENT_CRYPTO_ACCELERATORS

/******************************************************
 *               Variable Definitions
 ******************************************************/

osStatus_t os_status;
static osThreadId_t threadId0;
static osThreadId_t threadId1;
static osThreadId_t threadId2;

const osThreadAttr_t thread_attributes_0 = {
  .name       = "thread0_app",
  .attr_bits  = 0,
  .cb_mem     = 0,
  .cb_size    = 0,
  .stack_mem  = 0,
  .stack_size = 3072,
  .priority   = osPriorityLow,
  .tz_module  = 0,
  .reserved   = 0,
};

const osThreadAttr_t thread_attributes_1 = {
  .name       = "thread1_sha_app",
  .attr_bits  = 0,
  .cb_mem     = 0,
  .cb_size    = 0,
  .stack_mem  = 0,
  .stack_size = 3072,
  .priority   = osPriorityLow4,
  .tz_module  = 0,
  .reserved   = 0,
};

const osThreadAttr_t thread_attributes_2 = {
  .name       = "thread2_aes_app",
  .attr_bits  = 0,
  .cb_mem     = 0,
  .cb_size    = 0,
  .stack_mem  = 0,
  .stack_size = 3072,
  .priority   = osPriorityLow4,
  .tz_module  = 0,
  .reserved   = 0,
};

const osThreadAttr_t thread_attributes_2_same_acc = {
  .name       = "thread2_sha_app",
  .attr_bits  = 0,
  .cb_mem     = 0,
  .cb_size    = 0,
  .stack_mem  = 0,
  .stack_size = 3072,
  .priority   = osPriorityLow4,
  .tz_module  = 0,
  .reserved   = 0,
};

static const sl_wifi_device_configuration_t client_configuration = {
  .boot_option = LOAD_NWP_FW,
  .mac_address = NULL,
  .band        = SL_SI91X_WIFI_BAND_2_4GHZ,
  .region_code = US,
  .boot_config = { .oper_mode       = SL_SI91X_CLIENT_MODE,
                   .coex_mode       = SL_SI91X_WLAN_ONLY_MODE,
                   .feature_bit_map = (SL_WIFI_FEAT_SECURITY_PSK | SL_WIFI_FEAT_AGGREGATION
#ifdef SLI_SI91X_MCU_INTERFACE
                                       | SL_WIFI_FEAT_WPS_DISABLE
#endif
                                       ),
                   .tcp_ip_feature_bit_map     = (SL_SI91X_TCP_IP_FEAT_DHCPV4_CLIENT),
                   .custom_feature_bit_map     = (SL_WIFI_SYSTEM_CUSTOM_FEAT_EXTENSION_VALID),
                   .ext_custom_feature_bit_map = (
#ifdef SLI_SI91X_MCU_INTERFACE
                     SL_SI91X_RAM_LEVEL_NWP_ADV_MCU_BASIC
#else
                     SL_SI91X_RAM_LEVEL_NWP_ALL_AVAILABLE
#endif
#if defined(SLI_SI917)
                     | SL_SI91X_EXT_FEAT_FRONT_END_SWITCH_PINS_ULP_GPIO_4_5_0
#endif
                     ),
                   .bt_feature_bit_map         = 0,
                   .ext_tcp_ip_feature_bit_map = 0,
                   .ble_feature_bit_map        = 0,
                   .ble_ext_feature_bit_map    = 0,
                   .config_feature_bit_map     = 0 }
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
  osThreadNew((osThreadFunc_t)application_start, NULL, &thread_attributes_0);
}

static void application_start(void *argument)
{
  threadId0 = osThreadGetId();
  UNUSED_PARAMETER(argument);

  sl_status_t status = sl_net_init(SL_NET_WIFI_CLIENT_INTERFACE, &client_configuration, NULL, NULL);
  if (status != SL_STATUS_OK) {
    printf("Failed to start Wi-Fi client interface: 0x%lx\r\n", status);
    return;
  }
  printf("\r\nWi-Fi Init Success\r\n");

  psa_status_t ret;
  ret = psa_crypto_init();
  if (ret != PSA_SUCCESS) {
    printf("PSA Crypto Init failed with status : %ld\r\n", ret);
  } else {
    printf("PSA Crypto Init Success\r\n");
  }

  if (osThreadNew((osThreadFunc_t)sha_app_process, NULL, &thread_attributes_1) == NULL) {
    printf("\r\n SHA App1 fail \r\n");
    return;
  }
  printf("\r\n SHA_APP1_Task created \r\n");

#if defined(DIFFERENT_CRYPTO_ACCELERATORS)
  if (osThreadNew((osThreadFunc_t)aes_app_process, NULL, &thread_attributes_2) == NULL) {
    printf("\r\n AES App2 fail \r\n");
    return;
  }
  printf("\r\n AES_APP2_Task created \r\n");
#endif

#if defined(SAME_CRYPTO_ACCELERATORS)
  if (osThreadNew((osThreadFunc_t)sha_app_process_2, NULL, &thread_attributes_2_same_acc) == NULL) {
    printf("\r\n SHA App2 fail \r\n");
    return;
  }
  printf("\r\n SHA_APP2_Task created \r\n");
#endif

  os_status = osThreadSuspend(threadId0);
}

/***************************************************************************/ /**
 * Application state machine, called infinitely.
 ******************************************************************************/
void sha_app_process(void)
{
  threadId1 = osThreadGetId();
  sha_app_process_action();
  printf("\r\n SHA operation done\r\n");
  os_status = osThreadSuspend(threadId1);
}

void aes_app_process(void)
{
  threadId2 = osThreadGetId();
  aes_app_process_action(PSA_ALG_ECB_NO_PADDING);
  printf("\r\n AES operation done\r\n");
  os_status = osThreadSuspend(threadId2);
}

void sha_app_process_2(void)
{
  threadId2 = osThreadGetId();
  sha_app_process_action();
  printf("\r\n SHA operation done\r\n");
  os_status = osThreadSuspend(threadId2);
}
