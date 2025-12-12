#include "../busy_i.h"

#include <gui/modules/anim_image.h>

#include "../widgets/summary_view.h"

typedef struct {
    SummaryView* front_summary;
    AnimImage* front_anim;
} BusySceneEndig;

static void busy_scnene_ending_summary_finished_callback(void* context) {
    furi_assert(context);

    BusyApp* instance = context;
    BusySceneEndig* data =
        scene_manager_get_scene_data(instance->scene_manager, BusyAppSceneIdEnding);

    widget_set_visible(anim_image_get_base(data->front_anim), true);
    anim_image_start(data->front_anim);
}

static void busy_scene_ending_anim_completed_callback(AnimImage* anim_image, void* context) {
    furi_assert(context);
    UNUSED(anim_image);

    BusyApp* instance = context;
    busy_send_custom_event(instance, BusyCustomEventAnimationEnded);
}

static void busy_scene_ending_on_enter(void* context) {
    furi_assert(context);

    BusyApp* instance = context;
    BusySceneEndig* data =
        scene_manager_get_scene_data(instance->scene_manager, BusyAppSceneIdEnding);

    BusyTimerConfig config;
    busy_timer_get_config(instance->busy_timer, &config);

    with_gui(instance->gui, {
        data->front_summary = summary_view_alloc(instance->front_window);
        // TODO: take cycles count into account
        // summary_view_set_cycles_count(data->front_summary, config.cycle_count);
        summary_view_set_cycles_count(data->front_summary, 10);
        summary_view_set_finished_callback(
            data->front_summary, busy_scnene_ending_summary_finished_callback, instance);
        widget_set_align(summary_view_get_base(data->front_summary), AlignCenter);

        data->front_anim = anim_image_alloc(instance->front_window);
        anim_image_set_source(data->front_anim, BUSY_ANIM_PATH("busy_ending_72x16.anim"));
        anim_image_set_completed_callback(
            data->front_anim, busy_scene_ending_anim_completed_callback, instance);
        anim_image_set_loop(data->front_anim, false);
        anim_image_stop(data->front_anim);
        widget_set_visible(anim_image_get_base(data->front_anim), false);
    });

    busy_start_transition(instance);
}

static void busy_scene_ending_on_exit(void* context) {
    furi_assert(context);

    BusyApp* instance = context;
    BusySceneEndig* data =
        scene_manager_get_scene_data(instance->scene_manager, BusyAppSceneIdEnding);

    with_gui(instance->gui, {
        summary_view_free(data->front_summary);
        anim_image_free(data->front_anim);
    });
}

static bool busy_scene_ending_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(context);
    BusyApp* instance = context;

    bool consumed = false;

    if(event->type == SceneManagerEventTypeCustom) {
        if(event->event == BusyCustomEventAnimationEnded) {
            busy_prepare_transition(instance, BusyTransitionTypeEnding);
            scene_manager_next_scene(instance->scene_manager, BusyAppSceneIdFinish);
        }

        consumed = true;

    } else if(event->type == SceneManagerEventTypeBack) {
        consumed = true;
    }

    return consumed;
}

const Scene busy_scene_ending = {
    .enter_callback = busy_scene_ending_on_enter,
    .exit_callback = busy_scene_ending_on_exit,
    .event_callback = busy_scene_ending_on_event,
    .data_size = sizeof(BusySceneEndig),
};
