/***************************************************************************//**
 * @file
 * @brief Routines for the Color Control plugin.
 *******************************************************************************
 * # License
 * <b>Copyright 2018 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

#include "app/framework/include/af.h"
#include "app/framework/util/attribute-storage.h"

#include "zap-cluster-command-parser.h"

/** @brief Is Color Supported
 *
 * This function will be called to determine whether a color is supported by a
 * device. The color will be specified by hue and saturation.
 *
 * @param hue   Ver.: always
 * @param saturation   Ver.: always
 */
bool sl_zigbee_af_color_control_is_color_supported_cb(uint8_t hue,
                                                      uint8_t saturation);

typedef struct {
  bool active;
  uint8_t commandId;
  bool hueMoveDirection; // true for up
  bool satMoveDirection; // true for up
  bool acceleratedHue; // Used for HueAndSat; true if Hue move > Sat move
  uint8_t acceleratedMoveRate;
  uint8_t hueMoveToLevel;
  uint8_t saturationMoveToLevel;
  uint32_t eventDuration; // All time fields in milliseconds
  uint32_t elapsedTime;
  uint32_t transitionTime;
} sli_zigbee_color_control_state_t;

static sli_zigbee_color_control_state_t stateTable[SL_ZIGBEE_ZCL_COLOR_CONTROL_CLUSTER_SERVER_ENDPOINT_COUNT];

static sli_zigbee_color_control_state_t *getColorControlState(uint8_t endpoint);

#ifdef ZCL_USING_COLOR_CONTROL_CLUSTER_COLOR_CONTROL_REMAINING_TIME_ATTRIBUTE
static void colorControlClearRemainingTime(uint8_t endpoint);
#endif // ZCL_USING_COLOR_CONTROL_CLUSTER_COLOR_CONTROL_REMAINING_TIME_ATTRIBUTE

static void colorControlSetHue(uint8_t endpoint, uint8_t hue);
static void colorControlSetSaturation(uint8_t endpoint, uint8_t saturation);
static void colorControlSetColorModeToZero(void);
static sl_zigbee_af_status_t colorControlReadCurrentHue(uint8_t endpoint, uint8_t* hue);
static sl_zigbee_af_status_t colorControlReadCurrentSaturation(uint8_t endpoint,
                                                               uint8_t* saturation);
static bool colorControlHueAndSaturationHandler(uint8_t endpoint);

static sli_zigbee_color_control_state_t *getColorControlState(uint8_t endpoint)
{
  uint8_t index = sl_zigbee_af_find_cluster_server_endpoint_index(endpoint, ZCL_COLOR_CONTROL_CLUSTER_ID);
  return (index == SL_ZIGBEE_AF_INVALID_ENDPOINT_INDEX ? NULL : &stateTable[index]);
}

void sl_zigbee_af_color_control_cluster_server_init_cb(uint8_t endpoint)
{
  sli_zigbee_color_control_state_t *state = getColorControlState(endpoint);
  if (state == NULL) {
    return;
  }

  state->active = false;
#ifdef ZCL_USING_COLOR_CONTROL_CLUSTER_COLOR_CONTROL_REMAINING_TIME_ATTRIBUTE
  colorControlClearRemainingTime(endpoint);
#endif // ZCL_USING_COLOR_CONTROL_CLUSTER_COLOR_CONTROL_REMAINING_TIME_ATTRIBUTE
}

void sl_zigbee_af_color_control_cluster_server_tick_cb(uint8_t endpoint)
{
  sli_zigbee_color_control_state_t *state = getColorControlState(endpoint);
  sl_zigbee_af_status_t status;
  uint8_t hue, sat;
  bool hueUp = state->hueMoveDirection;
  bool satUp = state->satMoveDirection;

  if (state == NULL) {
    return;
  }

  status = colorControlReadCurrentHue(endpoint, &hue);
  if (status != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    sl_zigbee_af_color_control_cluster_println("ERR: could not read current hue %02X",
                                               status);
    return;
  }
  status = colorControlReadCurrentSaturation(endpoint, &sat);
  if (status != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    sl_zigbee_af_color_control_cluster_println("ERR: could not read current saturation %02X",
                                               status);
    return;
  }

  switch (state->commandId) {
    case ZCL_MOVE_TO_HUE_AND_SATURATION_COMMAND_ID:
      state->active = colorControlHueAndSaturationHandler(endpoint);
      break;
    case ZCL_STEP_HUE_COMMAND_ID:
    case ZCL_MOVE_TO_HUE_COMMAND_ID:
    case ZCL_MOVE_HUE_COMMAND_ID:
      hue = hueUp ? hue + 1 : hue - 1;
      if (hue == 0xFF) {
        hue = hueUp ? 0x00 : 0xFE;
      }
      colorControlSetHue(endpoint, hue);
      if (state->commandId != ZCL_MOVE_HUE_COMMAND_ID) {
        state->active = (hue != state->hueMoveToLevel);
      }
      break;
    case ZCL_STEP_SATURATION_COMMAND_ID:
    case ZCL_MOVE_TO_SATURATION_COMMAND_ID:
    case ZCL_MOVE_SATURATION_COMMAND_ID:
      sat = satUp ? sat + 1 : sat - 1;
      if (sat == 0xFF) {
        sat = satUp ? 0xFE : 0x00;
      }
      colorControlSetSaturation(endpoint, sat);
      if (state->commandId != ZCL_MOVE_SATURATION_COMMAND_ID) {
        state->active = (sat != state->saturationMoveToLevel
                         || sat == 0x00
                         || sat == 0xFE);
      }
      break;
    default:
      sl_zigbee_af_color_control_cluster_println("ERR: unknown color control command.");
      return;
  }

#ifdef ZCL_USING_COLOR_CONTROL_CLUSTER_COLOR_CONTROL_REMAINING_TIME_ATTRIBUTE
  {
    uint16_t remainingTime;

    state->elapsedTime = (state->active
                          ? state->elapsedTime + state->eventDuration
                          : state->transitionTime);

    // If we're done, we should clear the remaining time
    if (state->elapsedTime == state->transitionTime) {
      colorControlClearRemainingTime(endpoint);
      return;
    }

    // The remainingTime attribute expects values in tenths of seconds;
    // we maintain in milliseconds
    remainingTime = state->elapsedTime / 100;
    status = sl_zigbee_af_write_server_attribute(endpoint,
                                                 ZCL_COLOR_CONTROL_CLUSTER_ID,
                                                 ZCL_COLOR_CONTROL_REMAINING_TIME_ATTRIBUTE_ID,
                                                 (uint8_t *)&remainingTime,
                                                 ZCL_INT16U_ATTRIBUTE_TYPE);
    if (status != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
      sl_zigbee_af_color_control_cluster_println("ERR: writing remaining time %02X", status);
      return;
    }
  }
#endif // ZCL_USING_COLOR_CONTROL_CLUSTER_COLOR_CONTROL_REMAINING_TIME_ATTRIBUTE

  if (!state->active) {
    return;
  }

  //schedule the next tick
  sl_zigbee_zcl_schedule_server_tick(endpoint,
                                     ZCL_COLOR_CONTROL_CLUSTER_ID,
                                     state->eventDuration);
}

// Clear remaining time
#ifdef ZCL_USING_COLOR_CONTROL_CLUSTER_COLOR_CONTROL_REMAINING_TIME_ATTRIBUTE
static void colorControlClearRemainingTime(uint8_t endpoint)
{
  uint16_t data = 0xFFFF;
  sl_zigbee_af_status_t status = sl_zigbee_af_write_server_attribute(endpoint,
                                                                     ZCL_COLOR_CONTROL_CLUSTER_ID,
                                                                     ZCL_COLOR_CONTROL_REMAINING_TIME_ATTRIBUTE_ID,
                                                                     (uint8_t *)&data,
                                                                     ZCL_INT16U_ATTRIBUTE_TYPE);
  if (status != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    sl_zigbee_af_color_control_cluster_println("ERR: writing remaining time %02X", status);
    return;
  }
}
#endif // ZCL_USING_COLOR_CONTROL_CLUSTER_COLOR_CONTROL_REMAINING_TIME_ATTRIBUTE

// Sets the hue attribute
static void colorControlSetHue(uint8_t endpoint, uint8_t hue)
{
  sl_zigbee_af_status_t status = sl_zigbee_af_write_server_attribute(endpoint,
                                                                     ZCL_COLOR_CONTROL_CLUSTER_ID,
                                                                     ZCL_COLOR_CONTROL_CURRENT_HUE_ATTRIBUTE_ID,
                                                                     (uint8_t *)&hue,
                                                                     ZCL_INT8U_ATTRIBUTE_TYPE);
  if (status != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    sl_zigbee_af_color_control_cluster_println("ERR: writing current hue %02X", status); sl_zigbee_af_color_control_cluster_flush();
    return;
  }
  sl_zigbee_af_debug_println("hue=%02X", hue);
}

// Sets the saturation attribute
static void colorControlSetSaturation(uint8_t endpoint, uint8_t saturation)
{
  sl_zigbee_af_status_t status = sl_zigbee_af_write_server_attribute(endpoint,
                                                                     ZCL_COLOR_CONTROL_CLUSTER_ID,
                                                                     ZCL_COLOR_CONTROL_CURRENT_SATURATION_ATTRIBUTE_ID,
                                                                     (uint8_t *)&saturation,
                                                                     ZCL_INT8U_ATTRIBUTE_TYPE);
  if (status != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    sl_zigbee_af_color_control_cluster_println("ERR: writing current saturation %02X",
                                               status);
    return;
  }
  sl_zigbee_af_debug_println("saturation=%02X", saturation);
}

static void colorControlSetColorModeToZero(void)
{
#ifdef ZCL_USING_COLOR_CONTROL_CLUSTER_COLOR_CONTROL_COLOR_MODE_ATTRIBUTE
  // Set the optional Color Mode attribute to zero as per the spec.  If an
  // error occurs, log it, but ignore it.
  uint8_t colorMode = 0;
  sl_zigbee_af_status_t status = sl_zigbee_af_write_server_attribute(sl_zigbee_af_current_endpoint(),
                                                                     ZCL_COLOR_CONTROL_CLUSTER_ID,
                                                                     ZCL_COLOR_CONTROL_COLOR_MODE_ATTRIBUTE_ID,
                                                                     &colorMode,
                                                                     ZCL_ENUM8_ATTRIBUTE_TYPE);
  if (status != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    sl_zigbee_af_color_control_cluster_println("ERR: writing color mode%02X", status);
  }
  sl_zigbee_af_debug_println("colorMode=%02X", colorMode); sl_zigbee_af_color_control_cluster_flush();
#else
  sl_zigbee_af_debug_println("no color mode attribute"); sl_zigbee_af_color_control_cluster_flush();
#endif //ZCL_USING_COLOR_CONTROL_CLUSTER_COLOR_CONTROL_COLOR_MODE_ATTRIBUTE
}

// returns a ZCL status, SL_ZIGBEE_ZCL_STATUS_SUCCESS when it works. Sets newHue to the
// current Hue attribute value
static sl_zigbee_af_status_t colorControlReadCurrentHue(uint8_t endpoint, uint8_t* hue)
{
  sl_zigbee_af_status_t status = sl_zigbee_af_read_server_attribute(endpoint,
                                                                    ZCL_COLOR_CONTROL_CLUSTER_ID,
                                                                    ZCL_COLOR_CONTROL_CURRENT_HUE_ATTRIBUTE_ID,
                                                                    hue,
                                                                    sizeof(uint8_t));
  if (status != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    sl_zigbee_af_color_control_cluster_println("ERR: reading current hue %02X", status);
  }
  return status;
}

// returns a ZCL status, SL_ZIGBEE_ZCL_STATUS_SUCCESS when it works. Sets newHue to the
// current Hue attribute value
static sl_zigbee_af_status_t colorControlReadCurrentSaturation(uint8_t endpoint,
                                                               uint8_t* saturation)
{
  sl_zigbee_af_status_t status = sl_zigbee_af_read_server_attribute(endpoint,
                                                                    ZCL_COLOR_CONTROL_CLUSTER_ID,
                                                                    ZCL_COLOR_CONTROL_CURRENT_SATURATION_ATTRIBUTE_ID,
                                                                    saturation,
                                                                    sizeof(uint8_t));
  if (status != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    sl_zigbee_af_color_control_cluster_println("ERR: reading current saturation %02X",
                                               status);
  }
  return status;
}

/****************** COMMAND HANDLERS **********************/

static sl_zigbee_af_status_t moveToHueCommandHandler(uint8_t hue,
                                                     uint8_t direction,
                                                     uint16_t transitionTime,
                                                     uint8_t optionsMask,
                                                     uint8_t optionsOverride)
{
  UNUSED_VAR(optionsMask);
  UNUSED_VAR(optionsOverride);

  sli_zigbee_color_control_state_t *state = getColorControlState(sl_zigbee_af_current_endpoint());
  sl_zigbee_af_status_t status;
  uint8_t currentHue, currentSaturation;

  sl_zigbee_af_color_control_cluster_println("ColorControl: MoveToHue (%02X, %02X, %04X)",
                                             hue,
                                             direction,
                                             transitionTime);

  if (state == NULL) {
    status = SL_ZIGBEE_ZCL_STATUS_FAILURE;
    goto send_default_response;
  }

  // If the color specified is not achievable by the hardware, then the
  // color shall not be set and a ZCL default response command shall be
  // generated with status code equal to INVALID_VALUE.
  status = colorControlReadCurrentSaturation(sl_zigbee_af_current_endpoint(),
                                             &currentSaturation);
  if (status != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    goto send_default_response;
  }
  if (!sl_zigbee_af_color_control_is_color_supported_cb(hue, currentSaturation)) {
    status = SL_ZIGBEE_ZCL_STATUS_INVALID_VALUE;
    goto send_default_response;
  }

  status = colorControlReadCurrentHue(sl_zigbee_af_current_endpoint(), &currentHue);
  if (status != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    goto send_default_response;
  }

  // Nothing to do, prevents divide-by-zero
  if ( hue == currentHue ) {
    status = SL_ZIGBEE_ZCL_STATUS_SUCCESS;
    goto send_default_response;
  }

  // As hue is effectively measured on a circle, the new hue may be moved to in
  // either direction.  The direction of hue change is given by the Direction
  // field. If Directionis "Shortest distance," the direction is taken that
  // involves the shortest path round the circle.  This case corresponds to
  // expected normal usage.  If Direction is "Longest distance," the direction
  // is taken that involves the longest path round the circle.  This case can be
  // used for "rainbow effects."  In both cases, if both distances are the same,
  // the Up direction shall be taken.
  switch (direction) {
    case SL_ZIGBEE_ZCL_HUE_DIRECTION_SHORTEST_DISTANCE:
      state->hueMoveDirection = ((uint8_t) (hue - currentHue) <= 127 ? true : false);
      break;
    case SL_ZIGBEE_ZCL_HUE_DIRECTION_LONGEST_DISTANCE:
      state->hueMoveDirection = ((uint8_t) (hue - currentHue) >= 127 ? true : false);
      break;
    case SL_ZIGBEE_ZCL_HUE_DIRECTION_UP:
      state->hueMoveDirection = true;
      break;
    case SL_ZIGBEE_ZCL_HUE_DIRECTION_DOWN:
      state->hueMoveDirection = false;
      break;
    default:
      status = SL_ZIGBEE_ZCL_STATUS_INVALID_FIELD;
      goto send_default_response;
  }

  state->commandId = ZCL_MOVE_TO_HUE_COMMAND_ID;
  state->elapsedTime = 0;
  state->hueMoveToLevel = hue;
  state->transitionTime = transitionTime * MILLISECOND_TICKS_PER_SECOND / 10;
  state->eventDuration = state->transitionTime / ((state->hueMoveDirection)
                                                  ? (hue - currentHue)
                                                  : (currentHue - hue));
  // Set the Color Mode attribute to zero as per the spec, ignoring any errors.
  colorControlSetColorModeToZero();

  // Schedule the next tick
  if (sl_zigbee_zcl_schedule_server_tick(sl_zigbee_af_current_endpoint(),
                                         ZCL_COLOR_CONTROL_CLUSTER_ID,
                                         state->eventDuration)
      != SL_STATUS_OK) {
    status = SL_ZIGBEE_ZCL_STATUS_FAILURE;
  } else {
    status = SL_ZIGBEE_ZCL_STATUS_SUCCESS;
  }

  state->active = true;
  goto send_default_response;

  send_default_response:
  return status;
}

// Move hue to a given hue, taking transitionTime until completed.
sl_zigbee_af_zcl_request_status_t sl_zigbee_af_color_control_cluster_move_to_hue_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_color_control_cluster_move_to_hue_command_t cmd_data;

  if (zcl_decode_color_control_cluster_move_to_hue_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  return moveToHueCommandHandler(cmd_data.hue,
                                 cmd_data.direction,
                                 cmd_data.transitionTime,
                                 cmd_data.optionsMask,
                                 cmd_data.optionsOverride);
}

// Move hue continuously at the given rate. If mode is stop, then stop.
sl_zigbee_af_zcl_request_status_t sl_zigbee_af_color_control_cluster_move_hue_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_color_control_cluster_move_hue_command_t cmd_data;
  uint8_t moveMode;
  uint8_t rate;

  sli_zigbee_color_control_state_t *state = getColorControlState(sl_zigbee_af_current_endpoint());
  sl_zigbee_af_status_t status;

  if (zcl_decode_color_control_cluster_move_hue_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  moveMode = cmd_data.moveMode;
  rate = cmd_data.rate;

  sl_zigbee_af_color_control_cluster_println("ColorControl: MoveHue (%02X, %02X)",
                                             moveMode,
                                             rate);

  if (state == NULL) {
    status = SL_ZIGBEE_ZCL_STATUS_FAILURE;
    goto send_default_response;
  }

  // If the rate is set to 0, then the command shall have no effect
  // and a ZCL default response command shall be generated with status
  // code equal to INVALID_VALUE.

  if (rate == 0) {
    status = SL_ZIGBEE_ZCL_STATUS_INVALID_FIELD;
    goto send_default_response;
  }

  state->active = true;

  switch (moveMode) {
    case SL_ZIGBEE_ZCL_HUE_MOVE_MODE_STOP:
      state->active = false;
      sl_zigbee_zcl_deactivate_server_tick(sl_zigbee_af_current_endpoint(),
                                           ZCL_COLOR_CONTROL_CLUSTER_ID);
#ifdef ZCL_USING_COLOR_CONTROL_CLUSTER_COLOR_CONTROL_REMAINING_TIME_ATTRIBUTE
      colorControlClearRemainingTime(sl_zigbee_af_current_endpoint());
#endif // ZCL_USING_COLOR_CONTROL_CLUSTER_COLOR_CONTROL_REMAINING_TIME_ATTRIBUTE
      status = SL_ZIGBEE_ZCL_STATUS_SUCCESS;
      goto send_default_response;
    case SL_ZIGBEE_ZCL_HUE_MOVE_MODE_UP:
      state->hueMoveDirection = true;
      break;
    case SL_ZIGBEE_ZCL_HUE_MOVE_MODE_DOWN:
      state->hueMoveDirection = false;
      break;
    default:
      status = SL_ZIGBEE_ZCL_STATUS_INVALID_FIELD;
      goto send_default_response;
  }

  state->commandId = ZCL_MOVE_HUE_COMMAND_ID;
  if (state->active) {
    state->eventDuration = MILLISECOND_TICKS_PER_SECOND / rate;
  }

  // Set the Color Mode attribute to zero as per the spec, ignoring any errors.
  colorControlSetColorModeToZero();

  //schedule the next tick
  if (sl_zigbee_zcl_schedule_server_tick(sl_zigbee_af_current_endpoint(),
                                         ZCL_COLOR_CONTROL_CLUSTER_ID,
                                         state->eventDuration)
      != SL_STATUS_OK) {
    status = SL_ZIGBEE_ZCL_STATUS_FAILURE;
  } else {
    status = SL_ZIGBEE_ZCL_STATUS_SUCCESS;
  }

  goto send_default_response;

  send_default_response:
  return status;
}

// Step hue by one step, taking time as specified.
sl_zigbee_af_zcl_request_status_t sl_zigbee_af_color_control_cluster_step_hue_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_color_control_cluster_step_hue_command_t cmd_data;
  uint8_t stepMode;
  uint8_t stepSize;
  uint8_t transitionTime;

  sli_zigbee_color_control_state_t *state = getColorControlState(sl_zigbee_af_current_endpoint());
  sl_zigbee_af_status_t status;
  uint8_t currentHue, currentSaturation;

  if (zcl_decode_color_control_cluster_step_hue_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  stepMode = cmd_data.stepMode;
  stepSize = cmd_data.stepSize;
  transitionTime = cmd_data.transitionTime;

  sl_zigbee_af_color_control_cluster_println("ColorControl: StepHue (%02X, %02X, %02X)",
                                             stepMode,
                                             stepSize,
                                             transitionTime);

  if (state == NULL) {
    status = SL_ZIGBEE_ZCL_STATUS_FAILURE;
    goto send_default_response;
  }

  status = colorControlReadCurrentHue(sl_zigbee_af_current_endpoint(), &currentHue);
  if (status != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    goto send_default_response;
  }

  // Add or subtract the step size to/from the current hue to get the final hue.
  // However, the range for hues is 0x00 to 0xFE, so if we land on 0xFF or if we
  // roll past it, we have to add or subtract one to the final hue, effectively
  // skipping over the invalid 0xFF.
  switch (stepMode) {
    case SL_ZIGBEE_ZCL_HUE_STEP_MODE_UP:
      state->hueMoveToLevel = currentHue + stepSize;
      if (stepSize == 0xFF - currentHue) {
        state->hueMoveToLevel++;
      }
      state->hueMoveDirection = true;
      break;
    case SL_ZIGBEE_ZCL_HUE_STEP_MODE_DOWN:
      state->hueMoveToLevel = currentHue - stepSize;
      if (stepSize == currentHue - 1) {
        state->hueMoveToLevel--;
      }
      state->hueMoveDirection = false;
      break;
    default:
      status = SL_ZIGBEE_ZCL_STATUS_INVALID_FIELD;
      goto send_default_response;
  }

  // If the color specified is not achievable by the hardware, then the
  // color shall not be set and a ZCL default response command shall be
  // generated with status code equal to INVALID_VALUE.
  status = colorControlReadCurrentSaturation(sl_zigbee_af_current_endpoint(),
                                             &currentSaturation);
  if (status != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    goto send_default_response;
  }

  if (!sl_zigbee_af_color_control_is_color_supported_cb(state->hueMoveToLevel,
                                                        currentSaturation)
      || stepSize == 0 ) {
    status = SL_ZIGBEE_ZCL_STATUS_INVALID_VALUE;
    goto send_default_response;
  }

  state->commandId = ZCL_STEP_HUE_COMMAND_ID;
  state->transitionTime = transitionTime * MILLISECOND_TICKS_PER_SECOND / 10;
  state->elapsedTime = 0;
  state->eventDuration = state->transitionTime / stepSize;

  state->active = true;

  // Set the Color Mode attribute to zero as per the spec, ignoring any errors.
  colorControlSetColorModeToZero();

  //schedule the next tick
  if (sl_zigbee_zcl_schedule_server_tick(sl_zigbee_af_current_endpoint(),
                                         ZCL_COLOR_CONTROL_CLUSTER_ID,
                                         state->eventDuration)
      != SL_STATUS_OK) {
    status = SL_ZIGBEE_ZCL_STATUS_FAILURE;
  } else {
    status = SL_ZIGBEE_ZCL_STATUS_SUCCESS;
  }

  goto send_default_response;

  send_default_response:
  return status;
}

// Move saturation to a given saturation, taking transitionTime until completed.
static sl_zigbee_af_status_t moveToSaturationCommandHandler(uint8_t saturation,
                                                            uint16_t transitionTime,
                                                            uint8_t optionsMask,
                                                            uint8_t optionsOverride)
{
  UNUSED_VAR(optionsMask);
  UNUSED_VAR(optionsOverride);

  sli_zigbee_color_control_state_t *state = getColorControlState(sl_zigbee_af_current_endpoint());
  sl_zigbee_af_status_t status;
  uint8_t currentHue, currentSaturation;

  sl_zigbee_af_color_control_cluster_println("ColorControl: MoveToSaturation (%02X, %04X)",
                                             saturation,
                                             transitionTime);

  if (state == NULL) {
    status = SL_ZIGBEE_ZCL_STATUS_FAILURE;
    goto send_default_response;
  }

  // If the color specified is not achievable by the hardware, then the
  // color shall not be set and a ZCL default response command shall be
  // generated with status code equal to INVALID_VALUE.
  status = colorControlReadCurrentHue(sl_zigbee_af_current_endpoint(),
                                      &currentHue);
  if (status != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    goto send_default_response;
  }
  if (!sl_zigbee_af_color_control_is_color_supported_cb(currentHue, saturation)) {
    status = SL_ZIGBEE_ZCL_STATUS_INVALID_VALUE;
    goto send_default_response;
  }

  status = colorControlReadCurrentSaturation(sl_zigbee_af_current_endpoint(),
                                             &currentSaturation);
  if (status != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    goto send_default_response;
  }

  // Nothing to do, prevent divide-by-zero
  if ( saturation == currentSaturation ) {
    status = SL_ZIGBEE_ZCL_STATUS_SUCCESS;
    goto send_default_response;
  }

  state->commandId = ZCL_MOVE_TO_SATURATION_COMMAND_ID;
  state->elapsedTime = 0;
  state->saturationMoveToLevel = saturation;
  state->satMoveDirection =
    (saturation > currentSaturation ? true : false);
  state->eventDuration = state->transitionTime / ((state->satMoveDirection)
                                                  ? (saturation - currentSaturation)
                                                  : (currentSaturation - saturation));

  // Set the Color Mode attribute to zero as per the spec, ignoring any errors.
  colorControlSetColorModeToZero();

  //schedule the next tick
  if (sl_zigbee_zcl_schedule_server_tick(sl_zigbee_af_current_endpoint(),
                                         ZCL_COLOR_CONTROL_CLUSTER_ID,
                                         state->eventDuration)
      != SL_STATUS_OK) {
    status = SL_ZIGBEE_ZCL_STATUS_FAILURE;
  } else {
    status = SL_ZIGBEE_ZCL_STATUS_SUCCESS;
  }

  goto send_default_response;

  send_default_response:
  return status;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_color_control_cluster_move_to_saturation_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_color_control_cluster_move_to_saturation_command_t cmd_data;

  if (zcl_decode_color_control_cluster_move_to_saturation_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  return moveToSaturationCommandHandler(cmd_data.saturation,
                                        cmd_data.transitionTime,
                                        cmd_data.optionsMask,
                                        cmd_data.optionsOverride);
}

// Move sat continuously at the given rate. If mode is stop, then stop.
sl_zigbee_af_zcl_request_status_t sl_zigbee_af_color_control_cluster_move_saturation_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_color_control_cluster_move_saturation_command_t cmd_data;
  uint8_t moveMode;
  uint8_t rate;

  sli_zigbee_color_control_state_t *state = getColorControlState(sl_zigbee_af_current_endpoint());
  sl_zigbee_af_status_t status;

  if (zcl_decode_color_control_cluster_move_saturation_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  moveMode = cmd_data.moveMode;
  rate = cmd_data.rate;

  sl_zigbee_af_color_control_cluster_println("ColorControl: MoveSaturation (%02X, %02X)",
                                             moveMode,
                                             rate);

  if (state == NULL) {
    status = SL_ZIGBEE_ZCL_STATUS_FAILURE;
    goto send_default_response;
  }

  // If the rate is set to 0, then the command shall have no effect
  // and a ZCL default response command shall be generated with status
  // code equal to INVALID_VALUE.

  if (rate == 0) {
    status = SL_ZIGBEE_ZCL_STATUS_INVALID_FIELD;
    goto send_default_response;
  }

  state->active = true;

  switch (moveMode) {
    case SL_ZIGBEE_ZCL_SATURATION_MOVE_MODE_STOP:
      state->active = false;
      sl_zigbee_zcl_deactivate_server_tick(sl_zigbee_af_current_endpoint(),
                                           ZCL_COLOR_CONTROL_CLUSTER_ID);
#ifdef ZCL_USING_COLOR_CONTROL_CLUSTER_COLOR_CONTROL_REMAINING_TIME_ATTRIBUTE
      colorControlClearRemainingTime(sl_zigbee_af_current_endpoint());
#endif // ZCL_USING_COLOR_CONTROL_CLUSTER_COLOR_CONTROL_REMAINING_TIME_ATTRIBUTE
      status = SL_ZIGBEE_ZCL_STATUS_SUCCESS;
      goto send_default_response;
    case SL_ZIGBEE_ZCL_SATURATION_MOVE_MODE_UP:
      state->satMoveDirection = true;
      break;
    case SL_ZIGBEE_ZCL_SATURATION_MOVE_MODE_DOWN:
      state->satMoveDirection = false;
      break;
    default:
      status = SL_ZIGBEE_ZCL_STATUS_INVALID_FIELD;
      goto send_default_response;
  }

  state->commandId = ZCL_MOVE_SATURATION_COMMAND_ID;
  if (state->active) {
    state->eventDuration = MILLISECOND_TICKS_PER_SECOND / rate;
  }

  // Set the Color Mode attribute to zero as per the spec, ignoring any errors.
  colorControlSetColorModeToZero();

  //schedule the next tick
  if (sl_zigbee_zcl_schedule_server_tick(sl_zigbee_af_current_endpoint(),
                                         ZCL_COLOR_CONTROL_CLUSTER_ID,
                                         state->eventDuration)
      != SL_STATUS_OK) {
    status = SL_ZIGBEE_ZCL_STATUS_FAILURE;
  } else {
    status = SL_ZIGBEE_ZCL_STATUS_SUCCESS;
  }
  goto send_default_response;

  send_default_response:
  return status;
}

// Step sat by one step, taking time as specified.
sl_zigbee_af_zcl_request_status_t sl_zigbee_af_color_control_cluster_step_saturation_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_color_control_cluster_step_saturation_command_t cmd_data;
  uint8_t stepMode;
  uint8_t stepSize;
  uint8_t transitionTime;

  sli_zigbee_color_control_state_t *state = getColorControlState(sl_zigbee_af_current_endpoint());
  sl_zigbee_af_status_t status;
  uint8_t currentHue, currentSaturation;

  if (zcl_decode_color_control_cluster_step_saturation_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  stepMode = cmd_data.stepMode;
  stepSize = cmd_data.stepSize;
  transitionTime = cmd_data.transitionTime;

  sl_zigbee_af_color_control_cluster_println("ColorControl: StepSaturation (%02X, %02X, %02X)",
                                             stepMode,
                                             stepSize,
                                             transitionTime);

  if (state == NULL) {
    status = SL_ZIGBEE_ZCL_STATUS_FAILURE;
    goto send_default_response;
  }

  status = colorControlReadCurrentSaturation(sl_zigbee_af_current_endpoint(),
                                             &currentSaturation);
  if (status != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    goto send_default_response;
  }

  // Add or subtract the step size to/from the current saturation to get the
  // final saturation.  However, the range for saturations is 0x00 to 0xFE, so
  // don't increment or decrement past those bounds.
  switch (stepMode) {
    case SL_ZIGBEE_ZCL_SATURATION_STEP_MODE_UP:
      state->saturationMoveToLevel = (stepSize > 0xFE - currentSaturation
                                      ? 0xFE
                                      : currentSaturation + stepSize);
      state->satMoveDirection = true;
      break;
    case SL_ZIGBEE_ZCL_SATURATION_STEP_MODE_DOWN:
      state->saturationMoveToLevel = (stepSize > currentSaturation
                                      ? 0x00
                                      : currentSaturation - stepSize);
      state->satMoveDirection = false;
      break;
    default:
      status = SL_ZIGBEE_ZCL_STATUS_INVALID_FIELD;
      goto send_default_response;
  }

  // If the color specified is not achievable by the hardware, then the
  // color shall not be set and a ZCL default response command shall be
  // generated with status code equal to INVALID_VALUE.
  status = colorControlReadCurrentHue(sl_zigbee_af_current_endpoint(),
                                      &currentHue);
  if (status != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    goto send_default_response;
  }

  if (!sl_zigbee_af_color_control_is_color_supported_cb(currentHue,
                                                        state->saturationMoveToLevel)
      || stepSize == 0 ) {
    status = SL_ZIGBEE_ZCL_STATUS_INVALID_VALUE;
    goto send_default_response;
  }

  state->commandId = ZCL_STEP_SATURATION_COMMAND_ID;
  state->transitionTime = transitionTime * MILLISECOND_TICKS_PER_SECOND / 10;
  state->elapsedTime = 0;
  state->eventDuration = state->transitionTime / stepSize;

  state->active = true;

  // Set the Color Mode attribute to zero as per the spec, ignoring any errors.
  colorControlSetColorModeToZero();

  //schedule the next tick
  if (sl_zigbee_zcl_schedule_server_tick(sl_zigbee_af_current_endpoint(),
                                         ZCL_COLOR_CONTROL_CLUSTER_ID,
                                         state->eventDuration)
      != SL_STATUS_OK) {
    status = SL_ZIGBEE_ZCL_STATUS_FAILURE;
  } else {
    status = SL_ZIGBEE_ZCL_STATUS_SUCCESS;
  }

  goto send_default_response;

  send_default_response:
  return status;
}

// Move hue and saturation to a given values, taking time as specified.
sl_zigbee_af_zcl_request_status_t sl_zigbee_af_color_control_cluster_move_to_hue_and_saturation_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_color_control_cluster_move_to_hue_and_saturation_command_t cmd_data;
  uint8_t hue;
  uint8_t saturation;
  uint16_t transitionTime;

  sli_zigbee_color_control_state_t *state = getColorControlState(sl_zigbee_af_current_endpoint());
  sl_zigbee_af_status_t status;
  uint8_t currentSaturation, currentHue, hueDiff, satDiff;

  if (zcl_decode_color_control_cluster_move_to_hue_and_saturation_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  hue = cmd_data.hue;
  saturation = cmd_data.saturation;
  transitionTime = cmd_data.transitionTime;

  sl_zigbee_af_color_control_cluster_println("ColorControl: MoveToHueAndSaturation (%02X, %02X, %04X)",
                                             hue,
                                             saturation,
                                             transitionTime);

  if (state == NULL) {
    status = SL_ZIGBEE_ZCL_STATUS_FAILURE;
    goto send_default_response;
  }

  // If the color specified is not achievable by the hardware, then the
  // color shall not be set and a ZCL default response command shall be
  // generated with status code equal to INVALID_VALUE.
  if (!sl_zigbee_af_color_control_is_color_supported_cb(hue, saturation)) {
    status = SL_ZIGBEE_ZCL_STATUS_INVALID_VALUE;
    goto send_default_response;
  }

  status = colorControlReadCurrentSaturation(sl_zigbee_af_current_endpoint(),
                                             &currentSaturation);
  if (status != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    goto send_default_response;
  }

  status = colorControlReadCurrentHue(sl_zigbee_af_current_endpoint(), &currentHue);
  if (status != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    goto send_default_response;
  }

  state->commandId = ZCL_MOVE_TO_HUE_AND_SATURATION_COMMAND_ID;
  state->hueMoveToLevel = hue;
  state->hueMoveDirection = (hue - currentHue <= 127 ? true : false);
  hueDiff = (state->hueMoveDirection ? hue - currentHue : currentHue - hue);

  state->saturationMoveToLevel = saturation;
  state->satMoveDirection = (saturation - currentSaturation <= 127 ? true : false);
  satDiff = (state->satMoveDirection
             ? saturation - currentSaturation
             : currentSaturation - saturation);

  if (hueDiff == 0) {
    return moveToSaturationCommandHandler(saturation,
                                          transitionTime,
                                          0x00,  // optionsMask
                                          0x00); // optionsOverride
  } else if (satDiff == 0) {
    return moveToHueCommandHandler(hue,
                                   SL_ZIGBEE_ZCL_HUE_DIRECTION_SHORTEST_DISTANCE,
                                   transitionTime,
                                   0x00,  // optionsMask
                                   0x00); // optionsOverride
  }

  state->elapsedTime = 0;
  state->acceleratedHue = hueDiff > satDiff;
  state->acceleratedMoveRate = (state->acceleratedHue
                                ? hueDiff / satDiff
                                : satDiff / hueDiff);

  state->transitionTime = transitionTime * MILLISECOND_TICKS_PER_SECOND / 10;
  state->eventDuration = state->transitionTime / ((state->acceleratedHue)
                                                  ? hueDiff
                                                  : satDiff);

  // Set the Color Mode attribute to zero as per the spec, ignoring any errors.
  colorControlSetColorModeToZero();

  state->active = true;

  //schedule the next tick
  if (sl_zigbee_zcl_schedule_server_tick(sl_zigbee_af_current_endpoint(),
                                         ZCL_COLOR_CONTROL_CLUSTER_ID,
                                         state->eventDuration)
      != SL_STATUS_OK) {
    status = SL_ZIGBEE_ZCL_STATUS_FAILURE;
  } else {
    status = SL_ZIGBEE_ZCL_STATUS_SUCCESS;
  }

  goto send_default_response;

  send_default_response:
  return status;
}

static bool colorControlHueAndSaturationHandler(uint8_t endpoint)
{
  sl_zigbee_af_status_t status;
  sli_zigbee_color_control_state_t *state = getColorControlState(endpoint);
  uint8_t hue, sat, hueDiff, satDiff;
  bool hueUp = state->hueMoveDirection;
  bool satUp = state->satMoveDirection;
  bool proceed = true;

  if (state == NULL) {
    return false;
  }

  status = colorControlReadCurrentHue(endpoint, &hue);
  if (status != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    sl_zigbee_af_color_control_cluster_println("ERR: could not read current hue %02X",
                                               status);
    return false;
  }
  status = colorControlReadCurrentSaturation(endpoint, &sat);
  if (status != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    sl_zigbee_af_color_control_cluster_println("ERR: could not read current saturation %02X",
                                               status);
    return false;
  }

  // Calculate the hue and saturation differentials
  hueDiff = (hue > state->hueMoveToLevel)
            ? (0xFF - (hue - state->hueMoveToLevel))
            : (state->hueMoveToLevel - hue);
  satDiff = (sat > state->saturationMoveToLevel)
            ? (0xFF - (sat - state->saturationMoveToLevel))
            : (state->saturationMoveToLevel - sat);

  // See where we are relative to our expected correction threshold
  if (state->acceleratedHue) {
    if ( satDiff == 0 ) {
      return false;
    }
    hueDiff = hueDiff / satDiff;
    if (hueDiff > state->acceleratedMoveRate) {
      hueDiff = state->acceleratedMoveRate - 1;
    } else {
      hueDiff = state->acceleratedMoveRate + 1;
    }

    sat = satUp ? sat + 1 : sat - 1;
    if (sat == 0xFF) {
      sat = satUp ? 0x00 : 0xFE;
    }
    // Are we finished?
    if (sat == state->saturationMoveToLevel) {
      hue = state->hueMoveToLevel;
      proceed = false;
      goto set_values;
    }
    hue = hueUp ? hue + hueDiff : hue - hueDiff;
    if (hue == 0xFF) {
      hue = hueUp ? 0x00 : 0xFE;
    }
  } else {
    if ( hueDiff == 0 ) {
      return false;
    }
    satDiff = satDiff / hueDiff;
    if (satDiff > state->acceleratedMoveRate) {
      satDiff = state->acceleratedMoveRate - 1;
    } else {
      satDiff = state->acceleratedMoveRate + 1;
    }

    hue = hueUp ? hue + 1 : hue - 1;
    if (hue == 0xFF) {
      hue = hueUp ? 0x00 : 0xFE;
    }
    // Are we finished?
    if (hue == state->hueMoveToLevel) {
      sat = state->saturationMoveToLevel;
      proceed = false;
      goto set_values;
    }
    sat = satUp ? sat + satDiff : sat - satDiff;
    if (sat == 0xFF) {
      sat = satUp ? 0x00 : 0xFE;
    }
  }

  set_values:
  colorControlSetHue(endpoint, hue);
  colorControlSetSaturation(endpoint, sat);
  return proceed;
}

uint32_t sl_zigbee_af_color_control_cluster_server_command_parse(sl_service_opcode_t opcode,
                                                                 sl_service_function_context_t *context)
{
  (void)opcode;

  sl_zigbee_af_cluster_command_t *cmd = (sl_zigbee_af_cluster_command_t *)context->data;
  sl_zigbee_af_zcl_request_status_t status = SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;

  if (!cmd->mfgSpecific) {
    switch (cmd->commandId) {
      case ZCL_MOVE_TO_HUE_COMMAND_ID:
      {
        status = sl_zigbee_af_color_control_cluster_move_to_hue_cb(cmd);
        break;
      }
      case ZCL_MOVE_HUE_COMMAND_ID:
      {
        status = sl_zigbee_af_color_control_cluster_move_hue_cb(cmd);
        break;
      }
      case ZCL_STEP_HUE_COMMAND_ID:
      {
        status = sl_zigbee_af_color_control_cluster_step_hue_cb(cmd);
        break;
      }
      case ZCL_MOVE_TO_SATURATION_COMMAND_ID:
      {
        status = sl_zigbee_af_color_control_cluster_move_to_saturation_cb(cmd);
        break;
      }
      case ZCL_MOVE_SATURATION_COMMAND_ID:
      {
        status = sl_zigbee_af_color_control_cluster_move_saturation_cb(cmd);
        break;
      }
      case ZCL_STEP_SATURATION_COMMAND_ID:
      {
        status = sl_zigbee_af_color_control_cluster_step_saturation_cb(cmd);
        break;
      }
      case ZCL_MOVE_TO_HUE_AND_SATURATION_COMMAND_ID:
      {
        status = sl_zigbee_af_color_control_cluster_move_to_hue_and_saturation_cb(cmd);
        break;
      }
    }
  }

  return status;
}
