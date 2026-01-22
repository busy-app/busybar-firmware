#include "../busy_i.h"

#include <gui/modules/anim_player.h>

#include "../widgets/prompt_overlay.h"
#include "../widgets/summary_label.h"

typedef struct {
    AnimPlayer* front_anim;
    SummaryLabel* front_summary;
    PromptOverlay* front_prompt;
} BusySceneFinish;

static bool busy_scene_finish_input_callback(const InputEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);

    BusyApp* instance = context;

    bool consumed = false;
    BusyCustomEvent custom_event;

    if(event->key == InputKeyStart) {
        if(event->type == InputTypeShort) {
            custom_event = BusyCustomEventStartShortPressed;
            consumed = true;
        }
    }

    if(consumed) {
        busy_send_custom_event(instance, custom_event);
    }

    return consumed;
}

static void busy_scene_finish_prompt_overlay_callback(void* context) {
    furi_assert(context);
    BusyApp* instance = context;

    BusySceneFinish* data =
        scene_manager_get_scene_data(instance->scene_manager, BusyAppSceneIdFinish);

    summary_label_switch_display(data->front_summary);
}

static void busy_scene_finish_handle_back(BusyApp* instance) {
    busy_prepare_transition(instance, BusyTransitionTypeDefault);

    if(!busy_return_to_start_scene(instance)) {
        busy_exit(instance);
    }
}

static void busy_scene_finish_on_enter(void* context) {
    furi_assert(context);

    BusyApp* instance = context;
    BusySceneFinish* data =
        scene_manager_get_scene_data(instance->scene_manager, BusyAppSceneIdFinish);

    BusyTimerConfig timer_config;
    busy_timer_get_config(instance->busy_timer, &timer_config);

    with_gui(instance->gui, {
        GuiLayer* layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_add_input_callback(layer, busy_scene_finish_input_callback, instance);

        data->front_anim = anim_player_alloc(instance->front_window);
        if(anim_player_set_source(
               data->front_anim, BUSY_ANIM_PATH("finished_confetti_72x16.anim"))) {
            anim_player_loop_default(data->front_anim);
        }

        data->front_summary = summary_label_alloc(instance->front_window);
        widget_set_pos_y(summary_label_get_base(data->front_summary), 5);
        widget_set_align(summary_label_get_base(data->front_summary), AlignTopMid);

        data->front_prompt = prompt_overlay_alloc(instance->front_window);
        prompt_overlay_set_animation_target(
            data->front_prompt, summary_label_get_base(data->front_summary));

        if(timer_config.mode == BusyTimerModeInterval) {
            summary_label_set_cycles_count(data->front_summary, timer_config.cycle_count);
            prompt_overlay_set_callback(
                data->front_prompt, busy_scene_finish_prompt_overlay_callback, instance);
        }
    });

    audio_play_file(instance->audio, BUSY_SOUND_PATH("session_completed.snd"));

    busy_start_transition(instance);
}

static void busy_scene_finish_on_exit(void* context) {
    furi_assert(context);

    BusyApp* instance = context;
    BusySceneFinish* data =
        scene_manager_get_scene_data(instance->scene_manager, BusyAppSceneIdFinish);

    with_gui(instance->gui, {
        GuiLayer* layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_remove_input_callback(layer, busy_scene_finish_input_callback);

        anim_player_free(data->front_anim);
        summary_label_free(data->front_summary);
        prompt_overlay_free(data->front_prompt);
    });
}

static bool busy_scene_finish_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(context);
    BusyApp* instance = context;

    bool consumed = false;

    if(event->type == SceneManagerEventTypeCustom) {
        if(event->event == BusyCustomEventStartShortPressed) {
            busy_scene_finish_handle_back(instance);

        } else if(event->event == BusyCustomEventReturnToStart) {
            busy_scene_finish_handle_back(instance);
        }

        consumed = true;

    } else if(event->type == SceneManagerEventTypeBack) {
        busy_scene_finish_handle_back(instance);

        consumed = true;
    }

    return consumed;
}

const Scene busy_scene_finish = {
    .enter_callback = busy_scene_finish_on_enter,
    .exit_callback = busy_scene_finish_on_exit,
    .event_callback = busy_scene_finish_on_event,
    .data_size = sizeof(BusySceneFinish),
};
