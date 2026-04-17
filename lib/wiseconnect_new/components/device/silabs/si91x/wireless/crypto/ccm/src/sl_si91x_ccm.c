/***************************************************************************/ /**
 * @file
 * @brief SL SI91X CCM source file
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

#include "sl_si91x_ccm.h"
#include "sl_si91x_crypto.h"
#include "sl_status.h"
#include "sl_constants.h"
#include "sl_si91x_protocol_types.h"
#include "sl_si91x_driver.h"
#include "sli_wifi_utility.h"
#if defined(SLI_MULTITHREAD_DEVICE_SI91X)
#include "sl_si91x_crypto_thread.h"
#endif
#include <string.h>

static sl_status_t sli_si91x_ccm_config_check(sl_si91x_ccm_config_t *config)
{
  // Only 32 bytes M4 OTA built-in key support is present
  if (config->key_config.b0.key_type == SL_SI91X_BUILT_IN_KEY) {
    if (((int)config->key_config.b0.key_size != (int)SL_SI91X_KEY_SIZE_1)
        || (config->key_config.b0.key_slot != SL_SI91X_KEY_SLOT_1))
      return SL_STATUS_INVALID_PARAMETER;
  }

  if ((config->nonce_length < SLI_SI91X_CCM_IV_MIN_SIZE) || (config->nonce_length > SLI_SI91X_CCM_IV_MAX_SIZE)) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  return SL_STATUS_OK;
}

#if defined(SLI_SI917B0)
static void sli_si91x_ccm_get_key_info(sli_si91x_ccm_request_t *request, const sl_si91x_ccm_config_t *config)
{
  request->key_info.key_type                         = config->key_config.b0.key_type;
  request->key_info.key_detail.key_size              = config->key_config.b0.key_size;
  request->key_info.key_detail.key_spec.key_slot     = config->key_config.b0.key_slot;
  request->key_info.key_detail.key_spec.wrap_iv_mode = config->key_config.b0.wrap_iv_mode;
  request->key_info.reserved                         = config->key_config.b0.reserved;
  if (config->key_config.b0.wrap_iv_mode) {
    memcpy(request->key_info.key_detail.key_spec.wrap_iv, config->key_config.b0.wrap_iv, SL_SI91X_IV_SIZE);
  }
  memcpy(request->key_info.key_detail.key_spec.key_buffer,
         config->key_config.b0.key_buffer,
         config->key_config.b0.key_size);
}
#endif

#ifndef SL_SI91X_SIDE_BAND_CRYPTO
static sl_status_t sli_si91x_ccm_pending(sl_si91x_ccm_config_t *config,
                                         uint16_t chunk_length,
                                         uint8_t ccm_flags,
                                         uint8_t *output)
{
  sl_status_t status               = SL_STATUS_FAIL;
  sl_wifi_buffer_t *buffer         = NULL;
  sl_wifi_system_packet_t *packet  = NULL;
  sli_si91x_ccm_request_t *request = (sli_si91x_ccm_request_t *)malloc(sizeof(sli_si91x_ccm_request_t));

  SL_VERIFY_POINTER_OR_RETURN(request, SL_STATUS_ALLOCATION_FAILED);

  status = sli_si91x_ccm_config_check(config);
  if (status != SL_STATUS_OK) {
    return status;
  }

  memset(request, 0, sizeof(sli_si91x_ccm_request_t));

  request->algorithm_type       = CCM;
  request->ccm_flags            = ccm_flags;
  request->total_msg_length     = config->msg_length;
  request->current_chunk_length = chunk_length;
  request->encrypt_decryption   = config->encrypt_decrypt;
  request->ad_length            = config->ad_length;
  request->tag_length           = config->tag_length;
  request->nonce_length         = config->nonce_length;

  if (config->ad_length > 0) {
    memcpy(request->ad, config->ad, config->ad_length);
  }
  if (chunk_length > 0) {
    memcpy(request->msg, config->msg, chunk_length);
  }
  memcpy(request->nonce, config->nonce, config->nonce_length);
  memcpy(request->tag, config->tag, config->tag_length);

#if defined(SLI_SI917B0)
  sli_si91x_ccm_get_key_info(request, config);

#else
  memcpy(request->key, config->key_config.a0.key, request->key_length);
  request->key_length = config->key_config.a0.key_length;
#endif

  status = sli_si91x_driver_send_command(
    SLI_COMMON_REQ_ENCRYPT_CRYPTO,
    SLI_WIFI_COMMON_CMD,
    request,
    (sizeof(sli_si91x_ccm_request_t) - SL_SI91X_MAX_DATA_SIZE_IN_BYTES_FOR_CCM + chunk_length),
    SLI_WIFI_WAIT_FOR_RESPONSE(SLI_COMMON_RSP_ENCRYPT_CRYPTO_WAIT_TIME),
    NULL,
    &buffer);

  if (status != SL_STATUS_OK) {
    free(request);
    if (buffer != NULL)
      sli_si91x_host_free_buffer(buffer);
  }
  VERIFY_STATUS_AND_RETURN(status);

  packet = sli_wifi_host_get_buffer_data(buffer, 0, NULL);

  memcpy(output, packet->data, packet->length);
  if (buffer != NULL)
    sli_si91x_host_free_buffer(buffer);
  free(request);
  return status;
}

#else
static sl_status_t sli_si91x_ccm_side_band(sl_si91x_ccm_config_t *config, uint8_t *output)
{

  sl_status_t status               = SL_STATUS_FAIL;
  sli_si91x_ccm_request_t *request = (sli_si91x_ccm_request_t *)malloc(sizeof(sli_si91x_ccm_request_t));

  SL_VERIFY_POINTER_OR_RETURN(request, SL_STATUS_ALLOCATION_FAILED);

  status = sli_si91x_ccm_config_check(config);
  if (status != SL_STATUS_OK) {
    return status;
  }

  memset(request, 0, sizeof(sli_si91x_ccm_request_t));

  request->algorithm_type     = CCM;
  request->total_msg_length   = config->msg_length;
  request->encrypt_decryption = config->encrypt_decrypt;
  request->ad_length          = config->ad_length;
  request->tag_length         = config->tag_length;
  request->nonce_length       = config->nonce_length;

  if (config->ad_length > 0) {
    request->ad = (uint8_t *)config->ad;
  }
  if (config->msg_length > 0) {
    request->msg = (uint8_t *)config->msg;
  }
  request->nonce = (uint8_t *)config->nonce;
  request->tag   = config->tag;

  sli_si91x_ccm_get_key_info(request, config);
  request->output = output;

  status = sl_si91x_driver_send_side_band_crypto(SLI_COMMON_REQ_ENCRYPT_CRYPTO,
                                                 request,
                                                 (sizeof(sli_si91x_ccm_request_t)),
                                                 SLI_WIFI_WAIT_FOR_RESPONSE(SLI_COMMON_RSP_ENCRYPT_CRYPTO_WAIT_TIME));
  free(request);
  VERIFY_STATUS_AND_RETURN(status);
  return status;
}
#endif

sl_status_t sl_si91x_ccm(sl_si91x_ccm_config_t *config, uint8_t *output)
{
  sl_status_t status = SL_STATUS_FAIL;
#ifndef SL_SI91X_SIDE_BAND_CRYPTO
  uint16_t chunk_len    = 0;
  uint16_t offset       = 0;
  uint8_t ccm_flags     = 0;
  uint16_t total_length = 0;
#endif

  SL_VERIFY_POINTER_OR_RETURN(config->msg, SL_STATUS_NULL_POINTER);

  if (((config->msg == NULL) && (config->msg_length != 0)) || ((config->ad == NULL) && (config->ad_length != 0))
      || (config->nonce == NULL) || (config->tag == NULL) || ((output == NULL) && (config->msg_length != 0))
#if !defined(SLI_SI917B0)
      || (config->key_config.a0.key == NULL)
#endif
  ) {
    status = SL_STATUS_INVALID_PARAMETER;
  }

#ifdef SL_SI91X_SIDE_BAND_CRYPTO
  status = sli_si91x_ccm_side_band(config, output);
  return status;
#else
  total_length = config->msg_length;
#if defined(SLI_MULTITHREAD_DEVICE_SI91X)
  if (crypto_ccm_mutex == NULL) {
    crypto_ccm_mutex = sl_si91x_crypto_threadsafety_init(crypto_ccm_mutex);
  }
  mutex_result = sl_si91x_crypto_mutex_acquire(crypto_ccm_mutex);
#endif

  if (total_length != 0) {
    while (total_length) {
      // Check total length
      if (total_length > SL_SI91X_MAX_DATA_SIZE_IN_BYTES_FOR_CCM) {
        chunk_len = SL_SI91X_MAX_DATA_SIZE_IN_BYTES_FOR_CCM;
        if (offset == 0) {
          // Make ccm_flags as first chunk
          ccm_flags |= FIRST_CHUNK;
        } else {
          // Make ccm_flags as middle chunk
          ccm_flags = MIDDLE_CHUNK;
        }
      } else {
        chunk_len = total_length;
        ccm_flags = LAST_CHUNK;
        if (offset == 0) {
          // If the total length is less than 1200 and offset is zero, make ccm_flags as both first chunk as well as last chunk
          ccm_flags |= FIRST_CHUNK;
        }
      }

      // Send the current chunk length message
      status = sli_si91x_ccm_pending(config, chunk_len, ccm_flags, output);
      if (status != SL_STATUS_OK) {
#if defined(SLI_MULTITHREAD_DEVICE_SI91X)
        mutex_result = sl_si91x_crypto_mutex_release(crypto_ccm_mutex);
#endif
        return status;
      }

      // Increment the offset value
      offset += chunk_len;
      config->msg += chunk_len;

      // Decrement the total message length
      total_length -= chunk_len;
    }
  } else {
    ccm_flags = LAST_CHUNK | FIRST_CHUNK;

    // Send the current chunk length message
    status = sli_si91x_ccm_pending(config, chunk_len, ccm_flags, output);
    if (status != SL_STATUS_OK) {
#if defined(SLI_MULTITHREAD_DEVICE_SI91X)
      mutex_result = sl_si91x_crypto_mutex_release(crypto_ccm_mutex);
#endif
      return status;
    }
  }

  // Copy the tag to the tag_buffer
  if (config->encrypt_decrypt == SL_SI91X_CCM_ENCRYPT) {
    memcpy(config->tag, output + config->msg_length, config->tag_length);
  }

#if defined(SLI_MULTITHREAD_DEVICE_SI91X)
  mutex_result = sl_si91x_crypto_mutex_release(crypto_ccm_mutex);
#endif

  return status;
#endif
}
