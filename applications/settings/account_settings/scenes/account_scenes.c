#include "account_scenes.h"

extern const Scene account_scene_connecting;
extern const Scene account_scene_not_linked;
extern const Scene account_scene_link_pin;
extern const Scene account_scene_linked;
extern const Scene account_scene_error;
extern const Scene account_scene_no_wifi;

const Scene* const account_settings_scenes[SceneIdsCount] = {
    [SceneIdConnecting] = &account_scene_connecting,
    [SceneIdNotLinked] = &account_scene_not_linked,
    [SceneIdLinkPin] = &account_scene_link_pin,
    [SceneIdLinked] = &account_scene_linked,
    [SceneIdError] = &account_scene_error,
    [SceneIdNoWifi] = &account_scene_no_wifi,
};
