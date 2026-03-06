#include "../brightness_control.h"

#include <setting_provider.h>

typedef enum {
    BrightnessSettingV1IdxMode,
    BrightnessSettingV1IdxBrightness,

    BrightnessSettingV1IdxCount
} BrightnessSettingV1Idx;

typedef struct {
    BrightnessControlBrightnessMode mode;
    int brightness;
} BrightnessSettingsV1;
