#include "generic_command_handling.h"
#include <stdint.h>

extern void ui_task_req_adv_cmd_handler(uint8_t argc, uint8_t *argv[]);
extern void ui_task_req_scan_cmd_handler(uint8_t argc, uint8_t *argv[]);
extern void ui_task_req_adv_stop_cmd_handler(uint8_t argc, uint8_t *argv[]);
extern void ui_task_req_scan_stop_cmd_handler(uint8_t argc, uint8_t *argv[]);
extern void ui_task_req_conn_cmd_handler(uint8_t argc, uint8_t *argv[]);
extern void ui_task_req_disconnect_cmd_handler(uint8_t argc, uint8_t *argv[]);
extern void ui_task_cmd_conn_params_update_cmd_handler(uint8_t argc, uint8_t *argv[]);

command_lut_entry_t ble_commands_lut[] = {
  { .command_name = "req_adv", .handler = ui_task_req_adv_cmd_handler },
  { .command_name = "req_scan", .handler = ui_task_req_scan_cmd_handler },
  { .command_name = "req_adv_stop", .handler = ui_task_req_adv_stop_cmd_handler },
  { .command_name = "req_scan_stop", .handler = ui_task_req_scan_stop_cmd_handler },
  { .command_name = "req_conn", .handler = ui_task_req_conn_cmd_handler },
  { .command_name = "req_disconnect", .handler = ui_task_req_disconnect_cmd_handler },
  { .command_name = "req_conn_params_update", .handler = ui_task_cmd_conn_params_update_cmd_handler },
  { .command_name = NULL, .handler = NULL }
};

module_lut_entry_t ble_module_lut[] = {
  { .module_name = NULL, .sub_modules = NULL, .commands = NULL, .custom_handler = NULL }
};
