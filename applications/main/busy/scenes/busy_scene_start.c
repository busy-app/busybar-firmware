#include "../busy_i.h"
#include "../widgets/anim_menu.h"

#include <gui/modules/menu.h>
#include <gui/modules/anim_image.h>
#include <gui/modules/flex_layout.h>

#include <lvgl.h>

#define ANIM_MENU_IDLE_FRAMES       (120)
#define ANIM_MENU_TRANSITION_FRAMES (10)

typedef struct {
    FlexLayout* front_layout;
    AnimImage* front_logo;
    AnimMenu* front_menu;
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
    BusySceneStart* data =
        scene_manager_get_scene_data(instance->scene_manager, BusyAppSceneIdStart);

    with_gui(instance->gui, {
        nav_bar_reset_location(instance->nav_bar);

        widget_set_visible(timer_card_get_base(instance->timer_card), false);
        widget_set_visible(nav_bar_get_base(instance->nav_bar), true);

        data->front_layout = flex_layout_alloc(instance->front_window, FlexLayoutTypeRow);

        data->front_logo = anim_image_alloc(flex_layout_get_base(data->front_layout));

        anim_image_set_source(data->front_logo, busy_get_global_preset(instance)->start_anim_path);

        anim_image_set_loop(data->front_logo, false);

        data->front_menu = anim_menu_alloc(flex_layout_get_base(data->front_layout));
        anim_menu_set_callback(data->front_menu, busy_scene_start_menu_callback, instance);
        anim_menu_set_source(
            data->front_menu,
            BUSY_ANIM_PATH("start_menu_31x16.anim"),
            ANIM_MENU_IDLE_FRAMES,
            ANIM_MENU_TRANSITION_FRAMES);

        data->back_menu = menu_alloc(instance->back_window);
        menu_add_item(
            data->back_menu, "START", NULL, BUSY_IMG_PATH("start_12x12.bin"), 0, NULL, NULL);
        menu_add_item(
            data->back_menu, "SETUP", NULL, BUSY_IMG_PATH("setup_12x12.bin"), 0, NULL, NULL);
    });

    busy_start_transition(instance);
}

static void busy_scene_start_on_exit(void* context) {
    furi_assert(context);

    BusyApp* instance = context;
    BusySceneStart* data =
        scene_manager_get_scene_data(instance->scene_manager, BusyAppSceneIdStart);

    with_gui(instance->gui, {
        flex_layout_free(data->front_layout);
        menu_free(data->back_menu);
    });
}

static bool busy_scene_start_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(context);
    BusyApp* instance = context;

    bool consumed = false;

    if(event->type == SceneManagerEventTypeCustom) {
        if(event->event == BusySceneStartMenuIndexStart) {
            with_gui(instance->gui, {
                widget_set_visible(nav_bar_get_base(instance->nav_bar), false);
                widget_set_visible(timer_card_get_base(instance->timer_card), true);

                timer_card_show_header(instance->timer_card, false);
                timer_card_show_time(instance->timer_card, false);
            });

            busy_prepare_transition(instance, BusyTransitionTypeSelect);

            BusyTimerConfig timer_config;
            busy_timer_get_config(instance->busy_timer, &timer_config);

            if(timer_config.mode == BusyTimerModeInterval) {
                scene_manager_next_scene(instance->scene_manager, BusyAppSceneIdOverview);
            } else {
                scene_manager_next_scene(instance->scene_manager, BusyAppSceneIdTimer);
            }

        } else if(event->event == BusySceneStartMenuIndexSetup) {
            busy_push_location(instance, "SETUP");
            scene_manager_next_scene(instance->scene_manager, BusyAppSceneIdSetup);
        }

        consumed = true;
    } else if(event->type == SceneManagerEventTypeBack) {
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
