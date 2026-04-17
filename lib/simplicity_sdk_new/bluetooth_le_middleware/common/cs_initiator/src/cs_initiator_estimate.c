/***************************************************************************//**
 * @file
 * @brief CS initiator - estimate implementation
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

#include <stdbool.h>
#include <math.h>
#include <limits.h>
#include "sl_bt_api.h"
#include "sl_rtl_clib_api.h"
#include "sl_sleeptimer.h"
#include "sl_component_catalog.h"

#include "cs_result.h"
#include "cs_initiator_common.h"
#include "cs_initiator_log.h"
#include "cs_initiator_extract.h"
#include "cs_initiator_error.h"
#include "cs_ras_format_converter.h"
#include "cs_initiator_estimate.h"

#ifdef SL_CATALOG_CS_INITIATOR_REPORT_PRESENT
#include "cs_initiator_report.h"
#else
#define cs_initiator_report(evt)
#endif // SL_CATALOG_CS_INITIATOR_REPORT_PRESENT

// -----------------------------------------------------------------------------
// Static function declarations
static void show_rtl_api_call_result(cs_initiator_t *initiator,
                                     enum sl_rtl_error_code err_code);
static void report_result(cs_initiator_t *initiator);
static void report_intermediate_result(cs_initiator_t *initiator);
static void cs_mode_converter(sl_bt_cs_mode_t cs_mode_bt,
                              char **cs_mode_str,
                              sl_rtl_cs_mode *cs_mode_rtl);
// -----------------------------------------------------------------------------
// Static function definitions

/******************************************************************************
 * Show error messages based on RTL API call error codes.
 *
 * @param[in] conn_handle connection handle if the selected initiator instance.
 * @param[in] err_code RTL API error code.
 *****************************************************************************/
static void show_rtl_api_call_result(cs_initiator_t *initiator,
                                     enum sl_rtl_error_code err_code)
{
  switch (err_code) {
    case SL_RTL_ERROR_SUCCESS:
      break;
    case SL_RTL_ERROR_ARGUMENT:
      initiator_log_error(INSTANCE_PREFIX "RTL - invalid argument! [E: 0x%x]" LOG_NL,
                          initiator->conn_handle,
                          err_code);
      break;

    case SL_RTL_ERROR_OUT_OF_MEMORY:
      initiator_log_error(INSTANCE_PREFIX "RTL - memory allocation error! [E: 0x%x]" LOG_NL,
                          initiator->conn_handle,
                          err_code);
      break;

    case SL_RTL_ERROR_ESTIMATION_IN_PROGRESS:
      initiator_log_info(INSTANCE_PREFIX "RTL - estimation not yet finished! [E: 0x%x]" LOG_NL,
                         initiator->conn_handle,
                         err_code);
      break;

    case SL_RTL_ERROR_NUMBER_OF_SNAPHOTS_DO_NOT_MATCH:
      initiator_log_error(INSTANCE_PREFIX "RTL - initialized and calculated "
                                          "snapshots do not match! [E: 0x%x]" LOG_NL,
                          initiator->conn_handle,
                          err_code);
      break;

    case SL_RTL_ERROR_ESTIMATOR_NOT_CREATED:
      initiator_log_error(INSTANCE_PREFIX "RTL - estimator not created! [E: 0x%x]" LOG_NL,
                          initiator->conn_handle,
                          err_code);
      break;

    case SL_RTL_ERROR_ESTIMATOR_ALREADY_CREATED:
      initiator_log_error(INSTANCE_PREFIX "RTL - estimator already created! [E: 0x%x]" LOG_NL,
                          initiator->conn_handle,
                          err_code);
      break;

    case SL_RTL_ERROR_NOT_INITIALIZED:
      initiator_log_error(INSTANCE_PREFIX "RTL - library item not initialized! [E: 0x%x]" LOG_NL,
                          initiator->conn_handle,
                          err_code);
      break;

    case SL_RTL_ERROR_INTERNAL:
      initiator_log_error(INSTANCE_PREFIX "RTL - internal error! [E: 0x%x]" LOG_NL,
                          initiator->conn_handle,
                          err_code);
      break;

    case SL_RTL_ERROR_IQ_SAMPLE_QA:
      initiator_log_error(INSTANCE_PREFIX "RTL - IQ sample quality analysis failed! [E: 0x%x]" LOG_NL,
                          initiator->conn_handle,
                          err_code);
      break;

    case SL_RTL_ERROR_FEATURE_NOT_SUPPORTED:
      initiator_log_error(INSTANCE_PREFIX "RTL - feature not supported! [E: 0x%x]" LOG_NL,
                          initiator->conn_handle,
                          err_code);
      break;

    case SL_RTL_ERROR_INCORRECT_MEASUREMENT:
      initiator_log_error(INSTANCE_PREFIX "RTL - incorrect measurement! Error of the last"
                                          " measurement was too large! [E: 0x%x]" LOG_NL,
                          initiator->conn_handle,
                          err_code);
      break;

    case SL_RTL_ERROR_CS_CHANNEL_MAP_TOO_SPARSE:
      initiator_log_error(INSTANCE_PREFIX "RTL - too many skipped channels "
                                          "in the proposed channel map! [E: 0x%x]" LOG_NL,
                          initiator->conn_handle,
                          err_code);
      break;

    case SL_RTL_ERROR_CS_CHANNEL_MAP_TOO_FEW_CHANNELS:
      initiator_log_error(INSTANCE_PREFIX "RTL - too few channels "
                                          "in the proposed channel map! [E: 0x%x]" LOG_NL,
                          initiator->conn_handle,
                          err_code);
      break;

    case SL_RTL_ERROR_CS_CHANNEL_SPACING_TOO_LARGE:
      initiator_log_error(INSTANCE_PREFIX "RTL - channel spacing is too large "
                                          "in the proposed channel map! [E: 0x%x]" LOG_NL,
                          initiator->conn_handle,
                          err_code);
      break;

    case SL_RTL_ERROR_POOR_INPUT_DATA_QUALITY:
      initiator_log_error(INSTANCE_PREFIX "RTL - input data quality is poor! [E: 0x%x]" LOG_NL,
                          initiator->conn_handle,
                          err_code);
      break;

    default:
      initiator_log_error(INSTANCE_PREFIX "RTL - unknown error! [E: 0x%x]" LOG_NL,
                          initiator->conn_handle,
                          err_code);
      break;
  }
  (void)initiator;
}

/******************************************************************************
 * Handle successful RTL process, and get distance.
 *
 * @param[in] initiator initiator instance.
 *****************************************************************************/
static void report_result(cs_initiator_t *initiator)
{
  sl_status_t sc = SL_STATUS_OK;
  bool estimation_valid = false;
  enum sl_rtl_error_code rtl_err = SL_RTL_ERROR_NOT_INITIALIZED;
  sl_rtl_cs_estimator_param param;
  sl_rtl_cs_distance_estimate_mode mode;

  float rtl_value = 0.0f;
  float last_known_distance = 0.0f;

  // initialize result data
  cs_result_initialize_results_data(&initiator->result_data);

  if (initiator->config.cs_sub_mode == sl_bt_cs_submode_disabled) {
    mode = SL_RTL_CS_BEST_ESTIMATE;
  } else {
    mode = SL_RTL_CS_MAIN_MODE_ESTIMATE;
  }

  // --------------------------------
  // Get distance
  initiator_log_debug(INSTANCE_PREFIX "RTL - get distance" LOG_NL,
                      initiator->conn_handle);
  rtl_err = sl_rtl_cs_get_distance_estimate(&initiator->rtl_handle,
                                            SL_RTL_CS_DISTANCE_ESTIMATE_TYPE_FILTERED,
                                            mode,
                                            &last_known_distance);

  show_rtl_api_call_result(initiator, rtl_err);
  if (rtl_err == SL_RTL_ERROR_SUCCESS) {
    sc = cs_result_append_field(&initiator->result_data,
                                CS_RESULT_FIELD_DISTANCE_MAINMODE,
                                (uint8_t *)&last_known_distance,
                                initiator->result);
    if (sc != SL_STATUS_OK) {
      initiator_log_error(INSTANCE_PREFIX "RTL - failed to append distance! [sc: 0x%lx]" LOG_NL,
                          initiator->conn_handle,
                          (unsigned long)sc);
    } else {
      estimation_valid = true;
    }
  } else {
    initiator_log_error(INSTANCE_PREFIX "RTL - failed to get distance data! [E: 0x%x]" LOG_NL,
                        initiator->conn_handle,
                        rtl_err);
  }

  if (initiator->config.cs_sub_mode != sl_bt_cs_submode_disabled) {
    // Submode requested
    rtl_err = sl_rtl_cs_get_distance_estimate(&initiator->rtl_handle,
                                              SL_RTL_CS_DISTANCE_ESTIMATE_TYPE_FILTERED,
                                              SL_RTL_CS_SUB_MODE_ESTIMATE,
                                              &last_known_distance);
    show_rtl_api_call_result(initiator, rtl_err);
    if (rtl_err == SL_RTL_ERROR_SUCCESS) {
      sc = cs_result_append_field(&initiator->result_data,
                                  CS_RESULT_FIELD_DISTANCE_SUBMODE,
                                  (uint8_t *)&last_known_distance,
                                  initiator->result);
      if (sc != SL_STATUS_OK) {
        initiator_log_error(INSTANCE_PREFIX "RTL - failed to append sub mode distance! [sc: 0x%lx]" LOG_NL,
                            initiator->conn_handle,
                            (unsigned long)sc);
      } else {
        estimation_valid = true;
      }
    } else {
      initiator_log_error(INSTANCE_PREFIX "RTL - failed to get sub mode distance data! [E: 0x%x]" LOG_NL,
                          initiator->conn_handle,
                          rtl_err);
    }
  }

  // --------------------------------
  // Get RAW distance
  rtl_err = sl_rtl_cs_get_distance_estimate(&initiator->rtl_handle,
                                            SL_RTL_CS_DISTANCE_ESTIMATE_TYPE_RAW,
                                            mode,
                                            &rtl_value);
  show_rtl_api_call_result(initiator, rtl_err);
  if (rtl_err == SL_RTL_ERROR_SUCCESS) {
    sc = cs_result_append_field(&initiator->result_data,
                                CS_RESULT_FIELD_DISTANCE_RAW_MAINMODE,
                                (uint8_t *)&rtl_value,
                                initiator->result);
    if (sc != SL_STATUS_OK) {
      initiator_log_error(INSTANCE_PREFIX "RTL - failed to append RAW distance! [sc: 0x%lx]" LOG_NL,
                          initiator->conn_handle,
                          (unsigned long)sc);
    } else {
      estimation_valid = true;
    }
  } else {
    initiator_log_error(INSTANCE_PREFIX "RTL - failed to get RAW distance data! [E: 0x%x]" LOG_NL,
                        initiator->conn_handle,
                        rtl_err);
  }

  if (initiator->config.cs_sub_mode != sl_bt_cs_submode_disabled) {
    rtl_err = sl_rtl_cs_get_distance_estimate(&initiator->rtl_handle,
                                              SL_RTL_CS_DISTANCE_ESTIMATE_TYPE_RAW,
                                              SL_RTL_CS_SUB_MODE_ESTIMATE,
                                              &rtl_value);
    show_rtl_api_call_result(initiator, rtl_err);
    if (rtl_err == SL_RTL_ERROR_SUCCESS) {
      sc = cs_result_append_field(&initiator->result_data,
                                  CS_RESULT_FIELD_DISTANCE_RAW_SUBMODE,
                                  (uint8_t *)&rtl_value,
                                  initiator->result);
      if (sc != SL_STATUS_OK) {
        initiator_log_error(INSTANCE_PREFIX "RTL - failed to append RAW sub mode distance! [sc: 0x%lx]" LOG_NL,
                            initiator->conn_handle,
                            (unsigned long)sc);
      } else {
        estimation_valid = true;
      }
    } else {
      initiator_log_error(INSTANCE_PREFIX "RTL - failed to get RAW sub mode distance data! [E: 0x%x]" LOG_NL,
                          initiator->conn_handle,
                          rtl_err);
    }
  }

  // --------------------------------
  // Get distance likeliness
  rtl_err = sl_rtl_cs_get_distance_estimate_confidence(&initiator->rtl_handle,
                                                       SL_RTL_CS_DISTANCE_ESTIMATE_CONFIDENCE_TYPE_LIKELINESS,
                                                       mode,
                                                       &rtl_value);
  show_rtl_api_call_result(initiator, rtl_err);
  if (rtl_err == SL_RTL_ERROR_SUCCESS) {
    sc = cs_result_append_field(&initiator->result_data,
                                CS_RESULT_FIELD_LIKELINESS_MAINMODE,
                                (uint8_t *)&rtl_value,
                                initiator->result);
    if (sc != SL_STATUS_OK) {
      initiator_log_error(INSTANCE_PREFIX "RTL - failed to append distance likeliness! [sc: 0x%lx]" LOG_NL,
                          initiator->conn_handle,
                          (unsigned long)sc);
    } else {
      estimation_valid = true;
    }
  } else {
    initiator_log_error(INSTANCE_PREFIX "RTL - failed to get distance likeliness! [E: 0x%x]" LOG_NL,
                        initiator->conn_handle,
                        rtl_err);
  }

  if (initiator->config.cs_sub_mode != sl_bt_cs_submode_disabled) {
    rtl_err = sl_rtl_cs_get_distance_estimate_confidence(&initiator->rtl_handle,
                                                         SL_RTL_CS_DISTANCE_ESTIMATE_CONFIDENCE_TYPE_LIKELINESS,
                                                         SL_RTL_CS_SUB_MODE_ESTIMATE,
                                                         &rtl_value);
    show_rtl_api_call_result(initiator, rtl_err);
    if (rtl_err == SL_RTL_ERROR_SUCCESS) {
      sc = cs_result_append_field(&initiator->result_data,
                                  CS_RESULT_FIELD_LIKELINESS_SUBMODE,
                                  (uint8_t *)&rtl_value,
                                  initiator->result);
      if (sc != SL_STATUS_OK) {
        initiator_log_error(INSTANCE_PREFIX "RTL - failed to append sub mode distance likeliness! [sc: 0x%lx]" LOG_NL,
                            initiator->conn_handle,
                            (unsigned long)sc);
      } else {
        estimation_valid = true;
      }
    } else {
      initiator_log_error(INSTANCE_PREFIX "RTL - failed to get sub mode distance likeliness! [E: 0x%x]" LOG_NL,
                          initiator->conn_handle,
                          rtl_err);
    }
  }

  // --------------------------------
  // Get distance RSSI

  // Set reference TX power for RSSI calculation
  param.type = SL_RTL_REF_TX_POWER;
  param.value.ref_tx_power = initiator->config.rssi_ref_tx_power;
  rtl_err = sl_rtl_cs_set_estimator_param(&initiator->rtl_handle, &param);
  if (rtl_err != SL_RTL_ERROR_SUCCESS) {
    initiator_log_error(INSTANCE_PREFIX "RTL - failed to set RSSI reference TX power! [E: 0x%x]" LOG_NL,
                        initiator->conn_handle,
                        rtl_err);
  }

  rtl_err = sl_rtl_cs_get_distance_estimate(&initiator->rtl_handle,
                                            SL_RTL_CS_DISTANCE_ESTIMATE_TYPE_RSSI,
                                            mode,
                                            &rtl_value);
  show_rtl_api_call_result(initiator, rtl_err);
  if (rtl_err == SL_RTL_ERROR_SUCCESS) {
    sc = cs_result_append_field(&initiator->result_data,
                                CS_RESULT_FIELD_DISTANCE_RSSI,
                                (uint8_t *)&rtl_value,
                                initiator->result);
    if (sc != SL_STATUS_OK) {
      initiator_log_error(INSTANCE_PREFIX "RTL - failed to append RSSI distance! [sc: 0x%lx]" LOG_NL,
                          initiator->conn_handle,
                          (unsigned long)sc);
    } else {
      estimation_valid = true;
      param.type = SL_RTL_LAST_KNOWN_DISTANCE;
      param.value.last_known_distance = last_known_distance;
      rtl_err = sl_rtl_cs_set_estimator_param(&initiator->rtl_handle, &param);
      show_rtl_api_call_result(initiator, rtl_err);
      if (rtl_err != SL_RTL_ERROR_SUCCESS) {
        initiator_log_error(INSTANCE_PREFIX "RTL - failed to set last known distance! [E: 0x%x]" LOG_NL,
                            initiator->conn_handle,
                            rtl_err);
      }
    }
  } else {
    initiator_log_error(INSTANCE_PREFIX "RTL - failed to get RSSI distance! [E: 0x%x]" LOG_NL,
                        initiator->conn_handle,
                        rtl_err);
  }

  // --------------------------------
  // Get velocity
  if (initiator->rtl_config.algo_mode == SL_RTL_CS_ALGO_MODE_REAL_TIME_FAST
      && initiator->config.cs_main_mode == sl_bt_cs_mode_pbr
      && (initiator->config.channel_map_preset == CS_CHANNEL_MAP_PRESET_HIGH
          || initiator->config.channel_map_preset == CS_CHANNEL_MAP_PRESET_MEDIUM)) {
    rtl_err = sl_rtl_cs_get_distance_estimate(&initiator->rtl_handle,
                                              SL_RTL_CS_DISTANCE_ESTIMATE_TYPE_VELOCITY,
                                              mode,
                                              &rtl_value);
    show_rtl_api_call_result(initiator, rtl_err);
    if (rtl_err == SL_RTL_ERROR_SUCCESS) {
      sc = cs_result_append_field(&initiator->result_data,
                                  CS_RESULT_FIELD_VELOCITY_MAINMODE,
                                  (uint8_t *)&rtl_value,
                                  initiator->result);
      if (sc != SL_STATUS_OK) {
        initiator_log_error(INSTANCE_PREFIX "RTL - failed to append velocity! [sc: 0x%lx]" LOG_NL,
                            initiator->conn_handle,
                            (unsigned long)sc);
      } else {
        estimation_valid = true;
      }
    } else {
      initiator_log_error(INSTANCE_PREFIX "RTL - failed to get velocity! [E: 0x%x]" LOG_NL,
                          initiator->conn_handle,
                          rtl_err);
    }
  }

  // --------------------------------
  // Get bit error rate - RTT only
  if (initiator->config.cs_main_mode == sl_bt_cs_mode_rtt) {
    rtl_err = sl_rtl_cs_get_distance_estimate_confidence(&initiator->rtl_handle,
                                                         SL_RTL_CS_DISTANCE_ESTIMATE_CONFIDENCE_TYPE_BIT_ERROR_RATE,
                                                         mode,
                                                         &rtl_value);
    show_rtl_api_call_result(initiator, rtl_err);
    if (rtl_err == SL_RTL_ERROR_SUCCESS) {
      sc = cs_result_append_field(&initiator->result_data,
                                  CS_RESULT_FIELD_BIT_ERROR_RATE,
                                  (uint8_t *)&rtl_value,
                                  initiator->result);
      if (sc != SL_STATUS_OK) {
        initiator_log_error(INSTANCE_PREFIX "RTL - failed to append BER! [sc: 0x%lx]" LOG_NL,
                            initiator->conn_handle,
                            (unsigned long)sc);
      } else {
        estimation_valid = true;
      }
    } else {
      initiator_log_error(INSTANCE_PREFIX "RTL - failed to get BER! [E: 0x%x]" LOG_NL,
                          initiator->conn_handle,
                          rtl_err);
    }
  }

  if (estimation_valid && initiator->result_cb != NULL) {
    cs_initiator_report(CS_INITIATOR_REPORT_ESTIMATION_END);
    // Copy results
    initiator->ranging_data_result.num_steps = initiator->data.num_steps;
    initiator->ranging_data_result.step_channels
      = &initiator->data.step_channels[0];
    initiator->ranging_data_result.initiator.ranging_data_size
      = initiator->data.initiator.ranging_data_size;
    initiator->ranging_data_result.initiator.ranging_data
      = &initiator->data.initiator.ranging_data[0];
    initiator->ranging_data_result.reflector.ranging_data_size
      = initiator->data.reflector.ranging_data_size;
    initiator->ranging_data_result.reflector.ranging_data
      = &initiator->data.reflector.ranging_data[0];

    // Call result callback in case of successful process call
    initiator->result_cb(initiator->conn_handle,
                         initiator->ranging_counter,
                         initiator->result,
                         &initiator->result_data,
                         &initiator->ranging_data_result,
                         NULL);
  }
}

/******************************************************************************
 * Handle progressive RTL process, and get intermediate result.
 *
 * @param[in] initiator initiator instance.
 *****************************************************************************/
static void report_intermediate_result(cs_initiator_t *initiator)
{
  enum sl_rtl_error_code rtl_err = SL_RTL_ERROR_NOT_INITIALIZED;
  cs_intermediate_result_t intermediate_result;

  memset(&intermediate_result, 0u, sizeof(intermediate_result));
  intermediate_result.connection = initiator->conn_handle;

  rtl_err =
    sl_rtl_cs_get_distance_estimate_extended_info(&initiator->rtl_handle,
                                                  SL_RTL_CS_DISTANCE_ESTIMATE_EXTENDED_INFO_TYPE_PROGRESS_PERCENTAGE,
                                                  &intermediate_result.progress_percentage);
  show_rtl_api_call_result(initiator, rtl_err);
  if (rtl_err == SL_RTL_ERROR_SUCCESS) {
    initiator_log_info(INSTANCE_PREFIX "RTL - got estimation state" LOG_NL, initiator->conn_handle);
    if (initiator->intermediate_result_cb != NULL) {
      // Call intermediate result callback in case of in process call
      initiator->intermediate_result_cb(&intermediate_result, NULL);
    }
  } else {
    initiator_log_error(INSTANCE_PREFIX "RTL - failed to get estimation extended info! [E: 0x%x]" LOG_NL,
                        initiator->conn_handle,
                        rtl_err);
  }
  initiator_log_info(INSTANCE_PREFIX "RTL - estimation in progress" LOG_NL,
                     initiator->conn_handle);
}

static void cs_mode_converter(sl_bt_cs_mode_t cs_mode_bt,
                              char **cs_mode_str,
                              sl_rtl_cs_mode *cs_mode_rtl)
{
  switch (cs_mode_bt) {
    case sl_bt_cs_mode_rtt:
      *cs_mode_str = "RTT";
      *cs_mode_rtl = SL_RTL_CS_MODE_RTT;
      break;
    case sl_bt_cs_mode_pbr:
      *cs_mode_str = "PBR";
      *cs_mode_rtl = SL_RTL_CS_MODE_PBR;
      break;
    case sl_bt_cs_submode_disabled:
      *cs_mode_str = "disabled";
      *cs_mode_rtl = SL_RTL_CS_MODE_NONE;
      break;
    default:
      *cs_mode_str = "unknown";
      *cs_mode_rtl = SL_RTL_CS_MODE_NONE;
      break;
  }
}

// -----------------------------------------------------------------------------
// Internal function definitions

/******************************************************************************
 * RTL Lib init function
 *****************************************************************************/
enum sl_rtl_error_code rtl_library_init(const uint8_t conn_handle,
                                        sl_rtl_cs_libitem *handle,
                                        rtl_config_t      *config,
                                        uint8_t           *instance_id)
{
  enum sl_rtl_error_code rtl_err;

  if (handle != NULL && *handle != NULL) {
    rtl_err = sl_rtl_cs_deinit(handle);
    if (rtl_err != SL_RTL_ERROR_SUCCESS) {
      initiator_log_error(INSTANCE_PREFIX "RTL - failed to deinit lib! "
                                          "[E: 0x%x]" LOG_NL,
                          conn_handle,
                          rtl_err);
      return rtl_err;
    }
    *handle = NULL;
  }

  rtl_err = sl_rtl_cs_init(handle);
  if (rtl_err != SL_RTL_ERROR_SUCCESS) {
    initiator_log_error(INSTANCE_PREFIX "RTL - failed to init lib! "
                                        "[E: 0x%x]" LOG_NL,
                        conn_handle,
                        rtl_err);
    return rtl_err;
  }

  rtl_err = sl_rtl_cs_log_get_instance_id(handle, instance_id);
  if (rtl_err != SL_RTL_ERROR_SUCCESS) {
    initiator_log_error(INSTANCE_PREFIX "RTL - failed to get instance id! "
                                        "[E: 0x%x]" LOG_NL,
                        conn_handle,
                        rtl_err);
    return rtl_err;
  }

  if (config->rtl_logging_enabled) {
    rtl_err = sl_rtl_cs_log_enable(handle);
    if (rtl_err != SL_RTL_ERROR_SUCCESS) {
      initiator_log_error(INSTANCE_PREFIX "RTL - failed to enable log! "
                                          "[E: 0x%x]" LOG_NL,
                          conn_handle,
                          rtl_err);
      return rtl_err;
    }
  }
  (void)conn_handle;
  return rtl_err;
};

enum sl_rtl_error_code rtl_library_create_estimator(const uint8_t conn_handle,
                                                    sl_rtl_cs_libitem *handle,
                                                    rtl_config_t      *config,
                                                    sl_rtl_cs_params  *cs_parameters,
                                                    const uint8_t     cs_main_mode,
                                                    const uint8_t     cs_sub_mode)
{
  enum sl_rtl_error_code rtl_err;

  rtl_err = sl_rtl_cs_set_algo_mode(handle, config->algo_mode);
  if (rtl_err != SL_RTL_ERROR_SUCCESS) {
    initiator_log_error(INSTANCE_PREFIX "RTL - set algo mode failed! "
                                        "[E: 0x%x]" LOG_NL,
                        conn_handle,
                        rtl_err);
    return rtl_err;
  }
  sl_rtl_cs_mode rtl_main_mode;
  sl_rtl_cs_mode rtl_sub_mode;
  char *main_mode_str;
  char *sub_mode_str;

  cs_mode_converter(cs_main_mode, &main_mode_str, &rtl_main_mode);
  cs_mode_converter(cs_sub_mode, &sub_mode_str, &rtl_sub_mode);

  initiator_log_debug(INSTANCE_PREFIX "CS mode: %s, Submode: %s" LOG_NL,
                      conn_handle,
                      main_mode_str,
                      sub_mode_str);
  rtl_err = sl_rtl_cs_set_cs_mode(handle, rtl_main_mode, rtl_sub_mode);
  if (rtl_err != SL_RTL_ERROR_SUCCESS) {
    initiator_log_error(INSTANCE_PREFIX "RTL - failed to set CS mode and sub mode! "
                                        "[E: 0x%x]" LOG_NL,
                        conn_handle,
                        rtl_err);
    return rtl_err;
  }
  rtl_err = sl_rtl_cs_set_cs_params(handle, cs_parameters);
  if (rtl_err != SL_RTL_ERROR_SUCCESS) {
    initiator_log_error(INSTANCE_PREFIX "RTL - failed to set CS parameters! "
                                        "[err: 0x%x]" LOG_NL,
                        conn_handle,
                        rtl_err);
    return rtl_err;
  }
  initiator_log_debug(INSTANCE_PREFIX "RTL - CS parameters set." LOG_NL, conn_handle);

  initiator_log_debug(INSTANCE_PREFIX "RTL - create estimator" LOG_NL, conn_handle);
  rtl_err = sl_rtl_cs_create_estimator(handle);
  if (rtl_err != SL_RTL_ERROR_SUCCESS) {
    initiator_log_error(INSTANCE_PREFIX "RTL - failed to create estimator! [E: 0x%x]" LOG_NL,
                        conn_handle,
                        rtl_err);
    return rtl_err;
  } else {
    initiator_log_debug(INSTANCE_PREFIX "RTL - estimator created." LOG_NL, conn_handle);
  }
  (void)conn_handle;
  return rtl_err;
}

/******************************************************************************
 * Get number of tones in channel map
 *****************************************************************************/
uint32_t get_num_tones_from_channel_map(const uint8_t  *ch_map,
                                        const uint32_t ch_map_len)
{
  uint8_t current_ch_map;
  uint32_t num_cs_channels = 0;

  if (ch_map == NULL) {
    initiator_log_error("null reference to channel map! Can not get number of tones!" LOG_NL);
    return num_cs_channels;
  } else {
    for (uint32_t ch_map_index = 0; ch_map_index < ch_map_len; ch_map_index++) {
      current_ch_map = ch_map[ch_map_index];
      for (uint8_t current_bit_index = 0;
           current_bit_index < sizeof(uint8_t) * CHAR_BIT;
           current_bit_index++) {
        if (current_ch_map & (1 << current_bit_index)) {
          num_cs_channels++;
        }
      }
    }
  }
  return num_cs_channels;
}

/******************************************************************************
 * Calculate distance between initiator and reflector using RTL library.
 *
 * @param[in] initiator Initiator instance.
 *****************************************************************************/
void calculate_distance(cs_initiator_t *initiator)
{
  enum sl_rtl_error_code rtl_err;
  cs_initiator_report(CS_INITIATOR_REPORT_ESTIMATION_BEGIN);

  // Initiator: Measurement data
  sl_rtl_ras_measurement initiator_measurement = {
    .ranging_data_body
      = (sl_rtl_ras_ranging_data_body *)initiator->data.initiator.ranging_data,
    .ranging_data_body_len = initiator->data.initiator.ranging_data_size
  };

  // Reflector: Measurement data
  sl_rtl_ras_measurement reflector_measurement = {
    .ranging_data_body
      = (sl_rtl_ras_ranging_data_body*)initiator->data.reflector.ranging_data,
    .ranging_data_body_len = initiator->data.reflector.ranging_data_size
  };

  // Create procedure data structure
  sl_rtl_ras_procedure procedure_data = {
    .cs_procedure_config = initiator->cs_procedure_config,
    .ras_info = {
      .num_antenna_paths = initiator->num_antenna_path,
      .num_steps_reported = initiator->data.num_steps,
      .step_channels = initiator->data.step_channels
    },
    .initiator_measurement_type = SL_RTL_RAS,
    .initiator_ras_measurement = &initiator_measurement,
    .reflector_measurement_type = SL_RTL_RAS,
    .reflector_ras_measurement = &reflector_measurement,
  };

  initiator_log_debug(INSTANCE_PREFIX "RTL RAS process start" LOG_NL, initiator->conn_handle);

  // Start estimation
  // Note: procedure count is always 1.
  #if defined (CS_INITIATOR_INTERNAL_SKIP_RTL_PROCESS) && (CS_INITIATOR_INTERNAL_SKIP_RTL_PROCESS == 0)
  rtl_err = sl_rtl_ras_process(&initiator->rtl_handle,
                               1,
                               &procedure_data);

  show_rtl_api_call_result(initiator, rtl_err);
  switch (rtl_err) {
    case SL_RTL_ERROR_SUCCESS:
      report_result(initiator);
      break;
    case SL_RTL_ERROR_ESTIMATION_IN_PROGRESS:
      report_intermediate_result(initiator);
      break;
    default:
      initiator_log_error(INSTANCE_PREFIX "RTL - failed to process CS data! [E: 0x%x]" LOG_NL,
                          initiator->conn_handle,
                          rtl_err);
      // on_error - app will decide what to do with RTL errors e.g start counter
      on_error(initiator,
               CS_ERROR_EVENT_RTL_PROCESS_ERROR,
               rtl_err);
      break;
  }
  #else
  initiator_log_debug(INSTANCE_PREFIX "RTL - RTL process skipped" LOG_NL, initiator->conn_handle);
  // Call result callback in case of successful process call
  initiator->result_cb(initiator->conn_handle,
                       initiator->ranging_counter,
                       NULL,
                       NULL,
                       NULL,
                       NULL);
  #endif // CS_INITIATOR_INTERNAL_SKIP_RTL_PROCESS
}
