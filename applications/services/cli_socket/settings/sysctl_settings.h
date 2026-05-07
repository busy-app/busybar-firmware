#pragma once

#include "sysctl_settings_interface_v1.h"

typedef SysctlSettingsV1 SysctlSettings;

bool sysctl_settings_load(SysctlSettings* settings);
bool sysctl_settings_save(const SysctlSettings* settings);
