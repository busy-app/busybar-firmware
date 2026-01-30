#pragma once

#include <containers/pipe.h>

void cli_command_rtc_date(PipeSide* pipe, FuriString* args, void* context);
void cli_command_rtc_timezone(PipeSide* pipe, FuriString* args, void* context);
