#include "../busy.h"

#include <gui/modules/label.h>

typedef struct {
    Label* front_label;
    BusyTimerState timer_state;
} BusySceneNext;

static bool busy_scene_next_input_callback(const InputEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);

    BusyApp* instance = context;

    bool consumed = false;
    BusyCustomEvent custom_event;

    if(event->key == InputKeyStart) {
        if(event->type == InputTypePress) {
            custom_event = BusyCustomEventStartPressed;
            consumed = true;

        } else if(event->type == InputTypeRelease) {
            custom_event = BusyCustomEventStartReleased;
            consumed = true;
        }
    }

    if(consumed) {
        busy_send_custom_event(instance, custom_event);
    }

    return consumed;
}

static void busy_scene_next_on_enter(void* context) {
    furi_assert(context);

    BusyApp* instance = context;
    BusySceneNext* data = scene_manager_get_current_scene_data(instance->scene_manager);

    const BusyTimerState state = busy_timer_get_state(instance->busy_timer);

    with_gui(instance->gui, {
        GuiLayer* layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_add_input_callback(layer, busy_scene_next_input_callback, instance);

        data->front_label = label_alloc(instance->front_window);

        if(state == BusyTimerStateIdle) {
            label_set_text(data->front_label, "FINISHED!");
        } else if(state == BusyTimerStateWork) {
            label_set_text(data->front_label, "BUSY...");
        } else if(state == BusyTimerStateRest) {
            label_set_text(data->front_label, "REST...");
        }

        widget_set_align(label_get_base(data->front_label), AlignCenter);
    });

    data->timer_state = state;
}

static void busy_scene_next_on_exit(void* context) {
    furi_assert(context);

    BusyApp* instance = context;
    BusySceneNext* data = scene_manager_get_current_scene_data(instance->scene_manager);

    with_gui(instance->gui, {
        GuiLayer* layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_remove_input_callback(layer, busy_scene_next_input_callback);

        label_free(data->front_label);
    });
}

static bool busy_scene_next_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(context);
    furi_assert(event);

    BusyApp* instance = context;

    bool consumed = false;

    if(event->type == SceneManagerEventTypeCustom) {
        if(event->event == BusyCustomEventStartPressed) {
            // TODO: Play different animation sequence

        } else if(event->event == BusyCustomEventStartReleased) {
            scene_manager_search_and_switch_to_previous_scene(
                instance->scene_manager, BusyAppSceneIdTimer);
        }

        consumed = true;

    } else if(event->type == SceneManagerEventTypeBack) {
        consumed = true;
    }

    return consumed;
}

const Scene busy_scene_next = {
    .enter_callback = busy_scene_next_on_enter,
    .exit_callback = busy_scene_next_on_exit,
    .event_callback = busy_scene_next_on_event,
    .data_size = sizeof(BusySceneNext),
};
