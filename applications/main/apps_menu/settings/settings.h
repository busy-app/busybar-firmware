#pragma once

#include "interface_v1.h"

typedef AppsMenuSettingsV1 AppsMenuSettings;

bool apps_menu_settings_reset(AppsMenuSettings* settings);
bool apps_menu_settings_load(AppsMenuSettings* settings);
bool apps_menu_settings_save(const AppsMenuSettings* settings);
