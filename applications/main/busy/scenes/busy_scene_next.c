#include "../busy_i.h"

#include <gui/modules/label.h>
#include <gui/modules/flex_box.h>
#include <gui/modules/anim_image.h>

#include "../widgets/progress_view.h"

#define WAIT_ANIM_BEGIN (0)
#define WAIT_ANIM_END   (179)

#define PRESS_ANIM_BEGIN (180)
#define PRESS_ANIM_END   (185)

typedef struct {
    FlexBox* front_flex;
    ProgressView* front_progress_view;
    BusyTimerState timer_state;
} BusySceneNext;

typedef struct {
    const char* arrow_anim_path;
    const char* label_text;
    Color label_color;
} BusySceneNextPreset;

static const BusySceneNextPreset busy_scene_next_presets[BusyTimerStateMax];

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
    BusySceneNext* data =
        scene_manager_get_scene_data(instance->scene_manager, BusyAppSceneIdNext);

    const BusyTimerState timer_state = busy_timer_get_state(instance->busy_timer);

    BusyTimerCycles timer_cycles;
    busy_timer_get_cycles(instance->busy_timer, &timer_cycles);

    const uint32_t prev_interval_idx = timer_cycles.current_idx - 1;
    const BusySceneNextPreset* preset = &busy_scene_next_presets[timer_state];

    with_gui(instance->gui, {
        GuiLayer* layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_add_input_callback(layer, busy_scene_next_input_callback, instance);

        data->front_flex = flex_box_alloc(instance->front_window);
        flex_box_set_flow(data->front_flex, FlexBoxFlowRow);
        flex_box_set_align(data->front_flex, FlexBoxAlignStart, FlexBoxAlignCenter);
        flex_box_set_spacing(data->front_flex, 2);
        widget_set_align(flex_box_get_base(data->front_flex), AlignTopMid);
        widget_set_pos_y(flex_box_get_base(data->front_flex), 3);

        Label* start_label = label_alloc(flex_box_get_base(data->front_flex));
        label_set_text(start_label, "Start");

        if(preset->arrow_anim_path) {
            AnimImage* anim = anim_image_alloc(flex_box_get_base(data->front_flex));
            anim_image_set_source(anim, preset->arrow_anim_path);
        }

        Label* message_label = label_alloc(flex_box_get_base(data->front_flex));
        label_set_text(message_label, preset->label_text);
        label_set_text_color(message_label, preset->label_color);

        data->front_progress_view = progress_view_alloc(instance->front_window);
        progress_view_set_progress(
            data->front_progress_view, prev_interval_idx, timer_cycles.total_count, true);
        widget_set_align(progress_view_get_base(data->front_progress_view), AlignBottomMid);
    });

    if(timer_state == BusyTimerStateIdle) {
        audio_play_file(instance->audio, BUSY_SOUND_PATH("session_completed.snd"));
    }

    data->timer_state = timer_state;

    busy_start_transition(instance);
}

static void busy_scene_next_on_exit(void* context) {
    furi_assert(context);

    BusyApp* instance = context;
    BusySceneNext* data =
        scene_manager_get_scene_data(instance->scene_manager, BusyAppSceneIdNext);

    with_gui(instance->gui, {
        GuiLayer* layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_remove_input_callback(layer, busy_scene_next_input_callback);

        flex_box_free(data->front_flex);
        progress_view_free(data->front_progress_view);
    });
}

static bool busy_scene_next_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(context);
    furi_assert(event);

    BusyApp* instance = context;

    bool consumed = false;

    if(event->type == SceneManagerEventTypeCustom) {
        const BusySceneNext* data =
            scene_manager_get_scene_data(instance->scene_manager, BusyAppSceneIdNext);

        if(event->event == BusyCustomEventStartPressed) {
        } else if(event->event == BusyCustomEventStartReleased) {
            BusyAppSceneId scene_id;
            BusyTransitionType transition_type;

            const BusyTimerState timer_state = data->timer_state;

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
        if(!busy_return_to_start_scene(instance)) {
            busy_exit(instance);
        }

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

static const BusySceneNextPreset busy_scene_next_presets[BusyTimerStateMax] = {
    // TODO: Remove later
    [BusyTimerStateIdle] =
        {
            .label_color = COLOR_MAKE_HEX(0xFFFFFF),
            .label_text = "Finished!",
        },
    [BusyTimerStateWork] =
        {
            .arrow_anim_path = BUSY_ANIM_PATH("arrow_red_5x5.anim"),
            .label_color = COLOR_MAKE_HEX(0xFF3C4A),
            .label_text = "BUSY",
        },
    [BusyTimerStateRest] =
        {
            .arrow_anim_path = BUSY_ANIM_PATH("arrow_green_5x5.anim"),
            .label_color = COLOR_MAKE_HEX(0x0AE974),
            .label_text = "REST",
        },
};
