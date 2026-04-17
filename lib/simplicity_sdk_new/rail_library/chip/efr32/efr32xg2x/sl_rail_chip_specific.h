/***************************************************************************//**
 * @file
 * @brief This file contains the type definitions for EFR32xG2x chip-specific
 *   aspects of RAIL.
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

#ifdef  SLI_LIBRARY_BUILD

// This file should not be included when doing SLI_LIBRARY_BUILDs

#else//!SLI_LIBRARY_BUILD

#ifndef SL_RAIL_CHIP_SPECIFIC_H
#if !defined(SL_RAIL_TYPES_H) && !defined(DOXYGEN_SHOULD_SKIP_THIS)
#warning sl_rail_chip_specific.h should only be included by sl_rail_types.h
#include "sl_rail_types.h" // Force sl_rail_chip_specific.h only within sl_rail_types.h
#else // SL_RAIL_TYPES_H
/// Include guard
#define SL_RAIL_CHIP_SPECIFIC_H

#include "sl_rail_features.h"
#ifdef RAIL_PRIVATE_BUILD
#include "sl_rail_chip_specific_internal.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * General Structures
 *****************************************************************************/
/**
 * @addtogroup General_EFR32XG2X EFR32xG2x
 * @ingroup General
 * @{
 * @brief Types specific to the EFR32xG2x for general configuration.
 */

#ifndef DOXYGEN_UNDOCUMENTED

/**
 * @def SL_RAIL_EFR32XG21_STATE_BUFFER_BYTES
 * @brief The EFR32xG21 series size needed for
 *   \ref sl_rail_state_buffer_entry_t::buffer_bytes.
 */
#define SL_RAIL_EFR32XG21_STATE_BUFFER_BYTES 608U  // DO NOT HAND-EDIT THESE VALUES

/**
 * @def SL_RAIL_EFR32XG22_STATE_BUFFER_BYTES
 * @brief The EFR32xG22 series size needed for
 *   \ref sl_rail_state_buffer_entry_t::buffer_bytes.
 */
#define SL_RAIL_EFR32XG22_STATE_BUFFER_BYTES 616U  // DO NOT HAND-EDIT THESE VALUES

/**
 * @def SL_RAIL_EFR32XG23_STATE_BUFFER_BYTES
 * @brief The EFR32xG23 series size needed for
 *   \ref sl_rail_state_buffer_entry_t::buffer_bytes.
 */
#define SL_RAIL_EFR32XG23_STATE_BUFFER_BYTES 632U  // DO NOT HAND-EDIT THESE VALUES

/**
 * @def SL_RAIL_EFR32XG24_STATE_BUFFER_BYTES
 * @brief The EFR32xG24 series size needed for
 *   \ref sl_rail_state_buffer_entry_t::buffer_bytes.
 */
#define SL_RAIL_EFR32XG24_STATE_BUFFER_BYTES 648U  // DO NOT HAND-EDIT THESE VALUES

/**
 * @def SL_RAIL_EFR32XG25_STATE_BUFFER_BYTES
 * @brief The EFR32xG25 series size needed for
 *   \ref sl_rail_state_buffer_entry_t::buffer_bytes.
 */
#define SL_RAIL_EFR32XG25_STATE_BUFFER_BYTES 624U  // DO NOT HAND-EDIT THESE VALUES

/**
 * @def SL_RAIL_EFR32XG26_STATE_BUFFER_BYTES
 * @brief The EFR32xG26 series size needed for
 *   \ref sl_rail_state_buffer_entry_t::buffer_bytes.
 */
#define SL_RAIL_EFR32XG26_STATE_BUFFER_BYTES 648U  // DO NOT HAND-EDIT THESE VALUES

/**
 * @def SL_RAIL_EFR32XG27_STATE_BUFFER_BYTES
 * @brief The EFR32xG27 series size needed for
 *   \ref sl_rail_state_buffer_entry_t::buffer_bytes.
 */
#define SL_RAIL_EFR32XG27_STATE_BUFFER_BYTES 616U  // DO NOT HAND-EDIT THESE VALUES

/**
 * @def SL_RAIL_EFR32XG28_STATE_BUFFER_BYTES
 * @brief The EFR32xG28 series size needed for
 *   \ref sl_rail_state_buffer_entry_t::buffer_bytes.
 */
#define SL_RAIL_EFR32XG28_STATE_BUFFER_BYTES 632U  // DO NOT HAND-EDIT THESE VALUES

/**
 * @def SL_RAIL_EFR32XG29_STATE_BUFFER_BYTES
 * @brief The EFR32xG29 series size needed for
 *   \ref sl_rail_state_buffer_entry_t::buffer_bytes.
 */
#define SL_RAIL_EFR32XG29_STATE_BUFFER_BYTES 616U  // DO NOT HAND-EDIT THESE VALUES

/**
 * @def SL_RAIL_EFR32XG2D_STATE_BUFFER_BYTES
 * @brief The EFR32xG2D series size needed for
 *   \ref sl_rail_state_buffer_entry_t::buffer_bytes.
 */
#define SL_RAIL_EFR32XG2D_STATE_BUFFER_BYTES 632U  // DO NOT HAND-EDIT THESE VALUES

#ifndef SL_RAIL_STATE_BUFFER_BYTES
/**
 * @def SL_RAIL_STATE_BUFFER_BYTES
 * @brief The size needed for \ref sl_rail_state_buffer_entry_t::buffer_bytes
 *   on this platform for this radio. This compile-time size may be slightly
 *   larger than what \ref sl_rail_get_state_buffer_bytes() determines at run-time.
 */
#if (_SILICON_LABS_32B_SERIES_2_CONFIG == 1)
#define SL_RAIL_STATE_BUFFER_BYTES SL_RAIL_EFR32XG21_STATE_BUFFER_BYTES
#elif (_SILICON_LABS_32B_SERIES_2_CONFIG == 2)
#define SL_RAIL_STATE_BUFFER_BYTES SL_RAIL_EFR32XG22_STATE_BUFFER_BYTES
#elif (_SILICON_LABS_32B_SERIES_2_CONFIG == 3)
#define SL_RAIL_STATE_BUFFER_BYTES SL_RAIL_EFR32XG23_STATE_BUFFER_BYTES
#elif (_SILICON_LABS_32B_SERIES_2_CONFIG == 4)
#define SL_RAIL_STATE_BUFFER_BYTES SL_RAIL_EFR32XG24_STATE_BUFFER_BYTES
#elif (_SILICON_LABS_32B_SERIES_2_CONFIG == 5)
#define SL_RAIL_STATE_BUFFER_BYTES SL_RAIL_EFR32XG25_STATE_BUFFER_BYTES
#elif (_SILICON_LABS_32B_SERIES_2_CONFIG == 6)
#define SL_RAIL_STATE_BUFFER_BYTES SL_RAIL_EFR32XG26_STATE_BUFFER_BYTES
#elif (_SILICON_LABS_32B_SERIES_2_CONFIG == 7)
#define SL_RAIL_STATE_BUFFER_BYTES SL_RAIL_EFR32XG27_STATE_BUFFER_BYTES
#elif (_SILICON_LABS_32B_SERIES_2_CONFIG == 8)
#define SL_RAIL_STATE_BUFFER_BYTES SL_RAIL_EFR32XG28_STATE_BUFFER_BYTES
#elif (_SILICON_LABS_32B_SERIES_2_CONFIG == 9)
#define SL_RAIL_STATE_BUFFER_BYTES SL_RAIL_EFR32XG29_STATE_BUFFER_BYTES
#elif (_SILICON_LABS_32B_SERIES_2_CONFIG == 13)
#define SL_RAIL_STATE_BUFFER_BYTES SL_RAIL_EFR32XG2D_STATE_BUFFER_BYTES
#else
#define SL_RAIL_STATE_BUFFER_BYTES 0 // Sate Doxygen
#error "Unsupported platform!"
#endif
#endif //#ifndef SL_RAIL_STATE_BUFFER_BYTES

#endif//DOXYGEN_UNDOCUMENTED

#ifndef DOXYGEN_UNDOCUMENTED

/**
 * @def SL_RAIL_SEQ_IMAGE_0
 * @brief A macro for the primary sequencer image.
 */
#define SL_RAIL_SEQ_IMAGE_0 1

/**
 * @def SL_RAIL_SEQ_IMAGE_1
 * @brief A macro for the alternate sequencer image.
 */
#define SL_RAIL_SEQ_IMAGE_1 2

#if ((_SILICON_LABS_32B_SERIES_2_CONFIG == 4) || (_SILICON_LABS_32B_SERIES_2_CONFIG == 6))
/**
 * @def SL_RAIL_SEQ_IMAGE_PA_10_DBM
 * @brief A chip-specific macro for the sequencer image used on EFR32xG24 and EFR32xG26 OPNs
 *   with a 10 dBm PA.
 */
#define SL_RAIL_SEQ_IMAGE_PA_10_DBM SL_RAIL_SEQ_IMAGE_0

/**
 * @def SL_RAIL_SEQ_IMAGE_PA_20_DBM
 * @brief A chip-specific macro for the sequencer image used on EFR32xG24 and EFR32xG26 OPNs
 *   with a 20 dBm PA.
 */
#define SL_RAIL_SEQ_IMAGE_PA_20_DBM SL_RAIL_SEQ_IMAGE_1

/**
 * @def SL_RAIL_SEQ_IMAGE_COUNT
 * @brief A macro for the total number of sequencer images supported on the
 *   platform.
 */
#define SL_RAIL_SEQ_IMAGE_COUNT 2

#else //((_SILICON_LABS_32B_SERIES_2_CONFIG != 4) && (_SILICON_LABS_32B_SERIES_2_CONFIG != 6))

/**
 * @def SL_RAIL_SEQ_IMAGE_DEFAULT
 * @brief A chip-specific macro for the default sequencer image on platforms
 *   that support only one sequencer image.
 */
#define SL_RAIL_SEQ_IMAGE_DEFAULT SL_RAIL_SEQ_IMAGE_0

/**
 * @def SL_RAIL_SEQ_IMAGE_COUNT
 * @brief A macro for the total number of sequencer images supported on the
 *   platform.
 */
#define SL_RAIL_SEQ_IMAGE_COUNT 1
#endif //((_SILICON_LABS_32B_SERIES_2_CONFIG == 4) || (_SILICON_LABS_32B_SERIES_2_CONFIG == 6))

/**
 * @struct sl_rail_rffpll_config_t
 * @brief Stores information relevant to the Radio-Friendly Frequency
 *   Phase-Locked Loop (RFFPLL) for the PHY configuration currently loaded in
 *   memory.
 */
typedef struct {
  /**
   * Divider X (Modem Clock), Divider Y (M33 System Clock), and Divider N (Feedback) values.
   */
  uint32_t dividers;
  /**
   * Radio clock frequency in Hz.
   */
  uint32_t radio_freq_hz;
  /**
   * System clock frequency in Hz.
   */
  uint32_t sysclk_freq_hz;
} sl_rail_rffpll_config_t;

/**
 * @def SL_RAIL_RFFPLL_DIVIDER_X_MASK
 * @brief Bit mask for RFFPLL DIVX in \ref sl_rail_rffpll_config_t::dividers.
 */
#define SL_RAIL_RFFPLL_DIVIDER_X_MASK   0x000000FFUL

/**
 * @def SL_RAIL_RFFPLL_DIVIDER_X_SHIFT
 * @brief Shift value for RFFPLL DIVX in \ref sl_rail_rffpll_config_t::dividers.
 */
#define SL_RAIL_RFFPLL_DIVIDER_X_SHIFT  0

/**
 * @def SL_RAIL_RFFPLL_DIVIDER_Y_MASK
 * @brief Bit mask for RFFPLL DIVY in \ref sl_rail_rffpll_config_t::dividers.
 */
#define SL_RAIL_RFFPLL_DIVIDER_Y_MASK   0x0000FF00UL

/**
 * @def SL_RAIL_RFFPLL_DIVIDER_Y_SHIFT
 * @brief Shift value for RFFPLL DIVY in \ref sl_rail_rffpll_config_t::dividers.
 */
#define SL_RAIL_RFFPLL_DIVIDER_Y_SHIFT  8

/**
 * @def SL_RAIL_RFFPLL_DIVIDER_N_MASK
 * @brief Bit mask for RFFPLL DIVN in \ref sl_rail_rffpll_config_t::dividers.
 */
#define SL_RAIL_RFFPLL_DIVIDER_N_MASK   0x00FF0000UL

/**
 * @def SL_RAIL_RFFPLL_DIVIDER_N_SHIFT
 * @brief Shift value for RFFPLL DIVN in \ref sl_rail_rffpll_config_t::dividers.
 */
#define SL_RAIL_RFFPLL_DIVIDER_N_SHIFT  16

/**
 * @typedef sl_rail_timer_tick_t
 * @brief Internal RAIL hardware timer tick that drives the RAIL timebase.
 *   A tick is roughly 0.5 microseconds but it wraps somewhat before
 *   0xFFFFFFFF giving a time range of about 17 minutes.
 *
 * @note \ref sl_rail_timer_ticks_to_us() can be used to convert the delta between
 *   two \ref sl_rail_timer_tick_t values to microseconds.
 */
typedef uint32_t sl_rail_timer_tick_t;

/**
 * @def sl_rail_get_timer_tick(timer_tick_type)
 * @brief The RAIL hardware timer ticks value.
 *
 * @note timer_tick_type is added for compatibility reasons and is ignored here;
 *   this gets the equivalent of \ref SL_RAIL_TIMER_TICK_DEFAULT.
 */
#define sl_rail_get_timer_tick(timer_tick_type) (*sl_rail_timer_tick)

/**
 * A global pointer to the memory address of the 32-bit
 * \ref sl_rail_timer_tick_t internal RAIL hardware timer that drives
 * the RAIL timebase.
 * Equivalent to \ref sl_rail_timer_tick_t for its granularity and range.
 */
extern volatile uint32_t * const sl_rail_timer_tick;

/**
 * A global pointer to the memory address of the internal RAIL hardware timer
 * that captures the latest RX packet reception time.
 * See \ref sl_rail_timer_tick_t for its granularity and range.
 *
 * @note This would not include the RX chain delay, so may not exactly
 *   correspond to the \ref sl_rail_time_t packet time stamp available within
 *   \ref sl_rail_rx_packet_details_t::time_received which reflects the actual
 *   on-air time that the packet finished.
 */
extern volatile uint32_t * const sl_rail_rx_packet_time_stamp;

/**
 * Get elapsed time, in microseconds, between two \ref sl_rail_timer_tick_t ticks.
 *
 * @param[in] start_tick Tick recorded at the start of the operation.
 * @param[in] end_tick Tick recorded at the end of the operation.
 * @return The elapsed time, in microseconds, between two timer ticks.
 */
sl_rail_time_t sl_rail_timer_ticks_to_us(sl_rail_timer_tick_t start_tick,
                                         sl_rail_timer_tick_t end_tick);

/**
 * Get \ref sl_rail_timer_tick_t tick corresponding to a \ref sl_rail_time_t time.
 *
 * @param[in] microseconds Time in microseconds.
 * @return The \ref sl_rail_timer_tick_t tick corresponding to the
 *   \ref sl_rail_time_t time.
 */
sl_rail_timer_tick_t sl_rail_us_to_timer_ticks(sl_rail_time_t microseconds);

#endif//DOXYGEN_UNDOCUMENTED

/** @} */ // end of group General_EFR32XG2X

/******************************************************************************
 * Multiprotocol
 *****************************************************************************/
/**
 * @addtogroup Multiprotocol_EFR32XG2X EFR32xG2x
 * @ingroup Multiprotocol
 * @{
 * @brief EFR32xG2x-specific multiprotocol support defines.
 */

/**
 * @def SL_RAIL_TRANSITION_TIME_US
 * @brief Time it takes to take care of protocol switching.
 */
#if _SILICON_LABS_32B_SERIES_2_CONFIG > 1
// XG22 + devices
#define SL_RAIL_TRANSITION_TIME_US 510
#else
// XG21
#define SL_RAIL_TRANSITION_TIME_US 500
#endif

/** @} */ // end of group Multiprotocol_EFR32XG2X

/******************************************************************************
 * Calibration
 *****************************************************************************/
/**
 * @addtogroup Calibration_EFR32XG2X EFR32xG2x
 * @ingroup Calibration
 * @{
 * @brief EFR32xG2x-specific Calibrations.
 */

/**
 * @def SL_RAIL_RF_PATHS_2P4_GHZ
 * @brief Indicates the number of 2.4 GHz RF Paths suppported.
 */
#ifndef SL_RAIL_RF_PATHS_2P4_GHZ
#if ((_SILICON_LABS_32B_SERIES_2_CONFIG == 1) || (_SILICON_LABS_32B_SERIES_2_CONFIG == 4) || (_SILICON_LABS_32B_SERIES_2_CONFIG == 6))
#define SL_RAIL_RF_PATHS_2P4_GHZ 2
#elif ((_SILICON_LABS_32B_SERIES_2_CONFIG == 2) \
  || (_SILICON_LABS_32B_SERIES_2_CONFIG == 7)   \
  || (_SILICON_LABS_32B_SERIES_2_CONFIG == 9)   \
  || (_SILICON_LABS_32B_SERIES_2_CONFIG == 8))
#define SL_RAIL_RF_PATHS_2P4_GHZ 1
#else
#define SL_RAIL_RF_PATHS_2P4_GHZ 0
#endif
#endif//SL_RAIL_RF_PATHS_2P4GHZ

/**
 * @def SL_RAIL_RF_PATHS_SUB_GHZ
 * @brief Indicates the number of Sub-GHz RF Paths supported.
 */
#ifndef SL_RAIL_RF_PATHS_SUB_GHZ
#if _SILICON_LABS_32B_SERIES_2_CONFIG == 3
#define SL_RAIL_RF_PATHS_SUB_GHZ 2
#elif  _SILICON_LABS_32B_SERIES_2_CONFIG == 5
#define SL_RAIL_RF_PATHS_SUB_GHZ 2
#elif _SILICON_LABS_32B_SERIES_2_CONFIG == 8
#define SL_RAIL_RF_PATHS_SUB_GHZ 1
#elif  _SILICON_LABS_32B_SERIES_2_CONFIG == 13
#define SL_RAIL_RF_PATHS_SUB_GHZ 2
#else
#define SL_RAIL_RF_PATHS_SUB_GHZ 0
#endif
#endif//SL_RAIL_RF_PATHS_SUB_GHZ

/**
 * @def SL_RAIL_RF_PATHS
 * @brief Indicates the number of RF Paths supported.
 */
#define SL_RAIL_RF_PATHS (SL_RAIL_RF_PATHS_SUB_GHZ + SL_RAIL_RF_PATHS_2P4_GHZ)

#if     (SL_RAIL_RF_PATHS > SL_RAIL_MAX_RF_PATHS)
#error "Update sl_rail_types.h SL_RAIL_MAX_RF_PATHS"
#endif

/**
 * @def SL_RAIL_RADIO_CONFIG_ENABLE_IR_CAL_MULTIPLE_RF_PATHS
 * @brief Indicates this version of RAIL supports IR calibration on multiple RF paths
 * Needed for backwards compatibility.
 */
#if SL_RAIL_RF_PATHS > 1
#define SL_RAIL_RADIO_CONFIG_ENABLE_IR_CAL_MULTIPLE_RF_PATHS 1
#else
#ifdef  DOXYGEN_SHOULD_SKIP_THIS // Leave undefined except for doxygen
#define SL_RAIL_RADIO_CONFIG_ENABLE_IR_CAL_MULTIPLE_RF_PATHS 0
#endif //DOXYGEN_SHOULD_SKIP_THIS
#endif //SL_RAIL_RF_PATHS

/**
 * @struct sl_rail_channel_config_entry_attr
 * @brief A channel configuration entry attribute structure. Items listed
 *  are designed to be altered and updated during run-time.
 */
struct sl_rail_channel_config_entry_attr {
  /**
   * IR calibration attributes specific to each channel configuration entry.
   */
 #if     SL_RAIL_SUPPORTS_OFDM_PA
  sl_rail_ir_cal_values_t rx_ir_cal_values;
 #else//!SL_RAIL_SUPPORTS_OFDM_PA
  sl_rail_rx_ir_cal_values_t rx_ir_cal_values;
 #endif//SL_RAIL_SUPPORTS_OFDM_PA
};

/** @} */ // end of group Calibration_EFR32XG2X

/******************************************************************************
 * Transmit
 *****************************************************************************/
/**
 * @addtogroup PA_EFR32XG2X EFR32xG2x
 * @ingroup PA
 * @{
 * @brief Types specific to the EFR32xG2x for dealing with the on-chip PAs.
 */

#ifndef DOXYGEN_UNDOCUMENTED

#ifndef SLI_RAIL_TX_POWER_LEVEL_2P4_GHZ_HP_MAX
#if _SILICON_LABS_32B_SERIES_2_CONFIG == 1
/**
 * The maximum valid value for the \ref sli_rail_tx_power_level_t when in \ref
 * SL_RAIL_TX_POWER_MODE_2P4_GHZ_HP mode.
 */
#define SLI_RAIL_TX_POWER_LEVEL_2P4_GHZ_HP_MAX  (180U)
/**
 * The minimum valid value for the \ref sli_rail_tx_power_level_t when in \ref
 * SL_RAIL_TX_POWER_MODE_2P4_GHZ_HP mode.
 */
#define SLI_RAIL_TX_POWER_LEVEL_2P4_GHZ_HP_MIN  (1U)
/**
 * The maximum valid value for the \ref sli_rail_tx_power_level_t when in \ref
 * SL_RAIL_TX_POWER_MODE_2P4_GHZ_MP mode.
 */
#define SLI_RAIL_TX_POWER_LEVEL_2P4_GHZ_MP_MAX  (90U)
/**
 * The maximum valid value for the \ref sli_rail_tx_power_level_t when in \ref
 * SL_RAIL_TX_POWER_MODE_2P4_GHZ_MP mode.
 */
#define SLI_RAIL_TX_POWER_LEVEL_2P4_GHZ_MP_MIN  (1U)
/**
 * The maximum valid value for the \ref sli_rail_tx_power_level_t when in \ref
 * SL_RAIL_TX_POWER_MODE_2P4_GHZ_LP mode.
 */
#define SLI_RAIL_TX_POWER_LEVEL_2P4_GHZ_LP_MAX  (64U)
/**
 * The minimum valid value for the \ref sli_rail_tx_power_level_t when in \ref
 * SL_RAIL_TX_POWER_MODE_2P4_GHZ_LP mode.
 */
#define SLI_RAIL_TX_POWER_LEVEL_2P4_GHZ_LP_MIN  (0U)
#elif ((_SILICON_LABS_32B_SERIES_2_CONFIG == 4) || (_SILICON_LABS_32B_SERIES_2_CONFIG == 6))
/**
 * The maximum valid value for the \ref sli_rail_tx_power_level_t when in \ref
 * SL_RAIL_TX_POWER_MODE_2P4_GHZ_HP mode.
 *  EFR32xG24: capable of 20dBm max output power has max powerlevel:180
 *  EFR32xG24: capable of 10dBm max output power has max powerlevel:90
 *  EFR32xG26: capable of 20dBm max output power has max powerlevel:180
 *  EFR32xG26: capable of 10dBm max output power has max powerlevel:90
 */
#if defined (_SILICON_LABS_EFR32_2G4HZ_HP_PA_PRESENT) \
  && (_SILICON_LABS_EFR32_2G4HZ_HP_PA_MAX_OUTPUT_DBM > 10)
#define SLI_RAIL_TX_POWER_LEVEL_2P4_GHZ_HP_MAX  (180U)
#else
#define SLI_RAIL_TX_POWER_LEVEL_2P4_GHZ_HP_MAX  (90U)
#endif
/**
 * The minimum valid value for the \ref sli_rail_tx_power_level_t when in \ref
 * SL_RAIL_TX_POWER_MODE_2P4_GHZ_HP mode.
 */
#define SLI_RAIL_TX_POWER_LEVEL_2P4_GHZ_HP_MIN  (0U)
/**
 * The maximum valid value for the \ref sli_rail_tx_power_level_t when in \ref
 * SL_RAIL_TX_POWER_MODE_2P4_GHZ_LP mode.
 */
#define SLI_RAIL_TX_POWER_LEVEL_2P4_GHZ_LP_MAX  (15U)
/**
 * The minimum valid value for the \ref sli_rail_tx_power_level_t when in \ref
 * SL_RAIL_TX_POWER_MODE_2P4_GHZ_LP mode.
 */
#define SLI_RAIL_TX_POWER_LEVEL_2P4_GHZ_LP_MIN  (0U)
#elif ((_SILICON_LABS_32B_SERIES_2_CONFIG == 2) \
  || (_SILICON_LABS_32B_SERIES_2_CONFIG == 7)   \
  || (_SILICON_LABS_32B_SERIES_2_CONFIG == 9))
/**
 * The maximum valid value for the \ref sli_rail_tx_power_level_t when in \ref
 * SL_RAIL_TX_POWER_MODE_2P4_GHZ_HP mode.
 */
#define SLI_RAIL_TX_POWER_LEVEL_2P4_GHZ_HP_MAX  (127U)
/**
 * The minimum valid value for the \ref sli_rail_tx_power_level_t when in \ref
 * SL_RAIL_TX_POWER_MODE_2P4_GHZ_HP mode.
 */
#define SLI_RAIL_TX_POWER_LEVEL_2P4_GHZ_HP_MIN  (0U)
/**
 * The maximum valid value for the \ref sli_rail_tx_power_level_t when in \ref
 * SL_RAIL_TX_POWER_MODE_2P4_GHZ_LP mode.
 */
#define SLI_RAIL_TX_POWER_LEVEL_2P4_GHZ_LP_MAX  (15U)
/**
 * The minimum valid value for the \ref sli_rail_tx_power_level_t when in \ref
 * SL_RAIL_TX_POWER_MODE_2P4_GHZ_LP mode.
 */
#define SLI_RAIL_TX_POWER_LEVEL_2P4_GHZ_LP_MIN  (0U)
#elif _SILICON_LABS_32B_SERIES_2_CONFIG == 8
/**
 * The maximum valid value for the \ref sli_rail_tx_power_level_t when in \ref
 * SL_RAIL_TX_POWER_MODE_2P4_GHZ_HP mode.
 */
#define SLI_RAIL_TX_POWER_LEVEL_2P4_GHZ_HP_MAX  (240)
/**
 * The minimum valid value for the \ref sli_rail_tx_power_level_t when in \ref
 * SL_RAIL_TX_POWER_MODE_2P4_GHZ_HP mode.
 */
#define SLI_RAIL_TX_POWER_LEVEL_2P4_GHZ_HP_MIN  (1U)
#else //EFR32xG23
/**
 * The maximum valid value for the \ref sli_rail_tx_power_level_t when in \ref
 * SL_RAIL_TX_POWER_MODE_2P4_GHZ_HP mode.
 */
#define SLI_RAIL_TX_POWER_LEVEL_2P4_GHZ_HP_MAX  (240)
/**
 * The minimum valid value for the \ref sli_rail_tx_power_level_t when in \ref
 * SL_RAIL_TX_POWER_MODE_2P4_GHZ_HP mode.
 */
#define SLI_RAIL_TX_POWER_LEVEL_2P4_GHZ_HP_MIN  (1U)
/**
 * The maximum valid value for the \ref sli_rail_tx_power_level_t when in \ref
 * SL_RAIL_TX_POWER_MODE_2P4_GHZ_MP mode.
 */
#define SLI_RAIL_TX_POWER_LEVEL_2P4_GHZ_MP_MAX  (SLI_RAIL_TX_POWER_LEVEL_2P4_GHZ_HP_MAX)
/**
 * The maximum valid value for the \ref sli_rail_tx_power_level_t when in \ref
 * SL_RAIL_TX_POWER_MODE_2P4_GHZ_MP mode.
 */
#define SLI_RAIL_TX_POWER_LEVEL_2P4_GHZ_MP_MIN  (SLI_RAIL_TX_POWER_LEVEL_2P4_GHZ_HP_MIN)
/**
 * The maximum valid value for the \ref sli_rail_tx_power_level_t when in \ref
 * SL_RAIL_TX_POWER_MODE_2P4_GHZ_LP mode.
 */
#define SLI_RAIL_TX_POWER_LEVEL_2P4_GHZ_LP_MAX  (SLI_RAIL_TX_POWER_LEVEL_2P4_GHZ_HP_MAX)
/**
 * The minimum valid value for the \ref sli_rail_tx_power_level_t when in \ref
 * SL_RAIL_TX_POWER_MODE_2P4_GHZ_LP mode.
 */
#define SLI_RAIL_TX_POWER_LEVEL_2P4_GHZ_LP_MIN  (SLI_RAIL_TX_POWER_LEVEL_2P4_GHZ_HP_MIN)
/**
 * The maximum valid value for the \ref sli_rail_tx_power_level_t when in \ref
 * SL_RAIL_TX_POWER_MODE_2P4_GHZ_LLP mode.
 */
#define SLI_RAIL_TX_POWER_LEVEL_2P4_GHZ_LLP_MAX  (SLI_RAIL_TX_POWER_LEVEL_2P4_GHZ_HP_MAX)
/**
 * The minimum valid value for the \ref sli_rail_tx_power_level_t when in \ref
 * SL_RAIL_TX_POWER_MODE_2P4_GHZ_LLP mode.
 */
#define SLI_RAIL_TX_POWER_LEVEL_2P4_GHZ_LLP_MIN  (SLI_RAIL_TX_POWER_LEVEL_2P4_GHZ_HP_MIN)
#endif //_SILICON_LABS_32B_SERIES_2_CONFIG
#endif //SLI_RAIL_TX_POWER_LEVEL_2P4_GHZ_HP_MAX

#if SL_RAIL_SUPPORTS_SUB_GHZ_BAND
/**
 * The maximum valid value for the \ref sli_rail_tx_power_level_t when using
 * a Sub-GHz PA mode.
 */
#ifndef SLI_RAIL_TX_POWER_LEVEL_SUB_GHZ_MAX
#if ((_SILICON_LABS_32B_SERIES_2_CONFIG == 3) \
  || (_SILICON_LABS_32B_SERIES_2_CONFIG == 8) \
  || (_SILICON_LABS_32B_SERIES_2_CONFIG == 13))
#define SLI_RAIL_TX_POWER_LEVEL_SUB_GHZ_MAX 240U
#elif _SILICON_LABS_32B_SERIES_2_CONFIG == 5
#define SLI_RAIL_TX_POWER_LEVEL_SUB_GHZ_MAX 79U
#else
#define SLI_RAIL_TX_POWER_LEVEL_SUB_GHZ_MAX 0U
#endif
#endif//SLI_RAIL_TX_POWER_LEVEL_SUB_GHZ_MAX

/**
 * The minimum valid value for the \ref sli_rail_tx_power_level_t when using
 * a Sub-GHz PA mode.
 */
#define SLI_RAIL_TX_POWER_LEVEL_SUB_GHZ_MIN 1U

/**
 * The maximum valid value for the \ref sli_rail_tx_power_level_t when in \ref
 * SL_RAIL_TX_POWER_MODE_SUB_GHZ_HP mode.
 */
#define SLI_RAIL_TX_POWER_LEVEL_SUB_GHZ_HP_MAX (SLI_RAIL_TX_POWER_LEVEL_SUB_GHZ_MAX)

/**
 * The minimum valid value for the \ref sli_rail_tx_power_level_t when in \ref
 * SL_RAIL_TX_POWER_MODE_SUB_GHZ_HP mode.
 */
#define SLI_RAIL_TX_POWER_LEVEL_SUB_GHZ_HP_MIN (SLI_RAIL_TX_POWER_LEVEL_SUB_GHZ_MIN)

/**
 * The maximum valid value for the \ref sli_rail_tx_power_level_t when in \ref
 * SL_RAIL_TX_POWER_MODE_SUB_GHZ_MP mode.
 */
#define SLI_RAIL_TX_POWER_LEVEL_SUB_GHZ_MP_MAX (SLI_RAIL_TX_POWER_LEVEL_SUB_GHZ_MAX)

/**
 * The minimum valid value for the \ref sli_rail_tx_power_level_t when in \ref
 * SL_RAIL_TX_POWER_MODE_SUB_GHZ_MP mode.
 */
#define SLI_RAIL_TX_POWER_LEVEL_SUB_GHZ_MP_MIN (SLI_RAIL_TX_POWER_LEVEL_SUB_GHZ_MIN)

/**
 * The maximum valid value for the \ref sli_rail_tx_power_level_t when in \ref
 * SL_RAIL_TX_POWER_MODE_SUB_GHZ_LP mode.
 */
#define SLI_RAIL_TX_POWER_LEVEL_SUB_GHZ_LP_MAX (SLI_RAIL_TX_POWER_LEVEL_SUB_GHZ_MAX)

/**
 * The minimum valid value for the \ref sli_rail_tx_power_level_t when in \ref
 * SL_RAIL_TX_POWER_MODE_SUB_GHZ_LP mode.
 */
#define SLI_RAIL_TX_POWER_LEVEL_SUB_GHZ_LP_MIN (SLI_RAIL_TX_POWER_LEVEL_SUB_GHZ_MIN)

/**
 * The maximum valid value for the \ref sli_rail_tx_power_level_t when in \ref
 * SL_RAIL_TX_POWER_MODE_SUB_GHZ_LLP mode.
 */
#define SLI_RAIL_TX_POWER_LEVEL_SUB_GHZ_LLP_MAX (SLI_RAIL_TX_POWER_LEVEL_SUB_GHZ_MAX)

/**
 * The minimum valid value for the \ref sli_rail_tx_power_level_t when in \ref
 * SL_RAIL_TX_POWER_MODE_SUB_GHZ_LLP mode.
 */
#define SLI_RAIL_TX_POWER_LEVEL_SUB_GHZ_LLP_MIN (SLI_RAIL_TX_POWER_LEVEL_SUB_GHZ_MIN)
#endif //SL_RAIL_SUPPORTS_SUB_GHZ_BAND

#if SL_RAIL_SUPPORTS_OFDM_PA
#if _SILICON_LABS_32B_SERIES_2_CONFIG == 5
/**
 * The maximum valid value for the \ref sl_rail_txpower_level_t when in \ref
 * SL_RAIL_TX_POWER_MODE_OFDM_PA_POWERSETTING_TABLE mode on EFR32xG25.
 */
#define SLI_RAIL_OFDM_PA_MAX      204U
/**
 * The PA multiplier for PA control on EFR32xG25.
 */
#define SLI_RAIL_OFDM_PA_MULT     5U
/**
 * The minimum valid value for the \ref sl_rail_txpower_level_t when in \ref
 * SL_RAIL_TX_POWER_MODE_OFDM_PA_POWERSETTING_TABLE mode on EFR32xG25.
 */
#define SLI_RAIL_OFDM_PA_MIN      0U
#endif

/**
 * The maximum valid value for the \ref sli_rail_tx_power_level_t when in \ref
 * SL_RAIL_TX_POWER_MODE_OFDM_PA mode.
 */
#define SLI_RAIL_TX_POWER_LEVEL_OFDM_PA_MAX (SLI_RAIL_OFDM_PA_MAX)

/**
 * The minimum valid value for the \ref sli_rail_tx_power_level_t when in \ref
 * SL_RAIL_TX_POWER_MODE_OFDM_PA mode.
 */
#define SLI_RAIL_TX_POWER_LEVEL_OFDM_PA_MIN (SLI_RAIL_OFDM_PA_MIN)
#endif //SL_RAIL_SUPPORTS_OFDM_PA

#endif//DOXYGEN_UNDOCUMENTED

#ifndef DOXYGEN_SHOULD_SKIP_THIS
// Self-referencing defines minimize compiler complaints when using SLI_RAIL_ENUM
// Only those supported per-platform are defined, for use with #ifdef in
// apps or librail code.
#if     SL_RAIL_SUPPORTS_2P4_GHZ_BAND
#define SL_RAIL_TX_PA_MODE_2P4_GHZ      ((sl_rail_tx_pa_mode_t)(SL_RAIL_TX_PA_MODE_2P4_GHZ))
#define SLI_RAIL_HAS_PA_MODE_2P4_GHZ 1
#else
#define SLI_RAIL_HAS_PA_MODE_2P4_GHZ 0
#endif
#if     SL_RAIL_SUPPORTS_SUB_GHZ_BAND
#define SL_RAIL_TX_PA_MODE_SUB_GHZ      ((sl_rail_tx_pa_mode_t)(SL_RAIL_TX_PA_MODE_SUB_GHZ))
#define SLI_RAIL_HAS_PA_MODE_SUB_GHZ 1
#else
#define SLI_RAIL_HAS_PA_MODE_SUB_GHZ 0
#endif
#if     SL_RAIL_SUPPORTS_OFDM_PA
#define SL_RAIL_TX_PA_MODE_SUB_GHZ_OFDM ((sl_rail_tx_pa_mode_t)(SL_RAIL_TX_PA_MODE_SUB_GHZ_OFDM))
#define SLI_RAIL_HAS_PA_MODE_SUB_GHZ_OFDM 1
#else
#define SLI_RAIL_HAS_PA_MODE_SUB_GHZ_OFDM 0
#endif
#endif//DOXYGEN_SHOULD_SKIP_THIS

/**
 * Indicates on number of PA modes present on a chip.
 */
#define SL_RAIL_TX_PA_MODES_COUNT \
  (SLI_RAIL_HAS_PA_MODE_2P4_GHZ   \
   + SLI_RAIL_HAS_PA_MODE_SUB_GHZ \
   + SLI_RAIL_HAS_PA_MODE_SUB_GHZ_OFDM)

/**
 * The number of PA's on this chip (including Virtual PAs).
 */
#ifndef SL_RAIL_NUM_PA
#if ((_SILICON_LABS_32B_SERIES_2_CONFIG == 2) \
  || (_SILICON_LABS_32B_SERIES_2_CONFIG == 4) \
  || (_SILICON_LABS_32B_SERIES_2_CONFIG == 6) \
  || (_SILICON_LABS_32B_SERIES_2_CONFIG == 7) \
  || (_SILICON_LABS_32B_SERIES_2_CONFIG == 9))
#define SL_RAIL_NUM_PA (2U)
#elif ((_SILICON_LABS_32B_SERIES_2_CONFIG == 3) \
  || (_SILICON_LABS_32B_SERIES_2_CONFIG == 13))
#define SL_RAIL_NUM_PA (4U)
#elif (_SILICON_LABS_32B_SERIES_2_CONFIG == 8)
#define SL_RAIL_NUM_PA (5U)
#elif (_SILICON_LABS_32B_SERIES_2_CONFIG == 5)
#define SL_RAIL_NUM_PA (4U)
#else
#define SL_RAIL_NUM_PA (3U)
#endif
#endif //#ifndef SL_RAIL_NUM_PA

#ifndef DOXYGEN_SHOULD_SKIP_THIS
// Self-referencing defines minimize compiler complaints when using SLI_RAIL_ENUM
// Only those supported per-platform are defined, for use with #ifdef in
// apps or librail code.
#if     SL_RAIL_SUPPORTS_2P4_GHZ_BAND
#define SL_RAIL_TX_POWER_MODE_2P4_GHZ_HP ((sl_rail_tx_power_mode_t) SL_RAIL_TX_POWER_MODE_2P4_GHZ_HP)
#if     _SILICON_LABS_32B_SERIES_2_CONFIG == 1
#define SL_RAIL_TX_POWER_MODE_2P4_GHZ_MP ((sl_rail_tx_power_mode_t) SL_RAIL_TX_POWER_MODE_2P4_GHZ_MP)
#endif//_SILICON_LABS_32B_SERIES_2_CONFIG == 1
#if     ((_SILICON_LABS_32B_SERIES_2_CONFIG != 3) && (_SILICON_LABS_32B_SERIES_2_CONFIG != 8))
#define SL_RAIL_TX_POWER_MODE_2P4_GHZ_LP ((sl_rail_tx_power_mode_t) SL_RAIL_TX_POWER_MODE_2P4_GHZ_LP)
#endif//((_SILICON_LABS_32B_SERIES_2_CONFIG != 3) && (_SILICON_LABS_32B_SERIES_2_CONFIG != 8))
#define SL_RAIL_TX_POWER_MODE_2P4_GHZ_HIGHEST ((sl_rail_tx_power_mode_t) SL_RAIL_TX_POWER_MODE_2P4_GHZ_HIGHEST)
#endif//SL_RAIL_SUPPORTS_2P4_GHZ_BAND

#if     SL_RAIL_SUPPORTS_SUB_GHZ_BAND
#if     SL_RAIL_SUPPORTS_DBM_POWERSETTING_MAPPING_TABLE
#define SL_RAIL_TX_POWER_MODE_SUB_GHZ_POWERSETTING_TABLE ((sl_rail_tx_power_mode_t) SL_RAIL_TX_POWER_MODE_SUB_GHZ_POWERSETTING_TABLE)
#else//!SL_RAIL_SUPPORTS_DBM_POWERSETTING_MAPPING_TABLE
#define SL_RAIL_TX_POWER_MODE_SUB_GHZ_HP ((sl_rail_tx_power_mode_t) SL_RAIL_TX_POWER_MODE_SUB_GHZ_HP)
#define SL_RAIL_TX_POWER_MODE_SUB_GHZ_MP ((sl_rail_tx_power_mode_t) SL_RAIL_TX_POWER_MODE_SUB_GHZ_MP)
#define SL_RAIL_TX_POWER_MODE_SUB_GHZ_LP ((sl_rail_tx_power_mode_t) SL_RAIL_TX_POWER_MODE_SUB_GHZ_LP)
#define SL_RAIL_TX_POWER_MODE_SUB_GHZ_LLP ((sl_rail_tx_power_mode_t) SL_RAIL_TX_POWER_MODE_SUB_GHZ_LLP)
#endif//SL_RAIL_SUPPORTS_DBM_POWERSETTING_MAPPING_TABLE
#define SL_RAIL_TX_POWER_MODE_SUB_GHZ_HIGHEST ((sl_rail_tx_power_mode_t) SL_RAIL_TX_POWER_MODE_SUB_GHZ_HIGHEST)
#endif//SL_RAIL_SUPPORTS_SUB_GHZ_BAND
#if     SL_RAIL_SUPPORTS_OFDM_PA
#if     SL_RAIL_SUPPORTS_DBM_POWERSETTING_MAPPING_TABLE
#define SL_RAIL_TX_POWER_MODE_OFDM_PA_POWERSETTING_TABLE ((sl_rail_tx_power_mode_t) SL_RAIL_TX_POWER_MODE_OFDM_PA_POWERSETTING_TABLE)
#endif//SL_RAIL_SUPPORTS_DBM_POWERSETTING_MAPPING_TABLE
#endif//SL_RAIL_SUPPORTS_OFDM_PA
#endif//DOXYGEN_SHOULD_SKIP_THIS

/**
 * Convenience macro for any mapping table mode.
 */
#define SL_RAIL_POWER_MODE_IS_ANY_DBM_POWERSETTING_MAPPING_TABLE(x) \
  ((x) != SL_RAIL_TX_POWER_MODE_NONE)

/**
 * Convenience macro to check if the power mode supports raw setting.
 */
#define SL_RAIL_POWER_MODE_SUPPORTS_RAW_SETTING(x)           \
  (((x) != SL_RAIL_TX_POWER_MODE_OFDM_PA_POWERSETTING_TABLE) \
   && ((x) != SL_RAIL_TX_POWER_MODE_SUB_GHZ_POWERSETTING_TABLE))

/** @} */ // end of group PA_EFR32XG2X

/******************************************************************************
 * RX Channel Hopping
 *****************************************************************************/
/**
 * @addtogroup Rx_Channel_Hopping_EFR32XG2X EFR32xG2x
 * @ingroup Rx_Channel_Hopping
 * @{
 * @brief EFR32xG2x-specific RX channel hopping.
 */

/**
 * The static amount of memory needed per channel for channel hopping, measured
 * in 32 bit words, regardless of the size of radio configuration structures.
 */
#if _SILICON_LABS_32B_SERIES_2_CONFIG == 8
#define SL_RAIL_CHANNEL_HOPPING_BUFFER_WORDS_PER_CHANNEL (65U)
#else
#define SL_RAIL_CHANNEL_HOPPING_BUFFER_WORDS_PER_CHANNEL (56U)
#endif

#if     (SL_RAIL_CHANNEL_HOPPING_BUFFER_WORDS_PER_CHANNEL \
         > SL_RAIL_CHANNEL_HOPPING_BUFFER_WORDS_PER_CHANNEL_WORST_CASE)
#error "Update sl_rail_types.h SL_RAIL_CHANNEL_HOPPING_BUFFER_WORDS_PER_CHANNEL_WORST_CASE"
#endif

/** @} */  // end of group Rx_Channel_Hopping_EFR32XG2X

/******************************************************************************
 * Sleep Structures
 *****************************************************************************/
/**
 * @addtogroup Sleep_EFR32XG2X EFR32xG2x
 * @ingroup Sleep
 * @{
 * @brief EFR32xG2x-specific Sleeping.
 */

/**
 * Default PRS channel to use when configuring sleep.
 */
#define SLI_RAIL_TIMER_SYNC_PRS_CHANNEL_DEFAULT  (7U)

#ifndef SLI_RAIL_TIMER_SYNC_RTCC_CHANNEL_DEFAULT
/**
 * Default RTCC channel to use when configuring sleep.
 */
#if ((_SILICON_LABS_32B_SERIES_2_CONFIG == 2) \
  || (_SILICON_LABS_32B_SERIES_2_CONFIG == 7) \
  || (_SILICON_LABS_32B_SERIES_2_CONFIG == 9))
#define SLI_RAIL_TIMER_SYNC_RTCC_CHANNEL_DEFAULT (1U)
#else
#define SLI_RAIL_TIMER_SYNC_RTCC_CHANNEL_DEFAULT (0U)
#endif
#endif //#ifndef SLI_RAIL_TIMER_SYNC_RTCC_CHANNEL_DEFAULT

/** @} */ // end of group Sleep_EFR32XG2X

/******************************************************************************
 * State Transitions
 *****************************************************************************/
/**
 * @addtogroup State_Transitions_EFR32XG2X EFR32xG2x
 * @ingroup State_Transitions
 * @{
 * @brief EFR32xG2x-specific State Transitions.
 */

/**
 * @def SL_RAIL_MINIMUM_TRANSITION_US
 * @brief The minimum value for a consistent RAIL transition.
 * @note Transitions may need to be slower than this when using longer
 *   \ref sl_rail_set_tx_pa_ramp_time() values.
 */
#define SL_RAIL_MINIMUM_TRANSITION_US (100U)

/**
 * @def SL_RAIL_MAXIMUM_TRANSITION_US
 * @brief The maximum value for a consistent RAIL transition.
 */
#define SL_RAIL_MAXIMUM_TRANSITION_US (1000000U)

#ifndef DOXYGEN_UNDOCUMENTED
/**
 * Internal Radio State type mapping for EFR32 chips.
 */
typedef sl_rail_radio_state_efr32_t sl_rail_rac_radio_state_t;
#endif//DOXYGEN_UNDOCUMENTED

/** @} */ // end of group State_Transitions_EFR32XG2X

#ifdef __cplusplus
}
#endif

#endif //SL_RAIL_TYPES_H

#endif //SL_RAIL_CHIP_SPECIFIC_H

#endif //SLI_LIBRARY_BUILD
