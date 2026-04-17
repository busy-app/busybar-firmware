/***************************************************************************//**
 * @file sl_wsncp_buffer.c
 * @brief Wi-SUN NCP Fixed Buffer Management Implementation
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
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

#include <string.h>
#include "em_core.h"
#include "sl_wsncp_buffer.h"
#include "sl_wisun_trace_api.h"


sl_status_t sl_wsncp_buffer_init(sl_wsncp_buffer_t *buffer)
{
  if (buffer == NULL) {
    sl_wisun_trace_error("[sl_wsncp_buffer_init] Buffer pointer is NULL");
    return SL_STATUS_NULL_POINTER;
  }
  
  memset(buffer, 0, sizeof(sl_wsncp_buffer_t));
  buffer->state = SL_WSNCP_BUFFER_STATE_EMPTY;
  
  return SL_STATUS_OK;
}

sl_status_t sl_wsncp_buffer_add_data(sl_wsncp_buffer_t *buffer,
                                     uint16_t len,
                                     const uint8_t *data,
                                     void (*on_complete_callback)(void))
{
  if (buffer == NULL || data == NULL) {
    return SL_STATUS_NULL_POINTER;
  }
  
  // Check if buffer is in valid state for adding data
  if (buffer->state != SL_WSNCP_BUFFER_STATE_EMPTY) {
    return SL_STATUS_INVALID_STATE;
  }
  
  // Check for overflow - drop message if too large
  if (len > SL_WSNCP_BUFFER_SIZE) {
    // Buffer stays in EMPTY state - ready for next message
    return SL_STATUS_WOULD_OVERFLOW;
  }
  
  // Add complete message to buffer (NCP receives complete messages)
  memcpy(buffer->data, data, len);
  buffer->length = len;
  
  // Validate we have a complete message
  if (buffer->length >= sizeof(sl_wisun_msg_header_t)) {
    const sl_wisun_msg_header_t *header = (const sl_wisun_msg_header_t *)buffer->data;
    if (buffer->length >= header->length) {
      buffer->state = SL_WSNCP_BUFFER_STATE_COMPLETE;
      
      // Call completion callback if provided
      if (on_complete_callback != NULL) {
        on_complete_callback();
      }
    } else {
      // Clear buffer and stay in EMPTY state - ready for next message
      sl_wsncp_buffer_clear(buffer);
      return SL_STATUS_INVALID_PARAMETER;
    }
  } else {
    // Clear buffer and stay in EMPTY state - ready for next message
    sl_wsncp_buffer_clear(buffer);
    return SL_STATUS_INVALID_PARAMETER;
  }
  
  return SL_STATUS_OK;
}

sl_status_t sl_wsncp_buffer_clear(sl_wsncp_buffer_t *buffer)
{
  if (buffer == NULL) {
    return SL_STATUS_NULL_POINTER;
  }
  
  memset(buffer->data, 0, buffer->length);
  buffer->length = 0;
  buffer->state = SL_WSNCP_BUFFER_STATE_EMPTY;
  
  return SL_STATUS_OK;
}

// Add state management functions
sl_wsncp_buffer_state_t sl_wsncp_buffer_get_state(const sl_wsncp_buffer_t *buffer)
{
  if (buffer == NULL) {
    return SL_WSNCP_BUFFER_STATE_EMPTY; // Return safe default
  }
  
  return buffer->state;
}
