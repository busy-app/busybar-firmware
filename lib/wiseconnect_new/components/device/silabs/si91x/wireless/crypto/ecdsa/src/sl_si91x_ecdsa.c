/***************************************************************************/ /**
 * @file
 * @brief SL SI91X ECDSA source file
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

#include "sl_si91x_ecdsa.h"
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

static sl_status_t sl_si91x_ecdsa_pending(sl_si91x_ecdsa_config_t *config,
                                          uint16_t chunk_length,
                                          uint8_t ecdsa_flags,
                                          uint8_t *output)
{
  sl_status_t status                = SL_STATUS_FAIL;
  sl_wifi_buffer_t *buffer          = NULL;
  sl_wifi_system_packet_t *packet   = NULL;
  sl_si91x_ecdsa_request_t *request = (sl_si91x_ecdsa_request_t *)malloc(sizeof(sl_si91x_ecdsa_request_t));

  SL_VERIFY_POINTER_OR_RETURN(request, SL_STATUS_ALLOCATION_FAILED);

  memset(request, 0, sizeof(sl_si91x_ecdsa_request_t));

  request->algorithm_type       = ECDSA;
  request->algorithm_sub_type   = config->ecdsa_operation;
  request->ecdsa_flags          = ecdsa_flags;
  request->sha_mode             = config->sha_mode;
  request->curve_id             = config->curve_id;
  request->private_key_length   = config->private_key_length;
  request->public_key_length    = config->public_key_length;
  request->signature_length     = config->signature_length;
  request->msg_len              = config->msg_length;
  request->current_chunk_length = chunk_length;

  if (config->private_key_length > 0) {
    memcpy(request->private_key, config->private_key, config->private_key_length);
  }

  if (request->public_key_length > 0) {
    memcpy(request->public_key, config->public_key, config->public_key_length);
  }

  if (chunk_length > 0) {
    memcpy(request->msg, config->msg, chunk_length);
  }

  if (config->signature_length > 0) {
    memcpy(request->signature, config->signature, config->signature_length);
  }

#if defined(SLI_SI917B0)
  request->key_info.key_type                         = config->key_config.b0.key_type;
  request->key_info.key_detail.key_size              = config->key_config.b0.key_size;
  request->key_info.key_detail.key_spec.key_slot     = config->key_config.b0.key_slot;
  request->key_info.key_detail.key_spec.wrap_iv_mode = config->key_config.b0.wrap_iv_mode;
  request->key_info.reserved                         = config->key_config.b0.reserved;
  if (config->key_config.b0.wrap_iv_mode) {
    memcpy(request->key_info.key_detail.key_spec.wrap_iv, config->key_config.b0.wrap_iv, SL_SI91X_IV_SIZE);
  }
#else
  request->key_length = config->key_config.a0.key_length;
#endif

  status = sli_si91x_driver_send_command(
    SLI_COMMON_REQ_ENCRYPT_CRYPTO,
    SLI_WIFI_COMMON_CMD,
    request,
    (sizeof(sl_si91x_ecdsa_request_t) - SL_SI91X_MAX_DATA_SIZE_IN_BYTES_FOR_ECDSA + chunk_length),
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

  if (config->ecdsa_operation == SL_SI91X_GENERATE_ECC_KEY_PAIR) {
    // Verify the length from the firmware against the expected length
    SL_ASSERT(packet->length == (config->public_key_length));
  }

  if (config->ecdsa_operation == SL_SI91X_ECDSA_GENERATE_SIGN) {
    config->signature_length = packet->length;
  }

  memcpy(output, packet->data, packet->length);
  if (buffer != NULL)
    sli_si91x_host_free_buffer(buffer);
  free(request);

  return status;
}

sl_status_t sl_si91x_ecdsa(sl_si91x_ecdsa_config_t *config, uint8_t *output)
{
  uint16_t chunk_len  = 0;
  uint16_t offset     = 0;
  uint8_t ecdsa_flags = 0;
  sl_status_t status  = SL_STATUS_FAIL;

  if (config->curve_id == 0) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  if (config->ecdsa_operation != SL_SI91X_GENERATE_ECC_KEY_PAIR) {
    SL_VERIFY_POINTER_OR_RETURN(config->msg, SL_STATUS_NULL_POINTER);
  }

  if (config->ecdsa_operation == SL_SI91X_ECDSA_GENERATE_SIGN && config->private_key == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  if (config->ecdsa_operation == SL_SI91X_ECDSA_VERIFY_SIGN
      && (config->public_key == NULL || config->signature == NULL)) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  uint16_t total_length = config->msg_length;

#if defined(SLI_MULTITHREAD_DEVICE_SI91X)
  if (crypto_ecdsa_mutex == NULL) {
    crypto_ecdsa_mutex = sl_si91x_crypto_threadsafety_init(crypto_ecdsa_mutex);
  }
  mutex_result = sl_si91x_crypto_mutex_acquire(crypto_ecdsa_mutex);
#endif

  if (total_length != 0) {
    while (total_length) {
      // Check total length
      if (total_length > SL_SI91X_MAX_DATA_SIZE_IN_BYTES_FOR_ECDSA) {
        chunk_len = SL_SI91X_MAX_DATA_SIZE_IN_BYTES_FOR_ECDSA;
        if (offset == 0) {
          // Make ecdsa_flags as first chunk
          ecdsa_flags |= FIRST_CHUNK;
        } else {
          // Make ecdsa_flags as middle chunk
          ecdsa_flags = MIDDLE_CHUNK;
        }
      } else {
        chunk_len   = total_length;
        ecdsa_flags = LAST_CHUNK;
        if (offset == 0) {
          // If the total length is less than 1000 and offset is zero, make ecdsa_flags as both first chunk as well as last chunk
          ecdsa_flags |= FIRST_CHUNK;
        }
      }

      // Send the current chunk length message
      status = sl_si91x_ecdsa_pending(config, chunk_len, ecdsa_flags, output);
      if (status != SL_STATUS_OK) {
#if defined(SLI_MULTITHREAD_DEVICE_SI91X)
        mutex_result = sl_si91x_crypto_mutex_release(crypto_ecdsa_mutex);
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
    chunk_len   = total_length;
    ecdsa_flags = LAST_CHUNK;
    ecdsa_flags |= FIRST_CHUNK;

    // Send the current chunk length message
    status = sl_si91x_ecdsa_pending(config, chunk_len, ecdsa_flags, output);
    if (status != SL_STATUS_OK) {
#if defined(SLI_MULTITHREAD_DEVICE_SI91X)
      mutex_result = sl_si91x_crypto_mutex_release(crypto_ecdsa_mutex);
#endif
      return status;
    }
  }

#if defined(SLI_MULTITHREAD_DEVICE_SI91X)
  mutex_result = sl_si91x_crypto_mutex_release(crypto_ecdsa_mutex);
#endif

  return status;
}
