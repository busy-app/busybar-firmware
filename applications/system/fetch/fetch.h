#pragma once

#include <cli/cli_command.h>

void fetch_command(PipeSide* pipe, FuriString* args, void* context);
bool fetch_download_file(FuriString* url, FuriString* path);
