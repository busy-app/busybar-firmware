/***************************************************************************//**
 * @file
 * @brief app_process.c
 *******************************************************************************
 * # License
 * <b>Copyright 2018 Silicon Laboratories Inc. www.silabs.com</b>
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
//                                   Includes
// -----------------------------------------------------------------------------
#include "sl_rail.h"
#include "sl_component_catalog.h"
#include "app_process.h"
#include "sl_rail_util_init.h"
#include "sl_rail_sdk_simple_assistance.h"
#include "sl_rail_sdk_wmbus_support.h"
#include "em_emu.h"

#if defined(SL_CATALOG_KERNEL_PRESENT)
#include "app_task_init.h"
#endif

#include "cmsis_compiler.h"
#include "sl_rail_sdk_fifo_size_config.h"
#include "sl_code_classification.h"

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
//                          Static Function Declarations
// -----------------------------------------------------------------------------
static void print_blocks(const uint8_t *buffer, uint16_t length);
static void print_rx_packets(sl_rail_handle_t rail_handle);

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                                Static Variables
// -----------------------------------------------------------------------------
static volatile state_t state = S_IDLE;

/// Contains the last RAIL Rx/Tx error events
static volatile uint64_t current_rail_err = 0;

/// Contains the status of RAIL Calibration
static volatile sl_rail_status_t calibration_status = 0;

static uint8_t rx_buffer[SL_RAIL_SDK_RX_FIFO_SIZE];

// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------
void set_next_state(state_t next_state)
{
  state = next_state;
}

/******************************************************************************
 * Application state machine, called infinitely
 *****************************************************************************/
void app_process_action(void)
{
  // Get RAIL handle, used later by the application
  sl_rail_handle_t rail_handle = sl_rail_util_get_handle(SL_RAIL_UTIL_HANDLE_INST0);
  uint64_t current_rail_err_tmp = current_rail_err;

  switch (state) {
    case S_PACKET_RECEIVED:
      print_rx_packets(rail_handle);
      state = S_IDLE;
#if defined(SL_CATALOG_KERNEL_PRESENT)
      app_task_notify();
#endif
      break;
    case S_RX_PACKET_ERROR:
      // Handle Rx error
      app_log_error("Radio RX Error occurred\nEvents: %lld\n", current_rail_err_tmp);
      state = S_IDLE;
#if defined(SL_CATALOG_KERNEL_PRESENT)
      app_task_notify();
#endif
      break;
    case S_CALIBRATION_ERROR:
      app_log_warning("Radio Calibration Error occurred\nEvents: %lld\nsl_rail_calibrate() result:%ld\n",
                      current_rail_err_tmp,
                      calibration_status);
      state = S_IDLE;
#if defined(SL_CATALOG_KERNEL_PRESENT)
      app_task_notify();
#endif
      break;
    case S_IDLE:
      break;
    default:
      break;
  }

  ///////////////////////////////////////////////////////////////////////////
  // Put your application code here!                                       //
  // This is called infinitely.                                            //
  // Do not call blocking functions from here!                             //
  ///////////////////////////////////////////////////////////////////////////
}

/******************************************************************************
 * RAIL callback, called if a RAIL event occurs
 *****************************************************************************/
SL_CODE_RAM void sl_rail_util_on_event(sl_rail_handle_t rail_handle, sl_rail_events_t events)
{
  if (events & SL_RAIL_EVENTS_RX_COMPLETION) {
    if (events & SL_RAIL_EVENT_RX_PACKET_RECEIVED) {
      // Keep the packet in the radio buffer, download it later at the state machine
      sl_rail_hold_rx_packet(rail_handle);
      state = S_PACKET_RECEIVED;
    } else {
      // Handle Rx error
      current_rail_err = (events & SL_RAIL_EVENTS_RX_COMPLETION);
      state = S_RX_PACKET_ERROR;
    }
  }

  // Perform all calibrations when needed
  if (events & SL_RAIL_EVENT_CAL_NEEDED) {
    calibration_status = sl_rail_calibrate(rail_handle, NULL, SL_RAIL_CAL_ALL_PENDING);
    if (calibration_status != SL_RAIL_STATUS_NO_ERROR) {
      current_rail_err = (events & SL_RAIL_EVENT_CAL_NEEDED);
      state = S_CALIBRATION_ERROR;
    }
  }
#if defined(SL_CATALOG_KERNEL_PRESENT)
  app_task_notify();
#endif
}

// -----------------------------------------------------------------------------
//                          Static Function Definitions
// -----------------------------------------------------------------------------
static void print_blocks(const uint8_t *buffer, uint16_t length)
{
  for (uint8_t i = 0; i < length; i++) {
    if (i % 16 == 0) {
      app_log_info("[0x%02X ", buffer[i]);
    } else if (i % 16 == 8) {
      app_log_info("| 0x%02X ", buffer[i]);
    } else if (i % 16 == 15 || i == length - 1) {
      app_log_info("0x%02X]\n", buffer[i]);
    } else {
      app_log_info("0x%02X ", buffer[i]);
    }
  }
}

static void print_rx_packets(sl_rail_handle_t rail_handle)
{
  sl_rail_rx_packet_info_t packet_info;
  sl_rail_status_t rail_status;
  sl_rail_rx_packet_details_t packet_details;
  sl_rail_rx_packet_handle_t rx_packet_handle;

  rx_packet_handle = sl_rail_get_rx_packet_info(rail_handle, SL_RAIL_RX_PACKET_HANDLE_OLDEST_COMPLETE, &packet_info);

  while (rx_packet_handle != SL_RAIL_RX_PACKET_HANDLE_INVALID) {
    packet_details.time_received.total_packet_bytes = packet_info.packet_bytes;
    packet_details.time_received.time_position = SL_RAIL_PACKET_TIME_AT_SYNC_END;
    sl_rail_get_rx_packet_details(rail_handle, rx_packet_handle, &packet_details);
    if (packet_info.packet_bytes <= SL_RAIL_SDK_RX_FIFO_SIZE) {
      rail_status = sl_rail_copy_rx_packet(rail_handle, rx_buffer, &packet_info);
    }
    rail_status = sl_rail_release_rx_packet(rail_handle, rx_packet_handle);
    if (rail_status != SL_RAIL_STATUS_NO_ERROR) {
      app_log_warning("sl_rail_release_rx_packet() result: %lu\n", rail_status);
    }

    if (packet_info.packet_bytes <= SL_RAIL_SDK_RX_FIFO_SIZE) {
      const sl_rail_sdk_wmbus_dll_header_t *dll_header = (sl_rail_sdk_wmbus_dll_header_t*)rx_buffer;
      const sl_rail_sdk_wmbus_stl_header_t *stl_header = (sl_rail_sdk_wmbus_stl_header_t*)(rx_buffer + sizeof(sl_rail_sdk_wmbus_dll_header_t));

      char mField[3];
      sl_rail_sdk_wmbus_frame_mfield_to_chars(dll_header->address.detailed.manufacturer, mField);
      app_log_info("RX:[Time:%lu]\n", packet_details.time_received.packet_time);
      app_log_info("Block-1:[L:%d,C:0x%02X,M:%c%c%c,ID:%08X,Version:0x%02X,devType:0x%02X]\n",
                   dll_header->lField,
                   dll_header->c_field.raw,
                   mField[0], mField[1], mField[2],
                   (unsigned int)dll_header->address.detailed.id,
                   (unsigned int)dll_header->address.detailed.version,
                   (unsigned int)dll_header->address.detailed.device_type);
      if (stl_header->ci_field == WMBUS_CI_EN13757_3_APPLICATION_SHORT) {
        uint8_t *payload_start = rx_buffer + sizeof(sl_rail_sdk_wmbus_dll_header_t) + sizeof(sl_rail_sdk_wmbus_stl_header_t);
        uint16_t payload_len = dll_header->lField - sizeof(sl_rail_sdk_wmbus_dll_header_t) - sizeof(sl_rail_sdk_wmbus_stl_header_t) + 1;
        app_log_info("AppHeader:[CI:0x%02X,AccessNr:%d,Status:0x%02X,encMode:%d,Accessibility:%02X,encBlocks:%d,sync:%d]\n",
                     stl_header->ci_field,
                     stl_header->access_number,
                     stl_header->status,
                     stl_header->conf_word.mode_0_5.mode,
                     stl_header->conf_word.mode_0_5.accessibility,
                     stl_header->conf_word.mode_0_5.num_of_enc_blocks,
                     stl_header->conf_word.mode_0_5.synchronized);
        if (stl_header->conf_word.mode_0_5.mode == 5) {
          uint8_t iv[16];
          //with long transport layer header, the address from the header should be used
          memcpy(iv, &(dll_header->address.raw), 8);
          memset(iv + 8, stl_header->access_number, 8);
          sl_rail_sdk_wmbus_frame_crypto5_decrypt(payload_start, payload_start, iv, payload_len);
        }
        print_blocks(payload_start, payload_len);
      } else {
        print_blocks(rx_buffer + sizeof(sl_rail_sdk_wmbus_dll_header_t), dll_header->lField - sizeof(sl_rail_sdk_wmbus_dll_header_t) + 1);
      }

      toggle_receive_led();
    }

    rx_packet_handle = sl_rail_get_rx_packet_info(rail_handle, SL_RAIL_RX_PACKET_HANDLE_OLDEST_COMPLETE, &packet_info);
  }
}
