#pragma once

#include <containers/pipe.h>

/** Call sl command by name, print response and terminate sl cli session.
 *
 * @param      pipe      pointer to PipeSide instance
 * @param      sl_cmd    sl command name
 *
 * @return true on success
 */
bool cli_command_sl_cli_send_command_get_response(PipeSide* pipe, const char* sl_cmd);

void cli_command_sl_cli(PipeSide* pipe, FuriString* args, void* context);
void cli_command_sl_echo(PipeSide* pipe, FuriString* args, void* context);
