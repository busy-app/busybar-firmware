/***************************************************************************//**
 * @file
 * @brief Extended result event serializer.
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

#include <stdint.h>
#include <string.h>
#include "sl_status.h"
#include "sl_common.h"
#include "sl_power_manager.h"
#include "sl_bt_api.h"
#include "app_log.h"
#include "cs_acp.h"
#include "cs_result.h"
#include "cs_initiator_config.h"
#include "extended_result.h"

// -----------------------------------------------------------------------------
// Macros

// This is not an exact calculation, but a good enough approximation that can
// safely store the procedure data coming from the CS initiator component.
// Including size of the header, step channel array and 2 RAS data arrays.
#define EVT_DATA_BUFFER_MAX_SIZE (                                  \
    sizeof(cs_acp_result_evt_t) + sizeof(uint8_t)                   \
    + sizeof(uint8_t) + CS_MAX_STEP_COUNT                           \
    + ((sizeof(uint32_t) + CS_INITIATOR_MAX_RANGING_DATA_SIZE) * 2) \
    )

#define EVT_OVERHEAD             (sizeof(cs_acp_event_id_t) + 3)
#define EVT_MAX_DATA             (UINT8_MAX - EVT_OVERHEAD)

// -----------------------------------------------------------------------------
// Static variables

static uint8_t evt_data_buffer[EVT_DATA_BUFFER_MAX_SIZE];
static size_t evt_data_buffer_len = 0;
static uint8_t *evt_data_ptr;
static uint8_t connection;

// -----------------------------------------------------------------------------
// Static function declarations

static sl_status_t serialize_extended_result(const uint16_t ranging_counter,
                                             const uint8_t*result,
                                             uint8_t result_size,
                                             const cs_ranging_data_t *ranging_data,
                                             size_t max_data_size,
                                             size_t *data_len,
                                             uint8_t *data);

// -----------------------------------------------------------------------------
// Public function definitions

/******************************************************************************
 * Add extended result data to the ACP event buffer.
 *****************************************************************************/
void cs_on_extended_result(const uint8_t conn_handle,
                           const uint16_t ranging_counter,
                           const uint8_t *result,
                           const cs_result_session_data_t *result_metadata,
                           const cs_ranging_data_t *ranging_data,
                           const void *user_data)
{
  sl_status_t sc;
  size_t data_len;

  (void)user_data;

  if (evt_data_buffer_len > 0) {
    app_log_error("Event data buffer busy" APP_LOG_NL);
    return;
  }

  sc = serialize_extended_result(ranging_counter,
                                 result,
                                 result_metadata->size,
                                 ranging_data,
                                 sizeof(evt_data_buffer),
                                 &data_len,
                                 evt_data_buffer);
  if (sc != SL_STATUS_OK) {
    app_log_status_error_f(sc, "Event data serialization failed" APP_LOG_NL);
    return;
  }

  connection = conn_handle;
  evt_data_buffer_len = data_len;
  evt_data_ptr = evt_data_buffer;
  // Keep the MCU awake until all fragments are sent.
  sl_power_manager_add_em_requirement(SL_POWER_MANAGER_EM1);
}

/******************************************************************************
 * Extended result event serializer task.
 *****************************************************************************/
void extended_result_step(void)
{
  if (evt_data_buffer_len == 0) {
    return;
  }

  // Avoid big buffer allocation on the stack by using static variable.
  static uint8_t evt_data[UINT8_MAX];
  uint8_t evt_data_len = (uint8_t)SL_MIN(evt_data_buffer_len, (size_t)EVT_MAX_DATA);
  cs_acp_event_t *evt = (cs_acp_event_t *)evt_data;
  evt->connection_id = connection;
  evt->acp_evt_id = CS_ACP_EVT_EXTENDED_RESULT_ID;
  memcpy(evt->data.ext_result.fragment.data, evt_data_ptr, evt_data_len);
  evt->data.ext_result.fragment.len = evt_data_len;
  evt_data_buffer_len -= evt_data_len;
  // Calculate remaining number of fragments using ceiling division.
  evt->data.ext_result.fragments_left = (uint8_t)((evt_data_buffer_len + EVT_MAX_DATA - 1) / EVT_MAX_DATA);
  if (evt_data_ptr == evt_data_buffer) {
    // First fragment
    evt->data.ext_result.fragments_left |= CS_ACP_FIRST_FRAGMENT_MASK;
  }
  evt_data_ptr += evt_data_len;
  sl_bt_send_evt_user_cs_service_message_to_host(evt_data_len + EVT_OVERHEAD, evt_data);

  if (evt_data_buffer_len == 0) {
    sl_power_manager_remove_em_requirement(SL_POWER_MANAGER_EM1);
  }
}

// -----------------------------------------------------------------------------
// Internal function definitions

/******************************************************************************
 * Serialize extended result data.
 *****************************************************************************/
static sl_status_t serialize_extended_result(const uint16_t ranging_counter,
                                             const uint8_t *result,
                                             uint8_t result_size,
                                             const cs_ranging_data_t *ranging_data,
                                             size_t max_data_size,
                                             size_t *data_len,
                                             uint8_t *data)
{
  (void)ranging_counter;
  size_t data_len_calculated
    = sizeof(result_size)
      + result_size
      + sizeof(ranging_data->num_steps)
      + ranging_data->num_steps
      + sizeof(ranging_data->initiator.ranging_data_size)
      + ranging_data->initiator.ranging_data_size
      + sizeof(ranging_data->initiator.ranging_data_size)
      + ranging_data->reflector.ranging_data_size;

  if (data_len_calculated > max_data_size) {
    return SL_STATUS_WOULD_OVERFLOW;
  }

  // Serialize result size
  memcpy(data, &result_size, sizeof(result_size));
  data += sizeof(result_size);

  // Serialize result
  memcpy(data, result, result_size);
  data += result_size;

  // Serialize num_steps
  memcpy(data, &ranging_data->num_steps, sizeof(ranging_data->num_steps));
  data += sizeof(ranging_data->num_steps);

  // Serialize step channel data (1 octet per step_channel)
  memcpy(data, ranging_data->step_channels, ranging_data->num_steps);
  data += ranging_data->num_steps;

  // Serialize ranging data size for initiator
  memcpy(data,
         &ranging_data->initiator.ranging_data_size,
         sizeof(ranging_data->initiator.ranging_data_size));
  data += sizeof(ranging_data->initiator.ranging_data_size);

  // Serialize ranging data for initiator
  memcpy(data,
         ranging_data->initiator.ranging_data,
         ranging_data->initiator.ranging_data_size);
  data += ranging_data->initiator.ranging_data_size;

  // Serialize ranging data size for reflector
  memcpy(data,
         &ranging_data->reflector.ranging_data_size,
         sizeof(ranging_data->reflector.ranging_data_size));
  data += sizeof(ranging_data->reflector.ranging_data_size);

  // Serialize ranging data for reflector
  memcpy(data,
         ranging_data->reflector.ranging_data,
         ranging_data->reflector.ranging_data_size);
  data += ranging_data->reflector.ranging_data_size;

  *data_len = data_len_calculated;

  return SL_STATUS_OK;
}
