#include "../busy.h"

#include <gui/modules/label.h>
#include <gui/modules/anim_image.h>

#define WAIT_ANIM_BEGIN (0)
#define WAIT_ANIM_END   (179)

#define PRESS_ANIM_BEGIN (180)
#define PRESS_ANIM_END   (185)

typedef struct {
    Label* front_label;
    AnimImage* front_anim;
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

        if(state == BusyTimerStateIdle) {
            data->front_label = label_alloc(instance->front_window);
            label_set_text(data->front_label, "FINISHED!");
            widget_set_align(label_get_base(data->front_label), AlignCenter);

        } else {
            data->front_anim = anim_image_alloc(instance->front_window);

            if(state == BusyTimerStateWork) {
                anim_image_set_source(
                    data->front_anim, BUSY_ANIM_PATH("A_busy_waiting_72x16.anim"));
            } else if(state == BusyTimerStateRest) {
                anim_image_set_source(
                    data->front_anim, BUSY_ANIM_PATH("A_rest_waiting_72x16.anim"));
            }

            anim_image_set_range(data->front_anim, WAIT_ANIM_BEGIN, WAIT_ANIM_END, true, false);
        }
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

        if(data->front_label) {
            label_free(data->front_label);
            data->front_label = NULL;
        }

        if(data->front_anim) {
            anim_image_free(data->front_anim);
            data->front_anim = NULL;
        }
    });
}

static bool busy_scene_next_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(context);
    furi_assert(event);

    BusyApp* instance = context;

    bool consumed = false;

    if(event->type == SceneManagerEventTypeCustom) {
        const BusySceneNext* data = scene_manager_get_current_scene_data(instance->scene_manager);

        if(event->event == BusyCustomEventStartPressed) {
            if(data->front_anim) {
                anim_image_set_range(
                    data->front_anim, PRESS_ANIM_BEGIN, PRESS_ANIM_END, false, false);
            }

        } else if(event->event == BusyCustomEventStartReleased) {
            if(data->timer_state == BusyTimerStateIdle) {
                scene_manager_search_and_switch_to_previous_scene(
                    instance->scene_manager, BusyAppSceneIdStart);
            } else {
                scene_manager_search_and_switch_to_previous_scene(
                    instance->scene_manager, BusyAppSceneIdTimer);
            }
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
