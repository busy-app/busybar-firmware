#include "status_lights_preset_defs.h"

#include <stddef.h>

const StatusLightsPresetBase* const status_lights_preset_list[StatusLightsPresetMax] = {
    [StatusLightsPresetStaticColor] = &status_lights_preset_static_color,
    [StatusLightsPresetRainbowGradient] = &status_lights_preset_rainbow_gradient,
    [StatusLightsPresetFade] = &status_lights_preset_fade,
};
