/***************************************************************************//**
 * @file
 * @brief ESL Tag demo component logic.
 *******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
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
#include <stdio.h>
#include "esl_tag_core.h"
#include "esl_tag_wstk_lcd_driver.h"
#include "glib.h"

#define GLIB_X_START 10
#define GLIB_Y_START_GROUPID 70
#define GLIB_Y_START_ESLID 54
#define GLIB_Y_START_ERR 62
#define GLIB_CLIP_START_Y 44
#define GLIB_TEXT_WIDTH 127
#define GLIB_TEXT_HEIGHT 40

static void esl_tag_demo_draw_rectangle(GLIB_Context_t *context, GLIB_Rectangle_t coords)
{
  int32_t x, y;

  // Reduce draw area by 1-1 pixels on each side to leave a border
  coords.xMin += 2;
  coords.yMin += 2;
  coords.xMax -= 2;
  coords.yMax -= 2;

  // Draw 1 pixel less on each end to create a 1px bevel effect
  for (x = coords.xMin + 1; x <= coords.xMax - 1; x++) {
    GLIB_drawPixel(context, x, coords.yMin);
    GLIB_drawPixel(context, x, coords.yMax);
  }

  for (y = coords.yMin + 1; y <= coords.yMax - 1; y++) {
    GLIB_drawPixel(context, coords.xMin, y);
    GLIB_drawPixel(context, coords.xMax, y);
  }
}

void esl_core_update_complete_callback(void)
{
  GLIB_Context_t context;
  esl_address_t address;
  static const char * error_msg = "Not configured!";
  char msg[] = "Group ID: % 3d";
  GLIB_Rectangle_t clip_rect = { 0,
                                 GLIB_CLIP_START_Y,
                                 GLIB_TEXT_WIDTH,
                                 GLIB_CLIP_START_Y + GLIB_TEXT_HEIGHT };
  sl_status_t status = esl_core_read_esl_address(&address);

  GLIB_contextInit(&context);
  context.backgroundColor = White;
  context.foregroundColor = Black;
  // Clear only a region for text will keep the rest of the image in the "background"
  GLIB_setClippingRegion(&context, &clip_rect);
  GLIB_clearRegion(&context);
  // Restore the clipping area to enable the use of screen coordinates for GLIB_drawString() calls
  GLIB_resetDisplayClippingArea(&context);
  esl_tag_demo_draw_rectangle(&context, clip_rect);
  if (status == SL_STATUS_OK) {
    uint8_t id = esl_core_get_id(address);
    uint8_t group_id = esl_core_get_group_id(address);
    snprintf(msg, sizeof(msg), msg, group_id);
    GLIB_drawString(&context, msg, strlen(msg), GLIB_X_START, GLIB_Y_START_GROUPID, true);
    // Manual center alignment with extra spaces around the text
    snprintf(msg, sizeof(msg), " ESL ID: % 3d ", id);
    GLIB_drawString(&context, msg, strlen(msg), GLIB_X_START, GLIB_Y_START_ESLID, true);
  } else {
    GLIB_drawString(&context, error_msg, strlen(error_msg), GLIB_X_START, GLIB_Y_START_ERR, true);
  }
  DMD_updateDisplay();
}

sl_status_t esl_tag_wstk_lcd_run_qrcode()
{
  sl_status_t sl_status = SL_STATUS_FAIL;
  if (esl_wstk_lcd_is_logo()) {
    EMSTATUS status;
    static GLIB_Context_t context;
    GLIB_contextInit(&context);
    context.backgroundColor = White;
    context.foregroundColor = Black;
    GLIB_resetDisplayClippingArea(&context);
    GLIB_clearRegion(&context);
    char run_qrcode[] = "Please run";
    // Note: all text positions below are pre-calculated for central alignment
    status = GLIB_drawString(&context, run_qrcode, strlen(run_qrcode), 24, 48, true);
    snprintf(run_qrcode, sizeof(run_qrcode), "the QRCode");
    status = GLIB_drawString(&context, run_qrcode, strlen(run_qrcode), 24, 58, true);
    snprintf(run_qrcode, sizeof(run_qrcode), "generator");
    status = GLIB_drawString(&context, run_qrcode, strlen(run_qrcode), 28, 68, true);
    snprintf(run_qrcode, sizeof(run_qrcode), "script!");
    status = GLIB_drawString(&context, run_qrcode, strlen(run_qrcode), 36, 78, true);
    if (status == DMD_OK) {
      DMD_updateDisplay();
      sl_status = SL_STATUS_OK;
    }
  }
  return sl_status;
}
