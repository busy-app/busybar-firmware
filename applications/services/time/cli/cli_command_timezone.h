#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <containers/pipe.h>

void cli_command_timezone(PipeSide* pipe, FuriString* args, void* context);

#ifdef __cplusplus
}
#endif
