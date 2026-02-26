#include "../brightness_control.h"
#include <toolbox/setting_provider.h>

typedef enum {
    BrightnessSettingIdxMode,
    BrightnessSettingIdxBrightness,

    BrightnessSettingIdxCount
} SntpSettingV1Idx;

typedef struct {
    BrightnessControlBrightnessMode mode;
    int brightness;
} BrightnessSettingsV1;
