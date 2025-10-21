#pragma once

#include <toolbox/setting_provider.h>

#define SNTP_V0_TIMEZONE_OFFSET_MIN     (-12 * 60)
#define SNTP_V0_TIMEZONE_OFFSET_MAX     (+14 * 60)
#define SNTP_V0_TIMEZONE_OFFSET_DEFAULT (+4 * 60)

typedef enum {
    SntpSettingV0IdxTimezone,

    SntpSettingV0IdxsCount,
} SntpSettingV0Idx;

extern const SettingProviderSetting sntp_v0_settings[];
