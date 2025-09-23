#pragma once

#include <furi/furi.h>

typedef struct CheckUpdateFw CheckUpdateFw;

#ifdef __cplusplus
extern "C" {
#endif

void check_update_fw_status_update(CheckUpdateFw* instance, bool success);

#ifdef __cplusplus
}
#endif
