/***************************************************************************/ /**
 * @file
 * @brief Crypto GCM Application
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
#include "cmsis_os2.h"
#include "sl_net.h"
#include "sl_wifi.h"
#include "sl_si91x_gcm.h"
#include "sl_si91x_wrap.h"
#include "sl_si91x_crypto_utility.h"

/******************************************************
*                    Constants
******************************************************/
#define BUFFER_SIZE      32
#define USE_WRAPPED_KEYS 0

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

uint8_t encrypted_buffer[BUFFER_SIZE + SL_SI91X_TAG_SIZE];
uint8_t decrypted_buffer[BUFFER_SIZE + SL_SI91X_TAG_SIZE];

uint8_t key[] = { 0xca, 0x74, 0xfa, 0xb, 0xb5, 0x9a, 0xb5, 0x28, 0x93, 0x70, 0x35, 0xf2, 0x72, 0xd2, 0x16, 0x32 };

//! IV should be 12 Bytes/96 bits
uint8_t nonce[] = { 0x40, 0x92, 0xa5, 0x2c, 0xc9, 0x55, 0xc4, 0xcb, 0xee, 0x97, 0x93, 0x8d };

uint8_t plaintext[] = {
  0x5d, 0x80, 0x2f, 0xb6, 0x5e, 0x25, 0x20, 0x21, 0xb9, 0xc1, 0xec, 0x63, 0x34, 0x1c, 0x65, 0x41
};

uint8_t encryption_tag[SL_SI91X_TAG_SIZE] = { 0 };

uint8_t ciphertext[] = {
  0x14, 0x00, 0x00, 0x0c, 0x79, 0xb2, 0x1a, 0xa0, 0xf7, 0xff, 0x7a, 0x77, 0x34, 0x86, 0xaa, 0x32
};

uint8_t decryption_tag[SL_SI91X_TAG_SIZE] = { 0 };

uint8_t additional_data[] = { 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x16, 0x3, 0x3, 0x0, 0x10 };

uint8_t cmac_key[] = { 0x60, 0x3d, 0xeb, 0x10, 0x15, 0xca, 0x71, 0xbe, 0x2b, 0x73, 0xae, 0xf0, 0x85, 0x7d, 0x77, 0x81,
                       0x1f, 0x35, 0x2c, 0x07, 0x3b, 0x61, 0x08, 0xd7, 0x2d, 0x98, 0x10, 0xa3, 0x09, 0x14, 0xdf, 0xf4 };
uint8_t cmac_msg[] = { 0x6b, 0xc1, 0xbe, 0xe2, 0x2e, 0x40, 0x9f, 0x96, 0xe9, 0x3d, 0x7e, 0x11, 0x73, 0x93,
                       0x17, 0x2a, 0xae, 0x2d, 0x8a, 0x57, 0x1e, 0x03, 0xac, 0x9c, 0x9e, 0xb7, 0x6f, 0xac,
                       0x45, 0xaf, 0x8e, 0x51, 0x30, 0xc8, 0x1c, 0x46, 0xa3, 0x5c, 0xe4, 0x11 };

uint8_t mac_buffer[SL_SI91X_TAG_SIZE] = { 0 };

#if USE_WRAPPED_KEYS
sl_si91x_wrap_config_t wrap_config = { 0 };
uint8_t wrapped_key[SL_SI91X_GCM_KEY_SIZE_256];
uint8_t wrap_iv[] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f };
#endif

/******************************************************
*               Function Declarations
******************************************************/
static void application_start(void *argument);
static sl_status_t gcm_encryption(void);
static sl_status_t gcm_decryption(void);
static sl_status_t cmac_compute(void);

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

  sl_status_t status = sl_net_init(SL_NET_WIFI_CLIENT_INTERFACE, &client_configuration, NULL, NULL);
  if (status != SL_STATUS_OK) {
    printf("Failed to start Wi-Fi client interface: 0x%lx\r\n", status);
    return;
  }
  printf("\r\nWi-Fi Init Success\r\n");

  status = gcm_encryption();
  if (status != SL_STATUS_OK) {
    return;
  }

  status = gcm_decryption();
  if (status != SL_STATUS_OK) {
    return;
  }

  status = cmac_compute();
  if (status != SL_STATUS_OK) {
    return;
  }
}

sl_status_t gcm_encryption(void)
{
  sl_status_t status;

  sl_si91x_gcm_config_t config_gcm;
  memset(&config_gcm, 0, sizeof(sl_si91x_gcm_config_t));
  config_gcm.encrypt_decrypt = SL_SI91X_GCM_ENCRYPT;
  config_gcm.dma_use         = SL_SI91X_GCM_DMA_ENABLE;
  config_gcm.msg             = plaintext;
  config_gcm.msg_length      = sizeof(plaintext);
  config_gcm.nonce           = nonce;
  config_gcm.nonce_length    = sizeof(nonce);
  config_gcm.ad              = additional_data;
  config_gcm.ad_length       = sizeof(additional_data);

#ifdef SLI_SI917B0
  config_gcm.gcm_mode    = SL_SI91X_GCM_MODE;
  size_t key_buffer_size = sizeof(key);

  switch (key_buffer_size) {
    case SL_SI91X_GCM_KEY_SIZE_128:
      config_gcm.key_config.b0.key_size = SL_SI91X_GCM_KEY_SIZE_128;
      break;
    case SL_SI91X_GCM_KEY_SIZE_192:
      config_gcm.key_config.b0.key_size = SL_SI91X_GCM_KEY_SIZE_192;
      break;
    case SL_SI91X_GCM_KEY_SIZE_256:
      config_gcm.key_config.b0.key_size = SL_SI91X_GCM_KEY_SIZE_256;
      break;
    default:
      printf("\r\nInvalid Key Size !\r\n");
      return SL_STATUS_INVALID_PARAMETER;
  }
  config_gcm.gcm_mode               = SL_SI91X_GCM_MODE;
  config_gcm.key_config.b0.key_slot = 0;
#if USE_WRAPPED_KEYS
  wrap_config.key_type     = SL_SI91X_TRANSPARENT_KEY;
  wrap_config.key_size     = key_buffer_size;
  wrap_config.wrap_iv_mode = SL_SI91X_WRAP_IV_CBC_MODE;
  memcpy(wrap_config.key_buffer, key, wrap_config.key_size);
  memcpy(wrap_config.wrap_iv, wrap_iv, SL_SI91X_IV_SIZE);

  status = sl_si91x_wrap(&wrap_config, wrapped_key);
  if (status != SL_STATUS_OK) {
    printf("\r\nWrap failed, Error Code : 0x%lX\r\n", status);
    return status;
  }
  printf("\r\nWrap success\r\n");
  config_gcm.key_config.b0.key_type = SL_SI91X_WRAPPED_KEY;
  //for 128 bits key, wrap key size is 128 bits,
  //for 192 and 256 bits keys, wrap key size is 256 bits
  if (config_gcm.key_config.b0.key_size == SL_SI91X_GCM_KEY_SIZE_128) {
    memcpy(config_gcm.key_config.b0.key_buffer, wrapped_key, SL_SI91X_GCM_KEY_SIZE_128);
  } else {
    memcpy(config_gcm.key_config.b0.key_buffer, wrapped_key, SL_SI91X_GCM_KEY_SIZE_256);
  }
  config_gcm.key_config.b0.wrap_iv_mode = SL_SI91X_WRAP_IV_CBC_MODE;
  memcpy(config_gcm.key_config.b0.wrap_iv, wrap_iv, SL_SI91X_IV_SIZE);

#else
  config_gcm.key_config.b0.key_type = SL_SI91X_TRANSPARENT_KEY;
  //  config_gcm.key_config.b0.wrap_iv_mode = SL_SI91X_WRAP_IV_CBC_MODE;
  memcpy(config_gcm.key_config.b0.key_buffer, key, config_gcm.key_config.b0.key_size);
#endif
#else
  config_gcm.key_config.a0.key_length = sizeof(key);
  memcpy(config_gcm.key_config.a0.key, key, config_gcm.key_config.a0.key_length);
#endif
  status = sl_si91x_gcm(&config_gcm, encrypted_buffer);
  if (status != SL_STATUS_OK) {
    printf("\r\nGCM encryption failed, Error Code : 0x%lX\r\n", status);
    return status;
  }
  memcpy(encryption_tag, encrypted_buffer + sizeof(plaintext), SL_SI91X_TAG_SIZE);
  printf("\r\nGCM encryption success\r\n");
  return status;
}

sl_status_t gcm_decryption(void)
{
  sl_status_t status;

  sl_si91x_gcm_config_t config_gcm;
  memset(&config_gcm, 0, sizeof(sl_si91x_gcm_config_t));
  config_gcm.encrypt_decrypt = SL_SI91X_GCM_DECRYPT;
  config_gcm.dma_use         = SL_SI91X_GCM_DMA_ENABLE;
  config_gcm.msg             = ciphertext;
  config_gcm.msg_length      = sizeof(ciphertext);
  config_gcm.nonce           = nonce;
  config_gcm.nonce_length    = sizeof(nonce);
  config_gcm.ad              = additional_data;
  config_gcm.ad_length       = sizeof(additional_data);
#ifdef SLI_SI917B0
  config_gcm.gcm_mode    = SL_SI91X_GCM_MODE;
  size_t key_buffer_size = sizeof(key);
  switch (key_buffer_size) {
    case SL_SI91X_GCM_KEY_SIZE_128:
      config_gcm.key_config.b0.key_size = SL_SI91X_GCM_KEY_SIZE_128;
      break;
    case SL_SI91X_GCM_KEY_SIZE_192:
      config_gcm.key_config.b0.key_size = SL_SI91X_GCM_KEY_SIZE_192;
      break;
    case SL_SI91X_GCM_KEY_SIZE_256:
      config_gcm.key_config.b0.key_size = SL_SI91X_GCM_KEY_SIZE_256;
      break;
    default:
      printf("\r\nInvalid Key Size !\r\n");
      return SL_STATUS_INVALID_PARAMETER;
  }
  config_gcm.key_config.b0.key_slot = 0;
#if USE_WRAPPED_KEYS
  config_gcm.key_config.b0.key_type = SL_SI91X_WRAPPED_KEY;
  //for 128 bits key, wrap key size is 128 bits,
  //for 192 and 256 bits keys, wrap key size is 256 bits
  if (config_gcm.key_config.b0.key_size == SL_SI91X_GCM_KEY_SIZE_128) {
    memcpy(config_gcm.key_config.b0.key_buffer, wrapped_key, SL_SI91X_GCM_KEY_SIZE_128);
  } else {
    memcpy(config_gcm.key_config.b0.key_buffer, wrapped_key, SL_SI91X_GCM_KEY_SIZE_256);
  }
  config_gcm.key_config.b0.wrap_iv_mode = SL_SI91X_WRAP_IV_CBC_MODE;
  memcpy(config_gcm.key_config.b0.wrap_iv, wrap_iv, SL_SI91X_IV_SIZE);
#else
  config_gcm.key_config.b0.key_type = SL_SI91X_TRANSPARENT_KEY;
  memcpy(config_gcm.key_config.b0.key_buffer, key, config_gcm.key_config.b0.key_size);
#endif
#else
  config_gcm.key_config.a0.key_length = sizeof(key);
#endif
  status = sl_si91x_gcm(&config_gcm, decrypted_buffer);
  if (status != SL_STATUS_OK) {
    printf("\r\nGCM decryption failed, Error Code : 0x%lX\r\n", status);
    return status;
  }
  printf("\r\nGCM Decryption Success\r\n");

  memcpy(decryption_tag, decrypted_buffer + sizeof(ciphertext), SL_SI91X_TAG_SIZE);
  if (memcmp(decryption_tag, encryption_tag, SL_SI91X_TAG_SIZE) != 0) {
    printf("\r\nGCM Authentication Failed !!\r\n");
    status = SL_STATUS_INVALID_SIGNATURE;
  }

  return status;
}

sl_status_t cmac_compute(void)
{
  sl_status_t status = SL_STATUS_FAIL;
#ifdef SLI_SI917B0
  sl_si91x_gcm_config_t config_cmac;
  memset(&config_cmac, 0, sizeof(sl_si91x_gcm_config_t));
  config_cmac.gcm_mode     = SL_SI91X_CMAC_MODE;
  config_cmac.dma_use      = SL_SI91X_GCM_DMA_ENABLE;
  config_cmac.msg          = cmac_msg;
  config_cmac.msg_length   = sizeof(cmac_msg);
  config_cmac.nonce        = NULL;
  config_cmac.nonce_length = 0;
  config_cmac.ad           = NULL;
  config_cmac.ad_length    = 0;
  size_t key_buffer_size   = sizeof(cmac_key);
  switch (key_buffer_size) {
    case SL_SI91X_GCM_KEY_SIZE_128:
      config_cmac.key_config.b0.key_size = SL_SI91X_GCM_KEY_SIZE_128;
      break;
    case SL_SI91X_GCM_KEY_SIZE_192:
      config_cmac.key_config.b0.key_size = SL_SI91X_GCM_KEY_SIZE_192;
      break;
    case SL_SI91X_GCM_KEY_SIZE_256:
      config_cmac.key_config.b0.key_size = SL_SI91X_GCM_KEY_SIZE_256;
      break;
    default:
      printf("\r\nInvalid Key Size !\r\n");
      return SL_STATUS_INVALID_PARAMETER;
  }
  config_cmac.key_config.b0.key_slot = 0;
#if USE_WRAPPED_KEYS
  wrap_config.key_type     = SL_SI91X_TRANSPARENT_KEY;
  wrap_config.key_size     = key_buffer_size;
  wrap_config.wrap_iv_mode = SL_SI91X_WRAP_IV_CBC_MODE;
  memset(wrapped_key, 0, key_buffer_size);
  memcpy(wrap_config.key_buffer, cmac_key, wrap_config.key_size);
  memcpy(wrap_config.wrap_iv, wrap_iv, SL_SI91X_IV_SIZE);

  status = sl_si91x_wrap(&wrap_config, wrapped_key);
  if (status != SL_STATUS_OK) {
    printf("\r\nWrap failed, Error Code : 0x%lX\r\n", status);
    return status;
  }
  printf("\r\nWrap success\r\n");
  config_cmac.key_config.b0.key_type = SL_SI91X_WRAPPED_KEY;
  //for 128 bits key, wrap key size is 128 bits,
  //for 192 and 256 bits keys, wrap key size is 256 bits
  if (config_cmac.key_config.b0.key_size == SL_SI91X_GCM_KEY_SIZE_128) {
    memcpy(config_cmac.key_config.b0.key_buffer, wrapped_key, SL_SI91X_GCM_KEY_SIZE_128);
  } else {
    memcpy(config_cmac.key_config.b0.key_buffer, wrapped_key, SL_SI91X_GCM_KEY_SIZE_256);
  }
  config_cmac.key_config.b0.wrap_iv_mode = SL_SI91X_WRAP_IV_CBC_MODE;
  memcpy(config_cmac.key_config.b0.wrap_iv, wrap_iv, SL_SI91X_IV_SIZE);

#else
  config_cmac.key_config.b0.key_type = SL_SI91X_TRANSPARENT_KEY;
  memcpy(config_cmac.key_config.b0.key_buffer, cmac_key, config_cmac.key_config.b0.key_size);
#endif
  // Expected CMAC MAC: aa f3 d8 f1 de 56 40 c2 32 f5 b1 69 b9 c9 11 e6
  status = sl_si91x_gcm(&config_cmac, mac_buffer);
  if (status != SL_STATUS_OK) {
    printf("\r\nCMAC Compute failed, Error Code : 0x%lX\r\n", status);
    return status;
  }
  printf("\r\nCMAC Compute Success\r\n");

#else
  status = SL_STATUS_NOT_SUPPORTED;
#endif
  return status;
}
