/***************************************************************************//**
 * @file
 * @brief Application log source file
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
#if defined(_WIN32) || defined(__MSYS__)
// Weak symbol limitation for MinGW,
// see https://sourceware.org/bugzilla/show_bug.cgi?id=9687
#define SL_WEAK
#else
#include "sl_common.h"
#endif

#ifdef SL_COMPONENT_CATALOG_PRESENT
#include "sl_component_catalog.h"
#endif // SL_COMPONENT_CATALOG_PRESENT

#include "sl_status.h"
#include "app_log.h"
#include "app_log_config.h"
#include <string.h>

#if APP_LOG_ENABLE == 1 && APP_LOG_TIME_ENABLE == 1
#ifdef SL_CATALOG_SLEEPTIMER_PRESENT
#include "sl_sleeptimer.h"
#else
#include <time.h>
#endif // SL_CATALOG_SLEEPTIMER_PRESENT
#endif // APP_LOG_TIME_ENABLE

// -----------------------------------------------------------------------------
// Global variables

/// IOStream for logging
sl_iostream_t * app_log_iostream = NULL;

// -----------------------------------------------------------------------------
// Local variables

#if APP_LOG_ENABLE == 1 && APP_LOG_COUNTER_ENABLE == 1
/// Counter for logging
static uint32_t counter = 0;
#endif // APP_LOG_ENABLE == 1 && APP_LOG_COUNTER_ENABLE == 1

#if defined(APP_LOG_LEVEL_FILTER_DYNAMIC) && APP_LOG_LEVEL_FILTER_DYNAMIC == 1
/// Simple filter status
static bool level_filter_enabled = APP_LOG_LEVEL_FILTER_ENABLE;

/// Simple filter for logging
static uint8_t level_filter_threshold = APP_LOG_LEVEL_FILTER_THRESHOLD;

/// Mask status
static bool level_mask_enabled = APP_LOG_LEVEL_MASK_ENABLE;

/// Mask for logging
static uint8_t level_mask = APP_LOG_LEVEL_MASK;

/***************************************************************************//**
 * Checking log level
 ******************************************************************************/
bool _app_log_check_level(uint8_t level)
{
  bool ret = true;
  if (level_filter_enabled) {
    if (level > level_filter_threshold) {
      ret = false;
    }
  }
  if (level_mask_enabled) {
    if ( !(level_mask & (1 << level)) ) {
      ret = false;
    }
  }
  return ret;
}

/***************************************************************************//**
 * Enable or disable simple log level filter
 ******************************************************************************/
void _app_log_filter_threshold_enable(bool enable)
{
  level_filter_enabled = enable;
}

/***************************************************************************//**
 * Enable or disable masking for log levels
 ******************************************************************************/
void _app_log_filter_mask_enable(bool enable)
{
  level_mask_enabled = enable;
}

/***************************************************************************//**
 * Set simple filter threshold log level
 ******************************************************************************/
sl_status_t _app_log_filter_threshold_set(uint8_t level)
{
  sl_status_t sc = SL_STATUS_OK;
  if (level < APP_LOG_LEVEL_COUNT) {
    level_filter_threshold = level;
  } else {
    sc = SL_STATUS_INVALID_PARAMETER;
  }
  return sc;
}

/***************************************************************************//**
 * Set masks for log levels
 ******************************************************************************/
sl_status_t _app_log_filter_mask_set(uint8_t mask)
{
  sl_status_t sc = SL_STATUS_OK;
  if (mask < (1 << (APP_LOG_LEVEL_COUNT))) {
    level_mask = mask;
  } else {
    sc = SL_STATUS_INVALID_PARAMETER;
  }
  return sc;
}

/***************************************************************************//**
 * Get simple filter threshold log level
 ******************************************************************************/
uint8_t _app_log_filter_threshold_get(void)
{
  return level_filter_threshold;
}

/***************************************************************************//**
 * Get masks for log levels
 ******************************************************************************/
uint8_t _app_log_filter_mask_get(void)
{
  return level_mask;
}
#endif // APP_LOG_LEVEL_FILTER_DYNAMIC

/***************************************************************************//**
 * Sets IO Stream
 ******************************************************************************/
sl_status_t app_log_iostream_set(sl_iostream_t *stream)
{
  if (stream == NULL) {
    return SL_STATUS_NULL_POINTER;
  }
  app_log_iostream = stream;
  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Get the current IO Stream for logging
 ******************************************************************************/
sl_iostream_t * app_log_iostream_get(void)
{
  return app_log_iostream;
}

/******************************************************************************
* Log status string
******************************************************************************/
void _app_log_status_string(sl_status_t sc)
{
  sl_iostream_printf(app_log_iostream, "(");
  sl_iostream_t * default_stream_cfgd = sl_iostream_get_default();
  sl_iostream_set_default(app_log_iostream);
  sl_status_print(sc);
  sl_iostream_set_default(default_stream_cfgd);
  sl_iostream_printf(app_log_iostream, ") ");
}

/***************************************************************************//**
 * Log time
 ******************************************************************************/
SL_WEAK void _app_log_time()
{
  #if APP_LOG_ENABLE == 1 && APP_LOG_TIME_ENABLE == 1
  #ifdef SL_CATALOG_SLEEPTIMER_PRESENT
  uint32_t time_ms = (uint32_t)(sl_sleeptimer_get_tick_count64()
                                * 1000
                                / sl_sleeptimer_get_timer_frequency());
  app_log_append(APP_LOG_TIME_FORMAT APP_LOG_SEPARATOR,
                 (time_ms / 3600000),
                 (time_ms / 60000) % 60,
                 (time_ms / 1000) % 60,
                 time_ms % 1000);
  #else // SL_CATALOG_SLEEPTIMER_PRESENT
  time_t current_time = time(NULL);
  struct tm *loc_time;

  loc_time = localtime(&current_time);
  if (loc_time != NULL) {
    app_log_append(APP_LOG_TIME_FORMAT APP_LOG_SEPARATOR,
                   loc_time->tm_hour,
                   loc_time->tm_min,
                   loc_time->tm_sec,
                   (uint32_t)(tv.tv_usec / 1000));
  }
  #endif // SL_CATALOG_SLEEPTIMER_PRESENT
  #endif // APP_LOG_ENABLE == 1 && APP_LOG_TIME_ENABLE == 1
}

/***************************************************************************//**
 * Log counter
 ******************************************************************************/
void _app_log_counter()
{
  #if APP_LOG_ENABLE == 1 && APP_LOG_COUNTER_ENABLE == 1
  app_log_append(APP_LOG_COUNTER_FORMAT
                 APP_LOG_SEPARATOR,
                 counter++);
  #endif // APP_LOG_ENABLE == 1 && APP_LOG_COUNTER_ENABLE == 1
}

/******************************************************************************
* Application log init
******************************************************************************/
void app_log_init()
{
  #if defined(APP_LOG_LEVEL_FILTER_DYNAMIC) && APP_LOG_LEVEL_FILTER_DYNAMIC == 1
  // Init filter and mask
  level_filter_enabled = APP_LOG_LEVEL_FILTER_ENABLE;
  level_mask_enabled = APP_LOG_LEVEL_MASK_ENABLE;
  level_filter_threshold = APP_LOG_LEVEL_FILTER_THRESHOLD;
  level_mask = APP_LOG_LEVEL_MASK;
  #endif // APP_LOG_LEVEL_FILTER_DYNAMIC

  #if defined(APP_LOG_OVERRIDE_DEFAULT_STREAM) && APP_LOG_OVERRIDE_DEFAULT_STREAM == 1
  char *name = APP_LOG_STREAM_INSTANCE;
  sl_iostream_type_t type = APP_LOG_STREAM_TYPE;

  sl_iostream_t * iostream = NULL;
  sl_iostream_t * iostream_type = NULL;

  for (uint32_t i = 0; i < sl_iostream_instances_count; i++) {
    if (sl_iostream_instances_info[i]->type == type) {
      if (strcmp(sl_iostream_instances_info[i]->name, name) == 0) {
        iostream = sl_iostream_instances_info[i]->handle;
        break;
      }
      if (iostream_type == NULL) {
        iostream_type = sl_iostream_instances_info[i]->handle;
      }
    }
  }

  if (iostream == NULL) {
    // The stream is not found by name
    if (iostream_type != NULL) {
      // Stream found by type
      iostream = iostream_type;
    } else {
      // Not found stream, set to default
      iostream = sl_iostream_get_default();
    }
  }

  app_log_iostream = iostream;
  #else  // APP_LOG_OVERRIDE_DEFAULT_STREAM
  app_log_iostream = sl_iostream_get_default();
  #endif // APP_LOG_OVERRIDE_DEFAULT_STREAM
}

/******************************************************************************
* Weak implementation of status print
******************************************************************************/
SL_WEAK void sl_status_print(sl_status_t status)
{
  (void) status;
  app_log_append(APP_LOG_UNRESOLVED_STATUS);
}
