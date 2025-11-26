#pragma once

#include "busy_timer.h"
#include "time_macros.h"

typedef struct {
    BusyTimerConfig timer_config;
} BusySettings;

bool busy_settings_load(BusySettings* settings);

bool busy_settings_save(const BusySettings* settings);
