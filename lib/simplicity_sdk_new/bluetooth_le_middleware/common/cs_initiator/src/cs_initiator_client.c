/***************************************************************************//**
 * @file
 * @brief CS initiator - client implementation
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
// Includes

#include <stdint.h>
#include <string.h>
#include "cs_initiator_config.h"
#include "cs_initiator_client.h"
#include "sl_rtl_clib_api.h"

// -----------------------------------------------------------------------------
// Defnitions
#define SUB_MODE_OFFSET_MS              20
#define STATIC_MODE_CONNECTION_INTERVAL 6
#define STATIC_MODE_PROCEDURE_INTERVAL  38

// -----------------------------------------------------------------------------
// Structs
SL_PACK_START(1)
typedef struct {
  struct cs_initiator_input_values_t {
    cs_procedure_scheduling_t procedure_scheduling;
    uint8_t channel_map_preset;
    uint8_t algo_mode;
    uint8_t antenna_path;
    uint8_t use_real_time_ras_mode;
  } input;
  struct cs_initiator_output_values_t {
    uint16_t conn_interval;
    uint16_t proc_interval;
  } output;
} SL_ATTRIBUTE_PACKED cs_initiator_values_t;
SL_PACK_END()

static const cs_initiator_values_t initiator_values_optimized[] = {
  // Frequency optimized values - RAS real-time - ALGO real-time-basic
  // HIGH CHANNEL MAP
#if defined(CS_INITIATOR_RAS_MODE_USE_REAL_TIME_MODE) && (CS_INITIATOR_RAS_MODE_USE_REAL_TIME_MODE == 1) || defined(SL_CATALOG_CS_INITIATOR_CLIENT_PRESENT)
  { .input = { .procedure_scheduling = CS_PROCEDURE_SCHEDULING_OPTIMIZED_FOR_FREQUENCY, .channel_map_preset = CS_CHANNEL_MAP_PRESET_HIGH, .algo_mode = SL_RTL_CS_ALGO_MODE_REAL_TIME_BASIC, .antenna_path = CS_ANTENNA_CONFIG_INDEX_DUAL_ONLY, .use_real_time_ras_mode = 1 },
    .output = { .conn_interval = 8u, .proc_interval = 26u } },
  { .input = { .procedure_scheduling = CS_PROCEDURE_SCHEDULING_OPTIMIZED_FOR_FREQUENCY, .channel_map_preset = CS_CHANNEL_MAP_PRESET_HIGH, .algo_mode = SL_RTL_CS_ALGO_MODE_REAL_TIME_BASIC, .antenna_path = CS_ANTENNA_CONFIG_INDEX_DUAL_I_SINGLE_R, .use_real_time_ras_mode = 1 },
    .output = { .conn_interval = 7u, .proc_interval = 17u } },
  { .input = { .procedure_scheduling = CS_PROCEDURE_SCHEDULING_OPTIMIZED_FOR_FREQUENCY, .channel_map_preset = CS_CHANNEL_MAP_PRESET_HIGH, .algo_mode = SL_RTL_CS_ALGO_MODE_REAL_TIME_BASIC, .antenna_path = CS_ANTENNA_CONFIG_INDEX_SINGLE_I_DUAL_R, .use_real_time_ras_mode = 1 },
    .output = { .conn_interval = 7u, .proc_interval = 17u } },
  { .input = { .procedure_scheduling = CS_PROCEDURE_SCHEDULING_OPTIMIZED_FOR_FREQUENCY, .channel_map_preset = CS_CHANNEL_MAP_PRESET_HIGH, .algo_mode = SL_RTL_CS_ALGO_MODE_REAL_TIME_BASIC, .antenna_path = CS_ANTENNA_CONFIG_INDEX_SINGLE_ONLY, .use_real_time_ras_mode = 1 },
    .output = { .conn_interval = 7u, .proc_interval = 12u } },
  // MEDIUM CHANNEL MAP
  { .input = { .procedure_scheduling = CS_PROCEDURE_SCHEDULING_OPTIMIZED_FOR_FREQUENCY, .channel_map_preset = CS_CHANNEL_MAP_PRESET_MEDIUM, .algo_mode = SL_RTL_CS_ALGO_MODE_REAL_TIME_BASIC, .antenna_path = CS_ANTENNA_CONFIG_INDEX_DUAL_ONLY, .use_real_time_ras_mode = 1 },
    .output = { .conn_interval = 7u, .proc_interval = 13u } },
  { .input = { .procedure_scheduling = CS_PROCEDURE_SCHEDULING_OPTIMIZED_FOR_FREQUENCY, .channel_map_preset = CS_CHANNEL_MAP_PRESET_MEDIUM, .algo_mode = SL_RTL_CS_ALGO_MODE_REAL_TIME_BASIC, .antenna_path = CS_ANTENNA_CONFIG_INDEX_DUAL_I_SINGLE_R, .use_real_time_ras_mode = 1 },
    .output = { .conn_interval = 6u, .proc_interval = 10u } },
  { .input = { .procedure_scheduling = CS_PROCEDURE_SCHEDULING_OPTIMIZED_FOR_FREQUENCY, .channel_map_preset = CS_CHANNEL_MAP_PRESET_MEDIUM, .algo_mode = SL_RTL_CS_ALGO_MODE_REAL_TIME_BASIC, .antenna_path = CS_ANTENNA_CONFIG_INDEX_SINGLE_I_DUAL_R, .use_real_time_ras_mode = 1 },
    .output = { .conn_interval = 6u, .proc_interval = 10u } },
  { .input = { .procedure_scheduling = CS_PROCEDURE_SCHEDULING_OPTIMIZED_FOR_FREQUENCY, .channel_map_preset = CS_CHANNEL_MAP_PRESET_MEDIUM, .algo_mode = SL_RTL_CS_ALGO_MODE_REAL_TIME_BASIC, .antenna_path = CS_ANTENNA_CONFIG_INDEX_SINGLE_ONLY, .use_real_time_ras_mode = 1 },
    .output = { .conn_interval = 6u, .proc_interval = 9u } },
  // LOW CHANNEL MAP
  { .input = { .procedure_scheduling = CS_PROCEDURE_SCHEDULING_OPTIMIZED_FOR_FREQUENCY, .channel_map_preset = CS_CHANNEL_MAP_PRESET_LOW, .algo_mode = SL_RTL_CS_ALGO_MODE_REAL_TIME_BASIC, .antenna_path = CS_ANTENNA_CONFIG_INDEX_DUAL_ONLY, .use_real_time_ras_mode = 1 },
    .output = { .conn_interval = 6u, .proc_interval = 9u } },
  { .input = { .procedure_scheduling = CS_PROCEDURE_SCHEDULING_OPTIMIZED_FOR_FREQUENCY, .channel_map_preset = CS_CHANNEL_MAP_PRESET_LOW, .algo_mode = SL_RTL_CS_ALGO_MODE_REAL_TIME_BASIC, .antenna_path = CS_ANTENNA_CONFIG_INDEX_DUAL_I_SINGLE_R, .use_real_time_ras_mode = 1 },
    .output = { .conn_interval = 6u, .proc_interval = 8u } },
  { .input = { .procedure_scheduling = CS_PROCEDURE_SCHEDULING_OPTIMIZED_FOR_FREQUENCY, .channel_map_preset = CS_CHANNEL_MAP_PRESET_LOW, .algo_mode = SL_RTL_CS_ALGO_MODE_REAL_TIME_BASIC, .antenna_path = CS_ANTENNA_CONFIG_INDEX_SINGLE_I_DUAL_R, .use_real_time_ras_mode = 1 },
    .output = { .conn_interval = 6u, .proc_interval = 8u } },
  { .input = { .procedure_scheduling = CS_PROCEDURE_SCHEDULING_OPTIMIZED_FOR_FREQUENCY, .channel_map_preset = CS_CHANNEL_MAP_PRESET_LOW, .algo_mode = SL_RTL_CS_ALGO_MODE_REAL_TIME_BASIC, .antenna_path = CS_ANTENNA_CONFIG_INDEX_SINGLE_ONLY, .use_real_time_ras_mode = 1 },
    .output = { .conn_interval = 6u, .proc_interval = 7u } },

  // Frequency optimized values - RAS real-time - ALGO real-time-fast
  // HIGH CHANNEL MAP
  { .input = { .procedure_scheduling = CS_PROCEDURE_SCHEDULING_OPTIMIZED_FOR_FREQUENCY, .channel_map_preset = CS_CHANNEL_MAP_PRESET_HIGH, .algo_mode = SL_RTL_CS_ALGO_MODE_REAL_TIME_FAST, .antenna_path = CS_ANTENNA_CONFIG_INDEX_DUAL_ONLY, .use_real_time_ras_mode = 1 },
    .output = { .conn_interval = 6u, .proc_interval = 15u } },
  { .input = { .procedure_scheduling = CS_PROCEDURE_SCHEDULING_OPTIMIZED_FOR_FREQUENCY, .channel_map_preset = CS_CHANNEL_MAP_PRESET_HIGH, .algo_mode = SL_RTL_CS_ALGO_MODE_REAL_TIME_FAST, .antenna_path = CS_ANTENNA_CONFIG_INDEX_DUAL_I_SINGLE_R, .use_real_time_ras_mode = 1 },
    .output = { .conn_interval = 6u, .proc_interval = 12u } },
  { .input = { .procedure_scheduling = CS_PROCEDURE_SCHEDULING_OPTIMIZED_FOR_FREQUENCY, .channel_map_preset = CS_CHANNEL_MAP_PRESET_HIGH, .algo_mode = SL_RTL_CS_ALGO_MODE_REAL_TIME_FAST, .antenna_path = CS_ANTENNA_CONFIG_INDEX_SINGLE_I_DUAL_R, .use_real_time_ras_mode = 1 },
    .output = { .conn_interval = 6u, .proc_interval = 12u } },
  { .input = { .procedure_scheduling = CS_PROCEDURE_SCHEDULING_OPTIMIZED_FOR_FREQUENCY, .channel_map_preset = CS_CHANNEL_MAP_PRESET_HIGH, .algo_mode = SL_RTL_CS_ALGO_MODE_REAL_TIME_FAST, .antenna_path = CS_ANTENNA_CONFIG_INDEX_SINGLE_ONLY, .use_real_time_ras_mode = 1 },
    .output = { .conn_interval = 6u, .proc_interval = 10u } },
  // MEDIUM CHANNEL MAP
  { .input = { .procedure_scheduling = CS_PROCEDURE_SCHEDULING_OPTIMIZED_FOR_FREQUENCY, .channel_map_preset = CS_CHANNEL_MAP_PRESET_MEDIUM, .algo_mode = SL_RTL_CS_ALGO_MODE_REAL_TIME_FAST, .antenna_path = CS_ANTENNA_CONFIG_INDEX_DUAL_ONLY, .use_real_time_ras_mode = 1 },
    .output = { .conn_interval = 6u, .proc_interval = 9u } },
  { .input = { .procedure_scheduling = CS_PROCEDURE_SCHEDULING_OPTIMIZED_FOR_FREQUENCY, .channel_map_preset = CS_CHANNEL_MAP_PRESET_MEDIUM, .algo_mode = SL_RTL_CS_ALGO_MODE_REAL_TIME_FAST, .antenna_path = CS_ANTENNA_CONFIG_INDEX_DUAL_I_SINGLE_R, .use_real_time_ras_mode = 1 },
    .output = { .conn_interval = 6u, .proc_interval = 8u } },
  { .input = { .procedure_scheduling = CS_PROCEDURE_SCHEDULING_OPTIMIZED_FOR_FREQUENCY, .channel_map_preset = CS_CHANNEL_MAP_PRESET_MEDIUM, .algo_mode = SL_RTL_CS_ALGO_MODE_REAL_TIME_FAST, .antenna_path = CS_ANTENNA_CONFIG_INDEX_SINGLE_I_DUAL_R, .use_real_time_ras_mode = 1 },
    .output = { .conn_interval = 6u, .proc_interval = 8u } },
  { .input = { .procedure_scheduling = CS_PROCEDURE_SCHEDULING_OPTIMIZED_FOR_FREQUENCY, .channel_map_preset = CS_CHANNEL_MAP_PRESET_MEDIUM, .algo_mode = SL_RTL_CS_ALGO_MODE_REAL_TIME_FAST, .antenna_path = CS_ANTENNA_CONFIG_INDEX_SINGLE_ONLY, .use_real_time_ras_mode = 1 },
    .output = { .conn_interval = 6u, .proc_interval = 6u } },
  // LOW CHANNEL MAP
  { .input = { .procedure_scheduling = CS_PROCEDURE_SCHEDULING_OPTIMIZED_FOR_FREQUENCY, .channel_map_preset = CS_CHANNEL_MAP_PRESET_LOW, .algo_mode = SL_RTL_CS_ALGO_MODE_REAL_TIME_FAST, .antenna_path = CS_ANTENNA_CONFIG_INDEX_DUAL_ONLY, .use_real_time_ras_mode = 1 },
    .output = { .conn_interval = 6u, .proc_interval = 6u } },
  { .input = { .procedure_scheduling = CS_PROCEDURE_SCHEDULING_OPTIMIZED_FOR_FREQUENCY, .channel_map_preset = CS_CHANNEL_MAP_PRESET_LOW, .algo_mode = SL_RTL_CS_ALGO_MODE_REAL_TIME_FAST, .antenna_path = CS_ANTENNA_CONFIG_INDEX_DUAL_I_SINGLE_R, .use_real_time_ras_mode = 1 },
    .output = { .conn_interval = 6u, .proc_interval = 5u } },
  { .input = { .procedure_scheduling = CS_PROCEDURE_SCHEDULING_OPTIMIZED_FOR_FREQUENCY, .channel_map_preset = CS_CHANNEL_MAP_PRESET_LOW, .algo_mode = SL_RTL_CS_ALGO_MODE_REAL_TIME_FAST, .antenna_path = CS_ANTENNA_CONFIG_INDEX_SINGLE_I_DUAL_R, .use_real_time_ras_mode = 1 },
    .output = { .conn_interval = 6u, .proc_interval = 5u } },
  { .input = { .procedure_scheduling = CS_PROCEDURE_SCHEDULING_OPTIMIZED_FOR_FREQUENCY, .channel_map_preset = CS_CHANNEL_MAP_PRESET_LOW, .algo_mode = SL_RTL_CS_ALGO_MODE_REAL_TIME_FAST, .antenna_path = CS_ANTENNA_CONFIG_INDEX_SINGLE_ONLY, .use_real_time_ras_mode = 1 },
    .output = { .conn_interval = 6u, .proc_interval = 4u } },

  // Energy optimized values - RAS real-time
  // HIGH CHANNEL MAP
  { .input = { .procedure_scheduling = CS_PROCEDURE_SCHEDULING_OPTIMIZED_FOR_ENERGY, .channel_map_preset = CS_CHANNEL_MAP_PRESET_HIGH, .algo_mode = SL_RTL_CS_ALGO_MODE_REAL_TIME_BASIC, .antenna_path = CS_ANTENNA_CONFIG_INDEX_DUAL_ONLY, .use_real_time_ras_mode = 1 },
    .output = { .conn_interval = 16u, .proc_interval = 13u } },
  { .input = { .procedure_scheduling = CS_PROCEDURE_SCHEDULING_OPTIMIZED_FOR_ENERGY, .channel_map_preset = CS_CHANNEL_MAP_PRESET_HIGH, .algo_mode = SL_RTL_CS_ALGO_MODE_REAL_TIME_BASIC, .antenna_path = CS_ANTENNA_CONFIG_INDEX_DUAL_I_SINGLE_R, .use_real_time_ras_mode = 1 },
    .output = { .conn_interval = 16u, .proc_interval = 9u } },
  { .input = { .procedure_scheduling = CS_PROCEDURE_SCHEDULING_OPTIMIZED_FOR_ENERGY, .channel_map_preset = CS_CHANNEL_MAP_PRESET_HIGH, .algo_mode = SL_RTL_CS_ALGO_MODE_REAL_TIME_BASIC, .antenna_path = CS_ANTENNA_CONFIG_INDEX_SINGLE_I_DUAL_R, .use_real_time_ras_mode = 1 },
    .output = { .conn_interval = 16u, .proc_interval = 9u } },
  { .input = { .procedure_scheduling = CS_PROCEDURE_SCHEDULING_OPTIMIZED_FOR_ENERGY, .channel_map_preset = CS_CHANNEL_MAP_PRESET_HIGH, .algo_mode = SL_RTL_CS_ALGO_MODE_REAL_TIME_BASIC, .antenna_path = CS_ANTENNA_CONFIG_INDEX_SINGLE_ONLY, .use_real_time_ras_mode = 1 },
    .output = { .conn_interval = 16u, .proc_interval = 8u } },
  // MEDIUM CHANNEL MAP
  { .input = { .procedure_scheduling = CS_PROCEDURE_SCHEDULING_OPTIMIZED_FOR_ENERGY, .channel_map_preset = CS_CHANNEL_MAP_PRESET_MEDIUM, .algo_mode = SL_RTL_CS_ALGO_MODE_REAL_TIME_BASIC, .antenna_path = CS_ANTENNA_CONFIG_INDEX_DUAL_ONLY, .use_real_time_ras_mode = 1 },
    .output = { .conn_interval = 15u, .proc_interval = 7u } },
  { .input = { .procedure_scheduling = CS_PROCEDURE_SCHEDULING_OPTIMIZED_FOR_ENERGY, .channel_map_preset = CS_CHANNEL_MAP_PRESET_MEDIUM, .algo_mode = SL_RTL_CS_ALGO_MODE_REAL_TIME_BASIC, .antenna_path = CS_ANTENNA_CONFIG_INDEX_DUAL_I_SINGLE_R, .use_real_time_ras_mode = 1 },
    .output = { .conn_interval = 13u, .proc_interval = 6u } },
  { .input = { .procedure_scheduling = CS_PROCEDURE_SCHEDULING_OPTIMIZED_FOR_ENERGY, .channel_map_preset = CS_CHANNEL_MAP_PRESET_MEDIUM, .algo_mode = SL_RTL_CS_ALGO_MODE_REAL_TIME_BASIC, .antenna_path = CS_ANTENNA_CONFIG_INDEX_SINGLE_I_DUAL_R, .use_real_time_ras_mode = 1 },
    .output = { .conn_interval = 13u, .proc_interval = 6u } },
  { .input = { .procedure_scheduling = CS_PROCEDURE_SCHEDULING_OPTIMIZED_FOR_ENERGY, .channel_map_preset = CS_CHANNEL_MAP_PRESET_MEDIUM, .algo_mode = SL_RTL_CS_ALGO_MODE_REAL_TIME_BASIC, .antenna_path = CS_ANTENNA_CONFIG_INDEX_SINGLE_ONLY, .use_real_time_ras_mode = 1 },
    .output = { .conn_interval = 12u, .proc_interval = 6u } },
  // LOW CHANNEL MAP
  { .input = { .procedure_scheduling = CS_PROCEDURE_SCHEDULING_OPTIMIZED_FOR_ENERGY, .channel_map_preset = CS_CHANNEL_MAP_PRESET_LOW, .algo_mode = SL_RTL_CS_ALGO_MODE_REAL_TIME_BASIC, .antenna_path = CS_ANTENNA_CONFIG_INDEX_DUAL_ONLY, .use_real_time_ras_mode = 1 },
    .output = { .conn_interval = 10u, .proc_interval = 7u } },
  { .input = { .procedure_scheduling = CS_PROCEDURE_SCHEDULING_OPTIMIZED_FOR_ENERGY, .channel_map_preset = CS_CHANNEL_MAP_PRESET_LOW, .algo_mode = SL_RTL_CS_ALGO_MODE_REAL_TIME_BASIC, .antenna_path = CS_ANTENNA_CONFIG_INDEX_DUAL_I_SINGLE_R, .use_real_time_ras_mode = 1 },
    .output = { .conn_interval = 9u, .proc_interval = 6u } },
  { .input = { .procedure_scheduling = CS_PROCEDURE_SCHEDULING_OPTIMIZED_FOR_ENERGY, .channel_map_preset = CS_CHANNEL_MAP_PRESET_LOW, .algo_mode = SL_RTL_CS_ALGO_MODE_REAL_TIME_BASIC, .antenna_path = CS_ANTENNA_CONFIG_INDEX_SINGLE_I_DUAL_R, .use_real_time_ras_mode = 1 },
    .output = { .conn_interval = 9u, .proc_interval = 6u } },
  { .input = { .procedure_scheduling = CS_PROCEDURE_SCHEDULING_OPTIMIZED_FOR_ENERGY, .channel_map_preset = CS_CHANNEL_MAP_PRESET_LOW, .algo_mode = SL_RTL_CS_ALGO_MODE_REAL_TIME_BASIC, .antenna_path = CS_ANTENNA_CONFIG_INDEX_SINGLE_ONLY, .use_real_time_ras_mode = 1 },
    .output = { .conn_interval = 8u, .proc_interval = 6u } },

#endif

// Frequency optimized values - RAS on-demand - ALGO real-time-basic
// HIGH CHANNEL MAP
#if defined(CS_INITIATOR_RAS_MODE_USE_REAL_TIME_MODE) && (CS_INITIATOR_RAS_MODE_USE_REAL_TIME_MODE == 0) || defined(SL_CATALOG_CS_INITIATOR_CLIENT_PRESENT)
  { .input = { .procedure_scheduling = CS_PROCEDURE_SCHEDULING_OPTIMIZED_FOR_FREQUENCY, .channel_map_preset = CS_CHANNEL_MAP_PRESET_HIGH, .algo_mode = SL_RTL_CS_ALGO_MODE_REAL_TIME_BASIC, .antenna_path = CS_ANTENNA_CONFIG_INDEX_DUAL_ONLY, .use_real_time_ras_mode = 0 },
    .output = { .conn_interval = 9u, .proc_interval = 34u } },
  { .input = { .procedure_scheduling = CS_PROCEDURE_SCHEDULING_OPTIMIZED_FOR_FREQUENCY, .channel_map_preset = CS_CHANNEL_MAP_PRESET_HIGH, .algo_mode = SL_RTL_CS_ALGO_MODE_REAL_TIME_BASIC, .antenna_path = CS_ANTENNA_CONFIG_INDEX_DUAL_I_SINGLE_R, .use_real_time_ras_mode = 0 },
    .output = { .conn_interval = 8u, .proc_interval = 25u } },
  { .input = { .procedure_scheduling = CS_PROCEDURE_SCHEDULING_OPTIMIZED_FOR_FREQUENCY, .channel_map_preset = CS_CHANNEL_MAP_PRESET_HIGH, .algo_mode = SL_RTL_CS_ALGO_MODE_REAL_TIME_BASIC, .antenna_path = CS_ANTENNA_CONFIG_INDEX_SINGLE_I_DUAL_R, .use_real_time_ras_mode = 0 },
    .output = { .conn_interval = 8u, .proc_interval = 25u } },
  { .input = { .procedure_scheduling = CS_PROCEDURE_SCHEDULING_OPTIMIZED_FOR_FREQUENCY, .channel_map_preset = CS_CHANNEL_MAP_PRESET_HIGH, .algo_mode = SL_RTL_CS_ALGO_MODE_REAL_TIME_BASIC, .antenna_path = CS_ANTENNA_CONFIG_INDEX_SINGLE_ONLY, .use_real_time_ras_mode = 0 },
    .output = { .conn_interval = 8u, .proc_interval = 22u } },
  // MEDIUM CHANNEL MAP
  { .input = { .procedure_scheduling = CS_PROCEDURE_SCHEDULING_OPTIMIZED_FOR_FREQUENCY, .channel_map_preset = CS_CHANNEL_MAP_PRESET_MEDIUM, .algo_mode = SL_RTL_CS_ALGO_MODE_REAL_TIME_BASIC, .antenna_path = CS_ANTENNA_CONFIG_INDEX_DUAL_ONLY, .use_real_time_ras_mode = 0 },
    .output = { .conn_interval = 9u, .proc_interval = 18u } },
  { .input = { .procedure_scheduling = CS_PROCEDURE_SCHEDULING_OPTIMIZED_FOR_FREQUENCY, .channel_map_preset = CS_CHANNEL_MAP_PRESET_MEDIUM, .algo_mode = SL_RTL_CS_ALGO_MODE_REAL_TIME_BASIC, .antenna_path = CS_ANTENNA_CONFIG_INDEX_DUAL_I_SINGLE_R, .use_real_time_ras_mode = 0 },
    .output = { .conn_interval = 8u, .proc_interval = 15u } },
  { .input = { .procedure_scheduling = CS_PROCEDURE_SCHEDULING_OPTIMIZED_FOR_FREQUENCY, .channel_map_preset = CS_CHANNEL_MAP_PRESET_MEDIUM, .algo_mode = SL_RTL_CS_ALGO_MODE_REAL_TIME_BASIC, .antenna_path = CS_ANTENNA_CONFIG_INDEX_SINGLE_I_DUAL_R, .use_real_time_ras_mode = 0 },
    .output = { .conn_interval = 8u, .proc_interval = 15u } },
  { .input = { .procedure_scheduling = CS_PROCEDURE_SCHEDULING_OPTIMIZED_FOR_FREQUENCY, .channel_map_preset = CS_CHANNEL_MAP_PRESET_MEDIUM, .algo_mode = SL_RTL_CS_ALGO_MODE_REAL_TIME_BASIC, .antenna_path = CS_ANTENNA_CONFIG_INDEX_SINGLE_ONLY, .use_real_time_ras_mode = 0 },
    .output = { .conn_interval = 7u, .proc_interval = 13u } },
  // LOW CHANNEL MAP
  { .input = { .procedure_scheduling = CS_PROCEDURE_SCHEDULING_OPTIMIZED_FOR_FREQUENCY, .channel_map_preset = CS_CHANNEL_MAP_PRESET_LOW, .algo_mode = SL_RTL_CS_ALGO_MODE_REAL_TIME_BASIC, .antenna_path = CS_ANTENNA_CONFIG_INDEX_DUAL_ONLY, .use_real_time_ras_mode = 0 },
    .output = { .conn_interval = 8u, .proc_interval = 11u } },
  { .input = { .procedure_scheduling = CS_PROCEDURE_SCHEDULING_OPTIMIZED_FOR_FREQUENCY, .channel_map_preset = CS_CHANNEL_MAP_PRESET_LOW, .algo_mode = SL_RTL_CS_ALGO_MODE_REAL_TIME_BASIC, .antenna_path = CS_ANTENNA_CONFIG_INDEX_DUAL_I_SINGLE_R, .use_real_time_ras_mode = 0 },
    .output = { .conn_interval = 7u, .proc_interval = 12u } },
  { .input = { .procedure_scheduling = CS_PROCEDURE_SCHEDULING_OPTIMIZED_FOR_FREQUENCY, .channel_map_preset = CS_CHANNEL_MAP_PRESET_LOW, .algo_mode = SL_RTL_CS_ALGO_MODE_REAL_TIME_BASIC, .antenna_path = CS_ANTENNA_CONFIG_INDEX_SINGLE_I_DUAL_R, .use_real_time_ras_mode = 0 },
    .output = { .conn_interval = 7u, .proc_interval = 12u } },
  { .input = { .procedure_scheduling = CS_PROCEDURE_SCHEDULING_OPTIMIZED_FOR_FREQUENCY, .channel_map_preset = CS_CHANNEL_MAP_PRESET_LOW, .algo_mode = SL_RTL_CS_ALGO_MODE_REAL_TIME_BASIC, .antenna_path = CS_ANTENNA_CONFIG_INDEX_SINGLE_ONLY, .use_real_time_ras_mode = 0 },
    .output = { .conn_interval = 7u, .proc_interval = 10u } },

  // Frequency optimized values - RAS on-demand - ALGO real-time-fast
  // HIGH CHANNEL MAP
  { .input = { .procedure_scheduling = CS_PROCEDURE_SCHEDULING_OPTIMIZED_FOR_FREQUENCY, .channel_map_preset = CS_CHANNEL_MAP_PRESET_HIGH, .algo_mode = SL_RTL_CS_ALGO_MODE_REAL_TIME_FAST, .antenna_path = CS_ANTENNA_CONFIG_INDEX_DUAL_ONLY, .use_real_time_ras_mode = 0 },
    .output = { .conn_interval = 8u, .proc_interval = 20u } },
  { .input = { .procedure_scheduling = CS_PROCEDURE_SCHEDULING_OPTIMIZED_FOR_FREQUENCY, .channel_map_preset = CS_CHANNEL_MAP_PRESET_HIGH, .algo_mode = SL_RTL_CS_ALGO_MODE_REAL_TIME_FAST, .antenna_path = CS_ANTENNA_CONFIG_INDEX_DUAL_I_SINGLE_R, .use_real_time_ras_mode = 0 },
    .output = { .conn_interval = 8u, .proc_interval = 18u } },
  { .input = { .procedure_scheduling = CS_PROCEDURE_SCHEDULING_OPTIMIZED_FOR_FREQUENCY, .channel_map_preset = CS_CHANNEL_MAP_PRESET_HIGH, .algo_mode = SL_RTL_CS_ALGO_MODE_REAL_TIME_FAST, .antenna_path = CS_ANTENNA_CONFIG_INDEX_SINGLE_I_DUAL_R, .use_real_time_ras_mode = 0 },
    .output = { .conn_interval = 8u, .proc_interval = 18u } },
  { .input = { .procedure_scheduling = CS_PROCEDURE_SCHEDULING_OPTIMIZED_FOR_FREQUENCY, .channel_map_preset = CS_CHANNEL_MAP_PRESET_HIGH, .algo_mode = SL_RTL_CS_ALGO_MODE_REAL_TIME_FAST, .antenna_path = CS_ANTENNA_CONFIG_INDEX_SINGLE_ONLY, .use_real_time_ras_mode = 0 },
    .output = { .conn_interval = 7u, .proc_interval = 16u } },
  // MEDIUM CHANNEL MAP
  { .input = { .procedure_scheduling = CS_PROCEDURE_SCHEDULING_OPTIMIZED_FOR_FREQUENCY, .channel_map_preset = CS_CHANNEL_MAP_PRESET_MEDIUM, .algo_mode = SL_RTL_CS_ALGO_MODE_REAL_TIME_FAST, .antenna_path = CS_ANTENNA_CONFIG_INDEX_DUAL_ONLY, .use_real_time_ras_mode = 0 },
    .output = { .conn_interval = 7u, .proc_interval = 14u } },
  { .input = { .procedure_scheduling = CS_PROCEDURE_SCHEDULING_OPTIMIZED_FOR_FREQUENCY, .channel_map_preset = CS_CHANNEL_MAP_PRESET_MEDIUM, .algo_mode = SL_RTL_CS_ALGO_MODE_REAL_TIME_FAST, .antenna_path = CS_ANTENNA_CONFIG_INDEX_DUAL_I_SINGLE_R, .use_real_time_ras_mode = 0 },
    .output = { .conn_interval = 6u, .proc_interval = 13u } },
  { .input = { .procedure_scheduling = CS_PROCEDURE_SCHEDULING_OPTIMIZED_FOR_FREQUENCY, .channel_map_preset = CS_CHANNEL_MAP_PRESET_MEDIUM, .algo_mode = SL_RTL_CS_ALGO_MODE_REAL_TIME_FAST, .antenna_path = CS_ANTENNA_CONFIG_INDEX_SINGLE_I_DUAL_R, .use_real_time_ras_mode = 0 },
    .output = { .conn_interval = 6u, .proc_interval = 135u } },
  { .input = { .procedure_scheduling = CS_PROCEDURE_SCHEDULING_OPTIMIZED_FOR_FREQUENCY, .channel_map_preset = CS_CHANNEL_MAP_PRESET_MEDIUM, .algo_mode = SL_RTL_CS_ALGO_MODE_REAL_TIME_FAST, .antenna_path = CS_ANTENNA_CONFIG_INDEX_SINGLE_ONLY, .use_real_time_ras_mode = 0 },
    .output = { .conn_interval = 6u, .proc_interval = 12u } },
  // LOW CHANNEL MAP
  { .input = { .procedure_scheduling = CS_PROCEDURE_SCHEDULING_OPTIMIZED_FOR_FREQUENCY, .channel_map_preset = CS_CHANNEL_MAP_PRESET_LOW, .algo_mode = SL_RTL_CS_ALGO_MODE_REAL_TIME_FAST, .antenna_path = CS_ANTENNA_CONFIG_INDEX_DUAL_ONLY, .use_real_time_ras_mode = 0 },
    .output = { .conn_interval = 6u, .proc_interval = 12u } },
  { .input = { .procedure_scheduling = CS_PROCEDURE_SCHEDULING_OPTIMIZED_FOR_FREQUENCY, .channel_map_preset = CS_CHANNEL_MAP_PRESET_LOW, .algo_mode = SL_RTL_CS_ALGO_MODE_REAL_TIME_FAST, .antenna_path = CS_ANTENNA_CONFIG_INDEX_DUAL_I_SINGLE_R, .use_real_time_ras_mode = 0 },
    .output = { .conn_interval = 6u, .proc_interval = 11u } },
  { .input = { .procedure_scheduling = CS_PROCEDURE_SCHEDULING_OPTIMIZED_FOR_FREQUENCY, .channel_map_preset = CS_CHANNEL_MAP_PRESET_LOW, .algo_mode = SL_RTL_CS_ALGO_MODE_REAL_TIME_FAST, .antenna_path = CS_ANTENNA_CONFIG_INDEX_SINGLE_I_DUAL_R, .use_real_time_ras_mode = 0 },
    .output = { .conn_interval = 6u, .proc_interval = 11u } },
  { .input = { .procedure_scheduling = CS_PROCEDURE_SCHEDULING_OPTIMIZED_FOR_FREQUENCY, .channel_map_preset = CS_CHANNEL_MAP_PRESET_LOW, .algo_mode = SL_RTL_CS_ALGO_MODE_REAL_TIME_FAST, .antenna_path = CS_ANTENNA_CONFIG_INDEX_SINGLE_ONLY, .use_real_time_ras_mode = 0 },
    .output = { .conn_interval = 6u, .proc_interval = 10u } },

  // Energy optimized values - RAS on-demand
  // HIGH CHANNEL MAP
  { .input = { .procedure_scheduling = CS_PROCEDURE_SCHEDULING_OPTIMIZED_FOR_ENERGY, .channel_map_preset = CS_CHANNEL_MAP_PRESET_HIGH, .algo_mode = SL_RTL_CS_ALGO_MODE_REAL_TIME_BASIC, .antenna_path = CS_ANTENNA_CONFIG_INDEX_DUAL_ONLY, .use_real_time_ras_mode = 0 },
    .output = { .conn_interval = 18u, .proc_interval = 18u } },
  { .input = { .procedure_scheduling = CS_PROCEDURE_SCHEDULING_OPTIMIZED_FOR_ENERGY, .channel_map_preset = CS_CHANNEL_MAP_PRESET_HIGH, .algo_mode = SL_RTL_CS_ALGO_MODE_REAL_TIME_BASIC, .antenna_path = CS_ANTENNA_CONFIG_INDEX_DUAL_I_SINGLE_R, .use_real_time_ras_mode = 0 },
    .output = { .conn_interval = 17u, .proc_interval = 16u } },
  { .input = { .procedure_scheduling = CS_PROCEDURE_SCHEDULING_OPTIMIZED_FOR_ENERGY, .channel_map_preset = CS_CHANNEL_MAP_PRESET_HIGH, .algo_mode = SL_RTL_CS_ALGO_MODE_REAL_TIME_BASIC, .antenna_path = CS_ANTENNA_CONFIG_INDEX_SINGLE_I_DUAL_R, .use_real_time_ras_mode = 0 },
    .output = { .conn_interval = 17u, .proc_interval = 16u } },
  { .input = { .procedure_scheduling = CS_PROCEDURE_SCHEDULING_OPTIMIZED_FOR_ENERGY, .channel_map_preset = CS_CHANNEL_MAP_PRESET_HIGH, .algo_mode = SL_RTL_CS_ALGO_MODE_REAL_TIME_BASIC, .antenna_path = CS_ANTENNA_CONFIG_INDEX_SINGLE_ONLY, .use_real_time_ras_mode = 0 },
    .output = { .conn_interval = 17u, .proc_interval = 14u } },
  // MEDIUM CHANNEL MAP
  { .input = { .procedure_scheduling = CS_PROCEDURE_SCHEDULING_OPTIMIZED_FOR_ENERGY, .channel_map_preset = CS_CHANNEL_MAP_PRESET_MEDIUM, .algo_mode = SL_RTL_CS_ALGO_MODE_REAL_TIME_BASIC, .antenna_path = CS_ANTENNA_CONFIG_INDEX_DUAL_ONLY, .use_real_time_ras_mode = 0 },
    .output = { .conn_interval = 17u, .proc_interval = 12u } },
  { .input = { .procedure_scheduling = CS_PROCEDURE_SCHEDULING_OPTIMIZED_FOR_ENERGY, .channel_map_preset = CS_CHANNEL_MAP_PRESET_MEDIUM, .algo_mode = SL_RTL_CS_ALGO_MODE_REAL_TIME_BASIC, .antenna_path = CS_ANTENNA_CONFIG_INDEX_DUAL_I_SINGLE_R, .use_real_time_ras_mode = 0 },
    .output = { .conn_interval = 17u, .proc_interval = 10u } },
  { .input = { .procedure_scheduling = CS_PROCEDURE_SCHEDULING_OPTIMIZED_FOR_ENERGY, .channel_map_preset = CS_CHANNEL_MAP_PRESET_MEDIUM, .algo_mode = SL_RTL_CS_ALGO_MODE_REAL_TIME_BASIC, .antenna_path = CS_ANTENNA_CONFIG_INDEX_SINGLE_I_DUAL_R, .use_real_time_ras_mode = 0 },
    .output = { .conn_interval = 17u, .proc_interval = 10u } },
  { .input = { .procedure_scheduling = CS_PROCEDURE_SCHEDULING_OPTIMIZED_FOR_ENERGY, .channel_map_preset = CS_CHANNEL_MAP_PRESET_MEDIUM, .algo_mode = SL_RTL_CS_ALGO_MODE_REAL_TIME_BASIC, .antenna_path = CS_ANTENNA_CONFIG_INDEX_SINGLE_ONLY, .use_real_time_ras_mode = 0 },
    .output = { .conn_interval = 16u, .proc_interval = 10u } },
  // LOW CHANNEL MAP
  { .input = { .procedure_scheduling = CS_PROCEDURE_SCHEDULING_OPTIMIZED_FOR_ENERGY, .channel_map_preset = CS_CHANNEL_MAP_PRESET_LOW, .algo_mode = SL_RTL_CS_ALGO_MODE_REAL_TIME_BASIC, .antenna_path = CS_ANTENNA_CONFIG_INDEX_DUAL_ONLY, .use_real_time_ras_mode = 0 },
    .output = { .conn_interval = 16u, .proc_interval = 10u } },
  { .input = { .procedure_scheduling = CS_PROCEDURE_SCHEDULING_OPTIMIZED_FOR_ENERGY, .channel_map_preset = CS_CHANNEL_MAP_PRESET_LOW, .algo_mode = SL_RTL_CS_ALGO_MODE_REAL_TIME_BASIC, .antenna_path = CS_ANTENNA_CONFIG_INDEX_DUAL_I_SINGLE_R, .use_real_time_ras_mode = 0 },
    .output = { .conn_interval = 16u, .proc_interval = 9u } },
  { .input = { .procedure_scheduling = CS_PROCEDURE_SCHEDULING_OPTIMIZED_FOR_ENERGY, .channel_map_preset = CS_CHANNEL_MAP_PRESET_LOW, .algo_mode = SL_RTL_CS_ALGO_MODE_REAL_TIME_BASIC, .antenna_path = CS_ANTENNA_CONFIG_INDEX_SINGLE_I_DUAL_R, .use_real_time_ras_mode = 0 },
    .output = { .conn_interval = 16u, .proc_interval = 9u } },
  { .input = { .procedure_scheduling = CS_PROCEDURE_SCHEDULING_OPTIMIZED_FOR_ENERGY, .channel_map_preset = CS_CHANNEL_MAP_PRESET_LOW, .algo_mode = SL_RTL_CS_ALGO_MODE_REAL_TIME_BASIC, .antenna_path = CS_ANTENNA_CONFIG_INDEX_SINGLE_ONLY, .use_real_time_ras_mode = 0 },
    .output = { .conn_interval = 16u, .proc_interval = 8u } }
#endif
};

// -----------------------------------------------------------------------------
// Public function definitions

/******************************************************************************
 * Set channel map based on the preset.
 *****************************************************************************/
void cs_initiator_apply_channel_map_preset(cs_channel_map_preset_t preset, uint8_t *channel_map)
{
  switch (preset) {
    case CS_CHANNEL_MAP_PRESET_LOW:
    {
      uint8_t channel_map_low[10] = { 0x00, 0x00, 0x00, 0xC0, 0xFF, 0xFF, 0x03, 0x00, 0x00, 0x00 };
      memcpy(channel_map, channel_map_low, sizeof(channel_map_low));
    }
    break;
    case CS_CHANNEL_MAP_PRESET_MEDIUM:
    {
      uint8_t channel_map_medium[10] = { 0x54, 0x55, 0x55, 0x54, 0x55, 0x55, 0x55, 0x55, 0x55, 0x15 };
      memcpy(channel_map, channel_map_medium, sizeof(channel_map_medium));
    }
    break;
    case CS_CHANNEL_MAP_PRESET_HIGH:
    {
      uint8_t channel_map_high[10] = { 0xFC, 0xFF, 0x7F, 0xFC, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x1F };
      memcpy(channel_map, channel_map_high, sizeof(channel_map_high));
    }
    break;
    case CS_CHANNEL_MAP_PRESET_CUSTOM:
    {
      uint8_t channel_map_custom[10] = CS_CUSTOM_CHANNEL_MAP;
      memcpy(channel_map, channel_map_custom, sizeof(channel_map_custom));
    }
    break;
    default:
      // No other values allowed
      break;
  }
}

/******************************************************************************
 * Get the connection and procedure intervals based on
 * the procedure scheduling and input values.
 *****************************************************************************/
sl_status_t cs_initiator_get_intervals(uint8_t main_mode,
                                       uint8_t sub_mode,
                                       cs_procedure_scheduling_t procedure_scheduling,
                                       uint8_t channel_map_preset,
                                       uint8_t algo_mode,
                                       uint8_t antenna_path,
                                       uint8_t use_real_time_ras_mode,
                                       uint16_t *conn_interval,
                                       uint16_t *proc_interval)
{
  uint8_t input_values[5];
  input_values[0] = procedure_scheduling;
  input_values[1] = channel_map_preset;
  input_values[2] = algo_mode;
  input_values[3] = antenna_path;
  input_values[4] = use_real_time_ras_mode;
  if (conn_interval == NULL || proc_interval == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  if (procedure_scheduling == CS_PROCEDURE_SCHEDULING_CUSTOM) {
    return SL_STATUS_IDLE;
  }
  if (channel_map_preset == CS_CHANNEL_MAP_PRESET_CUSTOM
      || main_mode == sl_bt_cs_mode_rtt) {
    return SL_STATUS_NOT_SUPPORTED;
  }
  if (algo_mode == SL_RTL_CS_ALGO_MODE_STATIC_HIGH_ACCURACY) {
    *conn_interval = STATIC_MODE_CONNECTION_INTERVAL;
    *proc_interval = STATIC_MODE_PROCEDURE_INTERVAL;
    return SL_STATUS_OK;
  }
  if (input_values[0] == CS_PROCEDURE_SCHEDULING_OPTIMIZED_FOR_ENERGY
      && input_values[2] == SL_RTL_CS_ALGO_MODE_REAL_TIME_FAST) {
    input_values[2] = SL_RTL_CS_ALGO_MODE_REAL_TIME_BASIC;
  }
  for (uint16_t i = 0; i < sizeof(initiator_values_optimized); i++) {
    if (!memcmp(&initiator_values_optimized[i].input, &input_values, sizeof(input_values))) {
      *conn_interval = initiator_values_optimized[i].output.conn_interval;
      *proc_interval = initiator_values_optimized[i].output.proc_interval;
      // Add offset in case of submode present
      if (sub_mode != sl_bt_cs_submode_disabled) {
        uint16_t conn_ms = (uint16_t)(*conn_interval * 1.25f);
        uint16_t offset = (SUB_MODE_OFFSET_MS + conn_ms - 1) / conn_ms;
        *proc_interval += offset;
      }
      return SL_STATUS_OK;
    }
  }
  return SL_STATUS_NOT_FOUND;
}
