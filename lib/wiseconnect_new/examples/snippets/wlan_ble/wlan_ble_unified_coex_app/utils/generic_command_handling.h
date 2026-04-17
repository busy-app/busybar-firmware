#ifndef __GENERIC_COMMAND_HANDLING_H__
#define __GENERIC_COMMAND_HANDLING_H__
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
//#include "gap_cmd_hdlr_auto_gen.h"

#define MAX_NO_OF_LENGTH 25
#define MAX_BUFF_LENGTH  1024 //250
typedef struct command_lut_entry_s command_lut_entry_t;
typedef struct module_lut_entry_s module_lut_entry_t;
command_lut_entry_t *get_command_entry(uint8_t **argv, command_lut_entry_t *current_command);
module_lut_entry_t *get_module_entry(uint8_t **argv, module_lut_entry_t *current_module);
void display_submodule(uint8_t **argv, module_lut_entry_t *current_module);
void display_command(uint8_t **argv, command_lut_entry_t *current_command);

typedef struct command_lut_entry_s {
  char *command_name;
  void (*handler)(uint8_t argc, uint8_t *argv[]);
} command_lut_entry_t;
//void (*custom_handler)(uint8_t argc, uint8_t *argv[]);
typedef struct module_lut_entry_s {
  char *module_name;
  module_lut_entry_t *sub_modules;
  command_lut_entry_t *commands;
  void (*custom_handler)(uint8_t argc, uint8_t *argv[]);
} module_lut_entry_t;

#endif