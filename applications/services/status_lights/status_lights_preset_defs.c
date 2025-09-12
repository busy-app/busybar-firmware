#include "status_lights_preset_defs.h"

#include <stddef.h>

extern const StatusLightsPresetBase status_lights_preset_static_color;
extern const StatusLightsPresetBase status_lights_preset_rainbow_gradient;
extern const StatusLightsPresetBase status_lights_preset_fade;

const StatusLightsPresetBase* const status_lights_preset_list[StatusLightsPresetsCount] = {
    [StatusLightsPresetOff] = NULL,
    [StatusLightsPresetStaticColor] = &status_lights_preset_static_color,
    [StatusLightsPresetRainbowGradient] = &status_lights_preset_rainbow_gradient,
    [StatusLightsPresetFade] = &status_lights_preset_fade,
};
