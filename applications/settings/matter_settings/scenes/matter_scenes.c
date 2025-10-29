#include "matter_scenes.h"

extern const Scene matter_scene_main;
extern const Scene matter_scene_pairing;

extern const Scene matter_scene_commission_start;
extern const Scene matter_scene_commission_done;
extern const Scene matter_scene_commission_fail;

extern const Scene matter_scene_connect_wifi;
extern const Scene matter_scene_reboot;

const Scene* const matter_scenes[SceneIdsCount] = {
    [SceneIdMain] = &matter_scene_main,
    [SceneIdPairing] = &matter_scene_pairing,

    [SceneIdCommissionStart] = &matter_scene_commission_start,
    [SceneIdCommissionDone] = &matter_scene_commission_done,
    [SceneIdCommissionFail] = &matter_scene_commission_fail,

    [SceneIdConnectWifi] & matter_scene_connect_wifi,
    [SceneIdReboot] & matter_scene_reboot,
};
