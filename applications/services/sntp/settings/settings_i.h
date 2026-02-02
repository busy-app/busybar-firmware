#pragma once

#include "settings.h"

void sntp_settings_reset(SntpSettings* settings);
void sntp_settings_load(SntpSettings* settings);
bool sntp_settings_save(const SntpSettings* settings);
