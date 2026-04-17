/***************************************************************************//**
 * @file
 * @brief The Z-Wave specific header file for the RAIL library.
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

#ifndef SL_RAIL_ZWAVE_H
#define SL_RAIL_ZWAVE_H

#include "sl_rail_types.h"

#ifdef __cplusplus
extern "C" {
#endif

///
/// @addtogroup Z_Wave Z-Wave
/// @ingroup Protocol_Specific
/// @brief Z-Wave configuration routines
///
/// The functions in this group configure RAIL Z-Wave hardware
/// acceleration features.
///
/// To configure Z-Wave functionality, the application must first set up
/// a RAIL instance with \ref sl_rail_init() and other setup functions.
/// @code{.c}
/// sl_rail_zwave_beam_details_t g_recent_beam_details;
///
/// // Main SL_RAIL_EVENT callback
/// static void rail_events_callback(sl_rail_handle_t rail_handle,
///                                  sl_rail_events_t events)
/// {
///   // Get beam details from the beam packet
///   if (events & SL_RAIL_EVENT_ZWAVE_BEAM) {
///     if (sl_rail_zwave_is_enabled(rail_handle)) {
///       if (sl_rail_zwave_get_rx_beam_details(rail_handle, &g_recent_beam_details)
///           != SL_RAIL_STATUS_NO_ERROR)
///         return;
///       }
///     }
///   }
/// }
///
/// static const sl_rail_zwave_config_t zwave_config = {
///   .options = SL_RAIL_ZWAVE_OPTIONS_DEFAULT
/// };
///
/// void zwave_init(void)
/// {
///   // initialize Z-Wave
///   sl_rail_status_t status = sl_rail_zwave_init(rail_handle, &zwave_config);
///   assert(status == SL_RAIL_STATUS_NO_ERROR);
///
///   uint8_t my_home_id[4] = { 0xDE, 0xAD, 0xBE, 0xEF };
///   sl_rail_zwave_set_node_id(rail_handle, SL_RAIL_ZWAVE_NODE_ID_DEFAULT);
///   sl_rail_zwave_set_home_id(rail_handle, my_home_id, SL_RAIL_ZWAVE_HOME_ID_HASH_DONT_CARE);
///
///   // configure region to EU(European Union)
///   status = sl_rail_zwave_config_region(rail_handle, SL_RAIL_ZWAVE_REGION_EU);
///   assert(status == SL_RAIL_STATUS_NO_ERROR);
/// }
/// @endcode
///
/// @{
///

/**
 * @enum sl_rail_zwave_options_t
 * @brief Z-Wave options.
 */
SLI_RAIL_ENUM_GENERIC(sl_rail_zwave_options_t, uint32_t) {
  /**
   * Shift position of \ref SL_RAIL_ZWAVE_OPTION_PROMISCUOUS_MODE bit.
   */
  SL_RAIL_ZWAVE_OPTION_PROMISCUOUS_MODE_SHIFT = 0,
  /**
   * Shift position of \ref SL_RAIL_ZWAVE_OPTION_DETECT_BEAM_FRAMES bit.
   */
  SL_RAIL_ZWAVE_OPTION_DETECT_BEAM_FRAMES_SHIFT = 1,
  /**
   * Shift position of \ref SL_RAIL_ZWAVE_OPTION_NODE_ID_FILTERING bit.
   */
  SL_RAIL_ZWAVE_OPTION_NODE_ID_FILTERING_SHIFT = 2,
  /**
   * Shift position of \ref SL_RAIL_ZWAVE_OPTION_PROMISCUOUS_BEAM_MODE bit.
   */
  SL_RAIL_ZWAVE_OPTION_PROMISCUOUS_BEAM_MODE_SHIFT = 3,
};

/**
 * A value representing no options.
 */
#define SL_RAIL_ZWAVE_OPTIONS_NONE 0U

/**
 * All options are disabled by default.
 */
#define SL_RAIL_ZWAVE_OPTIONS_DEFAULT SL_RAIL_ZWAVE_OPTIONS_NONE

/**
 * An option to configure promiscuous mode, accepting non-beam packets
 * regardless of their Home Id. By default packets are filtered by their Home Id.
 * When true, such filtering is disabled.
 */
#define SL_RAIL_ZWAVE_OPTION_PROMISCUOUS_MODE \
  (1u << SL_RAIL_ZWAVE_OPTION_PROMISCUOUS_MODE_SHIFT)

/**
 * An option to filter non-beam packets based on their Node Id when
 * \ref SL_RAIL_ZWAVE_OPTION_PROMISCUOUS_MODE is disabled.
 *
 * @note This option has no effect when
 *   \ref SL_RAIL_ZWAVE_OPTION_PROMISCUOUS_MODE is enabled.
 */
#define SL_RAIL_ZWAVE_OPTION_NODE_ID_FILTERING \
  (1u << SL_RAIL_ZWAVE_OPTION_NODE_ID_FILTERING_SHIFT)

/**
 * An option to configure beam frame recognition. By default beams are not
 * considered special and will be received as if they were normal Z-Wave
 * frames, assuredly triggering \ref SL_RAIL_EVENT_RX_FRAME_ERROR.
 * When true, beam frames that are broadcast or match the Node Id and
 * Home Id hash values will trigger \ref SL_RAIL_EVENT_ZWAVE_BEAM event.
 * (All beams additionally trigger \ref SL_RAIL_EVENT_RX_PACKET_ABORTED
 * regardless of Node Id / Home Id hash values.)
 *
 * @note This option takes precedence over \ref
 *   SL_RAIL_ZWAVE_OPTION_PROMISCUOUS_MODE when receiving a beam frame.
 *   For promiscuous beam handling see related
 *   \ref SL_RAIL_ZWAVE_OPTION_PROMISCUOUS_BEAM_MODE option.
 */
#define SL_RAIL_ZWAVE_OPTION_DETECT_BEAM_FRAMES \
  (1u << SL_RAIL_ZWAVE_OPTION_DETECT_BEAM_FRAMES_SHIFT)

/**
 * An option to receive all beams promiscuously when \ref
 * SL_RAIL_ZWAVE_OPTION_DETECT_BEAM_FRAMES is enabled.
 * When true, beam frames are received regardless of their Node Id or Home Id hash
 * resulting in \ref SL_RAIL_EVENT_ZWAVE_BEAM (and also \ref
 * SL_RAIL_EVENT_RX_PACKET_ABORTED) for each beam frame.
 *
 * @note This option has no effect when
 *   \ref SL_RAIL_ZWAVE_OPTION_DETECT_BEAM_FRAMES is disabled.
 */
#define SL_RAIL_ZWAVE_OPTION_PROMISCUOUS_BEAM_MODE \
  (1u << SL_RAIL_ZWAVE_OPTION_PROMISCUOUS_BEAM_MODE_SHIFT)

/**
 * A value representing all options.
 */
#define SL_RAIL_ZWAVE_OPTIONS_ALL 0xFFFFFFFFU

/**
 * @enum sl_rail_zwave_node_id_t
 * @brief A Z-Wave Node Id.
 *
 * This data type is 12 bits wide when using the ZWave Long Range PHY, and
 * 8 bits wide otherwise.
 *
 * @note When using the Long Range PHY, values 0xFA1..0xFFE are reserved.
 *   Otherwise, values 0xE9..0xFE are reserved.
 */
SLI_RAIL_ENUM_GENERIC(sl_rail_zwave_node_id_t, uint16_t) {
  /**
   * The unknown Node Id for uninitialized nodes.
   */
  SL_RAIL_ZWAVE_NODE_ID_NONE = 0x00U,
  /**
   * The broadcast Node Id.
   */
  SL_RAIL_ZWAVE_NODE_ID_BROADCAST = 0xFFU,
  /**
   * Default to the broadcast Node Id.
   */
  SL_RAIL_ZWAVE_NODE_ID_DEFAULT = SL_RAIL_ZWAVE_NODE_ID_BROADCAST,
  // All other values between 0x00 and 0xFE are valid Node Ids normally
  /**
   * The Long Range broadcast Node Id.
   */
  SL_RAIL_ZWAVE_NODE_ID_LONG_RANGE_BROADCAST = 0xFFFU,
  /**
   * Default to the Long Range broadcast Node Id.
   */
  SL_RAIL_ZWAVE_NODE_ID_LONG_RANGE_DEFAULT = SL_RAIL_ZWAVE_NODE_ID_LONG_RANGE_BROADCAST,
  // All values from 0x001 to 0xFA1 are valid Node Ids with a Long Range PHY.
};

#ifndef DOXYGEN_SHOULD_SKIP_THIS
// Self-referencing defines minimize compiler complaints when using SLI_RAIL_ENUM
#define SL_RAIL_ZWAVE_NODE_ID_NONE                 ((sl_rail_zwave_node_id_t) SL_RAIL_ZWAVE_NODE_ID_NONE)
#define SL_RAIL_ZWAVE_NODE_ID_BROADCAST            ((sl_rail_zwave_node_id_t) SL_RAIL_ZWAVE_NODE_ID_BROADCAST)
#define SL_RAIL_ZWAVE_NODE_ID_DEFAULT              ((sl_rail_zwave_node_id_t) SL_RAIL_ZWAVE_NODE_ID_DEFAULT)
#define SL_RAIL_ZWAVE_NODE_ID_LONG_RANGE_BROADCAST ((sl_rail_zwave_node_id_t) SL_RAIL_ZWAVE_NODE_ID_LONG_RANGE_BROADCAST)
#define SL_RAIL_ZWAVE_NODE_ID_LONG_RANGE_DEFAULT   ((sl_rail_zwave_node_id_t) SL_RAIL_ZWAVE_NODE_ID_LONG_RANGE_DEFAULT)
#endif //DOXYGEN_SHOULD_SKIP_THIS

/**
 * @enum sl_rail_zwave_home_id_t
 * @brief A Z-Wave Home Id.
 *
 * @note Home Ids in the range 0x54000000..0x55FFFFFF are illegal.
 */
SLI_RAIL_ENUM_GENERIC(sl_rail_zwave_home_id_t, uint32_t) {
  /**
   * The unknown Home Id.
   */
  SL_RAIL_ZWAVE_HOME_ID_UNKNOWN = 0x00000000U,
  /**
   * An impossible and unlikely Home Id.
   */
  SL_RAIL_ZWAVE_HOME_ID_DEFAULT = 0x54545454U,
};

#ifndef DOXYGEN_SHOULD_SKIP_THIS
// Self-referencing defines minimize compiler complaints when using SLI_RAIL_ENUM
#define SL_RAIL_ZWAVE_HOME_ID_UNKNOWN ((sl_rail_zwave_home_id_t) SL_RAIL_ZWAVE_HOME_ID_UNKNOWN)
#define SL_RAIL_ZWAVE_HOME_ID_DEFAULT ((sl_rail_zwave_home_id_t) SL_RAIL_ZWAVE_HOME_ID_DEFAULT)
#endif //DOXYGEN_SHOULD_SKIP_THIS

/**
 * @enum sl_rail_zwave_home_id_hash_t
 * @brief A Z-Wave Home Id hash.
 *
 * @note Certain values (as shown) are illegal.
 */
SLI_RAIL_ENUM(sl_rail_zwave_home_id_hash_t) {
  /**
   * An illegal Home Id hash value.
   */
  SL_RAIL_ZWAVE_HOME_ID_HASH_ILLEGAL_0 = 0x0AU,
  /**
   * An illegal Home Id hash value.
   */
  SL_RAIL_ZWAVE_HOME_ID_HASH_ILLEGAL_1 = 0x4AU,
  /**
   * An illegal Home Id hash value.
   */
  SL_RAIL_ZWAVE_HOME_ID_HASH_ILLEGAL_2 = 0x55U,
  /**
   * Illegal Home Id hash value that suppresses checking the
   * Home Id hash field of beam packets.
   */
  SL_RAIL_ZWAVE_HOME_ID_HASH_DONT_CARE = 0x55U,
  /**
   * Default to don't care.
   */
  SL_RAIL_ZWAVE_HOME_ID_HASH_DEFAULT = SL_RAIL_ZWAVE_HOME_ID_HASH_DONT_CARE,
};

#ifndef DOXYGEN_SHOULD_SKIP_THIS
// Self-referencing defines minimize compiler complaints when using SLI_RAIL_ENUM
#define SL_RAIL_ZWAVE_HOME_ID_HASH_ILLEGAL_0 ((sl_rail_zwave_home_id_hash_t) SL_RAIL_ZWAVE_HOME_ID_HASH_ILLEGAL_0)
#define SL_RAIL_ZWAVE_HOME_ID_HASH_ILLEGAL_1 ((sl_rail_zwave_home_id_hash_t) SL_RAIL_ZWAVE_HOME_ID_HASH_ILLEGAL_1)
#define SL_RAIL_ZWAVE_HOME_ID_HASH_ILLEGAL_2 ((sl_rail_zwave_home_id_hash_t) SL_RAIL_ZWAVE_HOME_ID_HASH_ILLEGAL_2)
#define SL_RAIL_ZWAVE_HOME_ID_HASH_DONT_CARE ((sl_rail_zwave_home_id_hash_t) SL_RAIL_ZWAVE_HOME_ID_HASH_DONT_CARE)
#define SL_RAIL_ZWAVE_HOME_ID_HASH_DEFAULT   ((sl_rail_zwave_home_id_hash_t) SL_RAIL_ZWAVE_HOME_ID_HASH_DEFAULT)
#endif //DOXYGEN_SHOULD_SKIP_THIS

/**
 * @struct sl_rail_zwave_config_t
 * @brief A configuration structure for Z-Wave in RAIL.
 */
typedef struct sl_rail_zwave_config {
  /**
   * Defines Z-Wave options.
   */
  sl_rail_zwave_options_t options;
  /**
   * Defines Z-Wave Acking configuration.
   */
  sl_rail_auto_ack_config_t ack_config;
  /**
   * Defines state timings for Z-Wave.
   */
  sl_rail_state_timing_t timings;
} sl_rail_zwave_config_t;

/**
 * @enum sl_rail_zwave_baud_t
 * @brief Z-Wave supported baud rates or PHYs.
 */
SLI_RAIL_ENUM(sl_rail_zwave_baud_t) {
  /**
   * 9.6 kbps baud rate.
   */
  SL_RAIL_ZWAVE_BAUD_9600 = 0u,
  /**
   * 40 kbps baud rate.
   */
  SL_RAIL_ZWAVE_BAUD_40_K = 1u,
  /**
   * 100 kbps baud rate.
   */
  SL_RAIL_ZWAVE_BAUD_100_K = 2u,
  /**
   * Long Range PHY.
   */
  SL_RAIL_ZWAVE_BAUD_LR = 3u,
  /**
   * Sentinel value for invalid baud rate. Must be last.
   */
  SL_RAIL_ZWAVE_BAUD_INVALID
};

#ifndef DOXYGEN_SHOULD_SKIP_THIS
// Self-referencing defines minimize compiler complaints when using SLI_RAIL_ENUM
#define SL_RAIL_ZWAVE_BAUD_9600    ((sl_rail_zwave_baud_t) SL_RAIL_ZWAVE_BAUD_9600)
#define SL_RAIL_ZWAVE_BAUD_40_K    ((sl_rail_zwave_baud_t) SL_RAIL_ZWAVE_BAUD_40_K)
#define SL_RAIL_ZWAVE_BAUD_100_K   ((sl_rail_zwave_baud_t) SL_RAIL_ZWAVE_BAUD_100_K)
#define SL_RAIL_ZWAVE_BAUD_LR      ((sl_rail_zwave_baud_t) SL_RAIL_ZWAVE_BAUD_LR)
#define SL_RAIL_ZWAVE_BAUD_INVALID ((sl_rail_zwave_baud_t) SL_RAIL_ZWAVE_BAUD_INVALID)
#endif //DOXYGEN_SHOULD_SKIP_THIS

/**
 * @enum sl_rail_zwave_phy_t
 * @brief The Z-Wave PHY identifier (aka PTI radio configuration id).
 */
SLI_RAIL_ENUM(sl_rail_zwave_phy_t) {
  /* Z-Wave 9.6 kbps 908 MHz PHY. */
  SL_RAIL_ZWAVE_PHY_9P6_KBPS = 0u,
  /* Z-Wave 9.6 kbps 908 MHz BCR Concurrent PHY. */
  SL_RAIL_ZWAVE_PHY_9P6_KBPS_CONCURRENT = 1u,
  /* Z-Wave 40 kbps 908 MHz Viterbi Concurrent PHY. */
  SL_RAIL_ZWAVE_PHY_40_KBPS = 2u,
  /* Z-Wave 40 kbps 908 MHz Viterbi Beam PHY. */
  SL_RAIL_ZWAVE_PHY_40_KBPS_BEAM = 3u,
  /* Z-Wave 100 kbps 916 MHz Viterbi PHY. */
  SL_RAIL_ZWAVE_PHY_100_KBPS = 4u,
  /* Z-Wave 100 kbps 916 MHz Viterbi Lowside PHY. */
  SL_RAIL_ZWAVE_PHY_100_KBPS_LOWSIDE = 5u,
  /* Z-Wave 100 kbps 916 MHz Viterbi Beam PHY. */
  SL_RAIL_ZWAVE_PHY_100_KBPS_BEAM = 6u,
  /* Z-Wave 100 kbps 916 MHz Viterbi Lowside Beam PHY. */
  SL_RAIL_ZWAVE_PHY_100_KBPS_LOWSIDE_BEAM = 7u,
  /* Z-Wave 100 kbps 912 MHz OQPSK (Long Range) PHY. */
  SL_RAIL_ZWAVE_PHY_LONG_RANGE = 8u,
  /* Z-Wave 100 kbps 912 MHz OQPSK (Long Range) Beam PHY. */
  SL_RAIL_ZWAVE_PHY_LONG_RANGE_BEAM = 9u,
};

#ifndef DOXYGEN_SHOULD_SKIP_THIS
// Self-referencing defines minimize compiler complaints when using SLI_RAIL_ENUM
#define SL_RAIL_ZWAVE_PHY_9P6_KBPS              ((sl_rail_zwave_phy_t) SL_RAIL_ZWAVE_PHY_9P6_KBPS)
#define SL_RAIL_ZWAVE_PHY_9P6_KBPS_CONCURRENT   ((sl_rail_zwave_phy_t) SL_RAIL_ZWAVE_PHY_9P6_KBPS_CONCURRENT)
#define SL_RAIL_ZWAVE_PHY_40_KBPS               ((sl_rail_zwave_phy_t) SL_RAIL_ZWAVE_PHY_40_KBPS)
#define SL_RAIL_ZWAVE_PHY_40_KBPS_BEAM          ((sl_rail_zwave_phy_t) SL_RAIL_ZWAVE_PHY_40_KBPS_BEAM)
#define SL_RAIL_ZWAVE_PHY_100_KBPS              ((sl_rail_zwave_phy_t) SL_RAIL_ZWAVE_PHY_100_KBPS)
#define SL_RAIL_ZWAVE_PHY_100_KBPS_LOWSIDE      ((sl_rail_zwave_phy_t) SL_RAIL_ZWAVE_PHY_100_KBPS_LOWSIDE)
#define SL_RAIL_ZWAVE_PHY_100_KBPS_BEAM         ((sl_rail_zwave_phy_t) SL_RAIL_ZWAVE_PHY_100_KBPS_BEAM)
#define SL_RAIL_ZWAVE_PHY_100_KBPS_LOWSIDE_BEAM ((sl_rail_zwave_phy_t) SL_RAIL_ZWAVE_PHY_100_KBPS_LOWSIDE_BEAM)
#define SL_RAIL_ZWAVE_PHY_LONG_RANGE            ((sl_rail_zwave_phy_t) SL_RAIL_ZWAVE_PHY_LONG_RANGE)
#define SL_RAIL_ZWAVE_PHY_LONG_RANGE_BEAM       ((sl_rail_zwave_phy_t) SL_RAIL_ZWAVE_PHY_LONG_RANGE_BEAM)
#endif //DOXYGEN_SHOULD_SKIP_THIS

#ifndef DOXYGEN_UNDOCUMENTED
/**
 * @enum sl_rail_zwave_region_options_t
 * @brief Region Specific Physical.
 */
SLI_RAIL_ENUM(sl_rail_zwave_region_options_t) {
  /**
   * Bit shift for US Long Range 3.
   */
  SL_RAIL_ZWAVE_REGION_OPTION_LONG_RANGE_3_SHIFT = 0,
  /**
   * Bit shift for special low side config, mostly for Japan and Korea.
   */
  SL_RAIL_ZWAVE_REGION_OPTION_LOW_SIDE_SHIFT = 1,
  /**
   * Bit shift for US long range range configurations.
   */
  SL_RAIL_ZWAVE_REGION_OPTION_LONG_RANGE_SHIFT = 2,
};

/*
 * sl_rail_zwave_region_options_t bitmasks.
 */
/**
 * A value representing US Long Range regions.
 */
#define SL_RAIL_ZWAVE_REGION_OPTION_LONG_RANGE_MASK  (1u << SL_RAIL_ZWAVE_REGION_OPTION_LONG_RANGE_SHIFT)
/**
 * A value representing lowside configurations: JP and KR.
 */
#define SL_RAIL_ZWAVE_REGION_OPTION_LOW_SIDE_MASK (1u << SL_RAIL_ZWAVE_REGION_OPTION_LOW_SIDE_SHIFT)
/**
 * A value representing Long Range 3 region.
 */
#define SL_RAIL_ZWAVE_REGION_OPTION_LONG_RANGE_3_MASK (1u << SL_RAIL_ZWAVE_REGION_OPTION_LONG_RANGE_3_SHIFT)
/**
 * A value representing No bit to be enabled.
 */
#define SL_RAIL_ZWAVE_REGION_OPTIONS_NONE 0u
#endif //DOXYGEN_UNDOCUMENTED

/**
 * Sentinel value to indicate that a channel (and thus its frequency)
 * are invalid.
 */
#define SL_RAIL_ZWAVE_FREQ_INVALID 0xFFFFFFFFUL

/**
 * @enum sl_rail_zwave_region_id_t
 * @brief Z-Wave region identifications.
 */
SLI_RAIL_ENUM(sl_rail_zwave_region_id_t) {
  /**
   * Unknown/Invalid.
   */
  SL_RAIL_ZWAVE_REGION_ID_UNKNOWN = 0u,
  /**
   * European Union.
   */
  SL_RAIL_ZWAVE_REGION_ID_EU = 1u,
  /**
   * United States.
   */
  SL_RAIL_ZWAVE_REGION_ID_US = 2u,
  /**
   * Australia/New Zealand.
   */
  SL_RAIL_ZWAVE_REGION_ID_ANZ = 3u,
  /**
   * Hong Kong.
   */
  SL_RAIL_ZWAVE_REGION_ID_HK = 4u,
  /**
   * Malaysia.
   */
  SL_RAIL_ZWAVE_REGION_ID_MY = 5u,
  /**
   * India.
   */
  SL_RAIL_ZWAVE_REGION_ID_IN = 6u,
  /**
   * Japan.
   */
  SL_RAIL_ZWAVE_REGION_ID_JP = 7u,
  /**
   * Russian Federation.
   */
  SL_RAIL_ZWAVE_REGION_ID_RU = 8u,
  /**
   * Israel.
   */
  SL_RAIL_ZWAVE_REGION_ID_IL = 9u,
  /**
   * Korea.
   */
  SL_RAIL_ZWAVE_REGION_ID_KR = 10u,
  /**
   * China.
   */
  SL_RAIL_ZWAVE_REGION_ID_CN = 11u,
  /**
   * United States, with first long range PHY.
   */
  SL_RAIL_ZWAVE_REGION_ID_US_LR_1 = 12u,
  /**
   * United States, with second long range PHY.
   */
  SL_RAIL_ZWAVE_REGION_ID_US_LR_2 = 13u,
  /**
   * United States, with third long range PHY.
   */
  SL_RAIL_ZWAVE_REGION_ID_US_LR_3 = 14u,
  /**
   * European Union, with first long range PHY.
   */
  SL_RAIL_ZWAVE_REGION_ID_EU_LR_1 = 15u,
  /**
   * European Union, with second long range PHY.
   */
  SL_RAIL_ZWAVE_REGION_ID_EU_LR_2 = 16u,
  /**
   * European Union, with third long range PHY.
   */
  SL_RAIL_ZWAVE_REGION_ID_EU_LR_3 = 17u,
  /**
   * Count of known regions. Must be last.
   */
  SL_RAIL_ZWAVE_REGION_ID_COUNT
};

#ifndef DOXYGEN_SHOULD_SKIP_THIS
// Self-referencing defines minimize compiler complaints when using SLI_RAIL_ENUM
#define SL_RAIL_ZWAVE_REGION_ID_UNKNOWN ((sl_rail_zwave_region_id_t) SL_RAIL_ZWAVE_REGION_ID_UNKNOWN)
#define SL_RAIL_ZWAVE_REGION_ID_EU      ((sl_rail_zwave_region_id_t) SL_RAIL_ZWAVE_REGION_ID_EU)
#define SL_RAIL_ZWAVE_REGION_ID_US      ((sl_rail_zwave_region_id_t) SL_RAIL_ZWAVE_REGION_ID_US)
#define SL_RAIL_ZWAVE_REGION_ID_ANZ     ((sl_rail_zwave_region_id_t) SL_RAIL_ZWAVE_REGION_ID_ANZ)
#define SL_RAIL_ZWAVE_REGION_ID_HK      ((sl_rail_zwave_region_id_t) SL_RAIL_ZWAVE_REGION_ID_HK)
#define SL_RAIL_ZWAVE_REGION_ID_MY      ((sl_rail_zwave_region_id_t) SL_RAIL_ZWAVE_REGION_ID_MY)
#define SL_RAIL_ZWAVE_REGION_ID_IN      ((sl_rail_zwave_region_id_t) SL_RAIL_ZWAVE_REGION_ID_IN)
#define SL_RAIL_ZWAVE_REGION_ID_JP      ((sl_rail_zwave_region_id_t) SL_RAIL_ZWAVE_REGION_ID_JP)
#define SL_RAIL_ZWAVE_REGION_ID_RU      ((sl_rail_zwave_region_id_t) SL_RAIL_ZWAVE_REGION_ID_RU)
#define SL_RAIL_ZWAVE_REGION_ID_IL      ((sl_rail_zwave_region_id_t) SL_RAIL_ZWAVE_REGION_ID_IL)
#define SL_RAIL_ZWAVE_REGION_ID_KR      ((sl_rail_zwave_region_id_t) SL_RAIL_ZWAVE_REGION_ID_KR)
#define SL_RAIL_ZWAVE_REGION_ID_CN      ((sl_rail_zwave_region_id_t) SL_RAIL_ZWAVE_REGION_ID_CN)
#define SL_RAIL_ZWAVE_REGION_ID_US_LR_1 ((sl_rail_zwave_region_id_t) SL_RAIL_ZWAVE_REGION_ID_US_LR_1)
#define SL_RAIL_ZWAVE_REGION_ID_US_LR_2 ((sl_rail_zwave_region_id_t) SL_RAIL_ZWAVE_REGION_ID_US_LR_2)
#define SL_RAIL_ZWAVE_REGION_ID_US_LR_3 ((sl_rail_zwave_region_id_t) SL_RAIL_ZWAVE_REGION_ID_US_LR_3)
#define SL_RAIL_ZWAVE_REGION_ID_EU_LR_1 ((sl_rail_zwave_region_id_t) SL_RAIL_ZWAVE_REGION_ID_EU_LR_1)
#define SL_RAIL_ZWAVE_REGION_ID_EU_LR_2 ((sl_rail_zwave_region_id_t) SL_RAIL_ZWAVE_REGION_ID_EU_LR_2)
#define SL_RAIL_ZWAVE_REGION_ID_EU_LR_3 ((sl_rail_zwave_region_id_t) SL_RAIL_ZWAVE_REGION_ID_EU_LR_3)
#define SL_RAIL_ZWAVE_REGION_ID_COUNT   ((sl_rail_zwave_region_id_t) SL_RAIL_ZWAVE_REGION_ID_COUNT)
#endif //DOXYGEN_SHOULD_SKIP_THIS

#ifndef DOXYGEN_UNDOCUMENTED
/**
 * Largest Ack timeout period based on
 * aPhyTurnaroundTimeRxTx (1 ms max) + (aMacTransferAckTimeTX (168 bits) * (1/data rate))
 * for the slowest Data Rate R1 (19.6 kbit/s).
 */
#define SL_RAIL_ZWAVE_MAX_ACK_TIMEOUT_US        (9600U)

/**
 * Idle-to-Rx transition timing.
 */
#define SL_RAIL_ZWAVE_TIME_IDLE_TO_RX_US        (100U)
/**
 * Tx-to-Rx transition timing.
 */
#define SL_RAIL_ZWAVE_TIME_TX_TO_RX_US          (0U)
/**
 * Idle-to-Tx transition timing.
 */
#define SL_RAIL_ZWAVE_TIME_IDLE_TO_TX_US        (0U)
/**
 * Rx-to-Tx transition timing.
 */
#define SL_RAIL_ZWAVE_TIME_RX_TO_TX_US          (1000U)
#endif//DOXYGEN_UNDOCUMENTED

/**
 * Invalid beam TX power value returned when \ref sl_rail_zwave_get_rx_beam_details()
 * is called after receiving a regular non-long-range beam.
 */
#define SL_RAIL_ZWAVE_LR_BEAM_TX_POWER_INVALID  (0xFFU)

/**
 * @struct sl_rail_zwave_lr_ack_data_t
 * @brief Configuration structure for Z-Wave Long Range Ack.
 */
typedef struct sl_rail_zwave_lr_ack_data {
  /**
   * Radio noise level measured on the channel the frame is transmitted on.
   */
  int8_t noise_floor_dbm;
  /**
   * Transmit power used to transmit the ongoing Z-Wave Long Range Ack.
   */
  int8_t transmit_power_dbm;
  /**
   * Signal strength measured while receiving the Z-Wave Long Range frame.
   */
  int8_t receive_rssi_dbm;
} sl_rail_zwave_lr_ack_data_t;

/**
 * @struct sl_rail_zwave_beam_rx_config_t
 * @brief Configuration structure for Z-Wave beam detection.
 *
 * @warning This structure should not be used without direct instruction
 *   by Silicon Labs. Appropriate defaults for this are built into
 *   the RAIL library.
 */
typedef struct sl_rail_zwave_beam_rx_config {
  /**
   * Channel hopping pattern to use for beam detection.
   */
  sl_rail_rx_channel_hopping_config_t channel_hopping_config;
  /**
   * Amount of time to spend trying to receive a beam once detected.
   * 100 kbps only.
   */
  sl_rail_rx_duty_cycle_config_t receive_config_100_kbps;
  /**
   * Amount of time to spend trying to receive a beam once detected.
   * 40 kbps only.
   */
  sl_rail_rx_duty_cycle_config_t receive_config_40_kbps;
} sl_rail_zwave_beam_rx_config_t;

/**
 * Number of channels in each of Z-Wave's region-based PHYs.
 */
#define SL_RAIL_NUM_ZWAVE_CHANNELS (4U)

/**
 * @struct sl_rail_zwave_region_config_t
 * @brief Each Z-Wave region supports 3 channels.
 */
typedef struct sl_rail_zwave_region_config {
  /**
   * Channel frequency in hertz.
   */
  uint32_t frequency_hz[SL_RAIL_NUM_ZWAVE_CHANNELS];
  /**
   * The maximum power allowed on the channel, in dBm.
   */
  sl_rail_tx_power_t max_power_dbm[SL_RAIL_NUM_ZWAVE_CHANNELS];
  /**
   * Channel baud rate index.
   */
  sl_rail_zwave_baud_t baud_rate[SL_RAIL_NUM_ZWAVE_CHANNELS];
  /**
   * Identification number for the region.
   */
  sl_rail_zwave_region_id_t region_id;
  /**
   * Encapsulates region-specific options.
   */
  sl_rail_zwave_region_options_t region_options;
} sl_rail_zwave_region_config_t;

/**
 * @struct sl_rail_zwave_beam_details_t
 * @brief Structure to get Z-Wave received beam related details.
 */
typedef struct sl_rail_zwave_beam_details {
  /**
   * Channel hopping index of the last received beam frame.
   */
  uint8_t channel_index;
  /**
   * Node Id of the last received beam frame.
   */
  sl_rail_zwave_node_id_t node_id;
  /**
   * Home Id hash of the last received beam frame.
   */
  sl_rail_zwave_home_id_hash_t home_id_hash;
  /**
   * RSSI in dBm of the last received beam frame.
   */
  int8_t rssi_dbm;
  /**
   * TX power value from within the last received Long Range beam.
   * The following table shows how to translate the Long Range Tx
   * power value to dBm:
   *
   * <table>
   * <tr><th>Tx Power Value <th>Description
   * <tr><td>0 <td>-6 dBm
   * <tr><td>1 <td>-2 dBm
   * <tr><td>2 <td>+2 dBm
   * <tr><td>3 <td>+6 dBm
   * <tr><td>4 <td>+10 dBm
   * <tr><td>5 <td>+13 dBm
   * <tr><td>6 <td>+16 dBm
   * <tr><td>7 <td>+19 dBm
   * <tr><td>8 <td>+21 dBm
   * <tr><td>9 <td>+23 dBm
   * <tr><td>10 <td>+25 dBm
   * <tr><td>11 <td>+26 dBm
   * <tr><td>12 <td>+27 dBm
   * <tr><td>13 <td>+28 dBm
   * <tr><td>14 <td>+29 dBm
   * <tr><td>15 <td>+30 dBm
   * </table>
   */
  uint8_t lr_tx_power;
} sl_rail_zwave_beam_details_t;

/**
 * @struct sl_rail_zwave_ir_cal_val_t
 * @brief Structure for Z-Wave Image Rejection Calibration.
 *
 * @note Index 0 will hold the low side image rejection calibration value (channel 0),
 *   while index 1 will hold the high side image rejection value (channel 1).
 */
typedef struct sl_rail_zwave_ir_cal_val {
  /**
   * Low side and high side image rejection values.
   */
  sl_rail_ir_cal_values_t image_rejection[2];
} sl_rail_zwave_ir_cal_val_t;

/**
 * @typedef sl_rail_zwave_rx_channel_hopping_parameters_t
 * @brief Rx channel hopping on-channel time for all Z-Wave channels in a region.
 */
typedef sl_rail_rx_channel_hopping_parameter_t sl_rail_zwave_rx_channel_hopping_parameters_t[SL_RAIL_NUM_ZWAVE_CHANNELS];

/**
 * Switch the Z-Wave region.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in] p_region_config A pointer to a Z-Wave channel configuration for the selected region.
 * @return Status code indicating success of the function call.
 *
 * @note Setting a new Z-Wave Region will default any Low Power values to
 *   Normal Power values for the region.
 *   Z-Wave Region configuration must always be followed by a Low Power setup
 *   in case one desires to have the Low Power Acking functionality.
 */
sl_rail_status_t sl_rail_zwave_config_region(sl_rail_handle_t rail_handle,
                                             const sl_rail_zwave_region_config_t *p_region_config);

/**
 * Perform image rejection calibration on all valid channels of a
 * Z-Wave region.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in,out] p_ir_cal_vals An application-provided pointer of
 *   type \ref sl_rail_zwave_ir_cal_val_t. This is populated with image rejection
 *   calibration values, if not NULL or initialized with
 *   \ref SL_RAIL_CAL_INVALID_VALUE or if force_ir_cal is true.
 * @param[in] force_ir_cal If true, will always perform image rejection calibration
 *   and not use previously cached values.
 * @return Status code indicating success of the function call.
 *
 * @note This function also calibrates for beam detection and should be
 *   called before \ref sl_rail_zwave_receive_beam() and after the Z-Wave region
 *   has been configured via \ref sl_rail_zwave_config_region().
 *   Channel hopping must be disabled otherwise this function will return
 *   \ref SL_RAIL_STATUS_INVALID_CALL.
 */
sl_rail_status_t sl_rail_zwave_perform_ir_cal(sl_rail_handle_t rail_handle,
                                              sl_rail_zwave_ir_cal_val_t *p_ir_cal_vals,
                                              bool force_ir_cal);

/**
 * Initialize RAIL for Z-Wave features.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in] p_config A pointer to a Z-Wave configuration structure.
 * @return Status code indicating success of the function call.
 *
 * This function is the entry point for working with Z-Wave within
 * RAIL. It sets up relevant hardware acceleration for Z-Wave-specific
 * features, such as Home Id filtering and beam packets (as
 * specified in the configuration) and allows users to select the
 * relevant Z-Wave region-specific PHY via \ref sl_rail_zwave_config_region().
 */
sl_rail_status_t sl_rail_zwave_init(sl_rail_handle_t rail_handle,
                                    const sl_rail_zwave_config_t *p_config);

/**
 * De-initialize Z-Wave hardware acceleration.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @return Status code indicating success of the function call.
 *
 * Disables and resets all Z-Wave hardware acceleration features. This
 * function should only be called when the radio is idle.
 */
sl_rail_status_t sl_rail_zwave_deinit(sl_rail_handle_t rail_handle);

/**
 * Return whether Z-Wave hardware acceleration is currently enabled.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @return true if Z-Wave hardware acceleration was enabled to start with
 *   and false otherwise.
 */
bool sl_rail_zwave_is_enabled(sl_rail_handle_t rail_handle);

/**
 * Configure Z-Wave options.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in] mask A bitmask containing which options should be modified.
 * @param[in] options A bitmask containing desired configuration settings.
 *   Bit positions for each option are found in the \ref sl_rail_zwave_options_t.
 * @return Status code indicating success of the function call.
 */
sl_rail_status_t sl_rail_zwave_config_options(sl_rail_handle_t rail_handle,
                                              sl_rail_zwave_options_t mask,
                                              sl_rail_zwave_options_t options);

/**
 * Inform RAIL of the Z-Wave node's Node Id for receive filtering.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in] node_id A Z-Wave Node Id.
 * @return Status code indicating success of the function call.
 *
 * @note Until this API is called, RAIL will assume the Node Id is
 *   \ref SL_RAIL_ZWAVE_NODE_ID_DEFAULT.
 */
sl_rail_status_t sl_rail_zwave_set_node_id(sl_rail_handle_t rail_handle,
                                           sl_rail_zwave_node_id_t node_id);

/**
 * Inform RAIL of the Z-Wave node's Home Id and its hash for receive filtering.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in] home_id A Z-Wave Home Id.
 * @param[in] home_id_hash The hash of the home_id expected in beam frames.
 *   If this is \ref SL_RAIL_ZWAVE_HOME_ID_HASH_DONT_CARE, beam frame detection
 *   will not check the Home Id hash in a received beam frame at all, and
 *   \ref SL_RAIL_EVENT_ZWAVE_BEAM will trigger based solely on the Node Id
 *   in the beam frame.
 * @return Status code indicating success of the function call.
 *
 * @note Until this API is called, RAIL will assume the Home Id is an
 *   illegal one of \ref SL_RAIL_ZWAVE_HOME_ID_DEFAULT, and its hash is \ref
 *   SL_RAIL_ZWAVE_HOME_ID_HASH_DONT_CARE.
 */
sl_rail_status_t sl_rail_zwave_set_home_id(sl_rail_handle_t rail_handle,
                                           sl_rail_zwave_home_id_t home_id,
                                           sl_rail_zwave_home_id_hash_t home_id_hash);

/**
 * Get detailed information about a Z-Wave beam frame that triggered
 * \ref SL_RAIL_EVENT_ZWAVE_BEAM.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[out] p_beam_details An application provided pointer to
 *   \ref sl_rail_zwave_beam_details_t to populate the Node Id, Home Id hash,
 *   channel hopping index, RSSI, and TX Power (for long range beam only)
 *   of the received beam frame.
 * @return Status code indicating success of the function call.
 *
 * @note This is best called while handling the \ref SL_RAIL_EVENT_ZWAVE_BEAM
 *   event; if multiple beams are received only the most recent beam's
 *   details are provided.
 *
 * @note \ref sl_rail_zwave_beam_details_t::channel_index will be set to
 *   \ref SL_RAIL_CHANNEL_HOPPING_INVALID_INDEX if channel-hopping was off at the time
 *   the beam packet was received.
 *
 * @note \ref sl_rail_zwave_beam_details_t::lr_tx_power will be set to
 *   \ref SL_RAIL_ZWAVE_LR_BEAM_TX_POWER_INVALID if this API is called after
 *   receiving a regular non Long Range beam.
 */
sl_rail_status_t sl_rail_zwave_get_rx_beam_details(sl_rail_handle_t rail_handle,
                                                   sl_rail_zwave_beam_details_t *p_beam_details);

/**
 * Set the Low Power settings in deci-dBm.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in] power_ddbm Desired low power in deci-dBm.
 * @return Status code indicating success of the function call.
 *
 * Low Power settings are required during Ack transmissions when
 * the Low Power Bit is set. This setting is only valid for one
 * subsequent transmission, after which all transmissions will be
 * at the nominal power setting, until re-invoked.
 */
sl_rail_status_t sl_rail_zwave_set_tx_low_power_dbm(sl_rail_handle_t rail_handle,
                                                    sl_rail_tx_power_t power_ddbm);

/**
 * Get the TX low power in terms of deci-dBm.
 *
 * @param[in] rail_handle A RAIL instance handle.
 * @return The chip-specific \ref sl_rail_tx_power_t value of the low
 *   transmit power in deci-dBm.
 */
sl_rail_tx_power_t sl_rail_zwave_get_tx_low_power_dbm(sl_rail_handle_t rail_handle);

/**
 * Implement beam detection and reception algorithms.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[out] p_beam_detect_index A pointer to an indicator of whether or not a beam was detected
 *   at all, regardless of if it was received, generally for use only by instruction
 *   from Silicon Labs. Can be NULL.
 * @param[in] p_scheduler_info A pointer to information to allow the radio scheduler to place
 *   this operation appropriately. This is only used in multiprotocol version of
 *   RAIL and may be set to NULL in all other versions.
 *   Note that Z-Wave currently does not support multiprotocol, so this
 *   scheduler info exists to future proof the API for when it does.
 * @return Status code indicating success of the function call.
 *   Reasons for failure include an un-idled radio or a non-Japan non-Korea
 *   region configured before calling this function.
 *
 * This function takes care of all configuration and radio setup to
 * detect and receive beams in the current Z-Wave region.
 * If a beam is detected, RAIL will provide
 * the usual \ref SL_RAIL_EVENT_ZWAVE_BEAM event during which time users can
 * process the beam as expected. However, normal packets may also be
 * received during this time (also triggering \ref SL_RAIL_EVENTS_RX_COMPLETION
 * events), in which case, this API may need to be re-called to receive
 * a beam. Users should also listen for
 * \ref SL_RAIL_EVENT_RX_CHANNEL_HOPPING_COMPLETE, which will indicate
 * that no beam is heard. At that point, the radio will be automatically idled.
 * Until one of these events is received, users should not try to
 * reconfigure radio settings or start another radio operation. If an application
 * needs to do some other operation or configuration, it must first call
 * \ref sl_rail_idle() and wait for the radio to idle.
 *
 * @note: The radio must be idle before calling this function.
 *
 * @note: \ref sl_rail_config_rx_channel_hopping() must have been called successfully
 *   in Z-Wave before this function is called to provide a valid memory buffer
 *   for internal use (see \ref sl_rail_rx_channel_hopping_config_t::p_buffer).
 *
 * @note: This function alters radio functionality substantially. After calling
 *   it, the user should call \ref sl_rail_zwave_config_region(),
 *   \ref sl_rail_config_rx_channel_hopping(), \ref sl_rail_enable_rx_channel_hopping(),
 *   and \ref sl_rail_set_rx_transitions() to reset these parameters to whatever
 *   behaviors were desired before calling this function. Additionally,
 *   this function will idle the radio upon on exit.
 */
sl_rail_status_t sl_rail_zwave_receive_beam(sl_rail_handle_t rail_handle,
                                            uint8_t *p_beam_detect_index,
                                            const sl_rail_scheduler_info_t *p_scheduler_info);

/**
 * Configure the receive algorithm used in \ref sl_rail_zwave_receive_beam().
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in] p_config A pointer to a configuration for the beam detection algorithm.
 * @return Status code indicating success of the function call.
 *
 * @warning This function should not be used without direct instruction by Silicon Labs.
 */
sl_rail_status_t sl_rail_zwave_config_beam_rx(sl_rail_handle_t rail_handle,
                                              const sl_rail_zwave_beam_rx_config_t *p_config);

/**
 * Set the default RX beam configuration.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @return Status code indicating success of the function call.
 *
 * @note This function resets any changes made to the beam configuration via
 *   \ref sl_rail_zwave_config_beam_rx() and the default beam configuration will be in effect
 *   on subsequent call(s) to \ref sl_rail_zwave_receive_beam().
 */
sl_rail_status_t sl_rail_zwave_set_default_rx_beam_config(sl_rail_handle_t rail_handle);

/**
 * Get the current RX beam configuration.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[out] p_config A pointer to \ref sl_rail_zwave_beam_rx_config_t to be
 *   populated with the current beam configuration.
 * @return Status code indicating success of the function call.
 */
sl_rail_status_t sl_rail_zwave_get_rx_beam_config(sl_rail_handle_t rail_handle,
                                                  sl_rail_zwave_beam_rx_config_t *p_config);

/**
 * Configure the channel hop timings for use in Z-Wave RX channel hop configuration.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in,out] p_config A pointer to a configuration for Z-Wave RX channel hopping.
 *   This structure must be allocated in application global read-write memory.
 *   RAIL will populate fields within or referenced by this structure during its
 *   operation. Be sure to allocate \ref sl_rail_rx_channel_hopping_config_entry_t
 *   entries[] for \ref SL_RAIL_NUM_ZWAVE_CHANNELS. Be sure to set \ref
 *   sl_rail_rx_channel_hopping_config_t::number_of_channels to the desired number of
 *   channels.
 * @return Status code indicating success of the function call.
 *
 * @warning This function should not be used without direct instruction by Silicon Labs.
 *
 * @note: This API must be called before \ref sl_rail_enable_rx_channel_hopping(). This
 *   API must never be called while the radio is on with RX Duty Cycle or Channel
 *   Hopping enabled.
 */
sl_rail_status_t sl_rail_zwave_config_rx_channel_hopping(sl_rail_handle_t rail_handle,
                                                         sl_rail_rx_channel_hopping_config_t *p_config);

/**
 * Get the Z-Wave region.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @return The \ref sl_rail_zwave_region_id_t value.
 *
 * @note \ref sl_rail_zwave_config_region() must have been called successfully
 *   before this function is called. Otherwise, \ref SL_RAIL_ZWAVE_REGION_ID_UNKNOWN
 *   is returned.
 */
sl_rail_zwave_region_id_t sl_rail_zwave_get_region(sl_rail_handle_t rail_handle);

/**
 * Write the Auto-Ack FIFO for the next outgoing Z-Wave Long Range Ack.
 *
 * @param[in] rail_handle A real RAIL instance handle.
 * @param[in] p_lr_ack_data An application provided pointer to a const
 *   \ref sl_rail_zwave_lr_ack_data_t to populate the noise floor, TX power and receive
 *   rssi bytes of the outgoing Z-Wave Long Range Ack packet.
 * @return Status code indicating success of the function call.
 *
 * This function sets the Auto-Ack data to use in acknowledging the frame
 * being received. It must only be called while processing the \ref
 * SL_RAIL_EVENT_ZWAVE_LR_ACK_REQUEST_COMMAND.
 * This will return \ref SL_RAIL_STATUS_INVALID_STATE if it is too late to
 * write the outgoing Ack. When successful, the p_lr_ack_data will
 * only be sent once. Subsequent packets needing an Z-Wave Long Range Ack will
 * each need to call this function to write the Ack information.
 */
sl_rail_status_t sl_rail_zwave_set_lr_ack_data(sl_rail_handle_t rail_handle,
                                               const sl_rail_zwave_lr_ack_data_t *p_lr_ack_data);

/**
 * EU-European Union.
 */
extern const sl_rail_zwave_region_config_t sl_rail_zwave_region_eu;

/**
 * US-United States.
 */
extern const sl_rail_zwave_region_config_t sl_rail_zwave_region_us;

/**
 * ANZ-Australia/New Zealand.
 */
extern const sl_rail_zwave_region_config_t sl_rail_zwave_region_anz;

/**
 * HK-Hong Kong.
 */
extern const sl_rail_zwave_region_config_t sl_rail_zwave_region_hk;

/**
 * MY-Malaysia.
 */
extern const sl_rail_zwave_region_config_t sl_rail_zwave_region_my;

/**
 * IN-India.
 */
extern const sl_rail_zwave_region_config_t sl_rail_zwave_region_in;

/**
 * JP-Japan.
 */
extern const sl_rail_zwave_region_config_t sl_rail_zwave_region_jp;

/**
 * RU-Russia.
 */
extern const sl_rail_zwave_region_config_t sl_rail_zwave_region_ru;

/**
 * IL-Israel.
 */
extern const sl_rail_zwave_region_config_t sl_rail_zwave_region_il;

/**
 * KR-Korea.
 */
extern const sl_rail_zwave_region_config_t sl_rail_zwave_region_kr;

/**
 * CN-China.
 */
extern const sl_rail_zwave_region_config_t sl_rail_zwave_region_cn;

/**
 * US-Long Range 1.
 */
extern const sl_rail_zwave_region_config_t sl_rail_zwave_region_us_lr_1;

/**
 * US-Long Range 2.
 */
extern const sl_rail_zwave_region_config_t sl_rail_zwave_region_us_lr_2;

/**
 * US-Long Range 3.
 */
extern const sl_rail_zwave_region_config_t sl_rail_zwave_region_us_lr_3;

/**
 * EU-Long Range 1.
 */
extern const sl_rail_zwave_region_config_t sl_rail_zwave_region_eu_lr_1;

/**
 * EU-Long Range 2.
 */
extern const sl_rail_zwave_region_config_t sl_rail_zwave_region_eu_lr_2;

/**
 * EU-Long Range 3.
 */
extern const sl_rail_zwave_region_config_t sl_rail_zwave_region_eu_lr_3;

/**
 * Invalid Region.
 */
extern const sl_rail_zwave_region_config_t sl_rail_zwave_region_invalid;

/**
 * An array of region configurations indexed by \ref sl_rail_zwave_region_id_t.
 */
extern const sl_rail_zwave_region_config_t * const sl_rail_zwave_region_cfg[SL_RAIL_ZWAVE_REGION_ID_COUNT];

/// @} // end of Z_Wave

#ifdef __cplusplus
}
#endif

// Temporarily include RAIL 2.x APIs
#include "rail_zwave.h"

#endif // SL_RAIL_ZWAVE_H
