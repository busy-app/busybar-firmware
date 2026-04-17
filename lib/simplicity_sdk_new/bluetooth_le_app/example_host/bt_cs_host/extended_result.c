/***************************************************************************//**
 * @file
 * @brief Extended result event deserializer.
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

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "sl_bt_api.h"
#include "app_log.h"
#include "extended_result.h"

static uint8_t *evt_data_buffer = NULL;
static size_t evt_data_buffer_len = 0;
static size_t evt_data_size = 0;
static uint8_t expected_fragment = CS_ACP_FIRST_FRAGMENT_MASK;
static uint8_t expected_connection = SL_BT_INVALID_CONNECTION_HANDLE;

void on_extended_result_event(cs_acp_extended_result_evt_t *evt, uint8_t connection)
{
  if (((expected_fragment & CS_ACP_FIRST_FRAGMENT_MASK) != (evt->fragments_left & CS_ACP_FIRST_FRAGMENT_MASK))
      || (((expected_fragment & CS_ACP_FIRST_FRAGMENT_MASK) == 0) && (evt->fragments_left != expected_fragment))) {
    app_log_error("Unexpected event fragment: %u, expected: %u" APP_LOG_NL,
                  evt->fragments_left,
                  expected_fragment);
    // Wait for the start of the next message.
    expected_fragment = CS_ACP_FIRST_FRAGMENT_MASK;
    return;
  }

  if (evt->fragments_left & CS_ACP_FIRST_FRAGMENT_MASK) {
    evt_data_size = 0;
    expected_connection = connection;
    // Calculate expected total size for the worst case.
    size_t expected_length = UINT8_MAX * ((evt->fragments_left & CS_ACP_FRAGMENTS_LEFT_MASK) + 1);
    if (expected_length > evt_data_buffer_len) {
      if (evt_data_buffer != NULL) {
        free(evt_data_buffer);
        evt_data_buffer_len = 0;
      }
      evt_data_buffer = malloc(expected_length);
      if (evt_data_buffer == NULL) {
        app_log_error("Event buffer allocation failed" APP_LOG_NL);
        return;
      }
      evt_data_buffer_len = expected_length;
    }
  } else if (connection != expected_connection) {
    app_log_error("Unexpected connection handle: %u, expected: %u" APP_LOG_NL,
                  connection,
                  expected_connection);
    return;
  }

  memcpy(evt_data_buffer + evt_data_size, evt->fragment.data, evt->fragment.len);
  evt_data_size += evt->fragment.len;

  if ((evt->fragments_left & CS_ACP_FRAGMENTS_LEFT_MASK) == 0) {
    // Last fragment has arrived
    app_log_debug("Extended result event raw data: ");
    app_log_hexdump_debug(evt_data_buffer, evt_data_size);
    app_log_append_debug(APP_LOG_NL);
    expected_fragment = CS_ACP_FIRST_FRAGMENT_MASK;
  } else {
    expected_fragment = (evt->fragments_left & CS_ACP_FRAGMENTS_LEFT_MASK) - 1;
  }
}
