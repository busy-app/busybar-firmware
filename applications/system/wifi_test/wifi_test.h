#pragma once

#include <cli/cli_command.h>

typedef struct WifiTestApp WifiTestApp;

void wifi_test_command(PipeSide* pipe, FuriString* args, void* context);
