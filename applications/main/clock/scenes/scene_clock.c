#include "../clock_i.h"
#include "../widgets/clock_view.h"

#include <gui/modules/menu.h>
#include <gui/modules/anim_menu.h>
#include <gui/modules/overlap_fader.h>
#include <gui/modules/mirror_card.h>

typedef struct {
    ClockView* front_clock;

    MirrorCard* back_card;
} ThisScene;

static inline ThisScene* this_get_scene(ThisInstance* instance) {
    return scene_manager_get_scene_data(instance->scene_manager, ThisSceneIdxClock);
}

static void this_scene_on_enter(void* context) {
    furi_assert(context);

    ThisInstance* instance = context;
    ThisScene* scene = this_get_scene(instance);

    TimeSettings* time_settings = malloc(sizeof(*time_settings));
    time_get_settings(instance->time, time_settings);

    LocalTime local_time = time_get_local_time(instance->time);

    with_gui(instance->gui, {
        /* front layout setup */
        scene->front_clock = clock_view_alloc(instance->front_scene_window);
        widget_set_align(clock_view_get_base(scene->front_clock), AlignCenter);

        clock_view_set_show_date(scene->front_clock, instance->settings.show_date);
        clock_view_set_show_seconds(scene->front_clock, instance->settings.show_seconds);
        clock_view_set_time_format(scene->front_clock, time_settings->time_format);
        clock_view_set_date_time(scene->front_clock, &local_time.dt);

        /* back layout setup */
        scene->back_card = mirror_card_alloc(instance->back_scene_window);
        mirror_card_set_header_text(scene->back_card, "CLOCK");
        mirror_card_set_show_footer(scene->back_card, false);
        widget_set_align(mirror_card_get_base(scene->back_card), AlignLeftMid);
    });

    free(time_settings);
}

static void this_scene_on_exit(void* context) {
    furi_assert(context);

    ThisInstance* instance = context;
    ThisScene* scene = this_get_scene(instance);

    with_gui(instance->gui, {
        clock_view_free(scene->front_clock);
        mirror_card_free(scene->back_card);
    });
}

static bool this_scene_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(context);

    ThisInstance* instance = context;
    ThisScene* scene = this_get_scene(instance);

    if(event->type == SceneManagerEventTypeCustom) {
        switch(event->event) {
        case ThisEventTimerUpdate:
            LocalTime local_time = time_get_local_time(instance->time);
            with_gui(instance->gui, {
                clock_view_set_date_time(scene->front_clock, &local_time.dt);
            });
            return true;

        default:
            break;
        }
    } else if(event->type == SceneManagerEventTypeBack) {
        with_gui(instance->gui, {
            widget_set_visible(nav_bar_get_base(instance->back_nav_bar), true);
        });
    }

    return false;
}

const Scene clock_app_scene_clock = {
    .enter_callback = this_scene_on_enter,
    .exit_callback = this_scene_on_exit,
    .event_callback = this_scene_on_event,
    .data_size = sizeof(ThisScene),
};
