#pragma once

#include "settings.h"

#define UPDATER_SETTINGS_CHECK_URL_DEFAULT_ALIAS UPDATER_SETTINGS_V2_CHECK_URL_DEFAULT_ALIAS
#define UPDATER_SETTINGS_CHECK_URL_DEFAULT_VALUE UPDATER_SETTINGS_V2_CHECK_URL_DEFAULT_VALUE

bool updater_settings_reset(UpdaterSettings* settings);
bool updater_settings_load(UpdaterSettings* settings);
bool updater_settings_save(const UpdaterSettings* settings);

const char* updater_settings_get_check_url_value(UpdaterSettings* settings);
