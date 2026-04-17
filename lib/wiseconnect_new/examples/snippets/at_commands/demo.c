/***************************************************************************/ /**
 * @file demo.c
 * @brief AT Commands Application
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

#ifndef SLI_SI91X_MCU_INTERFACE
#include "sl_iostream.h"
#endif
#include "console.h"
#include "sl_constants.h"
#include "sl_status.h"
#include "sl_board_configuration.h"
#include "cmsis_os2.h"
#include "at_command_data_mode.h"
#include "sl_utility.h"
#include "sl_common.h"
#include "at_utility.h"
#include <stdbool.h>
#include <string.h>
#include <inttypes.h>

/******************************************************
 *                    Constants
 ******************************************************/

#define BUFFER_SIZE 256
bool end_of_cmd = false;

#define MY_ARG_TYPE uart, spi, i2c, tcp

#define OFFSETS(a, b, c, d) uint8_t offsets[] = { 0, sizeof(#a), sizeof(#b), sizeof(#c) }

#define UART_BUFFER_CHUNK_SIZE 256

/******************************************************
 *                   Enumerations
 ******************************************************/

/******************************************************
 *                 Type Definitions
 ******************************************************/

/******************************************************
 *                    Structures
 ******************************************************/

/******************************************************
 *               Static Function Declarations
 ******************************************************/

sl_status_t help_command_handler(console_args_t *arguments);
sl_status_t echo_command_handler(console_args_t *arguments);

#ifndef SLI_SI91X_MCU_INTERFACE
extern void cache_uart_rx_data(const char character);
#endif

void print_command_args(const console_descriptive_command_t *command);

void application_start(const void *unused);
void print_status(sl_status_t status);

/******************************************************
 *               Variable Definitions
 ******************************************************/
bool echo_enable = 0;

/******************************************************
 *               Function Definitions
 ******************************************************/

const osThreadAttr_t thread_attributes = {
  .name       = "application_thread",
  .attr_bits  = 0,
  .cb_mem     = 0,
  .cb_size    = 0,
  .stack_mem  = 0,
  .stack_size = 3072,
  .priority   = 0,
  .tz_module  = 0,
  .reserved   = 0,
};

void app_init(void)
{
  print_mutex = osMutexNew(NULL);
  assert(print_mutex != NULL);
  osThreadId_t thread_id = osThreadNew((osThreadFunc_t)application_start, NULL, &thread_attributes);
  assert(thread_id != NULL);
}

#ifndef SLI_SI91X_MCU_INTERFACE
void iostream_usart_init()
{
  /* Prevent buffering of output/input.*/
#if !defined(__CROSSWORKS_ARM) && defined(__GNUC__)
  setvbuf(stdout, NULL, _IONBF, 0); /*Set unbuffered mode for stdout (newlib)*/
  setvbuf(stdin, NULL, _IONBF, 0);  /*Set unbuffered mode for stdin (newlib)*/
#endif
}

void iostream_rx()
{
  char c               = 0;
  static uint8_t index = 0;
  sl_iostream_getchar(SL_IOSTREAM_STDIN, &c);
  if (c > 0) {
    cache_uart_rx_data(c);
    if ((c == '\n')) {
      index      = 0;
      end_of_cmd = true;
    } else {
      if (index < BUFFER_SIZE - 1) {
        index++;
      }
    }
  }
}
#endif

void application_start(const void *unused)
{
  UNUSED_PARAMETER(unused);
  console_args_t args;
  const console_descriptive_command_t *command;
#ifndef SLI_SI91X_MCU_INTERFACE
  iostream_usart_init();
#endif

  SL_DEBUG_LOG("app start\n");

  AT_PRINTF("Ready\r\n");
  console_line_ready = 0;

  while (1) {
    AT_PRINTF("\r\n> \r\n");
#ifndef SLI_SI91X_MCU_INTERFACE
    while (!end_of_cmd) {
      iostream_rx();
    }
    end_of_cmd = false;
#endif

    while (at_command_data_mode_process()) {
      osDelay(20);
    }

    while (!console_line_ready) {
      console_process_uart_data();
      osDelay(20);
    }

    sl_status_t result = console_process_at_command_buffer(&console_command_database, &args, &command);

    if (result == SL_STATUS_OK) {
      SL_DEBUG_LOG("Processing command\n");
      if (command->handler) {
        result = command->handler(&args);
        print_status(result);
      }
    } else if (result == SL_STATUS_COMMAND_IS_INVALID) {
      print_status(SL_STATUS_INVALID_PARAMETER);
    } else {
      AT_PRINTF("\r\nNot supported\r\n");
    }
    console_line_ready = 0;
  }
}

void print_status(sl_status_t status)
{
  if (status != SL_STATUS_OK) {
    AT_PRINTF("\r\n%s %" PRIi32 "\r\n", "ERROR", (int32_t)status);
  }
}

// at+echo=<echo-flag>
sl_status_t echo_command_handler(console_args_t *arguments)
{
  echo_enable = GET_OPTIONAL_COMMAND_ARG(arguments, 0, 0, bool);
  PRINT_AT_CMD_SUCCESS;
  return SL_STATUS_OK;
}

// at+ready?
sl_status_t ready_command_handler(console_args_t *arguments)
{
  UNUSED_PARAMETER(arguments);
  PRINT_AT_CMD_SUCCESS;
  return SL_STATUS_OK;
}

sl_status_t help_command_handler(console_args_t *arguments)
{
  UNUSED_PARAMETER(arguments);
  for (uint8_t a = 0; a < console_command_database.length; ++a) {
    AT_PRINTF("\r\n");
    AT_PRINTF("at+%s", console_command_database.entries[a].key);
    AT_PRINTF("  ");
    print_command_args((console_descriptive_command_t *)console_command_database.entries[a].value);
  }
  return SL_STATUS_OK;
}

extern const arg_list_t console_argument_types[];
void print_command_args(const console_descriptive_command_t *command)
{
  bool is_optional = false;
  for (int a = 0; command->argument_list[a] != CONSOLE_ARG_END; ++a) {
    if (command->argument_list[a] & CONSOLE_ARG_OPTIONAL) {
      char option_char[2] = { (char)command->argument_list[a] & CONSOLE_ARG_OPTIONAL_CHARACTER_MASK, 0 };
      AT_PRINTF("[-");
      AT_PRINTF("%s", option_char);
      AT_PRINTF(" ");
      is_optional = true;
      continue;
    } else if (command->argument_list[a] & CONSOLE_ARG_ENUM) {
      AT_PRINTF("{");
      uint8_t enum_index = command->argument_list[a] & CONSOLE_ARG_ENUM_INDEX_MASK;
      for (int b = 0; console_argument_types[enum_index][b] != NULL;
           /* Increment occurs in internal logic */) {
        AT_PRINTF("%s", console_argument_types[enum_index][b]);
        if (console_argument_types[enum_index][++b]) {
          AT_PRINTF("|");
        }
      }
      AT_PRINTF("}");
    } else {
      AT_PRINTF("<");
      if (command->argument_help && command->argument_help[a]) {
        AT_PRINTF("%s", command->argument_help[a]);
      } else {
        AT_PRINTF(console_argument_type_strings[command->argument_list[a] & CONSOLE_ARG_ENUM_INDEX_MASK]);
      }
      AT_PRINTF(">");
    }
    if (is_optional) {
      AT_PRINTF("] ");
      is_optional = false;
    } else {
      AT_PRINTF(" ");
    }
  }
}
