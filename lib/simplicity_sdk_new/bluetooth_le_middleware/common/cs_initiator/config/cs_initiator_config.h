/***************************************************************************//**
 * @file
 * @brief CS initiator - configuration header
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
#ifndef CS_INITIATOR_CONFIG_H
#define CS_INITIATOR_CONFIG_H

#include "sl_bt_api.h"
#include "sl_rtl_clib_api.h"
#include "cs_sync_antenna.h"

/***********************************************************************************************//**
 * @addtogroup cs_initiator
 * @{
 **************************************************************************************************/

// <<< Use Configuration Wizard in Context Menu >>>

// <h> Logging

// <e CS_INITIATOR_LOG> Initiator component
// <i> Default: 1
// <i> Enable Initiator component logging
#ifndef CS_INITIATOR_LOG
#define CS_INITIATOR_LOG                              (1)
#endif

// <q CS_INITIATOR_CONFIG_LOG_DATA> Data logging
// <i> Default: 0
// <i> Enable Initiator component data logging
#ifndef CS_INITIATOR_CONFIG_LOG_DATA
#define CS_INITIATOR_CONFIG_LOG_DATA                  (0)
#endif

// <s CS_INITIATOR_LOG_PREFIX> Log prefix
// <i> Default: "[Initiator]"
#ifndef CS_INITIATOR_LOG_PREFIX
#define CS_INITIATOR_LOG_PREFIX                       "[Initiator]"
#endif

// </e>

// <q CS_INITIATOR_RTL_LOG> RTL logging
// <i> Default: 1
#ifndef CS_INITIATOR_RTL_LOG
#define CS_INITIATOR_RTL_LOG                          (1)
#endif

// </h>

// <h> General

// <o CS_INITIATOR_MAX_CONNECTIONS> Maximum initiator connections <1..4>
// <i> If more than 1 initiator instances are created SL_BT_CONFIG_BUFFER_SIZE shall be increased.
// <i> Default: 1
#ifndef CS_INITIATOR_MAX_CONNECTIONS
#define CS_INITIATOR_MAX_CONNECTIONS                  (1)
#endif

// <o CS_INITIATOR_MAX_RANGING_DATA_SIZE> Maximum ranging data size <32..2500>
// <i> The optimal value of "Maximum ranging data size" is dependent on several configuration values,
// <i> and can be calculated by the following equation:
// <i> ranging_max_size = 4 + (subevents * 8) + (mode0_steps * mode0_size) +
// <i> channels * ( ( 1 + ( antenna_paths + 1 ) * 4) + 1 )
// <i> where
// <i> - subevents value is constant 1 since one subevent per procedure is supported,
// <i> - mode0_size is
// <i>   - 4 for Reflector and
// <i>   - 6 for Initiator,
// <i> - mode0_steps value is the configuration "Mode 0 steps",
// <i> - channels value means the number of channels from the channel mask that can be
// <i> derived from the "Channel map preset" settings:
// <i>   - "High"   - 72 (default),
// <i>   - "Medium" - 37,
// <i>   - "Low"    - 20,
// <i>   - "Custom" - Number of 1s in channel mask,
// <i> - antenna_paths value is controlled by the "Antenna configuration", and limited by number of
// <i> antennas presented on each board (capabilities). Maximum can be calculated using the product
// <i> of used Initiator and Reflector antennae. The default maximum value for antenna_paths is 4.
// <i> These settings were selected by assuming that the controller creates only one subevent per procedure,
// <i> and the measuring mode is PBR. In RTT mode there are far less data is created.
// <i> The default is calculated by using the constants and settings above using the worst case scenario,
// <i> which gives 1614 bytes.
// <i> Addition to that, if you use RTT as submode, you should add the following equation to calculate the
// <i> size.
// <i> (1 + mode1_size) * channels / main_mode_steps
// <i> where
// <i> mode1_size is 6, and main_mode_steps is 2. The later can be changed in cs_initiator_client.h.
// <i> RAM consumption can be reduced by changing the affected settings and reducing
// <i> "Procedure maximum length" accordingly.
// <i> Default: 1866
#ifndef CS_INITIATOR_MAX_RANGING_DATA_SIZE
#define CS_INITIATOR_MAX_RANGING_DATA_SIZE            (1866)
#endif

// <o CS_INITIATOR_MAX_DROP> Maximum dropped procedures <2..10>
// <i> Maximum number of dropped procedures if current ranging data is not received.
// <i> Default: 10
#ifndef CS_INITIATOR_MAX_DROP
#define CS_INITIATOR_MAX_DROP                         10
#endif

// <o CS_INITIATOR_ERROR_TIMEOUT_MS> Error timeout [msec] <100..5000>
// <i> Timeout value in order to avoid stuck in error state indefinitely.
// <i> Once the time elapses the initiator instance's error callback executes to
// <i> inform the user about the issue.
// <i> Default: 3000
#define CS_INITIATOR_ERROR_TIMEOUT_MS                3000

// <o CS_INITIATOR_PROCEDURE_TIMEOUT_MS> CS procedure timeout [msec] <100..5000>
// <i> Timeout value for procedures - in order to avoid getting stuck in a procedure indefinitely.
// <i> Once the time elapses the initiator instance's error callback executes to
// <i> inform the user about the issue.
// <i> Default: 3000
#define CS_INITIATOR_PROCEDURE_TIMEOUT_MS            3000

// <o CS_INITIATOR_DEFAULT_CONNECTION_PERIPHERAL_LATENCY> Connection peripheral latency
// <i> Peripheral latency, which defines how many connection
// <i> intervals the peripheral can skip if it has no data to send
// <i> Default: 0
#define CS_INITIATOR_DEFAULT_CONNECTION_PERIPHERAL_LATENCY  0

// <o CS_INITIATOR_DEFAULT_TIMEOUT> Supervision timeout <10..3200>
// <i> Connection supervision timeout in the units of 10 ms (from 100 ms to 32 s)
// <i> Default: 500
#define CS_INITIATOR_DEFAULT_TIMEOUT 500

// <o CS_INITIATOR_DEFAULT_MIN_CE_LENGTH> Minimum length of the connection event <1..65535>
// <i> Value in units of 0.625 ms
// <i> Default: 0
#define CS_INITIATOR_DEFAULT_MIN_CE_LENGTH 0

// <o CS_INITIATOR_DEFAULT_MAX_CE_LENGTH> Maximum length of the connection event <1..65535>
// <i> Value in units of 0.625 ms
// <i> Default: 65535
#define CS_INITIATOR_DEFAULT_MAX_CE_LENGTH        65535

// </h>

// <h> Distance estimation

// <o CS_INITIATOR_DEFAULT_ALGO_MODE> Object tracking mode
// <SL_RTL_CS_ALGO_MODE_REAL_TIME_BASIC=> Real-time basic
// <SL_RTL_CS_ALGO_MODE_STATIC_HIGH_ACCURACY=> Static high accuracy
// <SL_RTL_CS_ALGO_MODE_REAL_TIME_FAST=> Real-time fast
// <i> Default: SL_RTL_CS_ALGO_MODE_REAL_TIME_FAST
#define CS_INITIATOR_DEFAULT_ALGO_MODE           SL_RTL_CS_ALGO_MODE_REAL_TIME_FAST

// </h>

// <h> Channel Sounding

// <h> Modes

// <o CS_INITIATOR_DEFAULT_CS_MAIN_MODE> Main mode
// <sl_bt_cs_mode_rtt=> RTT
// <sl_bt_cs_mode_pbr=> PBR
#define CS_INITIATOR_DEFAULT_CS_MAIN_MODE            sl_bt_cs_mode_pbr

// <o CS_INITIATOR_DEFAULT_CS_SUB_MODE> Submode
// <sl_bt_cs_mode_rtt=> RTT
// <sl_bt_cs_submode_disabled=> No submode
// <i> Default: sl_bt_cs_submode_disabled
#define CS_INITIATOR_DEFAULT_CS_SUB_MODE             sl_bt_cs_submode_disabled

// </h>

// <h> Scheduling

// <o CS_INITIATOR_DEFAULT_MAX_PROCEDURE_COUNT> Procedure execution
// <0=> Free-running
// <1=> Synchronized
// <i> Default: 0
#define CS_INITIATOR_DEFAULT_MAX_PROCEDURE_COUNT         0

// <o CS_INITIATOR_DEFAULT_PROCEDURE_SCHEDULING> Procedure scheduling
// <CS_PROCEDURE_SCHEDULING_OPTIMIZED_FOR_FREQUENCY=> Optimized for frequency update
// <CS_PROCEDURE_SCHEDULING_OPTIMIZED_FOR_ENERGY=> Optimized for energy consumption
// <CS_PROCEDURE_SCHEDULING_CUSTOM=> Use custom procedure scheduling
// <i> Set your custom connection and procedure intervals in the 'Custom Scheduling' section when selecting 'Use custom procedure scheduling' (CS_PROCEDURE_SCHEDULING_CUSTOM).
// <i> Default: CS_PROCEDURE_SCHEDULING_OPTIMIZED_FOR_FREQUENCY
#define CS_INITIATOR_DEFAULT_PROCEDURE_SCHEDULING CS_PROCEDURE_SCHEDULING_OPTIMIZED_FOR_FREQUENCY

// </h>

// <h> Custom scheduling

// <o CS_INITIATOR_DEFAULT_MIN_CONNECTION_INTERVAL> Minimum connection interval (in 1.25 ms steps) <6..3200>
// <i> Default: 6
#define CS_INITIATOR_DEFAULT_MIN_CONNECTION_INTERVAL         6

// <o CS_INITIATOR_DEFAULT_MAX_CONNECTION_INTERVAL> Maximum connection interval (in 1.25 ms steps) <6..3200>
// <i> Default: 6
#define CS_INITIATOR_DEFAULT_MAX_CONNECTION_INTERVAL         6

// <o CS_INITIATOR_DEFAULT_MIN_PROCEDURE_INTERVAL> Minimum delay between CS measurements [connection events] <1..255>
// <i> Default: 38
// <i> Minimum duration in number of connection events between consecutive CS measurement procedures
#ifndef CS_INITIATOR_DEFAULT_MIN_PROCEDURE_INTERVAL
#define CS_INITIATOR_DEFAULT_MIN_PROCEDURE_INTERVAL                     (38)
#endif

// <o CS_INITIATOR_DEFAULT_MAX_PROCEDURE_INTERVAL> Maximum delay between CS measurements [connection events] <1..255>
// <i> Default: 38
// <i> Maximum duration in number of connection events between consecutive CS measurement procedures
#ifndef CS_INITIATOR_DEFAULT_MAX_PROCEDURE_INTERVAL
#define CS_INITIATOR_DEFAULT_MAX_PROCEDURE_INTERVAL                     (38)
#endif

// </h>

// <h> Channels

// <o CS_INITIATOR_DEFAULT_CHANNEL_MAP_PRESET> Channel map preset
// <CS_CHANNEL_MAP_PRESET_LOW=> Low
// <CS_CHANNEL_MAP_PRESET_MEDIUM=> Medium
// <CS_CHANNEL_MAP_PRESET_HIGH=> High
// <CS_CHANNEL_MAP_PRESET_CUSTOM=> Custom
// <i> Default: CS_CHANNEL_MAP_PRESET_HIGH
#define CS_INITIATOR_DEFAULT_CHANNEL_MAP_PRESET           CS_CHANNEL_MAP_PRESET_HIGH

// <a.10 CS_CUSTOM_CHANNEL_MAP> Custom channel map <0..255> <f.h>
// <i> Default: { 0xFC, 0xFF, 0x7F, 0xFC, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x1F }
#define CS_CUSTOM_CHANNEL_MAP                        { 0xFC, 0xFF, 0x7F, 0xFC, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x1F }

// </h>

// <h> PHYs

// <o CS_INITIATOR_DEFAULT_CONN_PHY> Connection PHY
// <sl_bt_gap_phy_1m=> 1M
// <sl_bt_gap_phy_2m=> 2M
// <i> Default: 2M
#define CS_INITIATOR_DEFAULT_CONN_PHY         sl_bt_gap_phy_2m

// <o CS_INITIATOR_DEFAULT_CS_SYNC_PHY> CS sync PHY
// <sl_bt_gap_phy_1m=> 1M
// <sl_bt_gap_phy_2m=> 2M
// <i> Default: sl_bt_gap_phy_1m
#define CS_INITIATOR_DEFAULT_CS_SYNC_PHY       sl_bt_gap_phy_1m

// </h>

// <h> Antenna configuration

// <o CS_INITIATOR_DEFAULT_CS_TONE_ANTENNA_CONFIG_IDX_REQ> CS tone antenna configuration requested
// <i> Use all the available antennas that can be supported
// <i> Value: 0. 1 antenna path, [1:1] antenna [initiator:reflector]
// <i> Value: 1. 2 antenna path, [2:1] antenna
// <i> Value: 2. 3 antenna path, [3:1] antenna (not supported)
// <i> Value: 3. 4 antenna path, [4:1] antenna (not supported)
// <i> Value: 4. 2 antenna path, [1:2] antenna
// <i> Value: 5. 3 antenna path, [1:3] antenna (not supported)
// <i> Value: 6. 4 antenna path, [1:4] antenna (not supported)
// <i> Value: 7. 4 antenna path, [2:2] antenna
// <CS_ANTENNA_CONFIG_INDEX_SINGLE_ONLY=> 1 antenna path, [1:1] antenna
// <CS_ANTENNA_CONFIG_INDEX_DUAL_I_SINGLE_R=> 2 antenna path, [2:1] antenna
// <CS_ANTENNA_CONFIG_INDEX_SINGLE_I_DUAL_R=> 2 antenna path, [1:2] antenna
// <CS_ANTENNA_CONFIG_INDEX_DUAL_ONLY=> 4 antenna path, [2:2] antenna
// <i> Default: CS_ANTENNA_CONFIG_INDEX_DUAL_ONLY
#define CS_INITIATOR_DEFAULT_CS_TONE_ANTENNA_CONFIG_IDX_REQ   CS_ANTENNA_CONFIG_INDEX_DUAL_ONLY

// <o CS_INITIATOR_DEFAULT_CS_SYNC_ANTENNA_REQ> CS sync antenna configuration requested
// <i> Use all the available antennas that can be supported
// <CS_SYNC_ANTENNA_1=> Antenna 1
// <CS_SYNC_ANTENNA_2=> Antenna 2
// <CS_SYNC_SWITCHING=> Switching
// <i> Default: CS_SYNC_SWITCHING
#define CS_INITIATOR_DEFAULT_CS_SYNC_ANTENNA_REQ  CS_SYNC_SWITCHING

// <o CS_INITIATOR_DEFAULT_PREFERRED_PEER_ANTENNA> Preferred peer antenna
// <i> The preferred peer-ordered antenna elements to be used by the remote device
// <i> for the antenna configuration denoted by the tone antenna config selection.
// <i> Default: 3
#define CS_INITIATOR_DEFAULT_PREFERRED_PEER_ANTENNA  3

// <o CS_INITIATOR_ANTENNA_OFFSET> Specify antenna offset
// <0=> Wireless antenna offset
// <1=> Wired antenna offset
// <i> Default: 0
#ifndef CS_INITIATOR_ANTENNA_OFFSET
#define CS_INITIATOR_ANTENNA_OFFSET                  0
#endif

// </h>

// <h> TX power

// <o CS_INITIATOR_DEFAULT_MAX_TX_POWER> Maximum transmit power level to be used in all CS transmissions <-127..10>
// <i> Default: 10
#define CS_INITIATOR_DEFAULT_MAX_TX_POWER  10

// <o CS_INITIATOR_DEFAULT_RSSI_REF_TX_POWER> RSSI reference TX power <-110..30>
// <i> Reference RSSI value of the remote Reflector device at 1.0 m distance in dBm
// <i> Default: -40.0F
#define CS_INITIATOR_DEFAULT_RSSI_REF_TX_POWER  -40.0F

// </h>

// <h> Details

// <o CS_INITIATOR_DEFAULT_CREATE_CONTEXT> Create context
// <i> Write CS configuration in the local controller only
// <i> or both local and remote controller using a configuration procedure
// <0=> Local controller only
// <1=> Both local and remote
// <i> Default: 1
#define CS_INITIATOR_DEFAULT_CREATE_CONTEXT  1

// <o CS_INITIATOR_DEFAULT_MODE0_STEPS> Mode 0 steps
// <i> Number of Mode 0 steps to be included at the beginning of the test CS subevent
// <i> Default: 3
#define CS_INITIATOR_DEFAULT_MODE0_STEPS      3

// <o CS_INITIATOR_DEFAULT_RTT_TYPE> RTT Type
// <sl_bt_cs_rtt_type_aa_only=> RTT Access Address (AA) only
// <sl_bt_cs_rtt_type_fractional_32_bit_sounding=> RTT Fractional with 32-bit Sounding Sequence
// <sl_bt_cs_rtt_type_fractional_96_bit_sounding=> RTT Fractional with 96-bit Sounding Sequence
// <sl_bt_cs_rtt_type_fractional_32_bit_random=> RTT Fractional with 32-bit Random Sequence
// <sl_bt_cs_rtt_type_fractional_64_bit_random=> RTT Fractional with 64-bit Random Sequence
// <sl_bt_cs_rtt_type_fractional_96_bit_random=> RTT Fractional with 96-bit Random Sequence
// <sl_bt_cs_rtt_type_fractional_128_bit_random=> RTT Fractional with 128-bit Random Sequence
// <i> Default: sl_bt_cs_rtt_type_fractional_96_bit_sounding
#define CS_INITIATOR_DEFAULT_RTT_TYPE         sl_bt_cs_rtt_type_fractional_96_bit_sounding

// <o CS_INITIATOR_DEFAULT_CHANNEL_SELECTION_TYPE> Channel selection type
// <sl_bt_cs_channel_selection_algorithm_3b=> Algorithm 3b
// <sl_bt_cs_channel_selection_algorithm_3c=> Algorithm 3c
// <sl_bt_cs_channel_selection_algorithm_user_shape_interleaved=> Algorithm user shape interleaved
// <i> Default: sl_bt_cs_channel_selection_algorithm_3b
#define CS_INITIATOR_DEFAULT_CHANNEL_SELECTION_TYPE          sl_bt_cs_channel_selection_algorithm_3b

// <o CS_INITIATOR_DEFAULT_CH3C_SHAPE> Ch3c shape
// <sl_bt_cs_ch3c_shape_hat=> Ch3c shape hat
// <sl_bt_cs_chc3_shape_interleaved=> Ch3c shape interleaved
// <i> Default: sl_bt_cs_ch3c_shape_hat
#define CS_INITIATOR_DEFAULT_CH3C_SHAPE             sl_bt_cs_ch3c_shape_hat

// </h>

// </h>

// <h> Ranging Service

// <q CS_INITIATOR_RAS_MODE_USE_REAL_TIME_MODE> Use Real-Time RAS mode
// <i> Default: 1
#ifndef CS_INITIATOR_RAS_MODE_USE_REAL_TIME_MODE
#define CS_INITIATOR_RAS_MODE_USE_REAL_TIME_MODE                        1
#endif

// <q CS_INITIATOR_RAS_REAL_TIME_INDICATION> Use real-time ranging data indication
// <i> Default: 0
#ifndef CS_INITIATOR_RAS_REAL_TIME_INDICATION
#define CS_INITIATOR_RAS_REAL_TIME_INDICATION        (0)
#endif

// <q CS_INITIATOR_RAS_ON_DEMAND_INDICATION> Use on-demand ranging data indication
// <i> Default: 0
#ifndef CS_INITIATOR_RAS_ON_DEMAND_INDICATION
#define CS_INITIATOR_RAS_ON_DEMAND_INDICATION        (0)
#endif

// <q CS_INITIATOR_RAS_DATA_READY_NOTIFICATION> Use ranging data ready notification
// <i> This is only applicable for On-Demand Ranging Data mode
// <i> Default: 1
#ifndef CS_INITIATOR_RAS_DATA_READY_NOTIFICATION
#define CS_INITIATOR_RAS_DATA_READY_NOTIFICATION     (1)
#endif

// <q CS_INITIATOR_RAS_DATA_OVERWRITTEN_NOTIFICATION> Use ranging data overwritten notification
// <i> This is only applicable for On-Demand Ranging Data mode
// <i> Default: 1
#ifndef CS_INITIATOR_RAS_DATA_OVERWRITTEN_NOTIFICATION
#define CS_INITIATOR_RAS_DATA_OVERWRITTEN_NOTIFICATION     (1)
#endif

// </h>

// <<< end of configuration section >>>

// Ch3c jump <2..8>
// Number of channels skipped in each rising and falling sequence.
// Default: 2
#define CS_INITIATOR_DEFAULT_CH3C_JUMP   2

// Max procedure duration <1..65535>
// Maximum duration for each measurement procedure.
// Value in units of 0.625 ms
// Default: 65535
#define CS_INITIATOR_DEFAULT_MAX_PROCEDURE_DURATION   65535

// Transmit power delta, in signed dB.
// Default: 0
#define CS_INITIATOR_DEFAULT_TX_PWR_DELTA   0

// Config ID
// Only one channel is supported
// Default: 1
#define CS_INITIATOR_DEFAULT_CONFIG_ID      1

// Minimum subevent length. <1250..3999999>
// Minimum suggested duration for each CS subevent in microseconds.
// Default: 1250
#define CS_INITIATOR_DEFAULT_MIN_SUBEVENT_LEN          1250

// Maximum subevent length. <1250..3999999>
// Maximum suggested duration for each CS subevent in microseconds.
// Default: 3999999
#define CS_INITIATOR_DEFAULT_MAX_SUBEVENT_LEN          3999999

// Internal define to skip RTL processing
#define CS_INITIATOR_INTERNAL_SKIP_RTL_PROCESS         0

/** @} (end addtogroup cs_initiator) */
#endif // CS_INITIATOR_CONFIG_H
