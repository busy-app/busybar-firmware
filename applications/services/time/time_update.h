#pragma once

#include "time.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*TimeTimeUpdateCallback)(Time* instance, bool is_success);

void time_update_run(Time* instance, TimeTimeUpdateCallback callback);

#ifdef __cplusplus
}
#endif
