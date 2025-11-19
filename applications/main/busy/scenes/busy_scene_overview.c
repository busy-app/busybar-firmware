#include "../busy.h"

#include <gui/modules/anim_image.h>

#include "../widgets/overview_label.h"

typedef struct {
    AnimImage* front_bg_anim;
    OverviewLabel* front_overview_label;
    RunLater* run_later;
} BusySceneOverview;

static void busy_scene_overview_run_later_callback(void* context) {
    furi_assert(context);
    BusyApp* instance = context;

    busy_prepare_transition(instance, BusyTransitionTypeAutomatic);
    scene_manager_next_scene(instance->scene_manager, BusyAppSceneIdTimer);
}

static bool busy_scene_overview_input_callback(const InputEvent* event, void* context) {
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

static void busy_scene_overview_on_enter(void* context) {
    furi_assert(context);

    BusyApp* instance = context;
    BusySceneOverview* data =
        scene_manager_get_scene_data(instance->scene_manager, BusyAppSceneIdOverview);

    BusyTimerConfig timer_config;
    busy_timer_get_config(instance->busy_timer, &timer_config);

    with_gui(instance->gui, {
        GuiLayer* layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_add_input_callback(layer, busy_scene_overview_input_callback, instance);

        data->front_bg_anim = anim_image_alloc(instance->front_window);
        anim_image_set_source(data->front_bg_anim, BUSY_ANIM_PATH("overview_72x16.anim"));
        anim_image_set_loop(data->front_bg_anim, false);

        data->front_overview_label = overview_label_alloc(instance->front_window);
        overview_label_set_intervals(
            data->front_overview_label, timer_config.work_time_mn, timer_config.rest_time_mn);
    });

    data->run_later =
        run_later(instance->event_loop, busy_scene_overview_run_later_callback, instance, 2250);

    busy_start_transition(instance);
}

static void busy_scene_overview_on_exit(void* context) {
    furi_assert(context);

    BusyApp* instance = context;
    BusySceneOverview* data =
        scene_manager_get_scene_data(instance->scene_manager, BusyAppSceneIdOverview);

    run_later_cancel(data->run_later);

    with_gui(instance->gui, {
        GuiLayer* layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_remove_input_callback(layer, busy_scene_overview_input_callback);

        anim_image_free(data->front_bg_anim);
        overview_label_free(data->front_overview_label);
    });
}

static bool busy_scene_overview_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(context);
    BusyApp* instance = context;

    bool consumed = false;

    if(event->type == SceneManagerEventTypeCustom) {
        if(event->event == BusyCustomEventStartShortPressed) {
            busy_prepare_transition(instance, BusyTransitionTypeSkip);
            scene_manager_next_scene(instance->scene_manager, BusyAppSceneIdTimer);
        }

        consumed = true;

    } else if(event->type == SceneManagerEventTypeBack) {
        busy_prepare_transition(instance, BusyTransitionTypeDefault);
    }

    return consumed;
}

const Scene busy_scene_overview = {
    .enter_callback = busy_scene_overview_on_enter,
    .exit_callback = busy_scene_overview_on_exit,
    .event_callback = busy_scene_overview_on_event,
    .data_size = sizeof(BusySceneOverview),
};
