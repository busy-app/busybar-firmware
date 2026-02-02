#pragma once

#include <gui/scene_manager.h>
#include <sntp/settings/settings.h>

typedef enum {
    SceneIdMenu,
    SceneIdTimezone,
    SceneIdFormat,
    SceneIdsCount,
} SceneId;

extern const Scene* const time_settings_scenes[SceneIdsCount];
extern const char* time_settings_format_names[SntpSettingTimeFormatCount];
