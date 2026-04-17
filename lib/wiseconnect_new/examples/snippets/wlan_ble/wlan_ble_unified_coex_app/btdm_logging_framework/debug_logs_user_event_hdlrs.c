/*******************************************************************************
 * @file  debug_logs_user_event_hdlrs.c
 * @brief
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

#include <rsi_common_app.h>
#if BTDM_DEBUG_LOGGING
#include <stdbool.h>
#include "debug_logs_event_hdlr_auto_gen.h"
#include "event_loop.h"
#include "rsi_bt_common_apis.h"
#include "SEGGER_RTT.h"
#include <stdint.h>
#include <string.h>
// RTT Channel for controller logs (using channel 1, can be changed as needed)
#define RTT_CONTROLLER_LOG_CHANNEL 1

// Maximum buffer size for RTT logging
#define RTT_MAX_BUFFER_SIZE 1024

// Global variables for RTT logging state
static uint8_t rtt_log_buffer[RTT_MAX_BUFFER_SIZE];
static uint32_t rtt_buffer_index = 0;

// Function to flush RTT buffer
static void flush_rtt_buffer(void)
{
  if (rtt_buffer_index > 0) {
    SEGGER_RTT_Write(RTT_CONTROLLER_LOG_CHANNEL, rtt_log_buffer, rtt_buffer_index);
    rtt_buffer_index = 0;
  }
}

// Function to write data to RTT buffer
static void write_to_rtt_buffer(const char *data, uint32_t length)
{
  // Check if adding this data would exceed buffer size
  if (rtt_buffer_index + length >= RTT_MAX_BUFFER_SIZE) {
    // Flush current buffer first
    flush_rtt_buffer();
  }

  // Add data to buffer
  if (rtt_buffer_index + length < RTT_MAX_BUFFER_SIZE) {
    memcpy(&rtt_log_buffer[rtt_buffer_index], data, length);
    rtt_buffer_index += length;
  }
}

/**
* @fn          void rsi_bt_on_controller_logs(uint16_t status, void *event_data)
* @brief       Handler for controller logs event to be executed in the GAP task context.
* @param[in]   uint16_t status, event status.
* @param[in]   void *event_data, event data containing controller logs.
* @return      None.
*
* @section description
* This function handles the controller logs event and outputs the logs via SEGGER RTT.
* The logs are buffered and sent through RTT channel 1 for real-time debugging.
*/
void rsi_bt_on_controller_logs(uint16_t status, void *event_data)
{
  UNUSED_PARAMETER(status);
  // Cast event_data to char *
  char *data = (char *)event_data;

  // Extract event data length and payload
  size_t event_data_len = (uint8_t)data[0] - 1; // Assuming the first byte contains the length
  char *log_data        = &data[1];             // Data starts from the second byte
  // Write the raw event data to RTT buffer
  write_to_rtt_buffer(log_data, event_data_len);

  // Flush buffer to ensure data is sent
  flush_rtt_buffer();
}
#endif