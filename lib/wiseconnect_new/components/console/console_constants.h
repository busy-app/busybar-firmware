/*******************************************************************************
* @file  console_constants.h
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

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************
 *                     Macros
 ******************************************************/

#define CONSOLE_ENUM_ARG(type)                (CONSOLE_ARG_ENUM | CONSOLE_##type##_TYPE)
#define CONSOLE_OPTIONAL_ARG(character, type) (CONSOLE_ARG_OPTIONAL | ((character)&0x7F)), type
#define CONSOLE_SUB_COMMAND_ARG(command_table)                                           \
  CONSOLE_ARG_SUB_COMMAND, ((command_table >> 0) & 0xFF), ((command_table >> 8) & 0xFF), \
    ((command_table >> 16) & 0xFF), ((command_table >> 24) & 0xFF)

#define CONSOLE_DATABASE_ENTRIES(...)                                                                \
  .length  = sizeof((console_database_entry_t[]){ __VA_ARGS__ }) / sizeof(console_database_entry_t), \
  .entries = { __VA_ARGS__ }

#define CONSOLE_VARIABLE(name_string, var, ...)                   \
  {                                                               \
    .name = name_string, .variable = var, .type = { __VA_ARGS__ } \
  }
#define CONSOLE_VARIABLE_ENUM(name_string, var, type)                                           \
  {                                                                                             \
    .name = name_string, .variable = var, .type = {(CONSOLE_ARG_ENUM | CONSOLE_##type##_TYPE) } \
  }

#define IS_CONSOLE_ARG_VALID(args, arg_number) ((args->bitmap & (1 << arg_number)) != 0)
#define GET_COMMAND_ARG(args, arg_number)      (args->arg[arg_number])
#define GET_OPTIONAL_COMMAND_ARG(args, arg_number, default_value, type) \
  (IS_CONSOLE_ARG_VALID(args, arg_number) ? (type)(args->arg[arg_number]) : (type)(default_value))

#define CONSOLE_AUTO_TABLE_ENTRY(name) { STRINGIFY(name), &name##_command },
#define CONSOLE_AUTO_FUNCTION_DEFINITION(name)               \
  extern const console_descriptive_command_t name##_command; \
  sl_status_t name##_command_handler(console_args_t *arguments)

#define member_size(type, member) sizeof(((type *)0)->member)

#define SL_CONSOLE_STRUCTURE_ENTRY(name, member, struct_type, var_type)             \
  {                                                                                 \
    name, offsetof(struct_type, member), member_size(struct_type, member), var_type \
  }

#define SL_CONSOLE_VARIABLE_GROUP(keystring, _table)                                       \
  {                                                                                        \
    .type = SL_CONSOLE_VARIABLE_GROUP_NODE, .key = keystring, .data.group.table = &_table, \
    .data.group.table_size = sizeof(_table) / sizeof(console_variable_node_t)              \
  }
#define SL_CONSOLE_VARIABLE_RAM_STRUCTURE(keystring, table_object, instance)                                 \
  {                                                                                                          \
    .type = SL_CONSOLE_VARIABLE_RAM_STRUCT_NODE, .key = keystring, .data.ram_structure.table = table_object, \
    .data.ram_structure.table_size = sizeof(table_object) / sizeof(structure_descriptor_entry_t),            \
    .data.ram_structure.object     = &instance                                                               \
  }
#define SL_CONSOLE_VARIABLE_NVM_STRUCTURE(keystring, id, function)                                     \
  {                                                                                                    \
    .type = SL_CONSOLE_VARIABLE_NVM_STRUCT_NODE, .key = keystring, .data.custom_structure.nvm_id = id, \
    .data.custom_structure.handler = (console_variable_handler_t)function                              \
  }
#define SL_CONSOLE_VARIABLE_VARIABLE(keystring, object, size)                                  \
  {                                                                                            \
    .type = SL_CONSOLE_VARIABLE_VARIABLE_NODE, .key = keystring, .data.variable.data = object, \
    .data.variable.data_size = size                                                            \
  }

#define sl_console_get_argument_mac_address(a, n) ((sl_mac_address_t *)(a->arg[n]))

/*
 * SL CLI compatible macros
 */
#define sl_cli_get_argument_int8(a, n)   ((int8_t)(a->arg[n]))
#define sl_cli_get_argument_int16(a, n)  ((int16_t)(a->arg[n]))
#define sl_cli_get_argument_int32(a, n)  ((int32_t)(a->arg[n]))
#define sl_cli_get_argument_uint8(a, n)  ((uint8_t)(a->arg[n]))
#define sl_cli_get_argument_uint16(a, n) ((uint16_t)(a->arg[n]))
#define sl_cli_get_argument_uint32(a, n) ((uint32_t)(a->arg[n]))
#define sl_cli_get_argument_string(a, n) ((char *)(a->arg[n]))
#define sl_cli_get_argument_hex(a, n)    ((uint8_t *)(a->arg[n]))

/******************************************************
 *                    Constants
 ******************************************************/

#ifndef SL_SI91X_CLI_CONSOLE_MAX_ARG_COUNT
#define SL_SI91X_CLI_CONSOLE_MAX_ARG_COUNT (18)
#endif

#define CONSOLE_ARG_OPTIONAL_CHARACTER_MASK 0x7F
#define CONSOLE_ARG_ENUM_INDEX_MASK         0x3F

/******************************************************
 *                   Enumerations
 ******************************************************/

/*
 * Optional arguments use the lower 7 bits to record the character and are then followed by a non-optional argument type
 * e.g.
 */
typedef enum {
  CONSOLE_ARG_NONE        = 0, // Used for optional arguments that are just flags and don't need any further data
  CONSOLE_ARG_UINT        = 1,
  CONSOLE_ARG_INT         = 2,
  CONSOLE_ARG_STRING      = 3,
  CONSOLE_ARG_IP_ADDRESS  = 4,
  CONSOLE_ARG_MAC_ADDRESS = 5,
  CONSOLE_ARG_HEX         = 6,
  CONSOLE_ARG_SUB_COMMAND = 7,
  CONSOLE_ARG_REMAINING_COMMAND_LINE = 8,
  CONSOLE_NUMBER_OF_ARGS_WITH_STRINGS =
    9, // This is the number of argument types that match to a specific string. Other arg types should follow

  CONSOLE_ARG_ENUM     = (1 << 6),
  CONSOLE_ARG_OPTIONAL = (1 << 7),
  CONSOLE_ARG_END      = 0xFF
} console_argument_type_t;

typedef enum {
  CONSOLE_VARIABLE_NONE                   = 0,
  CONSOLE_VARIABLE_UINT                   = 1,
  CONSOLE_VARIABLE_INT                    = 2,
  CONSOLE_VARIABLE_STRING                 = 3,
  CONSOLE_VARIABLE_IP_ADDRESS             = 4,
  CONSOLE_VARIABLE_MAC_ADDRESS            = 5,
  CONSOLE_VARIABLE_HEX                    = 6,
  CONSOLE_VARIABLE_BOOL                   = 7,
  CONSOLE_VARIABLE_REMAINING_COMMAND_LINE = 8,

  CONSOLE_VARIABLE_ENUM     = (1 << 6),
  CONSOLE_VARIABLE_OPTIONAL = (1 << 7),
  CONSOLE_VARIABLE_END      = 0xFF
} console_variable_type_t;

typedef enum {
  SL_CONSOLE_VARIABLE_GROUP_NODE,
  SL_CONSOLE_VARIABLE_RAM_STRUCT_NODE,
  SL_CONSOLE_VARIABLE_CUSTOM_STRUCT_NODE,
  SL_CONSOLE_VARIABLE_NVM_STRUCT_NODE,
  SL_CONSOLE_VARIABLE_VARIABLE_NODE,
} console_variable_node_type_t;

typedef enum {
  SL_CONSOLE_VARIABLE_GET,
  SL_CONSOLE_VARIABLE_SET,
} console_variable_action_t;

typedef enum {
  SL_CONSOLE_TOKENIZE_ON_SPACE = (1 << 0),
  SL_CONSOLE_TOKENIZE_ON_DOT   = (1 << 1),
  SL_CONSOLE_TOKENIZE_ON_EQUAL = (1 << 2),
  SL_CONSOLE_TOKENIZE_ON_COMMA = (1 << 3),
  SL_CONSOLE_TOKENIZE_ON_QUERY = (1 << 4),
} sl_console_tokenize_options_t;

#ifdef __cplusplus
} /*extern "C" */
#endif
