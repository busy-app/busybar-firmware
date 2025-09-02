#pragma once
#include <stdint.h>
#include "busy_timer.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    const char* name;
    BusyTimerConfig default_timer_config;
} BusyConfig;

extern const BusyConfig busy_config_default;

#ifdef __cplusplus
}
#endif
