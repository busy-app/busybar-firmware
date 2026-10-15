#pragma once

#include "interface_v2.h"

typedef AppsMenuSettingsV2 AppsMenuSettings;

bool apps_menu_settings_reset(AppsMenuSettings* settings);
bool apps_menu_settings_load(AppsMenuSettings* settings);
bool apps_menu_settings_save(const AppsMenuSettings* settings);
