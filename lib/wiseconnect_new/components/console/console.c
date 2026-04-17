/*******************************************************************************
* @file  console.c
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

#include "console.h"
#include "sl_ieee802_types.h"
#include "sl_ip_types.h"
#include "sl_constants.h"
#include "sl_string.h"
#include "sl_utility.h"
#include <stdint.h>
#include <stddef.h>
#include <limits.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
/******************************************************
 *                      Macros
 ******************************************************/

#define MIN(a, b) ((a) < (b) ? (a) : (b))

/******************************************************
 *                    Constants
 ******************************************************/

#ifndef COMMAND_HISTORY_BUFFER_SIZE
#define COMMAND_HISTORY_BUFFER_SIZE 128
#endif

/******************************************************
 *                   Enumerations
 ******************************************************/

/******************************************************
 *                 Type Definitions
 ******************************************************/

/******************************************************
 *                    Structures
 ******************************************************/

typedef struct {
  uint8_t length;
  uint8_t data[];
} console_history_entry_t;

/******************************************************
 *               Static Function Declarations
 ******************************************************/

static inline uint8_t parse_enum_arg(const char *line, const char *const *options);

char *string_token(register char *s, register const char *delim, char **lasts);
sl_status_t console_tokenize(char *start,
                             const char *end,
                             char **token,
                             char **token_end,
                             sl_console_tokenize_options_t options);

/******************************************************
 *               Variable Definitions
 ******************************************************/

const char *const console_argument_type_strings[] = {
  [CONSOLE_ARG_NONE]                   = "",
  [CONSOLE_ARG_UINT]                   = "uint",
  [CONSOLE_ARG_INT]                    = "int",
  [CONSOLE_ARG_STRING]                 = "string",
  [CONSOLE_ARG_MAC_ADDRESS]            = "xx:xx:xx:xx:xx",
  [CONSOLE_ARG_IP_ADDRESS]             = "x.x.x.x",
  [CONSOLE_ARG_HEX]                    = "0x...",
  [CONSOLE_ARG_REMAINING_COMMAND_LINE] = "...",
};

char console_history_buffer[COMMAND_HISTORY_BUFFER_SIZE];

uint32_t console_history_first = 0xFFFFFFFF;
uint32_t console_history_last  = 0xFFFFFFFF;
uint32_t console_history_end   = 0;

extern const arg_list_t console_argument_types[];
extern const value_list_t console_argument_values[];

/******************************************************
 *               Function Definitions
 ******************************************************/

sl_status_t console_parse_command(char *command_line,
                                  const console_database_t *db,
                                  console_args_t *args,
                                  const console_descriptive_command_t **output_command)
{
  sl_status_t status                    = SL_STATUS_OK;
  const console_database_entry_t *entry = NULL;
  char *token;
  uint32_t index;
  const char *command_line_end = command_line + sl_strlen(command_line); // The end should be passed in by the user

#ifdef CONSOLE_SUB_COMMAND_SUPPORT
#endif

  args->bitmap = 0;

  // Try find matching command
  index  = 0;
  status = console_find_command(&command_line, command_line_end, db, &entry, &index);
  if (status != SL_STATUS_OK)
    return status;

  *output_command                              = (const console_descriptive_command_t *)entry->value;
  const console_argument_type_t *argument_list = &((*output_command)->argument_list[0]);

  // Try parse command args
  int arg_index = 0;
  int arg_count = 0;
  console_argument_type_t type;

  while (true) {
    type = argument_list[arg_index];

    SL_ASSERT(arg_count < SL_SI91X_CLI_CONSOLE_MAX_ARG_COUNT, "Command has too many args");

    if (type == CONSOLE_ARG_REMAINING_COMMAND_LINE) {
      args->bitmap |= (1 << arg_count);
      args->arg[arg_count] = (uint32_t)(command_line + 1);
      return SL_STATUS_OK;
    }

    status = console_tokenize(command_line, command_line_end, &token, &command_line, SL_CONSOLE_TOKENIZE_ON_SPACE);

    // If no more tokens are available, verify there are only optional arguments left
    if (status != SL_STATUS_OK) {
      if (type == CONSOLE_ARG_END) {
        return SL_STATUS_OK;
      } else if (type & CONSOLE_ARG_OPTIONAL) {
        arg_index += 2;
        ++arg_count;
        continue;
      }
      return SL_STATUS_COMMAND_IS_INVALID;
    }

    // Check for optional argument
    if (token[0] == '-' && ((token[1] >= 'a' && token[1] <= 'z') || (token[1] >= 'A' && token[1] <= 'Z'))) {
      // Find optional argument descriptor
      for (uint32_t a = 0, arg_number = 0; argument_list[a] != CONSOLE_ARG_END; ++a, ++arg_number) {
        if (!(argument_list[a] & CONSOLE_ARG_OPTIONAL)) {
          continue;
        }
        if ((argument_list[a] & 0x7F) != token[1]) {
          ++a;
          continue;
        }
        // Found a match!
        args->bitmap |= (1 << arg_number);
        status = console_tokenize(command_line, command_line_end, &token, &command_line, SL_CONSOLE_TOKENIZE_ON_SPACE);
        if (status == SL_STATUS_OK) {
          status = console_parse_arg(argument_list[a + 1], token, &args->arg[arg_number]);
          if (status != SL_STATUS_OK) {
            return status;
          }
        }
        break;
      }
      continue;
    }

    // Verify current ordered argument is not an optional argument
    while (type & CONSOLE_ARG_OPTIONAL) {
      arg_index += 2;
      ++arg_count;
      type = argument_list[arg_index];
      if (type == CONSOLE_ARG_END) {
        return SL_STATUS_OK;
      }
    }

    status = console_parse_arg(type, token, &args->arg[arg_count]);
    if (status == SL_STATUS_OK) {
      args->bitmap |= (1 << arg_count);
    }
    ++arg_index;
    ++arg_count;
  }
}

void console_add_to_history(const char *line, uint8_t line_length)
{
  const uint32_t entry_length = (line_length + sizeof(console_history_entry_t) + 1);

  // Ensure there is enough space between the end and the start
  if (console_history_first != 0xFFFFFFFF) {
    uint32_t available_space = (console_history_first == 0xFFFFFFFF)
                                 ? sizeof(console_history_buffer)
                                 : (sizeof(console_history_buffer) + console_history_first - console_history_end)
                                     % sizeof(console_history_buffer);

    // Drop items from the start until there is enough available space
    while (available_space <= entry_length) {
      SL_ASSERT(((console_history_first < sizeof(console_history_first)) && (console_history_first != 0xFFFFFFFF)),
                "BAD");
      available_space += console_history_buffer[console_history_first] + sizeof(console_history_entry_t) + 1;
      console_history_first =
        (console_history_first + console_history_buffer[console_history_first] + sizeof(console_history_entry_t) + 1)
        % sizeof(console_history_buffer);
    }
  }

  // Write the entry header. There end should always point to the first available byte
  console_history_last = console_history_end;
  if (console_history_first == 0xFFFFFFFF) {
    console_history_first = console_history_last;
  }
  console_history_buffer[console_history_end] = line_length;

  console_history_end = (console_history_end + 1) % sizeof(console_history_buffer);

  // First contiguous copy
  uint8_t copy_length = (uint8_t)MIN((sizeof(console_history_buffer) - console_history_end), line_length);
  memcpy(&console_history_buffer[console_history_end], line, copy_length);
  console_history_end = (console_history_end + line_length) % sizeof(console_history_buffer);

  // Check if we need to copy more because we need to wrap around the end of the buffer
  if (copy_length != line_length) {
    memcpy(&console_history_buffer[0], line + copy_length, line_length - copy_length);
  }

  console_history_buffer[console_history_end] = line_length;
  console_history_end                         = (console_history_end + 1) % sizeof(console_history_buffer);
}

// Simple linear database search. This function could be replaced with a more complicated version
sl_status_t console_find_command(char **string,
                                 const char *string_end,
                                 const console_database_t *db,
                                 const console_database_entry_t **entry,
                                 uint32_t *starting_index)
{
  const console_descriptive_command_t *output_command;
  sl_status_t status;
  char *token;

  // Validate inputs
  if ((string == NULL) || (string_end == NULL) || (starting_index == NULL))
    return SL_STATUS_FAIL;

  char *iter = *string;

  do {
    *entry = NULL;

    // Find first token
    status =
      console_tokenize(iter, string_end, &token, &iter, SL_CONSOLE_TOKENIZE_ON_SPACE | SL_CONSOLE_TOKENIZE_ON_DOT);
    if (status != SL_STATUS_OK)
      return SL_STATUS_FAIL;

    size_t token_length = strlen(token);

    for (uint32_t i = 0; i < db->length; i++) {
      if (strncmp(token, db->entries[i].key, token_length) == 0) {
        *entry          = &(db->entries[i]);
        *starting_index = i;

        if (strlen(db->entries[i].key) == token_length) {
          break;
        } else {
          return SL_STATUS_COMMAND_INCOMPLETE;
        }
      }
    }
    if (*entry == NULL) {
      return SL_STATUS_FAIL;
    }

    output_command                               = (const console_descriptive_command_t *)(*entry)->value;
    const console_argument_type_t *argument_list = &(output_command->argument_list[0]);
    console_argument_type_t type;
    type = argument_list[0];
    if (type == CONSOLE_ARG_SUB_COMMAND) {
      iter = iter + 1;
      db   = output_command->sub_command_database;
      if (strlen(iter) != 0) {
        continue;
      }
    }
    break;
  } while (iter != NULL);
  *string = iter;
  return SL_STATUS_OK;
}

sl_status_t console_parse_arg(console_argument_type_t type, char *line, uint32_t *arg_result)
{
  if (type == CONSOLE_ARG_NONE) {
    *arg_result = 0;
    return SL_STATUS_OK;
  }

  if (type & CONSOLE_ARG_ENUM) {
    uint8_t enum_index  = type & CONSOLE_ARG_ENUM_INDEX_MASK;
    uint8_t value_index = parse_enum_arg(line, console_argument_types[enum_index]);
    if (value_index == 0xFF) {
      return SL_STATUS_COMMAND_IS_INVALID;
    }
    if (console_argument_values[enum_index] == NULL) {
      *arg_result = value_index;
    } else {
      *arg_result = console_argument_values[enum_index][value_index];
    }
    return SL_STATUS_OK;
  }

  // Otherwise parse standard arg types
  switch (type & CONSOLE_ARG_ENUM_INDEX_MASK) {
    case CONSOLE_ARG_UINT:
      *arg_result = strtoul(line, 0, 0);
      break;

    case CONSOLE_ARG_INT:
      *arg_result = strtoul(line, 0, 0);
      break;

    case CONSOLE_ARG_STRING:
      *arg_result = (uint32_t)line;
      break;

    case CONSOLE_ARG_MAC_ADDRESS: {
      sl_mac_address_t *temp_mac = (sl_mac_address_t *)line; // Write the converted value back into the line
      *arg_result                = (uint32_t)temp_mac;
      return convert_string_to_mac_address(line, temp_mac);
    }

    case CONSOLE_ARG_IP_ADDRESS: {
      sl_ipv4_address_t temp_ip = { 0 }; // Write the converted value back into the line
      sl_status_t status        = convert_string_to_sl_ipv4_address(line, &temp_ip);
      *arg_result               = temp_ip.value;
      return status;
    }

    case CONSOLE_ARG_HEX: {
      if (line[0] == '0' && line[1] == 'x') {
        line += 2;
      }
      *arg_result = strtoul(line, NULL, 16);
    } break;

    default:
      return SL_STATUS_COMMAND_IS_INVALID;
  }
  return SL_STATUS_OK;
}

static inline uint8_t parse_enum_arg(const char *line, const char *const *options)
{
  for (uint8_t a = 0; options[a] != NULL; ++a) {
    if (strcmp(line, options[a]) == 0) {
      return a;
    }
  }
  return 0xFF;
}

static bool escape(char *i, const char *end)
{
  // Check if next character is one of the escaped ones
  if (i[1] == '"' || i[1] == '{' || i[1] == '\\') {
    // Next input matches, so shift array
    memmove(i, i + 1, (size_t)(end - i));
    return true;
  } else {
    // Unknown sequence
    return false;
  }
}

sl_status_t console_tokenize(char *start,
                             const char *end,
                             char **token,
                             char **token_end,
                             sl_console_tokenize_options_t options)
{
  // Get size of string and declare variables
  //  uint32_t input_size = strlen( input );
  //  uint32_t i = 0;
  char *i = start;

  // Ignore preceding space or null
  while (*i == ' ' || *i == '\0') {
    ++i;
    if (i >= end) {
      return SL_STATUS_FAIL;
    }
  }
  *token = i;
  while (i < end) {
    if (*i == '{' || *i == '"') {
      // Start of segment where splitting should not be performed
      char end_char;
      if (*i == '{') {
        end_char = '}';
        *token   = i;
      } else {
        end_char = '"';
        // Token ignores " character
        *token = i + 1;
      }
      // Verify that preceding character is space or end of string
      if (i > start && (i[-1] != ' ' && i[-1] != '\0')) {
        return SL_STATUS_INVALID_PARAMETER;
      }

      // Loop through input to find end character
      i++;
      while (*i != end_char) {
        if (*i == '\\') {
          // Next character should be escaped
          if (!escape(i, end)) {
            // Escape error
            return SL_STATUS_INVALID_PARAMETER;
          }
          --end;
        }
        ++i;
        if (i >= end) {
          // End character not found, return error
          return SL_STATUS_INVALID_PARAMETER;
        }
      }
      // Verify that next character is space or end of string
      if (i[1] != ' ' && i[1] != '\0' && &i[1] != end) {
        return SL_STATUS_INVALID_PARAMETER;
      }
      // Remove " from string end
      if (end_char == '"') {
        *i         = '\0';
        *token_end = i;
      } else {
        *token_end = i + 1;
      }
      return SL_STATUS_OK;
    } // Escape symbol encountered
    else if (*i == '\\') {
      if (!escape(i, end)) {
        // Escape error
        return SL_STATUS_INVALID_PARAMETER;
      }
      --end;

      // Ordinary splitting
    } else if (((options & SL_CONSOLE_TOKENIZE_ON_SPACE) && *i == ' ')
               || ((options & SL_CONSOLE_TOKENIZE_ON_DOT) && *i == '.')) {
      // Turn space into '\0' to indicate end of string
      *i         = '\0';
      *token_end = i;
      return SL_STATUS_OK;
    }
    ++i;
  }

  *token_end = i;
  return SL_STATUS_OK;
}
