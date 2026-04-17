/***************************************************************************//**
 * @file
 * @brief IO Stream interface over BGAPI Trace.
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
#include "sl_status.h"
#include "sli_bgapi_trace.h"
#include "iostream_bgapi_trace.h"

#define BGAPI_TRACE_MAX_LEN 247
#define LOG_SOURCE_INDICATOR 0xbb
#define END_OF_MESSAGE '\n'

static sl_status_t write(void *context, const void *buffer, size_t length);

// The first byte of the write buffer is used for indicating the log source.
static size_t write_len = 1;
static uint8_t write_buffer[BGAPI_TRACE_MAX_LEN] = { LOG_SOURCE_INDICATOR };

static sl_iostream_t iostream_bgapi_trace = {
  .write = write,
  .read = NULL,
  .context = NULL
};

sl_iostream_t *iostream_bgapi_trace_handle = &iostream_bgapi_trace;

static sl_status_t write(void *context, const void *buffer, size_t length)
{
  (void)context;
  for (size_t i = 0; i < length; i++) {
    const uint8_t byte = ((const uint8_t *)buffer)[i];
    write_buffer[write_len] = byte;
    write_len++;
    if (byte == END_OF_MESSAGE || write_len == BGAPI_TRACE_MAX_LEN) {
      size_t log_written = sli_bgapi_trace_log_custom_message(write_buffer,
                                                              write_len);
      // If BGAPI trace is disabled, sli_bgapi_trace_log_custom_message returns
      // 0. It should not be treated as error.
      if (log_written != 0 && log_written != write_len) {
        write_len = 1;
        return SL_STATUS_TRANSMIT;
      }
      write_len = 1;
    }
  }
  return SL_STATUS_OK;
}
