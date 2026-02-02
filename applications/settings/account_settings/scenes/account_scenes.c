#include "account_scenes.h"

extern const Scene account_scene_no_wifi;
extern const Scene account_scene_connecting;
extern const Scene account_scene_not_linked_menu;
extern const Scene account_scene_link_pin;
extern const Scene account_scene_linked_info;
extern const Scene account_scene_linked_menu;
extern const Scene account_scene_unlink;
extern const Scene account_scene_error;

const Scene* const account_settings_scenes[SceneIdsCount] = {
    [SceneIdNoWifi] = &account_scene_no_wifi,
    [SceneIdConnecting] = &account_scene_connecting,
    [SceneIdNotLinkedMenu] = &account_scene_not_linked_menu,
    [SceneIdLinkPin] = &account_scene_link_pin,
    [SceneIdLinkedInfo] = &account_scene_linked_info,
    [SceneIdLinkedMenu] = &account_scene_linked_menu,
    [SceneIdUnlink] = &account_scene_unlink,
    [SceneIdError] = &account_scene_error,
};
