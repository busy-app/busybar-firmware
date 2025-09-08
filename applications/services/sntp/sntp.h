#pragma once

#include <furi/furi.h>

typedef struct Sntp Sntp;

#ifdef __cplusplus
extern "C" {
#endif

void sntp_status_update(Sntp* instance, bool success);

#ifdef __cplusplus
}
#endif
