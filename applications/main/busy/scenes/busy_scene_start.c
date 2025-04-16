#include "../busy.h"
#include "../widgets/anim_menu.h"

#include <gui/modules/anim_image.h>

#define ANIM_MENU_IDLE_FRAMES       (120)
#define ANIM_MENU_TRANSITION_FRAMES (10)

typedef struct {
    AnimImage* logo;
    AnimMenu* menu;
} BusySceneStart;

typedef enum {
    BusySceneStartMenuIndexStart,
    BusySceneStartMenuIndexSetup,
    BusySceneStartMenuIndexMax,
} BusySceneStartMenuIndex;

static void busy_scene_start_input_callback(uint32_t index, void* context) {
    furi_assert(index < BusySceneStartMenuIndexMax);
    furi_assert(context);

    BusyApp* instance = context;
    busy_send_custom_event(instance, index);
}

static void busy_scene_start_on_enter(void* context) {
    furi_assert(context);

    BusyApp* instance = context;
    BusySceneStart* data = scene_manager_get_current_scene_data(instance->scene_manager);

    with_gui(instance->gui, {
        data->logo = anim_image_alloc(instance->front_window);
        anim_image_set_source(data->logo, ANIM_PATH("start.anim"));
        anim_image_start(data->logo);

        data->menu = anim_menu_alloc(instance->front_window);
        anim_menu_set_callback(data->menu, busy_scene_start_input_callback, instance);
        anim_menu_set_source(data->menu, ANIM_PATH("menu.anim"));
        anim_menu_set_intervals(data->menu, ANIM_MENU_IDLE_FRAMES, ANIM_MENU_TRANSITION_FRAMES);
        widget_set_pos_x(anim_menu_get_base(data->menu), 41);
    });
}

static void busy_scene_start_on_exit(void* context) {
    furi_assert(context);

    BusyApp* instance = context;
    BusySceneStart* data = scene_manager_get_current_scene_data(instance->scene_manager);

    with_gui(instance->gui, {
        anim_image_free(data->logo);
        anim_menu_free(data->menu);
    });
}

static bool busy_scene_start_on_event(const SceneManagerEvent* event, void* context) {
    BusyApp* instance = context;
    UNUSED(instance);

    bool consumed = false;

    if(event->type == SceneManagerEventTypeCustom) {
        if(event->event == BusySceneStartMenuIndexStart) {
            FURI_LOG_D("Start", "Start selected!");
            consumed = true;
        } else if(event->event == BusySceneStartMenuIndexSetup) {
            FURI_LOG_D("Start", "Setup selected!");
            consumed = true;
        }
    }
    return consumed;
}

const Scene busy_scene_start = {
    .enter_callback = busy_scene_start_on_enter,
    .exit_callback = busy_scene_start_on_exit,
    .event_callback = busy_scene_start_on_event,
    .data_size = sizeof(BusySceneStart),
};
