#pragma once
#include <furi.h>
#include <cli/cli_command.h>

void crypto_ecdsa_command(PipeSide* pipe, FuriString* args, void* context);
