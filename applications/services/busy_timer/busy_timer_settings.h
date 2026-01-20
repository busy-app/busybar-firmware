#pragma once

#include "busy_timer.h"
#include "time_macros.h"

typedef struct {
    BusyTimerConfig timer_config;
} BusyTimerSettings;

bool busy_timer_settings_load(BusyTimerSettings* settings, BusyTimerProfileId profile_id);

bool busy_timer_settings_save(const BusyTimerSettings* settings, BusyTimerProfileId profile_id);

void busy_timer_settings_set_default(BusyTimerSettings* settings, BusyTimerProfileId profile_id);
