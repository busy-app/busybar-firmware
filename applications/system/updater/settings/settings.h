#pragma once

#include "interface_v1.h"

typedef UpdaterSettingsV1 UpdaterSettings;

void updater_settings_reset(UpdaterSettings* settings);
void updater_settings_load(UpdaterSettings* settings);
bool updater_settings_save(const UpdaterSettings* settings);
