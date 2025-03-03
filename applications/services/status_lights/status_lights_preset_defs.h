#pragma once

#include "status_lights_preset_base.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const StatusLightsPresetBase status_ligth_preset_rainbow_gradient;
extern const StatusLightsPresetBase status_ligth_preset_white_fade;

extern const StatusLightsPresetBase* status_lights_preset_list[StatusLightsPresetNum];

#ifdef __cplusplus
}
#endif
