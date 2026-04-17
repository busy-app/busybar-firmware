/***************************************************************************/ /**
 * @file
 * @brief SL SI91X AES source file
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

#include "sl_si91x_aes.h"
#include "sl_si91x_crypto.h"
#include "sl_status.h"
#include "sl_constants.h"
#include "sl_si91x_protocol_types.h"
#if defined(SLI_MULTITHREAD_DEVICE_SI91X)
#include "sl_si91x_crypto_thread.h"
#endif
#include "sl_si91x_driver.h"
#include <string.h>
#include "sli_wifi_utility.h"
#define SLI_SI91X_MAX_DATA_SIZE_IN_BYTES_FOR_AES 1408

#ifndef SL_SI91X_SIDE_BAND_CRYPTO
static sl_status_t sli_si91x_aes_pending(const sl_si91x_aes_config_t *config,
                                         uint16_t chunk_length,
                                         uint8_t aes_flags,
                                         uint8_t *output)
{
  sl_status_t status                    = SL_STATUS_FAIL;
  sl_wifi_buffer_t *buffer              = NULL;
  const sl_wifi_system_packet_t *packet = NULL;
  sli_si91x_aes_request_t *request      = (sli_si91x_aes_request_t *)malloc(sizeof(sli_si91x_aes_request_t));
  SL_VERIFY_POINTER_OR_RETURN(request, SL_STATUS_ALLOCATION_FAILED);

  memset(request, 0, sizeof(sli_si91x_aes_request_t));

  request->algorithm_type       = AES;
  request->algorithm_sub_type   = (uint8_t)config->aes_mode;
  request->aes_flags            = aes_flags;
  request->total_msg_length     = config->msg_length;
  request->current_chunk_length = chunk_length;
  request->encrypt_decryption   = config->encrypt_decrypt;
  if (config->iv != NULL) {
    memcpy(request->IV, config->iv, SL_SI91X_IV_SIZE);
  }
  memcpy(request->msg, config->msg, chunk_length);

#if defined(SLI_SI917B0)
  request->key_info.key_type                         = config->key_config.b0.key_type;
  request->key_info.key_detail.key_size              = config->key_config.b0.key_size;
  request->key_info.key_detail.key_spec.key_slot     = config->key_config.b0.key_slot;
  request->key_info.key_detail.key_spec.wrap_iv_mode = config->key_config.b0.wrap_iv_mode;
  memcpy(request->key_info.key_detail.key_spec.wrap_iv, config->key_config.b0.wrap_iv, SL_SI91X_IV_SIZE);
  memcpy(request->key_info.key_detail.key_spec.key_buffer, config->key_config.b0.key_buffer, SL_SI91X_KEY_BUFFER_SIZE);
#else
  request->key_length = config->key_config.a0.key_length;
  memcpy(request->key, config->key_config.a0.key, request->key_length);
#endif

  status = sli_si91x_driver_send_command(
    SLI_COMMON_REQ_ENCRYPT_CRYPTO,
    SLI_WIFI_COMMON_CMD,
    request,
    (sizeof(sli_si91x_aes_request_t) - SLI_SI91X_MAX_DATA_SIZE_IN_BYTES_FOR_AES + chunk_length),
    SLI_WIFI_WAIT_FOR_RESPONSE(SLI_COMMON_RSP_ENCRYPT_CRYPTO_WAIT_TIME),
    NULL,
    &buffer);
  if (status != SL_STATUS_OK) {
    free(request);
    if (buffer != NULL)
      sli_si91x_host_free_buffer(buffer);
  }
  VERIFY_STATUS_AND_RETURN(status);
  packet = (sl_wifi_system_packet_t *)sli_wifi_host_get_buffer_data(buffer, 0, NULL);
  memcpy(output, packet->data, packet->length);
  sli_si91x_host_free_buffer(buffer);
  free(request);
  return status;
}

#else
static sl_status_t sli_si91x_aes_side_band(const sl_si91x_aes_config_t *config, uint8_t *output)
{

  sl_status_t status               = SL_STATUS_FAIL;
  sli_si91x_aes_request_t *request = (sli_si91x_aes_request_t *)malloc(sizeof(sli_si91x_aes_request_t));
  SL_VERIFY_POINTER_OR_RETURN(request, SL_STATUS_ALLOCATION_FAILED);

  memset(request, 0, sizeof(sli_si91x_aes_request_t));

  request->algorithm_type     = AES;
  request->algorithm_sub_type = config->aes_mode;
  request->total_msg_length   = config->msg_length;
  request->encrypt_decryption = config->encrypt_decrypt;
  if (config->iv != NULL) {
    request->IV = (uint8_t *)config->iv;
  }
  request->msg    = (uint8_t *)config->msg;
  request->output = output;

  request->key_info.key_type                         = config->key_config.b0.key_type;
  request->key_info.key_detail.key_size              = config->key_config.b0.key_size;
  request->key_info.key_detail.key_spec.key_slot     = config->key_config.b0.key_slot;
  request->key_info.key_detail.key_spec.wrap_iv_mode = config->key_config.b0.wrap_iv_mode;
  memcpy(request->key_info.key_detail.key_spec.wrap_iv, config->key_config.b0.wrap_iv, SL_SI91X_IV_SIZE);
  memcpy(request->key_info.key_detail.key_spec.key_buffer, config->key_config.b0.key_buffer, SL_SI91X_KEY_BUFFER_SIZE);

  status = sl_si91x_driver_send_side_band_crypto(SLI_COMMON_REQ_ENCRYPT_CRYPTO,
                                                 request,
                                                 (sizeof(sli_si91x_aes_request_t)),
                                                 SLI_WIFI_WAIT_FOR_RESPONSE(SLI_COMMON_RSP_ENCRYPT_CRYPTO_WAIT_TIME));
  free(request);
  VERIFY_STATUS_AND_RETURN(status);
  return status;
}
#endif

sl_status_t sl_si91x_aes(sl_si91x_aes_config_t *config, uint8_t *output)
{
  uint16_t chunk_len = 0;
  uint16_t offset    = 0;
  uint8_t aes_flags  = 0;
  sl_status_t status = SL_STATUS_FAIL;

  SL_VERIFY_POINTER_OR_RETURN(config->msg, SL_STATUS_NULL_POINTER);

  if (((config->aes_mode == SL_SI91X_AES_CBC) || (config->aes_mode == SL_SI91X_AES_CTR)) && (config->iv == NULL)) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  if (config->msg_length % SL_SI91X_AES_BLOCK_SIZE) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  uint16_t total_length = config->msg_length;

#ifdef SL_SI91X_SIDE_BAND_CRYPTO
  (void)chunk_len;
  (void)offset;
  (void)aes_flags;
  (void)total_length;

  status = sli_si91x_aes_side_band(config, output);
  return status;
#else

#if defined(SLI_MULTITHREAD_DEVICE_SI91X)
  if (crypto_aes_mutex == NULL) {
    crypto_aes_mutex = sl_si91x_crypto_threadsafety_init(crypto_aes_mutex);
  }
  mutex_result = sl_si91x_crypto_mutex_acquire(crypto_aes_mutex);
#endif

  while (total_length) {
    // Check total length
    if (total_length > SLI_SI91X_MAX_DATA_SIZE_IN_BYTES_FOR_AES) {
      chunk_len = SLI_SI91X_MAX_DATA_SIZE_IN_BYTES_FOR_AES;
      if (offset == 0) {
        // Make aes_flags as first chunk
        aes_flags |= FIRST_CHUNK;
      } else {
        // Make aes_flags as last chunk
        aes_flags = MIDDLE_CHUNK;
      }
    } else {
      chunk_len = total_length;
      aes_flags = LAST_CHUNK;
      if (offset == 0) {
        // If the total length is less than 1408 and offset is zero, make aes_flags as both first chunk as well as last chunk
        aes_flags |= FIRST_CHUNK;
      }
    }

    // Send the current chunk length message
    status = sli_si91x_aes_pending(config, chunk_len, aes_flags, output);
    if (status != SL_STATUS_OK) {
#if defined(SLI_MULTITHREAD_DEVICE_SI91X)
      mutex_result = sl_si91x_crypto_mutex_release(crypto_aes_mutex);
#endif
      return status;
    }

    // Increment the offset value
    offset += chunk_len;
    config->msg += chunk_len;

    output += chunk_len;

    // Decrement the total message length
    total_length -= chunk_len;
  }

#if defined(SLI_MULTITHREAD_DEVICE_SI91X)
  mutex_result = sl_si91x_crypto_mutex_release(crypto_aes_mutex);
#endif

  return status;
#endif
}

sl_status_t sl_si91x_aes_multipart(const sl_si91x_aes_config_t *config,
                                   uint16_t chunk_length,
                                   uint8_t aes_flags,
                                   uint8_t *output)
{

  SL_VERIFY_POINTER_OR_RETURN(config->msg, SL_STATUS_NULL_POINTER);

  if (((config->aes_mode == SL_SI91X_AES_CBC) || (config->aes_mode == SL_SI91X_AES_CTR)) && (config->iv == NULL)) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  if ((chunk_length % SL_SI91X_AES_BLOCK_SIZE) || (chunk_length > SLI_SI91X_MAX_DATA_SIZE_IN_BYTES_FOR_AES)) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  // Check if the AES mode is supported
  if ((config->aes_mode != SL_SI91X_AES_CBC) && (config->aes_mode != SL_SI91X_AES_CTR)) {
    return SL_STATUS_NOT_SUPPORTED;
  }

  // If side band crypto is enabled, call the side band function
#ifdef SL_SI91X_SIDE_BAND_CRYPTO
  (void)chunk_length;
  (void)aes_flags;
  return sli_si91x_aes_side_band(config, output);
#else
  // calling sli_si91x_aes_pending with the provided arguments
  return sli_si91x_aes_pending(config, chunk_length, aes_flags, output);
#endif
}
