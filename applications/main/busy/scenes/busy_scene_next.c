#include "../busy.h"

#include <gui/modules/label.h>
#include <gui/modules/anim_image.h>

#define WAIT_ANIM_BEGIN (0)
#define WAIT_ANIM_END   (179)

#define PRESS_ANIM_BEGIN (180)
#define PRESS_ANIM_END   (185)

typedef struct {
    AnimImage* front_anim;
    BusyTimerState timer_state;
} BusySceneNext;

static const char* front_anim_file_path[BusyTimerStateMax] = {
    [BusyTimerStateIdle] = BUSY_ANIM_PATH("finish_waiting_72x16.anim"),
    [BusyTimerStateWork] = BUSY_ANIM_PATH("busy_waiting_72x16.anim"),
    [BusyTimerStateRest] = BUSY_ANIM_PATH("rest_waiting_72x16.anim"),
};

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

    data->timer_state = busy_timer_get_state(instance->busy_timer);

    with_gui(instance->gui, {
        GuiLayer* layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_add_input_callback(layer, busy_scene_next_input_callback, instance);

        data->front_anim = anim_image_alloc(instance->front_window);

        anim_image_set_source(data->front_anim, front_anim_file_path[data->timer_state]);
        anim_image_set_range(data->front_anim, WAIT_ANIM_BEGIN, WAIT_ANIM_END, true, false);
    });

    if(data->timer_state == BusyTimerStateIdle) {
        audio_play_file(instance->audio, BUSY_SOUND_PATH("session_completed.snd"));
    }

    busy_start_transition(instance);
}

static void busy_scene_next_on_exit(void* context) {
    furi_assert(context);

    BusyApp* instance = context;
    BusySceneNext* data = scene_manager_get_current_scene_data(instance->scene_manager);

    with_gui(instance->gui, {
        GuiLayer* layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_remove_input_callback(layer, busy_scene_next_input_callback);

        anim_image_free(data->front_anim);
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
                with_gui(instance->gui, {
                    anim_image_set_range(
                        data->front_anim, PRESS_ANIM_BEGIN, PRESS_ANIM_END, false, false);
                });
            }

        } else if(event->event == BusyCustomEventStartReleased) {
            const BusyTimerState timer_state = data->timer_state;
            BusyTransitionType transition_type;
            BusyAppSceneId scene_id;

            if(timer_state == BusyTimerStateIdle) {
                scene_id = BusyAppSceneIdStart;
                transition_type = BusyTransitionTypeDefault;

            } else {
                scene_id = BusyAppSceneIdTimer;
                transition_type = (timer_state == BusyTimerStateWork) ? BusyTransitionTypeWork :
                                                                        BusyTransitionTypeRest;
            }

            busy_prepare_transition(instance, transition_type);
            scene_manager_search_and_switch_to_previous_scene(instance->scene_manager, scene_id);
        }

        consumed = true;

    } else if(event->type == SceneManagerEventTypeBack) {
        busy_timer_stop(instance->busy_timer);

        busy_prepare_transition(instance, BusyTransitionTypeDefault);
        scene_manager_search_and_switch_to_previous_scene(
            instance->scene_manager, BusyAppSceneIdStart);

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
