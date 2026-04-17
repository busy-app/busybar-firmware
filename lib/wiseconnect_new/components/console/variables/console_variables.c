/*******************************************************************************
* @file  console_variables.c
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

#define _DEFAULT_SOURCE

#include "console.h"
#include "sl_ip_types.h"
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include "sl_constants.h"

/******************************************************
 *                      Macros
 ******************************************************/

#define member_size(type, member) sizeof(((type *)0)->member)

/******************************************************
 *                    Constants
 ******************************************************/

#define KEY_SEPARATOR "."

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

const structure_descriptor_entry_t *find_structure_entry(const char *key,
                                                         const structure_descriptor_entry_t *table,
                                                         uint32_t table_size);
static void print_variable(const structure_descriptor_entry_t *entry, const void *object);

sl_status_t console_variable_get(const console_args_t *arguments);
sl_status_t console_variable_set(const console_args_t *arguments);

/******************************************************
 *               Variable Definitions
 ******************************************************/

extern const console_variable_node_t console_variable_table[];
extern const uint32_t console_variable_table_size;

/******************************************************
 *               Function Definitions
 ******************************************************/

static sl_status_t find_variable_node(char **key, const console_variable_node_t **node)
{
  char *token_ptr                      = NULL;
  const char *token                    = strtok_r(*key, KEY_SEPARATOR, &token_ptr);
  uint32_t iter                        = 0;
  const console_variable_node_t *table = console_variable_table;
  uint32_t table_size                  = console_variable_table_size;

  // Process key by traversing through the CLI variable table
  while (token != NULL) {
    // Check for a key match
    if (strcmp(token, table[iter].key) == 0) {
      if (table[iter].type == SL_CONSOLE_VARIABLE_GROUP_NODE) {
        table      = table[iter].data.group.table;
        table_size = table[iter].data.group.table_size;
      } else {
        *key  = strtok_r(NULL, KEY_SEPARATOR, &token_ptr);
        *node = &table[iter];
        return SL_STATUS_OK;
      }
    } else {
      ++iter;
      if (iter > table_size)
        return SL_STATUS_ABORT;
      continue;
    }

    token = strtok_r(NULL, KEY_SEPARATOR, &token_ptr);
  }

  return SL_STATUS_ABORT;
}

#ifdef SLI_SI91X_CONSOLE_SUBCOMMANDS
/*
 * get <variable>
 * set <variable> [item=value] [item2=value2]
 */
static sl_status_t process_structure_args(console_variable_action_t action,
                                          console_args_t *arguments,
                                          void *object,
                                          const structure_descriptor_entry_t *table,
                                          uint32_t table_size)
{
  // Check if no sub-sections are named. If so, print the full structure
  if (!IS_CONSOLE_ARG_VALID(arguments, 2)) {
    printf("\n");
    for (unsigned int a = 0; a < table_size; ++a) {
      printf("  %s: ", table[a].key);
      print_variable(&table[a], object);
    }
    return SL_STATUS_OK;
  }

  for (uint32_t index = 1; IS_CONSOLE_ARG_VALID(arguments, index); ++index) {
    char *temp = (char *)arguments->arg[index];

    // Find the key
    const char *key = strtok(temp, "=");

    // Find entry in the structure
    const structure_descriptor_entry_t *entry = find_structure_entry(key, table, table_size);

    if (entry == NULL) {
      printf("Struct does not contain %s\n", key);
      return SL_STATUS_ABORT;
    }

    // Do stuff
    switch (action) {
      case SL_CONSOLE_VARIABLE_GET:
        printf("\t%s: ", entry->key);
        print_variable(entry, object);
        break;

      case SL_CONSOLE_VARIABLE_SET:
        temp = strtok(NULL, " ");
        if (temp != NULL) {
          //          uint32_t memory;
          //          void *argv[1];
          //          int mem_index = 0;
          //          int arg_index = 0;
          //          if (sli_cli_validate_argument(entry->type, temp)) {
          //            sli_cli_arguments_convert_single(entry->type, temp, argv, &memory, &arg_index, &mem_index);
          //            if (entry->type == CONSOLE_ARG_STRING) {
          //              memcpy(object + entry->offset, argv[0], SL_MIN(entry->size, strlen((char*)argv[0])));
          //            } else {
          //              memcpy(object + entry->offset, argv[0], entry->size);
          //            }
          //          }
        }
        break;

      default:
        break;
    }
  }
  return SL_STATUS_OK;
}
#endif

static void print_variable(const structure_descriptor_entry_t *entry, const void *object)
{
  switch (entry->type) {
    case CONSOLE_VARIABLE_BOOL:
      if (*(const uint8_t *)(object + entry->offset)) {
        printf("true\n");
      } else {
        printf("false\n");
      }
      break;

    case CONSOLE_VARIABLE_UINT:
      switch (entry->size) {
        case 1:
          printf("0x%X\n", *(const uint8_t *)(object + entry->offset));
          break;
        case 2:
          printf("0x%X\n", *(const uint16_t *)(object + entry->offset));
          break;
        case 4:
          printf("0x%lX\n", *(const uint32_t *)(object + entry->offset));
          break;
        default:
          break;
      }
      break;

    case CONSOLE_VARIABLE_INT:
      switch (entry->size) {
        case 1:
          printf("%d\n", *(const int8_t *)(object + entry->offset));
          break;
        case 2:
          printf("%d\n", *(const int16_t *)(object + entry->offset));
          break;
        case 4:
          printf("%ld\n", *(const int32_t *)(object + entry->offset));
          break;
        default:
          break;
      }
      break;

    case CONSOLE_VARIABLE_STRING:
      printf("%s\n", (const char *)(object + entry->offset));
      break;

    case CONSOLE_VARIABLE_IP_ADDRESS: {
      const sl_ipv4_address_t *temp = (const sl_ipv4_address_t *)(object + entry->offset);
      printf("%d.%d.%d.%d\n", temp->bytes[0], temp->bytes[1], temp->bytes[2], temp->bytes[3]);
    } break;

    default:
      printf("\n");
      break;
  }
}

const structure_descriptor_entry_t *find_structure_entry(const char *key,
                                                         const structure_descriptor_entry_t *table,
                                                         uint32_t table_size)
{
  for (unsigned int a = 0; a < table_size; ++a) {
    if (strcmp(key, table[a].key) == 0)
      return &table[a];
  }

  return NULL;
}

sl_status_t console_variable_get(const console_args_t *arguments)
{
  const console_variable_node_t *node;
  char *key          = (char *)arguments->arg[0];
  sl_status_t status = find_variable_node(&key, &node);
  if (status != SL_STATUS_OK)
    return status;

  if (node->type == SL_CONSOLE_VARIABLE_RAM_STRUCT_NODE) {
    const structure_descriptor_entry_t *entry =
      find_structure_entry(key, node->data.ram_structure.table, node->data.ram_structure.table_size);
    print_variable(entry, node->data.ram_structure.object);
  } else if (node->type == SL_CONSOLE_VARIABLE_NVM_STRUCT_NODE) {
    node->data.custom_structure.handler(SL_CONSOLE_VARIABLE_GET, node, (void *)arguments->arg[1]);
  }

  return status;
}

sl_status_t console_variable_set(const console_args_t *arguments)
{
  const console_variable_node_t *node;
  char *key          = (char *)arguments->arg[0];
  sl_status_t status = find_variable_node(&key, &node);
  if (status != SL_STATUS_OK)
    return status;

  if (node->type == SL_CONSOLE_VARIABLE_RAM_STRUCT_NODE) {
    if (key == NULL)
      return SL_STATUS_COMMAND_IS_INVALID;

    const structure_descriptor_entry_t *entry =
      find_structure_entry(key, node->data.ram_structure.table, node->data.ram_structure.table_size);
    if (entry == NULL)
      return SL_STATUS_COMMAND_IS_INVALID;

    uint32_t arg_result;
    status = console_parse_arg(entry->type, (char *)arguments->arg[1], &arg_result);
    if (status != SL_STATUS_OK)
      return status;

    switch (entry->type) {
      case CONSOLE_VARIABLE_UINT:
      case CONSOLE_VARIABLE_INT:
        memcpy(node->data.ram_structure.object + entry->offset, &arg_result, entry->size);
        break;

      case CONSOLE_VARIABLE_STRING:
      case CONSOLE_VARIABLE_IP_ADDRESS:
      case CONSOLE_VARIABLE_MAC_ADDRESS:
        memcpy(node->data.ram_structure.object + entry->offset, (void *)arg_result, entry->size);
        break;

      default:
        return SL_STATUS_COMMAND_IS_INVALID;
    }
  } else if (node->type == SL_CONSOLE_VARIABLE_NVM_STRUCT_NODE) {
    node->data.custom_structure.handler(SL_CONSOLE_VARIABLE_SET, node, (void *)arguments->arg[1]);
  }
  return status;
}

sl_status_t console_variable_list(const console_args_t *arguments)
{
  UNUSED_PARAMETER(arguments);
  if (console_variable_table_size == 0) {
    printf("No variables available\n");
    return SL_STATUS_OK;
  }

  static const char spaces[] = "| | | | | | | | ";
#define MAX_LEVEL_DEPTH ((sizeof(spaces) - 1) / 2)

  uint8_t level = 0;
  const console_variable_node_t *tables[MAX_LEVEL_DEPTH];
  uint32_t table_index[MAX_LEVEL_DEPTH];
  uint32_t table_end[MAX_LEVEL_DEPTH];
  memset(table_index, 0, sizeof(table_index));

  table_end[0] = console_variable_table_size;
  tables[0]    = console_variable_table;

  uint32_t iter = 0;

  printf("\nConfigurable variables\n");

  while (1) {
    const console_variable_node_t *current_table = tables[level];
    switch (current_table[iter].type) {
      case SL_CONSOLE_VARIABLE_GROUP_NODE:
        printf("%.*s+ %s:\n", level * 2, spaces, current_table[iter].key);

        ++table_index[level];
        ++level;
        tables[level]    = current_table[iter].data.group.table;
        table_end[level] = current_table[iter].data.group.table_size;
        break;

      case SL_CONSOLE_VARIABLE_RAM_STRUCT_NODE:
        printf("%.*s+ %s:\n", level * 2, spaces, current_table[iter].key);
        ++table_index[level];
        ++level;
        const structure_descriptor_entry_t *table = current_table[iter].data.ram_structure.table;
        const uint32_t table_size                 = current_table[iter].data.ram_structure.table_size;
        for (unsigned int a = 0; a < table_size; ++a) {
          printf("%.*s- %s: ", level * 2, spaces, table[a].key);
          print_variable(&table[a], current_table[iter].data.ram_structure.object);
        }
        --level;
        break;

      case SL_CONSOLE_VARIABLE_VARIABLE_NODE:
        printf("%.*s%s:\n", level * 2, spaces, tables[level][iter].key);
        ++table_index[level];
        break;

      default:
        break;
    }

    // Make sure we have a valid node, otherwise go up the tree until we do
    while (table_index[level] >= table_end[level]) {
      if (level == 0) {
        return SL_STATUS_OK;
      }
      table_index[level] = 0;
      --level;
    }
    iter = table_index[level];
  }

  return SL_STATUS_OK;
}
