#include "generic_command_handling.h"

command_lut_entry_t root_commands_lut[] = { { .command_name = NULL, .handler = NULL } };

extern module_lut_entry_t btc_module_lut[];
extern command_lut_entry_t btc_commands_lut[];

module_lut_entry_t root_module_lut[] = {
  { .module_name = "btc", .sub_modules = btc_module_lut, .commands = btc_commands_lut, .custom_handler = NULL },
  { .module_name = NULL, .sub_modules = NULL, .commands = NULL, .custom_handler = NULL }
};
