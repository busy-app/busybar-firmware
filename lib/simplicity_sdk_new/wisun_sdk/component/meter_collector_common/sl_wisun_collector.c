/***************************************************************************//**
 * @file sl_wisun_collector.c
 * @brief Wi-SUN Collector
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
#include <string.h>

#include "sl_assert.h"
#include "sl_component_catalog.h"
#include "cmsis_os2.h"
#include "socket/socket.h"
#include "sl_status.h"
#include "sl_wisun_trace_util.h"
#include "sl_wisun_app_core_util.h"
#include "sl_wisun_meter_collector_config.h"
#include "sli_wisun_meter_collector.h"
#include "sl_wisun_collector.h"
// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------
#define SL_WISUN_COLLECTOR_RECV_BUFF_SIZE (1024UL)
// -----------------------------------------------------------------------------
//                          Static Function Declarations
// -----------------------------------------------------------------------------
static void _print_packet(const char *ip_str, const sl_wisun_meter_packet_packed_t *pkt);
// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                                Static Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------

void sl_wisun_collector_loop(void)
{
  int32_t sockd = SOCKET_INVALID_ID;
  int32_t res = SOCKET_RETVAL_ERROR;
  static sockaddr_in6_t listener_addr = { 0U };
  static sockaddr_in6_t meter_addr = { 0U };
  static uint8_t buff[SL_WISUN_COLLECTOR_RECV_BUFF_SIZE] = { 0U };
  static socklen_t addr_len = sizeof(struct sockaddr_in6);
  const char *meter_ip_str = NULL;
  static sl_wisun_meter_packet_packed_t meas_pkt_packed = { 0U };
  uint16_t pkt_cnt = 0U;

  // Create listener socket
  sockd = socket(AF_INET6, SOCK_DGRAM | SOCK_NONBLOCK, IPPROTO_UDP);
  EFM_ASSERT(sockd != SOCKET_INVALID_ID);
  listener_addr.sin6_family = AF_INET6;
  listener_addr.sin6_addr = in6addr_any;
  listener_addr.sin6_port = htons(SL_WISUN_METER_COLLECTOR_PORT);
  res = bind(sockd, (const struct sockaddr *) &listener_addr, sizeof(struct sockaddr_in6));
  EFM_ASSERT(res != SOCKET_RETVAL_ERROR);

  // Receiver loop
  SL_WISUN_THREAD_LOOP {
    res = recvfrom(sockd, buff, SL_WISUN_COLLECTOR_RECV_BUFF_SIZE, 0L,
                   (struct sockaddr *) &meter_addr, &addr_len);

    // Request received
    if (res > 0L) {
      // Get Collector IPv6 address string
      meter_ip_str = app_wisun_trace_util_get_ip_str(&meter_addr.sin6_addr);

      // Parse metrics
      pkt_cnt = (uint16_t)(res / sizeof(sl_wisun_meter_packet_packed_t));

      for (uint16_t i = 0U; i < pkt_cnt; ++i) {
        memcpy(&meas_pkt_packed, &buff[i * sizeof(sl_wisun_meter_packet_packed_t)],
               sizeof(sl_wisun_meter_packet_packed_t));

        _print_packet(meter_ip_str, &meas_pkt_packed);
      }

      app_wisun_trace_util_destroy_ip_str(meter_ip_str);
    }

    sl_wisun_app_core_util_dispatch_thread();
  }
}

// -----------------------------------------------------------------------------
//                          Static Function Definitions
// -----------------------------------------------------------------------------

static void _print_packet(const char *ip_str, const sl_wisun_meter_packet_packed_t *pkt)
{
  printf("[%s: {id: %u}{temp: %lu.%.2lu}{hum: %lu.%.2lu}{lx: %u}]\n",
         ip_str,
         pkt->id,
         pkt->temperature / 1000,
         (pkt->temperature % 1000) / 10,
         pkt->humidity / 1000,
         (pkt->humidity % 1000) / 10,
         pkt->light);
}
