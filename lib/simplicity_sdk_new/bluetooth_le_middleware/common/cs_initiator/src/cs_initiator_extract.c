/***************************************************************************//**
 * @file
 * @brief CS initiator - extract implementation
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

// -----------------------------------------------------------------------------
// Includes

#include <stdint.h>
#include <stdbool.h>
#include "sl_bt_api.h"
#include "sl_rtl_clib_api.h"
#include "sl_sleeptimer.h"
#include "sl_component_catalog.h"

#include "cs_initiator_common.h"
#include "cs_initiator_log.h"
#include "cs_initiator_state_machine.h"
#include "cs_ras_format_converter.h"

#ifdef SL_CATALOG_CS_INITIATOR_REPORT_PRESENT
#include "cs_initiator_report.h"
#else
#define cs_initiator_report(evt)
#endif // SL_CATALOG_CS_INITIATOR_REPORT_PRESENT

#include "cs_initiator_extract.h"

// -----------------------------------------------------------------------------
// Public functions

/******************************************************************************
 * Reset subevent data and synchronization for an initiator instance
 *****************************************************************************/
void reset_subevent_data(cs_initiator_t *initiator, bool init)
{
  initiator->data.num_steps = 0;
  initiator->last_subevent_header = NULL;
  initiator->data.initiator.ranging_data_size = 0;
  initiator->num_antenna_path = 0;
  initiator->ranging_counter = CS_RAS_INVALID_RANGING_COUNTER;
  if (!init) {
    initiator_log_debug(INSTANCE_PREFIX "subevent data reset executed." LOG_NL,
                        initiator->conn_handle);
  }
  memset(initiator->data.initiator.ranging_data,
         0xFF,
         sizeof(initiator->data.initiator.ranging_data));
}

/******************************************************************************
 * Extract cs results (step data, subevent data) and hand over to RTL Lib
 * instance to calculate distance out of it.
 *
 * @return true if procedure restart required, false otherwise
 *****************************************************************************/
cs_procedure_state_t extract_cs_result_data(cs_initiator_t *initiator,
                                            cs_result_data_t *cs_result_content)
{
  uint8_t num_steps;
  uint8_t procedure_done_status;
  uint8_t subevent_done_status;
  uint8_t *step_data;
  uint32_t step_data_len;
  cs_procedure_state_t procedure_state = CS_PROCEDURE_STATE_IN_PROGRESS;

  initiator_log_debug(INSTANCE_PREFIX "extract - initiator data" LOG_NL,
                      initiator->conn_handle);

  // Extract
  if (cs_result_content->first_cs_result) {
    initiator->ranging_counter =
      cs_result_content->cs_event->data.evt_cs_result.procedure_counter
      & CS_RAS_RANGING_COUNTER_MASK;
    initiator->num_antenna_path
      = cs_result_content->cs_event->data.evt_cs_result.num_antenna_paths;
    procedure_done_status
      = cs_result_content->cs_event->data.evt_cs_result.procedure_done_status;
    subevent_done_status
      = cs_result_content->cs_event->data.evt_cs_result.subevent_done_status;
    num_steps = cs_result_content->cs_event->data.evt_cs_result.num_steps;
    step_data
      = cs_result_content->cs_event->data.evt_cs_result.data.data;
    step_data_len
      = cs_result_content->cs_event->data.evt_cs_result.data.len;
  } else {
    initiator->num_antenna_path
      = cs_result_content->cs_event->data.evt_cs_result_continue.num_antenna_paths;
    procedure_done_status
      = cs_result_content->cs_event->data.evt_cs_result_continue.procedure_done_status;
    subevent_done_status
      = cs_result_content->cs_event->data.evt_cs_result_continue.subevent_done_status;
    num_steps
      = cs_result_content->cs_event->data.evt_cs_result_continue.num_steps;
    step_data
      = cs_result_content->cs_event->data.evt_cs_result_continue.data.data;
    step_data_len
      = cs_result_content->cs_event->data.evt_cs_result_continue.data.len;
  }

  initiator_log_debug(INSTANCE_PREFIX "Initiator CS packet received - #%u procedure "
                                      "[proc_done_sts:%u, subevent_done_sts:%u]" LOG_NL,
                      initiator->conn_handle,
                      initiator->ranging_counter,
                      procedure_done_status,
                      subevent_done_status);

  cs_initiator_report(CS_INITIATOR_REPORT_FIRST_CS_RESULT);

  // Ranging header
  cs_ras_ranging_header_t *ranging_header
    = (cs_ras_ranging_header_t *)initiator->data.initiator.ranging_data;

  if (cs_result_content->first_cs_result) {
    if (initiator->last_subevent_header == NULL) {
      // Initialize size to the end of subevent header
      initiator->data.initiator.ranging_data_size = sizeof(cs_ras_ranging_header_t);
    }
    initiator->last_subevent_header
      = (cs_ras_subevent_header_t *)&initiator->data.initiator.ranging_data[initiator->data.initiator.ranging_data_size];
    // Increase size
    initiator->data.initiator.ranging_data_size += sizeof(cs_ras_subevent_header_t);
  }

  // Convert header
  (void)cs_ras_format_convert_header(initiator->last_subevent_header,
                                     ranging_header,
                                     cs_result_content->cs_event,
                                     initiator->antenna_config,
                                     !cs_result_content->first_cs_result);

  uint8_t *data_dst
    = &initiator->data.initiator.ranging_data[initiator->data.initiator.ranging_data_size];
  uint8_t *data_src = step_data;
  uint8_t step_mode;
  // Iterate over steps
  for (uint8_t i = 0; i < num_steps; i++) {
    cs_ras_step_header_t * step_header = (cs_ras_step_header_t *)data_src;
    // Check mode and abort
    step_mode = step_header->step_mode & CS_RAS_STEP_MODE_MASK;
    if (subevent_done_status == sl_bt_cs_done_status_aborted) {
      step_mode |= CS_RAS_STEP_ABORTED_MASK;
    }
    // Copy step mode
    memcpy(data_dst, &(step_mode), sizeof(step_mode));
    data_dst += sizeof(step_mode);
    data_src += sizeof(cs_ras_step_header_t);
    if (subevent_done_status != sl_bt_cs_done_status_aborted) {
      // Copy step data
      memcpy(data_dst, data_src, step_header->step_data_length);
      data_dst += step_header->step_data_length;
    }
    // Move on with source only
    data_src += step_header->step_data_length;
    // Add step channel
    initiator->data.step_channels[initiator->data.num_steps]
      = step_header->step_channel;
    initiator->data.num_steps++;
    if (data_src > data_src + step_data_len) {
      initiator_log_error(INSTANCE_PREFIX "Step data is partial" LOG_NL,
                          initiator->conn_handle);
      return CS_PROCEDURE_STATE_ABORTED;
    }
  }
  initiator->data.initiator.ranging_data_size
    = data_dst - initiator->data.initiator.ranging_data;

  switch (subevent_done_status) {
    case sl_bt_cs_done_status_complete:
      // Subevent done
      initiator_log_debug(INSTANCE_PREFIX "Subevent done with %d steps. Step count = %u" LOG_NL,
                          initiator->conn_handle,
                          initiator->last_subevent_header->number_of_steps_reported,
                          initiator->data.num_steps);
      break;
    case sl_bt_cs_done_status_partial_results_continue:
      // Subevent continue, more data to follow
      // Don't do anything. Data are buffered to the temporary buffer.
      initiator_log_debug(INSTANCE_PREFIX "Subevent continue" LOG_NL,
                          initiator->conn_handle);
      break;
    case sl_bt_cs_done_status_aborted:
      // Subevent aborted
      initiator_log_debug(INSTANCE_PREFIX "Subevent aborted" LOG_NL,
                          initiator->conn_handle);
      break;
    default:
      initiator_log_debug(INSTANCE_PREFIX "Unknown subevent done status" LOG_NL,
                          initiator->conn_handle);
      break;
  }

  switch (procedure_done_status) {
    case sl_bt_cs_done_status_complete:
      procedure_state = CS_PROCEDURE_STATE_COMPLETED;
      #if defined(CS_INITIATOR_CONFIG_LOG_DATA) && (CS_INITIATOR_CONFIG_LOG_DATA == 1)
      initiator_log_debug(INSTANCE_PREFIX "Initiator Ranging Data %u ready" LOG_NL,
                          initiator->conn_handle,
                          initiator->ranging_counter);
      initiator_log_hexdump_debug((initiator->data.initiator.ranging_data),
                                  (initiator->data.initiator.ranging_data_size));
      initiator_log_append_debug(LOG_NL);
      initiator_log_debug(INSTANCE_PREFIX "Procedure %u step count: %u, channels::" LOG_NL,
                          initiator->conn_handle,
                          initiator->ranging_counter,
                          initiator->data.num_steps);
      initiator_log_hexdump_debug((initiator->data.step_channels),
                                  (initiator->data.num_steps));
      initiator_log_append_debug(LOG_NL);

      #endif // defined(CS_INITIATOR_CONFIG_LOG_DATA) && (CS_INITIATOR_CONFIG_LOG_DATA == 1)
      break;
    case sl_bt_cs_done_status_partial_results_continue:
      // Continue gathering data
      procedure_state = CS_PROCEDURE_STATE_IN_PROGRESS;
      break;
    case sl_bt_cs_done_status_aborted:
      initiator_log_debug(INSTANCE_PREFIX "Procedure aborted" LOG_NL,
                          initiator->conn_handle);
      procedure_state = CS_PROCEDURE_STATE_ABORTED;
      break;
    default:
      initiator_log_error(INSTANCE_PREFIX "Unknown procedure done status" LOG_NL,
                          initiator->conn_handle);
      break;
  }

  initiator_log_info("----" LOG_NL);

  return procedure_state;
}

cs_procedure_state_t ranging_data_is_complete(uint8_t *data,
                                              uint32_t size,
                                              bool is_initiator,
                                              uint8_t antenna_path_num)
{
  uint8_t *data_end = data + size;
  cs_ras_subevent_header_t *subevent_header = NULL;
  bool aborted = false;
  bool completed = false;
  sl_status_t sc = SL_STATUS_OK;
  cs_procedure_state_t state = CS_PROCEDURE_STATE_ABORTED;
  sc = cs_ras_format_get_first_subevent_header(data, data_end, &subevent_header);
  uint8_t subevent = 0;

  while (sc == SL_STATUS_OK) {
    // Check for subevent and procedure aborts
    aborted |= (subevent_header->ranging_done_status == sl_bt_cs_done_status_aborted);
    aborted |= (subevent_header->subevent_done_status == sl_bt_cs_done_status_aborted);
    // Check for completed state
    completed |= (subevent_header->ranging_done_status == sl_bt_cs_done_status_complete);
    initiator_log_debug("Parse subevent %u, completed: %u, aborted: %u" LOG_NL,
                        subevent++,
                        completed,
                        aborted);
    if (aborted || completed) {
      break;
    }
    sc = cs_ras_format_get_next_subevent_header(subevent_header,
                                                data_end,
                                                is_initiator,
                                                antenna_path_num,
                                                &subevent_header);
  }
  (void)subevent;

  if (aborted) {
    return CS_PROCEDURE_STATE_ABORTED;
  }
  if (completed) {
    return CS_PROCEDURE_STATE_COMPLETED;
  }
  return state;
}
