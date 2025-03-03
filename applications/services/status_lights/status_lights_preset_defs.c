#include "status_lights_preset_defs.h"

const StatusLightsPresetBase* status_lights_preset_list[StatusLightsPresetNum] = {
    [StatusLightsPresetRainbowGradient] = &status_ligth_preset_rainbow_gradient,
};
