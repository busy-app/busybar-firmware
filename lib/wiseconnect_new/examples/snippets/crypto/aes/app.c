/***************************************************************************/ /**
 * @file
 * @brief Crypto AES Application
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
#include "sl_si91x_aes.h"
#include "sl_si91x_wrap.h"
#include "sl_si91x_crypto_utility.h"

/******************************************************
 *                    Constants
 ******************************************************/
#define BUFFER_SIZE      1024 // Update the buffer size based on the message size.
#define USE_WRAPPED_KEYS 0
#define USE_MULTIPART    0
#define PKCS_7_PADDING   0

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

uint8_t encrypted_buffer[BUFFER_SIZE];
uint8_t decrypted_buffer[BUFFER_SIZE];

uint8_t msg[1024] = { 0x6b, 0xc1, 0xbe, 0xe2, 0x2e, 0x40, 0x9f, 0x96, 0xe9, 0x3d, 0x7e, 0x11, 0x73, 0x93, 0x17, 0x2a };

uint8_t key[SL_SI91X_AES_KEY_SIZE_256] = { 0x60, 0x3d, 0xeb, 0x10, 0x15, 0xca, 0x71, 0xbe, 0x2b, 0x73, 0xae,
                                           0xf0, 0x85, 0x7d, 0x77, 0x81, 0x1f, 0x35, 0x2c, 0x07, 0x3b, 0x61,
                                           0x08, 0xd7, 0x2d, 0x98, 0x10, 0xa3, 0x09, 0x14, 0xdf, 0xf4 };

uint8_t wrapped_key[SL_SI91X_AES_KEY_SIZE_256];

// IV used for SL_SI91X_AES_CBC and SL_SI91X_AES_CTR modes.
uint8_t iv[SL_SI91X_IV_SIZE] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                                 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F };

sl_si91x_wrap_config_t wrap_config = { 0 };

int sent_data = 0;

#if PKCS_7_PADDING
size_t padded_data_length = 0;
uint8_t decrypted_unpadded_data_buffer[BUFFER_SIZE];
#endif

/******************************************************
 *               Function Declarations
 ******************************************************/
static void application_start(void *argument);
#if USE_MULTIPART
static sl_status_t aes_multipart_encryption(void);
static sl_status_t aes_multipart_decryption(void);
#else
static sl_status_t aes_encryption(void);
static sl_status_t aes_decryption(void);
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

  sl_status_t status = sl_net_init(SL_NET_WIFI_CLIENT_INTERFACE, &client_configuration, NULL, NULL);
  if (status != SL_STATUS_OK) {
    printf("Failed to start Wi-Fi client interface: 0x%lx\r\n", status);
    return;
  }
  printf("\r\nWi-Fi Init Success\r\n");

#if USE_MULTIPART
  status = aes_multipart_encryption();
  if (status != SL_STATUS_OK) {
    return;
  }

  status = aes_multipart_decryption();
  if (status != SL_STATUS_OK) {
    return;
  }
#else
  status = aes_encryption();
  if (status != SL_STATUS_OK) {
    return;
  }

  status = aes_decryption();
  if (status != SL_STATUS_OK) {
    return;
  }
#endif
}

#if USE_MULTIPART
sl_status_t aes_multipart_encryption(void)
{
  sl_status_t status;
  sl_si91x_aes_config_t config;
  memset(&config, 0, sizeof(sl_si91x_aes_config_t));

  uint8_t aes_flags = 0;

  config.aes_mode                   = SL_SI91X_AES_CTR;
  config.encrypt_decrypt            = SL_SI91X_AES_ENCRYPT;
  config.msg                        = msg;
  config.msg_length                 = sizeof(msg);
  config.iv                         = iv;
  config.key_config.b0.key_size     = SL_SI91X_AES_KEY_SIZE_256;
  config.key_config.b0.key_slot     = 0;
  config.key_config.b0.wrap_iv_mode = SL_SI91X_WRAP_IV_ECB_MODE;
  config.key_config.b0.key_type     = SL_SI91X_TRANSPARENT_KEY;
  memcpy(config.key_config.b0.key_buffer, &key, config.key_config.b0.key_size);

#if USE_WRAPPED_KEYS
  wrap_config.key_type     = SL_SI91X_TRANSPARENT_KEY;
  wrap_config.key_size     = SL_SI91X_AES_KEY_SIZE_256;
  wrap_config.wrap_iv_mode = SL_SI91X_WRAP_IV_ECB_MODE;
  memcpy(wrap_config.key_buffer, key, wrap_config.key_size);

  status = sl_si91x_wrap(&wrap_config, wrapped_key);
  if (status != SL_STATUS_OK) {
    printf("\r\nWrap failed, Error Code : 0x%lX\r\n", status);
    return status;
  }
  printf("\r\nWrap success\r\n");

  config.key_config.b0.key_type = SL_SI91X_WRAPPED_KEY;
  //for 128 bits key, wrap key size is 128 bits,
  //for 192 and 256 bits keys, wrap key size is 256 bits
  if (config.key_config.b0.key_size == SL_SI91X_AES_KEY_SIZE_128) {
    memcpy(config.key_config.b0.key_buffer, &wrapped_key, SL_SI91X_AES_KEY_SIZE_128);
  } else if (config.key_config.b0.key_size == SL_SI91X_AES_KEY_SIZE_192
             || config.key_config.b0.key_size == SL_SI91X_AES_KEY_SIZE_256) {
    memcpy(config.key_config.b0.key_buffer, &wrapped_key, SL_SI91X_AES_KEY_SIZE_256);
  } else {
    printf("\r\n Invalid key length \r\n");
    return SL_STATUS_INVALID_PARAMETER;
  }
#endif

  uint32_t total_msg_length = config.msg_length;
  int chunk_len             = 0;
  int chunk_sizes[]         = { 128,
                                256,
                                128,
                                384,
                                128 }; // Make sure that sum of the chunk sizes are equal to total message length.
  int num_chunks            = sizeof(chunk_sizes) / sizeof(chunk_sizes[0]);
  for (int i = 0; i < num_chunks; i++) {
    chunk_len = chunk_sizes[i];
    if (i == 0) {
      aes_flags = SL_SI91X_AES_FIRST_CHUNK; // FIRST_CHUNK
    } else if (i == num_chunks - 1) {
      aes_flags = SL_SI91X_AES_LAST_CHUNK; // LAST_CHUNK
    } else {
      aes_flags = SL_SI91X_AES_MIDDLE_CHUNK; // MIDDLE_CHUNK
    }
    // Call AES multipart encryption function
    if (chunk_len % SL_SI91X_AES_BLOCK_SIZE == 0) {
      status = sl_si91x_aes_multipart(&config, chunk_len, aes_flags, encrypted_buffer + sent_data);
      if (status != SL_STATUS_OK) {
        printf("\r\nAES encryption failed, Error Code : 0x%lX\r\n", status);
        return status;
      }
      printf("\r\nAES encryption success\r\n");
      sent_data += chunk_len;
      printf("Encrypted %d plain text bytes out of %ld bytes\n", sent_data, total_msg_length);
      // Update pointer for the next chunk
      config.msg += chunk_len;
    } else {
      printf("Error: Chunk size %d is not 16-byte aligned. Skipping.\n", chunk_len);
    }
  }
#if PKCS_7_PADDING
  uint8_t *padded_data = NULL;
  if (chunk_len % SL_SI91X_AES_BLOCK_SIZE != 0) {
    padded_data = pkcs7_padding((const uint8_t *)config.msg,
                                chunk_sizes[num_chunks - 1],
                                SL_SI91X_AES_BLOCK_SIZE,
                                &padded_data_length);
    if (padded_data && padded_data_length % SL_SI91X_AES_BLOCK_SIZE == 0) {
      config.msg = padded_data;
      sl_si91x_aes_multipart(&config, padded_data_length, SL_SI91X_AES_LAST_CHUNK, encrypted_buffer + sent_data);
      sent_data += padded_data_length;
      printf("Final padded chunk sent, total: %d bytes\n", sent_data);
    } else {
      printf("Error: Padded data length %d is not 16-byte aligned. Skipping.\n", padded_data_length);
    }
  }

#if PKCS_7_PADDING
  SL_CLEANUP_MALLOC(padded_data);
#endif
#endif

  for (int i = 0; i < sent_data; i++) {
    if (i % 16 == 0) {
      printf("\n %d: ", i);
    }
    printf("0x%.2x ", encrypted_buffer[i]);
  }
  printf("\n");

  return status;
}
#else
sl_status_t aes_encryption(void)
{
  sl_status_t status;
  sl_si91x_aes_config_t config;
  memset(&config, 0, sizeof(sl_si91x_aes_config_t));

  config.aes_mode                   = SL_SI91X_AES_CTR;
  config.encrypt_decrypt            = SL_SI91X_AES_ENCRYPT;
  config.msg                        = msg;
  config.msg_length                 = sizeof(msg);
  config.iv                         = iv;
  config.key_config.b0.key_size     = SL_SI91X_AES_KEY_SIZE_256;
  config.key_config.b0.key_slot     = 0;
  config.key_config.b0.wrap_iv_mode = SL_SI91X_WRAP_IV_ECB_MODE;
  config.key_config.b0.key_type     = SL_SI91X_TRANSPARENT_KEY;
  memcpy(config.key_config.b0.key_buffer, &key, config.key_config.b0.key_size);

#if USE_WRAPPED_KEYS
  wrap_config.key_type     = SL_SI91X_TRANSPARENT_KEY;
  wrap_config.key_size     = SL_SI91X_AES_KEY_SIZE_256;
  wrap_config.wrap_iv_mode = SL_SI91X_WRAP_IV_ECB_MODE;
  memcpy(wrap_config.key_buffer, key, wrap_config.key_size);

  status = sl_si91x_wrap(&wrap_config, wrapped_key);
  if (status != SL_STATUS_OK) {
    printf("\r\nWrap failed, Error Code : 0x%lX\r\n", status);
    return status;
  }
  printf("\r\nWrap success\r\n");

  config.key_config.b0.key_type = SL_SI91X_WRAPPED_KEY;
  //for 128 bits key, wrap key size is 128 bits,
  //for 192 and 256 bits keys, wrap key size is 256 bits
  if (config.key_config.b0.key_size == SL_SI91X_AES_KEY_SIZE_128) {
    memcpy(config.key_config.b0.key_buffer, &wrapped_key, SL_SI91X_AES_KEY_SIZE_128);
  } else if (config.key_config.b0.key_size == SL_SI91X_AES_KEY_SIZE_192
             || config.key_config.b0.key_size == SL_SI91X_AES_KEY_SIZE_256) {
    memcpy(config.key_config.b0.key_buffer, &wrapped_key, SL_SI91X_AES_KEY_SIZE_256);
  } else {
    printf("\r\n Invalid key length \r\n");
    return SL_STATUS_INVALID_PARAMETER;
  }
#endif

#if PKCS_7_PADDING
  uint8_t *padded_data =
    pkcs7_padding((const uint8_t *)config.msg, config.msg_length, SL_SI91X_AES_BLOCK_SIZE, &padded_data_length);
  SL_VERIFY_POINTER_OR_RETURN(padded_data, SL_STATUS_NULL_POINTER);

  config.msg        = padded_data;
  config.msg_length = padded_data_length;
#endif

  status = sl_si91x_aes(&config, encrypted_buffer);
  if (status != SL_STATUS_OK) {
    printf("\r\nAES encryption failed, Error Code : 0x%lX\r\n", status);
    return status;
  }
  printf("\r\nAES encryption success\r\n");

  for (unsigned int i = 0; i < sizeof(msg); i++) {
    if (i % 16 == 0) {
      printf("\n %d: ", i);
    }
    printf("0x%.2x ", encrypted_buffer[i]);
  }
  printf("\n");

#if PKCS_7_PADDING
  SL_CLEANUP_MALLOC(padded_data);
#endif

  return status;
}
#endif

#if USE_MULTIPART
sl_status_t aes_multipart_decryption(void)
{
  sl_status_t status;
  sl_si91x_aes_config_t config;
  memset(&config, 0, sizeof(sl_si91x_aes_config_t));

  uint8_t aes_flags = 0;

  config.aes_mode                   = SL_SI91X_AES_CTR;
  config.encrypt_decrypt            = SL_SI91X_AES_DECRYPT;
  config.msg                        = encrypted_buffer;
  config.msg_length                 = sizeof(msg);
  config.iv                         = iv;
  config.key_config.b0.key_size     = SL_SI91X_AES_KEY_SIZE_256;
  config.key_config.b0.key_slot     = 0;
  config.key_config.b0.wrap_iv_mode = SL_SI91X_WRAP_IV_ECB_MODE;
  config.key_config.b0.key_type     = SL_SI91X_TRANSPARENT_KEY;
  memcpy(config.key_config.b0.key_buffer, &key, config.key_config.b0.key_size);

#if USE_WRAPPED_KEYS
  config.key_config.b0.key_type = SL_SI91X_WRAPPED_KEY;
  //for 128 bits key, wrap key size is 128 bits,
  //for 192 and 256 bits keys, wrap key size is 256 bits
  if (config.key_config.b0.key_size == SL_SI91X_AES_KEY_SIZE_128) {
    memcpy(config.key_config.b0.key_buffer, &wrapped_key, SL_SI91X_AES_KEY_SIZE_128);
  } else if (config.key_config.b0.key_size == SL_SI91X_AES_KEY_SIZE_192
             || config.key_config.b0.key_size == SL_SI91X_AES_KEY_SIZE_256) {
    memcpy(config.key_config.b0.key_buffer, &wrapped_key, SL_SI91X_AES_KEY_SIZE_256);
  } else {
    printf("\r\n Invalid key length \r\n");
    return SL_STATUS_INVALID_PARAMETER;
  }
#endif

#if PKCS_7_PADDING
  config.msg_length = sent_data;
#endif

  uint32_t total_msg_length = config.msg_length;
  int received_data         = 0;
  int chunk_sizes[]         = { 128, 256, 128, 384, 128 };
  int num_chunks            = sizeof(chunk_sizes) / sizeof(chunk_sizes[0]);

  for (int i = 0; i < num_chunks; i++) {
    int chunk_len = chunk_sizes[i];
    if (i == 0) {
      aes_flags = SL_SI91X_AES_FIRST_CHUNK; // FIRST_CHUNK
    } else if (i == num_chunks - 1) {
      aes_flags = SL_SI91X_AES_LAST_CHUNK; // LAST_CHUNK
    } else {
      aes_flags = SL_SI91X_AES_MIDDLE_CHUNK; // MIDDLE_CHUNK
    }
    // Call AES multipart decryption function
    if (chunk_len % SL_SI91X_AES_BLOCK_SIZE == 0) {
      status = sl_si91x_aes_multipart(&config, chunk_len, aes_flags, decrypted_buffer + received_data);

      if (status != SL_STATUS_OK) {
        printf("\r\nAES Multipart decryption failed, Error Code : 0x%lX\r\n", status);
        return status;
      }
      printf("\r\nAES Multipart decryption success\r\n");
      received_data += chunk_len;
      // Update pointer for the next chunk
      config.msg += chunk_len;
      printf("Decrypted %d cipher text bytes out of %ld bytes\n", received_data, total_msg_length);
    } else {
      printf("Error: Chunk size %d is not 16-byte aligned. Skipping.\n", chunk_len);
    }
  }
#if PKCS_7_PADDING
  int32_t unpadded_data_length = pkcs7_unpad(decrypted_buffer, received_data);
  if (unpadded_data_length < 0) {
    printf("Error: Padding removal failed.\n");
    return SL_STATUS_FAIL;
  }
  memset(decrypted_unpadded_data_buffer, 0, sent_data);
  memcpy(decrypted_unpadded_data_buffer, decrypted_buffer, unpadded_data_length);
  received_data = unpadded_data_length;
  printf("Decryption completed. Unpadded data length: %ld bytes\n", unpadded_data_length);
#endif

  for (int i = 0; i < received_data; i++) {
    if (i % 16 == 0) {
      printf("\n %d: ", i);
    }
    printf("0x%.2x ", decrypted_buffer[i]);
  }
  printf("\n");

  return status;
}

#else
sl_status_t aes_decryption(void)
{
  sl_status_t status;
  sl_si91x_aes_config_t config;
  memset(&config, 0, sizeof(sl_si91x_aes_config_t));

  config.aes_mode                   = SL_SI91X_AES_CTR;
  config.encrypt_decrypt            = SL_SI91X_AES_DECRYPT;
  config.msg                        = encrypted_buffer;
  config.msg_length                 = sizeof(msg);
  config.iv                         = iv;
  config.key_config.b0.key_size     = SL_SI91X_AES_KEY_SIZE_256;
  config.key_config.b0.key_slot     = 0;
  config.key_config.b0.wrap_iv_mode = SL_SI91X_WRAP_IV_ECB_MODE;
  config.key_config.b0.key_type     = SL_SI91X_TRANSPARENT_KEY;
  memcpy(config.key_config.b0.key_buffer, &key, config.key_config.b0.key_size);

#if USE_WRAPPED_KEYS
  config.key_config.b0.key_type = SL_SI91X_WRAPPED_KEY;
  //for 128 bits key, wrap key size is 128 bits,
  //for 192 and 256 bits keys, wrap key size is 256 bits
  if (config.key_config.b0.key_size == SL_SI91X_AES_KEY_SIZE_128) {
    memcpy(config.key_config.b0.key_buffer, &wrapped_key, SL_SI91X_AES_KEY_SIZE_128);
  } else if (config.key_config.b0.key_size == SL_SI91X_AES_KEY_SIZE_192
             || config.key_config.b0.key_size == SL_SI91X_AES_KEY_SIZE_256) {
    memcpy(config.key_config.b0.key_buffer, &wrapped_key, SL_SI91X_AES_KEY_SIZE_256);
  } else {
    printf("\r\n Invalid key length \r\n");
    return SL_STATUS_INVALID_PARAMETER;
  }
#endif

#if PKCS_7_PADDING
  config.msg_length = padded_data_length;
#endif

  status = sl_si91x_aes(&config, decrypted_buffer);
  if (status != SL_STATUS_OK) {
    printf("\r\nAES decryption failed, Error Code : 0x%lX\r\n", status);
    return status;
  }
  printf("\r\nAES decryption success\r\n");

  for (unsigned int i = 0; i < sizeof(msg); i++) {
    if (i % 16 == 0) {
      printf("\n %d: ", i);
    }
    printf("0x%.2x ", decrypted_buffer[i]);
  }
  printf("\n");

#if PKCS_7_PADDING
  int32_t unpadded_data_length = pkcs7_unpad(decrypted_buffer, padded_data_length);
  if (unpadded_data_length < 0) {
    return SL_STATUS_FAIL;
  }
  memset(decrypted_unpadded_data_buffer, 0, BUFFER_SIZE);
  memcpy(decrypted_unpadded_data_buffer, decrypted_buffer, unpadded_data_length);
#endif

  return status;
}
#endif
