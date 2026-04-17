/***************************************************************************//**
 * @file sl_wisun_connection_params_api.h
 * @brief Wi-SUN Connection Params API
 *******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifndef SL_WISUN_CONNECTION_PARAMS_API_H
#define SL_WISUN_CONNECTION_PARAMS_API_H

#include <stdint.h>
#include "sl_wisun_common.h"
#include "sl_common.h"
#include "sl_status.h"
#include "sl_wisun_types.h"

/// API version used to check compatibility (do not edit when using this header)
#define SL_WISUN_PARAMS_API_VERSION  9

/**************************************************************************//**
 * @addtogroup SL_WISUN_TYPES
 * @{
 *****************************************************************************/

/// Trickle parameter set
SL_PACK_START(1)
typedef struct {
  /// Minimum interval size (seconds)
  uint16_t imin_s;
  /// Maximum interval size (seconds)
  uint16_t imax_s;
  /// Redundancy constant (0 for infinity)
  uint8_t k;
  /// Reserved, set to zero
  uint8_t reserved[3];
} SL_ATTRIBUTE_PACKED sl_wisun_trickle_params_t;
SL_PACK_END()

/// Key lifetimes parameter set
SL_PACK_START(1)
typedef struct {
  /// PMK lifetime (minutes)
  uint32_t pmk_lifetime_m;
  /// LPMK lifetime (minutes)
  uint32_t lpmk_lifetime_m;
  /// PTK lifetime (minutes)
  uint32_t ptk_lifetime_m;
  /// LPTK lifetime (minutes)
  uint32_t lptk_lifetime_m;
  /// GTK_EXPIRE_OFFSET (minutes)
  /// The expiration time of a GTK is calculated as
  /// the expiration time of the GTK most recently installed at
  /// the Border Router plus GTK_EXPIRE_OFFSET
  uint32_t gtk_expire_offset_m;
  /// LGTK_EXPIRE_OFFSET (minutes)
  /// The expiration time of a LGTK is calculated as
  /// the expiration time of the GTK most recently installed at
  /// the Border Router plus LGTK_EXPIRE_OFFSET
  uint32_t lgtk_expire_offset_m;
  /// GTK_NEW_ACTIVATION_TIME
  /// The time at which the Border Router activates the next GTK
  /// prior to expiration of the currently activated GTK.
  uint32_t gtk_new_activation_time;
  /// LGTK_NEW_ACTIVATION_TIME
  /// The time at which the Border Router activates the next LGTK
  /// prior to expiration of the currently activated LGTK.
  uint32_t lgtk_new_activation_time;
  /// GTK_NEW_INSTALL_REQUIRED
  /// The percentage of time elapsed in the active GTK’s lifetime
  /// at which a new GTK must be installed on the Border Router
  uint32_t gtk_new_install_required;
  /// LGTK_NEW_INSTALL_REQUIRED
  /// The percentage of time elapsed in the active LGTK’s lifetime
  /// at which a new LGTK must be installed on the Border Router
  uint32_t lgtk_new_install_required;
  /// FFN_REVOCATION_LIFETIME_REDUCTION
  /// Factor by which the active GTK lifetime is reduced during node
  /// revocation procedures.
  uint32_t ffn_revocation_lifetime_reduction;
  /// LFN_REVOCATION_LIFETIME_REDUCTION
  /// Factor by which the active LGTK lifetime is reduced during node
  /// revocation procedures.
  uint32_t lfn_revocation_lifetime_reduction;
} SL_ATTRIBUTE_PACKED sl_wisun_key_lifetimes_params_t;
SL_PACK_END()

/// PAN discovery parameter set
SL_PACK_START(1)
struct sl_wisun_params_discovery_s {
  /// PAN Advertisement trickle timer
  sl_wisun_trickle_params_t trickle_pa;
  /// PAN Advertisement Solicit trickle timer
  sl_wisun_trickle_params_t trickle_pas;
  /// Minimum signal level for a node to be selected as the EAPOL target for
  /// authentication immediately after a PAN Advertisement reception.
  /// Range from -174 (0) to +80 (254) dBm, 255 to disable feature.
  uint8_t eapol_target_min_sens;
  /// If true, allow join state 1 to be skipped using cached information
  /// from the previous connection.
  uint8_t allow_skip;
  /// Reserved, set to zero
  uint8_t reserved[2];
} SL_ATTRIBUTE_PACKED;
SL_PACK_END()

/// PAN discovery parameter set
typedef struct sl_wisun_params_discovery_s sl_wisun_params_discovery;
/// PAN discovery parameter set
typedef struct sl_wisun_params_discovery_s sl_wisun_params_discovery_t;

/// Authentication parameter set
SL_PACK_START(1)
struct sl_wisun_params_eapol_s {
  /// Deprecated
  sl_wisun_trickle_params_t sec_prot_trickle;
  /// PMK lifetime (minutes)
  uint32_t pmk_lifetime_m;
  /// PTK lifetime (minutes)
  uint32_t ptk_lifetime_m;
  /// Deprecated
  uint16_t sec_prot_retry_timeout_s;
  /// Deprecated
  uint16_t initial_key_min_s;
  /// Max delay of first key request (seconds)
  uint16_t initial_key_max_s;
  /// Initial retransmission time (seconds)
  uint16_t initial_key_retry_min_s;
  /// Deprecated
  uint16_t initial_key_retry_max_s;
  /// Maximum duration for retransmissions (seconds)
  uint16_t initial_key_retry_max_limit_s;
  /// Deprecated
  uint16_t temp_min_timeout_s;
  /// Deprecated
  uint16_t gtk_request_imin_m;
  /// Deprecated
  uint16_t gtk_request_imax_m;
  /// GTK_MAX_MISMATCH (minutes)
  /// Maximum time between a SUP detecting a GTKHASH mismatch and the SUP
  /// initiating Msg1 of the authentication flow
  uint16_t gtk_max_mismatch_m;
  /// LGTK_MAX_MISMATCH (minutes)
  /// Maximum time between a SUP detecting a LGTKHASH mismatch and the SUP
  /// initiating Msg1 of the authentication flow
  uint16_t lgtk_max_mismatch_m;
  /// Deprecated
  uint8_t sec_prot_trickle_expirations;
  /// Maximum retransmission count
  uint8_t initial_key_retry_limit;
  /// If true, allow join state 2 to be skipped using cached credentials
  /// from the previous connection.
  uint8_t allow_skip;
  /// Reserved, set to zero
  uint8_t reserved[3];
} SL_ATTRIBUTE_PACKED;
SL_PACK_END()

/// Authentication parameter set
typedef struct sl_wisun_params_eapol_s sl_wisun_params_eapol;
/// Authentication parameter set
typedef struct sl_wisun_params_eapol_s sl_wisun_params_eapol_t;

/// PAN configuration parameter set
SL_PACK_START(1)
struct sl_wisun_params_configuration_s {
  /// PAN Configuration trickle timer
  sl_wisun_trickle_params_t trickle_pc;
  /// PAN Configuration Solicit trickle timer
  sl_wisun_trickle_params_t trickle_pcs;
} SL_ATTRIBUTE_PACKED;
SL_PACK_END()

/// PAN configuration parameter set
typedef struct sl_wisun_params_configuration_s sl_wisun_params_configuration;
/// PAN configuration parameter set
typedef struct sl_wisun_params_configuration_s sl_wisun_params_configuration_t;

/// RPL parameter set
SL_PACK_START(1)
struct sl_wisun_params_rpl_s {
  /// RFC8415 TX algorithm parameters for DAO transmission
  /// rand specification range [0.1, 0.5]
  /// irt_s specification range [1, 60]
  /// mrt_s specification range [0, 600]
  /// mrd_s specification range [0, 3600]
  /// mrc specification range [0, 15]
  sl_wisun_rfc8415_txalg_params_t dao_txalg;
  /// RPL first DIS maximum delay (seconds)
  uint16_t dis_max_delay_first_s;
  /// RPL DIS maximum delay (seconds)
  uint16_t dis_max_delay_s;
  /// Delay for preferred parent selection after first DIO reception (seconds)
  uint16_t init_parent_selection_s;
  /// Maximum period of NS probes used to get samples for ETX calculation (seconds)
  uint16_t etx_probe_period_max_s;
  /// Amount of time a parent should retain a registered GUA/ULA (seconds)
  uint16_t address_registration_lifetime_s;
  /// Number of samples used to calculate ETX during join state 4
  uint8_t etx_samples_init;
  /// Number of samples used to refresh ETX
  uint8_t etx_samples_refresh;
  /// RPL max candidate parents
  uint8_t candidate_parents_max;
  /// RPL max parents, capped at 4 due to Path Control field size in DAO
  uint8_t parents_max;
  /// Reserved, set to zero
  uint8_t reserved[2];
} SL_ATTRIBUTE_PACKED;
SL_PACK_END()

/// RPL parameter set
typedef struct sl_wisun_params_rpl_s sl_wisun_params_rpl;
/// RPL parameter set
typedef struct sl_wisun_params_rpl_s sl_wisun_params_rpl_t;

/// MPL parameter set
SL_PACK_START(1)
struct sl_wisun_params_mpl_s {
  /// MPL trickle timer
  sl_wisun_trickle_params_t trickle;
  /// MPL seed set entry lifetime (seconds)
  uint16_t seed_set_entry_lifetime_s;
  /// MPL trickle timer expirations
  uint8_t trickle_expirations;
  /// MPL seed-id type,
  ///  0 indicates the seed-id is elided and is the IPv6 source address,
  ///  3 indicates the seed-id is the 128-bit ULA/GUA of the source
  uint8_t seed_id_type;
} SL_ATTRIBUTE_PACKED;
SL_PACK_END()

/// MPL parameter set
typedef struct sl_wisun_params_mpl_s sl_wisun_params_mpl;
/// MPL parameter set
typedef struct sl_wisun_params_mpl_s sl_wisun_params_mpl_t;

/// DHCP parameter set
SL_PACK_START(1)
struct sl_wisun_params_dhcp_s {
  /// RFC8415 TX algorithm parameters for DHCP solicitation
  sl_wisun_rfc8415_txalg_params_t sol_txalg;
} SL_ATTRIBUTE_PACKED;
SL_PACK_END()

/// DHCP parameter set
typedef struct sl_wisun_params_dhcp_s sl_wisun_params_dhcp;
/// DHCP parameter set
typedef struct sl_wisun_params_dhcp_s sl_wisun_params_dhcp_t;

/// LFN parent parameter set
SL_PACK_START(1)
struct sl_wisun_params_lfn_parent_s {
  /// Deprecated
  uint16_t lfn_pan_timeout_m;
  /// Number of broadcast LFN Pan Config retries when LFN Version is incremented
  /// Referred to as LFN_MAINTAIN_PARENT_TIME in FAN TPS 1.1
  uint8_t lfn_lpc_retry_count;
  /// Deprecated
  uint8_t lfn_na_wait_duration_m;
} SL_ATTRIBUTE_PACKED;
SL_PACK_END()

/// LFN parent parameter set
typedef struct sl_wisun_params_lfn_parent_s sl_wisun_params_lfn_parent;
/// LFN parent parameter set
typedef struct sl_wisun_params_lfn_parent_s sl_wisun_params_lfn_parent_t;

/// Misc parameter set
SL_PACK_START(1)
struct sl_wisun_params_misc_s {
  /// Temporary neighbor link minimum timeout
  uint16_t temp_link_min_timeout_s;
  /// Border router communication timeout PAN_TIMEOUT
  uint8_t pan_timeout_m;
  /// Reserved, set to zero
  uint8_t reserved;
} SL_ATTRIBUTE_PACKED;
SL_PACK_END()

/// Misc parameter set
typedef struct sl_wisun_params_misc_s sl_wisun_params_misc;
/// Misc parameter set
typedef struct sl_wisun_params_misc_s sl_wisun_params_misc_t;

/// Traffic parameter set
SL_PACK_START(1)
typedef struct sl_wisun_params_traffic_s {
  /// Maximum Transmission Unit (MTU) for 6LoWPAN packets in bytes. A larger packet will be fragmented using 6LoWPAN fragmentation.
  uint16_t lowpan_mtu;
  /// Maximum Receive Unit (MRU) for fragmented IPv6 packets in bytes. A larger packet will be silently discarded.
  uint16_t ipv6_mru;
  /// Maximum number of 6LoWPAN/IPv6 fragments to send in a single EDFE transaction (0 to 10). 0 disables EDFE for fragmented packets.
  uint8_t max_edfe_fragment_count;
  /// Reserved, set to zero
  uint8_t reserved[3];
} SL_ATTRIBUTE_PACKED sl_wisun_params_traffic_t;
SL_PACK_END()

/// FFN parameter set
SL_PACK_START(1)
typedef struct {
  /**
   * Version of this API.
   *
   * This field allows to store the parameters in an NVM and check on reload
   * that they are compatible with the stack if there was an update.
   */
  uint32_t version;
  /// PAN discovery parameter set
  sl_wisun_params_discovery_t discovery;
  /// PAN configuration parameter set
  sl_wisun_params_configuration_t configuration;
  /// Authentication parameter set
  sl_wisun_params_eapol_t eapol;
  /// RPL parameter set
  sl_wisun_params_rpl_t rpl;
  /// MPL parameter set
  sl_wisun_params_mpl_t mpl;
  /// DHCP parameter set
  sl_wisun_params_dhcp_t dhcp;
  /// LFN parent parameter set
  sl_wisun_params_lfn_parent_t lfn_parent;
  /// Misc parameter set
  sl_wisun_params_misc_t misc;
  /// Direct Connect Authentication parameter set
  sl_wisun_params_eapol_t direct_connect_eapol;
  /// Traffic parameter set
  sl_wisun_params_traffic_t traffic;
  /// MAC parameter set
  sl_wisun_mac_params_t mac;
} SL_ATTRIBUTE_PACKED sl_wisun_connection_params_t;
SL_PACK_END()

/**************************************************************************//**
 * @addtogroup SL_WISUN_FFN_PARAMETER_SETS Predefined FFN parameter sets
 *
 * Predefined FFN parameter sets for sl_wisun_set_connection_parameters().
 *
 * These parameter sets can be used either as-is or used as an initialization
 * value for an application-specific parameter set.
 *
 * @{
 *****************************************************************************/

/// Profile for development (shorter connection time)
static const sl_wisun_connection_params_t SL_WISUN_PARAMS_PROFILE_TEST = {
  .version = SL_WISUN_PARAMS_API_VERSION,
  .discovery = {
    .trickle_pa = {
      .imin_s = 15,
      .imax_s = 60,
      .k = 3
    },
    .trickle_pas = {
      .imin_s = 15,
      .imax_s = 60,
      .k = 3
    },
    .eapol_target_min_sens = DBM_TO_RSL_RANGE(-60),
    .allow_skip = true
  },
  .configuration = {
    .trickle_pc = {
      .imin_s = 15,
      .imax_s = 60,
      .k = 3
    },
    .trickle_pcs = {
      .imin_s = 15,
      .imax_s = 60,
      .k = 3
    }
  },
  .eapol = {
    .sec_prot_trickle = {
      .imin_s = 0,
      .imax_s = 0,
      .k = 0,
    },
    .pmk_lifetime_m = 0,
    .ptk_lifetime_m = 0,
    .sec_prot_retry_timeout_s = 0,
    .initial_key_min_s = 0,
    .initial_key_max_s = 3,
    .initial_key_retry_min_s = 18,
    .initial_key_retry_max_s = 0,
    .initial_key_retry_max_limit_s = 42,
    .temp_min_timeout_s = 0,
    .gtk_request_imin_m = 0,
    .gtk_request_imax_m = 0,
    .gtk_max_mismatch_m = 64,
    .lgtk_max_mismatch_m = 60,
    .sec_prot_trickle_expirations = 0,
    .initial_key_retry_limit = 3,
    .allow_skip = true
  },
  .rpl = {
    .dao_txalg = {
      .rand = 0.1f,
      .max_delay_s = 1,
      .irt_s = 15,
      .mrt_s = 0,
      .mrd_s = 0,
      .mrc = 3,
    },
    .dis_max_delay_first_s = 2,
    .dis_max_delay_s = 60,
    .init_parent_selection_s = 1,
    .etx_probe_period_max_s = 4,
    .address_registration_lifetime_s = 2220,
    .etx_samples_init = 2,
    .etx_samples_refresh = 4,
    .candidate_parents_max = 5,
    .parents_max = 2,
  },
  .mpl = {
    .trickle = {
      .imin_s = 6,
      .imax_s = 48,
      .k = 3,
    },
    .seed_set_entry_lifetime_s = 1800,
    .trickle_expirations = 3,
    .seed_id_type = 0,
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
  .lfn_parent = {
    .lfn_pan_timeout_m = 0,
    .lfn_lpc_retry_count = 5,
    .lfn_na_wait_duration_m = 0,
  },
  .misc = {
    .temp_link_min_timeout_s = 260,
    .pan_timeout_m = 30,
  },
  .direct_connect_eapol = {
    .pmk_lifetime_m = 0,
    .ptk_lifetime_m = 0,
    .sec_prot_retry_timeout_s = 0,
    .initial_key_min_s = 0,
    .initial_key_max_s = 3,
    .initial_key_retry_min_s = 10,
    .initial_key_retry_max_s = 0,
    .initial_key_retry_max_limit_s = 30,
    .gtk_request_imin_m = 0,
    .gtk_request_imax_m = 0,
    .gtk_max_mismatch_m = 64,
    .initial_key_retry_limit = 3,
    .allow_skip = false
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

/// Profile for certification testing
static const sl_wisun_connection_params_t SL_WISUN_PARAMS_PROFILE_CERTIF = {
  .version = SL_WISUN_PARAMS_API_VERSION,
  .discovery = {
    .trickle_pa = {
      .imin_s = 15,
      .imax_s = 60,
      .k = 3
    },
    .trickle_pas = {
      .imin_s = 15,
      .imax_s = 60,
      .k = 3
    },
    .eapol_target_min_sens = 255,
    .allow_skip = false
  },
  .configuration = {
    .trickle_pc = {
      .imin_s = 15,
      .imax_s = 60,
      .k = 3
    },
    .trickle_pcs = {
      .imin_s = 15,
      .imax_s = 60,
      .k = 3
    }
  },
  .eapol = {
    .sec_prot_trickle = {
      .imin_s = 0,
      .imax_s = 0,
      .k = 0,
    },
    .pmk_lifetime_m = 0,
    .ptk_lifetime_m = 0,
    .sec_prot_retry_timeout_s = 0,
    .initial_key_min_s = 0,
    .initial_key_max_s = 30,
    .initial_key_retry_min_s = 180,
    .initial_key_retry_max_s = 0,
    .initial_key_retry_max_limit_s = 420,
    .temp_min_timeout_s = 0,
    .gtk_request_imin_m = 0,
    .gtk_request_imax_m = 0,
    .gtk_max_mismatch_m = 1,
    .lgtk_max_mismatch_m = 1,
    .sec_prot_trickle_expirations = 0,
    .initial_key_retry_limit = 3,
    .allow_skip = true
  },
  .rpl = {
    .dao_txalg = {
      .rand = 0.1f,
      .max_delay_s = 1,
      .irt_s = 15,
      .mrt_s = 0,
      .mrd_s = 0,
      .mrc = 3,
    },
    .dis_max_delay_first_s = 2,
    .dis_max_delay_s = 180,
    .init_parent_selection_s = 10,
    .etx_probe_period_max_s = 4,
    .address_registration_lifetime_s = 2220,
    .etx_samples_init = 1,
    .etx_samples_refresh = 4,
    .candidate_parents_max = 8,
    .parents_max = 4,
  },
  .mpl = {
    .trickle = {
      .imin_s = 10,
      .imax_s = 80,
      .k = 3,
    },
    .seed_set_entry_lifetime_s = 1800,
    .trickle_expirations = 3,
    .seed_id_type = 0,
  },
  .dhcp = {
    .sol_txalg = {
      .rand = 0.1f,
      .max_delay_s = 60,
      .irt_s = 60,
      .mrt_s = HOUR_TO_SEC(1),
      .mrd_s = 0,
      .mrc = 3,
    },
  },
  .lfn_parent = {
    .lfn_pan_timeout_m = 0,
    .lfn_lpc_retry_count = 5,
    .lfn_na_wait_duration_m = 0,
  },
  .misc = {
    .temp_link_min_timeout_s = 260,
    .pan_timeout_m = 30,
  },
  .direct_connect_eapol = {
    .pmk_lifetime_m = 0,
    .ptk_lifetime_m = 0,
    .sec_prot_retry_timeout_s = 0,
    .initial_key_min_s = 0,
    .initial_key_max_s = 3,
    .initial_key_retry_min_s = 10,
    .initial_key_retry_max_s = 0,
    .initial_key_retry_max_limit_s = 30,
    .gtk_request_imin_m = 0,
    .gtk_request_imax_m = 0,
    .gtk_max_mismatch_m = 64,
    .initial_key_retry_limit = 3,
    .allow_skip = false
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

/// Profile for a small network
static const sl_wisun_connection_params_t SL_WISUN_PARAMS_PROFILE_SMALL = {
  .version = SL_WISUN_PARAMS_API_VERSION,
  .discovery = {
    .trickle_pa = {
      .imin_s = 15,
      .imax_s = 60,
      .k = 1
    },
    .trickle_pas = {
      .imin_s = 15,
      .imax_s = 60,
      .k = 1
    },
    .eapol_target_min_sens = DBM_TO_RSL_RANGE(-60),
    .allow_skip = true
  },
  .configuration = {
    .trickle_pc = {
      .imin_s = 15,
      .imax_s = 60,
      .k = 1
    },
    .trickle_pcs = {
      .imin_s = 15,
      .imax_s = 60,
      .k = 1
    }
  },
  .eapol = {
    .sec_prot_trickle = {
      .imin_s = 0,
      .imax_s = 0,
      .k = 0,
    },
    .pmk_lifetime_m = 0,
    .ptk_lifetime_m = 0,
    .sec_prot_retry_timeout_s = 0,
    .initial_key_min_s = 0,
    .initial_key_max_s = 60,
    .initial_key_retry_min_s = 60,
    .initial_key_retry_max_s = 0,
    .initial_key_retry_max_limit_s = 180,
    .temp_min_timeout_s = 0,
    .gtk_request_imin_m = 0,
    .gtk_request_imax_m = 0,
    .gtk_max_mismatch_m = 64,
    .lgtk_max_mismatch_m = 60,
    .sec_prot_trickle_expirations = 0,
    .initial_key_retry_limit = 3,
    .allow_skip = true
  },
  .rpl = {
    .dao_txalg = {
      .rand = 0.1f,
      .max_delay_s = 1,
      .irt_s = 15,
      .mrt_s = 0,
      .mrd_s = 0,
      .mrc = 3,
    },
    .dis_max_delay_first_s = 2,
    .dis_max_delay_s = 180,
    .init_parent_selection_s = 10,
    .etx_probe_period_max_s = 30,
    .address_registration_lifetime_s = 2220,
    .etx_samples_init = 2,
    .etx_samples_refresh = 4,
    .candidate_parents_max = 5,
    .parents_max = 2,
  },
  .mpl = {
    .trickle = {
      .imin_s = 1,
      .imax_s = 10,
      .k = 8,
    },
    .seed_set_entry_lifetime_s = 180,
    .trickle_expirations = 2,
    .seed_id_type = 0,
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
  .lfn_parent = {
    .lfn_pan_timeout_m = 0,
    .lfn_lpc_retry_count = 5,
    .lfn_na_wait_duration_m = 0,
  },
  .misc = {
    .temp_link_min_timeout_s = 260,
    .pan_timeout_m = 30,
  },
  .direct_connect_eapol = {
    .pmk_lifetime_m = 0,
    .ptk_lifetime_m = 0,
    .sec_prot_retry_timeout_s = 0,
    .initial_key_min_s = 0,
    .initial_key_max_s = 3,
    .initial_key_retry_min_s = 10,
    .initial_key_retry_max_s = 0,
    .initial_key_retry_max_limit_s = 30,
    .gtk_request_imin_m = 0,
    .gtk_request_imax_m = 0,
    .gtk_max_mismatch_m = 64,
    .initial_key_retry_limit = 3,
    .allow_skip = false
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
  }
};

/// Profile for a medium network
static const sl_wisun_connection_params_t SL_WISUN_PARAMS_PROFILE_MEDIUM = {
  .version = SL_WISUN_PARAMS_API_VERSION,
  .discovery = {
    .trickle_pa = {
      .imin_s = 60,
      .imax_s = 960,
      .k = 1
    },
    .trickle_pas = {
      .imin_s = 60,
      .imax_s = 960,
      .k = 1
    },
    .eapol_target_min_sens = DBM_TO_RSL_RANGE(-60),
    .allow_skip = true
  },
  .configuration = {
    .trickle_pc = {
      .imin_s = 60,
      .imax_s = 960,
      .k = 1
    },
    .trickle_pcs = {
      .imin_s = 60,
      .imax_s = 960,
      .k = 1
    }
  },
  .eapol = {
    .sec_prot_trickle = {
      .imin_s = 0,
      .imax_s = 0,
      .k = 0,
    },
    .pmk_lifetime_m = 0,
    .ptk_lifetime_m = 0,
    .sec_prot_retry_timeout_s = 0,
    .initial_key_min_s = 0,
    .initial_key_max_s = 180,
    .initial_key_retry_min_s = 180,
    .initial_key_retry_max_s = 0,
    .initial_key_retry_max_limit_s = 720,
    .temp_min_timeout_s = 0,
    .gtk_request_imin_m = 0,
    .gtk_request_imax_m = 0,
    .gtk_max_mismatch_m = 64,
    .lgtk_max_mismatch_m = 60,
    .sec_prot_trickle_expirations = 0,
    .initial_key_retry_limit = 4,
    .allow_skip = true
  },
  .rpl = {
    .dao_txalg = {
      .rand = 0.1f,
      .max_delay_s = 10,
      .irt_s = 100,
      .mrt_s = 0,
      .mrd_s = 0,
      .mrc = 3,
    },
    .dis_max_delay_first_s = 60,
    .dis_max_delay_s = 180,
    .init_parent_selection_s = 10,
    .etx_probe_period_max_s = 60,
    .address_registration_lifetime_s = 2220,
    .etx_samples_init = 2,
    .etx_samples_refresh = 4,
    .candidate_parents_max = 5,
    .parents_max = 2,
  },
  .mpl = {
    .trickle = {
      .imin_s = 1,
      .imax_s = 32,
      .k = 8,
    },
    .seed_set_entry_lifetime_s = 576,
    .trickle_expirations = 2,
    .seed_id_type = 0,
  },
  .dhcp = {
    .sol_txalg = {
      .rand = 0.1f,
      .max_delay_s = 60,
      .irt_s = 60,
      .mrt_s = HOUR_TO_SEC(1),
      .mrd_s = 0,
      .mrc = 3,
    },
  },
  .lfn_parent = {
    .lfn_pan_timeout_m = 0,
    .lfn_lpc_retry_count = 20,
    .lfn_na_wait_duration_m = 0,
  },
  .misc = {
    .temp_link_min_timeout_s = 260,
    .pan_timeout_m = 60,
  },
  .direct_connect_eapol = {
    .pmk_lifetime_m = 0,
    .ptk_lifetime_m = 0,
    .sec_prot_retry_timeout_s = 0,
    .initial_key_min_s = 0,
    .initial_key_max_s = 3,
    .initial_key_retry_min_s = 10,
    .initial_key_retry_max_s = 0,
    .initial_key_retry_max_limit_s = 30,
    .gtk_request_imin_m = 0,
    .gtk_request_imax_m = 0,
    .gtk_max_mismatch_m = 64,
    .initial_key_retry_limit = 3,
    .allow_skip = false
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
  }
};

/// Profile for a large network
static const sl_wisun_connection_params_t SL_WISUN_PARAMS_PROFILE_LARGE = {
  .version = SL_WISUN_PARAMS_API_VERSION,
  .discovery = {
    .trickle_pa = {
      .imin_s = 120,
      .imax_s = 1536,
      .k = 1
    },
    .trickle_pas = {
      .imin_s = 120,
      .imax_s = 1536,
      .k = 1
    },
    .eapol_target_min_sens = DBM_TO_RSL_RANGE(-60),
    .allow_skip = true
  },
  .configuration = {
    .trickle_pc = {
      .imin_s = 120,
      .imax_s = 1536,
      .k = 1
    },
    .trickle_pcs = {
      .imin_s = 120,
      .imax_s = 1536,
      .k = 1
    }
  },
  .eapol = {
    .sec_prot_trickle = {
      .imin_s = 0,
      .imax_s = 0,
      .k = 0,
    },
    .pmk_lifetime_m = 0,
    .ptk_lifetime_m = 0,
    .sec_prot_retry_timeout_s = 0,
    .initial_key_min_s = 0,
    .initial_key_max_s = 300,
    .initial_key_retry_min_s = 300,
    .initial_key_retry_max_s = 0,
    .initial_key_retry_max_limit_s = 900,
    .temp_min_timeout_s = 0,
    .gtk_request_imin_m = 0,
    .gtk_request_imax_m = 0,
    .gtk_max_mismatch_m = 64,
    .lgtk_max_mismatch_m = 60,
    .sec_prot_trickle_expirations = 0,
    .initial_key_retry_limit = 5,
    .allow_skip = true
  },
  .rpl = {
    .dao_txalg = {
      .rand = 0.1f,
      .max_delay_s = 20,
      .irt_s = 200,
      .mrt_s = 0,
      .mrd_s = 0,
      .mrc = 3,
    },
    .dis_max_delay_first_s = 60,
    .dis_max_delay_s = 180,
    .init_parent_selection_s = 10,
    .etx_probe_period_max_s = 60,
    .address_registration_lifetime_s = 2220,
    .etx_samples_init = 2,
    .etx_samples_refresh = 4,
    .candidate_parents_max = 5,
    .parents_max = 2,
  },
  .mpl = {
    .trickle = {
      .imin_s = 5,
      .imax_s = 40,
      .k = 8,
    },
    .seed_set_entry_lifetime_s = 720,
    .trickle_expirations = 2,
    .seed_id_type = 0,
  },
  .dhcp = {
    .sol_txalg = {
      .rand = 0.1f,
      .max_delay_s = 60,
      .irt_s = 60,
      .mrt_s = HOUR_TO_SEC(1),
      .mrd_s = 0,
      .mrc = 3,
    },
  },
  .lfn_parent = {
    .lfn_pan_timeout_m = 0,
    .lfn_lpc_retry_count = 60,
    .lfn_na_wait_duration_m = 0,
  },
  .misc = {
    .temp_link_min_timeout_s = 520,
    .pan_timeout_m = 90,
  },
  .direct_connect_eapol = {
    .pmk_lifetime_m = 0,
    .ptk_lifetime_m = 0,
    .sec_prot_retry_timeout_s = 0,
    .initial_key_min_s = 0,
    .initial_key_max_s = 3,
    .initial_key_retry_min_s = 10,
    .initial_key_retry_max_s = 0,
    .initial_key_retry_max_limit_s = 30,
    .gtk_request_imin_m = 0,
    .gtk_request_imax_m = 0,
    .gtk_max_mismatch_m = 64,
    .initial_key_retry_limit = 3,
    .allow_skip = false
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
  }
};

/** @} */

/** @} (end SL_WISUN_TYPES) */

#endif
