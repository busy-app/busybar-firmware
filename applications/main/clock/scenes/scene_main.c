#include "../clock_i.h"
#include "../widgets/clock_view.h"

#include <gui/modules/menu.h>
#include <gui/modules/anim_menu.h>
#include <gui/modules/overlap_fader.h>

typedef enum {
    ThisSceneEventStart = ThisEventSceneEventsStart,
    ThisSceneEventSetup,
} ThisSceneEventIdx;

typedef struct {
    Widget* front_container;
    ClockView* front_clock;

    Menu* back_menu;
} ThisScene;

static inline ThisScene* this_get_scene(ThisInstance* instance) {
    return scene_manager_get_scene_data(instance->scene_manager, ThisSceneIdxMain);
}

static void clock_scene_start_menu_callback(uint32_t index, void* context) {
    furi_assert(context);

    ThisInstance* instance = context;

    clock_app_fire_event(instance, index);
}

static void this_scene_on_enter(void* context) {
    furi_assert(context);

    ThisInstance* instance = context;
    ThisScene* data = this_get_scene(instance);

    LocalTime local_time = sntp_get_local_time(instance->sntp);

    with_gui(instance->gui, {
        /* front layout setup */
        data->front_container = widget_alloc(instance->front_scene_window);

        data->front_clock = clock_view_alloc(data->front_container);
        widget_set_align(clock_view_get_base(data->front_clock), AlignLeftMid);
        widget_set_padding(clock_view_get_base(data->front_clock), 1, 0, 0, 0);

        clock_view_set_date_time(data->front_clock, &local_time.dt);

        AnimMenu* front_menu = anim_menu_alloc(data->front_container);
        anim_menu_set_source(front_menu, SHARED_ANIM_PATH("start_menu_31x16.anim"), 2);
        widget_set_align(anim_menu_get_base(front_menu), AlignRightMid);

        OverlapFader* front_fader = overlap_fader_alloc(data->front_container);
        widget_set_width(overlap_fader_get_base(front_fader), 10);
        overlap_fader_align_to(front_fader, anim_menu_get_base(front_menu), OverlapFaderSideLeft);

        /* back layout setup */
        data->back_menu = menu_alloc(instance->back_scene_window);
        menu_add_item(
            data->back_menu,
            "Start",
            NULL,
            SHARED_IMG_PATH("start_11x11.bin"),
            ThisSceneEventStart,
            clock_scene_start_menu_callback,
            instance);
        menu_add_item(
            data->back_menu,
            "Setup",
            NULL,
            SHARED_IMG_PATH("setup_11x11.bin"),
            ThisSceneEventSetup,
            clock_scene_start_menu_callback,
            instance);
    });
}

static void this_scene_on_exit(void* context) {
    furi_assert(context);

    ThisInstance* instance = context;
    ThisScene* data = this_get_scene(instance);

    with_gui(instance->gui, {
        widget_free(data->front_container);

        menu_free(data->back_menu);
    });
}

static bool this_scene_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(context);

    ThisInstance* instance = context;
    ThisScene* scene = this_get_scene(instance);

    if(event->type == SceneManagerEventTypeCustom) {
        switch(event->event) {
        case ThisEventTimerUpdate:
            LocalTime local_time = sntp_get_local_time(instance->sntp);
            with_gui(instance->gui, {
                clock_view_set_date_time(scene->front_clock, &local_time.dt);
            });
            return true;

        case ThisSceneEventStart:
            with_gui(instance->gui, {
                widget_set_visible(nav_bar_get_base(instance->back_nav_bar), false);
            });
            scene_manager_next_scene(instance->scene_manager, ThisSceneIdxClock);
            return true;

        case ThisSceneEventSetup:
            with_gui(instance->gui, { nav_bar_push_location(instance->back_nav_bar, "SETUP"); });
            scene_manager_next_scene(instance->scene_manager, ThisSceneIdxSetup);
            return true;

        default:
            break;
        }
    }

    return false;
}

const Scene clock_app_scene_main = {
    .enter_callback = this_scene_on_enter,
    .exit_callback = this_scene_on_exit,
    .event_callback = this_scene_on_event,
    .data_size = sizeof(ThisScene),
};
