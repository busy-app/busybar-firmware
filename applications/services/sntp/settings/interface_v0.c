#include "interface_v0.h"

static bool is_timezone_offset_valid(int timezone_offset) {
    return timezone_offset >= SNTP_V0_TIMEZONE_OFFSET_MIN &&
           timezone_offset <= SNTP_V0_TIMEZONE_OFFSET_MAX;
}

const SettingProviderSetting sntp_v0_settings[] = {
    [SntpSettingV0IdxTimezone] =
        {
            .name = "timezone",
            .interface =
                &(const SettingProviderIntInterface){
                    .default_value = SNTP_V0_TIMEZONE_OFFSET_DEFAULT,
                    .is_valid_callback = is_timezone_offset_valid,
                },
            .type = SettingProviderSettingTypeInt,
        },
};

static_assert(COUNT_OF(sntp_v0_settings) == SntpSettingV0IdxsCount);
