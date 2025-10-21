#pragma once

#include "interface_v1.h"

#ifdef __cplusplus
extern "C" {
#endif

bool sntp_settings_load_draft(SntpSettingsV1* settings);

bool sntp_settings_save_draft(const SntpSettingsV1* settings);

#ifdef __cplusplus
}
#endif
