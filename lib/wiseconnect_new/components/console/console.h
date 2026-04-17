/*******************************************************************************
* @file  console.h
* @brief 
*******************************************************************************
* # License
* <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
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

#pragma once

#include "console_types.h"
#include "sl_status.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************
 *                 Global Variables
 ******************************************************/

// command_database content is generated in cli_command_database.c
extern const console_database_t console_command_database;

extern const char *const console_argument_type_strings[];

extern volatile uint8_t console_line_ready;
extern volatile uint8_t console_data_rx_receive;

/******************************************************
 *               Function Declarations
 ******************************************************/

sl_status_t command_proccessor_init();
sl_status_t console_parse_command(char *command_line,
                                  const console_database_t *db,
                                  console_args_t *args,
                                  const console_descriptive_command_t **output_command);
sl_status_t console_find_command(char **string,
                                 const char *string_end,
                                 const console_database_t *db,
                                 const console_database_entry_t **entry,
                                 uint32_t *starting_index);

sl_status_t console_parse_arg(console_argument_type_t type, char *line, uint32_t *arg_result);
void console_add_to_history(const char *line, uint8_t line_length);
void console_process_uart_data(void);
sl_status_t console_process_buffer(const console_database_t *command_database,
                                   console_args_t *args,
                                   const console_descriptive_command_t **command);
void console_print_command_args(const console_descriptive_command_t *command);

char *console_get_command_buffer(void);
uint32_t console_read_data_from_cache(char *buffer, uint32_t buffer_size);

#ifdef SLI_AT_COMMAND_SUPPORT
sl_status_t console_process_at_command_buffer(const console_database_t *command_database,
                                              console_args_t *args,
                                              const console_descriptive_command_t **command);
#endif
#ifdef __cplusplus
} /*extern "C" */
#endif
