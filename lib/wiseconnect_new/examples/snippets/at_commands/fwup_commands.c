/*******************************************************************************
 * @file  fwup_commands.c
 * @brief Firmware Update AT Commands
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

#include "sl_board_configuration.h"
#include "sl_rsi_utility.h"
#include "sl_utility.h"
#include "console.h"
#include "sl_net.h"
#include "at_utility.h"
#include "firmware_upgradation.h"
#include "at_command_data_mode.h"
#include "firmware_upgradation.h"
#include <string.h>
#include <inttypes.h>

/******************************************************
 *                    Constants
 ******************************************************/

#ifndef FWUP_MAX_FWUP_CHUNK_SIZE
#define FWUP_MAX_FWUP_CHUNK_SIZE 1024
#endif

#ifndef FWUP_RPS_HEADER_SIZE
#define FWUP_RPS_HEADER_SIZE 64
#endif

/******************************************************
 *               Variable Definitions
 ******************************************************/

/******************************************************
 *               Function Definitions
 ******************************************************/

static sl_status_t fwup_start_send_buffer_handler(uint8_t *buffer, uint32_t length, void *user_data)
{
  UNUSED_PARAMETER(user_data);
  UNUSED_PARAMETER(length);

  sl_status_t status = sl_si91x_fwup_start(buffer);
  VERIFY_STATUS_AND_RETURN(status);

  PRINT_AT_CMD_SUCCESS;
  return SL_STATUS_OK;
}

static sl_status_t fwup_load_send_buffer_handler(uint8_t *buffer, uint32_t length, void *user_data)
{
  UNUSED_PARAMETER(user_data);

  sl_status_t status = sl_si91x_fwup_load(buffer, length);
  VERIFY_STATUS_AND_RETURN(status);

  PRINT_AT_CMD_SUCCESS;
  return SL_STATUS_OK;
}

sl_status_t ota_fw_up_response_handler(sl_net_event_t event, sl_status_t status, void *data, uint32_t user_data)
{
  UNUSED_PARAMETER(event);
  UNUSED_PARAMETER(data);
  UNUSED_PARAMETER(user_data);

  AT_PRINTF("at+FWUP_OTA_STATUS=%" PRIu32 "\r\n>\r\n", status);

  return status;
}

// at+fwup-fast
sl_status_t fwup_fast_command_handler(console_args_t *arguments)
{
  UNUSED_PARAMETER(arguments);
  return SL_STATUS_NOT_SUPPORTED;
}

// at+fwup-start=<rps-header-length>
sl_status_t fwup_start_command_handler(console_args_t *arguments)
{
  CHECK_ARGUMENT_BITMAP(arguments, 0x01);

  if (!sl_si91x_is_device_initialized()) {
    return SL_STATUS_NOT_INITIALIZED;
  }

  uint16_t rps_header_length = GET_OPTIONAL_COMMAND_ARG(arguments, 0, 0, uint16_t);
  if ((rps_header_length < FWUP_RPS_HEADER_SIZE) || (rps_header_length > FWUP_MAX_FWUP_CHUNK_SIZE)) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  sl_status_t status = at_command_goto_data_mode(fwup_start_send_buffer_handler, rps_header_length, NULL);
  VERIFY_STATUS_AND_RETURN(status);

  PRINT_AT_CMD_SUCCESS;
  return SL_STATUS_OK;
}

// at+fwup-load=<content-length>
sl_status_t fwup_load_command_handler(console_args_t *arguments)
{
  CHECK_ARGUMENT_BITMAP(arguments, 0x01);

  if (!sl_si91x_is_device_initialized()) {
    return SL_STATUS_NOT_INITIALIZED;
  }

  uint16_t content_length = GET_OPTIONAL_COMMAND_ARG(arguments, 0, 0, uint16_t);
  if ((content_length == 0) || (content_length > FWUP_MAX_FWUP_CHUNK_SIZE)) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  sl_status_t status = at_command_goto_data_mode(fwup_load_send_buffer_handler, content_length, NULL);
  VERIFY_STATUS_AND_RETURN(status);

  PRINT_AT_CMD_SUCCESS;
  return SL_STATUS_OK;
}

// at+fwup-abort
sl_status_t fwup_abort_command_handler(console_args_t *arguments)
{
  UNUSED_PARAMETER(arguments);

  sl_status_t status = sl_si91x_fwup_abort();
  VERIFY_STATUS_AND_RETURN(status);

  PRINT_AT_CMD_SUCCESS;
  return SL_STATUS_OK;
}

// at+fwup-otaf=<server-ip>,<server-port>,<chunk-number>,<timeout>,
//              <tcp-retry-count>
sl_status_t fwup_otaf_command_handler(console_args_t *arguments)
{
#ifdef SLI_SI91X_OFFLOAD_NETWORK_STACK
  CHECK_ARGUMENT_BITMAP(arguments, 0x1F);

  if (!sl_si91x_is_device_initialized()) {
    return SL_STATUS_NOT_INITIALIZED;
  }

  const char *server_ip = GET_OPTIONAL_COMMAND_ARG(arguments, 0, NULL, char *);
  if (server_ip == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  sl_ip_address_t ip_address;
  sl_status_t status = sl_net_inet_addr_auto(server_ip, &ip_address);
  VERIFY_STATUS_AND_RETURN(status);

  uint16_t server_port     = GET_OPTIONAL_COMMAND_ARG(arguments, 1, 0, uint16_t);
  uint16_t chunk_number    = GET_OPTIONAL_COMMAND_ARG(arguments, 2, 0, uint16_t);
  uint16_t timeout         = GET_OPTIONAL_COMMAND_ARG(arguments, 3, 0, uint16_t);
  uint16_t tcp_retry_count = GET_OPTIONAL_COMMAND_ARG(arguments, 4, 0, uint16_t);

  status = sl_si91x_ota_firmware_upgradation(ip_address, server_port, chunk_number, timeout, tcp_retry_count, true);
  if ((SL_STATUS_IN_PROGRESS == status) || (SL_STATUS_OK == status)) {
    PRINT_AT_CMD_SUCCESS;
    return SL_STATUS_OK;
  }

  return status;
#else
  UNUSED_PARAMETER(arguments);
  return SL_STATUS_NOT_SUPPORTED;
#endif
}
