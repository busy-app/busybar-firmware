/***************************************************************************//**
 * @file
 * @brief Direct test mode interface.
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

#ifndef DTM_H
#define DTM_H

#include <stdint.h>
#include "sl_status.h"
#include "sl_iostream.h"
#include "sl_bt_api.h"

#ifdef __cplusplus
extern "C" {
#endif

#define RX_BUF_SIZE 2                         //< Size of the receive buffer in bytes

typedef struct {
  sl_iostream_t     *stream;                  //< IO Stream instance used for communication with the DTM tester equipment
  uint8_t           rx_buf[RX_BUF_SIZE];      //< RX buffer
  uint8_t           rx_len;                   //< The amount of data in the RX buffer
  sl_status_t (*tx)(sl_iostream_t *stream, char c);  //< Function for sending a single response byte to the Upper Tester
  uint32_t command_ready_signal;              //< A signal emitted by sl_bt_external_signal when a command is ready
  uint32_t (*get_ticks)();                    //< Function for getting current clock ticks
  uint32_t (*ms_to_tick)(uint16_t time_ms);   //< Function to convert ms to ticks
} config_t;

/**************************************************************************//**
 * Initialize testmode library.
 * @param[in] config Configuration structure
 *****************************************************************************/
void testmode_init(const config_t *config);

/**************************************************************************//**
 * Process a single byte of a command received from the Upper Tester.
 * @param[in] byte The command byte to process
 *****************************************************************************/
void testmode_process_command_byte(const uint8_t byte);

/**************************************************************************//**
 * Handle a bluetooth event. This function can be called for all events received
 * from the Bluetooth stack.
 * @param[in] evt Event received from the Bluetooth stack
 *****************************************************************************/
void testmode_on_event(const sl_bt_msg_t *evt);

#ifdef __cplusplus
};
#endif

#endif // DTM_H
