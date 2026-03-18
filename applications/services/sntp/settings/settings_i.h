#pragma once

#include "settings.h"

bool sntp_settings_reset(SntpSettings* settings);
bool sntp_settings_load(SntpSettings* settings);
bool sntp_settings_save(const SntpSettings* settings);
