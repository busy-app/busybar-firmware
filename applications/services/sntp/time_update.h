#pragma once

#include "sntp.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*SntpTimeUpdateCallback)(Sntp* instance, bool is_success);

void sntp_time_update_run(Sntp* instance, SntpTimeUpdateCallback callback);

#ifdef __cplusplus
}
#endif
