#pragma once

#include "cli_i.h"

void cli_command_sl_cli_send_command_get_response(Cli* cli, const char* command);
void cli_command_sl_cli(Cli* cli, FuriString* args, void* context);