/***************************************************************************//**
 * @file sli_wisun_meter_collector.c
 * @brief Wi-SUN Meter Collector
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
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include "sl_component_catalog.h"
#include "sli_wisun_meter_collector.h"
#if defined(SL_CATALOG_TEMP_SENSOR_PRESENT)
  #include "sl_wisun_rht_measurement.h"
#endif
// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

/// Pack/Unpack measurement packet internal macro function
#define __pack_unpack_measurement_packets(dest, src) \
  do {                                               \
    if (src == NULL) {                               \
      return;                                        \
    }                                                \
    dest->id          = src->id;                     \
    dest->temperature = src->temperature;            \
    dest->humidity    = src->humidity;               \
    dest->light       = src->light;                  \
  } while (0)

// -----------------------------------------------------------------------------
//                          Static Function Declarations
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
//                                Static Variables
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------

#if defined(SL_CATALOG_WISUN_COLLECTOR_PRESENT)
bool sli_wisun_compare_addresses(const sockaddr_in6_t * addr1, const sockaddr_in6_t * addr2)
{
  const uint8_t *p1 = (const uint8_t *)&addr1->sin6_addr;
  const uint8_t *p2 = (const uint8_t *)&addr2->sin6_addr;
  for (uint8_t i = 0U; i < IPV6_ADDR_SIZE; ++i) {
    if (p1[i] != p2[i]) {
      return false;
    }
  }
  return true;
}
#endif

#if defined(SL_CATALOG_WISUN_METER_PRESENT)
void sli_wisun_meter_gen_packet_id(sl_wisun_meter_packet_t *packet)
{
  static uint16_t id = 0U;
  packet->id = id++;
}

void sli_wisun_meter_get_temperature(sl_wisun_meter_packet_t *packet)
{
#if !defined(SL_CATALOG_TEMP_SENSOR_PRESENT)
  /* Dummy temperature values table */
  static const uint16_t dummy_temp[] = { 32500U, 31200U, 29300U, 30000U, 28100U };
  static const uint16_t dummy_temp_size = sizeof(dummy_temp) / sizeof(uint16_t);
  packet->temperature = dummy_temp[packet->id % dummy_temp_size];
#else
  sl_status_t stat = SL_STATUS_FAIL;
  uint32_t dummy = 0U;

  stat = sl_wisun_rht_get(&dummy, &packet->temperature);
  if (stat != SL_STATUS_OK) {
    printf("[Si70xx Temperature measurement error]\n");
  }
#endif
}

void sli_wisun_meter_get_humidity(sl_wisun_meter_packet_t *packet)
{
#if !defined(SL_CATALOG_TEMP_SENSOR_PRESENT)
  /* Dummy humidity values table */
  static const uint16_t dummy_hum[] = { 40500U, 41200U, 39300U, 38000U, 37100U };
  static const uint16_t dummy_hum_size = sizeof(dummy_hum) / sizeof(uint16_t);
  packet->humidity = dummy_hum[packet->id % dummy_hum_size];
#else
  sl_status_t stat = SL_STATUS_FAIL;
  int32_t dummy = 0U;

  stat = sl_wisun_rht_get(&packet->humidity, &dummy);

  if (stat != SL_STATUS_OK) {
    printf("[Si70xx Humidity measurement error]\n");
  }
#endif
}

void sli_wisun_meter_get_light(sl_wisun_meter_packet_t *packet)
{
  /* Dummy lux table */
  static const uint16_t dummy_lux[]    = { 512U, 480U, 600U, 580U, 555U };
  static const uint16_t dummy_lux_size = sizeof(dummy_lux) / sizeof(uint16_t);
  static uint16_t cnt                  = 0U;

  if (cnt == dummy_lux_size) {
    cnt = 0U;
  }

  packet->light = dummy_lux[cnt++];
}
#endif

/* Unpack packet */
void sli_wisun_mc_unpack_measurement_packet(sl_wisun_meter_packet_t * const dest,
                                            const sl_wisun_meter_packet_packed_t * const src)
{
  __pack_unpack_measurement_packets(dest, src);
}

/* Pack packet */
void sli_wisun_mc_pack_measurement_packet(sl_wisun_meter_packet_packed_t * const dest,
                                          const sl_wisun_meter_packet_t * const src)
{
  __pack_unpack_measurement_packets(dest, src);
}

// -----------------------------------------------------------------------------
//                          Static Function Definitions
// -----------------------------------------------------------------------------
