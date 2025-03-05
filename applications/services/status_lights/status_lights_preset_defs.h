#pragma once

#include "status_lights_preset_base.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const StatusLightsPresetBase status_lights_preset_static_color;
extern const StatusLightsPresetBase status_lights_preset_rainbow_gradient;
extern const StatusLightsPresetBase status_lights_preset_fade;

extern const StatusLightsPresetBase* const status_lights_preset_list[StatusLightsPresetMax];

#ifdef __cplusplus
}
#endif
