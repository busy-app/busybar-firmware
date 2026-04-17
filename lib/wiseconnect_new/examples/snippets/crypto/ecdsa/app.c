/***************************************************************************/ /**
 * @file
 * @brief Crypto ECDSA Application
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

#include <string.h>
#include "cmsis_os2.h"
#include "sl_net.h"
#include "sl_wifi.h"
#include "sl_si91x_crypto.h"
#include "sl_si91x_ecdsa.h"
#include "sl_si91x_wrap.h"

/******************************************************
 *                    Constants
 ******************************************************/

#define USE_WRAPPED_KEYS             0
#define INPUT_MSG_SIZE               32
#define SL_SI91X_PRIVATE_KEY_SIZE    32
#define SL_SI91X_PUBLIC_KEY_SIZE     65
#define SL_SI91X_ECDSA_MAX_SIGN_SIZE 72

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
                   .ext_custom_feature_bit_map = (MEMORY_CONFIG
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

// Input message
static const unsigned char input_data[INPUT_MSG_SIZE] = { 0x24, 0x8d, 0x6a, 0x61, 0xd2, 0x06, 0x38, 0xb8,
                                                          0xe5, 0xc0, 0x26, 0x93, 0x0c, 0x3e, 0x60, 0x39,
                                                          0xa3, 0x3c, 0xe4, 0x59, 0x64, 0xff, 0x21, 0x67,
                                                          0xf6, 0xec, 0xed, 0xd4, 0x19, 0xdb, 0x06, 0xc1 };

static const unsigned char private_key[SL_SI91X_PRIVATE_KEY_SIZE] = { 0x41, 0x9c, 0x9c, 0x80, 0x33, 0x6c, 0x40, 0x1f,
                                                                      0xd2, 0x06, 0x49, 0x59, 0x8b, 0xb6, 0x5f, 0xb3,
                                                                      0xd8, 0xd8, 0xef, 0xd5, 0xeb, 0x4a, 0xe1, 0xe8,
                                                                      0x8a, 0x63, 0x36, 0x81, 0xcb, 0x0a, 0x21, 0x07 };

static const unsigned char public_key[SL_SI91X_PUBLIC_KEY_SIZE] = {
  0x04, 0xb6, 0xcd, 0x40, 0x84, 0x9a, 0xf6, 0xc4, 0xc2, 0x2b, 0x57, 0x99, 0x86, 0xa7, 0x7d, 0xfa, 0x19,
  0x61, 0xaa, 0xe2, 0x6e, 0x60, 0xe6, 0x83, 0x82, 0x11, 0xeb, 0xe5, 0xd1, 0x40, 0x79, 0x22, 0x25, 0xe4,
  0x12, 0x40, 0xfe, 0x30, 0xec, 0x63, 0x88, 0xab, 0x35, 0xaf, 0xb6, 0x34, 0xd8, 0x76, 0x03, 0xef, 0x81,
  0xb8, 0x11, 0x7d, 0x90, 0x43, 0xf6, 0x7e, 0x0a, 0x73, 0x01, 0xbd, 0x48, 0x5e, 0x7f
};

uint8_t iv[SL_SI91X_IV_SIZE] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                                 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F };

sl_si91x_wrap_config_t wrap_config = { 0 };

// Buffers to store responses
uint8_t wrapped_key[SL_SI91X_PRIVATE_KEY_SIZE]   = { 0 };
uint8_t public_key_gen[SL_SI91X_PUBLIC_KEY_SIZE] = { 0 };
uint8_t signature[SL_SI91X_ECDSA_MAX_SIGN_SIZE]  = { 0 };
uint8_t signature_length                         = 0;
uint8_t *verify_output                           = NULL;

/******************************************************
 *               Function Declarations
 ******************************************************/
static void application_start(void *argument);
static sl_status_t ecdsa_generate_key_pair(void);
static sl_status_t ecdsa_sign_message(void);
static sl_status_t ecdsa_verify_message(void);
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

  status = ecdsa_generate_key_pair();
  if (status != SL_STATUS_OK) {
    return;
  }

  status = ecdsa_sign_message();
  if (status != SL_STATUS_OK) {
    return;
  }

  status = ecdsa_verify_message();
  if (status != SL_STATUS_OK) {
    return;
  }
}

sl_status_t ecdsa_generate_key_pair(void)
{
  sl_status_t status;
  sl_si91x_ecdsa_config_t config = { 0 };

  config.ecdsa_operation    = SL_SI91X_GENERATE_ECC_KEY_PAIR;
  config.curve_id           = SL_SI91X_ECC_SECP256R1;
  config.sha_mode           = 0;
  config.msg                = NULL;
  config.msg_length         = 0;
  config.private_key        = NULL;
  config.private_key_length = 0;
  config.public_key         = NULL;
  if (config.curve_id == SL_SI91X_ECC_SECP224R1) {
    config.public_key_length = 57;
  } else if (config.curve_id == SL_SI91X_ECC_SECP256R1) {
    config.public_key_length = 65;
  }
  config.signature              = NULL;
  config.signature_length       = 0;
  config.key_config.b0.key_type = 0;
  config.key_config.b0.key_size = 0;
  config.key_config.b0.key_slot = 0;
  config.key_config.b0.reserved = 0;

  status = sl_si91x_ecdsa(&config, public_key_gen);
  if (status != SL_STATUS_OK) {
    printf("\r\nECC key pair generation failed, Error Code : 0x%lX\r\n", status);
    return status;
  }
  printf("\r\nECC key pair generation success\r\n");

  return status;
}

sl_status_t ecdsa_sign_message(void)
{
  sl_status_t status;
  sl_si91x_ecdsa_config_t config = { 0 };

  config.ecdsa_operation        = SL_SI91X_ECDSA_GENERATE_SIGN;
  config.curve_id               = SL_SI91X_ECC_SECP256R1;
  config.sha_mode               = SL_SI91X_ECDSA_SHA_256;
  config.msg                    = input_data;
  config.msg_length             = INPUT_MSG_SIZE;
  config.private_key            = private_key;
  config.private_key_length     = SL_SI91X_PRIVATE_KEY_SIZE;
  config.public_key             = NULL;
  config.public_key_length      = 0;
  config.signature_length       = signature_length;
  config.key_config.b0.key_type = SL_SI91X_TRANSPARENT_KEY;
  config.key_config.b0.key_size = 0;
  config.key_config.b0.key_slot = 0;
  config.key_config.b0.reserved = 0;

#if USE_WRAPPED_KEYS
  wrap_config.key_type     = SL_SI91X_TRANSPARENT_KEY;
  wrap_config.key_size     = SL_SI91X_PRIVATE_KEY_SIZE;
  wrap_config.wrap_iv_mode = SL_SI91X_WRAP_IV_ECB_MODE;
  memcpy(wrap_config.wrap_iv, iv, SL_SI91X_IV_SIZE);
  memcpy(wrap_config.key_buffer, private_key, wrap_config.key_size);

  status = sl_si91x_wrap(&wrap_config, wrapped_key);
  if (status != SL_STATUS_OK) {
    printf("\r\nWrap failed, Error Code : 0x%lX\r\n", status);
    return status;
  }
  printf("\r\nWrap success\r\n");

  config.key_config.b0.key_type = SL_SI91X_WRAPPED_KEY;
  config.private_key            = wrapped_key;
  config.private_key_length     = 32;

#endif

  status = sl_si91x_ecdsa(&config, signature);

  if (status != SL_STATUS_OK) {
    printf("\r\nECDSA signature generation failed, Error Code : 0x%lX\r\n", status);
    return status;
  }
  printf("\r\nECDSA signature generation success\r\n");
  signature_length = config.signature_length;

  return status;
}

sl_status_t ecdsa_verify_message(void)
{
  sl_status_t status;
  sl_si91x_ecdsa_config_t config = { 0 };

  config.ecdsa_operation        = SL_SI91X_ECDSA_VERIFY_SIGN;
  config.curve_id               = SL_SI91X_ECC_SECP256R1;
  config.sha_mode               = SL_SI91X_ECDSA_SHA_256;
  config.msg                    = input_data;
  config.msg_length             = INPUT_MSG_SIZE;
  config.private_key            = NULL;
  config.private_key_length     = 0;
  config.public_key             = public_key;
  config.public_key_length      = SL_SI91X_PUBLIC_KEY_SIZE;
  config.signature              = signature;
  config.signature_length       = signature_length;
  config.key_config.b0.key_type = SL_SI91X_TRANSPARENT_KEY;
  config.key_config.b0.key_size = 0;
  config.key_config.b0.key_slot = 0;
  config.key_config.b0.reserved = 0;

  status = sl_si91x_ecdsa(&config, verify_output);
  if (status != SL_STATUS_OK || *verify_output != 1) {
    printf("\r\nECDSA signature verification failed, Error Code : 0x%lX\r\n", status);
    return status;
  }
  printf("\r\nECDSA signature verification success\r\n");

  return status;
}
