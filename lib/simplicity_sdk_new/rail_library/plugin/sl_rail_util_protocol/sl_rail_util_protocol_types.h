/***************************************************************************//**
 * @file
 * @brief
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

#ifndef SL_RAIL_UTIL_PROTOCOL_TYPES_H
#define SL_RAIL_UTIL_PROTOCOL_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum sl_rail_util_protocol_type {
  SL_RAIL_UTIL_PROTOCOL_PROPRIETARY = 0, /* use rail_config.c/h */
  SL_RAIL_UTIL_PROTOCOL_BLE_1MBPS = 1,
  SL_RAIL_UTIL_PROTOCOL_BLE_2MBPS = 2,
  SL_RAIL_UTIL_PROTOCOL_BLE_CODED_125KBPS = 3,
  SL_RAIL_UTIL_PROTOCOL_BLE_CODED_500KBPS = 4,
  SL_RAIL_UTIL_PROTOCOL_BLE_QUUPPA_1MBPS = 5,
  SL_RAIL_UTIL_PROTOCOL_IEEE802154_2P4GHZ = 6,
  SL_RAIL_UTIL_PROTOCOL_IEEE802154_2P4GHZ_ANTDIV = 7,
  SL_RAIL_UTIL_PROTOCOL_IEEE802154_2P4GHZ_COEX = 8,
  SL_RAIL_UTIL_PROTOCOL_IEEE802154_2P4GHZ_ANTDIV_COEX = 9,
  SL_RAIL_UTIL_PROTOCOL_IEEE802154_GB868_915MHZ = 10,
  SL_RAIL_UTIL_PROTOCOL_IEEE802154_GB868_863MHZ = 11,
  SL_RAIL_UTIL_PROTOCOL_ZWAVE_ANZ = 12,
  SL_RAIL_UTIL_PROTOCOL_ZWAVE_CN = 13,
  SL_RAIL_UTIL_PROTOCOL_ZWAVE_EU = 14,
  SL_RAIL_UTIL_PROTOCOL_ZWAVE_HK = 15,
  SL_RAIL_UTIL_PROTOCOL_ZWAVE_IN = 16,
  SL_RAIL_UTIL_PROTOCOL_ZWAVE_IL = 17,
  SL_RAIL_UTIL_PROTOCOL_ZWAVE_JP = 18,
  SL_RAIL_UTIL_PROTOCOL_ZWAVE_KR = 19,
  SL_RAIL_UTIL_PROTOCOL_ZWAVE_MY = 20,
  SL_RAIL_UTIL_PROTOCOL_ZWAVE_RU = 21,
  SL_RAIL_UTIL_PROTOCOL_ZWAVE_US = 22,
  SL_RAIL_UTIL_PROTOCOL_ZWAVE_US_LR1 = 23,
  SL_RAIL_UTIL_PROTOCOL_ZWAVE_US_LR2 = 24,
  SL_RAIL_UTIL_PROTOCOL_ZWAVE_US_LR3 = 25,
  SL_RAIL_UTIL_PROTOCOL_ZWAVE_US_LR_END_DEVICE = SL_RAIL_UTIL_PROTOCOL_ZWAVE_US_LR3,
  SL_RAIL_UTIL_PROTOCOL_ZWAVE_EU_LR1 = 26,
  SL_RAIL_UTIL_PROTOCOL_ZWAVE_EU_LR2 = 27,
  SL_RAIL_UTIL_PROTOCOL_ZWAVE_EU_LR3 = 28,
  SL_RAIL_UTIL_PROTOCOL_ZWAVE_EU_LR_END_DEVICE = SL_RAIL_UTIL_PROTOCOL_ZWAVE_EU_LR3,
  SL_RAIL_UTIL_PROTOCOL_SIDEWALK_2GFSK_50KBPS = 29,
} sl_rail_util_protocol_type_t;

#define SL_RAIL_UTIL_PROTOCOL_IS_BLE(x)              \
  ((x == SL_RAIL_UTIL_PROTOCOL_BLE_1MBPS)            \
   || (x == SL_RAIL_UTIL_PROTOCOL_BLE_2MBPS)         \
   || (x == SL_RAIL_UTIL_PROTOCOL_BLE_CODED_125KBPS) \
   || (x == SL_RAIL_UTIL_PROTOCOL_BLE_CODED_500KBPS))

#define SL_RAIL_UTIL_PROTOCOL_IS_IEEE802154_2G4(x)          \
  ((x == SL_RAIL_UTIL_PROTOCOL_IEEE802154_2P4GHZ)           \
   || (x == SL_RAIL_UTIL_PROTOCOL_IEEE802154_2P4GHZ_ANTDIV) \
   || (x == SL_RAIL_UTIL_PROTOCOL_IEEE802154_2P4GHZ_COEX)   \
   || (x == SL_RAIL_UTIL_PROTOCOL_IEEE802154_2P4GHZ_ANTDIV_COEX))

#define SL_RAIL_UTIL_PROTOCOL_IS_IEEE802154_GB868(x)    \
  ((x == SL_RAIL_UTIL_PROTOCOL_IEEE802154_GB868_915MHZ) \
   || (x == SL_RAIL_UTIL_PROTOCOL_IEEE802154_GB868_863MHZ))

#define SL_RAIL_UTIL_PROTOCOL_IS_ZWAVE(x)       \
  ((x == SL_RAIL_UTIL_PROTOCOL_ZWAVE_ANZ)       \
   || (x == SL_RAIL_UTIL_PROTOCOL_ZWAVE_CN)     \
   || (x == SL_RAIL_UTIL_PROTOCOL_ZWAVE_EU)     \
   || (x == SL_RAIL_UTIL_PROTOCOL_ZWAVE_HK)     \
   || (x == SL_RAIL_UTIL_PROTOCOL_ZWAVE_IN)     \
   || (x == SL_RAIL_UTIL_PROTOCOL_ZWAVE_IL)     \
   || (x == SL_RAIL_UTIL_PROTOCOL_ZWAVE_JP)     \
   || (x == SL_RAIL_UTIL_PROTOCOL_ZWAVE_KR)     \
   || (x == SL_RAIL_UTIL_PROTOCOL_ZWAVE_MY)     \
   || (x == SL_RAIL_UTIL_PROTOCOL_ZWAVE_RU)     \
   || (x == SL_RAIL_UTIL_PROTOCOL_ZWAVE_US)     \
   || (x == SL_RAIL_UTIL_PROTOCOL_ZWAVE_US_LR1) \
   || (x == SL_RAIL_UTIL_PROTOCOL_ZWAVE_US_LR2) \
   || (x == SL_RAIL_UTIL_PROTOCOL_ZWAVE_US_LR3) \
   || (x == SL_RAIL_UTIL_PROTOCOL_ZWAVE_EU_LR1) \
   || (x == SL_RAIL_UTIL_PROTOCOL_ZWAVE_EU_LR2) \
   || (x == SL_RAIL_UTIL_PROTOCOL_ZWAVE_EU_LR3))

#define SL_RAIL_UTIL_PROTOCOL_IS_SIDEWALK(x) \
  ((x == SL_RAIL_UTIL_PROTOCOL_SIDEWALK_2GFSK_50KBPS))

#ifdef __cplusplus
}
#endif

#endif // SL_RAIL_UTIL_PROTOCOL_TYPES_H
