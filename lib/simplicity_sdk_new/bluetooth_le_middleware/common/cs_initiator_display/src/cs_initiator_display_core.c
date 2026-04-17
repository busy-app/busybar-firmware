/***************************************************************************//**
 * @file
 * @brief CS Initiator display core logic
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

#include "glib.h"
#include "dmd/dmd.h"

#include "cs_initiator_display_config.h"
#include "cs_initiator_display_core.h"
#include "cs_initiator_display.h"
#include "cs_initiator_config.h"

// -----------------------------------------------------------------------------
// Macros

#define DEFAULT_FONT_TYPE                        ((GLIB_Font_t *)&GLIB_FontNarrow6x8)
#define STRING_LEN                               40
#define INSTANCE_LEN                             30

// -----------------------------------------------------------------------------
// Static function declarations

// -----------------------------------------------------------------------------
// Static variables
static GLIB_Context_t glib_context;
static GLIB_Align_t glib_text_alignment;

cs_initiator_display_instance_t lcd_instance[CS_INITIATOR_MAX_CONNECTIONS];

/******************************************************************************
 * CS Initiator display clear specified row.
 *****************************************************************************/
void cs_initiator_display_clear_row(uint8_t row)
{
  const char empty[] = "                    ";
  GLIB_drawStringOnLine(&glib_context,
                        empty,
                        row,
                        GLIB_ALIGN_LEFT,
                        0,
                        -1 * row + 1,
                        true);
}

/******************************************************************************
 * CS Initiator display clear values of display instance.
 *****************************************************************************/
void cs_initiator_display_clear_instance(uint8_t instance)
{
  const char empty[] = CS_INITIATOR_DISPLAY_STATE_UNINITIALIZED_TEXT;
  for (uint8_t row = ROW_STATUS_VALUE; row <= ROW_BIT_ERROR_RATE_VALUE; row += 2) {
    GLIB_drawStringOnLine(&glib_context,
                          empty,
                          row,
                          GLIB_ALIGN_LEFT,
                          instance * INSTANCE_LEN,
                          -1 * row + 1,
                          true);
  }
}

/******************************************************************************
 * CS Initiator display update display content.
 *****************************************************************************/
void cs_initiator_display_update(void)
{
  for (uint8_t i = 0; i < CS_INITIATOR_MAX_CONNECTIONS; i++) {
    cs_initiator_display_update_instance(i);
  }
  DMD_updateDisplay();
}

/******************************************************************************
 * CS Initiator display init.
 *****************************************************************************/
sl_status_t cs_initiator_display_init(void)
{
  EMSTATUS status;
  for (int i = 0; i < CS_INITIATOR_MAX_CONNECTIONS; i++) {
    memset(&lcd_instance[i].content, 0, sizeof(cs_initiator_display_content_t));
    lcd_instance[i].status = CS_INITIATOR_DISPLAY_STATUS_UNINITIALIZED;
    lcd_instance[i].connection_id = SL_BT_INVALID_CONNECTION_HANDLE;
    lcd_instance[i].content.bit_error_rate = NAN;
    lcd_instance[i].content.progress_percentage = 100;
  }

  status = DMD_init(0);
  if (status != DMD_OK) {
    display_log_error("DMD_init failed! [E:0x%x]" NL, status);
    return SL_STATUS_INITIALIZATION;
  }

  status = GLIB_contextInit(&glib_context);
  if (status != GLIB_OK) {
    display_log_error("DMD_init failed! [E:0x%x]" NL, status);
    return SL_STATUS_INITIALIZATION;
  }

  glib_context.backgroundColor = White;
  glib_context.foregroundColor = Black;

  GLIB_setFont(&glib_context, DEFAULT_FONT_TYPE);
  GLIB_clear(&glib_context);

  cs_initiator_display_set_alignment(CS_INITIATOR_DISPLAY_ALIGNMENT_LEFT);
  cs_initiator_display_write_text_to_instance(CS_INITIATOR_DISPLAY_MODE_TEXT, ROW_MODE, 0);
  cs_initiator_display_write_text_to_instance(CS_INITIATOR_DISPLAY_INSTANCE_NUM, ROW_INSTANCE_NUM, 0);
  cs_initiator_display_write_text_to_instance(CS_INITIATOR_DISPLAY_STATUS_TEXT, ROW_STATUS_TEXT, 0);
  cs_initiator_display_write_text_to_instance(CS_INITIATOR_DISPLAY_STATE_UNINITIALIZED_TEXT, ROW_STATUS_VALUE, 0);
  cs_initiator_display_write_text_to_instance(CS_INITIATOR_DISPLAY_DISTANCE_TEXT, ROW_DISTANCE_TEXT, 0);
  cs_initiator_display_write_text_to_instance(CS_INITIATOR_DISPLAY_LIKELINESS_TEXT, ROW_LIKELINESS_TEXT, 0);
  cs_initiator_display_write_text_to_instance(CS_INITIATOR_DISPLAY_RSSI_DISTANCE_TEXT, ROW_RSSI_DISTANCE_TEXT, 0);
  cs_initiator_display_write_text_to_instance(CS_INITIATOR_DISPLAY_BER_TEXT, ROW_BIT_ERROR_RATE_TEXT, 0);
  cs_initiator_display_write_text_to_instance(CS_INITIATOR_DISPLAY_RAW_DISTANCE_TEXT, ROW_RAW_DISTANCE_TEXT, 0);

  // force update LCD content for drawing the first screen
  cs_initiator_display_update();
  return SL_STATUS_OK;
}

/******************************************************************************
 * CS Initiator display set display alignment.
 *****************************************************************************/
void cs_initiator_display_set_alignment(cs_initiator_display_alignment_t alignment)
{
  switch (alignment) {
    case CS_INITIATOR_DISPLAY_ALIGNMENT_LEFT:
      glib_text_alignment = GLIB_ALIGN_LEFT;
      break;

    case CS_INITIATOR_DISPLAY_ALIGNMENT_CENTER:
      glib_text_alignment = GLIB_ALIGN_CENTER;
      break;

    case CS_INITIATOR_DISPLAY_ALIGNMENT_RIGHT:
      glib_text_alignment = GLIB_ALIGN_RIGHT;
      break;
  }
}

/******************************************************************************
 * CS Initiator display write string to a specified row.
 *****************************************************************************/
void cs_initiator_display_write_text_to_instance(char *str,
                                                 uint8_t row,
                                                 uint8_t instance)
{
  if (!strlen(str)) {
    return;
  }
  const char empty[] = CS_INITIATOR_DISPLAY_STATE_UNINITIALIZED_TEXT;
  uint8_t offset = instance * INSTANCE_LEN;
  GLIB_drawStringOnLine(&glib_context,
                        empty,
                        row,
                        glib_text_alignment,
                        offset,
                        -1 * row + 1,
                        true);
  GLIB_drawStringOnLine(&glib_context,
                        str,
                        row,
                        glib_text_alignment,
                        offset,
                        -1 * row + 1,
                        true);
}

/******************************************************************************
 * CS Initiator display write float value to a specified row.
 *****************************************************************************/
void cs_initiator_display_write_float_to_instance(float value,
                                                  uint8_t row,
                                                  uint8_t instance)
{
  uint32_t base;
  uint32_t ext;
  char buffer_temp[5] = "\0";
  const char empty[] = CS_INITIATOR_DISPLAY_STATE_UNINITIALIZED_TEXT;
  base = (uint32_t)truncf(value);
  if (base < 10) {
    ext = (uint32_t)((value - (float)base) * 100);
    sprintf(buffer_temp, " %01lu.%02lu", base, ext);
  } else {
    ext = (uint32_t)((value - (float)base) * 10);
    sprintf(buffer_temp, " %02lu.%01lu", base, ext);
  }
  uint8_t offset = instance * INSTANCE_LEN;
  GLIB_drawStringOnLine(&glib_context,
                        empty,
                        row,
                        glib_text_alignment,
                        offset,
                        -1 * row + 1,
                        true);
  GLIB_drawStringOnLine(&glib_context,
                        buffer_temp,
                        row,
                        glib_text_alignment,
                        offset,
                        -1 * row + 1,
                        true);
}

// -----------------------------------------------------------------------------
// Event / callback declarations

/******************************************************************************
 * Bluetooth stack event handler.
 *****************************************************************************/
void cs_initiator_display_on_event(sl_bt_msg_t *evt)
{
  sl_status_t sc;

  switch (SL_BT_MSG_ID(evt->header)) {
    case sl_bt_evt_connection_opened_id:
      sc = cs_initiator_display_create_instance(evt->data.evt_connection_opened.connection);
      if (sc != SL_STATUS_OK) {
        display_log_error("Failed to create display instance!" NL);
      }
      break;
    case sl_bt_evt_connection_closed_id:
      sc = cs_initiator_display_delete_instance(evt->data.evt_connection_closed.connection);
      if (sc != SL_STATUS_OK) {
        display_log_error("Failed to delete display instance!" NL);
      }
      break;
    // do the same for these 2 events
    case sl_bt_evt_scanner_extended_advertisement_report_id:
    case sl_bt_evt_scanner_legacy_advertisement_report_id:
      break;
    default:
      break;
  }
}
