#include "wifi_scenes.h"

extern const Scene wifi_scene_not_connected;
extern const Scene wifi_scene_state;
extern const Scene wifi_scene_menu;
extern const Scene wifi_scene_info;
extern const Scene wifi_scene_forget;

const Scene* const wifi_settings_scenes[SceneIdsCount] = {
    [SceneIdNotConnected] = &wifi_scene_not_connected,
    [SceneIdState] = &wifi_scene_state,
    [SceneIdMenu] = &wifi_scene_menu,
    [SceneIdInfo] = &wifi_scene_info,
    [SceneIdForget] = &wifi_scene_forget,
};
