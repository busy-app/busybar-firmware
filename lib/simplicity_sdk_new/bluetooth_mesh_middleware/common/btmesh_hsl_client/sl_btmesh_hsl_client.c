/***************************************************************************//**
 * @file
 * @brief Bt Mesh HSL Client module
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
#include <stdbool.h>
#include "sl_status.h"

#include "sl_btmesh_api.h"
#include "sl_bt_api.h"
#include "sl_btmesh_dcd.h"

#include "sl_btmesh_generic_model_capi_types.h"
#include "sl_btmesh_lib.h"
#include "app_btmesh_rta.h"

#include "app_assert.h"
#include "app_timer.h"
#include "sl_btmesh_lighting_client.h"

#include "sl_btmesh_hsl_client_config.h"
#include "sl_btmesh_hsl_client.h"

// Warning! The app_btmesh_util shall be included after the component configuration
// header file in order to provide the component specific logging macro.
#include "app_btmesh_util.h"

/// No flags used for message
#define NO_FLAGS                  0
/// Immediate transition time is 0 seconds
#define IMMEDIATE                 0
/// Callback has no parameters
#define NO_CALLBACK_DATA          NULL
/// High Priority
#define HIGH_PRIORITY             0

/// Maximum lightness percentage value
#define LIGHTNESS_PCT_MAX         100
/// Maximum saturation percentage value
#define SATURATION_PCT_MAX        100
/// Maximum hue percentage value
#define HUE_PCT_MAX               100
/// Maximum lightness
#define LIGHTNESS_MAX             0xffff
/// Maximum saturation
#define SATURATION_MAX            0xffff
/// Maximum hue
#define HUE_MAX                   0xffff
/// Maximum hue degree
#define HUE_DEGREE_MAX            360
/// Saturation and Lightness level initial value
#define LEVEL_INIT                0
/// Delay unit value for request for hsl messages in millisecond
#define REQ_DELAY_MS              50

/// hue degree converted from percentage to actual value, range 0..65535
static uint16_t hue_level = LEVEL_INIT;
/// saturation level converted from percentage to actual value, range 0..65535
static uint16_t saturation_level = LEVEL_INIT;
/// lightness level converted from percentage to actual value, range 0..65535
static uint16_t lightness_level = LEVEL_INIT;

/// number of hsl requests to be sent
static uint8_t hsl_request_count = 0;
/// hsl transaction identifier
static uint8_t hsl_trid = 0;

/// periodic timer handle
static app_timer_t hsl_retransmission_timer;
/// periodic timer callback
static void  hsl_retransmission_timer_cb(app_timer_t *handle,
                                         void *data);

/*******************************************************************************
 * This function publishes one light HSL request to change the saturation and
 * hue level of light(s) in the group. Global variables saturation_level and
 * hue_level hold the latest desired light saturation and hue level.
 * The HSL request also send lightness_level which holds the latest desired light
 * lightness level.
 *
 * @param[in] retrans Indicates if this is the first request or a retransmission,
 *                    possible values are 0 = first request, 1 = retransmission.
 *
 * @note This application sends multiple hsl requests for each
 *       medium button press to improve reliability.
 *       The transaction ID is not incremented in case of a retransmission.
 ******************************************************************************/
static void send_hsl_request(uint8_t retrans)
{
  struct mesh_generic_request req;
  sl_status_t sc;

  req.kind = mesh_lighting_request_hsl;
  req.hsl.hue = hue_level;
  req.hsl.saturation = saturation_level;
  req.hsl.lightness = lightness_level;

  // Increment transaction ID for each request, unless it's a retransmission
  if (retrans == 0) {
    hsl_trid++;
  }

  // Delay for the request is calculated so that the last request will have
  // a zero delay and each of the previous request have delay that increases
  // in 50 ms steps. For example, when using three hsl requests
  // per button press the delays are set as 100, 50, 0 ms
  uint16_t delay = (hsl_request_count - 1) * REQ_DELAY_MS;

  sc = mesh_lib_generic_client_publish(MESH_LIGHTING_HSL_CLIENT_MODEL_ID,
                                       BTMESH_HSL_CLIENT_MAIN,
                                       hsl_trid,
                                       &req,
                                       IMMEDIATE,     // transition
                                       delay,
                                       NO_FLAGS       // flags
                                       );

  if (sc == SL_STATUS_OK) {
    log_info("HSL request sent, trid = %u, delay = %u" NL, hsl_trid, delay);
  } else {
    log_btmesh_status_f(sc, "HSL Client Publish failed" NL);
  }

  // Keep track of how many requests has been sent
  if (hsl_request_count > 0) {
    hsl_request_count--;
  }
}

/*******************************************************************************
 * This function changes the hue and send it to the server.
 *
 * @param[in] hue_percent Defines new hue value as percentage
 *                        Valid values 0-100 %
 ******************************************************************************/
void sl_btmesh_hsl_client_set_hue(uint8_t hue_percent)
{
  // Adjust light hue, using Light HSL model
  if (hue_percent > HUE_PCT_MAX) {
    return;
  }

  sl_status_t sc = app_btmesh_rta_acquire();
  if (sc != SL_STATUS_OK) {
    return;
  }

  hue_level = hue_percent * HUE_MAX / HUE_PCT_MAX;
  uint16_t hue_degree = HUE_DEGREE_MAX * hue_level / HUE_MAX;
  log("Set hue to %u %% / %udeg" NL, hue_percent, hue_degree);

  // Request is sent multiple times to improve reliability
  hsl_request_count = SL_BTMESH_HSL_CLIENT_RETRANSMISSION_COUNT_CFG_VAL;

  send_hsl_request(0);  //Send the first request

  // If there are more requests to send, start a repeating soft timer
  // to trigger retransmission of the request after 50 ms delay
  if (hsl_request_count > 0) {
    sc = app_timer_start(&hsl_retransmission_timer,
                         SL_BTMESH_HSL_CLIENT_RETRANSMISSION_TIMEOUT_CFG_VAL,
                         hsl_retransmission_timer_cb,
                         NO_CALLBACK_DATA,
                         true);
    app_assert_status_f(sc, "Failed to start periodic timer");
  }

  (void) app_btmesh_rta_release();
}

/*******************************************************************************
 * This function changes the saturation and send it to the server.
 *
 * @param[in] saturation_percent Defines new saturation value as percentage
 *                               Valid values 0-100 %
 ******************************************************************************/
void sl_btmesh_hsl_client_set_saturation(uint8_t saturation_percent)
{
  // Adjust saturation, using Light HSL model
  if (saturation_percent > SATURATION_PCT_MAX) {
    return;
  }

  sl_status_t sc = app_btmesh_rta_acquire();
  if (sc != SL_STATUS_OK) {
    return;
  }

  // Using square of percentage to change temperature more uniformly
  // just for demonstration
  saturation_level = saturation_percent * SATURATION_MAX / SATURATION_PCT_MAX;
  log("Set saturation to %u %% / level %u K" NL, saturation_percent, saturation_level);

  // Request is sent multiple times to improve reliability
  hsl_request_count = SL_BTMESH_HSL_CLIENT_RETRANSMISSION_COUNT_CFG_VAL;

  send_hsl_request(0);  //Send the first request

  // If there are more requests to send, start a repeating soft timer
  // to trigger retransmission of the request after 50 ms delay
  if (hsl_request_count > 0) {
    sc = app_timer_start(&hsl_retransmission_timer,
                         SL_BTMESH_HSL_CLIENT_RETRANSMISSION_TIMEOUT_CFG_VAL,
                         hsl_retransmission_timer_cb,
                         NO_CALLBACK_DATA,
                         true);
    app_assert_status_f(sc, "Failed to start periodic timer");
  }

  (void) app_btmesh_rta_release();
}

/*******************************************************************************
 * This function changes the lightness and send it to the server.
 *
 * @param[in] lightness_percent Defines new lightness value as percentage
 *                              Valid values 0-100 %
 ******************************************************************************/
void sl_btmesh_hsl_client_set_lightness(uint8_t lightness_percent)
{
  // Adjust lightness, using Light HSL model
  if (lightness_percent > LIGHTNESS_PCT_MAX) {
    return;
  }

  sl_status_t sc = app_btmesh_rta_acquire();
  if (sc != SL_STATUS_OK) {
    return;
  }

  lightness_level = LIGHTNESS_MAX * lightness_percent / LIGHTNESS_PCT_MAX;
  log("Set HSL lightness to %u %% / level %u K" NL, lightness_percent, lightness_level);

  // Request is sent multiple times to improve reliability
  hsl_request_count = SL_BTMESH_HSL_CLIENT_RETRANSMISSION_COUNT_CFG_VAL;

  send_hsl_request(0);  //Send the first request

  // If there are more requests to send, start a repeating soft timer
  // to trigger retransmission of the request after 50 ms delay
  if (hsl_request_count > 0) {
    sc = app_timer_start(&hsl_retransmission_timer,
                         SL_BTMESH_HSL_CLIENT_RETRANSMISSION_TIMEOUT_CFG_VAL,
                         hsl_retransmission_timer_cb,
                         NO_CALLBACK_DATA,
                         true);
    app_assert_status_f(sc, "Failed to start periodic timer");
  }

  (void) app_btmesh_rta_release();
}

/*******************************************************************************
 * Timer Callbacks
 * @param[in] handle pointer to handle instance
 * @param[in] data pointer to input data
 ******************************************************************************/
static void  hsl_retransmission_timer_cb(app_timer_t *handle,
                                         void *data)
{
  (void)data;
  (void)handle;

  sl_status_t sc = app_btmesh_rta_acquire();
  if (sc != SL_STATUS_OK) {
    return;
  }

  send_hsl_request(1);   // Retransmit hsl message
  // Stop retransmission timer if it was the last attempt
  if (hsl_request_count == 0) {
    sc = app_timer_stop(&hsl_retransmission_timer);
    app_assert_status_f(sc, "Failed to stop periodic timer");
  }

  (void) app_btmesh_rta_release();
}
