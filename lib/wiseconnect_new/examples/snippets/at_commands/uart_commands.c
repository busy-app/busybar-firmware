/*******************************************************************************
 * @file  uart_commands.c
 * @brief UART Configuration AT Commands
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
#include "sl_si91x_usart.h"
#include "at_command_data_mode.h"
#include <string.h>
#include <inttypes.h>
#include "uart_at_commands_config.h"

#ifdef USART_MODULE
#include "sl_si91x_usart_config.h"
#if USART_UC
#error "USART UC Configuration is enabled. Please disable it to use USART AT commands."
#endif
#endif

#ifdef UART_MODULE
#include "sl_si91x_uart_config.h"
#if UART_UC
#error "UART UC Configuration is enabled. Please disable it to use UART AT commands."
#endif
#endif

#ifdef ULP_UART_MODULE
#include "sl_si91x_ulp_uart_config.h"
#if ULP_UART_UC
#error "ULP UART UC Configuration is enabled. Please disable it to use UART AT commands."
#endif
#endif

/******************************************************
 *                    Constants
 ******************************************************/

#ifndef UART_PERIPHERAL
#define UART_PERIPHERAL ULPUART
#endif

#ifndef UART_MODE_DEFAULT
#define UART_MODE_DEFAULT SL_USART_MODE_ASYNCHRONOUS
#endif

#ifndef UART_MISCONTROL_DEFAULT
#define UART_MISCONTROL_DEFAULT SL_USART_MISC_CONTROL_NONE
#endif

#define UART_IS_DATA_BITS(data_bits)                                                                                 \
  ((data_bits) == SL_USART_DATA_BITS_5 || (data_bits) == SL_USART_DATA_BITS_6 || (data_bits) == SL_USART_DATA_BITS_7 \
   || (data_bits) == SL_USART_DATA_BITS_8)

#define UART_IS_STOP_BITS(stop_bits) \
  ((stop_bits) == SL_USART_STOP_BITS_1 || (stop_bits) == SL_USART_STOP_BITS_1_5 || (stop_bits) == SL_USART_STOP_BITS_2)

#define UART_IS_PARITY(parity) \
  ((parity) == SL_USART_NO_PARITY || (parity) == SL_USART_EVEN_PARITY || (parity) == SL_USART_ODD_PARITY)

#define UART_IS_HW_FLOW_CTRL(hw_flow_ctrl)                                                     \
  ((hw_flow_ctrl) == SL_USART_FLOW_CONTROL_NONE || (hw_flow_ctrl) == SL_USART_FLOW_CONTROL_CTS \
   || (hw_flow_ctrl) == SL_USART_FLOW_CONTROL_RTS || (hw_flow_ctrl) == SL_USART_FLOW_CONTROL_RTS_CTS)

/******************************************************
 *               Variable Definitions
 ******************************************************/

extern uint8_t rx_char;
static sl_usart_handle_t usart_handle;
static sl_si91x_usart_control_config_t usart_config = { .baudrate      = UART_BAUD_RATE_DEFAULT,
                                                        .mode          = UART_MODE_DEFAULT,
                                                        .databits      = UART_DATA_BITS_DEFAULT,
                                                        .misc_control  = UART_MISCONTROL_DEFAULT,
                                                        .stopbits      = UART_STOP_BITS_DEFAULT,
                                                        .parity        = UART_PARITY_DEFAULT,
                                                        .hwflowcontrol = UART_HW_FLOW_CTRL_DEFAULT };

/******************************************************
 *               Function Definitions
 ******************************************************/

extern void ARM_UART_SignalEvent(uint32_t event);

static sl_status_t uart_apply_configuration(usart_peripheral_t instance,
                                            sl_usart_handle_t *handle,
                                            sl_si91x_usart_control_config_t *configuration)
{
  sl_status_t status = sl_si91x_usart_init(instance, handle);
  VERIFY_STATUS_AND_RETURN(status);

  sl_si91x_usart_multiple_instance_unregister_event_callback(instance);
  status = sl_si91x_usart_multiple_instance_register_event_callback(instance, ARM_UART_SignalEvent);
  VERIFY_STATUS_AND_RETURN(status);

  status = sl_si91x_usart_set_configuration(*handle, configuration);
  VERIFY_STATUS_AND_RETURN(status);

  status = sl_si91x_usart_receive_data(*handle, (void *)&rx_char, 1);
  VERIFY_STATUS_AND_RETURN(status);

  return status;
}

void __wrap_Board_Debug_Init()
{
  (void)uart_apply_configuration(UART_PERIPHERAL, &usart_handle, &usart_config);
}

// at+uart-baud=<baud-rate>
sl_status_t uart_baud_command_handler(console_args_t *arguments)
{
  CHECK_ARGUMENT_BITMAP(arguments, 0x01);

  uint32_t baud_rate    = GET_OPTIONAL_COMMAND_ARG(arguments, 0, UART_BAUD_RATE_DEFAULT, uint32_t);
  usart_config.baudrate = baud_rate;

  // Should send before apply configuration
  PRINT_OK_WITH_CRLF;

  (void)uart_apply_configuration(UART_PERIPHERAL, &usart_handle, &usart_config);

  return SL_STATUS_OK;
}

// at+uart-databits=<data-bits>
sl_status_t uart_databits_command_handler(console_args_t *arguments)
{
  CHECK_ARGUMENT_BITMAP(arguments, 0x01);

  usart_databits_typedef_t data_bits =
    GET_OPTIONAL_COMMAND_ARG(arguments, 0, UART_DATA_BITS_DEFAULT, usart_databits_typedef_t);

  // Validate data bits
  if (!UART_IS_DATA_BITS(data_bits)) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  usart_config.databits = data_bits;

  // Should send before apply configuration
  PRINT_OK_WITH_CRLF;

  (void)uart_apply_configuration(UART_PERIPHERAL, &usart_handle, &usart_config);

  return SL_STATUS_OK;
}

// at+uart-stopbits=<stop-bits>
sl_status_t uart_stopbits_command_handler(console_args_t *arguments)
{
  CHECK_ARGUMENT_BITMAP(arguments, 0x01);

  usart_stopbit_typedef_t stop_bits =
    GET_OPTIONAL_COMMAND_ARG(arguments, 0, UART_STOP_BITS_DEFAULT, usart_stopbit_typedef_t);

  // Validate stop bits
  if (!UART_IS_STOP_BITS(stop_bits)) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  usart_config.stopbits = stop_bits;

  // Should send before apply configuration
  PRINT_OK_WITH_CRLF;

  (void)uart_apply_configuration(UART_PERIPHERAL, &usart_handle, &usart_config);

  return SL_STATUS_OK;
}

// at+uart-parity=<parity>
sl_status_t uart_parity_command_handler(console_args_t *arguments)
{
  CHECK_ARGUMENT_BITMAP(arguments, 0x01);

  usart_parity_typedef_t parity = GET_OPTIONAL_COMMAND_ARG(arguments, 0, UART_PARITY_DEFAULT, usart_parity_typedef_t);

  // Validate parity
  if (!UART_IS_PARITY(parity)) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  usart_config.parity = parity;

  // Should send before apply configuration
  PRINT_OK_WITH_CRLF;

  (void)uart_apply_configuration(UART_PERIPHERAL, &usart_handle, &usart_config);

  return SL_STATUS_OK;
}

// at+uart-hwflowctrl=<hw-flow-control>
sl_status_t uart_hwflowctrl_command_handler(console_args_t *arguments)
{
  CHECK_ARGUMENT_BITMAP(arguments, 0x01);

  usart_hwflowcontol_typedef_t hw_flow_ctrl =
    GET_OPTIONAL_COMMAND_ARG(arguments, 0, UART_HW_FLOW_CTRL_DEFAULT, usart_hwflowcontol_typedef_t);

  // Validate hardware flow control
  if (!UART_IS_HW_FLOW_CTRL(hw_flow_ctrl)) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  usart_config.hwflowcontrol = hw_flow_ctrl;

  // Should send before apply configuration
  PRINT_OK_WITH_CRLF;

  (void)uart_apply_configuration(UART_PERIPHERAL, &usart_handle, &usart_config);

  return SL_STATUS_OK;
}

// at+uart-conf?
sl_status_t uart_conf_query_command_handler(console_args_t *arguments)
{
  UNUSED_PARAMETER(arguments);

  sl_si91x_usart_control_config_t config = usart_config;

  sl_status_t status = sl_si91x_usart_get_configurations(UART_PERIPHERAL, &config);
  VERIFY_STATUS_AND_RETURN(status);

  // Display current UART configuration
  PRINT_AT_CMD_SUCCESS;
  AT_PRINTF("%" PRIu32 " %d %d %d %d %d %d %" PRIu32 " %d %d\r\n",
            config.baudrate,
            config.mode,
            config.parity,
            config.stopbits,
            config.hwflowcontrol,
            config.databits,
            config.misc_control,
            config.usart_module,
            config.config_enable,
            config.synch_mode);

  return SL_STATUS_OK;
}
