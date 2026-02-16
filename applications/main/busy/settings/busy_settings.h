#pragma once

#include "busy_settings_common.h"

#include "busy_settings_interface_v1.h"

typedef BusySettingsV1 BusySettings;

void busy_settings_load(BusySettings* settings, BusySettingsProfileId profile_id);

bool busy_settings_save(const BusySettings* settings, BusySettingsProfileId profile_id);
