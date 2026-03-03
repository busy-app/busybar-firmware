#include "interface_v1.h"

#define BRIGHTNESS_DEFAULT 50
#define MODE_DEFAULT       BrightnessControlBrightnessModeAuto

static const char* const mode_names[BrightnessControlBrightnessModeMax] = {
    [BrightnessControlBrightnessModeAuto] = "auto",
    [BrightnessControlBrightnessModeManual] = "manual",
};

static bool brightness_is_valid(const SettingProviderSetting* setting, int value) {
    UNUSED(setting);

    return (value >= BRIGHTNESS_MIN && value <= BRIGHTNESS_MAX);
}

const SettingProviderSetting brightness_v1_settings[] = {
    [BrightnessSettingIdxMode] =
        {
            .name = "mode",
            .interface =
                &(const SettingProviderEnumInterface){
                    .default_value = MODE_DEFAULT,
                    .count = BrightnessControlBrightnessModeMax,
                    .names = mode_names,
                },
            .field_offset = offsetof(BrightnessSettingsV1, mode),
            .type = SettingProviderSettingTypeEnum,
        },
    [BrightnessSettingIdxBrightness] =
        {
            .name = "brightness",
            .interface =
                &(const SettingProviderIntInterface){
                    .default_value = BRIGHTNESS_DEFAULT,
                    .is_valid_callback = brightness_is_valid,
                },
            .field_offset = offsetof(BrightnessSettingsV1, brightness),
            .type = SettingProviderSettingTypeInt,
        },
};

static_assert(COUNT_OF(brightness_v1_settings) == BrightnessSettingIdxCount);

const SettingProviderSetting brightness_v1_settings_root = {
    .name = NULL,
    .interface =
        &(const SettingProviderStructureInterface){
            .is_valid_callback = NULL,
            .inner_settings = brightness_v1_settings,
            .inner_settings_count = COUNT_OF(brightness_v1_settings),
        },
    .field_offset = 0,
    .type = SettingProviderSettingTypeStructure,
};
