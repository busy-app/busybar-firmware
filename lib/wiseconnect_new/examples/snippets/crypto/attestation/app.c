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
#include <stdio.h>
#include "rsi_error.h"
#include "string.h"
#include "app.h"
#include "t_cose_common.h"
#include "t_cose_sign1_sign.h"
#include "t_cose_sign1_verify.h"
#include "q_useful_buf.h"
#include "sl_status.h"
#include "cmsis_os2.h"
#include "sl_net.h"
#include "sl_si91x_driver.h"
#include "sl_component_catalog.h"
#include "sl_si91x_attestation.h"
#include "sl_si91x_trng.h"
/******************************************************
 *                    Constants
 ******************************************************/
uint8_t attestation_token[LENGTH_OF_TOKEN];
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
  .region_code = US,
  .boot_config = { .oper_mode              = SL_SI91X_CLIENT_MODE,
                   .coex_mode              = SL_SI91X_WLAN_ONLY_MODE,
                   .feature_bit_map        = (SL_WIFI_FEAT_SECURITY_PSK | SL_WIFI_FEAT_AGGREGATION),
                   .tcp_ip_feature_bit_map = (SL_SI91X_TCP_IP_FEAT_DHCPV4_CLIENT | SL_SI91X_TCP_IP_FEAT_HTTP_CLIENT
                                              | SL_SI91X_TCP_IP_FEAT_EXTENSION_VALID | SL_SI91X_TCP_IP_FEAT_OTAF
                                              | SL_SI91X_TCP_IP_FEAT_DNS_CLIENT),
                   .custom_feature_bit_map = SL_WIFI_SYSTEM_CUSTOM_FEAT_EXTENSION_VALID,
                   .ext_custom_feature_bit_map =
                     (SL_SI91X_EXT_FEAT_XTAL_CLK | SL_SI91X_EXT_FEAT_UART_SEL_FOR_DEBUG_PRINTS | MEMORY_CONFIG),
                   .bt_feature_bit_map = 0,
                   .ext_tcp_ip_feature_bit_map =
                     (SL_SI91X_EXT_FEAT_HTTP_OTAF_SUPPORT | SL_SI91X_EXT_TCP_IP_SSL_16K_RECORD
                      | SL_SI91X_CONFIG_FEAT_EXTENTION_VALID),
                   .ble_feature_bit_map     = 0,
                   .ble_ext_feature_bit_map = 0,
                   .config_feature_bit_map  = 0 }
};
/**
 * @fn    void app_init(void)
 * @brief \Start the application
 * @param[in]   none  
 @section description
 *      This API initiates application
 */
void app_init(void)
{
  osThreadNew((osThreadFunc_t)application_start, NULL, &thread_attributes);
}
/**
 * @fn          void application_start()
 * @brief       This API gets token
 * @param[in]   none
 *
 * @section description
 *      This API gets token
 */
void application_start(const void *unused)
{
  uint32_t trng_key[TRNG_KEY_SIZE]         = { 0x16157E2B, 0xA6D2AE28, 0x8815F7AB, 0x3C4FCF09 };
  uint32_t trng_num[LENGTH_OF_RANDOM_DATA] = { 0 };
  UNUSED_PARAMETER(unused);
  sl_status_t status = SL_STATUS_OK;
  status             = sl_net_init(SL_NET_WIFI_CLIENT_INTERFACE, &station_init_configuration, NULL, NULL);
  if (status != SL_STATUS_OK) {
    printf("Failed to start Wi-Fi client interface: 0x%lx\r\n", status);
    return;
  }
  printf("\r\nWi-Fi Init success\r\n");
  //! This API checks the Entropy of TRNG i.e source for TRNG
  status = sl_si91x_trng_entropy();

  if (status != SL_STATUS_OK) {
    printf("\r\n TRNG Entropy Failed \r\n");
    return;
  }
  printf("\r\n TRNG Entropy Successful \r\n");

  //! This API Initializes key which needs to be programmed to TRNG hardware engine
  status = sl_si91x_trng_program_key(trng_key, TRNG_KEY_SIZE);

  if (status != SL_STATUS_OK) {
    printf("\r\n TRNG Key Programming Failed \r\n");
    return;
  }
  printf("\r\n TRNG Key Programming Successful \r\n");

  //! Memset the buffer
  memset(trng_num, 0, sizeof(trng_num));

  //! Get Random dwords of desired length
  status = sl_si91x_trng_get_random_num(trng_num, LENGTH_OF_RANDOM_DATA);

  if (status != SL_STATUS_OK) {
    printf("\r\n Random number generation Failed \r\n");
    return;
  }
  printf("\r\n Random number generation Successful \r\n");

  //! Memset the buffer
  memset(attestation_token, 0, LENGTH_OF_TOKEN);

  //! Get Attestation token
  status = sl_si91x_attestation_get_token(attestation_token, LENGTH_OF_TOKEN, trng_num);

  if (status != SL_STATUS_OK) {
    printf("\r\n Attestation token not received \r\n");
    return;
  }
  printf("\r\n Attestation token received \r\n");

  sl_decode();
}

/**
 * @fn          int32 sl_decode()
 * @brief       This API decodes the token received from NWP
 * @param[in]   none
 *
 * @section description
 *      This API decodes the token received from NWP
 */
void sl_decode()
{
  struct q_useful_buf_c signed_cose;
  struct q_useful_buf_c returned_payload;
  struct t_cose_key key_pair;
  struct t_cose_sign1_verify_ctx verify_ctx;

  //! Assigning the token received for verification
  signed_cose.ptr = attestation_token;
  signed_cose.len = LENGTH_OF_TOKEN;

  //! Initialize the verification context.
  t_cose_sign1_verify_init(&verify_ctx, 0);

  t_cose_sign1_set_verification_key(&verify_ctx, key_pair);

  //! Perform the verification
  t_cose_sign1_verify(&verify_ctx,
                      signed_cose,       /* COSE to verify */
                      &returned_payload, /* Payload from signed_cose */
                      NULL);
}