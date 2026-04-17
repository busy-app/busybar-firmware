/***************************************************************************//**
 * @file
 * @brief Silicon Labs PSA Crypto Driver Mac functions.
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
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

#include "sli_psa_driver_features.h"

#if defined(SLI_MBEDTLS_DEVICE_HSE)

#include "sli_psa_driver_common.h"  // sli_psa_zeroize()
#include "psa/crypto.h"

#include "mbedtls/platform.h"

#include "sli_se_driver_mac.h"
#include "sli_se_manager_internal.h"
#include "sli_se_driver_key_management.h"
#include "sli_psa_driver_common.h"
#include "sl_se_manager.h"
#include "sl_se_manager_cipher.h"

#include <string.h>

//------------------------------------------------------------------------------
// Static functions

#if defined(SLI_PSA_DRIVER_FEATURE_HMAC)

sl_se_hash_type_t sli_se_hash_type_from_psa_hmac_alg(psa_algorithm_t alg,
                                                     size_t *length)
{
  if (!PSA_ALG_IS_HMAC(alg)) {
    return SL_SE_HASH_NONE;
  }

  psa_algorithm_t hash_alg = PSA_ALG_HMAC_GET_HASH(alg);
  switch (hash_alg) {
    case PSA_ALG_SHA_1:
      *length = 20;
      return SL_SE_HASH_SHA1;
    case PSA_ALG_SHA_224:
      *length = 28;
      return SL_SE_HASH_SHA224;
    case PSA_ALG_SHA_256:
      *length = 32;
      return SL_SE_HASH_SHA256;

    #if defined(SLI_MBEDTLS_DEVICE_HSE_VAULT_HIGH)
    case PSA_ALG_SHA_384:
      *length = 48;
      return SL_SE_HASH_SHA384;
    case PSA_ALG_SHA_512:
      *length = 64;
      return SL_SE_HASH_SHA512;
    #endif
    #if defined(_SILICON_LABS_32B_SERIES_3)
    case PSA_ALG_AES_MMO_ZIGBEE:
      // AES-MMO digest size is 16 bytes
      *length = 16;
      return SL_SE_HASH_AES_MMO;
    #endif
    default:
      return SL_SE_HASH_NONE;
  }
}

#endif // SLI_PSA_DRIVER_FEATURE_HMAC

//------------------------------------------------------------------------------
// Single-shot driver entry points

psa_status_t sli_se_driver_mac_compute(sl_se_key_descriptor_t *key_desc,
                                       psa_algorithm_t alg,
                                       const uint8_t *input,
                                       size_t input_length,
                                       uint8_t *mac,
                                       size_t mac_size,
                                       size_t *mac_length)
{
  #if defined(SLI_PSA_DRIVER_FEATURE_MAC)

  if (mac == NULL
      || mac_length == NULL
      || key_desc == NULL) {
    return PSA_ERROR_INVALID_ARGUMENT;
  }

  sl_status_t status;
  psa_status_t psa_status = PSA_ERROR_CORRUPTION_DETECTED;
  sl_se_command_context_t cmd_ctx = { 0 };

  status = sl_se_init_command_context(&cmd_ctx);
  if (status != SL_STATUS_OK) {
    return PSA_ERROR_HARDWARE_FAILURE;
  }

  #if defined(SLI_PSA_DRIVER_FEATURE_HMAC)
  if (PSA_ALG_IS_HMAC(alg)) {
    #if defined(SLI_PSA_DRIVER_FEATURE_HASH_STATE_64)
    uint8_t tmp_hmac[64];
    #else
    uint8_t tmp_hmac[32];
    #endif

    size_t requested_length = 0;
    sl_se_hash_type_t hash_type =
      sli_se_hash_type_from_psa_hmac_alg(alg, &requested_length);
    if (hash_type == SL_SE_HASH_NONE) {
      return PSA_ERROR_NOT_SUPPORTED;
    }

    if (PSA_MAC_TRUNCATED_LENGTH(alg) > requested_length) {
      return PSA_ERROR_INVALID_ARGUMENT;
    }

    if (PSA_MAC_TRUNCATED_LENGTH(alg) > 0) {
      requested_length = PSA_MAC_TRUNCATED_LENGTH(alg);
    }

    if (mac_size < requested_length) {
      return PSA_ERROR_BUFFER_TOO_SMALL;
    }

    #if defined(SLI_SE_KEY_PADDING_REQUIRED)
    uint8_t *temp_key_buf = NULL;
    uint32_t key_buffer_size = key_desc->storage.location.buffer.size;
    size_t padding = sli_se_get_padding(key_buffer_size);
    size_t word_aligned_buffer_size = 0;

    if (padding > 0u) {
      // We can only manipulate the transparent keys.
      if (key_desc->storage.method == SL_SE_KEY_STORAGE_EXTERNAL_PLAINTEXT) {
        word_aligned_buffer_size
          = sli_se_word_align(key_desc->storage.location.buffer.size);
        temp_key_buf = mbedtls_calloc(1, word_aligned_buffer_size);
        if (temp_key_buf == NULL) {
          return PSA_ERROR_INSUFFICIENT_MEMORY;
        }

        // Since we know that this must be a plaintext key, we can freely
        // modify the key descriptor
        memcpy(temp_key_buf,
               key_desc->storage.location.buffer.pointer,
               key_desc->storage.location.buffer.size);
        key_desc->storage.location.buffer.pointer = temp_key_buf;
        key_desc->storage.location.buffer.size = word_aligned_buffer_size;
      }
    }
    #endif     // SLI_SE_KEY_PADDING_REQUIRED

    status = sl_se_hmac(&cmd_ctx,
                        key_desc,
                        hash_type,
                        input,
                        input_length,
                        tmp_hmac,
                        sizeof(tmp_hmac));

    #if defined(SLI_SE_KEY_PADDING_REQUIRED)
    if (padding > 0u) {
      sli_psa_zeroize(temp_key_buf, word_aligned_buffer_size);
      mbedtls_free(temp_key_buf);
    }
    #endif     // SLI_SE_KEY_PADDING_REQUIRED

    if (status == PSA_SUCCESS) {
      memcpy(mac, tmp_hmac, requested_length);
      *mac_length = requested_length;
    } else {
      *mac_length = 0;
    }

    sli_psa_zeroize(tmp_hmac, sizeof(tmp_hmac));

    goto exit;
  }
  #endif   // SLI_PSA_DRIVER_FEATURE_HMAC

  #if defined(SLI_PSA_DRIVER_FEATURE_HMAC) \
  && (defined(SLI_PSA_DRIVER_FEATURE_CMAC) \
  || defined(SLI_PSA_DRIVER_FEATURE_CBC_MAC))
  else
  #endif

  #if defined(SLI_PSA_DRIVER_FEATURE_CMAC) || defined(SLI_PSA_DRIVER_FEATURE_CBC_MAC)
  {
    size_t output_length = PSA_MAC_TRUNCATED_LENGTH(alg);
    if (output_length == 0) {
      output_length = 16;
    } else if (output_length > 16) {
      return PSA_ERROR_INVALID_ARGUMENT;
    }
    if (mac_size < output_length) {
      return PSA_ERROR_BUFFER_TOO_SMALL;
    }

    switch (PSA_ALG_FULL_LENGTH_MAC(alg)) {
      #if defined(SLI_PSA_DRIVER_FEATURE_CBC_MAC)
      case PSA_ALG_CBC_MAC: {
        uint8_t tmp_buf[16] = { 0 };
        uint8_t tmp_mac[16] = { 0 };

        if (input_length % 16 != 0 || input_length < 16) {
          return PSA_ERROR_INVALID_ARGUMENT;
        }

        // Do an AES-CBC encrypt with zero IV, keeping only the last block.
        while (input_length > 0) {
          status = sl_se_aes_crypt_cbc(&cmd_ctx,
                                       key_desc,
                                       SL_SE_ENCRYPT,
                                       16,
                                       tmp_mac,
                                       input,
                                       tmp_buf);

          input_length -= 16;
          input += 16;
        }

        // Copy the requested number of bytes (max 16) to the user buffer.
        if (status == SL_STATUS_OK) {
          memcpy(mac, tmp_mac, output_length);
          sli_psa_zeroize(tmp_mac, sizeof(tmp_mac));
          *mac_length = output_length;
        }

        goto exit;
        break;
      }
      #endif     // SLI_PSA_DRIVER_FEATURE_CBC_MAC

      #if defined(SLI_PSA_DRIVER_FEATURE_CMAC)
      case PSA_ALG_CMAC: {
        uint8_t tmp_mac[16] = { 0 };

        status = sl_se_cmac(&cmd_ctx,
                            key_desc,
                            input,
                            input_length,
                            tmp_mac);

        // Copy the requested number of bytes (max 16) to the user buffer.
        if (status == SL_STATUS_OK) {
          memcpy(mac, tmp_mac, output_length);
          sli_psa_zeroize(tmp_mac, sizeof(tmp_mac));
          *mac_length = output_length;
        }

        goto exit;
        break;
      }
      #endif     // SLI_PSA_DRIVER_FEATURE_CMAC

      default:
        return PSA_ERROR_NOT_SUPPORTED;
        break;
    }
  }
  #endif   // SLI_PSA_DRIVER_FEATURE_CMAC || SLI_PSA_DRIVER_FEATURE_CBC_MAC

  exit:

  if (status == SL_STATUS_INVALID_PARAMETER) {
    psa_status = PSA_ERROR_INVALID_ARGUMENT;
  } else if (status != SL_STATUS_OK) {
    psa_status = PSA_ERROR_HARDWARE_FAILURE;
  } else {
    psa_status = PSA_SUCCESS;
  }

  // Cleanup
  status = sl_se_deinit_command_context(&cmd_ctx);
  if (status != SL_STATUS_OK) {
    return PSA_ERROR_HARDWARE_FAILURE;
  }

  return psa_status;

  #else // SLI_PSA_DRIVER_FEATURE_MAC

  (void)key_desc;
  (void)alg;
  (void)input;
  (void)input_length;
  (void)mac;
  (void)mac_size;
  (void)mac_length;

  return PSA_ERROR_NOT_SUPPORTED;

  #endif // SLI_PSA_DRIVER_FEATURE_MAC
}

//------------------------------------------------------------------------------
// Multi-part driver entry points

#if defined(SLI_PSA_DRIVER_FEATURE_CMAC) || defined(SLI_PSA_DRIVER_FEATURE_CBC_MAC)

psa_status_t sli_se_driver_mac_sign_setup(
  sli_se_driver_mac_operation_t *operation,
  const psa_key_attributes_t *attributes,
  psa_algorithm_t alg)
{
  if (operation == NULL
      || attributes == NULL) {
    return PSA_ERROR_INVALID_ARGUMENT;
  }

  // Start by resetting context
  memset(operation, 0, sizeof(*operation));

  switch (PSA_ALG_FULL_LENGTH_MAC(alg)) {
    #if defined(SLI_PSA_DRIVER_FEATURE_CBC_MAC)
    case PSA_ALG_CBC_MAC:
      if (psa_get_key_type(attributes) != PSA_KEY_TYPE_AES) {
        return PSA_ERROR_NOT_SUPPORTED;
      }
      if (PSA_MAC_TRUNCATED_LENGTH(alg) > 16) {
        return PSA_ERROR_INVALID_ARGUMENT;
      }
      break;
    #endif // SLI_PSA_DRIVER_FEATURE_CBC_MAC

    #if defined(SLI_PSA_DRIVER_FEATURE_CMAC)
    case PSA_ALG_CMAC:
      if (psa_get_key_type(attributes) != PSA_KEY_TYPE_AES) {
        return PSA_ERROR_NOT_SUPPORTED;
      }
      if (PSA_MAC_TRUNCATED_LENGTH(alg) > 16) {
        return PSA_ERROR_INVALID_ARGUMENT;
      }
      break;
    #endif // SLI_PSA_DRIVER_FEATURE_CMAC

    default:
      return PSA_ERROR_NOT_SUPPORTED;
  }

  operation->alg = alg;
  return PSA_SUCCESS;
}

#endif // SLI_PSA_DRIVER_FEATURE_CMAC || SLI_PSA_DRIVER_FEATURE_CBC_MAC

#if defined(SLI_PSA_DRIVER_FEATURE_CMAC) || defined(SLI_PSA_DRIVER_FEATURE_CBC_MAC) || (defined(SLI_PSA_DRIVER_FEATURE_HMAC) && defined(_SILICON_LABS_32B_SERIES_3))

psa_status_t sli_se_driver_mac_update(sli_se_driver_mac_operation_t *operation,
                                      sl_se_key_descriptor_t *key_desc,
                                      const uint8_t *input,
                                      size_t input_length)
{
  #if defined(SLI_PSA_DRIVER_FEATURE_MAC_MULTIPART)

  if (operation == NULL
      || (input == NULL && input_length > 0)
      || key_desc == NULL) {
    return PSA_ERROR_INVALID_ARGUMENT;
  }

  if (input_length == 0) {
    return PSA_SUCCESS;
  }

  sl_status_t status;
  psa_status_t psa_status = PSA_ERROR_CORRUPTION_DETECTED;

  // Ephemeral contexts
  sl_se_command_context_t cmd_ctx = { 0 };

  status = sl_se_init_command_context(&cmd_ctx);
  if (status != SL_STATUS_OK) {
    return PSA_ERROR_HARDWARE_FAILURE;
  }

  #if defined(SLI_PSA_DRIVER_FEATURE_HMAC) && defined(_SILICON_LABS_32B_SERIES_3)
  if (PSA_ALG_IS_HMAC(operation->alg)) {
    size_t hmac_state_len = 0;
    size_t requested_length = 0;
    sl_se_hash_type_t hash_type = sli_se_hash_type_from_psa_hmac_alg(operation->alg,
                                                                     &requested_length);
    if (hash_type == SL_SE_HASH_NONE) {
      return PSA_ERROR_NOT_SUPPORTED;
    }

    if (requested_length > sizeof(operation->ctx.hmac.hmac_result)) {
      return PSA_ERROR_NOT_SUPPORTED;
    }

    size_t block_size = 0u;
    switch (hash_type) {
      case SL_SE_HASH_SHA1:
      {
        block_size = 64u;
        hmac_state_len = 28u;
        break;
      }
      case SL_SE_HASH_SHA224:
      case SL_SE_HASH_SHA256:
      {
        block_size = 64u;
        hmac_state_len = 40u;
        break;
      }
      case SL_SE_HASH_SHA384:
      case SL_SE_HASH_SHA512:
      {
        block_size = 128u;
        hmac_state_len = 72u;
        break;
      }
      default:
      {
        return PSA_ERROR_NOT_SUPPORTED;
      }
    }

    size_t offset = 0u;
    // If there is a partial data in the buffer, fill it up to a block size
    if (operation->ctx.hmac.buffer_length > 0) {
      size_t to_copy = block_size - operation->ctx.hmac.buffer_length;
      if (input_length < to_copy) {
        memcpy(operation->ctx.hmac.buffer + operation->ctx.hmac.buffer_length,
               input,
               input_length);
        operation->ctx.hmac.buffer_length += input_length;
        return PSA_SUCCESS;
      } else {
        memcpy(operation->ctx.hmac.buffer + operation->ctx.hmac.buffer_length,
               input,
               to_copy);
        if (operation->ctx.hmac.processed_msg_len == 0) {
          status = sl_se_hmac_multipart_starts(&cmd_ctx, key_desc, hash_type,
                                               (const unsigned char *)operation->ctx.hmac.buffer, block_size,
                                               operation->ctx.hmac.hmac_state_buffer, hmac_state_len);
        } else {
          status = sl_se_hmac_multipart_update(&cmd_ctx, hash_type,
                                               (const unsigned char *)operation->ctx.hmac.buffer, block_size,
                                               operation->ctx.hmac.hmac_state_buffer, hmac_state_len);
        }
        if (status != SL_STATUS_OK) {
          goto exit;
        }
        operation->ctx.hmac.processed_msg_len += block_size;
        operation->ctx.hmac.buffer_length = 0;
        offset += to_copy;
      }
    }

    // Now we can process the full blocks of data from the input
    while (offset + block_size <= input_length) {
      size_t blocks_to_process = (input_length - offset) / block_size;
      size_t data_to_process = blocks_to_process * block_size;

      if (operation->ctx.hmac.processed_msg_len == 0) {
        status = sl_se_hmac_multipart_starts(&cmd_ctx, key_desc, hash_type,
                                             (const unsigned char *)(input + offset), data_to_process,
                                             operation->ctx.hmac.hmac_state_buffer, hmac_state_len);
      } else {
        status = sl_se_hmac_multipart_update(&cmd_ctx, hash_type,
                                             (const unsigned char *)(input + offset), data_to_process,
                                             operation->ctx.hmac.hmac_state_buffer, hmac_state_len);
      }
      if (status != SL_STATUS_OK) {
        goto exit;
      }
      operation->ctx.hmac.processed_msg_len += data_to_process;
      offset += data_to_process;
    }

    // Store remaining bytes in buffer
    if (offset < input_length) {
      operation->ctx.hmac.buffer_length = input_length - offset;
      memcpy(operation->ctx.hmac.buffer, input + offset, operation->ctx.hmac.buffer_length);
    }

    goto exit;
  }
  #endif // SLI_PSA_DRIVER_FEATURE_HMAC && defined(_SILICON_LABS_32B_SERIES_3)

  #if defined(SLI_PSA_DRIVER_FEATURE_CMAC) || defined(SLI_PSA_DRIVER_FEATURE_CBC_MAC)
  switch (PSA_ALG_FULL_LENGTH_MAC(operation->alg)) {
    #if defined(SLI_PSA_DRIVER_FEATURE_CBC_MAC)
    case PSA_ALG_CBC_MAC:
      // Add bytes to the streaming buffer up to the next block boundary
      if (operation->ctx.cbcmac.processed_length % 16 != 0) {
        size_t bytes_to_boundary
          = 16 - operation->ctx.cbcmac.processed_length % 16;
        if (input_length < bytes_to_boundary) {
          memcpy(&operation->ctx.cbcmac.streaming_block[16 - bytes_to_boundary],
                 input,
                 input_length);
          operation->ctx.cbcmac.processed_length += input_length;
          return PSA_SUCCESS;
        }

        memcpy(&operation->ctx.cbcmac.streaming_block[16 - bytes_to_boundary],
               input,
               bytes_to_boundary);
        input_length -= bytes_to_boundary;
        input += bytes_to_boundary;
        operation->ctx.cbcmac.processed_length += bytes_to_boundary;

        status = sl_se_aes_crypt_cbc(&cmd_ctx,
                                     key_desc,
                                     SL_SE_ENCRYPT,
                                     16,
                                     operation->ctx.cbcmac.iv,
                                     operation->ctx.cbcmac.streaming_block,
                                     operation->ctx.cbcmac.iv);

        if (status != SL_STATUS_OK) {
          goto exit;
        }
      }

      // Draw all full blocks
      while (input_length >= 16) {
        status = sl_se_aes_crypt_cbc(&cmd_ctx,
                                     key_desc,
                                     SL_SE_ENCRYPT,
                                     16,
                                     operation->ctx.cbcmac.iv,
                                     input,
                                     operation->ctx.cbcmac.iv);

        if (status != SL_STATUS_OK) {
          goto exit;
        }

        operation->ctx.cbcmac.processed_length += 16;
        input += 16;
        input_length -= 16;
      }

      if (input_length > 0) {
        memcpy(operation->ctx.cbcmac.streaming_block,
               input,
               input_length);
        operation->ctx.cbcmac.processed_length += input_length;
      }

      goto exit;
    #endif // SLI_PSA_DRIVER_FEATURE_CBC_MAC

    #if defined(SLI_PSA_DRIVER_FEATURE_CMAC)
    case PSA_ALG_CMAC:
      status = sl_se_cmac_multipart_update(&operation->ctx.cmac,
                                           &cmd_ctx,
                                           key_desc,
                                           input,
                                           input_length);
      if (status != SL_STATUS_OK) {
        goto exit;
      }
      break;
    #endif // SLI_PSA_DRIVER_FEATURE_CMAC

    default:
      return PSA_ERROR_BAD_STATE;
  }
  #endif // SLI_PSA_DRIVER_FEATURE_CMAC || SLI_PSA_DRIVER_FEATURE_CBC_MAC

  exit:

  if (status == SL_STATUS_INVALID_PARAMETER) {
    psa_status = PSA_ERROR_INVALID_ARGUMENT;
  } else if (status != SL_STATUS_OK) {
    psa_status = PSA_ERROR_HARDWARE_FAILURE;
  } else {
    psa_status = PSA_SUCCESS;
  }

  // Cleanup
  status = sl_se_deinit_command_context(&cmd_ctx);
  if (status != SL_STATUS_OK) {
    return PSA_ERROR_HARDWARE_FAILURE;
  }

  return psa_status;

  #else // SLI_PSA_DRIVER_FEATURE_MAC_MULTIPART
  (void)operation;
  (void)key_desc;
  (void)input;
  (void)input_length;

  return PSA_ERROR_NOT_SUPPORTED;
  #endif // SLI_PSA_DRIVER_FEATURE_MAC_MULTIPART
}

psa_status_t sli_se_driver_mac_sign_finish(
  sli_se_driver_mac_operation_t *operation,
  sl_se_key_descriptor_t *key_desc,
  uint8_t *mac,
  size_t mac_size,
  size_t *mac_length)
{
  #if defined(SLI_PSA_DRIVER_FEATURE_MAC_MULTIPART)

  if (operation == NULL
      || mac == NULL
      || mac_size == 0
      || mac_length == NULL
      || key_desc == NULL) {
    return PSA_ERROR_INVALID_ARGUMENT;
  }

  sl_status_t status;
  psa_status_t psa_status = PSA_ERROR_CORRUPTION_DETECTED;

  // Ephemeral contexts
  sl_se_command_context_t cmd_ctx = { 0 };
  status = sl_se_init_command_context(&cmd_ctx);
  if (status != SL_STATUS_OK) {
    return PSA_ERROR_HARDWARE_FAILURE;
  }

  #if defined(SLI_PSA_DRIVER_FEATURE_HMAC) && defined(_SILICON_LABS_32B_SERIES_3)
  if (PSA_ALG_IS_HMAC(operation->alg)) {
    #if defined(SLI_PSA_DRIVER_FEATURE_HASH_STATE_64)
    uint8_t tmp_hmac[64];
    #else
    uint8_t tmp_hmac[32];
    #endif

    size_t requested_length = 0;
    sl_se_hash_type_t hash_type =
      sli_se_hash_type_from_psa_hmac_alg(operation->alg, &requested_length);
    if (hash_type == SL_SE_HASH_NONE) {
      return PSA_ERROR_NOT_SUPPORTED;
    }

    if (PSA_MAC_TRUNCATED_LENGTH(operation->alg) > requested_length) {
      return PSA_ERROR_INVALID_ARGUMENT;
    }

    if (PSA_MAC_TRUNCATED_LENGTH(operation->alg) > 0) {
      requested_length = PSA_MAC_TRUNCATED_LENGTH(operation->alg);
    }

    if (mac_size < requested_length) {
      return PSA_ERROR_BUFFER_TOO_SMALL;
    }

    size_t hmac_state_len = 0;
    switch (hash_type) {
      case SL_SE_HASH_SHA1:
      {
        hmac_state_len = 28u;
        break;
      }
      case SL_SE_HASH_SHA224:
      case SL_SE_HASH_SHA256:
      {
        hmac_state_len = 40u;
        break;
      }
      case SL_SE_HASH_SHA384:
      case SL_SE_HASH_SHA512:
      {
        hmac_state_len = 72u;
        break;
      }
      default:
      {
        return PSA_ERROR_NOT_SUPPORTED;
      }
    }

    if (operation->ctx.hmac.processed_msg_len == 0) {
      status = sli_se_driver_mac_compute(key_desc,
                                         operation->alg,
                                         operation->ctx.hmac.buffer,
                                         operation->ctx.hmac.buffer_length,
                                         tmp_hmac,
                                         sizeof(tmp_hmac),
                                         mac_length);
    } else {
      status = sl_se_hmac_multipart_finish(&cmd_ctx, key_desc, hash_type,
                                           (const unsigned char *)&operation->ctx.hmac.buffer, operation->ctx.hmac.buffer_length,
                                           operation->ctx.hmac.hmac_state_buffer, hmac_state_len,
                                           tmp_hmac, sizeof(tmp_hmac));
    }

    if (status != SL_STATUS_OK) {
      *mac_length = 0;
      goto exit;
    }
    operation->ctx.hmac.processed_msg_len = 0;
    operation->ctx.hmac.buffer_length = 0;

    memcpy(mac, tmp_hmac, requested_length);
    *mac_length = requested_length;

    goto exit;
  }
  #endif // SLI_PSA_DRIVER_FEATURE_HMAC && defined(_SILICON_LABS_32B_SERIES_3)

  #if defined(SLI_PSA_DRIVER_FEATURE_CMAC) || defined(SLI_PSA_DRIVER_FEATURE_CBC_MAC)
  // Set maximum output size to 16 or truncated length
  if (mac_size > 16) {
    mac_size = 16;
  }

  size_t truncated_length = PSA_MAC_TRUNCATED_LENGTH(operation->alg);
  if (truncated_length != 0
      && mac_size > truncated_length) {
    mac_size = truncated_length;
  }

  switch (PSA_ALG_FULL_LENGTH_MAC(operation->alg)) {
    #if defined(SLI_PSA_DRIVER_FEATURE_CBC_MAC)
    case PSA_ALG_CBC_MAC: {
      (void)key_desc;
      if (operation->ctx.cbcmac.processed_length % 16 != 0) {
        return PSA_ERROR_BAD_STATE;
      }

      // Copy the requested number of bytes (max 16) to the user buffer.
      memcpy(mac, operation->ctx.cbcmac.iv, mac_size);
      *mac_length = mac_size;
      status = SL_STATUS_OK;
      goto exit;
      break;
    }
    #endif // SLI_PSA_DRIVER_FEATURE_CBC_MAC

    #if defined(SLI_PSA_DRIVER_FEATURE_CMAC)
    case PSA_ALG_CMAC: {
      // Ephemeral contexts
      memset(&cmd_ctx, 0, sizeof(sl_se_command_context_t));
      uint8_t tmp_mac[16] = { 0 };

      status = sl_se_cmac_multipart_finish(&operation->ctx.cmac,
                                           &cmd_ctx,
                                           key_desc,
                                           tmp_mac);
      if (status != SL_STATUS_OK) {
        *mac_length = 0;
        goto exit;
      }

      // Copy the requested number of bytes (max 16) to the user buffer.
      memcpy(mac, tmp_mac, mac_size);
      *mac_length = mac_size;
      goto exit;
      break;
    }
    #endif // SLI_PSA_DRIVER_FEATURE_CMAC

    default:
      return PSA_ERROR_BAD_STATE;
  }
  #endif // SLI_PSA_DRIVER_FEATURE_CMAC || SLI_PSA_DRIVER_FEATURE_CBC_MAC

  exit:

  if (status == SL_STATUS_INVALID_PARAMETER) {
    psa_status = PSA_ERROR_INVALID_ARGUMENT;
  } else if (status != SL_STATUS_OK) {
    psa_status = PSA_ERROR_HARDWARE_FAILURE;
  } else {
    psa_status = PSA_SUCCESS;
  }

  // Cleanup
  status = sl_se_deinit_command_context(&cmd_ctx);
  if (status != SL_STATUS_OK) {
    return PSA_ERROR_HARDWARE_FAILURE;
  }

  return psa_status;

  #else // SLI_PSA_DRIVER_FEATURE_MAC_MULTIPART

  (void)operation;
  (void)key_desc;
  (void)mac;
  (void)mac_size;
  (void)mac_length;

  return PSA_ERROR_NOT_SUPPORTED;

  #endif // SLI_PSA_DRIVER_FEATURE_MAC_MULTIPART
}

#endif // SLI_PSA_DRIVER_FEATURE_CMAC || SLI_PSA_DRIVER_FEATURE_CBC_MAC || (defined(SLI_PSA_DRIVER_FEATURE_HMAC) && defined(_SILICON_LABS_32B_SERIES_3))

#endif // SLI_MBEDTLS_DEVICE_HSE
