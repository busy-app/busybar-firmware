#include "../busy_i.h"

#include <gui/modules/label.h>
#include <gui/modules/flex_box.h>
#include <gui/modules/anim_image.h>

#include "../widgets/progress_view.h"
#include "../widgets/prompt_overlay.h"

#define FLEX_OFFSET_INIT  (3)
#define FLEX_OFFSET_PRESS (FLEX_OFFSET_INIT + 1)

typedef struct {
    PromptOverlay* front_prompt;
    FlexBox* front_flex;
    ProgressView* front_progress_view;
    BusyTimerState timer_state;
} BusySceneNext;

typedef struct {
    const char* arrow_anim_path;
    const char* label_text;
    Color label_color;
} BusySceneNextPreset;

typedef enum {
    BusySceneNextPresetIdWork,
    BusySceneNextPresetIdRest,
    BusySceneNextPresetIdMax,
} BusySceneNextPresetId;

static const BusySceneNextPreset busy_scene_next_presets[BusySceneNextPresetIdMax];

static bool busy_scene_next_input_callback(const InputEvent* event, void* context) {
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

static void busy_scene_next_handle_start_short_pressed(BusyApp* instance) {
    const BusySceneNext* data =
        scene_manager_get_scene_data(instance->scene_manager, BusyAppSceneIdNext);

    const BusyTimerState timer_state = data->timer_state;
    furi_assert(timer_state != BusyTimerStateIdle);

    const BusyTransitionType transition_type =
        (timer_state == BusyTimerStateWork) ? BusyTransitionTypeWork : BusyTransitionTypeRest;

    busy_prepare_transition(instance, transition_type);
    scene_manager_search_and_switch_to_previous_scene(
        instance->scene_manager, BusyAppSceneIdTimer);
}

static void busy_scene_next_handle_back(BusyApp* instance) {
    busy_timer_stop(instance->busy_timer);

    busy_prepare_transition(instance, BusyTransitionTypeDefault);

    if(!busy_return_to_start_scene(instance)) {
        busy_exit(instance);
    }
}

static const BusySceneNextPreset* busy_scene_next_get_preset(const BusyTimerState timer_state) {
    const BusySceneNextPreset* ret;

    if(timer_state == BusyTimerStateWork) {
        ret = &busy_scene_next_presets[BusySceneNextPresetIdWork];
    } else if(timer_state == BusyTimerStateRest) {
        ret = &busy_scene_next_presets[BusySceneNextPresetIdRest];
    } else {
        furi_crash("Invalid BusyTimerState value");
    }

    return ret;
}

static void busy_scene_next_on_enter(void* context) {
    furi_assert(context);

    BusyApp* instance = context;
    BusySceneNext* data =
        scene_manager_get_scene_data(instance->scene_manager, BusyAppSceneIdNext);

    BusyTimerCycles timer_cycles;
    busy_timer_get_cycles(instance->busy_timer, &timer_cycles);

    const BusyTimerState timer_state = busy_timer_get_state(instance->busy_timer);

    with_gui(instance->gui, {
        GuiLayer* layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_add_input_callback(layer, busy_scene_next_input_callback, instance);

        data->front_flex = flex_box_alloc(instance->front_window);
        flex_box_set_flow(data->front_flex, FlexBoxFlowRow);
        flex_box_set_align(data->front_flex, FlexBoxAlignStart, FlexBoxAlignCenter);
        flex_box_set_spacing(data->front_flex, 2);
        widget_set_align(flex_box_get_base(data->front_flex), AlignTopMid);
        widget_set_pos_y(flex_box_get_base(data->front_flex), FLEX_OFFSET_INIT);

        Label* start_label = label_alloc(flex_box_get_base(data->front_flex));
        label_set_text(start_label, "Start");

        const BusySceneNextPreset* preset = busy_scene_next_get_preset(timer_state);

        if(preset->arrow_anim_path) {
            AnimImage* anim = anim_image_alloc(flex_box_get_base(data->front_flex));
            anim_image_set_source(anim, preset->arrow_anim_path);
        }

        Label* message_label = label_alloc(flex_box_get_base(data->front_flex));
        label_set_text(message_label, preset->label_text);
        label_set_text_color(message_label, preset->label_color);

        data->front_progress_view = progress_view_alloc(instance->front_window);
        progress_view_set_progress(
            data->front_progress_view,
            timer_cycles.current_idx - 1,
            timer_cycles.total_count,
            true);
        widget_set_align(progress_view_get_base(data->front_progress_view), AlignBottomMid);

        data->front_prompt = prompt_overlay_alloc(instance->front_window);
        prompt_overlay_set_animation_target(
            data->front_prompt, flex_box_get_base(data->front_flex));
    });

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

        prompt_overlay_free(data->front_prompt);
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
        if(event->event == BusyCustomEventStartShortPressed) {
            busy_scene_next_handle_start_short_pressed(instance);

        } else if(event->event == BusyCustomEventReturnToStart) {
            busy_scene_next_handle_back(instance);
        }

        consumed = true;

    } else if(event->type == SceneManagerEventTypeBack) {
        busy_scene_next_handle_back(instance);
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

static const BusySceneNextPreset busy_scene_next_presets[BusySceneNextPresetIdMax] = {
    [BusySceneNextPresetIdWork] =
        {
            .arrow_anim_path = BUSY_ANIM_PATH("arrow_red_5x5.anim"),
            .label_color = COLOR_MAKE_HEX(0xFF3C4A),
            .label_text = "BUSY",
        },
    [BusySceneNextPresetIdRest] =
        {
            .arrow_anim_path = BUSY_ANIM_PATH("arrow_green_5x5.anim"),
            .label_color = COLOR_MAKE_HEX(0x0AE974),
            .label_text = "REST",
        },
};
