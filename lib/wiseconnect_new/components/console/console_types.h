/*******************************************************************************
* @file  console_types.h
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

#include "console_constants.h"
#include "sl_status.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************
 *                 Type Definitions
 ******************************************************/

typedef const char *const_string_t;
typedef const const_string_t *arg_list_t;
typedef const arg_list_t *all_arg_list_t;
typedef const uint32_t *value_list_t;

/******************************************************
 *                    Structures
 ******************************************************/

typedef struct {
  uint32_t bitmap;
  uint32_t arg[SL_SI91X_CLI_CONSOLE_MAX_ARG_COUNT];
} console_args_t;

typedef sl_status_t (*console_handler_t)(console_args_t *arguments);

typedef struct {
  const char *description;
  const char *const *argument_help;
  union {
    console_handler_t handler;
    const void *sub_command_database;
  };
  console_argument_type_t argument_list[];
} console_descriptive_command_t;

typedef struct {
  const char *key;
  const void *value;
} console_database_entry_t;

typedef struct {
  uint32_t length;
  console_database_entry_t entries[];
} console_database_t;

/*
 * Console variable structures
 */
typedef struct {
  const char *key;
  uint16_t offset;
  uint8_t size;
  uint8_t type;
} structure_descriptor_entry_t;

struct console_variable_node_s;
typedef sl_status_t (*console_variable_handler_t)(console_variable_action_t action,
                                                  const struct console_variable_node_s *node,
                                                  const char *value);

typedef struct console_variable_node_s {
  uint32_t type;
  const char *key;
  union {
    // A node that points to another group of nodes
    struct {
      void *table;
      uint32_t table_size;
    } group;

    // A node that points to an object in memory
    struct {
      void *data;
      uint32_t data_size;
    } variable;

    // A node that represents a basic structure in RAM
    struct {
      const void *table;
      uint32_t table_size;
      void *object;
    } ram_structure;

    // A node that represents a structure with a custom handler
    struct {
      union {
        void *address;
        uint32_t nvm_id;
      };
      console_variable_handler_t handler;
    } custom_structure;
  } data;
} console_variable_node_t;

#ifdef __cplusplus
} /*extern "C" */
#endif
