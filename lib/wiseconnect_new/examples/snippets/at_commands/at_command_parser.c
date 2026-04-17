/*******************************************************************************
 * @file  at_command_parser.c
 * @brief 
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
#include "console.h"
#include "sl_ieee802_types.h"
#include "sl_constants.h"
#include "sl_status.h"
#include "sl_string.h"
#include "sl_utility.h"
#include <stdbool.h>
#include <string.h>
#include <strings.h>

/******************************************************
 *                    Constants
 ******************************************************/
#define AT_CMD_PREFIX     "at+"
#define AT_CMD_PREFIX_LEN sl_strlen(AT_CMD_PREFIX)

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
static sl_status_t at_command_tokenize(char *start,
                                       const char *end,
                                       char **token,
                                       char **token_end,
                                       sl_console_tokenize_options_t options);

static sl_status_t console_find_at_command(char **string,
                                           const char *string_end,
                                           const console_database_t *db,
                                           const console_database_entry_t **entry,
                                           uint32_t *starting_index);

static bool escape(char *i, const char *end);

static inline uint8_t at_command_parse_enum_arg(const char *line, const char *const *options);

static sl_status_t at_command_strtoul(unsigned long *out_val, const char *str, int base);

static sl_status_t at_command_parse_arg(console_argument_type_t type, char *line, uint32_t *arg_result);

/******************************************************
 *               Variable Definitions
 ******************************************************/

extern const arg_list_t console_argument_types[];
extern const value_list_t console_argument_values[];

/******************************************************
 *               Function Definitions
 ******************************************************/

sl_status_t console_parse_at_command(char *command_line,
                                     const console_database_t *db,
                                     console_args_t *args,
                                     const console_descriptive_command_t **output_command);

// Simple linear database search. This function could be replaced with a more complicated version
sl_status_t console_find_at_command(char **string,
                                    const char *string_end,
                                    const console_database_t *db,
                                    const console_database_entry_t **entry,
                                    uint32_t *starting_index)
{
  const console_descriptive_command_t *output_command;
  sl_status_t status;
  char *token = NULL;

  // Validate inputs
  if ((string == NULL) || (string_end == NULL) || (starting_index == NULL))
    return SL_STATUS_FAIL;

  char *iter = *string;

  do {
    *entry = NULL;

    // Find first token
    status =
      at_command_tokenize(iter, string_end, &token, &iter, SL_CONSOLE_TOKENIZE_ON_COMMA | SL_CONSOLE_TOKENIZE_ON_EQUAL);
    if (status != SL_STATUS_OK)
      return SL_STATUS_FAIL;

    size_t token_length = strlen(token);

    for (uint32_t i = 0; i < db->length; i++) {
      if (strncasecmp(token, db->entries[i].key, token_length) == 0 && db->entries[i].key[token_length] == 0) {
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

static bool escape(char *i, const char *end)
{
  if (i + 1 >= end) {
    // No next character
    return false;
  }

  // Check if next character is one of the escaped ones
  if (i[1] == '"' || i[1] == '{' || i[1] == '\\') {
    // Next input matches, so shift array
    memmove(i, i + 1, (size_t)(end - i - 1));
    return true;
  } else {
    // Unknown sequence
    return false;
  }
}

static sl_status_t at_command_tokenize(char *start,
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
  while (i < end && (*i == ' ' || *i == '\0')) {
    ++i;
  }
  if (i >= end) {
    return SL_STATUS_FAIL;
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
      // Verify that preceding character is comma or end of string
      if (i > start && (i[-1] != ',' && i[-1] != '\0')) {
        return SL_STATUS_INVALID_PARAMETER;
      }

      // Loop through input to find end character
      i++;
      while (i < end && *i != end_char) {
        if (*i == '\\') {
          // Next character should be escaped
          if (!escape(i, end)) {
            // Escape error
            return SL_STATUS_INVALID_PARAMETER;
          }
          --end;
        }
        ++i;
      }
      if (i >= end) {
        // End character not found, return error
        return SL_STATUS_INVALID_PARAMETER;
      }
      // Verify that next character is comma or end of string
      if ((i + 1 < end) && (i[1] != ',') && (i[1] != '\0')) {
        return SL_STATUS_INVALID_PARAMETER;
      }
      // Remove " from string end
      if (end_char == '"') {
        *i = '\0';
        // Remove comma incase it is followed by "
        if ((i + 1 < end) && (i[1] == ',')) {
          i[1]       = '\0';
          *token_end = i + 1;
        } else {
          *token_end = i;
        }
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
    } else if (((options & SL_CONSOLE_TOKENIZE_ON_EQUAL) && *i == '=')
               || ((options & SL_CONSOLE_TOKENIZE_ON_COMMA) && *i == ',')) {
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

static inline uint8_t at_command_parse_enum_arg(const char *line, const char *const *options)
{
  for (uint8_t a = 0; options[a] != NULL; ++a) {
    if (strcmp(line, options[a]) == 0) {
      return a;
    }
  }
  return 0xFF;
}

static sl_status_t at_command_strtoul(unsigned long *out_val, const char *str, int base)
{
  if (out_val == NULL) {
    return SL_STATUS_COMMAND_IS_INVALID;
  }

  if ((str == NULL) || (*str == '\0')) {
    *out_val = 0;
    return SL_STATUS_OK;
  }

  char *end         = NULL;
  unsigned long val = strtoul(str, &end, base);

  if ((end == str) || (*end != '\0')) {
    return SL_STATUS_COMMAND_IS_INVALID;
  }

  *out_val = val;
  return SL_STATUS_OK;
}

static sl_status_t at_command_parse_arg(console_argument_type_t type, char *line, uint32_t *arg_result)
{
  if (type == CONSOLE_ARG_NONE) {
    *arg_result = 0;
    return SL_STATUS_OK;
  }

  if (type & CONSOLE_ARG_ENUM) {
    uint8_t enum_index  = type & CONSOLE_ARG_ENUM_INDEX_MASK;
    uint8_t value_index = at_command_parse_enum_arg(line, console_argument_types[enum_index]);
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
      return at_command_strtoul(arg_result, line, 0);

    case CONSOLE_ARG_INT:
      return at_command_strtoul(arg_result, line, 0);

    case CONSOLE_ARG_STRING:
      *arg_result = (uint32_t)line;
      break;

    case CONSOLE_ARG_MAC_ADDRESS: {
      sl_mac_address_t temp_mac = { 0 };
      sl_status_t status        = convert_string_to_mac_address(line, &temp_mac);
      if (status == SL_STATUS_OK) {
        memcpy(line, &temp_mac, sizeof(temp_mac));
        *arg_result = (uint32_t)line;
      }

      return status;
    }

    case CONSOLE_ARG_IP_ADDRESS: {
      sl_ipv4_address_t temp_ip = { 0 }; // Write the converted value back into the line
      sl_status_t status        = convert_string_to_sl_ipv4_address(line, &temp_ip);
      if (status == SL_STATUS_OK) {
        *arg_result = temp_ip.value;
      }

      return status;
    }

    case CONSOLE_ARG_HEX: {
      if (line[0] == '0' && line[1] == 'x') {
        line += 2;
      }
      return at_command_strtoul(arg_result, line, 16);
    }

    default:
      return SL_STATUS_COMMAND_IS_INVALID;
  }
  return SL_STATUS_OK;
}

extern bool echo_enable;

void parse_echo_command(char *command_line)
{
  if (echo_enable == 1) {
    printf("%s\r\n", command_line);
  }
}

sl_status_t console_parse_at_command(char *command_line,
                                     const console_database_t *db,
                                     console_args_t *args,
                                     const console_descriptive_command_t **output_command)
{
  sl_status_t status                    = SL_STATUS_OK;
  const console_database_entry_t *entry = NULL;
  char *token;
  uint32_t index;
  const char *command_line_end = command_line + sl_strlen(command_line); // The end should be passed in by the user

  parse_echo_command(command_line);
  if (0 == strncasecmp(command_line, AT_CMD_PREFIX, AT_CMD_PREFIX_LEN)) {
    command_line += AT_CMD_PREFIX_LEN;
  } else {
    return SL_STATUS_FAIL;
  }

  args->bitmap = 0;

  // Try find matching command
  index  = 0;
  status = console_find_at_command(&command_line, command_line_end, db, &entry, &index);
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

    status = at_command_tokenize(command_line, command_line_end, &token, &command_line, SL_CONSOLE_TOKENIZE_ON_COMMA);

    // If no more tokens are available, verify there are only optional arguments left
    if (status != SL_STATUS_OK) {
      if (type == CONSOLE_ARG_END) {
        return SL_STATUS_OK;
      } else if (type & CONSOLE_ARG_OPTIONAL) {
        arg_index += 2;
        ++arg_count;
        continue;
      }
      return SL_STATUS_OK;
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
        status =
          at_command_tokenize(command_line, command_line_end, &token, &command_line, SL_CONSOLE_TOKENIZE_ON_SPACE);
        if (status == SL_STATUS_OK) {
          status = at_command_parse_arg(argument_list[a + 1], token, &args->arg[arg_number]);
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

    status = at_command_parse_arg(type, token, &args->arg[arg_count]);
    if ((status == SL_STATUS_OK) && (token[0] != 0)) {
      args->bitmap |= (1 << arg_count);
    }
    if (status != SL_STATUS_OK) {
      return status;
    }
    ++arg_index;
    ++arg_count;
  }
}
