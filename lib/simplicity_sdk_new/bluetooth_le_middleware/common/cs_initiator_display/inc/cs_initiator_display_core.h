/***************************************************************************//**
 * @file
 * @brief CS Initiator display core API
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
#ifndef CS_INITIATOR_DISPLAY_CORE_H
#define CS_INITIATOR_DISPLAY_CORE_H

/***********************************************************************************************//**
 * @addtogroup cs_initiator_display
 * @{
 **************************************************************************************************/

// -----------------------------------------------------------------------------
// Includes

#include "sl_bt_api.h"
#include "sl_status.h"

#ifdef SL_CATALOG_APP_LOG_PRESENT
#include "app_log.h"
#endif // SL_CATALOG_APP_LOG_PRESENT

#include "cs_initiator_display_config.h"

#ifdef __cplusplus
extern "C"
{
#endif

// -----------------------------------------------------------------------------
// Macros

// Component logging
#if defined(SL_CATALOG_APP_LOG_PRESENT) && CS_INITIATOR_DISPLAY_LOG
#define LOG_PREFIX                "[lcd] "
#define NL                        APP_LOG_NL
#define display_log_debug(...)    app_log_debug(LOG_PREFIX  __VA_ARGS__)
#define display_log_info(...)     app_log_info(LOG_PREFIX  __VA_ARGS__)
#define display_log_warning(...)  app_log_warning(LOG_PREFIX  __VA_ARGS__)
#define display_log_error(...)    app_log_error(LOG_PREFIX  __VA_ARGS__)
#define display_log_critical(...) app_log_critical(LOG_PREFIX  __VA_ARGS__)
#else
#define NL
#define display_log_debug(...)
#define display_log_info(...)
#define display_log_warning(...)
#define display_log_error(...)
#define display_log_critical(...)
#endif // defined(SL_CATALOG_APP_LOG_PRESENT) && CS_INITIATOR_DISPLAY_LOG

// -----------------------------------------------------------------------------
// Enums, structs, typedefs

/// Display rows
typedef enum {
  ROW_MODE,
  ROW_INSTANCE_NUM,
  ROW_STATUS_TEXT,
  ROW_STATUS_VALUE,
  ROW_DISTANCE_TEXT,
  ROW_DISTANCE_VALUE,
  ROW_RAW_DISTANCE_TEXT,
  ROW_RAW_DISTANCE_VALUE,
  ROW_LIKELINESS_TEXT,
  ROW_LIKELINESS_VALUE,
  ROW_RSSI_DISTANCE_TEXT,
  ROW_RSSI_DISTANCE_VALUE,
  ROW_BIT_ERROR_RATE_TEXT,
  ROW_BIT_ERROR_RATE_VALUE
} cs_initiator_display_row_t;

/// UI alignment
typedef enum {
  CS_INITIATOR_DISPLAY_ALIGNMENT_LEFT = 0u,
  CS_INITIATOR_DISPLAY_ALIGNMENT_CENTER,
  CS_INITIATOR_DISPLAY_ALIGNMENT_RIGHT
} cs_initiator_display_alignment_t;

// -----------------------------------------------------------------------------
// Function declarations

/**************************************************************************//**
 * Initialize the display.
 *****************************************************************************/
sl_status_t cs_initiator_display_init(void);

/**************************************************************************//**
 * Set the display text alignment.
 * @param[in] alignment text alignment to use
 *****************************************************************************/
void cs_initiator_display_set_alignment(cs_initiator_display_alignment_t align);

/**************************************************************************//**
 * Write text on the LCD.
 * @param[in] str pointer to the text to print
 * @param[in] row row to print the text on
 * @param[in] instance instance number
 *****************************************************************************/
void cs_initiator_display_write_text_to_instance(char *str, uint8_t row, uint8_t instance);

/**************************************************************************//**
 * Write float value on the LCD.
 * @param[in] value float value to print
 * @param[in] row row to print the value on
 * @param[in] instance instance number
 *****************************************************************************/
void cs_initiator_display_write_float_to_instance(float value, uint8_t row, uint8_t instance);

/**************************************************************************//**
 * Clear instance values on the display.
 * @param[in] instance instance number
 *****************************************************************************/
void cs_initiator_display_clear_instance(uint8_t instance);

/**************************************************************************//**
 * Clear a specified row on the display.
 * @param[in] row row to clear
 *****************************************************************************/
void cs_initiator_display_clear_row(uint8_t row);

/**************************************************************************//**
 * Update the display.
 *****************************************************************************/
void cs_initiator_display_update(void);

// -----------------------------------------------------------------------------
// Event / callback declarations

/**************************************************************************//**
 * Bluetooth stack event handler.
 * @param[in] evt Event coming from the Bluetooth stack.
 *****************************************************************************/
void cs_initiator_display_on_event(sl_bt_msg_t *evt);

#ifdef __cplusplus
}
#endif

/** @} (end addtogroup cs_initiator_display) */
#endif // CS_INITIATOR_DISPLAY_CORE_H
