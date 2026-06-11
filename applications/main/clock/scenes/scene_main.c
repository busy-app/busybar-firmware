#include "../clock_i.h"
#include "../widgets/clock_view.h"

#include <gui/modules/menu.h>
#include <gui/modules/anim_menu.h>
#include <gui/modules/overlap_fader.h>

typedef enum {
    ClockSceneMainEventStart = ClockEventSceneEventsStart,
    ClockSceneMainEventSetup,
} ClockSceneMainEvent;

typedef struct {
    Widget* front_container;
    ClockView* front_clock;

    Menu* back_menu;

    LocalTime displayed_time;
} ClockSceneMain;

static const TransitionOverlayPreset clock_scene_main_start_transition_overlay_preset = {
    .type = TransitionOverlayTypeMask,
    .blend_mode = TransitionOverlayBlendModeAdd,
    .timings =
        {
            .in_ms = 100,
            .out_ms = 1000,
        },
    .effect = TransitionOverlayEffectPress,
    .mask.file_path = SHARED_ANIM_PATH("transition_select_72x16.anim"),
};

static void clock_scene_main_menu_callback(uint32_t index, void* context) {
    furi_assert(context);

    Clock* instance = context;

    clock_internal_fire_event(instance, index);
}

static void clock_scene_main_on_enter(void* context) {
    furi_assert(context);

    Clock* instance = context;
    ClockSceneMain* scene =
        scene_manager_get_scene_data(instance->scene_manager, ClockSceneIdxMain);

    scene->displayed_time = time_get_local_time(instance->time);

    TimeSettings* time_settings = malloc(sizeof(*time_settings));
    time_get_settings(instance->time, time_settings);

    with_gui(instance->gui, {
        /* front layout setup */
        scene->front_container = widget_alloc(instance->front_scene_window);

        scene->front_clock = clock_view_alloc(scene->front_container);
        widget_set_align(clock_view_get_base(scene->front_clock), AlignLeftMid);

        clock_view_set_time_format(scene->front_clock, time_settings->time_format);
        clock_view_set_show_date(scene->front_clock, true);
        clock_view_set_show_seconds(scene->front_clock, false);
        clock_view_set_blink_colons(scene->front_clock, instance->settings.blink_colons);

        clock_view_set_date_time(scene->front_clock, &scene->displayed_time.dt);

        AnimMenu* front_menu = anim_menu_alloc(scene->front_container);
        anim_menu_set_source(front_menu, SHARED_ANIM_PATH("start_menu_31x16.anim"), 2);
        widget_set_align(anim_menu_get_base(front_menu), AlignRightMid);

        OverlapFader* front_fader = overlap_fader_alloc(scene->front_container);
        widget_set_width(overlap_fader_get_base(front_fader), 10);
        overlap_fader_align_to(front_fader, anim_menu_get_base(front_menu), OverlapFaderSideLeft);

        /* back layout setup */
        scene->back_menu = menu_alloc(instance->back_scene_window);
        menu_add_item(
            scene->back_menu,
            "Start",
            NULL,
            SHARED_IMG_PATH("start_11x11.image"),
            ClockSceneMainEventStart,
            clock_scene_main_menu_callback,
            instance);
        menu_add_item(
            scene->back_menu,
            "Setup",
            NULL,
            SHARED_IMG_PATH("setup_11x11.image"),
            ClockSceneMainEventSetup,
            clock_scene_main_menu_callback,
            instance);

        transition_overlay_start(instance->front_transition_overlay);
    });

    free(time_settings);
}

static void clock_scene_main_on_exit(void* context) {
    furi_assert(context);

    Clock* instance = context;
    ClockSceneMain* scene =
        scene_manager_get_scene_data(instance->scene_manager, ClockSceneIdxMain);

    with_gui(instance->gui, {
        widget_free(scene->front_container);

        menu_free(scene->back_menu);
    });
}

static bool clock_scene_main_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(context);

    Clock* instance = context;
    ClockSceneMain* scene =
        scene_manager_get_scene_data(instance->scene_manager, ClockSceneIdxMain);

    if(event->type == SceneManagerEventTypeCustom) {
        switch(event->event) {
        case ClockEventTimerUpdate:
            LocalTime local_time = time_get_local_time(instance->time);
            if(utz_udatetime_cmp(&scene->displayed_time.dt, &local_time.dt) != 0) {
                with_gui(instance->gui, {
                    clock_view_set_date_time(scene->front_clock, &local_time.dt);
                });
            }
            scene->displayed_time = local_time;
            return true;

        case ClockSceneMainEventStart:
            with_gui(instance->gui, {
                widget_set_visible(nav_bar_get_base(instance->back_nav_bar), false);
            });

            transition_overlay_set_preset(
                instance->front_transition_overlay,
                &clock_scene_main_start_transition_overlay_preset);
            transition_overlay_show(instance->front_transition_overlay);

            scene_manager_next_scene(instance->scene_manager, ClockSceneIdxClock);
            return true;

        case ClockSceneMainEventSetup:
            with_gui(instance->gui, { nav_bar_push_location(instance->back_nav_bar, "SETUP"); });
            scene_manager_next_scene(instance->scene_manager, ClockSceneIdxSetup);
            return true;

        default:
            break;
        }
    }

    return false;
}

const Scene clock_internal_scene_main = {
    .enter_callback = clock_scene_main_on_enter,
    .exit_callback = clock_scene_main_on_exit,
    .event_callback = clock_scene_main_on_event,
    .data_size = sizeof(ClockSceneMain),
};
