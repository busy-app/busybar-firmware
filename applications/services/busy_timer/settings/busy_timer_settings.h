#pragma once

#include "busy_timer_settings_interface_v1.h"

typedef BusyTimerSettingsV1 BusyTimerSettings;

void busy_timer_settings_load(BusyTimerSettings* settings, BusyTimerProfileId profile_id);

void busy_timer_settings_save(const BusyTimerSettings* settings, BusyTimerProfileId profile_id);
