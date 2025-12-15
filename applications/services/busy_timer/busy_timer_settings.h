#pragma once

#include "busy_timer.h"
#include "time_macros.h"

typedef struct {
    BusyTimerConfig timer_config;
} BusyTimerSettings;

bool busy_timer_settings_load(BusyTimerSettings* settings);

bool busy_timer_settings_save(const BusyTimerSettings* settings);
