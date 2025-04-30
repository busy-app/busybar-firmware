#include "../busy.h"
#include "../widgets/anim_menu.h"
#include "../widgets/nav_header.h"

#include <gui/modules/menu.h>
#include <gui/modules/anim_image.h>
#include <gui/modules/flex_layout.h>

#define ANIM_MENU_IDLE_FRAMES       (120)
#define ANIM_MENU_TRANSITION_FRAMES (10)

typedef struct {
    FlexLayout* front_layout;
    AnimImage* front_logo;
    AnimMenu* front_menu;
    FlexLayout* back_layout;
    NavHeader* back_header;
    Menu* back_menu;
} BusySceneStart;

typedef enum {
    BusySceneStartMenuIndexStart,
    BusySceneStartMenuIndexSetup,
    BusySceneStartMenuIndexMax,
} BusySceneStartMenuIndex;

static void busy_scene_start_menu_callback(uint32_t index, void* context) {
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
        data->front_layout = flex_layout_alloc(instance->front_window, FlexLayoutTypeRow);

        data->front_logo = anim_image_alloc(flex_layout_get_base(data->front_layout));
        anim_image_set_source(data->front_logo, BUSY_ANIM_PATH("A_start_logo_41x16.anim"));
        anim_image_start(data->front_logo);

        data->front_menu = anim_menu_alloc(flex_layout_get_base(data->front_layout));
        anim_menu_set_callback(data->front_menu, busy_scene_start_menu_callback, instance);
        anim_menu_set_source(data->front_menu, BUSY_ANIM_PATH("A_start_menu_31x16.anim"));
        anim_menu_set_intervals(
            data->front_menu, ANIM_MENU_IDLE_FRAMES, ANIM_MENU_TRANSITION_FRAMES);

        data->back_layout = flex_layout_alloc(instance->back_window, FlexLayoutTypeColumn);

        data->back_header = nav_header_alloc(flex_layout_get_base(data->back_layout));
        nav_header_set_image(data->back_header, (const void*)&I_header_40x16);

        data->back_menu = menu_alloc(flex_layout_get_base(data->back_layout));
        menu_add_item(data->back_menu, "START", NULL, (const void*)&I_start_12x12, 0, NULL, NULL);
        menu_add_item(data->back_menu, "SETUP", NULL, (const void*)&I_setup_12x12, 0, NULL, NULL);
    });
}

static void busy_scene_start_on_exit(void* context) {
    furi_assert(context);

    BusyApp* instance = context;
    BusySceneStart* data = scene_manager_get_current_scene_data(instance->scene_manager);

    with_gui(instance->gui, {
        flex_layout_free(data->front_layout);
        flex_layout_free(data->back_layout);
    });
}

static bool busy_scene_start_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(context);
    BusyApp* instance = context;

    bool consumed = false;

    if(event->type == SceneManagerEventTypeCustom) {
        if(event->event == BusySceneStartMenuIndexStart) {
            scene_manager_next_scene(instance->scene_manager, BusyAppSceneIdTimer);
        } else if(event->event == BusySceneStartMenuIndexSetup) {
            scene_manager_next_scene(instance->scene_manager, BusyAppSceneIdSetup);
        }

        consumed = true;
    }

    return consumed;
}

const Scene busy_scene_start = {
    .enter_callback = busy_scene_start_on_enter,
    .exit_callback = busy_scene_start_on_exit,
    .event_callback = busy_scene_start_on_event,
    .data_size = sizeof(BusySceneStart),
};
