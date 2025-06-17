#pragma once

#include <cli/cli_command.h>

typedef struct CryptoTestApp CryptoTestApp;

void crypto_test_command(PipeSide* pipe, FuriString* args, void* context);
