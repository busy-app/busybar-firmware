#pragma once

#include <cli/cli_command.h>

void fetch_command(PipeSide* pipe, FuriString* args, void* context);
void fetch_url(PipeSide* pipe, FuriString* url, FuriString* args, void* context);
