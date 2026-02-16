#pragma once

#include "settings.h"

void updater_settings_reset(UpdaterSettings* settings);
void updater_settings_load(UpdaterSettings* settings);
bool updater_settings_save(const UpdaterSettings* settings);
void updater_settings_copy(UpdaterSettings* target, const UpdaterSettings* source);
