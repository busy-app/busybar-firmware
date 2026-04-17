/***************************************************************************//**
 * @file
 * @brief Dynamic Hardware Configuration interface.
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

#ifndef DHC_H
#define DHC_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "sl_status.h"

// Payload structure
// {header} {command} {setting} {value}
// {header} - always 0xDC (for Dynamic Configuration)
// {command} - 0x00: read, 0x01: write
// {setting} - setting to read or write (see dhc_setting_t)
// {values} - values to write (not needed in case of reading)
//
// Example payload: 0xDC 0x00 0x00
// This payload reads the version of the configuration settings.
//
// Example payload: 0xDC 0x01 0x05 0x42 0x00 0x00 0x00
// This payload writes the value 0x42 to the CTUNE setting.
#define DHC_HEADER_LEN    3u
#define DHC_ID            0xDC

// The version shows the revision of the configuration settings
// It is used to check if the configuration settings are compatible
// with the current firmware.
// The version shall be increased every time the configuration layout
// is changed.
#define DHC_VERSION       0x01

typedef enum {
  DHC_CMD_READ            = 0,
  DHC_CMD_WRITE           = 1,
  DHC_CMD_COUNT           = 2,
} dhc_command_t;

typedef enum {
  DHC_SETTING_VERSION     = 0,
  DHC_SETTING_RSSI_OFFSET = 1,
  DHC_SETTING_PA_CURVE    = 2,
  DHC_SETTING_PA_MODE     = 3,
  DHC_SETTING_CTUNE       = 4,
  DHC_SETTING_COUNT       = 5,
} dhc_setting_t;

/**************************************************************************//**
 * Read configuration.
 *
 * @param[in] setting   Configuration to read.
 * @param[in] msg       Additional message. (Only required for RCP.)
 *****************************************************************************/
void sl_dhc_handle_read(dhc_setting_t setting, void *msg);

/**************************************************************************//**
 * Set configuration.
 *
 * @param[in] setting   Configuration type.
 * @param[in] data      Configuration value.
 * @param[in] data_len  Configuration value length.
 * @param[in] msg       Additional message. (Only required for RCP.)
 *****************************************************************************/
void sl_dhc_handle_write(dhc_setting_t setting, uint8_t *data, uint8_t data_len, void *msg);

/**************************************************************************//**
 * Initialize Dynamic Hardware Configuration transport.
 *****************************************************************************/
void sl_dhc_init(void);

/**************************************************************************//**
 * Send response.
 * Depending on the transport layer either implemented using NCP user message
 * responses or HCI responses.
 *
 * @param[in] status    Status code to send.
 * @param[in] data      Data.
 * @param[in] data_len  Data length.
 * @param[in] msg       Additional message. (Only required for RCP.)
 *****************************************************************************/
void sl_dhc_send_response(sl_status_t status, uint8_t *data, size_t data_len, void *msg);

#endif // DHC_H
