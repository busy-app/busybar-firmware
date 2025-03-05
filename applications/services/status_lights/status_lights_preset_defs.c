#include "status_lights_preset_defs.h"

#include <stddef.h>

const StatusLightsPresetBase* const status_lights_preset_list[StatusLightsPresetMax] = {
    [StatusLightsPresetStatic] = NULL, // Special case for static color
    [StatusLightsPresetRainbowGradient] = &status_ligth_preset_rainbow_gradient,
    [StatusLightsPresetFade] = &status_ligth_preset_white_fade,
};
