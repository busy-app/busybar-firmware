#pragma once

#include <cli/cli_command.h>

void tar_command(PipeSide* pipe, FuriString* args, void* context);
bool tar_compress_directory_cli(FuriString* path, FuriString* args);
bool tar_extract_files_cli(FuriString* path, FuriString* args);
