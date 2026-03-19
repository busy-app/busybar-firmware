#pragma once

#include <furi.h>

#define DEVICE_NAME_MAX_LENGTH (20U)

bool device_name_validate_cstr(const char* name, FuriString* error);

bool device_name_validate(FuriString* name, FuriString* error);
