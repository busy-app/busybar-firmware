#include "../busy.h"

typedef struct {
} BusySceneNext;

static void busy_scene_next_on_enter(void* context) {
    BusyApp* instance = context;
    UNUSED(instance);
}

static void busy_scene_next_on_exit(void* context) {
    BusyApp* instance = context;
    UNUSED(instance);
}

static bool busy_scene_next_on_event(const SceneManagerEvent* event, void* context) {
    BusyApp* instance = context;
    UNUSED(instance);
    UNUSED(event);

    return true;
}

const Scene busy_scene_next = {
    .enter_callback = busy_scene_next_on_enter,
    .exit_callback = busy_scene_next_on_exit,
    .event_callback = busy_scene_next_on_event,
    .data_size = sizeof(BusySceneNext),
};
