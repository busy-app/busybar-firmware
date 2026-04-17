/***************************************************************************//**
 * @file
 * @brief CS RAS Format converter
 *******************************************************************************
 * # License
 * <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
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

#include "cs_ras_format_converter.h"

// -----------------------------------------------------------------------------
// Definitions

#define CS_RAS_RANGING_COUNTER_BIT_OFFSET  4
#define CS_RAS_STATUS_BIT_OFFSET           4
#define CS_RAS_DONE_STATUS_MASK            0x0f
#define CS_RAS_CONFIG_ID_MASK              0x0f
#define ANTENNA_PERMUTATION_INDEX_BYTE_LEN 1
#define TONE_BYTE_LEN                      4
#define MAX_ANTENNA_PATH_NUM               4
#define MODE_0_SIZE(i)                     ((i) ? 5 : 3)
#define MODE_1_SIZE                        6
#define MODE_2_SIZE(a)                     (ANTENNA_PERMUTATION_INDEX_BYTE_LEN + ((a) + 1) * TONE_BYTE_LEN)
#define AP1                                (1 << 0)
#define AP2                                (1 << 1)
#define AP3                                (1 << 2)
#define AP4                                (1 << 3)

typedef enum {
  CS_RAS_STEP_MODE_CALIBRATION = 0x00,
  CS_RAS_STEP_MODE_RTT         = 0x01,
  CS_RAS_STEP_MODE_PBR         = 0x02,
} cs_ras_step_mode_t;

// -----------------------------------------------------------------------------
// Public functions

sl_status_t cs_ras_format_convert_header(cs_ras_subevent_header_t *ras_subevent_header,
                                         cs_ras_ranging_header_t *ras_header,
                                         const sl_bt_msg_t *cs_res,
                                         uint8_t antenna_config,
                                         bool continue_evt)
{
  sl_status_t sc = SL_STATUS_OK;

  if (cs_res == NULL
      || ras_header == NULL
      || ras_subevent_header == NULL) {
    return SL_STATUS_NULL_POINTER;
  }

  if (continue_evt) {
    ras_subevent_header->subevent_done_status
      = cs_res->data.evt_cs_result_continue.subevent_done_status;
    ras_subevent_header->ranging_done_status
      = cs_res->data.evt_cs_result_continue.procedure_done_status;
    ras_subevent_header->abort_reason
      = cs_res->data.evt_cs_result_continue.abort_reason;
    ras_subevent_header->number_of_steps_reported
      += cs_res->data.evt_cs_result_continue.num_steps;
  } else {
    // Construct the antenna_paths_mask field.
    // This field will be deleted. The implementation is for v1.0 compatibility
    switch (antenna_config) {
      case 0:
        ras_header->antenna_paths_mask = AP1;
        break;
      case 1:
      case 4:
        ras_header->antenna_paths_mask = AP1 | AP2;
        break;
      case 7:
        ras_header->antenna_paths_mask = AP1 | AP2 | AP3 | AP4;
        break;
      default:
        ras_header->antenna_paths_mask = AP1 | AP2 | AP3 | AP4;
        break;
    }
    ras_header->ranging_counter = cs_res->data.evt_cs_result.procedure_counter
                                  & CS_RAS_RANGING_COUNTER_MASK;
    ras_header->configuration_id = cs_res->data.evt_cs_result.config_id
                                   & CS_RAS_CONFIG_ID_MASK;
    ras_subevent_header->start_acl_connection_event
      = cs_res->data.evt_cs_result.start_acl_conn_event;
    ras_subevent_header->frequency_compensation
      = cs_res->data.evt_cs_result.frequency_compensation;
    ras_subevent_header->subevent_done_status
      = cs_res->data.evt_cs_result.subevent_done_status;
    ras_subevent_header->ranging_done_status
      = cs_res->data.evt_cs_result.procedure_done_status;
    ras_subevent_header->abort_reason
      = cs_res->data.evt_cs_result.abort_reason;
    ras_subevent_header->reference_power_level
      = cs_res->data.evt_cs_result.reference_power_level;
    ras_subevent_header->number_of_steps_reported
      = cs_res->data.evt_cs_result.num_steps;
  }

  return sc;
}

sl_status_t cs_ras_format_get_first_subevent_header(uint8_t *data,
                                                    uint8_t *data_end,
                                                    cs_ras_subevent_header_t **subevent_header_out)
{
  if ((data == NULL)
      || (subevent_header_out == NULL)
      || (data_end == NULL)) {
    return SL_STATUS_NULL_POINTER;
  }
  if (data_end < (data + sizeof(cs_ras_ranging_header_t) + sizeof(cs_ras_subevent_header_t))) {
    return SL_STATUS_WOULD_OVERFLOW;
  }
  // Calculate position
  *subevent_header_out = (cs_ras_subevent_header_t *)(data + sizeof(cs_ras_ranging_header_t));
  return SL_STATUS_OK;
}

sl_status_t cs_ras_format_get_next_subevent_header(cs_ras_subevent_header_t *subevent_header,
                                                   uint8_t *data_end,
                                                   bool is_initiator,
                                                   uint8_t antenna_path_num,
                                                   cs_ras_subevent_header_t **subevent_header_out)
{
  if ((subevent_header == NULL)
      || (subevent_header_out == NULL)
      || (data_end == NULL)) {
    return SL_STATUS_NULL_POINTER;
  }
  if ((antenna_path_num == 0) || (antenna_path_num > MAX_ANTENNA_PATH_NUM)) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  uint8_t step_mode;
  bool step_aborted;
  uint8_t step_size;
  uint8_t *position = ((uint8_t *)subevent_header)
                      + sizeof(cs_ras_subevent_header_t);
  if (position + sizeof(cs_ras_subevent_header_t) > data_end) {
    return SL_STATUS_WOULD_OVERFLOW;
  }
  // Iterate over steps
  for (uint8_t i = 0; i < subevent_header->number_of_steps_reported; i++) {
    step_mode = (*position) & CS_RAS_STEP_MODE_MASK;
    // Check aborted bit
    step_aborted = ((*position) & CS_RAS_STEP_ABORTED_MASK) > 0;
    // Calculate step size based on the mode, role and number of antenna path.
    if (!step_aborted) {
      switch ((cs_ras_step_mode_t)step_mode) {
        case CS_RAS_STEP_MODE_CALIBRATION:
          step_size = MODE_0_SIZE(is_initiator);
          break;
        case CS_RAS_STEP_MODE_RTT:
          step_size = MODE_1_SIZE;
          break;
        case CS_RAS_STEP_MODE_PBR:
          step_size = MODE_2_SIZE(antenna_path_num);
          break;
        default:
          return SL_STATUS_INVALID_MODE;
      }
    } else {
      // If the Step is aborted and bit 7 is set to 1, then bits 0-6 do not
      // contain any valid data and the length of Step_Data [i] is 0.
      step_size = 0;
    }
    // Move
    position += sizeof(step_mode) + step_size;
    // Check end or overflow
    if (position == data_end) {
      return SL_STATUS_NOT_FOUND;
    } else if (position > data_end) {
      return SL_STATUS_WOULD_OVERFLOW;
    }
  }
  // Set output
  *subevent_header_out = (cs_ras_subevent_header_t *)position;
  return SL_STATUS_OK;
}
