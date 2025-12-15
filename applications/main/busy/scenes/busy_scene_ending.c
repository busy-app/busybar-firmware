#include "../busy_i.h"

#include <gui/modules/anim_image.h>

#include "../widgets/summary_view.h"

#include "../helpers/animate.h"

#define SUMMARY_SLIDE_POS_Y   (-6)
#define SUMMARY_SLIDE_TIME_MS (500)

typedef struct {
    SummaryView* front_summary;
    AnimImage* front_anim;
} BusySceneEndig;

static bool busy_scene_ending_input_callback(const InputEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);

    BusyApp* instance = context;

    bool consumed = false;
    BusyCustomEvent custom_event;

    if(event->type == InputTypeShort) {
        if(event->key == InputKeyStart) {
            custom_event = BusyCustomEventStartShortPressed;
            consumed = true;
        }
    }

    if(consumed) {
        busy_send_custom_event(instance, custom_event);
    }

    return consumed;
}

static void busy_scene_ending_summary_finished_callback(void* context) {
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
    busy_send_custom_event(instance, BusyCustomEventAnimationCompleted);
}

static void busy_scene_ending_handle_animation_completed(BusyApp* instance) {
    busy_prepare_transition(instance, BusyTransitionTypeEnding);
    scene_manager_next_scene(instance->scene_manager, BusyAppSceneIdFinish);
}

static void busy_scene_ending_handle_back(BusyApp* instance) {
    busy_prepare_transition(instance, BusyTransitionTypeDefault);

    if(!busy_return_to_start_scene(instance)) {
        busy_exit(instance);
    }
}

static void busy_scene_ending_on_enter(void* context) {
    furi_assert(context);

    BusyApp* instance = context;
    BusySceneEndig* data =
        scene_manager_get_scene_data(instance->scene_manager, BusyAppSceneIdEnding);

    BusyTimerConfig config;
    busy_timer_get_config(instance->busy_timer, &config);

    with_gui(instance->gui, {
        GuiLayer* layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_add_input_callback(layer, busy_scene_ending_input_callback, instance);

        data->front_summary = summary_view_alloc(instance->front_window);
        summary_view_set_cycles_count(data->front_summary, config.cycle_count);
        summary_view_set_completed_callback(
            data->front_summary, busy_scene_ending_summary_finished_callback, instance);
        widget_set_align(summary_view_get_base(data->front_summary), AlignCenter);

        data->front_anim = anim_image_alloc(instance->front_window);
        anim_image_set_source(data->front_anim, BUSY_ANIM_PATH("busy_ending_72x16.anim"));
        anim_image_set_completed_callback(
            data->front_anim, busy_scene_ending_anim_completed_callback, instance);
        anim_image_set_loop(data->front_anim, false);
        anim_image_stop(data->front_anim);
        widget_set_visible(anim_image_get_base(data->front_anim), false);

        animate_pos_y(
            summary_view_get_base(data->front_summary),
            SUMMARY_SLIDE_POS_Y,
            0,
            SUMMARY_SLIDE_TIME_MS);
    });

    busy_start_transition(instance);
}

static void busy_scene_ending_on_exit(void* context) {
    furi_assert(context);

    BusyApp* instance = context;
    BusySceneEndig* data =
        scene_manager_get_scene_data(instance->scene_manager, BusyAppSceneIdEnding);

    with_gui(instance->gui, {
        GuiLayer* layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_remove_input_callback(layer, busy_scene_ending_input_callback);

        summary_view_free(data->front_summary);
        anim_image_free(data->front_anim);
    });
}

static bool busy_scene_ending_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(context);
    BusyApp* instance = context;

    bool consumed = false;

    if(event->type == SceneManagerEventTypeCustom) {
        if(event->event == BusyCustomEventAnimationCompleted ||
           event->event == BusyCustomEventStartShortPressed) {
            busy_scene_ending_handle_animation_completed(instance);

        } else if(event->event == BusyCustomEventReturnToStart) {
            busy_scene_ending_handle_back(instance);
        }

        consumed = true;

    } else if(event->type == SceneManagerEventTypeBack) {
        busy_scene_ending_handle_back(instance);

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
