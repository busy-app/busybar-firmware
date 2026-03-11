#pragma once

#include "settings.h"

bool updater_settings_reset(UpdaterSettings* settings);
bool updater_settings_load(UpdaterSettings* settings);
bool updater_settings_save(const UpdaterSettings* settings);
