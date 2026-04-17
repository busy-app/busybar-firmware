/***************************************************************************//**
 * @file
 * @brief CS initiator - state machine implementation
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
#include "sl_status.h"
#include "sl_bt_api.h"
#include "sl_component_catalog.h"
#include "app_timer.h"

#include "cs_initiator_common.h"
#include "cs_initiator_config.h"
#include "cs_initiator_extract.h"
#include "cs_initiator_error.h"
#include "cs_initiator_estimate.h"
#include "cs_initiator_log.h"
#include "cs_ras_format_converter.h"

#ifdef SL_CATALOG_CS_INITIATOR_REPORT_PRESENT
#include "cs_initiator_report.h"
#else
#define cs_initiator_report(evt)
#endif // SL_CATALOG_CS_INITIATOR_REPORT_PRESENT

#include "cs_initiator_state_machine.h"

// -----------------------------------------------------------------------------
// Macros

// Procedure enable maximum retry count
#define CS_MAX_PROCEDURE_ENABLE_RETRY  3

// -----------------------------------------------------------------------------
// Static function declarations

static sl_status_t state_any_on_error(cs_initiator_t             *initiator,
                                      state_machine_event_data_t *data);
static sl_status_t state_any_on_delete_instance(cs_initiator_t             *initiator,
                                                state_machine_event_data_t *data);
static sl_status_t state_uninitialized_on_init_started(cs_initiator_t             *initiator,
                                                       state_machine_event_data_t *data);
static sl_status_t state_init_on_start_init_completed(cs_initiator_t             *initiator,
                                                      state_machine_event_data_t *data);
static sl_status_t state_start_procedure_on_start_procedure(cs_initiator_t             *initiator,
                                                            state_machine_event_data_t *data);
static sl_status_t state_wait_procedure_enable_complete_on_enable_completed(cs_initiator_t             *initiator,
                                                                            state_machine_event_data_t *data);
static sl_status_t state_in_procedure_on_cs_result(cs_initiator_t             *initiator,
                                                   state_machine_event_data_t *data);
static sl_status_t state_wait_reflector_on_ranging_data(cs_initiator_t             *initiator,
                                                        state_machine_event_data_t *data,
                                                        bool                       complete);
static sl_status_t state_in_procedure_on_ranging_data(cs_initiator_t             *initiator,
                                                      state_machine_event_data_t *data);
static sl_status_t state_wait_procedure_disable_on_procedure_enable_completed(cs_initiator_t             *initiator,
                                                                              state_machine_event_data_t *data);
static sl_status_t state_delete_on_procedure_enable_completed(cs_initiator_t             *initiator,
                                                              state_machine_event_data_t *data);
static void handle_procedure_enable_completed_event_disable(cs_initiator_t *initiator);
static initiator_state_t initiator_stop_procedure_on_invalid_state(cs_initiator_t *initiator);
static sl_status_t initiator_finalize_cleanup(cs_initiator_t *initiator);
static void procedure_timer_cb(app_timer_t *handle, void *data);

// -----------------------------------------------------------------------------
// Static function definitions

static sl_status_t state_any_on_error(cs_initiator_t             *initiator,
                                      state_machine_event_data_t *data)
{
  app_timer_stop(&initiator->timer_handle);
  initiator_log_error(INSTANCE_PREFIX "Instance new state: ERROR" LOG_NL, initiator->conn_handle);
  initiator->initiator_state = (uint8_t)INITIATOR_STATE_ERROR;
  on_error(initiator,
           data->evt_error.error_type,
           data->evt_error.sc);
  return SL_STATUS_OK;
}

static sl_status_t state_any_on_delete_instance(cs_initiator_t             *initiator,
                                                state_machine_event_data_t *data)
{
  (void)data;
  sl_status_t sc = SL_STATUS_FAIL;
  state_machine_event_data_t data_out;

  sc = sl_bt_cs_procedure_enable(initiator->conn_handle,
                                 sl_bt_cs_procedure_state_disabled,
                                 initiator->config.config_id);

  if (sc == SL_STATUS_OK) {
    initiator_log_info(INSTANCE_PREFIX "Instance new state: DELETING" LOG_NL,
                       initiator->conn_handle);
    initiator->initiator_state = (uint8_t)INITIATOR_STATE_DELETE;
  } else if (sc == SL_STATUS_BT_CTRL_COMMAND_DISALLOWED
             || sc == SL_STATUS_BT_CTRL_INVALID_COMMAND_PARAMETERS
             || sc == SL_STATUS_INVALID_HANDLE) {
    // Procedure is already stopped by stack
    app_timer_stop(&initiator->timer_handle);
    sc = initiator_finalize_cleanup(initiator);
    if (sc == SL_STATUS_OK) {
      initiator_log_info(INSTANCE_PREFIX "Instance new state: UNINITIALIZED" LOG_NL,
                         initiator->conn_handle);
      initiator->initiator_state = (uint8_t)INITIATOR_STATE_UNINITIALIZED;
      return SL_STATUS_OK;
    } else {
      initiator->initiator_state = (uint8_t)INITIATOR_STATE_ERROR;
      data_out.evt_error.error_type = CS_ERROR_EVENT_INITIATOR_FAILED_TO_FINALIZE_CLEANUP;
      data_out.evt_error.sc = sc;
      sc = initiator_state_machine_event_handler(initiator,
                                                 INITIATOR_EVT_ERROR,
                                                 &data_out);
      return sc;
    }
  } else {
    // no other error code will be returned for now
    // if there will be one in the future it will generate an error state
    initiator_log_error(INSTANCE_PREFIX "failed to stop CS procedure! [sc: 0x%lx]" LOG_NL,
                        initiator->conn_handle,
                        (unsigned long)sc);
    initiator->initiator_state = (uint8_t)INITIATOR_STATE_ERROR;
    data_out.evt_error.error_type = CS_ERROR_EVENT_CS_PROCEDURE_STOP_FAILED;
    data_out.evt_error.sc = sc;
    sc = initiator_state_machine_event_handler(initiator,
                                               INITIATOR_EVT_ERROR,
                                               &data_out);
  }

  return sc;
}

static sl_status_t state_uninitialized_on_init_started(cs_initiator_t             *initiator,
                                                       state_machine_event_data_t *data)
{
  (void)data;
  initiator_log_info(INSTANCE_PREFIX "Instance new state: INIT" LOG_NL,
                     initiator->conn_handle);
  initiator->initiator_state = (uint8_t)INITIATOR_STATE_INIT;

  return SL_STATUS_OK;
}

static sl_status_t state_init_on_start_init_completed(cs_initiator_t             *initiator,
                                                      state_machine_event_data_t *data)
{
  sl_status_t sc = SL_STATUS_FAIL;
  state_machine_event_data_t data_out;

  if (data->evt_init_completed) {
    initiator_log_info(INSTANCE_PREFIX "Instance new state: START_PROCEDURE" LOG_NL,
                       initiator->conn_handle);
    initiator->initiator_state = (uint8_t)INITIATOR_STATE_START_PROCEDURE;
    initiator->procedure_enable_retry_counter = 0;
    if (initiator->ras_client.real_time_mode) {
      sc = cs_ras_client_real_time_receive(initiator->conn_handle,
                                           sizeof(initiator->data.reflector.ranging_data),
                                           initiator->data.reflector.ranging_data);
      if (sc != SL_STATUS_OK) {
        initiator_log_error(INSTANCE_PREFIX "RAS - failed to receive real-time data! [sc: 0x%lx]" LOG_NL,
                            initiator->conn_handle,
                            (unsigned long)sc);
        on_error(initiator,
                 CS_ERROR_EVENT_RAS_CLIENT_REALTIME_RECEIVE_FAILED,
                 sc);
        return sc;
      }
      initiator_log_info(INSTANCE_PREFIX "RAS - real-time data reception started" LOG_NL,
                         initiator->conn_handle);
    }
    sc = initiator_state_machine_event_handler(initiator,
                                               INITIATOR_EVT_START_PROCEDURE,
                                               NULL);
  } else {
    initiator->initiator_state = (uint8_t)INITIATOR_STATE_ERROR;
    data_out.evt_error.error_type = CS_ERROR_EVENT_INIT_FAILED;
    data_out.evt_error.sc = SL_STATUS_FAIL;
    sc = initiator_state_machine_event_handler(initiator,
                                               INITIATOR_EVT_ERROR,
                                               &data_out);
  }

  return sc;
}

static sl_status_t state_start_procedure_on_start_procedure(cs_initiator_t             *initiator,
                                                            state_machine_event_data_t *data)
{
  (void)data;
  sl_status_t sc = SL_STATUS_FAIL;
  state_machine_event_data_t data_out;

  initiator_log_debug(INSTANCE_PREFIX "CS procedure - request to start." LOG_NL,
                      initiator->conn_handle);
  // Before enabling procedure, check the security state
  if (!initiator->cs_security_enabled) {
    // Security is not enabled, move to error state
    initiator->initiator_state = (uint8_t)INITIATOR_STATE_ERROR;
    data_out.evt_error.error_type = CS_ERROR_EVENT_INITIATOR_FAILED_TO_ENABLE_CS_SECURITY;
    data_out.evt_error.sc = sc;
    sc = initiator_state_machine_event_handler(initiator,
                                               INITIATOR_EVT_ERROR,
                                               &data_out);
    return sc;
  }
  cs_initiator_report(CS_INITIATOR_REPORT_CS_PROCEDURE_BEGIN);
  sc = sl_bt_cs_procedure_enable(initiator->conn_handle,
                                 sl_bt_cs_procedure_state_enabled,
                                 initiator->config.config_id);

  if (sc == SL_STATUS_OK) {
    // Start timer for procedure timeout
    // The timer is stopped when the procedure is completed/aborted
    // for both the initiator and the reflector or when there was an error
    sc = app_timer_start(&initiator->timer_handle,
                         CS_INITIATOR_PROCEDURE_TIMEOUT_MS,
                         procedure_timer_cb,
                         (void *)initiator,
                         false);
    if (sc == SL_STATUS_OK) {
      cs_ras_client_procedure_enabled(initiator->conn_handle,
                                      true);
      initiator_log_info(INSTANCE_PREFIX "Instance new state: WAIT_PROCEDURE_ENABLE_COMPLETE" LOG_NL,
                         initiator->conn_handle);
      initiator->initiator_state = (uint8_t)INITIATOR_STATE_WAIT_PROCEDURE_ENABLE_COMPLETE;
      sc = SL_STATUS_OK;
    } else {
      initiator->initiator_state = (uint8_t)INITIATOR_STATE_ERROR;
      data_out.evt_error.error_type = CS_ERROR_EVENT_CS_PROCEDURE_START_TIMER_FAILED;
      data_out.evt_error.sc = sc;
      sc = initiator_state_machine_event_handler(initiator,
                                                 INITIATOR_EVT_ERROR,
                                                 &data_out);
    }
  } else {
    initiator->initiator_state = (uint8_t)INITIATOR_STATE_ERROR;
    data_out.evt_error.error_type = CS_ERROR_EVENT_CS_PROCEDURE_START_FAILED;
    data_out.evt_error.sc = sc;
    sc = initiator_state_machine_event_handler(initiator,
                                               INITIATOR_EVT_ERROR,
                                               &data_out);
  }

  return sc;
}

static sl_status_t state_wait_procedure_enable_complete_on_enable_completed(cs_initiator_t             *initiator,
                                                                            state_machine_event_data_t *data)
{
  sl_status_t sc = SL_STATUS_FAIL;
  state_machine_event_data_t data_out;

  if (data->evt_procedure_enable_completed->status == SL_STATUS_OK) {
    cs_initiator_report(CS_INITIATOR_REPORT_CS_PROCEDURE_STARTED);
    uint32_t time_tick = sl_sleeptimer_get_tick_count();
    initiator->procedure_start_time_ms =
      sl_sleeptimer_tick_to_ms(time_tick);

    initiator->cs_procedure_config.subevent_len =
      data->evt_procedure_enable_completed->subevent_len;
    initiator->cs_procedure_config.subevents_per_event =
      data->evt_procedure_enable_completed->subevents_per_event;
    initiator->cs_procedure_config.subevent_interval =
      data->evt_procedure_enable_completed->subevent_interval;
    initiator->cs_procedure_config.event_interval =
      data->evt_procedure_enable_completed->event_interval;
    initiator->cs_procedure_config.procedure_interval =
      data->evt_procedure_enable_completed->procedure_interval;
    initiator->cs_procedure_config.procedure_count =
      data->evt_procedure_enable_completed->procedure_count;

    initiator_log_info(INSTANCE_PREFIX "-------------------------------------------" LOG_NL,
                       initiator->conn_handle);
    initiator_log_info(INSTANCE_PREFIX "CS procedure - started @ %lu ms (%lu tick) "
                                       "subevent_len: %lu, "
                                       "subevent_interval: %u, "
                                       "subevents_per_event: %u, "
                                       "event_interval: %u, "
                                       "procedure_interval: %u, "
                                       "procedure_count: %u" LOG_NL,
                       initiator->conn_handle,
                       (unsigned long)initiator->procedure_start_time_ms,
                       (unsigned long)time_tick,
                       initiator->cs_procedure_config.subevent_len,
                       initiator->cs_procedure_config.subevent_interval,
                       initiator->cs_procedure_config.subevents_per_event,
                       initiator->cs_procedure_config.event_interval,
                       initiator->cs_procedure_config.procedure_interval,
                       initiator->cs_procedure_config.procedure_count);

    initiator_log_info(INSTANCE_PREFIX "Instance new state: IN_PROCEDURE" LOG_NL,
                       initiator->conn_handle);
    // Start with a clean slate
    reset_subevent_data(initiator, false);
    initiator->initiator_state = (uint8_t)INITIATOR_STATE_IN_PROCEDURE;
    sc = SL_STATUS_OK;
  } else {
    initiator_log_error(INSTANCE_PREFIX "CS procedure - start received error response! [status: 0x%x]" LOG_NL,
                        initiator->conn_handle,
                        data->evt_procedure_enable_completed->status);
    initiator->procedure_enable_retry_counter++;
    if (initiator->procedure_enable_retry_counter < CS_MAX_PROCEDURE_ENABLE_RETRY ) {
      initiator_log_error(INSTANCE_PREFIX "CS procedure - setting procedure parameters again..." LOG_NL,
                          initiator->conn_handle);
      sc = sl_bt_cs_set_procedure_parameters(initiator->conn_handle,
                                             initiator->config.config_id,
                                             initiator->config.max_procedure_duration,
                                             initiator->config.min_procedure_interval,
                                             initiator->config.max_procedure_interval,
                                             initiator->config.max_procedure_count,
                                             initiator->config.min_subevent_len,
                                             initiator->config.max_subevent_len,
                                             initiator->config.cs_tone_antenna_config_idx,
                                             initiator->config.conn_phy,
                                             initiator->config.tx_pwr_delta,
                                             initiator->config.preferred_peer_antenna,
                                             initiator->config.snr_control_initiator,
                                             initiator->config.snr_control_reflector);

      if (sc != SL_STATUS_OK) {
        initiator_log_error(INSTANCE_PREFIX "CS procedure - failed to set parameters! "
                                            "[sc: 0x%lx]" LOG_NL,
                            initiator->conn_handle,
                            (unsigned long)sc);
        on_error(initiator,
                 CS_ERROR_EVENT_CS_SET_PROCEDURE_PARAMETERS_FAILED,
                 sc);
        return sc;
      }
      initiator_log_info(INSTANCE_PREFIX "Instance new state: START_PROCEDURE" LOG_NL,
                         initiator->conn_handle);
      initiator->initiator_state = (uint8_t)INITIATOR_STATE_START_PROCEDURE;
      sc = initiator_state_machine_event_handler(initiator,
                                                 INITIATOR_EVT_START_PROCEDURE,
                                                 NULL);
      return sc;
    }
    app_timer_stop(&initiator->timer_handle);
    initiator->initiator_state = (uint8_t)INITIATOR_STATE_ERROR;
    data_out.evt_error.error_type = CS_ERROR_EVENT_CS_PROCEDURE_ENABLE_FAILED;
    data_out.evt_error.sc = SL_STATUS_FAIL;
    sc = initiator_state_machine_event_handler(initiator,
                                               INITIATOR_EVT_ERROR,
                                               &data_out);
  }

  return sc;
}

static sl_status_t state_in_procedure_on_ranging_data(cs_initiator_t             *initiator,
                                                      state_machine_event_data_t *data)
{
  sl_status_t sc = SL_STATUS_FAIL;
  state_machine_event_data_t data_out;

  // Reflector data is not valid
  if (!data->evt_ranging_data.initiator_part) {
    if (initiator->config.max_procedure_count != 0) {
      // Invalid event, stopping procedure
      initiator->initiator_state = (uint8_t)initiator_stop_procedure_on_invalid_state(initiator);
      sc = SL_STATUS_OK;
      if (initiator->initiator_state == ((uint8_t)INITIATOR_STATE_ERROR)) {
        data_out.evt_error.error_type = CS_ERROR_EVENT_CS_PROCEDURE_UNEXPECTED_DATA;
        data_out.evt_error.sc = sc;
        sc = initiator_state_machine_event_handler(initiator,
                                                   INITIATOR_EVT_ERROR,
                                                   &data_out);
      }
      return sc;
    }
    initiator_log_info(INSTANCE_PREFIX "CS - ignoring ranging data %u because of the ongoing measurement" LOG_NL,
                       initiator->conn_handle,
                       data->evt_ranging_data.ranging_counter);
    return SL_STATUS_OK;
  }
  // Initiator data
  if (data->evt_ranging_data.procedure_state == CS_PROCEDURE_STATE_ABORTED) {
    if (initiator->config.max_procedure_count != 0) {
      initiator_log_info(INSTANCE_PREFIX "Instance new state: WAIT_REFLECTOR_PROCEDURE_ABORTED" LOG_NL,
                         initiator->conn_handle);
      initiator->initiator_state = (uint8_t)INITIATOR_STATE_WAIT_REFLECTOR_PROCEDURE_ABORTED;
    } else {
      initiator_log_info(INSTANCE_PREFIX "Instance new state: INITIATOR_STATE_IN_PROCEDURE" LOG_NL,
                         initiator->conn_handle);
      initiator->initiator_state = (uint8_t)INITIATOR_STATE_IN_PROCEDURE;
      // Allow upcoming procedures
      reset_subevent_data(initiator, false);
      sc = SL_STATUS_OK;
    }
  } else if (data->evt_ranging_data.procedure_state == CS_PROCEDURE_STATE_COMPLETED) {
    initiator_log_info(INSTANCE_PREFIX "Instance new state: WAIT_REFLECTOR_PROCEDURE_COMPLETE" LOG_NL,
                       initiator->conn_handle);
    initiator->initiator_state = (uint8_t)INITIATOR_STATE_WAIT_REFLECTOR_PROCEDURE_COMPLETE;
    sc = SL_STATUS_OK;
  }
  return sc;
}

static sl_status_t state_in_procedure_on_cs_result(cs_initiator_t             *initiator,
                                                   state_machine_event_data_t *data)
{
  sl_status_t sc = SL_STATUS_OK;
  cs_procedure_state_t procedure_state;
  state_machine_event_data_t data_out;

  procedure_state = extract_cs_result_data(initiator, &data->evt_cs_result);

  if (procedure_state == CS_PROCEDURE_STATE_IN_PROGRESS) {
    initiator->initiator_state = (uint8_t)INITIATOR_STATE_IN_PROCEDURE;
    return sc;
  }

  if (procedure_state == CS_PROCEDURE_STATE_ABORTED) {
    initiator_log_info(INSTANCE_PREFIX "Initiator ranging data %u aborted" LOG_NL,
                       initiator->conn_handle,
                       initiator->ranging_counter);
  } else {
    initiator_log_info(INSTANCE_PREFIX "Initiator ranging data %u complete" LOG_NL,
                       initiator->conn_handle,
                       initiator->ranging_counter);
  }

  // Pass a ranging data event
  data_out.evt_ranging_data.initiator_part = true;
  data_out.evt_ranging_data.ranging_counter = initiator->ranging_counter;
  data_out.evt_ranging_data.data = initiator->data.initiator.ranging_data;
  data_out.evt_ranging_data.data_size = initiator->data.initiator.ranging_data_size;
  data_out.evt_ranging_data.procedure_state = procedure_state;
  sc = initiator_state_machine_event_handler(initiator,
                                             INITIATOR_EVT_RANGING_DATA,
                                             &data_out);
  return sc;
}

static sl_status_t state_wait_reflector_on_ranging_data(cs_initiator_t             *initiator,
                                                        state_machine_event_data_t *data,
                                                        bool                       initiator_complete)
{
  sl_status_t sc = SL_STATUS_FAIL;
  state_machine_event_data_t data_out;

  if (data->evt_ranging_data.initiator_part) {
    if (initiator->config.max_procedure_count != 0) {
      // Disable procedure
      initiator->initiator_state = (uint8_t)initiator_stop_procedure_on_invalid_state(initiator);
      sc = SL_STATUS_OK;
      if (initiator->initiator_state == ((uint8_t)INITIATOR_STATE_ERROR)) {
        data_out.evt_error.error_type = CS_ERROR_EVENT_CS_PROCEDURE_UNEXPECTED_DATA;
        data_out.evt_error.sc = sc;
        sc = initiator_state_machine_event_handler(initiator,
                                                   INITIATOR_EVT_ERROR,
                                                   &data_out);
      }
      return sc;
    }
    initiator_log_info(INSTANCE_PREFIX "CS - ignoring initiator ranging data %u  because %u is in progress" LOG_NL,
                       initiator->conn_handle,
                       data->evt_ranging_data.ranging_counter,
                       initiator->ranging_counter);
    return SL_STATUS_OK;
  }
  #if defined(CS_INITIATOR_CONFIG_LOG_DATA) && (CS_INITIATOR_CONFIG_LOG_DATA == 1)
  initiator_log_debug(INSTANCE_PREFIX "Reflector Ranging Data %u ready" LOG_NL,
                      initiator->conn_handle,
                      ((cs_ras_ranging_header_t *)initiator->data.reflector.ranging_data)->ranging_counter);
  initiator_log_hexdump_debug((initiator->data.reflector.ranging_data),
                              (initiator->data.reflector.ranging_data_size));
  initiator_log_append_debug(LOG_NL);
  #endif // defined(CS_INITIATOR_CONFIG_LOG_DATA) && (CS_INITIATOR_CONFIG_LOG_DATA == 1)
  if (data->evt_ranging_data.ranging_counter != initiator->ranging_counter) {
    if (initiator->config.max_procedure_count != 0) {
      // Disable procedure
      initiator->initiator_state = (uint8_t)initiator_stop_procedure_on_invalid_state(initiator);
      sc = SL_STATUS_OK;
      if (initiator->initiator_state == ((uint8_t)INITIATOR_STATE_ERROR)) {
        data_out.evt_error.error_type = CS_ERROR_EVENT_CS_PROCEDURE_UNEXPECTED_DATA;
        data_out.evt_error.sc = sc;
        sc = initiator_state_machine_event_handler(initiator,
                                                   INITIATOR_EVT_ERROR,
                                                   &data_out);
      }
      return sc;
    }
    initiator_log_info(INSTANCE_PREFIX "CS - ignoring reflector ranging data %u because %u is in progress" LOG_NL,
                       initiator->conn_handle,
                       data->evt_ranging_data.ranging_counter,
                       initiator->ranging_counter);

    return SL_STATUS_OK;
  }

  sc = app_timer_stop(&initiator->timer_handle);
  if (sc != SL_STATUS_OK) {
    initiator->initiator_state = (uint8_t)INITIATOR_STATE_ERROR;
    data_out.evt_error.error_type = CS_ERROR_EVENT_CS_PROCEDURE_STOP_TIMER_FAILED;
    data_out.evt_error.sc = sc;
    sc = initiator_state_machine_event_handler(initiator,
                                               INITIATOR_EVT_ERROR,
                                               &data_out);
  }
  if (initiator->config.max_procedure_count != 0) {
    initiator_log_info(INSTANCE_PREFIX "Instance new state: START_PROCEDURE" LOG_NL,
                       initiator->conn_handle);
    initiator->initiator_state = (uint8_t)INITIATOR_STATE_START_PROCEDURE;
    sc = initiator_state_machine_event_handler(initiator,
                                               INITIATOR_EVT_START_PROCEDURE,
                                               NULL);
    if ((data->evt_ranging_data.procedure_state == CS_PROCEDURE_STATE_COMPLETED)
        && initiator_complete) {
      cs_initiator_report(CS_INITIATOR_REPORT_LAST_CS_RESULT);
      calculate_distance(initiator);
    } else {
      initiator_log_info(INSTANCE_PREFIX "Procedure not completed: %u" LOG_NL,
                         initiator->conn_handle,
                         data->evt_ranging_data.ranging_counter);
    }
    // Also reset subevent data
    reset_subevent_data(initiator, false);
    return sc;
  }

  initiator_log_info(INSTANCE_PREFIX "Instance new state: IN_PROCEDURE" LOG_NL,
                     initiator->conn_handle);
  initiator->initiator_state = (uint8_t)INITIATOR_STATE_IN_PROCEDURE;

  if ((data->evt_ranging_data.procedure_state == CS_PROCEDURE_STATE_COMPLETED)
      && initiator_complete) {
    cs_initiator_report(CS_INITIATOR_REPORT_LAST_CS_RESULT);
    calculate_distance(initiator);
  } else {
    initiator_log_info(INSTANCE_PREFIX "Procedure not completed: %u" LOG_NL,
                       initiator->conn_handle,
                       data->evt_ranging_data.ranging_counter);
  }
  // Procedure data processed in free running mode, clear the subevent data
  reset_subevent_data(initiator, false);

  sc = SL_STATUS_OK;
  return sc;
}

static sl_status_t state_wait_procedure_disable_on_procedure_enable_completed(cs_initiator_t             *initiator,
                                                                              state_machine_event_data_t *data)
{
  sl_status_t sc = SL_STATUS_FAIL;
  state_machine_event_data_t data_out;
  if (data->evt_procedure_enable_completed->state) {
    return SL_STATUS_OK;
  }
  handle_procedure_enable_completed_event_disable(initiator);
  if (data->evt_procedure_enable_completed->status == SL_STATUS_OK) {
    app_timer_stop(&initiator->timer_handle);
    initiator_log_info(INSTANCE_PREFIX "Instance new state: START_PROCEDURE" LOG_NL,
                       initiator->conn_handle);
    initiator->initiator_state = (uint8_t)INITIATOR_STATE_START_PROCEDURE;
    sc = initiator_state_machine_event_handler(initiator,
                                               INITIATOR_EVT_START_PROCEDURE,
                                               NULL);
    return sc;
  }
  initiator_log_error(INSTANCE_PREFIX "CS procedure - stop received error response! [status: 0x%x]" LOG_NL,
                      initiator->conn_handle,
                      data->evt_procedure_enable_completed->status);
  initiator->initiator_state = (uint8_t)INITIATOR_STATE_ERROR;
  data_out.evt_error.error_type = CS_ERROR_EVENT_CS_PROCEDURE_STOP_FAILED;
  data_out.evt_error.sc = data->evt_procedure_enable_completed->status;
  sc = initiator_state_machine_event_handler(initiator,
                                             INITIATOR_EVT_ERROR,
                                             &data_out);
  return sc;
}

static sl_status_t state_delete_on_procedure_enable_completed(cs_initiator_t             *initiator,
                                                              state_machine_event_data_t *data)
{
  sl_status_t sc = SL_STATUS_FAIL;
  state_machine_event_data_t data_out;
  if (data->evt_procedure_enable_completed->state) {
    return SL_STATUS_OK;
  }
  if (data->evt_procedure_enable_completed->status == SL_STATUS_OK) {
    // Procedure was disabled
    handle_procedure_enable_completed_event_disable(initiator);
    sc = initiator_finalize_cleanup(initiator);
    if (sc == SL_STATUS_OK) {
      initiator_log_info(INSTANCE_PREFIX "Instance new state: UNINITIALIZED" LOG_NL,
                         initiator->conn_handle);
      initiator->initiator_state = (uint8_t)INITIATOR_STATE_UNINITIALIZED;
      return sc;
    }
    initiator->initiator_state = (uint8_t)INITIATOR_STATE_ERROR;
    data_out.evt_error.error_type = CS_ERROR_EVENT_INITIATOR_FAILED_TO_FINALIZE_CLEANUP;
    data_out.evt_error.sc = sc;
    sc = initiator_state_machine_event_handler(initiator,
                                               INITIATOR_EVT_ERROR,
                                               &data_out);
    return sc;
  }
  // Only happens if the remote device rejects the procedure termination - error state
  initiator_log_error(INSTANCE_PREFIX "CS procedure - stop received error response! [status: 0x%x]" LOG_NL,
                      initiator->conn_handle,
                      data->evt_procedure_enable_completed->status);
  initiator->initiator_state = (uint8_t)INITIATOR_STATE_ERROR;
  data_out.evt_error.error_type = CS_ERROR_EVENT_CS_PROCEDURE_STOP_FAILED;
  data_out.evt_error.sc = data->evt_procedure_enable_completed->status;
  sc = initiator_state_machine_event_handler(initiator,
                                             INITIATOR_EVT_ERROR,
                                             &data_out);
  return sc;
}

/******************************************************************************
 * Handle procedure_enable_completed event when procedure was
 * disabled successfully.
 * @param[in] initiator initiator instance
 *****************************************************************************/
static void handle_procedure_enable_completed_event_disable(cs_initiator_t *initiator)
{
  uint32_t time_tick = sl_sleeptimer_get_tick_count();
  initiator->procedure_stop_time_ms =
    sl_sleeptimer_tick_to_ms(time_tick);
  initiator_log_info(INSTANCE_PREFIX "CS procedure - stopped @ %lu ms "
                                     "(%lu tick)" LOG_NL,
                     initiator->conn_handle,
                     (unsigned long)initiator->procedure_stop_time_ms,
                     (unsigned long)time_tick);
}

static initiator_state_t initiator_stop_procedure_on_invalid_state(cs_initiator_t *initiator)
{
  sl_status_t sc;

  sc = sl_bt_cs_procedure_enable(initiator->conn_handle,
                                 sl_bt_cs_procedure_state_disabled,
                                 initiator->config.config_id);
  if (sc == SL_STATUS_OK) {
    initiator_log_info(INSTANCE_PREFIX "Instance new state: WAIT_PROCEDURE_DISABLE_COMPLETE" LOG_NL,
                       initiator->conn_handle);
    return INITIATOR_STATE_WAIT_PROCEDURE_DISABLE_COMPLETE;
  } else if (sc == SL_STATUS_INVALID_HANDLE) {
    // Procedure is already stopped by stack
    app_timer_stop(&initiator->timer_handle);
    initiator_log_info(INSTANCE_PREFIX "Instance new state: START_PROCEDURE" LOG_NL,
                       initiator->conn_handle);
    return INITIATOR_STATE_START_PROCEDURE;
  } else {
    // all other errors creates an error state
    return INITIATOR_STATE_ERROR;
  }
}

/******************************************************************************
 * Initiator finalize cleanup. Remove the configuration and deinit RTL lib.
 * This function is called after the procedure was stopped.
 * @param[in] initiator pointer to the initiator instance.
 * @return SL_STATUS_OK if cleanup was successful.
 * @return SL_STATUS_FAIL if cleanup failed.
 *****************************************************************************/
static sl_status_t initiator_finalize_cleanup(cs_initiator_t *initiator)
{
  enum sl_rtl_error_code rtl_err;
  sl_status_t sc = SL_STATUS_OK;
  (void)sl_bt_cs_remove_config(initiator->conn_handle, initiator->config.config_id);

  if (initiator->rtl_handle != NULL) {
    rtl_err = sl_rtl_cs_deinit(&initiator->rtl_handle);
    if (rtl_err != SL_RTL_ERROR_SUCCESS) {
      initiator_log_error(INSTANCE_PREFIX "Failed to deinit RTL lib! [err: 0x%02x]" LOG_NL,
                          initiator->conn_handle,
                          rtl_err);
      return SL_STATUS_FAIL;
    }
  }

  initiator_log_debug(INSTANCE_PREFIX "deleting instance" LOG_NL,
                      initiator->conn_handle);

  memset(initiator, 0, sizeof(cs_initiator_t));
  initiator_log_info(INSTANCE_PREFIX "instance deleted" LOG_NL, initiator->conn_handle);
  initiator->conn_handle = SL_BT_INVALID_CONNECTION_HANDLE;

  initiator->ras_client.real_time_mode = CS_INITIATOR_RAS_MODE_USE_REAL_TIME_MODE;

  initiator->ras_client.service = 0xFFFFFFFF;
  initiator->ras_client.mtu = ATT_MTU_MIN;

  initiator->ras_client.config.real_time_ranging_data_indication =
    CS_INITIATOR_RAS_REAL_TIME_INDICATION;
  initiator->ras_client.config.on_demand_ranging_data_indication =
    CS_INITIATOR_RAS_ON_DEMAND_INDICATION;
  initiator->ras_client.config.ranging_data_ready_notification =
    CS_INITIATOR_RAS_DATA_READY_NOTIFICATION;
  initiator->ras_client.config.ranging_data_overwritten_notification =
    CS_INITIATOR_RAS_DATA_OVERWRITTEN_NOTIFICATION;

  initiator->data.reflector.ranging_data_size = 0;
  memset(&initiator->data.reflector.ranging_data,
         0,
         sizeof(initiator->data.reflector.ranging_data));
  return sc;
}

// -----------------------------------------------------------------------------
// Event / callback definitions

/******************************************************************************
 * Initiator state machine
 *****************************************************************************/
sl_status_t initiator_state_machine_event_handler(cs_initiator_t *initiator,
                                                  state_machine_event_t      event,
                                                  state_machine_event_data_t *data)
{
  sl_status_t sc = SL_STATUS_FAIL;

  if (initiator == NULL) {
    initiator_log_error(INSTANCE_PREFIX "null reference to initiator instance!" LOG_NL,
                        (uint8_t)SL_BT_INVALID_CONNECTION_HANDLE);
    on_error(initiator,
             CS_ERROR_EVENT_STATE_MACHINE_FAILED,
             SL_STATUS_NULL_POINTER);
    return SL_STATUS_NULL_POINTER;
  }

  if (event == INITIATOR_EVT_ERROR) {
    sc = state_any_on_error(initiator, data);
  }

  if (event == INITIATOR_EVT_DELETE_INSTANCE) {
    sc = state_any_on_delete_instance(initiator, data);
    return sc;
  }

  switch (initiator->initiator_state) {
    case INITIATOR_STATE_UNINITIALIZED:
      if (event == INITIATOR_EVT_INIT_STARTED) {
        sc = state_uninitialized_on_init_started(initiator, data);
      }
      break;

    case INITIATOR_STATE_INIT:
      if (event == INITIATOR_EVT_INIT_COMPLETED) {
        sc = state_init_on_start_init_completed(initiator, data);
      }
      break;

    case INITIATOR_STATE_START_PROCEDURE:
      if (event == INITIATOR_EVT_START_PROCEDURE) {
        sc = state_start_procedure_on_start_procedure(initiator, data);
      }
      break;

    case INITIATOR_STATE_WAIT_PROCEDURE_ENABLE_COMPLETE:
      if (event == INITIATOR_EVT_PROCEDURE_ENABLE_COMPLETED) {
        sc = state_wait_procedure_enable_complete_on_enable_completed(initiator,
                                                                      data);
      }
      break;

    case INITIATOR_STATE_IN_PROCEDURE:
      if (event == INITIATOR_EVT_RANGING_DATA) {
        sc = state_in_procedure_on_ranging_data(initiator, data);
      }
      if ((event == INITIATOR_EVT_CS_RESULT)
          || (event == INITIATOR_EVT_CS_RESULT_CONTINUE)) {
        sc = state_in_procedure_on_cs_result(initiator, data);
      }
      break;

    case INITIATOR_STATE_WAIT_REFLECTOR_PROCEDURE_COMPLETE:
      if (event == INITIATOR_EVT_RANGING_DATA) {
        sc = state_wait_reflector_on_ranging_data(initiator, data, true);
      }
      break;

    case INITIATOR_STATE_WAIT_REFLECTOR_PROCEDURE_ABORTED:
      if (event == INITIATOR_EVT_RANGING_DATA) {
        sc = state_wait_reflector_on_ranging_data(initiator, data, false);
      }
      break;

    case INITIATOR_STATE_WAIT_PROCEDURE_DISABLE_COMPLETE:
      if (event == INITIATOR_EVT_PROCEDURE_ENABLE_COMPLETED) {
        sc = state_wait_procedure_disable_on_procedure_enable_completed(initiator,
                                                                        data);
      }
      break;

    case INITIATOR_STATE_DELETE:
      if (event == INITIATOR_EVT_PROCEDURE_ENABLE_COMPLETED) {
        sc = state_delete_on_procedure_enable_completed(initiator, data);
      }
      break;

    case INITIATOR_STATE_ERROR:
      break;

    default:
      break;
  }
  return sc;
}

/******************************************************************************
 * Procedure timer callback
 *****************************************************************************/
static void procedure_timer_cb(app_timer_t *handle, void *data)
{
  cs_initiator_t *initiator = (cs_initiator_t *)data;
  if (handle == &initiator->timer_handle) {
    initiator->error_timer_started = false;
    initiator->error_timer_elapsed = true;
    on_error(initiator,
             CS_ERROR_EVENT_TIMER_ELAPSED,
             SL_STATUS_TIMEOUT);
  }
}
