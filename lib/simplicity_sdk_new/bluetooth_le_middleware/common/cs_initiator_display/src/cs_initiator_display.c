/***************************************************************************//**
 * @file
 * @brief CS Initiator display
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
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "sl_rtl_clib_api.h"

#include "cs_initiator_display_config.h"
#include "cs_initiator_display_core.h"
#include "cs_initiator_display.h"
#include "cs_initiator_config.h"

// -----------------------------------------------------------------------------
// Macros

#define FONT_TYPE                                ((GLIB_Font_t *)&GLIB_FontNarrow6x8)
#define STRING_LEN                               40

extern cs_initiator_display_instance_t lcd_instance[CS_INITIATOR_MAX_CONNECTIONS];

static sl_status_t cs_initiator_display_measurement_modes(sl_bt_cs_mode_t mode,
                                                          uint8_t algo_mode);
static sl_status_t cs_initiator_display_get_instance(uint8_t conn_handle,
                                                     uint8_t *instance_num);

static void cs_initiator_display_get_status_text(char* text,
                                                 cs_initiator_display_status_t status);

// -----------------------------------------------------------------------------
// Public function definitions

/******************************************************************************
 * CS Initiator display show state.
 *****************************************************************************/
void cs_initiator_display_start_scanning()
{
  for (uint8_t i = 0; i < CS_INITIATOR_MAX_CONNECTIONS; i++) {
    if (lcd_instance[i].status == CS_INITIATOR_DISPLAY_STATUS_UNINITIALIZED) {
      lcd_instance[i].status = CS_INITIATOR_DISPLAY_STATUS_SCANNING;
      cs_initiator_display_write_text_to_instance(CS_INITIATOR_DISPLAY_STATE_SCANNING_TEXT,
                                                  ROW_STATUS_VALUE,
                                                  i);
    }
  }
}

/******************************************************************************
 * CS Initiator display clear every row of specified instance.
 *****************************************************************************/
sl_status_t cs_initiator_display_create_instance(uint8_t conn_handle)
{
  for (int i = 0; i < CS_INITIATOR_MAX_CONNECTIONS; i++) {
    if (lcd_instance[i].connection_id == SL_BT_INVALID_CONNECTION_HANDLE) {
      lcd_instance[i].connection_id = conn_handle;
      lcd_instance[i].status = CS_INITIATOR_DISPLAY_STATUS_UNINITIALIZED;
      memset(&lcd_instance[i].content, 0, sizeof(cs_initiator_display_content_t));
      lcd_instance[i].content.progress_percentage = 100;
      lcd_instance[i].content.bit_error_rate = NAN;
      return SL_STATUS_OK;
    }
  }
  return SL_STATUS_FAIL;
}

/******************************************************************************
 * CS Initiator display update display data.
 *****************************************************************************/
void cs_initiator_display_update_data(uint8_t instance_num,
                                      uint8_t conn_handle,
                                      uint8_t status,
                                      float distance,
                                      float rssi_distance,
                                      float likeliness,
                                      float bit_error_rate,
                                      float raw_distance,
                                      float progress_percentage,
                                      uint8_t algo_mode,
                                      sl_bt_cs_mode_t
                                      cs_mode)
{
  lcd_instance[instance_num].connection_id = conn_handle;
  lcd_instance[instance_num].status = status;
  lcd_instance[instance_num].content.distance = distance;
  lcd_instance[instance_num].content.rssi_distance = rssi_distance;
  lcd_instance[instance_num].content.likeliness = likeliness;
  lcd_instance[instance_num].content.bit_error_rate = bit_error_rate;
  lcd_instance[instance_num].content.raw_distance = raw_distance;
  lcd_instance[instance_num].content.progress_percentage = progress_percentage;
  lcd_instance[instance_num].content.mode = cs_mode;
  lcd_instance[instance_num].content.algo_mode = algo_mode;
}

/******************************************************************************
 * Create new display instance
 *****************************************************************************/
sl_status_t cs_initiator_display_delete_instance(uint8_t conn_handle)
{
  uint8_t instance_num;
  sl_status_t status = cs_initiator_display_get_instance(conn_handle, &instance_num);
  if (status != SL_STATUS_OK) {
    return status;
  }
  cs_initiator_display_clear_instance(instance_num);
  cs_initiator_display_write_text_to_instance(CS_INITIATOR_DISPLAY_STATE_SCANNING_TEXT,
                                              ROW_STATUS_VALUE,
                                              instance_num);
  memset(&lcd_instance[instance_num].content, 0, sizeof(cs_initiator_display_content_t));
  lcd_instance[instance_num].connection_id = SL_BT_INVALID_CONNECTION_HANDLE;
  lcd_instance[instance_num].content.progress_percentage = 100;
  lcd_instance[instance_num].content.bit_error_rate = NAN;
  return SL_STATUS_OK;
}

/******************************************************************************
 * CS initiator display write text to a specified row.
 *****************************************************************************/
void cs_initiator_display_update_instance(uint8_t instance_num)
{
  char text[6] = "\0";
  cs_initiator_display_get_status_text(text, lcd_instance[instance_num].status);
  cs_initiator_display_write_text_to_instance(text, ROW_STATUS_VALUE, instance_num);
  // do not display numbers if the instance is not in connected state
  if (lcd_instance[instance_num].status != CS_INITIATOR_DISPLAY_STATUS_CONNECTED) {
    return;
  }
  cs_initiator_display_write_float_to_instance(lcd_instance[instance_num].content.distance, ROW_DISTANCE_VALUE, instance_num);
  if (lcd_instance[instance_num].content.algo_mode == SL_RTL_CS_ALGO_MODE_STATIC_HIGH_ACCURACY) {
    cs_initiator_display_write_float_to_instance(lcd_instance[instance_num].content.progress_percentage, ROW_RAW_DISTANCE_VALUE, instance_num);
  } else {
    cs_initiator_display_write_float_to_instance(lcd_instance[instance_num].content.raw_distance, ROW_RAW_DISTANCE_VALUE, instance_num);
  }
  cs_initiator_display_write_float_to_instance(lcd_instance[instance_num].content.likeliness, ROW_LIKELINESS_VALUE, instance_num);
  cs_initiator_display_write_float_to_instance(lcd_instance[instance_num].content.rssi_distance, ROW_RSSI_DISTANCE_VALUE, instance_num);
  // BER is not supported in PBR mode
  if (lcd_instance[instance_num].content.mode != sl_bt_cs_mode_pbr) {
    cs_initiator_display_write_float_to_instance(lcd_instance[instance_num].content.bit_error_rate, ROW_BIT_ERROR_RATE_VALUE, instance_num);
  }
}

/******************************************************************************
 * Set the measurement mode and object tracking mode and show on LCD
 *****************************************************************************/
void cs_initiator_display_set_measurement_mode(sl_bt_cs_mode_t mode,
                                               uint8_t algo_mode)
{
  sl_status_t sc = SL_STATUS_OK;

  for (int i = 0; i < CS_INITIATOR_MAX_CONNECTIONS; i++) {
    lcd_instance[i].content.mode = mode;
    lcd_instance[i].content.algo_mode = algo_mode;
  }
  if (algo_mode == SL_RTL_CS_ALGO_MODE_STATIC_HIGH_ACCURACY) {
    cs_initiator_display_write_text_to_instance("               ", ROW_RAW_DISTANCE_TEXT, 0);
    cs_initiator_display_write_text_to_instance(CS_INITIATOR_DISPLAY_PERCENTAGE_TEXT, ROW_RAW_DISTANCE_TEXT, 0);
  }
  sc = cs_initiator_display_measurement_modes(mode, algo_mode);
  if (sc != SL_STATUS_OK) {
    display_log_error("Error during showing measurement mode and "
                      "algo mode on LCD! [E: 0x%x]" NL, sc);
  }
}

// -----------------------------------------------------------------------------
// Private function definitions

/******************************************************************************
 * Get instance number
 *****************************************************************************/
static sl_status_t cs_initiator_display_get_instance(uint8_t conn_handle,
                                                     uint8_t *instance_num)
{
  for (uint8_t i = 0; i < CS_INITIATOR_MAX_CONNECTIONS; i++) {
    if (lcd_instance[i].connection_id == conn_handle) {
      *instance_num = i;
      return SL_STATUS_OK;
    }
  }
  return SL_STATUS_NOT_FOUND;
}

/******************************************************************************
 * Get display status text based on the status
 *****************************************************************************/
static void cs_initiator_display_get_status_text(char* text,
                                                 cs_initiator_display_status_t status)
{
  switch (status) {
    case CS_INITIATOR_DISPLAY_STATUS_CONNECTED:
      strcpy(text, CS_INITIATOR_DISPLAY_STATE_CONNECTED_TEXT);
      break;
    case CS_INITIATOR_DISPLAY_STATUS_SCANNING:
      strcpy(text, CS_INITIATOR_DISPLAY_STATE_SCANNING_TEXT);
      break;
    case CS_INITIATOR_DISPLAY_STATUS_ESTIMATE:
      strcpy(text, CS_INITIATOR_DISPLAY_STATE_ESTIMATE_TEXT);
      break;
    case CS_INITIATOR_DISPLAY_STATUS_INITIALIZED:
      strcpy(text, CS_INITIATOR_DISPLAY_STATE_INITIALIZED_TEXT);
      break;
    case CS_INITIATOR_DISPLAY_STATUS_SPOOFED:
      strcpy(text, CS_INITIATOR_DISPLAY_STATE_SPOOFED_TEXT);
      break;
    case CS_INITIATOR_DISPLAY_STATUS_UNINITIALIZED:
      strcpy(text, CS_INITIATOR_DISPLAY_STATE_UNINITIALIZED_TEXT);
      break;
    default:
      strcpy(text, CS_INITIATOR_DISPLAY_STATE_UNINITIALIZED_TEXT);
      break;
  }
}

/******************************************************************************
 * CS initiator display measurement mode and object tracking mode.
 *****************************************************************************/
static sl_status_t cs_initiator_display_measurement_modes(sl_bt_cs_mode_t mode,
                                                          uint8_t algo_mode)
{
  sl_status_t sc = SL_STATUS_OK;
  char string[STRING_LEN] = "\0";

  if (strncat(string, CS_INITIATOR_DISPLAY_MODE_TEXT, (sizeof(string) - strlen(string) - 1u)) == NULL) {
    display_log_error("Failed to concat \'%s\' string!" NL,
                      CS_INITIATOR_DISPLAY_MODE_TEXT);
    sc = SL_STATUS_FAIL;
  }

  if (mode == sl_bt_cs_mode_rtt) {
    if (strncat(string, CS_INITIATOR_DISPLAY_MODE_RTT_TEXT, (sizeof(string) - strlen(string) - 1u)) == NULL) {
      display_log_error("Failed to concat \'%s\' string!" NL,
                        CS_INITIATOR_DISPLAY_MODE_RTT_TEXT);
      sc = SL_STATUS_FAIL;
    }
  } else {
    if (strncat(string, CS_INITIATOR_DISPLAY_MODE_PBR_TEXT, (sizeof(string) - strlen(string) - 1u)) == NULL) {
      display_log_error("Failed to concat \'%s\' string!" NL,
                        CS_INITIATOR_DISPLAY_MODE_PBR_TEXT);
      sc = SL_STATUS_FAIL;
    }
    cs_initiator_display_clear_row(ROW_BIT_ERROR_RATE_TEXT);
    cs_initiator_display_clear_row(ROW_BIT_ERROR_RATE_VALUE);
  }

  if (algo_mode == SL_RTL_CS_ALGO_MODE_REAL_TIME_BASIC) {
    if (strncat(string, CS_INITIATOR_DISPLAY_AMODE_MOVING_OBJ_TEXT, (sizeof(string) - strlen(string) - 1u)) == NULL) {
      display_log_error("Failed to concat \'%s\' string!" NL,
                        CS_INITIATOR_DISPLAY_AMODE_MOVING_OBJ_TEXT);
      sc = SL_STATUS_FAIL;
    }
  } else if (algo_mode == SL_RTL_CS_ALGO_MODE_REAL_TIME_FAST) {
    if (strncat(string, CS_INITIATOR_DISPLAY_AMODE_MOVING_OBJ_FAST_TEXT, (sizeof(string) - strlen(string) - 1u)) == NULL) {
      display_log_error("Failed to concat \'%s\' string!" NL,
                        CS_INITIATOR_DISPLAY_AMODE_MOVING_OBJ_FAST_TEXT);
      sc = SL_STATUS_FAIL;
    }
  } else {
    if (strncat(string, CS_INITIATOR_DISPLAY_AMODE_STATIONARY_OBJ_TEXT, (sizeof(string) - strlen(string) - 1u)) == NULL) {
      display_log_error("Failed to concat \'%s\' string!" NL,
                        CS_INITIATOR_DISPLAY_AMODE_STATIONARY_OBJ_TEXT);
      sc = SL_STATUS_FAIL;
    }
  }
  cs_initiator_display_write_text_to_instance(string, ROW_MODE, 0);
  return sc;
}
