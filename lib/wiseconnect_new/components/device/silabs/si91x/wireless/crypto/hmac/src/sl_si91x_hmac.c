/***************************************************************************/ /**
 * @file
 * @brief SL SI91X HMAC source file
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

#include "sl_si91x_hmac.h"
#include "sl_si91x_protocol_types.h"
#include "sl_constants.h"
#include "sl_si91x_driver.h"
#include "sli_wifi_utility.h"
#if defined(SLI_MULTITHREAD_DEVICE_SI91X)
#include "sl_si91x_crypto_thread.h"
#endif
#include <string.h>
#include "sli_wifi_utility.h"
#ifndef SL_SI91X_SIDE_BAND_CRYPTO
static sl_status_t sli_si91x_hmac_pending(const sl_si91x_hmac_config_t *config,
                                          const uint8_t *data,
                                          uint16_t chunk_length,
                                          uint32_t total_length,
                                          uint8_t hmac_sha_flags,
                                          uint8_t *output)
{
  sl_status_t status                    = SL_STATUS_FAIL;
  sl_wifi_buffer_t *buffer              = NULL;
  const sl_wifi_system_packet_t *packet = NULL;
  sli_si91x_hmac_sha_request_t *request = (sli_si91x_hmac_sha_request_t *)malloc(sizeof(sli_si91x_hmac_sha_request_t));
  SL_VERIFY_POINTER_OR_RETURN(request, SL_STATUS_ALLOCATION_FAILED);

  memset(request, 0, sizeof(sli_si91x_hmac_sha_request_t));

  request->algorithm_type       = HMAC_SHA;
  request->algorithm_sub_type   = (uint8_t)config->hmac_mode;
  request->hmac_sha_flags       = hmac_sha_flags;
  request->total_length         = (uint16_t)total_length;
  request->current_chunk_length = chunk_length;
  memcpy(request->hmac_data, data, chunk_length);

#if defined(SLI_SI917B0)
  request->key_info.key_type                         = config->key_config.B0.key_type;
  request->key_info.key_detail.key_size              = config->key_config.B0.key_size;
  request->key_info.key_detail.key_spec.key_slot     = config->key_config.B0.key_slot;
  request->key_info.key_detail.key_spec.wrap_iv_mode = config->key_config.B0.wrap_iv_mode;

  // NOTE: The parameter request->key_info.key_detail.key_spec.key_buffer isn't required in HMAC, as the key is being concatenated with the input data in sl_si91x_hmac().

  if (config->key_config.B0.wrap_iv_mode != SL_SI91X_WRAP_IV_ECB_MODE) {
    memcpy(request->key_info.key_detail.key_spec.wrap_iv, config->key_config.B0.wrap_iv, SL_SI91X_IV_SIZE);
  }
#else
  request->key_length = config->key_config.A0.key_length;
#endif

  status = sli_si91x_driver_send_command(
    SLI_COMMON_REQ_ENCRYPT_CRYPTO,
    SLI_WIFI_COMMON_CMD,
    request,
    (sizeof(sli_si91x_hmac_sha_request_t) - SL_SI91X_MAX_DATA_SIZE_IN_BYTES + chunk_length),
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
static sl_status_t sli_si91x_hmac_side_band(uint16_t total_length,
                                            uint8_t *data,
                                            sl_si91x_hmac_config_t *config,
                                            uint8_t *output)
{
  sl_status_t status                    = SL_STATUS_FAIL;
  sli_si91x_hmac_sha_request_t *request = (sli_si91x_hmac_sha_request_t *)malloc(sizeof(sli_si91x_hmac_sha_request_t));
  SL_VERIFY_POINTER_OR_RETURN(request, SL_STATUS_ALLOCATION_FAILED);

  memset(request, 0, sizeof(sli_si91x_hmac_sha_request_t));

  request->algorithm_type     = HMAC_SHA;
  request->algorithm_sub_type = config->hmac_mode;
  request->total_length       = total_length;
  request->hmac_data          = data;
  request->output             = output;

  request->key_info.key_type                         = config->key_config.B0.key_type;
  request->key_info.key_detail.key_size              = config->key_config.B0.key_size;
  request->key_info.key_detail.key_spec.key_slot     = config->key_config.B0.key_slot;
  request->key_info.key_detail.key_spec.wrap_iv_mode = config->key_config.B0.wrap_iv_mode;

  // NOTE: The parameter request->key_info.key_detail.key_spec.key_buffer isn't required in HMAC, as the key is being concatenated with the input data in sl_si91x_hmac().

  if (config->key_config.B0.wrap_iv_mode != SL_SI91X_WRAP_IV_ECB_MODE) {
    memcpy(request->key_info.key_detail.key_spec.wrap_iv, config->key_config.B0.wrap_iv, SL_SI91X_IV_SIZE);
  }

  status = sl_si91x_driver_send_side_band_crypto(SLI_COMMON_REQ_ENCRYPT_CRYPTO,
                                                 request,
                                                 (sizeof(sli_si91x_hmac_sha_request_t)),
                                                 SLI_WIFI_WAIT_FOR_RESPONSE(SLI_COMMON_RSP_ENCRYPT_CRYPTO_WAIT_TIME));
  free(request);
  VERIFY_STATUS_AND_RETURN(status);
  return status;
}

#endif

sl_status_t sl_si91x_hmac(const sl_si91x_hmac_config_t *config, uint8_t *output)
{
  uint32_t total_length  = 0;
  uint16_t chunk_len     = 0;
  uint16_t offset        = 0;
  uint8_t hmac_sha_flags = 0;
  uint32_t key_length    = 0;
  uint8_t *data          = NULL;
  sl_status_t status     = SL_STATUS_FAIL;

  SL_VERIFY_POINTER_OR_RETURN(config->msg, SL_STATUS_NULL_POINTER);

#if defined(SLI_SI917B0)
  key_length = config->key_config.B0.key_size;
#else
  key_length = config->key_config.A0.key_length;
#endif

  total_length = (config->msg_length + key_length);

  data = (uint8_t *)malloc(total_length);
  SL_VERIFY_POINTER_OR_RETURN(data, SL_STATUS_ALLOCATION_FAILED);

  memset(data, 0, total_length);

#if defined(SLI_SI917B0)
  memcpy(data, config->key_config.B0.key, key_length); // Copy key into data
#else
  memcpy(data, config->key_config.A0.key, key_length); // Copy key into data
#endif

  memcpy((data + key_length), config->msg, config->msg_length); // Copy message into data

#ifdef SL_SI91X_SIDE_BAND_CRYPTO
  // to avoid unused variable warning
  UNUSED_VARIABLE(chunk_len);
  UNUSED_VARIABLE(offset);
  UNUSED_VARIABLE(hmac_sha_flags);
  status = sli_si91x_hmac_side_band(total_length, data, config, output);
  return status;
#else
#if defined(SLI_MULTITHREAD_DEVICE_SI91X)
  if (crypto_hmac_mutex == NULL) {
    crypto_hmac_mutex = sl_si91x_crypto_threadsafety_init(crypto_hmac_mutex);
  }
  mutex_result = sl_si91x_crypto_mutex_acquire(crypto_hmac_mutex);

#endif

  while (total_length) {
    // Check total length
    if (total_length > SL_SI91X_MAX_DATA_SIZE_IN_BYTES) {
      chunk_len = SL_SI91X_MAX_DATA_SIZE_IN_BYTES;
      if (offset == 0) {
        // Make hmac_sha_flag as first chunk
        hmac_sha_flags |= FIRST_CHUNK;
      } else {
        // Make hmac_sha_flag as Middle chunk
        hmac_sha_flags = MIDDLE_CHUNK;
      }
    } else {
      chunk_len = (uint16_t)total_length;

      // Make hmac_sha_flag as Last chunk
      hmac_sha_flags = LAST_CHUNK;
      if (offset == 0) {
        // If the total length is less than 1400 and offset is zero, make hmac_sha_flag as both first chunk as well as last chunk
        hmac_sha_flags |= FIRST_CHUNK;
      }
    }

    status = sli_si91x_hmac_pending(config, data, chunk_len, (config->msg_length + key_length), hmac_sha_flags, output);
    if (status != SL_STATUS_OK) {
      free(data);
#if defined(SLI_MULTITHREAD_DEVICE_SI91X)
      mutex_result = sl_si91x_crypto_mutex_release(crypto_hmac_mutex);
#endif
      return status;
    }

    // Increment the offset value
    offset += chunk_len;
    output += chunk_len;

    // Decrement the total message length
    total_length -= chunk_len;
  }

  free(data);
#if defined(SLI_MULTITHREAD_DEVICE_SI91X)
  mutex_result = sl_si91x_crypto_mutex_release(crypto_hmac_mutex);
#endif
  return status;
#endif
}
