/***************************************************************************/ /**
 * @file
 * @brief SL SI91x ATTESTATION API
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

#include "sl_si91x_crypto.h"
#include "sl_status.h"
#include "sl_constants.h"
#include "sl_si91x_protocol_types.h"
#include "sl_si91x_driver.h"
#include <string.h>
#include "sl_si91x_attestation.h"
#include "sli_wifi_utility.h"
#if defined(SLI_MULTITHREAD_DEVICE_SI91X)
#include "sl_si91x_crypto_thread.h"
#endif

/**
 * @fn    sl_status_t sl_si91x_attestation_get_token(uint8_t *token, uint16_t length, uint32_t *nonce)
 * @brief \Process the nonce for attestation token
 * @param[in] length
 * @param[in] nonce
 * @param[out] token
 * @return return sl_status code 
 *      
 */
sl_status_t sl_si91x_attestation_get_token(uint8_t *token, uint16_t length, uint32_t *nonce)
{
  if ((token == NULL) || (length == 0)) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  sl_status_t status;
  sl_wifi_buffer_t *buffer        = NULL;
  sl_wifi_system_packet_t *packet = NULL;

  sli_si91x_rsi_token_req_t *attest = (sli_si91x_rsi_token_req_t *)malloc(sizeof(sli_si91x_rsi_token_req_t));

  if (attest == NULL) {
    return SL_STATUS_ALLOCATION_FAILED;
  };

  // Memset before filling
  memset(attest, 0, sizeof(sli_si91x_rsi_token_req_t));

  // Fill Algorithm type ATTESTATION - 30
  attest->algorithm_type = ATTESTATION;

  // Update the length
  attest->total_msg_length = length;

#ifdef SL_SI91X_SIDE_BAND_CRYPTO
  UNUSED_VARIABLE(packet); // to avoid unused variable warning
  attest->msg       = nonce;
  attest->token_buf = token;
#else
  // Send 32 byte nonce
  //! Memset before filling
  memset(&attest->msg[0], 0, NONCE_DATA_SIZE);

  //! Copy Data
  memcpy(&attest->msg[0], nonce, NONCE_DATA_SIZE);
#endif

#if defined(SLI_MULTITHREAD_DEVICE_SI91X)
  if (crypto_attestation_mutex == NULL) {
    crypto_attestation_mutex = sl_si91x_crypto_threadsafety_init(crypto_attestation_mutex);
  }
  mutex_result = sl_si91x_crypto_mutex_acquire(crypto_attestation_mutex);
#endif

#ifdef SL_SI91X_SIDE_BAND_CRYPTO
  status = sl_si91x_driver_send_side_band_crypto(SLI_COMMON_REQ_ENCRYPT_CRYPTO,
                                                 attest,
                                                 (sizeof(sli_si91x_rsi_token_req_t)),
                                                 SLI_WIFI_WAIT_FOR_RESPONSE(SLI_COMMON_RSP_ENCRYPT_CRYPTO_WAIT_TIME));
  if (status != SL_STATUS_OK) {
    free(attest);
    if (buffer != NULL)
      sli_si91x_host_free_buffer(buffer);
  }
  VERIFY_STATUS_AND_RETURN(status);
#else
  status = sli_si91x_driver_send_command(SLI_COMMON_REQ_ENCRYPT_CRYPTO,
                                         SLI_WIFI_COMMON_CMD,
                                         attest,
                                         sizeof(sli_si91x_rsi_token_req_t),
                                         SLI_WIFI_WAIT_FOR_RESPONSE(SLI_COMMON_RSP_ENCRYPT_CRYPTO_WAIT_TIME),
                                         NULL,
                                         &buffer);
  if (status != SL_STATUS_OK) {
    free(attest);
    if (buffer != NULL)
      sli_si91x_host_free_buffer(buffer);
#if defined(SLI_MULTITHREAD_DEVICE_SI91X)
    mutex_result = sl_si91x_crypto_mutex_release(crypto_attestation_mutex);
#endif
    return status;
  }
  packet = (sl_wifi_system_packet_t *)sli_wifi_host_get_buffer_data(buffer, 0, NULL);

  memcpy(token, packet->data, packet->length);
#endif

  sli_si91x_host_free_buffer(buffer);
  free(attest);

#if defined(SLI_MULTITHREAD_DEVICE_SI91X)
  mutex_result = sl_si91x_crypto_mutex_release(crypto_attestation_mutex);
#endif

  return status;
}
