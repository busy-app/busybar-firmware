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

#ifndef __RAIL_CHIP_SPECIFIC_H_
#if !defined(__RAIL_TYPES_H__) && !defined(DOXYGEN_SHOULD_SKIP_THIS)
#warning rail_chip_specific.h should only be included by rail_types.h
#include "rail_types.h" // Force rail_chip_specific.h only within rail_types.h
#else // __RAIL_TYPES_H__
/// Include guard
#define __RAIL_CHIP_SPECIFIC_H_

#include "rail_features.h"
#ifdef RAIL_PRIVATE_BUILD
#include "rail_chip_specific_internal.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if     (defined(DOXYGEN_SHOULD_SKIP_THIS) && !defined(RAIL_ENUM))
//  Copied from rail_types.h to satisfy doxygen build.
/// The RAIL library does not use enumerations because the ARM EABI leaves their
/// size ambiguous, which causes problems if the application is built
/// with different flags than the library. Instead, uint8_t typedefs
/// are used in compiled code for all enumerations. For documentation purposes, this is
/// converted to an actual enumeration since it's much easier to read in Doxygen.
#define RAIL_ENUM(name) enum name
/// This macro is a more generic version of the \ref RAIL_ENUM() macro that
/// allows the size of the type to be overridden instead of forcing the use of
/// a uint8_t. See \ref RAIL_ENUM() for more information.
#define RAIL_ENUM_GENERIC(name, type) enum name
#endif //(defined(DOXYGEN_SHOULD_SKIP_THIS) && !defined(RAIL_ENUM))

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
 * Synonym of \ref RAIL_EFR32_HANDLE for Series 3.
 *
 * @deprecated RAIL 2.x. synonym of \ref SL_RAIL_S3LPW_HANDLE.
 */
#define RAIL_S3LPW_HANDLE RAIL_EFR32_HANDLE

#ifndef DOXYGEN_UNDOCUMENTED

/**
 * @def RAIL_SIXG301_REDUCED_STATE_BUFFER_BYTES
 * @brief The SIxx301 series size needed for REDUCE_SEQ_SZ
 *   \ref RAIL_StateBufferEntry_t::bufferBytes.
 *
 * @deprecated RAIL 2.x. synonym of \ref SL_RAIL_SIXG301_REDUCED_STATE_BUFFER_BYTES.
 */
#define RAIL_SIXG301_REDUCED_STATE_BUFFER_BYTES 624U  // DO NOT HAND-EDIT THESE VALUES

/**
 * @def RAIL_SIXG301_STATE_BUFFER_BYTES
 * @brief The SIxx301 series size needed for
 *   \ref RAIL_StateBufferEntry_t::bufferBytes.
 *
 * @deprecated RAIL 2.x. synonym of \ref SL_RAIL_SIXG301_STATE_BUFFER_BYTES.
 */
#define RAIL_SIXG301_STATE_BUFFER_BYTES 640U  // DO NOT HAND-EDIT THESE VALUES

#ifndef RAIL_STATE_BUFFER_BYTES
/**
 * @def RAIL_STATE_BUFFER_BYTES
 * @brief The size needed for \ref RAIL_StateBufferEntry_t::bufferBytes
 *   on this platform for this radio. This compile-time size may be slightly
 *   larger than what \ref RAIL_GetStateBufferSize() determines at run-time.
 *
 * @deprecated RAIL 2.x. synonym of \ref SL_RAIL_STATE_BUFFER_BYTES.
 */
#if (_SILICON_LABS_32B_SERIES_3_CONFIG == 301)
#ifdef REDUCE_SEQ_SZ
#define RAIL_STATE_BUFFER_BYTES RAIL_SIXG301_REDUCED_STATE_BUFFER_BYTES
#else
#define RAIL_STATE_BUFFER_BYTES RAIL_SIXG301_STATE_BUFFER_BYTES
#endif
#elif (_SILICON_LABS_32B_SERIES_3_CONFIG == 300)
#define RAIL_STATE_BUFFER_BYTES RAIL_SIXG301_REDUCED_STATE_BUFFER_BYTES
#else
#define RAIL_STATE_BUFFER_BYTES 0 // Sate Doxygen
#error "Unsupported platform!"
#endif
#endif //#ifndef RAIL_STATE_BUFFER_BYTES

#endif//DOXYGEN_UNDOCUMENTED

#ifndef DOXYGEN_UNDOCUMENTED

#ifndef DOXYGEN_SHOULD_SKIP_THIS
/**
 * @typedef RAIL_TimerTick_t
 * @brief Internal RAIL hardware timer tick that drives the RAIL timebase.
 *   A tick is roughly 0.125 microseconds and it has a full 64-bit range
 *   (i.e, spanning 2^61 microseconds or ~73 millenia).
 *
 * @note \ref RAIL_TimerTicksToUs() can be used to convert the delta between
 *   two \ref RAIL_TimerTick_t values to microseconds.
 *
 * @deprecated RAIL 2.x. synonym of \ref sl_rail_timer_tick_t.
 */
typedef uint64_t RAIL_TimerTick_t;
#endif//DOXYGEN_SHOULD_SKIP_THIS

/**
 * @typedef RAIL_GetTimerTick_t
 * @brief A function pointer type for reading RAIL internal timer ticks.
 *
 * @param[in] timerTickType A timer tick type to read.
 * @return RAIL timer tick corresponding to the timerTickType.
 *
 * @deprecated RAIL 2.x. synonym of \ref sl_rail_get_timer_tick_t().
 */
typedef RAIL_TimerTick_t (*RAIL_GetTimerTick_t)(RAIL_TimerTickType_t timerTickType);

/**
 * Function pointer of type \ref RAIL_GetTimerTick_t to read RAIL internal
 * timer ticks.
 *
 * @deprecated RAIL 2.x. synonym of \ref sl_rail_get_timer_tick().
 */
extern RAIL_GetTimerTick_t RAIL_GetTimerTick;

#ifndef SLI_LIBRAIL_ALIAS

/**
 * A global pointer to the memory address of the least significant 32 bits
 * of the \ref RAIL_TimerTick_t internal RAIL hardware timer that drives
 * the RAIL timebase.
 * It's 0.125 microsecond tick range is 2^29 microseconds or ~9 minutes.
 *
 * @deprecated RAIL 2.x. synonym of \ref sl_rail_timer_tick.
 */
extern volatile uint32_t * const RAIL_TimerTick;

/**
 * A global pointer to the memory address of the least significant 32 bits
 * of the \ref RAIL_TimerTick_t internal RAIL hardware timer that captures
 * the latest RX packet reception time.
 * It's 0.125 microsecond tick range is 2^29 microseconds or ~9 minutes.
 *
 * @note This would not include the RX chain delay, so may not exactly
 *   correspond to the \ref RAIL_Time_t packet time stamp available within
 *   \ref RAIL_RxPacketDetails_t::timeReceived which reflects the actual
 *   on-air time that the packet finished.
 *
 * @deprecated RAIL 2.x. synonym of \ref sl_rail_rx_packet_time_stamp.
 */
extern volatile uint32_t * const RAIL_RxPacketTimestamp;

/**
 * Get elapsed time, in microseconds, between two \ref RAIL_TimerTick_t ticks.
 *
 * @param[in] startTick Tick recorded at the start of the operation.
 * @param[in] endTick Tick recorded at the end of the operation.
 * @return The elapsed time, in microseconds, between two timer ticks.
 *
 * @deprecated RAIL 2.x. synonym of \ref sl_rail_timer_ticks_to_us().
 */
RAIL_Time_t RAIL_TimerTicksToUs(RAIL_TimerTick_t startTick,
                                RAIL_TimerTick_t endTick);

/**
 * Get \ref RAIL_TimerTick_t tick corresponding to a \ref RAIL_Time_t time.
 *
 * @param[in] microseconds Time in microseconds.
 * @return The \ref RAIL_TimerTick_t tick corresponding to the
 *   \ref RAIL_Time_t time.
 *
 * @deprecated RAIL 2.x. synonym of \ref sl_rail_us_to_timer_ticks().
 */
RAIL_TimerTick_t RAIL_UsToTimerTicks(RAIL_Time_t microseconds);

#endif//SLI_LIBRAIL_ALIAS

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
 * @def TRANSITION_TIME_US
 * @brief Time it takes to take care of protocol switching.
 *
 * @deprecated RAIL 2.x. synonym of \ref SL_RAIL_TRANSITION_TIME_US.
 */
#define TRANSITION_TIME_US 450

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
 * @def RAIL_RF_PATHS_2P4GIG
 * @brief Indicates the number of 2.4 GHz RF Paths suppported.
 *
 * @deprecated RAIL 2.x. synonym of \ref SL_RAIL_RF_PATHS_2P4_GHZ.
 */
#ifndef RAIL_RF_PATHS_2P4GIG
#if (_SILICON_LABS_32B_SERIES_3_CONFIG == 301) || (_SILICON_LABS_32B_SERIES_3_CONFIG == 300)
#define RAIL_RF_PATHS_2P4GIG 1
#else
#define RAIL_RF_PATHS_2P4GIG 0
#endif
#endif

/**
 * @def RAIL_RF_PATHS_SUBGIG
 * @brief Indicates the number of Sub-GHz RF Paths supported.
 *
 * @deprecated RAIL 2.x. synonym of \ref SL_RAIL_RF_PATHS_SUB_GHZ.
 */
#define RAIL_RF_PATHS_SUBGIG 0

/**
 * @def RAIL_RF_PATHS
 * @brief Indicates the number of RF Paths supported.
 *
 * @deprecated RAIL 2.x. synonym of \ref SL_RAIL_RF_PATHS.
 */
#define RAIL_RF_PATHS (RAIL_RF_PATHS_SUBGIG + RAIL_RF_PATHS_2P4GIG)

#if     (RAIL_RF_PATHS > RAIL_MAX_RF_PATHS)
#error "Update rail_types.h RAIL_MAX_RF_PATHS"
#endif

/**
 * @def RADIO_CONFIG_ENABLE_IRCAL_MULTIPLE_RF_PATHS
 * @brief Indicates this version of RAIL supports IR calibration on multiple RF paths
 * Needed for backwards compatibility.
 *
 * @deprecated RAIL 2.x. synonym of \ref SL_RAIL_RADIO_CONFIG_ENABLE_IR_CAL_MULTIPLE_RF_PATHS.
 */
#if RAIL_RF_PATHS > 1
#define RADIO_CONFIG_ENABLE_IRCAL_MULTIPLE_RF_PATHS 1
#else
#ifdef  DOXYGEN_SHOULD_SKIP_THIS // Leave undefined except for doxygen
#define RADIO_CONFIG_ENABLE_IRCAL_MULTIPLE_RF_PATHS 0
#endif //DOXYGEN_SHOULD_SKIP_THIS
#endif //RAIL_RF_PATHS

/**
 * @struct RAIL_ChannelConfigEntryAttr
 * @brief A channel configuration entry attribute structure. Items listed
 *  are designed to be altered and updated during run-time.
 *
 * @deprecated RAIL 2.x. synonym of \ref sl_rail_channel_config_entry_attr.
 */
struct RAIL_ChannelConfigEntryAttr {
  /**
   * IR calibration attributes specific to each channel configuration entry.
   *
   * @deprecated RAIL 2.x. synonym of \ref sl_rail_channel_config_entry_attr::rx_ir_cal_values.
   */
 #if     RAIL_SUPPORTS_OFDM_PA
  RAIL_IrCalValues_t calValues;
 #else//!RAIL_SUPPORTS_OFDM_PA
  RAIL_RxIrCalValues_t calValues;
 #endif//RAIL_SUPPORTS_OFDM_PA
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

#ifndef RAIL_TX_POWER_LEVEL_2P4_HP_MAX
#if (_SILICON_LABS_32B_SERIES_3_CONFIG == 301) || (_SILICON_LABS_32B_SERIES_3_CONFIG == 300)
/**
 * The minimum valid value for the \ref RAIL_TxPowerLevel_t when in \ref
 * RAIL_TX_POWER_MODE_2P4GIG_HP or \ref RAIL_TX_POWER_MODE_2P4GIG_LP modes.
 *
 * @deprecated This RAIL 2.x. define has been eliminated,
 *   temporarily a synonym of SLI_RAIL_TX_POWER_LEVEL_2P4_GHZ_HP_LP_MIN.
 */
#define RAIL_TX_POWER_LEVEL_2P4GIG_HP_LP_MIN     (0U)
/**
 * The minimum valid value for the \ref RAIL_TxPowerLevel_t when in \ref
 * \ref RAIL_TX_POWER_MODE_2P4GIG_LP mode.
 *
 * @deprecated This RAIL 2.x. define has been eliminated,
 *   temporarily a synonym of SLI_RAIL_TX_POWER_LEVEL_2P4_GHZ_LP_MIN.
 */
#define RAIL_TX_POWER_LEVEL_2P4_LP_MIN        (RAIL_TX_POWER_LEVEL_2P4GIG_HP_LP_MIN)
/**
 * The maximum valid value for the \ref RAIL_TxPowerLevel_t when in \ref
 * \ref RAIL_TX_POWER_MODE_2P4GIG_LP mode.
 *
 * @deprecated This RAIL 2.x. define has been eliminated,
 *   temporarily a synonym of SLI_RAIL_TX_POWER_LEVEL_2P4_GHZ_LP_MAX.
 */
#define RAIL_TX_POWER_LEVEL_2P4_LP_MAX        (31U)
/**
 * The minimum valid value for the \ref RAIL_TxPowerLevel_t when in \ref
 * \ref RAIL_TX_POWER_MODE_2P4GIG_HP mode.
 *
 * @deprecated This RAIL 2.x. define has been eliminated,
 *   temporarily a synonym of SLI_RAIL_TX_POWER_LEVEL_2P4_GHZ_HP_MIN.
 */
#define RAIL_TX_POWER_LEVEL_2P4_HP_MIN        (RAIL_TX_POWER_LEVEL_2P4GIG_HP_LP_MIN)
/**
 * The maximum valid value for the \ref RAIL_TxPowerLevel_t when in \ref
 * \ref RAIL_TX_POWER_MODE_2P4GIG_HP mode.
 *
 * @deprecated This RAIL 2.x. define has been eliminated,
 *   temporarily a synonym of SLI_RAIL_TX_POWER_LEVEL_2P4_GHZ_HP_MAX.
 */
#define RAIL_TX_POWER_LEVEL_2P4_HP_MAX        (95U)
#else
#error "RAIL_TX_POWER_LEVEL not defined for this device"
#endif
#endif

/**
 * The number of PA's on this chip (including Virtual PAs).
 *
 * @deprecated RAIL 2.x. synonym of \ref SL_RAIL_NUM_PA.
 */
#ifndef RAIL_NUM_PA
#if (_SILICON_LABS_32B_SERIES_3_CONFIG == 301) || (_SILICON_LABS_32B_SERIES_3_CONFIG == 300)
#define RAIL_NUM_PA (2U)
#else
#error "RAIL_NUM_PA undefined for platform"
#endif
#endif

#ifndef DOXYGEN_SHOULD_SKIP_THIS
// Self-referencing defines minimize compiler complaints when using RAIL_ENUM
// Only those supported per-platform are defined, for use with #ifdef in
// apps or librail code.
#if     RAIL_SUPPORTS_2P4GHZ_BAND
#define RAIL_TX_POWER_MODE_2P4GIG_HP ((RAIL_TxPowerMode_t) RAIL_TX_POWER_MODE_2P4GIG_HP)
#define RAIL_TX_POWER_MODE_2P4_HP ((RAIL_TxPowerMode_t) RAIL_TX_POWER_MODE_2P4_HP)
#define RAIL_TX_POWER_MODE_2P4GIG_LP ((RAIL_TxPowerMode_t) RAIL_TX_POWER_MODE_2P4GIG_LP)
#define RAIL_TX_POWER_MODE_2P4_LP ((RAIL_TxPowerMode_t) RAIL_TX_POWER_MODE_2P4_LP)
#define RAIL_TX_POWER_MODE_2P4GIG_HIGHEST ((RAIL_TxPowerMode_t) RAIL_TX_POWER_MODE_2P4GIG_HIGHEST)
#define RAIL_TX_POWER_MODE_2P4_HIGHEST ((RAIL_TxPowerMode_t) RAIL_TX_POWER_MODE_2P4_HIGHEST)
#endif//RAIL_SUPPORTS_2P4GHZ_BAND
#endif//DOXYGEN_SHOULD_SKIP_THIS

/**
 * Convenience macro for any mapping table mode.
 *
 * @deprecated RAIL 2.x. synonym of \ref SL_RAIL_POWER_MODE_IS_ANY_DBM_POWERSETTING_MAPPING_TABLE().
 */
#define RAIL_POWER_MODE_IS_ANY_DBM_POWERSETTING_MAPPING_TABLE(x) \
  (((x) == RAIL_TX_POWER_MODE_2P4GIG_HP)                         \
   || ((x) == RAIL_TX_POWER_MODE_2P4GIG_LP))

/**
 * Convenience macro to check if the power mode supports raw setting.
 *
 * @deprecated RAIL 2.x. synonym of \ref SL_RAIL_POWER_MODE_SUPPORTS_RAW_SETTING().
 */
#define RAIL_POWER_MODE_SUPPORTS_RAW_SETTING(x) \
  (((x) == RAIL_TX_POWER_MODE_2P4GIG_HP) || ((x) == RAIL_TX_POWER_MODE_2P4GIG_LP))

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
 *
 * @deprecated RAIL 2.x. synonym of \ref SL_RAIL_CHANNEL_HOPPING_BUFFER_WORDS_PER_CHANNEL.
 */
#define RAIL_CHANNEL_HOPPING_BUFFER_SIZE_PER_CHANNEL (54U)

#if     (RAIL_CHANNEL_HOPPING_BUFFER_SIZE_PER_CHANNEL \
         > RAIL_CHANNEL_HOPPING_BUFFER_SIZE_PER_CHANNEL_WORST_CASE)
#error "Update rail_types.h RAIL_CHANNEL_HOPPING_BUFFER_SIZE_PER_CHANNEL_WORST_CASE"
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
 *
 * @deprecated RAIL 2.x. synonym of \ref SLI_RAIL_TIMER_SYNC_PRS_CHANNEL_DEFAULT.
 */
#define RAILINT_TIMER_SYNC_PRS_CHANNEL_DEFAULT  (7U)

/**
 * Default RTCC channel to use when configuring sleep.
 *
 * @deprecated RAIL 2.x. synonym of \ref SLI_RAIL_TIMER_SYNC_RTCC_CHANNEL_DEFAULT.
 */
#define RAILINT_TIMER_SYNC_RTCC_CHANNEL_DEFAULT (0U)

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
 * @def RAIL_MINIMUM_TRANSITION_US
 * @brief The minimum value for a consistent RAIL transition
 * @note Transitions may need to be slower than this when using longer
 *   \ref RAIL_TxPowerConfig_t::rampTime values
 *
 * @deprecated RAIL 2.x. synonym of \ref SL_RAIL_MINIMUM_TRANSITION_US.
 */
#define RAIL_MINIMUM_TRANSITION_US (75U)

/**
 * @def RAIL_MAXIMUM_TRANSITION_US
 * @brief The maximum value for a consistent RAIL transition
 *
 * @deprecated RAIL 2.x. synonym of \ref SL_RAIL_MAXIMUM_TRANSITION_US.
 */
#define RAIL_MAXIMUM_TRANSITION_US (1000000U)

#ifndef DOXYGEN_UNDOCUMENTED

/**
 * @enum RAIL_RadioStateSix3x_t
 * @brief Detailed Series 3 Radio state machine states.
 *
 * @deprecated RAIL 2.x. synonym of \ref sl_rail_radio_state_s3lpw_t.
 */
RAIL_ENUM(RAIL_RadioStateSix3x_t) {
  /**
   * Radio is off.
   *
   * @deprecated RAIL 2.x. synonym of \ref SL_RAIL_RAC_STATE_S3LPW_OFF.
   */
  RAIL_RAC_STATE_SIX3X_OFF = 0,
  /**
   * Radio is enabling the receiver.
   *
   * @deprecated RAIL 2.x. synonym of \ref SL_RAIL_RAC_STATE_S3LPW_RXWARM.
   */
  RAIL_RAC_STATE_SIX3X_RXWARM = 1,
  /**
   * Radio is listening for incoming frames.
   *
   * @deprecated RAIL 2.x. synonym of \ref SL_RAIL_RAC_STATE_S3LPW_RXSEARCH.
   */
  RAIL_RAC_STATE_SIX3X_RXSEARCH = 2,
  /**
   * Radio is receiving a frame.
   *
   * @deprecated RAIL 2.x. synonym of \ref SL_RAIL_RAC_STATE_S3LPW_RXFRAME.
   */
  RAIL_RAC_STATE_SIX3X_RXFRAME = 3,
  /**
   * Radio is wrapping up after receiving a frame.
   *
   * @deprecated RAIL 2.x. synonym of \ref SL_RAIL_RAC_STATE_S3LPW_RXWRAPUP.
   */
  RAIL_RAC_STATE_SIX3X_RXWRAPUP = 4,
  /**
   * Radio is enabling transmitter.
   *
   * @deprecated RAIL 2.x. synonym of \ref SL_RAIL_RAC_STATE_S3LPW_TXWARM.
   */
  RAIL_RAC_STATE_SIX3X_TXWARM = 5,
  /**
   * Radio is transmitting data.
   *
   * @deprecated RAIL 2.x. synonym of \ref SL_RAIL_RAC_STATE_S3LPW_TX.
   */
  RAIL_RAC_STATE_SIX3X_TX = 6,
  /**
   * Radio is wrapping up after transmitting a frame.
   *
   * @deprecated RAIL 2.x. synonym of \ref SL_RAIL_RAC_STATE_S3LPW_TXWRAPUP.
   */
  RAIL_RAC_STATE_SIX3X_TXWRAPUP = 7,
  /**
   * Radio is powering down and going to OFF state.
   *
   * @deprecated RAIL 2.x. synonym of \ref SL_RAIL_RAC_STATE_S3LPW_SHUTDOWN.
   */
  RAIL_RAC_STATE_SIX3X_SHUTDOWN = 8,
  /**
   * Radio power-on-reset state.
   *
   * @deprecated RAIL 2.x. synonym of \ref SL_RAIL_RAC_STATE_S3LPW_POR.
   */
  RAIL_RAC_STATE_SIX3X_POR = 9,
  /**
   * Invalid Radio state, must be the last entry.
   *
   * @deprecated RAIL 2.x. synonym of \ref SL_RAIL_RAC_STATE_S3LPW_NONE.
   */
  RAIL_RAC_STATE_SIX3X_NONE
};

#ifndef DOXYGEN_SHOULD_SKIP_THIS
// Self-referencing defines minimize compiler complaints when using RAIL_ENUM
#define RAIL_RAC_STATE_SIX3X_OFF         ((RAIL_RadioStateSix3x_t) RAIL_RAC_STATE_SIX3X_OFF)
#define RAIL_RAC_STATE_SIX3X_RXWARM      ((RAIL_RadioStateSix3x_t) RAIL_RAC_STATE_SIX3X_RXWARM)
#define RAIL_RAC_STATE_SIX3X_RXSEARCH    ((RAIL_RadioStateSix3x_t) RAIL_RAC_STATE_SIX3X_RXSEARCH)
#define RAIL_RAC_STATE_SIX3X_RXFRAME     ((RAIL_RadioStateSix3x_t) RAIL_RAC_STATE_SIX3X_RXFRAME)
#define RAIL_RAC_STATE_SIX3X_RXWRAPUP    ((RAIL_RadioStateSix3x_t) RAIL_RAC_STATE_SIX3X_RXWRAPUP)
#define RAIL_RAC_STATE_SIX3X_TXWARM      ((RAIL_RadioStateSix3x_t) RAIL_RAC_STATE_SIX3X_TXWARM)
#define RAIL_RAC_STATE_SIX3X_TX          ((RAIL_RadioStateSix3x_t) RAIL_RAC_STATE_SIX3X_TX)
#define RAIL_RAC_STATE_SIX3X_TXWRAPUP    ((RAIL_RadioStateSix3x_t) RAIL_RAC_STATE_SIX3X_TXWRAPUP)
#define RAIL_RAC_STATE_SIX3X_SHUTDOWN    ((RAIL_RadioStateSix3x_t) RAIL_RAC_STATE_SIX3X_SHUTDOWN)
#define RAIL_RAC_STATE_SIX3X_POR         ((RAIL_RadioStateSix3x_t) RAIL_RAC_STATE_SIX3X_POR)
#define RAIL_RAC_STATE_SIX3X_NONE        ((RAIL_RadioStateSix3x_t) RAIL_RAC_STATE_SIX3X_NONE)
/**
 * Redefined to use \ref RAIL_RadioStateSix3x_t instead of \ref RAIL_RadioStateEfr32_t.
 *
 * @deprecated RAIL 2.x synonym of \ref SL_RAIL_RAC_STATE_NONE.
 */
#ifdef  RAIL_RAC_STATE_NONE
#undef  RAIL_RAC_STATE_NONE
#define RAIL_RAC_STATE_NONE RAIL_RAC_STATE_SIX3X_NONE
#endif
#endif//DOXYGEN_SHOULD_SKIP_THIS

/**
 * Internal Radio State type mapping for SIxx3xx chips.
 *
 * @deprecated RAIL 2.x. synonym of \ref sl_rail_rac_radio_state_t.
 */
typedef RAIL_RadioStateSix3x_t RAIL_RacRadioState_t;

#endif//DOXYGEN_UNDOCUMENTED

/** @} */ // end of group State_Transitions_SIXX3XX

#ifdef __cplusplus
}
#endif

#endif //__RAIL_TYPES_H__

#endif //__RAIL_CHIP_SPECIFIC_H_

#endif //SLI_LIBRARY_BUILD
