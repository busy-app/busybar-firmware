/***************************************************************************//**
 * @file sl_wisun_meter.c
 * @brief Wi-SUN Meter
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

// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "sl_assert.h"
#include "cmsis_os2.h"
#include "em_device.h"
#include "sl_status.h"
#include "sl_mempool.h"
#include "sl_string.h"
#include "socket/socket.h"
#include "sl_wisun_event_mgr.h"
#include "sl_wisun_app_core.h"
#include "sl_wisun_config.h"
#include "sl_wisun_trace_util.h"
#include "sl_wisun_meter.h"
#include "sli_wisun_meter_collector.h"
#include "sl_wisun_meter_collector_config.h"

#if defined(SL_CATALOG_TEMP_SENSOR_PRESENT)
  #include "sl_wisun_rht_measurement.h"
#endif
// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

/// Meter socket buffer size
#define SL_WISUN_METER_RECV_BUFF_SIZE     (SL_WISUN_METER_COLLECTOR_TOKEN_MAX_SIZE + 64U)

/// Packed packet size. Used for calculations (sizeof cannot be used in #if directive)
#define SL_WISUN_METER_PACKED_PACKET_SIZE 12U

/// Max TX buffer size. Used for calculations
#define SL_WISUN_METER_MAX_TX_BUFF_SIZE   1024U

/// Maximum possible packet count for TX buffer
#define SL_WISUN_METER_MAX_POSSIBLE_PKT_CNT \
  (SL_WISUN_METER_MAX_TX_BUFF_SIZE / SL_WISUN_METER_PACKED_PACKET_SIZE)

/// Maximum meter packet count to transmit
#if (SL_WISUN_METER_MAX_POSSIBLE_PKT_CNT < SL_WISUN_METER_MEASUREMENT_BUFFER_SIZE)
#define SL_WISUN_METER_MAX_TX_PKT (SL_WISUN_METER_MAX_POSSIBLE_PKT_CNT)
#else
#define SL_WISUN_METER_MAX_TX_PKT (SL_WISUN_METER_MEASUREMENT_BUFFER_SIZE)
#endif

/// Warning message format string
#define SL_WISUN_METER_WARNING_MSG_FORMAT_STR            "[Warning: TX buffer is truncated to store %lu packets]\n"

/// Invalid arguments format string
#define SL_WISUN_METER_INVALID_ARGS_FORMAT_STR           "[Invalid arguments in request from '%s': '%s']\n"

/// Invalid token format string
#define SL_WISUN_METER_INVALID_TOKEN_FORMAT_STR          "[Invalid token: '%s']\n"

/// Collector registered format string
#define SL_WISUN_METER_COLLECTOR_REG_FORMAT_STR          "[Collector '%s' registered]\n"

/// Registering collector failed format string
#define SL_WISUN_METER_COLLECTOR_REG_FAILED_FORMAT_STR   "[Registering collector '%s' failed]\n"

/// Collector removed format string
#define SL_WISUN_METER_COLLECTOR_RM_FORMAT_STR           "[Collector '%s' removed]\n"

/// Collector registration failed format string
#define SL_WISUN_METER_COLLECTOR_RM_FAILED_FORMAT_STR   "[Removing collector '%s' failed]\n"

/// Invalid request format string
#define SL_WISUN_METER_INVALID_REQUEST_FORMAT_STR        "[Invalid request from '%s': '%s']\n"

/// Measurement failed format string
#define SL_WISUN_METER_MEAS_FAILED_FORMAT_STR            "[Measurement failed]\n"

/// Measurement sending failed format string
#define SL_WISUN_METER_MEAS_SEND_FAILED_FORMAT_STR       "[Measurement sending to '%s' failed]\n"

// -----------------------------------------------------------------------------
//                          Static Function Declarations
// -----------------------------------------------------------------------------

/**************************************************************************//**
 * @brief Register collector
 * @details Register collector to the collector list
 * @param[in] collector_addr Collector address
 * @return SL_STATUS_OK on success otherwise SL_STATUS_FAIL
 *****************************************************************************/
static sl_status_t _register_collector(const sockaddr_in6_t * const collector_addr);

/**************************************************************************//**
 * @brief Remove collector
 * @details Remove collector from the collector list
 * @param[in] collector_addr Collector address
 * @return SL_STATUS_OK on success otherwise SL_STATUS_FAIL
 *****************************************************************************/
static sl_status_t _remove_collector(const sockaddr_in6_t * const collector_addr);

/**************************************************************************//**
 * @brief Measure
 * @details Measure and pack the metrics
 * @param[out] packed_packet Packed packet
 * @return SL_STATUS_OK on success otherwise SL_STATUS_FAIL
 *****************************************************************************/
static sl_status_t _measure(sl_wisun_meter_packet_packed_t * const packed_packet);

/**************************************************************************//**
 * @brief Measure period
 * @details Measure period and send the metrics
 * @param[in] sockd Socket descriptor
 *****************************************************************************/
static void _measure_period(const int32_t sockd);

/**************************************************************************//**
 * @brief Send metrics
 * @details Send metrics to the collector
 * @param[in] sockd Socket descriptor
 * @param[in] addr_ptr Collector address
 * @param[in] buff Buffer
 * @param[in] buff_size Buffer size
 *****************************************************************************/
static void _send_metrics(const int32_t sockd,
                          sockaddr_in6_t * const addr_ptr,
                          const uint8_t *buff,
                          const size_t buff_size);

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                                Static Variables
// -----------------------------------------------------------------------------

/// Device type
#if defined(WISUN_CONFIG_DEVICE_TYPE)
static uint8_t _device_type = WISUN_CONFIG_DEVICE_TYPE;
#else
static uint8_t _device_type = SL_WISUN_ROUTER;
#endif

/// LFN event notification channel
static uint8_t _lfn_evt_notify_ch = 0U;

/// Metrics storage mempool
static sl_mempool_t _metrics_mpool = { 0 };

/// Metrics storage buffer
static sl_wisun_meter_packet_packed_t _metrics_buff[SL_WISUN_METER_MAX_TX_PKT] = { 0U };

/// Collector storage mempool
static sl_mempool_t _collector_mempool = { 0U };

/// Collector storage buffer
static sockaddr_in6_t _collector_buff[SL_WISUN_METER_MAX_REG_COLLECTOR] = { 0U };

/// Metrics counter
static sl_wisun_meter_metrics_cnt_t _metrics_cnt = {
  .tot_cnt = 0UL,
  .curr_cnt = 0UL,
  .err_cnt = 0UL,
  .fire_cnt = SL_WISUN_METER_MAX_TX_PKT,
  .reg_coll_cnt = 0UL,
  .max_reg_coll_cnt = SL_WISUN_METER_MAX_REG_COLLECTOR
};

// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------

void sl_wisun_meter_init(void)
{
#if defined(SL_CATALOG_TEMP_SENSOR_PRESENT)
  sl_wisun_rht_init();
#endif
  if (_device_type == SL_WISUN_LFN) {
    EFM_ASSERT(app_wisun_em_subscribe_evt_notification(SL_WISUN_MSG_LFN_WAKE_UP_IND_ID,
                                                       &_lfn_evt_notify_ch) != SL_STATUS_FAIL);
  }
  // Init metrics buffer
  EFM_ASSERT(sl_mempool_create(&_metrics_mpool,
                               SL_WISUN_METER_MAX_TX_PKT,
                               sizeof(sl_wisun_meter_packet_packed_t),
                               _metrics_buff,
                               sizeof(_metrics_buff)) == SL_STATUS_OK);

  EFM_ASSERT(sl_mempool_create(&_collector_mempool,
                               SL_WISUN_METER_MAX_REG_COLLECTOR,
                               sizeof(sockaddr_in6_t),
                               _collector_buff,
                               sizeof(_collector_buff)) == SL_STATUS_OK);

  if (SL_WISUN_METER_MAX_TX_PKT < SL_WISUN_METER_MEASUREMENT_BUFFER_SIZE) {
    printf(SL_WISUN_METER_WARNING_MSG_FORMAT_STR, (uint32_t)SL_WISUN_METER_MAX_TX_PKT);
  }
}

void sl_wisun_meter_loop(void)
{
  sl_status_t stat = SL_STATUS_FAIL;
  int32_t sockd = SOCKET_INVALID_ID;
  int32_t res = SOCKET_RETVAL_ERROR;
  static sockaddr_in6_t meter_addr = { 0U };
  static sockaddr_in6_t collector_addr = { 0U };
  static uint8_t buff[SL_WISUN_METER_RECV_BUFF_SIZE] = { 0U };
  static socklen_t addr_len = sizeof(struct sockaddr_in6);
  char *request = NULL;
  char *token = NULL;
  const char *collector_ip = NULL;
  static sl_wisun_meter_packet_packed_t meas_pkt_packed = { 0U };

  // Create listener socket
  sockd = socket(AF_INET6, SOCK_DGRAM | SOCK_NONBLOCK, IPPROTO_UDP);
  EFM_ASSERT(sockd != SOCKET_INVALID_ID);
  meter_addr.sin6_family = AF_INET6;
  meter_addr.sin6_addr = in6addr_any;
  meter_addr.sin6_port = htons(SL_WISUN_METER_COLLECTOR_PORT);
  res = bind(sockd, (const struct sockaddr *) &meter_addr, sizeof(struct sockaddr_in6));
  EFM_ASSERT(res != SOCKET_RETVAL_ERROR);

  // Receiver loop
  SL_WISUN_THREAD_LOOP {
    if (_device_type == SL_WISUN_LFN) {
      app_wisun_em_wait_evt_notification(SL_WISUN_MSG_LFN_WAKE_UP_IND_ID, _lfn_evt_notify_ch);
    }
    memset(buff, 0U, SL_WISUN_METER_RECV_BUFF_SIZE);
    res = recvfrom(sockd, buff, SL_WISUN_METER_RECV_BUFF_SIZE - 1, 0L,
                   (struct sockaddr *) &collector_addr, &addr_len);

    // Request received
    if (res > 0L) {
      // Get Collector IPv6 address string
      collector_ip = app_wisun_trace_util_get_ip_str(&collector_addr.sin6_addr);

      // Parse request
      request = strtok((char *) buff, SL_WISUN_METER_REQUEST_DELIMITER);
      token = strtok(NULL, SL_WISUN_METER_REQUEST_DELIMITER);
      if (request == NULL
          || token == NULL
          || strtok(NULL, SL_WISUN_METER_REQUEST_DELIMITER) != NULL) {
        printf(SL_WISUN_METER_INVALID_ARGS_FORMAT_STR, collector_ip, buff);
        app_wisun_trace_util_destroy_ip_str(collector_ip);
        continue;
      }

      // Check token
      if (strncmp(token,
                  SL_WISUN_METER_COLLECTOR_TOKEN,
                  SL_WISUN_METER_COLLECTOR_TOKEN_MAX_SIZE)) {
        printf(SL_WISUN_METER_INVALID_TOKEN_FORMAT_STR, token);
        app_wisun_trace_util_destroy_ip_str(collector_ip);
        continue;
      }

      // Check request type

      // Registration request
      if (!strncmp(request,
                   SL_WISUN_METER_REQUEST_TYPE_STR_REGISTER,
                   SL_WISUN_METER_RECV_BUFF_SIZE)) {
        stat = _register_collector(&collector_addr);
        if (stat == SL_STATUS_OK) {
          (void) _measure(&meas_pkt_packed);
          _metrics_cnt.reg_coll_cnt++;
          _send_metrics(sockd, &collector_addr, (const uint8_t *) &meas_pkt_packed,
                        sizeof(sl_wisun_meter_packet_packed_t));
          printf(SL_WISUN_METER_COLLECTOR_REG_FORMAT_STR, collector_ip);
        } else {
          printf(SL_WISUN_METER_COLLECTOR_REG_FAILED_FORMAT_STR, collector_ip);
        }

        // Remove request
      } else if (!strncmp(request,
                          SL_WISUN_METER_REQUEST_TYPE_STR_REMOVE,
                          SL_WISUN_METER_RECV_BUFF_SIZE)) {
        stat = _remove_collector(&collector_addr);
        if (stat == SL_STATUS_OK) {
          (void) _measure(&meas_pkt_packed);
          _metrics_cnt.reg_coll_cnt--;
          _send_metrics(sockd, &collector_addr, (const uint8_t *) &meas_pkt_packed,
                        sizeof(sl_wisun_meter_packet_packed_t));
          printf(SL_WISUN_METER_COLLECTOR_RM_FORMAT_STR, collector_ip);
        } else {
          printf(SL_WISUN_METER_COLLECTOR_RM_FAILED_FORMAT_STR, collector_ip);
        }

        // Async request
      } else if (!strncmp(request,
                          SL_WISUN_METER_REQUEST_TYPE_STR_ASYNC,
                          SL_WISUN_METER_RECV_BUFF_SIZE)) {
        (void) _measure(&meas_pkt_packed);
        _send_metrics(sockd, &collector_addr, (const uint8_t *) &meas_pkt_packed,
                      sizeof(sl_wisun_meter_packet_packed_t));
      } else {
        printf(SL_WISUN_METER_INVALID_REQUEST_FORMAT_STR, collector_ip, request);
      }

      app_wisun_trace_util_destroy_ip_str(collector_ip);
    }

    _measure_period(sockd);

    // schedule thread with constant delay for FFN meters only
    if (_device_type == SL_WISUN_ROUTER) {
      osDelay(SL_WISUN_METER_SCHEDULE_TIME);
    }
  }
}

// -----------------------------------------------------------------------------
//                          Static Function Definitions
// -----------------------------------------------------------------------------
static sl_status_t _register_collector(const sockaddr_in6_t * const collector_addr)
{
  sockaddr_in6_t *addr_ptr = NULL;
  if (collector_addr == NULL) {
    return SL_STATUS_FAIL;
  }

  // Check whether collector is already registered
  for (sl_mempool_block_hnd_t *block = _collector_mempool.blocks;
       block != NULL;
       block = block->next) {
    addr_ptr = (sockaddr_in6_t *) block->start_addr;
    if (!memcmp(&addr_ptr->sin6_addr,
                &collector_addr->sin6_addr,
                sizeof(in6_addr_t))) {
      return SL_STATUS_FAIL;
    }
  }

  addr_ptr = (sockaddr_in6_t *)sl_mempool_alloc(&_collector_mempool);

  if (addr_ptr == NULL) {
    return SL_STATUS_FAIL;
  }
  memcpy(addr_ptr, collector_addr, sizeof(sockaddr_in6_t));
  return SL_STATUS_OK;
}

static sl_status_t _remove_collector(const sockaddr_in6_t * const collector_addr)
{
  const sockaddr_in6_t *addr_ptr = NULL;

  if (collector_addr == NULL) {
    return SL_STATUS_FAIL;
  }

  for (sl_mempool_block_hnd_t *block = _collector_mempool.blocks;
       block != NULL;
       block = block->next) {
    addr_ptr = (sockaddr_in6_t *) block->start_addr;
    if (!memcmp(&addr_ptr->sin6_addr,
                &collector_addr->sin6_addr,
                sizeof(in6_addr_t))) {
      sl_mempool_free(&_collector_mempool, addr_ptr);
      return SL_STATUS_OK;
    }
  }

  return SL_STATUS_FAIL;
}

static sl_status_t _measure(sl_wisun_meter_packet_packed_t * const packed_packet)
{
  static sl_wisun_meter_packet_t pkt = { 0U };

  if (packed_packet == NULL) {
    return SL_STATUS_FAIL;
  }

  // Fill packet
  sli_wisun_meter_gen_packet_id(&pkt);
  sli_wisun_meter_get_temperature(&pkt);
  sli_wisun_meter_get_humidity(&pkt);
  sli_wisun_meter_get_light(&pkt);

  sli_wisun_mc_pack_measurement_packet(packed_packet, &pkt);
  _metrics_cnt.tot_cnt++;
  return SL_STATUS_OK;
}

static void _measure_period(const int32_t sockd)
{
  sl_status_t stat = SL_STATUS_FAIL;
  sl_wisun_meter_packet_packed_t *pkt_ptr = NULL;
  sockaddr_in6_t *addr_ptr = NULL;

  pkt_ptr = (sl_wisun_meter_packet_packed_t *) sl_mempool_alloc(&_metrics_mpool);
  stat = _measure(pkt_ptr);
  if (stat != SL_STATUS_OK) {
    printf(SL_WISUN_METER_MEAS_FAILED_FORMAT_STR);
    _metrics_cnt.err_cnt++;
    return;
  }

  _metrics_cnt.curr_cnt++;

  // Only measure if this is not the sending period
  if (_metrics_cnt.curr_cnt < _metrics_cnt.fire_cnt) {
    return;
  }

  // Reset current count
  _metrics_cnt.curr_cnt = 0UL;

  // Send to all collectors
  for (sl_mempool_block_hnd_t *block = _collector_mempool.blocks;
       block != NULL;
       block = block->next) {
    addr_ptr = (sockaddr_in6_t *) block->start_addr;
    _send_metrics(sockd, addr_ptr, (const uint8_t *) _metrics_buff,
                  sizeof(sl_wisun_meter_packet_packed_t) * _metrics_cnt.fire_cnt);
  }

  // Free all packets
  for (sl_mempool_block_hnd_t *block = _metrics_mpool.blocks;
       block != NULL;
       block = block->next) {
    pkt_ptr = (sl_wisun_meter_packet_packed_t *) block->start_addr;
    sl_mempool_free(&_metrics_mpool, pkt_ptr);
  }
}

static void _send_metrics(const int32_t sockd,
                          sockaddr_in6_t * const addr_ptr,
                          const uint8_t *buff,
                          const size_t buff_size)
{
  const char *collector_ip = NULL;
  int32_t res = SOCKET_RETVAL_ERROR;

  addr_ptr->sin6_port = htons(SL_WISUN_METER_COLLECTOR_PORT);

  res = sendto(sockd, buff, buff_size, 0L, (const struct sockaddr *) addr_ptr, sizeof(sockaddr_in6_t));

  if (res < 0L) {
    collector_ip = app_wisun_trace_util_get_ip_str(&addr_ptr->sin6_addr);
    printf(SL_WISUN_METER_MEAS_SEND_FAILED_FORMAT_STR, collector_ip);
    app_wisun_trace_util_destroy_ip_str(collector_ip);
  }
}
