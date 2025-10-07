#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <furi.h>
#include "../../settings.h"
#include "../settings_scenes.h"

bool matter_scene_replace_current(SettingsApp* app, SettingsCustomEvent event);

#ifdef __cplusplus
}
#endif
