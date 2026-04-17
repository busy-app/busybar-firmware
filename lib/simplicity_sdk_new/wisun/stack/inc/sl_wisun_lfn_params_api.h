/***************************************************************************//**
 * @file sl_wisun_lfn_params_api.h
 * @brief Wi-SUN LFN Connection Params API
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

#ifndef SL_WISUN_LFN_PARAMS_API_H
#define SL_WISUN_LFN_PARAMS_API_H

#include <stdint.h>
#include "sl_wisun_common.h"
#include "sl_common.h"
#include "sl_status.h"
#include "sl_wisun_connection_params_api.h"

/// API version used to check compatibility (do not edit when using this header)
#define SL_WISUN_LFN_PARAMS_API_VERSION  4

/**************************************************************************//**
 * @addtogroup SL_WISUN_TYPES
 * @{
 *****************************************************************************/

/// LFN connection parameters
SL_PACK_START(1)
typedef struct {
  /// Duration of LFN PAN Advertisement (LPA) listening slot (millisecond)
  /// Specification range [15, 255]
  uint8_t discovery_slot_time_ms;
  /// Number of LPA slots for which an LFN shall listen for LPA frames
  /// Specification range [1, 255]
  uint8_t discovery_slots;
  /// Reserved, set to zero
  uint8_t reserved[2];
} SL_ATTRIBUTE_PACKED sl_wisun_lfn_params_connection_t;
SL_PACK_END()

/// LFN data layer parameters
SL_PACK_START(1)
typedef struct {
  /// Initial LFN Unicast interval proposed by the LFN (milliseconds).
  /// The real unicast interval duration is negotiated with the LFN parent,
  /// between unicast_interval_min_ms and unicast_interval_max_ms.
  uint32_t unicast_interval_ms;
  /// Minimum acceptable LFN unicast interval (milliseconds)
  uint32_t unicast_interval_min_ms;
  /// Maximum acceptable LFN unicast interval (milliseconds)
  uint32_t unicast_interval_max_ms;
  /// Shorter LFN Unicast interval used during the EAPOL exchange (milliseconds).
  /// Wi-SUN FAN TPS mandates this to be greater or equal than unicast_interval_min_ms
  uint32_t eapol_unicast_interval_ms;
  /// The LFN assumes its parent is lost after [lfn_maintain_parent_time]
  /// number of Broadcast sync periods with no message received from its parent.
  /// Specification range [1, 60]
  uint8_t lfn_maintain_parent_time;
  /// Reserved, set to zero
  uint8_t reserved[3];
} SL_ATTRIBUTE_PACKED sl_wisun_lfn_params_data_layer_t;
SL_PACK_END()

/// LFN EAPOL parameters
SL_PACK_START(1)
typedef struct {
  /// Max delay of first key request (seconds)
  uint16_t initial_key_max_s;
  /// Initial retransmission time (seconds)
  uint16_t initial_key_retry_min_s;
  /// Maximum duration for retransmissions (seconds)
  uint16_t initial_key_retry_max_limit_s;
  /// LGTK_MAX_MISMATCH (minutes)
  /// Maximum time between a SUP detecting a LGTKHASH mismatch and the SUP
  /// initiating Msg1 of the authentication flow
  uint16_t lgtk_max_mismatch_m;
  /// Maximum retransmission count
  uint8_t initial_key_retry_limit;
  /// If true, allow join state 2 to be skipped using cached credentials
  /// from the previous connection.
  uint8_t allow_skip;
  /// Reserved, set to zero
  uint8_t reserved[2];
} SL_ATTRIBUTE_PACKED sl_wisun_lfn_params_eapol_t;
SL_PACK_END()

/// LFN network parameters
SL_PACK_START(1)
typedef struct {
  /// Address registration lifetime (IPv6 lease duration) the LFN requires
  /// to the Border Router (minutes).
  /// Specification range [1440, 5040]
  uint16_t lfn_registration_lifetime_m;
  /// Duration for which an LFN waits for a registration confirmation (minutes).
  /// This value is administratively configured and should be consistent throughout
  /// the network.
  /// Make sure that the value is equal to or greather than the corresponding field on FFNs.
  /// Specification range [30, 120] (set lower for test purposes)
  uint8_t lfn_na_wait_duration_m;
  /// Reserved, set to zero
  uint8_t reserved;
} SL_ATTRIBUTE_PACKED sl_wisun_lfn_params_network_t;
SL_PACK_END()

/// LFN power parameters
SL_PACK_START(1)
typedef struct {
  /// Minimum duration of the listening window.
  /// Applies to both Unicast and Broadcast slots.
  uint16_t listening_window_min_us;
  /// Minimum margin added to the listening window (before and after).
  /// The real margin increases with aging synchronization info.
  uint16_t window_margin_min_us;
  /// If true, the LFN wakes up only for broadcast slots containing
  /// synchronization information.
  /// If false, the node wakes up on every LFN broadcast slot.
  uint8_t broadcast_lts_only;
  /// Reserved, set to zero
  uint8_t reserved[3];
} SL_ATTRIBUTE_PACKED sl_wisun_lfn_params_power_t;
SL_PACK_END()

/// LFN parameter set
SL_PACK_START(1)
typedef struct {
  /**
   * Version of this API.
   *
   * This field allows to store the parameters in an NVM and check on reload
   * that they are compatible with the stack if there was an update.
   */
  uint32_t version;
  /// LFN connection parameters
  sl_wisun_lfn_params_connection_t connection;
  /// LFN data layer parameters
  sl_wisun_lfn_params_data_layer_t data_layer;
  /// LFN EAPOL parameters
  sl_wisun_lfn_params_eapol_t eapol;
  /// LFN network parameters
  sl_wisun_lfn_params_network_t network;
  /// LFN power parameters
  sl_wisun_lfn_params_power_t power;
  /// DHCP parameter set
  sl_wisun_params_dhcp_t dhcp;
  /// Traffic parameter set
  sl_wisun_params_traffic_t traffic;
  /// MAC parameter set
  sl_wisun_mac_params_t mac;
} SL_ATTRIBUTE_PACKED sl_wisun_lfn_params_t;
SL_PACK_END()

/**************************************************************************//**
 * @addtogroup SL_WISUN_LFN_PARAMETER_SETS Predefined LFN parameter sets
 *
 * Predefined LFN parameter sets for sl_wisun_set_lfn_parameters().
 *
 * These parameter sets can be used either as-is or used as an initialization value
 * for an application-specific parameter set.
 *
 * @{
 *****************************************************************************/

/// Profile for test usage, best performance but highest power consumption
static const sl_wisun_lfn_params_t SL_WISUN_PARAMS_LFN_TEST = {
  .version = SL_WISUN_LFN_PARAMS_API_VERSION,
  .connection = {
    .discovery_slot_time_ms = 60,
    .discovery_slots = 40,
    .reserved = { 0 }
  },
  .data_layer = {
    .unicast_interval_ms = SEC_TO_MS(6),
    .unicast_interval_min_ms = SEC_TO_MS(1),
    .unicast_interval_max_ms = SEC_TO_MS(60),
    .eapol_unicast_interval_ms = 1300,
    .lfn_maintain_parent_time = 5,
    .reserved = { 0 }
  },
  .eapol = {
    .initial_key_max_s = 3,
    .initial_key_retry_min_s = 18,
    .initial_key_retry_max_limit_s = 42,
    .lgtk_max_mismatch_m = 2,
    .initial_key_retry_limit = 3,
    .allow_skip = true
  },
  .network = {
    .lfn_registration_lifetime_m = HOUR_TO_MIN(36),
    .lfn_na_wait_duration_m = 5,
    .reserved = 0
  },
  .power = {
    .listening_window_min_us = 500,
    .window_margin_min_us = 1000,
    .broadcast_lts_only = 0,
    .reserved = { 0 }
  },
  .dhcp = {
    .sol_txalg = {
      .rand = 0.1f,
      .max_delay_s = 10,
      .irt_s = 10,
      .mrt_s = HOUR_TO_SEC(1),
      .mrd_s = 0,
      .mrc = 3,
    },
  },
  .traffic = {
    .lowpan_mtu = 1576,
    .ipv6_mru = 1504,
    .max_edfe_fragment_count = 5,
  },
  .mac = {
    .backoff_period_us = 0, // calculate from PHY by default
    .min_be = 3,
    .max_be = 5,
    .max_cca_retries = 8,
    .max_frame_retries = 7,
  },
};

/// Profile providing balance between power consumption and performance
static const sl_wisun_lfn_params_t SL_WISUN_PARAMS_LFN_BALANCED = {
  .version = SL_WISUN_LFN_PARAMS_API_VERSION,
  .connection = {
    .discovery_slot_time_ms = 60,
    .discovery_slots = 40,
    .reserved = { 0 }
  },
  .data_layer = {
    .unicast_interval_ms = SEC_TO_MS(60),
    .unicast_interval_min_ms = SEC_TO_MS(1),
    .unicast_interval_max_ms = SEC_TO_MS(300),
    .eapol_unicast_interval_ms = 1300,
    .lfn_maintain_parent_time = 20,
    .reserved = { 0 }
  },
  .eapol = {
    .initial_key_max_s = 30,
    .initial_key_retry_min_s = 180,
    .initial_key_retry_max_limit_s = 420,
    .lgtk_max_mismatch_m = 60,
    .initial_key_retry_limit = 3,
    .allow_skip = true
  },
  .network = {
    .lfn_registration_lifetime_m = HOUR_TO_MIN(60),
    .lfn_na_wait_duration_m = HOUR_TO_MIN(1),
    .reserved = 0
  },
  .power = {
    .listening_window_min_us = 500,
    .window_margin_min_us = 1000,
    .broadcast_lts_only = 0,
    .reserved = { 0 }
  },
  .dhcp = {
    .sol_txalg = {
      .rand = 0.1f,
      .max_delay_s = MIN_TO_SEC(5),
      .irt_s = MIN_TO_SEC(15),
      .mrt_s = HOUR_TO_SEC(1),
      .mrd_s = 0,
      .mrc = 3,
    },
  },
  .traffic = {
    .lowpan_mtu = 1576,
    .ipv6_mru = 1504,
    .max_edfe_fragment_count = 5,
  },
  .mac = {
    .backoff_period_us = 0, // calculate from PHY by default
    .min_be = 3,
    .max_be = 5,
    .max_cca_retries = 8,
    .max_frame_retries = 7,
  },
};

/// Profile optimized for low power consumption
static const sl_wisun_lfn_params_t SL_WISUN_PARAMS_LFN_ECO = {
  .version = SL_WISUN_LFN_PARAMS_API_VERSION,
  .connection = {
    .discovery_slot_time_ms = 60,
    .discovery_slots = 40,
    .reserved = { 0 }
  },
  .data_layer = {
    .unicast_interval_ms = SEC_TO_MS(300),
    .unicast_interval_min_ms = SEC_TO_MS(1),
    .unicast_interval_max_ms = SEC_TO_MS(16776), /// 4.660 hours per spec
    .eapol_unicast_interval_ms = 1300,
    .lfn_maintain_parent_time = 60,
    .reserved = { 0 }
  },
  .eapol = {
    .initial_key_max_s = 30,
    .initial_key_retry_min_s = 300,
    .initial_key_retry_max_limit_s = 900,
    .lgtk_max_mismatch_m = 60,
    .initial_key_retry_limit = 5,
    .allow_skip = true
  },
  .network = {
    .lfn_registration_lifetime_m = HOUR_TO_MIN(84),
    .lfn_na_wait_duration_m = HOUR_TO_MIN(2),
    .reserved = 0
  },
  .power = {
    .listening_window_min_us = 500,
    .window_margin_min_us = 1000,
    .broadcast_lts_only = 1,
    .reserved = { 0 }
  },
  .dhcp = {
    .sol_txalg = {
      .rand = 0.1f,
      .max_delay_s = MIN_TO_SEC(10),
      .irt_s = MIN_TO_SEC(30),
      .mrt_s = HOUR_TO_SEC(1),
      .mrd_s = 0,
      .mrc = 3,
    },
  },
  .traffic = {
    .lowpan_mtu = 1576,
    .ipv6_mru = 1504,
    .max_edfe_fragment_count = 5,
  },
  .mac = {
    .backoff_period_us = 0, // calculate from PHY by default
    .min_be = 3,
    .max_be = 5,
    .max_cca_retries = 8,
    .max_frame_retries = 7,
  },
};

/** @} */

/** @} (end SL_WISUN_TYPES) */

#endif
