/**
 * @brief Device Name service functions for internal use
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <furi.h>

#define DEVICE_NAME_DEFAULT "BUSY Bar"

bool device_name_validate(FuriString* name, FuriString* error);

#ifdef __cplusplus
}
#endif
