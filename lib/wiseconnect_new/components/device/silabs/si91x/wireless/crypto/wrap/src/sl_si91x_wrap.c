/***************************************************************************/ /**
 * @file
 * @brief SL SI91X Wrap source file
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

#include "sl_si91x_wrap.h"
#include "sl_si91x_crypto.h"
#include "sl_status.h"
#include "sl_constants.h"
#include "sl_si91x_protocol_types.h"
#include "sl_si91x_driver.h"
#include "string.h"
#include "sli_wifi_utility.h"
// Defines the buffer size per request for the SI91X wrap key operation.
#define SLI_SI91X_WRAP_KEY_BUFFER_SIZE_PER_REQUEST 1408

#ifndef SL_SI91X_SIDE_BAND_CRYPTO
static sl_status_t sli_si91x_wrap_pending(sl_si91x_wrap_config_t *config,
                                          const uint8_t *key_buffer,
                                          uint8_t wrap_flags,
                                          uint16_t chunk_length,
                                          uint8_t *output)
{
  sl_status_t status                    = SL_STATUS_FAIL;
  sl_wifi_buffer_t *buffer              = NULL;
  const sl_wifi_system_packet_t *packet = NULL;
  size_t output_size                    = 0;
  sli_si91x_wrap_request_t *request     = (sli_si91x_wrap_request_t *)malloc(sizeof(sli_si91x_wrap_request_t));
  SL_VERIFY_POINTER_OR_RETURN(request, SL_STATUS_ALLOCATION_FAILED);

  memset(request, 0, sizeof(sli_si91x_wrap_request_t));

  request->algorithm_type         = WRAP;
  request->current_chunk_length   = chunk_length;
  request->wrap_flags             = wrap_flags;
  request->key_info.key_size      = config->key_size;
  request->key_info.key_type      = config->key_type;
  request->key_info.padding       = config->padding;
  request->key_info.hmac_sha_mode = config->hmac_sha_mode;
  request->key_info.wrap_iv_mode  = config->wrap_iv_mode;
  memcpy(request->key_info.key_buffer, key_buffer, chunk_length);

  if (config->wrap_iv_mode == SL_SI91X_WRAP_IV_CBC_MODE) {
    memcpy(request->key_info.wrap_iv, config->wrap_iv, SL_SI91X_IV_SIZE);
  }

  status = sli_si91x_driver_send_command(
    SLI_COMMON_REQ_ENCRYPT_CRYPTO,
    SLI_WIFI_COMMON_CMD,
    request,
    (sizeof(sli_si91x_wrap_request_t) - SLI_SI91X_WRAP_KEY_BUFFER_SIZE_PER_REQUEST + chunk_length),
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

  //! Determine output block size based upon selected algorithm if HMAC padding is enabled
  if (request->key_info.padding & SL_SI91X_HMAC_PADDING) {
    int hmac_sha_mode = request->key_info.hmac_sha_mode & 0xF;
    if (hmac_sha_mode == SL_SI91X_WRAP_HMAC_SHA_384 || hmac_sha_mode == SL_SI91X_WRAP_HMAC_SHA_512) {
      output_size =
        (config->key_size > SL_SI91X_HMAC_512_BLOCK_SIZE)
          ? ((config->key_size + SL_SI91X_HMAC_512_BLOCK_SIZE_ALIGN) & (~SL_SI91X_HMAC_512_BLOCK_SIZE_ALIGN))
          : ((config->key_size + SL_SI91X_DEFAULT_16BYTE_ALIGN) & (~SL_SI91X_DEFAULT_16BYTE_ALIGN));
    } else {
      output_size =
        (config->key_size > SL_SI91X_HMAC_256_BLOCK_SIZE)
          ? ((config->key_size + SL_SI91X_HMAC_256_BLOCK_SIZE_ALIGN) & (~SL_SI91X_HMAC_256_BLOCK_SIZE_ALIGN))
          : ((config->key_size + SL_SI91X_DEFAULT_16BYTE_ALIGN) & (~SL_SI91X_DEFAULT_16BYTE_ALIGN));
    }
    config->key_size = output_size;
  }

  memcpy(output, packet->data, packet->length);
  sli_si91x_host_free_buffer(buffer);
  free(request);

  return status;
}

#else

static sl_status_t sli_si91x_wrap_side_band(sl_si91x_wrap_config_t *config, uint8_t *output)
{

  sl_status_t status                = SL_STATUS_FAIL;
  sli_si91x_wrap_request_t *request = (sli_si91x_wrap_request_t *)malloc(sizeof(sli_si91x_wrap_request_t));
  SL_VERIFY_POINTER_OR_RETURN(request, SL_STATUS_ALLOCATION_FAILED);

  memset(request, 0, sizeof(sli_si91x_wrap_request_t));

  request->algorithm_type         = WRAP;
  request->key_info.key_size      = config->key_size;
  request->key_info.key_type      = config->key_type;
  request->key_info.padding       = config->padding;
  request->key_info.hmac_sha_mode = config->hmac_sha_mode;
  request->key_info.wrap_iv_mode  = config->wrap_iv_mode;
  request->key_info.key_buffer    = config->key_buffer;
  request->key_info.wrap_iv       = config->wrap_iv;

  request->output = output;

  status = sl_si91x_driver_send_side_band_crypto(SLI_COMMON_REQ_ENCRYPT_CRYPTO,
                                                 request,
                                                 (sizeof(sli_si91x_wrap_request_t)),
                                                 SLI_WIFI_WAIT_FOR_RESPONSE(SLI_COMMON_RSP_ENCRYPT_CRYPTO_WAIT_TIME));
  free(request);
  VERIFY_STATUS_AND_RETURN(status);
  return status;
}

#endif

sl_status_t sl_si91x_wrap(sl_si91x_wrap_config_t *config, uint8_t *output)
{
  SL_VERIFY_POINTER_OR_RETURN(config->key_buffer, SL_STATUS_NULL_POINTER);
  SL_VERIFY_POINTER_OR_RETURN(output, SL_STATUS_NULL_POINTER);

  if (config->key_size > SL_SI91X_WRAP_KEY_BUFFER_SIZE) {
    return SL_STATUS_INVALID_CONFIGURATION;
  }

  sl_status_t status = SL_STATUS_FAIL;

#ifdef SL_SI91X_SIDE_BAND_CRYPTO
  status = sli_si91x_wrap_side_band(config, output);
  return status;
#else
  uint16_t chunk_len        = 0;
  uint16_t offset           = 0;
  uint8_t wrap_flags        = 0;
  uint32_t total_length     = config->key_size;
  const uint8_t *key_buffer = NULL;
  key_buffer                = config->key_buffer;

  while (total_length) {
    // Check total length
    if (total_length > SLI_SI91X_WRAP_KEY_BUFFER_SIZE_PER_REQUEST) {
      chunk_len = SLI_SI91X_WRAP_KEY_BUFFER_SIZE_PER_REQUEST;
      if (offset == 0) {
        // Make wrap_flags as first chunk
        wrap_flags |= FIRST_CHUNK;
      } else {
        // Make wrap_flags as middle chunk
        wrap_flags = MIDDLE_CHUNK;
      }
    } else {
      chunk_len  = (uint16_t)total_length;
      wrap_flags = LAST_CHUNK;
      if (offset == 0) {
        // If the total length is less than SLI_SI91X_WRAP_KEY_BUFFER_SIZE_PER_REQUEST and offset is zero, make wrap_flags as both first chunk as well as last chunk
        wrap_flags |= FIRST_CHUNK;
      }
    }

    // Send the current chunk length message
    status = sli_si91x_wrap_pending(config, key_buffer, wrap_flags, chunk_len, output);
    if (status != SL_STATUS_OK) {
      return status;
    }

    offset += chunk_len;
    key_buffer += chunk_len;
    output += chunk_len;
    total_length -= chunk_len;
  }

  return status;
#endif
}
