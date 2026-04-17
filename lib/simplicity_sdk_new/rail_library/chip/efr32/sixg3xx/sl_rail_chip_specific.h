/***************************************************************************//**
 * @file
 * @brief This file contains the type definitions for SIxx3xx chip-specific
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

#if     (defined(DOXYGEN_SHOULD_SKIP_THIS) && !defined(SLI_RAIL_ENUM))
//  Copied from sl_rail_types.h to satisfy doxygen build.
/// The RAIL library does not use enumerations because the ARM EABI leaves their
/// size ambiguous, which causes problems if the application is built
/// with different flags than the library. Instead, uint8_t typedefs
/// are used in compiled code for all enumerations. For documentation purposes, this is
/// converted to an actual enumeration since it's much easier to read in Doxygen.
#define SLI_RAIL_ENUM(name) enum name
/// This macro is a more generic version of the \ref SLI_RAIL_ENUM() macro that
/// allows the size of the type to be overridden instead of forcing the use of
/// a uint8_t. See \ref SLI_RAIL_ENUM() for more information.
#define SLI_RAIL_ENUM_GENERIC(name, type) enum name
#endif //(defined(DOXYGEN_SHOULD_SKIP_THIS) && !defined(SLI_RAIL_ENUM))

/******************************************************************************
 * General Structures
 *****************************************************************************/
/**
 * @addtogroup General_SIXX3XX SIxx3xx
 * @ingroup General
 * @{
 * @brief Types specific to the SIxx3xx for general configuration.
 */

/**
 * Synonym of \ref SL_RAIL_EFR32_HANDLE for Series 3.
 */
#define SL_RAIL_S3LPW_HANDLE SL_RAIL_EFR32_HANDLE

#ifndef DOXYGEN_UNDOCUMENTED

/**
 * @def SL_RAIL_SIXG301_REDUCED_STATE_BUFFER_BYTES
 * @brief The SIxx301 series size needed for REDUCE_SEQ_SZ
 *   \ref sl_rail_state_buffer_entry_t::buffer_bytes.
 */
#define SL_RAIL_SIXG301_REDUCED_STATE_BUFFER_BYTES 624U  // DO NOT HAND-EDIT THESE VALUES

/**
 * @def SL_RAIL_SIXG301_STATE_BUFFER_BYTES
 * @brief The SIxx301 series size needed for
 *   \ref sl_rail_state_buffer_entry_t::buffer_bytes.
 */
#define SL_RAIL_SIXG301_STATE_BUFFER_BYTES 640U  // DO NOT HAND-EDIT THESE VALUES

#ifndef SL_RAIL_STATE_BUFFER_BYTES
/**
 * @def SL_RAIL_STATE_BUFFER_BYTES
 * @brief The size needed for \ref sl_rail_state_buffer_entry_t::buffer_bytes
 *   on this platform for this radio. This compile-time size may be slightly
 *   larger than what \ref sl_rail_get_state_buffer_bytes() determines at run-time.
 */
#if (_SILICON_LABS_32B_SERIES_3_CONFIG == 301)
#ifdef REDUCE_SEQ_SZ
#define SL_RAIL_STATE_BUFFER_BYTES SL_RAIL_SIXG301_REDUCED_STATE_BUFFER_BYTES
#else
#define SL_RAIL_STATE_BUFFER_BYTES SL_RAIL_SIXG301_STATE_BUFFER_BYTES
#endif
#elif (_SILICON_LABS_32B_SERIES_3_CONFIG == 300)
#define SL_RAIL_STATE_BUFFER_BYTES SL_RAIL_SIXG301_REDUCED_STATE_BUFFER_BYTES
#else
#define SL_RAIL_STATE_BUFFER_BYTES 0 // Sate Doxygen
#error "Unsupported platform!"
#endif
#endif //#ifndef SL_RAIL_STATE_BUFFER_BYTES

#endif//DOXYGEN_UNDOCUMENTED

#ifndef DOXYGEN_UNDOCUMENTED

#ifndef DOXYGEN_SHOULD_SKIP_THIS
/**
 * @typedef sl_rail_timer_tick_t
 * @brief Internal RAIL hardware timer tick that drives the RAIL timebase.
 *   A tick is roughly 0.125 microseconds and it has a full 64-bit range
 *   (i.e, spanning 2^61 microseconds or ~73 millenia).
 *
 * @note \ref sl_rail_timer_ticks_to_us() can be used to convert the delta between
 *   two \ref sl_rail_timer_tick_t values to microseconds.
 */
typedef uint64_t sl_rail_timer_tick_t;
#endif//DOXYGEN_SHOULD_SKIP_THIS

/**
 * @typedef sl_rail_get_timer_tick_t
 * @brief A function pointer type for reading RAIL internal timer ticks.
 *
 * @param[in] timer_tick_type A timer tick type to read.
 * @return RAIL timer tick corresponding to the timer_tick_type.
 */
typedef sl_rail_timer_tick_t (*sl_rail_get_timer_tick_t)(sl_rail_timer_tick_type_t timer_tick_type);

/**
 * Function pointer of type \ref sl_rail_get_timer_tick_t to read RAIL internal
 * timer ticks.
 */
extern sl_rail_get_timer_tick_t sl_rail_get_timer_tick;

/**
 * A global pointer to the memory address of the least significant 32 bits
 * of the \ref sl_rail_timer_tick_t internal RAIL hardware timer that drives
 * the RAIL timebase.
 * It's 0.125 microsecond tick range is 2^29 microseconds or ~9 minutes.
 */
extern volatile uint32_t * const sl_rail_timer_tick;

/**
 * A global pointer to the memory address of the least significant 32 bits
 * of the \ref sl_rail_timer_tick_t internal RAIL hardware timer that captures
 * the latest RX packet reception time.
 * It's 0.125 microsecond tick range is 2^29 microseconds or ~9 minutes.
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

/** @} */ // end of group General_SIXX3XX

/******************************************************************************
 * Multiprotocol
 *****************************************************************************/
/**
 * @addtogroup Multiprotocol_SIXX3XX SIxx3xx
 * @ingroup Multiprotocol
 * @{
 * @brief SIxx3xx-specific multiprotocol support defines.
 */

/**
 * @def SL_RAIL_TRANSITION_TIME_US
 * @brief Time it takes to take care of protocol switching.
 */
#define SL_RAIL_TRANSITION_TIME_US 450

/** @} */ // end of group Multiprotocol_SIXX3XX

/******************************************************************************
 * Calibration
 *****************************************************************************/
/**
 * @addtogroup Calibration_SIXX3XX SIxx3xx
 * @ingroup Calibration
 * @{
 * @brief SIxx3xx-specific Calibrations.
 */

/**
 * @def SL_RAIL_RF_PATHS_2P4_GHZ
 * @brief Indicates the number of 2.4 GHz RF Paths suppported.
 */

#ifndef SL_RAIL_RF_PATHS_2P4_GHZ
#if (_SILICON_LABS_32B_SERIES_3_CONFIG == 301) || (_SILICON_LABS_32B_SERIES_3_CONFIG == 300)
#define SL_RAIL_RF_PATHS_2P4_GHZ 1
#else
#define SL_RAIL_RF_PATHS_2P4_GHZ 0
#endif
#endif

/**
 * @def SL_RAIL_RF_PATHS_SUB_GHZ
 * @brief Indicates the number of Sub-GHz RF Paths supported.
 */
#define SL_RAIL_RF_PATHS_SUB_GHZ 0

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

/** @} */ // end of group Calibration_SIXX3XX

/******************************************************************************
 * Transmit
 *****************************************************************************/
/**
 * @addtogroup PA_SIXX3XX SIxx3xx
 * @ingroup PA
 * @{
 * @brief Types specific to the SIxx3xx for dealing with the on-chip PAs.
 */

#ifndef DOXYGEN_UNDOCUMENTED

#ifndef SLI_RAIL_TX_POWER_LEVEL_2P4_GHZ_HP_MAX
#if (_SILICON_LABS_32B_SERIES_3_CONFIG == 301) || (_SILICON_LABS_32B_SERIES_3_CONFIG == 300)
/**
 * The minimum valid value for the \ref sli_rail_tx_power_level_t when in \ref
 * SL_RAIL_TX_POWER_MODE_2P4_GHZ_HP or \ref SL_RAIL_TX_POWER_MODE_2P4_GHZ_LP modes.
 */
#define SLI_RAIL_TX_POWER_LEVEL_2P4_GHZ_HP_LP_MIN     (0U)
/**
 * The minimum valid value for the \ref sli_rail_tx_power_level_t when in \ref
 * \ref SL_RAIL_TX_POWER_MODE_2P4_GHZ_LP mode.
 */
#define SLI_RAIL_TX_POWER_LEVEL_2P4_GHZ_LP_MIN (SLI_RAIL_TX_POWER_LEVEL_2P4_GHZ_HP_LP_MIN)
/**
 * The maximum valid value for the \ref sli_rail_tx_power_level_t when in \ref
 * \ref SL_RAIL_TX_POWER_MODE_2P4_GHZ_LP mode.
 */
#define SLI_RAIL_TX_POWER_LEVEL_2P4_GHZ_LP_MAX (31U)
/**
 * The minimum valid value for the \ref sli_rail_tx_power_level_t when in \ref
 * \ref SL_RAIL_TX_POWER_MODE_2P4_GHZ_HP mode.
 */
#define SLI_RAIL_TX_POWER_LEVEL_2P4_GHZ_HP_MIN (SLI_RAIL_TX_POWER_LEVEL_2P4_GHZ_HP_LP_MIN)
/**
 * The maximum valid value for the \ref sli_rail_tx_power_level_t when in \ref
 * \ref SL_RAIL_TX_POWER_MODE_2P4_GHZ_HP mode.
 */
#define SLI_RAIL_TX_POWER_LEVEL_2P4_GHZ_HP_MAX (95U)
#else
#error "SLI_RAIL_TX_POWER_LEVEL not defined for this device"
#endif
#endif

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
#if (_SILICON_LABS_32B_SERIES_3_CONFIG == 301) || (_SILICON_LABS_32B_SERIES_3_CONFIG == 300)
#define SL_RAIL_NUM_PA (2U)
#else
#error "SL_RAIL_NUM_PA undefined for platform"
#endif
#endif

#ifndef DOXYGEN_SHOULD_SKIP_THIS
// Self-referencing defines minimize compiler complaints when using SLI_RAIL_ENUM
// Only those supported per-platform are defined, for use with #ifdef in
// apps or librail code.
#if     SL_RAIL_SUPPORTS_2P4_GHZ_BAND
#define SL_RAIL_TX_POWER_MODE_2P4_GHZ_HP      ((sl_rail_tx_power_mode_t) SL_RAIL_TX_POWER_MODE_2P4_GHZ_HP)
#define SL_RAIL_TX_POWER_MODE_2P4_GHZ_LP      ((sl_rail_tx_power_mode_t) SL_RAIL_TX_POWER_MODE_2P4_GHZ_LP)
#define SL_RAIL_TX_POWER_MODE_2P4_GHZ_HIGHEST ((sl_rail_tx_power_mode_t) SL_RAIL_TX_POWER_MODE_2P4_GHZ_HIGHEST)
#endif//SL_RAIL_SUPPORTS_2P4_GHZ_BAND
#endif//DOXYGEN_SHOULD_SKIP_THIS

/**
 * Convenience macro for any mapping table mode.
 */
#define SL_RAIL_POWER_MODE_IS_ANY_DBM_POWERSETTING_MAPPING_TABLE(x) \
  ((x) != SL_RAIL_TX_POWER_MODE_NONE)

/**
 * Convenience macro to check if the power mode supports raw setting.
 */
#define SL_RAIL_POWER_MODE_SUPPORTS_RAW_SETTING(x) \
  (((x) == SL_RAIL_TX_POWER_MODE_2P4_GHZ_HP) || ((x) == SL_RAIL_TX_POWER_MODE_2P4_GHZ_LP))

/** @} */ // end of group PA_SIXX3XX

/******************************************************************************
 * RX Channel Hopping
 *****************************************************************************/
/**
 * @addtogroup Rx_Channel_Hopping_SIXX3XX SIxx3xx
 * @ingroup Rx_Channel_Hopping
 * @{
 * @brief SIxx3xx-specific RX channel hopping.
 */

/**
 * The static amount of memory needed per channel for channel hopping, measured
 * in 32 bit words, regardless of the size of radio configuration structures.
 */
#define SL_RAIL_CHANNEL_HOPPING_BUFFER_WORDS_PER_CHANNEL (54U)

#if     (SL_RAIL_CHANNEL_HOPPING_BUFFER_WORDS_PER_CHANNEL \
         > SL_RAIL_CHANNEL_HOPPING_BUFFER_WORDS_PER_CHANNEL_WORST_CASE)
#error "Update sl_rail_types.h SL_RAIL_CHANNEL_HOPPING_BUFFER_WORDS_PER_CHANNEL_WORST_CASE"
#endif

/** @} */  // end of group Rx_Channel_Hopping_SIXX3XX

/******************************************************************************
 * Sleep Structures
 *****************************************************************************/
/**
 * @addtogroup Sleep_SIXX3XX SIxx3xx
 * @ingroup Sleep
 * @{
 * @brief SIxx3xx-specific Sleeping.
 */

/**
 * Default PRS channel to use when configuring sleep.
 */
#define SLI_RAIL_TIMER_SYNC_PRS_CHANNEL_DEFAULT  (7U)

/**
 * Default RTCC channel to use when configuring sleep.
 */
#define SLI_RAIL_TIMER_SYNC_RTCC_CHANNEL_DEFAULT (0U)

/** @} */ // end of group Sleep_SIXX3XX

/******************************************************************************
 * State Transitions
 *****************************************************************************/
/**
 * @addtogroup State_Transitions_SIXX3XX SIxx3xx
 * @ingroup State_Transitions
 * @{
 * @brief SIxx3xx-specific State Transitions.
 */

/**
 * @def SL_RAIL_MINIMUM_TRANSITION_US
 * @brief The minimum value for a consistent RAIL transition
 * @note Transitions may need to be slower than this when using longer
 *   \ref sl_rail_set_tx_pa_ramp_time() values
 */
#define SL_RAIL_MINIMUM_TRANSITION_US (75U)

/**
 * @def SL_RAIL_MAXIMUM_TRANSITION_US
 * @brief The maximum value for a consistent RAIL transition
 */
#define SL_RAIL_MAXIMUM_TRANSITION_US (1000000U)

#ifndef DOXYGEN_UNDOCUMENTED

/**
 * @enum sl_rail_radio_state_s3lpw_t
 * @brief Detailed Series 3 LPW radio state machine states.
 */
SLI_RAIL_ENUM(sl_rail_radio_state_s3lpw_t) {
  /**
   * Radio is off.
   */
  SL_RAIL_RAC_STATE_S3LPW_OFF = 0u,
  /**
   * Radio is enabling the receiver.
   */
  SL_RAIL_RAC_STATE_S3LPW_RXWARM = 1u,
  /**
   * Radio is listening for incoming frames.
   */
  SL_RAIL_RAC_STATE_S3LPW_RXSEARCH = 2u,
  /**
   * Radio is receiving a frame.
   */
  SL_RAIL_RAC_STATE_S3LPW_RXFRAME = 3u,
  /**
   * Radio is wrapping up after receiving a frame.
   */
  SL_RAIL_RAC_STATE_S3LPW_RXWRAPUP = 4u,
  /**
   * Radio is enabling transmitter.
   */
  SL_RAIL_RAC_STATE_S3LPW_TXWARM = 5u,
  /**
   * Radio is transmitting data.
   */
  SL_RAIL_RAC_STATE_S3LPW_TX = 6u,
  /**
   * Radio is wrapping up after transmitting a frame.
   */
  SL_RAIL_RAC_STATE_S3LPW_TXWRAPUP = 7u,
  /**
   * Radio is powering down and going to OFF state.
   */
  SL_RAIL_RAC_STATE_S3LPW_SHUTDOWN = 8u,
  /**
   * Radio power-on-reset state.
   */
  SL_RAIL_RAC_STATE_S3LPW_POR = 9u,
  /**
   * Invalid Radio state, must be the last entry.
   */
  SL_RAIL_RAC_STATE_S3LPW_NONE
};

#ifndef DOXYGEN_SHOULD_SKIP_THIS
// Self-referencing defines minimize compiler complaints when using SLI_RAIL_ENUM
#define SL_RAIL_RAC_STATE_S3LPW_OFF         ((sl_rail_radio_state_s3lpw_t) SL_RAIL_RAC_STATE_S3LPW_OFF)
#define SL_RAIL_RAC_STATE_S3LPW_RXWARM      ((sl_rail_radio_state_s3lpw_t) SL_RAIL_RAC_STATE_S3LPW_RXWARM)
#define SL_RAIL_RAC_STATE_S3LPW_RXSEARCH    ((sl_rail_radio_state_s3lpw_t) SL_RAIL_RAC_STATE_S3LPW_RXSEARCH)
#define SL_RAIL_RAC_STATE_S3LPW_RXFRAME     ((sl_rail_radio_state_s3lpw_t) SL_RAIL_RAC_STATE_S3LPW_RXFRAME)
#define SL_RAIL_RAC_STATE_S3LPW_RXWRAPUP    ((sl_rail_radio_state_s3lpw_t) SL_RAIL_RAC_STATE_S3LPW_RXWRAPUP)
#define SL_RAIL_RAC_STATE_S3LPW_TXWARM      ((sl_rail_radio_state_s3lpw_t) SL_RAIL_RAC_STATE_S3LPW_TXWARM)
#define SL_RAIL_RAC_STATE_S3LPW_TX          ((sl_rail_radio_state_s3lpw_t) SL_RAIL_RAC_STATE_S3LPW_TX)
#define SL_RAIL_RAC_STATE_S3LPW_TXWRAPUP    ((sl_rail_radio_state_s3lpw_t) SL_RAIL_RAC_STATE_S3LPW_TXWRAPUP)
#define SL_RAIL_RAC_STATE_S3LPW_SHUTDOWN    ((sl_rail_radio_state_s3lpw_t) SL_RAIL_RAC_STATE_S3LPW_SHUTDOWN)
#define SL_RAIL_RAC_STATE_S3LPW_POR         ((sl_rail_radio_state_s3lpw_t) SL_RAIL_RAC_STATE_S3LPW_POR)
#define SL_RAIL_RAC_STATE_S3LPW_NONE        ((sl_rail_radio_state_s3lpw_t) SL_RAIL_RAC_STATE_S3LPW_NONE)
/**
 * Redefined to use \ref sl_rail_radio_state_s3lpw_t instead of \ref sl_rail_radio_state_efr32_t.
 */
#ifdef  SL_RAIL_RAC_STATE_NONE
#undef  SL_RAIL_RAC_STATE_NONE
#define SL_RAIL_RAC_STATE_NONE SL_RAIL_RAC_STATE_S3LPW_NONE
#endif
#endif//DOXYGEN_SHOULD_SKIP_THIS

/**
 * Internal Radio State type mapping for SIxx3xx chips.
 */
typedef sl_rail_radio_state_s3lpw_t sl_rail_rac_radio_state_t;

#endif//DOXYGEN_UNDOCUMENTED

/** @} */ // end of group State_Transitions_SIXX3XX

#ifdef __cplusplus
}
#endif

#endif //SL_RAIL_TYPES_H

#endif //SL_RAIL_CHIP_SPECIFIC_H

#endif //SLI_LIBRARY_BUILD
