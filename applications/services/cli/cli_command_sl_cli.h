#pragma once

#include "cli_i.h"

/** Call sl command by name, print response and terminate sl cli session.
 *
 * @param      cli       pointer to cli instance
 * @param      sl_cmd    sl command name
 */
void cli_command_sl_cli_send_command_get_response(Cli* cli, const char* sl_cmd);

void cli_command_sl_cli(Cli* cli, FuriString* args, void* context);
void cli_command_sl_echo(Cli* cli, FuriString* args, void* context);
