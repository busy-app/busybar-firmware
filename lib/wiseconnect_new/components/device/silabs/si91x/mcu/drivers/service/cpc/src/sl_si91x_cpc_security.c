/*******************************************************************************
 * @file  sl_si91x_cpc_security.c
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

#include "cmsis_os2.h"
#include "rsi_os.h"
#include "FreeRTOS.h"

#include "nvm3_generic.h"
#include "sl_net.h"
#include "sl_wifi.h"
#include "sl_si91x_psa_trng.h"
#include "sli_si91x_crypto_driver_functions.h"
#include "sl_si91x_cpc_security.h"
#include "rsi_debug.h"

osSemaphoreAttr_t sl_si91x_cpc_crypto_semaphore_attr_st;
osThreadId_t sl_si91x_cpc_crypto_init_thread;
osSemaphoreId_t sl_si91x_cpc_crypto_init_semaphore_id;
osStatus_t sl_si91x_cpc_crypto_init_semaphore_status;

osSemaphoreAttr_t sl_si91x_cpc_crypto_wait_semaphore_attr_st;
osSemaphoreId_t sl_si91x_cpc_crypto_wait_semaphore_id;
osStatus_t sl_si91x_cpc_crypto_wait_semaphore_status;

#define SL_SI91X_CPC_CRYPTO_INIT_TASK_STACK_SIZE 4096

const osThreadAttr_t cpc_crypto_init_thread_attributes = {
  .name       = "cpc_app", // Name of thread
  .attr_bits  = 0,
  .cb_mem     = 0,
  .cb_size    = 0,
  .stack_mem  = 0,
  .stack_size = SL_SI91X_CPC_CRYPTO_INIT_TASK_STACK_SIZE, // Stack size of sl_si91x_cpc_crypto_init_task task
  .priority   = osPriorityHigh, //osPriorityLow,                            // Priority of Sensor task
  .tz_module  = 0,
  .reserved   = 0,
};

static const sl_wifi_device_configuration_t client_configuration = {
  .boot_option = LOAD_NWP_FW,
  .mac_address = NULL,
  .band        = SL_WIFI_BAND_MODE_2_4GHZ,
  .region_code = SL_WIFI_REGION_US,
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
                     SL_SI91X_RAM_LEVEL_NWP_ALL_MCU_ZERO
#endif
#ifdef SLI_SI917
                     | SL_SI91X_EXT_FEAT_FRONT_END_SWITCH_PINS_ULP_GPIO_4_5_0
#endif
                     ),
                   .bt_feature_bit_map         = 0,
                   .ext_tcp_ip_feature_bit_map = 0,
                   .ble_feature_bit_map        = 0,
                   .ble_ext_feature_bit_map    = 0,
                   .config_feature_bit_map     = 0 }
};

// This function initializes a Semaphore with the `sl_si91x_cpc_crypto_semaphore_attr_st` atrribute
void sl_si91x_cpc_crypto_semaphore_init(void)
{
  sl_si91x_cpc_crypto_semaphore_attr_st.attr_bits = 0U;
  sl_si91x_cpc_crypto_semaphore_attr_st.cb_mem    = NULL;
  sl_si91x_cpc_crypto_semaphore_attr_st.cb_size   = 0U;
  sl_si91x_cpc_crypto_semaphore_attr_st.name      = NULL;
  sl_si91x_cpc_crypto_init_semaphore_id           = osSemaphoreNew(1U, 0U, &sl_si91x_cpc_crypto_semaphore_attr_st);
}

void sl_si91x_cpc_crypto_wait_semaphore_init(void)
{
  sl_si91x_cpc_crypto_wait_semaphore_attr_st.attr_bits = 0U;
  sl_si91x_cpc_crypto_wait_semaphore_attr_st.cb_mem    = NULL;
  sl_si91x_cpc_crypto_wait_semaphore_attr_st.cb_size   = 0U;
  sl_si91x_cpc_crypto_wait_semaphore_attr_st.name      = NULL;
  sl_si91x_cpc_crypto_wait_semaphore_id = osSemaphoreNew(1U, 0U, &sl_si91x_cpc_crypto_wait_semaphore_attr_st);
}

void sl_ta_trng_init(void)
{
  sl_status_t status = sl_net_init(SL_NET_WIFI_CLIENT_INTERFACE, &client_configuration, NULL, NULL);
  if (status != SL_STATUS_OK) {
    DEBUGOUT("Failed to start Wi-Fi client interface: 0x%lx\r\n", status);
    return;
  }
}

//  CPC Crypto Init Task performs the following operations:
//    - Create & acquire Semaphore to block calling of Crypto hardware drivers before TA is intialized.
//    - Initialize the TA to access Crypto Hardware Acclerators.
//    - Release the Semaphore so that `sli_cpc_security_init()` can call `psa_crypto_init()`.
void sl_si91x_cpc_crypto_init_task(void)
{
  sl_ta_trng_init();

  sl_si91x_cpc_crypto_init_semaphore_status = osSemaphoreRelease(sl_si91x_cpc_crypto_init_semaphore_id);
  if (sl_si91x_cpc_crypto_init_semaphore_status != osOK) {
    DEBUGOUT("Semaphore Release failed: %d \r\n", sl_si91x_cpc_crypto_init_semaphore_status);
  }

  sl_si91x_cpc_crypto_wait_semaphore_status = osSemaphoreAcquire(sl_si91x_cpc_crypto_wait_semaphore_id, osWaitForever);
  if (sl_si91x_cpc_crypto_wait_semaphore_status != osOK) {
    DEBUGOUT("Semaphore Acquire failed in task: %d \r\n", sl_si91x_cpc_crypto_wait_semaphore_status);
  }
}

//This function initializes the TA, making it ready for usage for Crypto Functions for CPC.
void sl_si91x_cpc_crypto_init(void)
{
  sl_si91x_cpc_crypto_semaphore_init();
  sl_si91x_cpc_crypto_wait_semaphore_init();
  sl_si91x_cpc_crypto_init_thread =
    osThreadNew((osThreadFunc_t)sl_si91x_cpc_crypto_init_task, NULL, &cpc_crypto_init_thread_attributes);
}
