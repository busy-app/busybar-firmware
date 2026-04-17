/***************************************************************************/ /**
 * @file
 * @brief SL SI91X ECDH source file
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
#include "sl_si91x_ecdh.h"
#include "sl_si91x_protocol_types.h"
#include "sl_constants.h"
#include "sl_si91x_driver.h"
#include "sl_utility.h"
#include "sli_wifi_utility.h"
#if defined(SLI_MULTITHREAD_DEVICE_SI91X)
#include "sl_si91x_crypto_thread.h"
#endif
#include <string.h>
#include "sli_wifi_utility.h"
static sl_status_t sli_si91x_get_size_from_ecdh_mode(sl_si91x_ecdh_mode_t ecdh_mode, uint8_t *size)
{
  switch (ecdh_mode) {
    case SL_SI91X_ECDH_192:
      *size = SL_SI91X_ECDH_VECTOR_SIZE_192;
      break;
    case SL_SI91X_ECDH_224:
      *size = SL_SI91X_ECDH_VECTOR_SIZE_224;
      break;
    case SL_SI91X_ECDH_256:
      *size = SL_SI91X_ECDH_VECTOR_SIZE_256;
      break;
    default:
      return SL_STATUS_INVALID_PARAMETER;
  }
  return SL_STATUS_OK;
}

#ifndef SL_SI91X_SIDE_BAND_CRYPTO
static void sli_si91x_ecdh_get_data_from_buffer(sl_wifi_buffer_t *buffer, uint8_t *rx, uint8_t *ry, uint8_t *rz)
{
  const uint8_t *result                 = NULL;
  uint16_t offset                       = 0;
  const sl_wifi_system_packet_t *packet = NULL;

  packet = (sl_wifi_system_packet_t *)sli_wifi_host_get_buffer_data(buffer, 0, NULL);
  result = packet->data;
  memcpy(rx, result, SL_SI91X_ECDH_MAX_VECTOR_SIZE);
  offset += SL_SI91X_ECDH_MAX_VECTOR_SIZE;
  memcpy(ry, (result + offset), SL_SI91X_ECDH_MAX_VECTOR_SIZE);
  offset += SL_SI91X_ECDH_MAX_VECTOR_SIZE;
  memcpy(rz, (result + offset), SL_SI91X_ECDH_MAX_VECTOR_SIZE);
}
#endif

static sl_status_t sli_si91x_ecdh_add_sub(sl_si91x_ecdh_mode_t ecdh_mode,
                                          sl_si91x_ecdh_sub_mode_t ecdh_sub_mode,
                                          const uint8_t *sx,
                                          const uint8_t *sy,
                                          const uint8_t *sz,
                                          const uint8_t *tx,
                                          const uint8_t *ty,
                                          const uint8_t *tz,
                                          uint8_t *rx,
                                          uint8_t *ry,
                                          uint8_t *rz)
{
  uint8_t size             = 0;
  sl_status_t status       = SL_STATUS_FAIL;
  sl_wifi_buffer_t *buffer = NULL;

  SL_VERIFY_POINTER_OR_RETURN(sx, SL_STATUS_NULL_POINTER);
  SL_VERIFY_POINTER_OR_RETURN(sy, SL_STATUS_NULL_POINTER);
  SL_VERIFY_POINTER_OR_RETURN(sz, SL_STATUS_NULL_POINTER);
  SL_VERIFY_POINTER_OR_RETURN(tx, SL_STATUS_NULL_POINTER);
  SL_VERIFY_POINTER_OR_RETURN(ty, SL_STATUS_NULL_POINTER);
  SL_VERIFY_POINTER_OR_RETURN(tz, SL_STATUS_NULL_POINTER);
  SL_VERIFY_POINTER_OR_RETURN(rx, SL_STATUS_NULL_POINTER);
  SL_VERIFY_POINTER_OR_RETURN(ry, SL_STATUS_NULL_POINTER);
  SL_VERIFY_POINTER_OR_RETURN(rz, SL_STATUS_NULL_POINTER);

  sli_si91x_ecdh_add_sub_request_t *request =
    (sli_si91x_ecdh_add_sub_request_t *)malloc(sizeof(sli_si91x_ecdh_add_sub_request_t));
  SL_VERIFY_POINTER_OR_RETURN(request, SL_STATUS_ALLOCATION_FAILED);

  memset(request, 0, sizeof(sli_si91x_ecdh_add_sub_request_t));

  status = sli_si91x_get_size_from_ecdh_mode(ecdh_mode, &size);
  if (status != SL_STATUS_OK) {
    free(request);
    return status;
  }

  request->algorithm_type = ECDH;
  request->ecdh_mode      = (uint8_t)ecdh_mode;
  request->ecdh_sub_mode  = (uint8_t)ecdh_sub_mode;
#ifdef SL_SI91X_SIDE_BAND_CRYPTO
  request->sx = (uint8_t *)sx;
  request->sy = (uint8_t *)sy;
  request->sz = (uint8_t *)sz;
  request->tx = (uint8_t *)tx;
  request->ty = (uint8_t *)ty;
  request->tz = (uint8_t *)tz;
  request->rx = (uint8_t *)rx;
  request->ry = (uint8_t *)ry;
  request->rz = (uint8_t *)rz;
#else
  memcpy(request->sx, sx, size);
  memcpy(request->sy, sy, size);
  memcpy(request->sz, sz, size);
  memcpy(request->tx, tx, size);
  memcpy(request->ty, ty, size);
  memcpy(request->tz, tz, size);
#endif

#if defined(SLI_MULTITHREAD_DEVICE_SI91X)
  if (crypto_ecdh_mutex == NULL) {
    crypto_ecdh_mutex = sl_si91x_crypto_threadsafety_init(crypto_ecdh_mutex);
  }
  mutex_result = sl_si91x_crypto_mutex_acquire(crypto_ecdh_mutex);
#endif

#ifdef SL_SI91X_SIDE_BAND_CRYPTO
  status = sl_si91x_driver_send_side_band_crypto(SLI_COMMON_REQ_ENCRYPT_CRYPTO,
                                                 request,
                                                 sizeof(sli_si91x_ecdh_add_sub_request_t),
                                                 SLI_WIFI_WAIT_FOR_RESPONSE(SLI_COMMON_RSP_ENCRYPT_CRYPTO_WAIT_TIME));
  if (status != SL_STATUS_OK) {
    free(request);
    if (buffer != NULL)
      sli_si91x_host_free_buffer(buffer);
  }
  VERIFY_STATUS_AND_RETURN(status);
#else
  status = sli_si91x_driver_send_command(SLI_COMMON_REQ_ENCRYPT_CRYPTO,
                                         SLI_WIFI_COMMON_CMD,
                                         request,
                                         sizeof(sli_si91x_ecdh_add_sub_request_t),
                                         SLI_WIFI_WAIT_FOR_RESPONSE(SLI_COMMON_RSP_ENCRYPT_CRYPTO_WAIT_TIME),
                                         NULL,
                                         &buffer);
  if (status != SL_STATUS_OK) {
    free(request);
    if (buffer != NULL)
      sli_si91x_host_free_buffer(buffer);
#if defined(SLI_MULTITHREAD_DEVICE_SI91X)
    mutex_result = sl_si91x_crypto_mutex_release(crypto_ecdh_mutex);
#endif
  }
  VERIFY_STATUS_AND_RETURN(status);

  sli_si91x_ecdh_get_data_from_buffer(buffer, rx, ry, rz);
#endif

  sli_si91x_host_free_buffer(buffer);
  free(request);
#if defined(SLI_MULTITHREAD_DEVICE_SI91X)
  mutex_result = sl_si91x_crypto_mutex_release(crypto_ecdh_mutex);
#endif
  return status;
}

sl_status_t sl_si91x_ecdh_point_addition(sl_si91x_ecdh_mode_t ecdh_mode,
                                         const uint8_t *sx,
                                         const uint8_t *sy,
                                         const uint8_t *sz,
                                         const uint8_t *tx,
                                         const uint8_t *ty,
                                         const uint8_t *tz,
                                         uint8_t *rx,
                                         uint8_t *ry,
                                         uint8_t *rz)
{
  return sli_si91x_ecdh_add_sub(ecdh_mode, SL_SI91X_ECDH_ADD, sx, sy, sz, tx, ty, tz, rx, ry, rz);
}

sl_status_t sl_si91x_ecdh_point_subtraction(sl_si91x_ecdh_mode_t ecdh_mode,
                                            const uint8_t *sx,
                                            const uint8_t *sy,
                                            const uint8_t *sz,
                                            const uint8_t *tx,
                                            const uint8_t *ty,
                                            const uint8_t *tz,
                                            uint8_t *rx,
                                            uint8_t *ry,
                                            uint8_t *rz)
{
  return sli_si91x_ecdh_add_sub(ecdh_mode, SL_SI91X_ECDH_SUB, sx, sy, sz, tx, ty, tz, rx, ry, rz);
}

sl_status_t sl_si91x_ecdh_point_multiplication(sl_si91x_ecdh_mode_t ecdh_mode,
                                               const uint8_t *d,
                                               const uint8_t *sx,
                                               const uint8_t *sy,
                                               const uint8_t *sz,
                                               uint32_t affinity,
                                               uint8_t *rx,
                                               uint8_t *ry,
                                               uint8_t *rz,
                                               uint8_t reverse)
{

  uint8_t size             = 0;
  sl_status_t status       = SL_STATUS_FAIL;
  sl_wifi_buffer_t *buffer = NULL;

  SL_VERIFY_POINTER_OR_RETURN(d, SL_STATUS_NULL_POINTER);
  SL_VERIFY_POINTER_OR_RETURN(sx, SL_STATUS_NULL_POINTER);
  SL_VERIFY_POINTER_OR_RETURN(sy, SL_STATUS_NULL_POINTER);
  SL_VERIFY_POINTER_OR_RETURN(sz, SL_STATUS_NULL_POINTER);
  SL_VERIFY_POINTER_OR_RETURN(rx, SL_STATUS_NULL_POINTER);
  SL_VERIFY_POINTER_OR_RETURN(ry, SL_STATUS_NULL_POINTER);
  SL_VERIFY_POINTER_OR_RETURN(rz, SL_STATUS_NULL_POINTER);

  sli_si91x_ecdh_mul_request_t *request = (sli_si91x_ecdh_mul_request_t *)malloc(sizeof(sli_si91x_ecdh_mul_request_t));
  SL_VERIFY_POINTER_OR_RETURN(request, SL_STATUS_ALLOCATION_FAILED);

  memset(request, 0, sizeof(sli_si91x_ecdh_mul_request_t));

  status = sli_si91x_get_size_from_ecdh_mode(ecdh_mode, &size);
  if (status != SL_STATUS_OK) {
    free(request);
    return status;
  }

  request->algorithm_type  = ECDH;
  request->ecdh_mode       = (uint8_t)ecdh_mode;
  request->ecdh_sub_mode   = SL_SI91X_ECDH_MUL;
  request->ecdh_curve_type = SL_SI91X_ECDH_CURVE_P;
  request->affinity        = affinity;
#ifdef SL_SI91X_SIDE_BAND_CRYPTO
  request->d  = (uint8_t *)d;
  request->sx = (uint8_t *)sx;
  request->sy = (uint8_t *)sy;
  request->sz = (uint8_t *)sz;
  request->rx = (uint8_t *)rx;
  request->ry = (uint8_t *)ry;
  request->rz = (uint8_t *)rz;
#else
  memcpy(request->d, d, size);
  memcpy(request->sx, sx, size);
  memcpy(request->sy, sy, size);
  memcpy(request->sz, sz, size);
#endif
  if (reverse) {
    sli_reverse_digits(request->sx, size);
    sli_reverse_digits(request->sy, size);
    sli_reverse_digits(request->sz, size);
    sli_reverse_digits(request->d, size);
  }

#if defined(SLI_MULTITHREAD_DEVICE_SI91X)
  if (crypto_ecdh_mutex == NULL) {
    crypto_ecdh_mutex = sl_si91x_crypto_threadsafety_init(crypto_ecdh_mutex);
  }
  mutex_result = sl_si91x_crypto_mutex_acquire(crypto_ecdh_mutex);
#endif

#ifdef SL_SI91X_SIDE_BAND_CRYPTO
  status = sl_si91x_driver_send_side_band_crypto(SLI_COMMON_REQ_ENCRYPT_CRYPTO,
                                                 request,
                                                 (sizeof(sli_si91x_ecdh_mul_request_t)),
                                                 SLI_WIFI_WAIT_FOR_RESPONSE(SLI_COMMON_RSP_ENCRYPT_CRYPTO_WAIT_TIME));
  if (status != SL_STATUS_OK) {
    free(request);
    if (buffer != NULL)
      sli_si91x_host_free_buffer(buffer);
  }
  VERIFY_STATUS_AND_RETURN(status);
#else
  status = sli_si91x_driver_send_command(SLI_COMMON_REQ_ENCRYPT_CRYPTO,
                                         SLI_WIFI_COMMON_CMD,
                                         request,
                                         (sizeof(sli_si91x_ecdh_mul_request_t)),
                                         SLI_WIFI_WAIT_FOR_RESPONSE(SLI_COMMON_RSP_ENCRYPT_CRYPTO_WAIT_TIME),
                                         NULL,
                                         &buffer);
  if (status != SL_STATUS_OK) {
    free(request);
    if (buffer != NULL)
      sli_si91x_host_free_buffer(buffer);
#if defined(SLI_MULTITHREAD_DEVICE_SI91X)
    mutex_result = sl_si91x_crypto_mutex_release(crypto_ecdh_mutex);
#endif
  }
  VERIFY_STATUS_AND_RETURN(status);

  sli_si91x_ecdh_get_data_from_buffer(buffer, rx, ry, rz);
#endif

  if (reverse) {
    sli_reverse_digits(rx, size);
    sli_reverse_digits(ry, size);
    sli_reverse_digits(rz, size);
  }

  sli_si91x_host_free_buffer(buffer);
  free(request);
#if defined(SLI_MULTITHREAD_DEVICE_SI91X)
  mutex_result = sl_si91x_crypto_mutex_release(crypto_ecdh_mutex);
#endif
  return status;
}

sl_status_t sl_si91x_ecdh_point_double(sl_si91x_ecdh_mode_t ecdh_mode,
                                       const uint8_t *sx,
                                       const uint8_t *sy,
                                       const uint8_t *sz,
                                       uint8_t *rx,
                                       uint8_t *ry,
                                       uint8_t *rz)
{
  uint8_t size             = 0;
  sl_status_t status       = SL_STATUS_FAIL;
  sl_wifi_buffer_t *buffer = NULL;

  SL_VERIFY_POINTER_OR_RETURN(sx, SL_STATUS_NULL_POINTER);
  SL_VERIFY_POINTER_OR_RETURN(sy, SL_STATUS_NULL_POINTER);
  SL_VERIFY_POINTER_OR_RETURN(sz, SL_STATUS_NULL_POINTER);
  SL_VERIFY_POINTER_OR_RETURN(rx, SL_STATUS_NULL_POINTER);
  SL_VERIFY_POINTER_OR_RETURN(ry, SL_STATUS_NULL_POINTER);
  SL_VERIFY_POINTER_OR_RETURN(rz, SL_STATUS_NULL_POINTER);

  sli_si91x_ecdh_double_request_t *request =
    (sli_si91x_ecdh_double_request_t *)malloc(sizeof(sli_si91x_ecdh_double_request_t));
  SL_VERIFY_POINTER_OR_RETURN(request, SL_STATUS_ALLOCATION_FAILED);

  memset(request, 0, sizeof(sli_si91x_ecdh_double_request_t));

  status = sli_si91x_get_size_from_ecdh_mode(ecdh_mode, &size);
  if (status != SL_STATUS_OK) {
    free(request);
    return status;
  }

  request->algorithm_type = ECDH;
  request->ecdh_mode      = (uint8_t)ecdh_mode;
  request->ecdh_sub_mode  = SL_SI91X_ECDH_DOUBLE;

#ifdef SL_SI91X_SIDE_BAND_CRYPTO
  request->sx = (uint8_t *)sx;
  request->sy = (uint8_t *)sy;
  request->sz = (uint8_t *)sz;
  request->rx = (uint8_t *)rx;
  request->ry = (uint8_t *)ry;
  request->rz = (uint8_t *)rz;
#else
  memcpy(request->sx, sx, size);
  memcpy(request->sy, sy, size);
  memcpy(request->sz, sz, size);
#endif

#if defined(SLI_MULTITHREAD_DEVICE_SI91X)
  if (crypto_ecdh_mutex == NULL) {
    crypto_ecdh_mutex = sl_si91x_crypto_threadsafety_init(crypto_ecdh_mutex);
  }
  mutex_result = sl_si91x_crypto_mutex_acquire(crypto_ecdh_mutex);
#endif

#ifdef SL_SI91X_SIDE_BAND_CRYPTO

  status = sl_si91x_driver_send_side_band_crypto(SLI_COMMON_REQ_ENCRYPT_CRYPTO,
                                                 request,
                                                 (sizeof(sli_si91x_ecdh_double_request_t)),
                                                 SLI_WIFI_WAIT_FOR_RESPONSE(SLI_COMMON_RSP_ENCRYPT_CRYPTO_WAIT_TIME));
  if (status != SL_STATUS_OK) {
    free(request);
    if (buffer != NULL)
      sli_si91x_host_free_buffer(buffer);
  }
  VERIFY_STATUS_AND_RETURN(status);
#else
  status = sli_si91x_driver_send_command(SLI_COMMON_REQ_ENCRYPT_CRYPTO,
                                         SLI_WIFI_COMMON_CMD,
                                         request,
                                         (sizeof(sli_si91x_ecdh_double_request_t)),
                                         SLI_WIFI_WAIT_FOR_RESPONSE(SLI_COMMON_RSP_ENCRYPT_CRYPTO_WAIT_TIME),
                                         NULL,
                                         &buffer);
  if (status != SL_STATUS_OK) {
    free(request);
    if (buffer != NULL)
      sli_si91x_host_free_buffer(buffer);
#if defined(SLI_MULTITHREAD_DEVICE_SI91X)
    mutex_result = sl_si91x_crypto_mutex_release(crypto_ecdh_mutex);
#endif
  }
  VERIFY_STATUS_AND_RETURN(status);

  sli_si91x_ecdh_get_data_from_buffer(buffer, rx, ry, rz);
#endif

  sli_si91x_host_free_buffer(buffer);
  free(request);
#if defined(SLI_MULTITHREAD_DEVICE_SI91X)
  mutex_result = sl_si91x_crypto_mutex_release(crypto_ecdh_mutex);
#endif
  return status;
}

sl_status_t sl_si91x_ecdh_point_affine(sl_si91x_ecdh_mode_t ecdh_mode,
                                       const uint8_t *sx,
                                       const uint8_t *sy,
                                       const uint8_t *sz,
                                       uint8_t *rx,
                                       uint8_t *ry,
                                       uint8_t *rz)
{
  uint8_t size             = 0;
  sl_status_t status       = SL_STATUS_FAIL;
  sl_wifi_buffer_t *buffer = NULL;

  SL_VERIFY_POINTER_OR_RETURN(sx, SL_STATUS_NULL_POINTER);
  SL_VERIFY_POINTER_OR_RETURN(sy, SL_STATUS_NULL_POINTER);
  SL_VERIFY_POINTER_OR_RETURN(sz, SL_STATUS_NULL_POINTER);
  SL_VERIFY_POINTER_OR_RETURN(rx, SL_STATUS_NULL_POINTER);
  SL_VERIFY_POINTER_OR_RETURN(ry, SL_STATUS_NULL_POINTER);
  SL_VERIFY_POINTER_OR_RETURN(rz, SL_STATUS_NULL_POINTER);

  sli_si91x_ecdh_affine_request_t *request =
    (sli_si91x_ecdh_affine_request_t *)malloc(sizeof(sli_si91x_ecdh_affine_request_t));
  SL_VERIFY_POINTER_OR_RETURN(request, SL_STATUS_ALLOCATION_FAILED);

  memset(request, 0, sizeof(sli_si91x_ecdh_affine_request_t));

  status = sli_si91x_get_size_from_ecdh_mode(ecdh_mode, &size);
  if (status != SL_STATUS_OK) {
    free(request);
    return status;
  }

  request->algorithm_type  = ECDH;
  request->ecdh_mode       = (uint8_t)ecdh_mode;
  request->ecdh_sub_mode   = SL_SI91X_ECDH_AFFINITY;
  request->ecdh_curve_type = SL_SI91X_ECDH_CURVE_P;
#ifdef SL_SI91X_SIDE_BAND_CRYPTO
  memcpy(rx, sx, size);
  memcpy(ry, sy, size);
  memcpy(rz, sz, size);
  request->sx = rx;
  request->sy = ry;
  request->sz = rz;
  request->rx = rx;
  request->ry = ry;
  request->rz = rz;
#else
  memcpy(request->sx, sx, size);
  memcpy(request->sy, sy, size);
  memcpy(request->sz, sz, size);
#endif

#if defined(SLI_MULTITHREAD_DEVICE_SI91X)
  if (crypto_ecdh_mutex == NULL) {
    crypto_ecdh_mutex = sl_si91x_crypto_threadsafety_init(crypto_ecdh_mutex);
  }
  mutex_result = sl_si91x_crypto_mutex_acquire(crypto_ecdh_mutex);
#endif

#ifdef SL_SI91X_SIDE_BAND_CRYPTO
  status = sl_si91x_driver_send_side_band_crypto(SLI_COMMON_REQ_ENCRYPT_CRYPTO,
                                                 request,
                                                 (sizeof(sli_si91x_ecdh_affine_request_t)),
                                                 SLI_WIFI_WAIT_FOR_RESPONSE(SLI_COMMON_RSP_ENCRYPT_CRYPTO_WAIT_TIME));
  if (status != SL_STATUS_OK) {
    free(request);
    if (buffer != NULL)
      sli_si91x_host_free_buffer(buffer);
  }
  VERIFY_STATUS_AND_RETURN(status);
#else
  status = sli_si91x_driver_send_command(SLI_COMMON_REQ_ENCRYPT_CRYPTO,
                                         SLI_WIFI_COMMON_CMD,
                                         request,
                                         (sizeof(sli_si91x_ecdh_affine_request_t)),
                                         SLI_WIFI_WAIT_FOR_RESPONSE(SLI_COMMON_RSP_ENCRYPT_CRYPTO_WAIT_TIME),
                                         NULL,
                                         &buffer);
  if (status != SL_STATUS_OK) {
    free(request);
    if (buffer != NULL)
      sli_si91x_host_free_buffer(buffer);
#if defined(SLI_MULTITHREAD_DEVICE_SI91X)
    mutex_result = sl_si91x_crypto_mutex_release(crypto_ecdh_mutex);
#endif
  }
  VERIFY_STATUS_AND_RETURN(status);

  sli_si91x_ecdh_get_data_from_buffer(buffer, rx, ry, rz);
#endif

  sli_si91x_host_free_buffer(buffer);
  free(request);
#if defined(SLI_MULTITHREAD_DEVICE_SI91X)
  mutex_result = sl_si91x_crypto_mutex_release(crypto_ecdh_mutex);
#endif
  return status;
}
