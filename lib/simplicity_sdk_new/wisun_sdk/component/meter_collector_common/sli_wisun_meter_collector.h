/***************************************************************************//**
 * @file sli_wisun_meter_collector.h
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

#ifndef SLI_WISUN_METER_COLLECTOR_H
#define SLI_WISUN_METER_COLLECTOR_H

#ifdef __cplusplus
extern "C" {
#endif

// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "sl_component_catalog.h"
#include "sl_status.h"
#include "em_common.h"
#include "socket/socket.h"
// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

/// Register request type string
#define SL_WISUN_METER_REQUEST_TYPE_STR_REGISTER      "register"

/// Async request type string
#define SL_WISUN_METER_REQUEST_TYPE_STR_ASYNC         "async"

/// Remove request type string
#define SL_WISUN_METER_REQUEST_TYPE_STR_REMOVE        "remove"

/// Meter request/response string max length
#define SL_WISUN_METER_REQUEST_RESPONSE_STR_MAX_LEN   (640U)

/// Meter LED0 toggle payload string
#define SL_WISUN_METER_LED0_TOGGLE_PAYLOAD_STR        "0"

/// Meter LED1 toggle payload string
#define SL_WISUN_METER_LED1_TOGGLE_PAYLOAD_STR        "1"

/// Meter LED toggle payload string max length
#define SL_WISUN_METER_LED_TOGGLE_PAYLOAD_STR_MAX_LEN (16U)

/// Meter request delimiter (only for simple Meter and Collector)
#define SL_WISUN_METER_REQUEST_DELIMITER             " "

/// Measurement packet structure
typedef struct sl_wisun_meter_packet {
  /// ID
  uint16_t id;
  /// Temperature [milliCelsius]
  int32_t temperature;
  /// Humidity    [Rel% * 1000]
  uint32_t humidity;
  /// Light       [Lux]
  uint16_t light;
} sl_wisun_meter_packet_t;

SL_PACK_START(1)
/// Packed meter packet structure for byte stream conversion
typedef struct sl_wisun_meter_packet_packed {
  /// ID
  uint16_t id;
  /// Temperature [milliCelsius]
  int32_t temperature;
  /// Humidity    [Rel% * 1000]
  uint32_t humidity;
  /// Light       [Lux]
  uint16_t light;
} SL_ATTRIBUTE_PACKED sl_wisun_meter_packet_packed_t;
SL_PACK_END()

/// Metrics counter structure
typedef struct sl_wisun_meter_metrics_cnt {
  /// Total count of metrics sending
  uint32_t tot_cnt;
  /// Current count of metrics sending in the current period
  uint32_t curr_cnt;
  /// Error count of metrics sending
  uint32_t err_cnt;
  /// Expected count of metrics sending in one period
  const uint32_t fire_cnt;
  /// Registered collector count
  uint32_t reg_coll_cnt;
  /// Maximum count of registered collector
  const uint32_t max_reg_coll_cnt;
} sl_wisun_meter_metrics_cnt_t;

/// Request type definition
typedef enum sl_wisun_request_type {
  SL_WISUN_MC_REQ_ASYNC = 0,
  SL_WISUN_MC_REQ_REGISTER,
  SL_WISUN_MC_REQ_REMOVE,
  SL_WISUN_MC_REQ_RD,
  SL_WISUN_MC_REQ_UNKNOWN
} sl_wisun_request_type_t;

typedef struct sl_wisun_meter_request {
  uint16_t length;
  uint8_t *buff;
} sl_wisun_meter_request_t;

/// Meter entry type definition
typedef struct sl_wisun_meter_entry {
  /// Address structure for meter
  sockaddr_in6_t addr;
  /// Request sent timestamp
  uint32_t req_sent_timestamp;
  /// Response received timestamp
  uint32_t resp_recv_timestamp;
  /// Type of the meter
  uint8_t type;
} sl_wisun_meter_entry_t;

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                          Public Function Declarations
// -----------------------------------------------------------------------------

/// Meter common declarations
#if defined(SL_CATALOG_WISUN_METER_PRESENT)
/**************************************************************************//**
 * @brief Generate packet ID
 * @details Generate packet ID for the packet.
 * @param[out] packet Pointer to the packet
 *****************************************************************************/
void sli_wisun_meter_gen_packet_id(sl_wisun_meter_packet_t *packet);

/**************************************************************************//**
 * @brief Get temperature
 * @details Get temperature from the sensor and set it to the packet.
 * @param[out] packet Pointer to the packet
 *****************************************************************************/
void sli_wisun_meter_get_temperature(sl_wisun_meter_packet_t *packet);

/**************************************************************************//**
 * @brief Get humidity
 * @details Get humidity from the sensor and set it to the packet.
 * @param[out] packet Pointer to the packet
 *****************************************************************************/
void sli_wisun_meter_get_humidity(sl_wisun_meter_packet_t *packet);

/**************************************************************************//**
 * @brief Get light
 * @details Get light from the sensor and set it to the packet.
 * @param[out] packet Pointer to the packet
 *****************************************************************************/
void sli_wisun_meter_get_light(sl_wisun_meter_packet_t *packet);

/// Collector common declarations
#endif

#if defined(SL_CATALOG_WISUN_COLLECTOR_PRESENT)
/**************************************************************************//**
 * @brief Compare addresses
 * @details Compare two addresses.
 * @param[in] addr1 Pointer to the first address
 * @param[in] addr2 Pointer to the second address
 * @return true if the addresses are equal, otherwise false
 *****************************************************************************/
bool sli_wisun_compare_addresses(const sockaddr_in6_t * addr1, const sockaddr_in6_t * addr2);
#endif

/**************************************************************************//**
 * @brief Unpack measurement packet
 * @details Unpack the measurement packet from the packed structure.
 * @param[out] dest Pointer to the destination packet
 * @param[in] src Pointer to the source packed packet
 *****************************************************************************/
void sli_wisun_mc_unpack_measurement_packet(sl_wisun_meter_packet_t * const dest,
                                            const sl_wisun_meter_packet_packed_t * const src);

/**************************************************************************//**
 * @brief Pack measurement packet
 * @details Pack the measurement packet to the packed structure.
 * @param[out] dest Pointer to the destination packed packet
 * @param[in] src Pointer to the source packet
 *****************************************************************************/
void sli_wisun_mc_pack_measurement_packet(sl_wisun_meter_packet_packed_t * const dest,
                                          const sl_wisun_meter_packet_t * const src);
#ifdef __cplusplus
}
#endif

#endif // SLI_WISUN_METER_COLLECTOR_H
