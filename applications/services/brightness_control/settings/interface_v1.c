#include "interface_v1.h"

#define BRIGHTNESS_DEFAULT 50
#define MODE_DEFAULT       BrightnessControlBrightnessModeAuto

static const char* const mode_string_map[] = {
    [BrightnessControlBrightnessModeAuto] = "auto",
    [BrightnessControlBrightnessModeManual] = "manual",
};

static bool brightness_is_valid(const SettingProviderSetting* setting, int value) {
    UNUSED(setting);

    return (value >= BRIGHTNESS_MIN && value <= BRIGHTNESS_MAX);
}

const SettingProviderSetting brightness_v1_settings[] = {
    [BrightnessSettingV1IdxMode] =
        {
            .name = "mode",
            .interface =
                &(const SettingProviderEnumInterface){
                    .string_map = mode_string_map,
                    .string_map_length = COUNT_OF(mode_string_map),
                    .type_size = SIZEOF_MEMBER(BrightnessSettingsV1, mode),
                    .default_value = &(const BrightnessControlBrightnessMode){MODE_DEFAULT},
                },
            .field_offset = offsetof(BrightnessSettingsV1, mode),
            .type = SettingProviderSettingTypeEnum,
        },
    [BrightnessSettingV1IdxBrightness] =
        {
            .name = "brightness",
            .interface =
                &(const SettingProviderIntInterface){
                    .is_valid_callback = brightness_is_valid,
                    .default_value = BRIGHTNESS_DEFAULT,
                },
            .field_offset = offsetof(BrightnessSettingsV1, brightness),
            .type = SettingProviderSettingTypeInt,
        },
};

static_assert(COUNT_OF(brightness_v1_settings) == BrightnessSettingV1IdxCount);

const SettingProviderSetting brightness_v1_settings_root = {
    .name = NULL,
    .interface =
        &(const SettingProviderStructInterface){
            .inner_settings = brightness_v1_settings,
            .inner_settings_count = COUNT_OF(brightness_v1_settings),
        },
    .field_offset = 0,
    .type = SettingProviderSettingTypeStruct,
};
