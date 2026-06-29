#include "../clock_i.h"
#include "../widgets/clock_view.h"

#include <gui/modules/menu.h>
#include <gui/modules/anim_menu.h>
#include <gui/modules/overlap_fader.h>
#include <gui/modules/mirror_card.h>

typedef struct {
    ClockView* front_clock;

    MirrorCard* back_card;

    LocalTime displayed_time;
} ClockSceneClock;

static const TransitionOverlayPreset clock_scene_clock_back_transition_overlay_preset = {
    .type = TransitionOverlayTypeColor,
    .blend_mode = TransitionOverlayBlendModeNormal,
    .timings =
        {
            .in_ms = 200,
            .out_ms = 200,
        },
    .mask.color = COLOR_MAKE_HEX(0x000000),
};

static void clock_scene_clock_on_enter(void* context) {
    furi_assert(context);

    Clock* instance = context;
    ClockSceneClock* scene =
        scene_manager_get_scene_data(instance->scene_manager, ClockSceneIdxClock);

    TimeSettings* time_settings = malloc(sizeof(*time_settings));
    time_get_settings(instance->time, time_settings);

    scene->displayed_time = time_get_local_time(instance->time);

    with_gui(instance->gui, {
        /* front layout setup */
        scene->front_clock = clock_view_alloc(instance->front_scene_window);
        widget_set_align(clock_view_get_base(scene->front_clock), AlignCenter);

        clock_view_set_time_format(scene->front_clock, time_settings->time_format);
        clock_view_set_show_date(scene->front_clock, instance->settings.show_date);
        clock_view_set_show_seconds(scene->front_clock, instance->settings.show_seconds);
        clock_view_set_blink_colons(scene->front_clock, instance->settings.blink_colons);

        clock_view_set_date_time(scene->front_clock, &scene->displayed_time.dt);

        /* back layout setup */
        scene->back_card = mirror_card_alloc(instance->back_scene_window);
        mirror_card_set_header_text(scene->back_card, "CLOCK");
        mirror_card_set_show_footer(scene->back_card, false);
        widget_set_align(mirror_card_get_base(scene->back_card), AlignCenter);
        widget_set_margin(mirror_card_get_base(scene->back_card), 0, 0, 2, 2);

        transition_overlay_start(instance->front_transition_overlay);
    });

    free(time_settings);
}

static void clock_scene_clock_on_exit(void* context) {
    furi_assert(context);

    Clock* instance = context;
    ClockSceneClock* scene =
        scene_manager_get_scene_data(instance->scene_manager, ClockSceneIdxClock);

    with_gui(instance->gui, {
        clock_view_free(scene->front_clock);
        mirror_card_free(scene->back_card);
    });
}

static bool clock_scene_clock_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(context);

    Clock* instance = context;
    ClockSceneClock* scene =
        scene_manager_get_scene_data(instance->scene_manager, ClockSceneIdxClock);

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

        default:
            break;
        }
    } else if(event->type == SceneManagerEventTypeBack) {
        with_gui(instance->gui, {
            widget_set_visible(nav_bar_get_base(instance->back_nav_bar), true);

            transition_overlay_set_preset(
                instance->front_transition_overlay,
                &clock_scene_clock_back_transition_overlay_preset);
            transition_overlay_show(instance->front_transition_overlay);
        });

        scene_manager_previous_scene(instance->scene_manager);

        return true;
    }

    return false;
}

const Scene clock_internal_scene_clock = {
    .enter_callback = clock_scene_clock_on_enter,
    .exit_callback = clock_scene_clock_on_exit,
    .event_callback = clock_scene_clock_on_event,
    .data_size = sizeof(ClockSceneClock),
};
