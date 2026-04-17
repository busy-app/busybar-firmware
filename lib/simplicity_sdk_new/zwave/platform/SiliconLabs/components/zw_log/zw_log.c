/**
 * @file
 * @brief Silicon Labs implementation of the Z-Wave PAL logging module.
 *
 * @copyright 2025 Silicon Laboratories Inc.
 */

#include "zpal_log.h"
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <FreeRTOS.h>
#include <assert.h>
#include <task.h>
#include "sl_iostream_handles.h"

#define CHANNEL_NAME_MAX_LENGTH 10

static uint8_t log_buffer[ZW_LOG_BUFFER_SIZE];

static const char log_channels[ZPAL_LOG_LEVEL_END][CHANNEL_NAME_MAX_LENGTH] = {
  [ZPAL_LOG_LEVEL_DEBUG] =   ZW_LOG_CHANNEL_DEBUG,
  [ZPAL_LOG_LEVEL_INFO] =    ZW_LOG_CHANNEL_INFO,
  [ZPAL_LOG_LEVEL_WARNING] = ZW_LOG_CHANNEL_WARNING,
  [ZPAL_LOG_LEVEL_ERROR] =   ZW_LOG_CHANNEL_ERROR,
};

static const char level_chars[] = {
  [ZPAL_LOG_LEVEL_DEBUG] =   'D',
  [ZPAL_LOG_LEVEL_INFO] =    'I',
  [ZPAL_LOG_LEVEL_WARNING] = 'W',
  [ZPAL_LOG_LEVEL_ERROR] =   'E'
};

#if defined(ZW_CONTROLLER)
static const char iostream_instance_override_ncp[] = "swo";
#endif // defined(ZW_CONTROLLER)

void zpal_log(
  zpal_log_component_t component,
  zpal_log_level_t level,
  const char * pFormat,
  ...)
{
  // Parse variable argument list
  va_list args;
  va_start(args, pFormat);

  // Validate log level and determine the character prefix to display
  char level_char = '\0';
  if (level < ZPAL_LOG_LEVEL_END) {
    level_char = level_chars[level];
  } else {
    assert(false); // Invalid log level
    return;
  }

  // Store the message in the log buffer
  int32_t msg_len = 0;
  char * p_buffer = (char *)log_buffer;
  if (ZW_LOG_DISPLAY_TIMESTAMP) {
    msg_len += snprintf(p_buffer, ZW_LOG_BUFFER_SIZE, "%09lu ", xTaskGetTickCount());
  }
  if (ZW_LOG_DISPLAY_LEVEL) {
    msg_len += snprintf(p_buffer + msg_len, ZW_LOG_BUFFER_SIZE - msg_len, "[%c] ", level_char);
  }
  if (ZW_LOG_DISPLAY_COMPONENT) {
    msg_len += snprintf(p_buffer + msg_len, ZW_LOG_BUFFER_SIZE - msg_len, "(%d) ", component);
  }
  msg_len += vsnprintf(p_buffer + msg_len, ZW_LOG_BUFFER_SIZE - msg_len, pFormat, args);
  va_end(args);

  if (msg_len > 0) {
    const char * channel = log_channels[level];
    if (channel[0] != '\0') {
#if defined(ZW_CONTROLLER)
      // Override VCOM IO Stream instance for NCP devices
      if (strncmp(channel, "vcom", 4) == 0) {
        channel = iostream_instance_override_ncp;
      }
#endif // defined(ZW_CONTROLLER)
      sl_iostream_t * iostream_handle = sl_iostream_get_handle(channel);
      if (iostream_handle) {
        sl_iostream_write(iostream_handle, log_buffer, msg_len);
      }
    }
  }
}
