#include "timer_indicator.h"

#include <gui/modules/anim_play_i.h>

#define MY_CLASS (&timer_indicator_lvgl_class)

#define FRAMES_TO_MS(x) ((x) * 1000 / 60)

struct TimerIndicator {
    AnimPlay base;
    TimerIndicatorAnimSources sources;
    TimerIndicatorState state;
};

typedef struct {
    int32_t start_width;
    int32_t end_width;
    uint32_t duration_ms;
} TimerTransitionPreset;

const lv_obj_class_t timer_indicator_lvgl_class;

static const TimerTransitionPreset timer_indicator_transition_presets[TimerIndicatorTransitionMax];

// LVGL-specific code

static void timer_indicator_lvgl_anim_callback(void* context, int32_t value) {
    furi_assert(context);

    lv_obj_t* instance = context;
    lv_obj_set_width(instance, value);
}

static void timer_indicator_lvgl_anim_completed_callback(lv_anim_t* anim) {
    furi_assert(anim);

    TimerIndicator* instance = anim->var;
    furi_assert(instance);

    AnimPlay* anim_play = (AnimPlay*)instance;

    anim_play_set_source(anim_play, instance->sources.states[instance->state]);

    widget_set_width((Widget*)instance, LV_SIZE_CONTENT);
}

static void timer_indicator_lvgl_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    UNUSED(class_p);

    TimerIndicator* instance = (TimerIndicator*)obj;
    instance->state = TimerIndicatorStateMax;
}

// Implementation

static void
    timer_indicator_run_transition(TimerIndicator* instance, TimerIndicatorTransition transition) {
    furi_assert(transition < TimerIndicatorTransitionMax);

    const TimerTransitionPreset* const preset = &timer_indicator_transition_presets[transition];

    lv_anim_t anim;
    lv_anim_init(&anim);

    lv_anim_set_values(&anim, preset->start_width, preset->end_width);
    lv_anim_set_duration(&anim, preset->duration_ms);

    lv_anim_set_bezier3_param(
        &anim,
        LV_BEZIER_VAL_FLOAT(0.3F),
        LV_BEZIER_VAL_FLOAT(0.0F),
        LV_BEZIER_VAL_FLOAT(0.3F),
        LV_BEZIER_VAL_FLOAT(1.0F));

    lv_anim_set_path_cb(&anim, lv_anim_path_custom_bezier3);
    lv_anim_set_exec_cb(&anim, timer_indicator_lvgl_anim_callback);
    lv_anim_set_completed_cb(&anim, timer_indicator_lvgl_anim_completed_callback);
    lv_anim_set_var(&anim, instance);

    lv_anim_start(&anim);
}

// Public API

TimerIndicator* timer_indicator_alloc(Widget* parent) {
    furi_check(parent);

    lv_obj_t* obj = lv_obj_class_create_obj(MY_CLASS, (lv_obj_t*)parent);
    lv_obj_class_init_obj(obj);

    TimerIndicator* instance = (TimerIndicator*)obj;
    return instance;
}

void timer_indicator_free(TimerIndicator* instance) {
    furi_check(instance);
    lv_obj_delete((lv_obj_t*)instance);
}

Widget* timer_indicator_get_base(TimerIndicator* instance) {
    furi_check(instance);
    return (Widget*)instance;
}

AnimPlay* timer_indicator_get_anim_play(TimerIndicator* instance) {
    furi_check(instance);
    return (AnimPlay*)instance;
}

void timer_indicator_set_anim_sources(
    TimerIndicator* instance,
    const TimerIndicatorAnimSources* sources) {
    furi_check(instance);
    furi_check(sources);

    instance->sources = *sources;
}

void timer_indicator_set_state(TimerIndicator* instance, TimerIndicatorState state) {
    furi_check(instance);
    furi_check(state < TimerIndicatorStateMax);

    AnimPlay* anim_play = (AnimPlay*)instance;

    if(instance->state == TimerIndicatorStateWorkBig && state == TimerIndicatorStateWork) {
        const TimerIndicatorTransition transition = TimerIndicatorTransitionOffToSimple;

        if(anim_play_set_source(anim_play, instance->sources.transitions[transition])) {
            AnimFile* file = anim_play_get_file(anim_play);
            furi_assert(file);
            bool success = anim_file_set_section_indexed(
                file, AnimFilePlayFlagLoop, ANIM_FILE_WHOLE_SECTION_INDEX);
            UNUSED(success);
        }

        timer_indicator_run_transition(instance, transition);

    } else {
        if(anim_play_set_source(anim_play, instance->sources.states[state])) {
            AnimFile* file = anim_play_get_file(anim_play);
            furi_assert(file);
            bool success = anim_file_set_section_indexed(
                file, AnimFilePlayFlagLoop, ANIM_FILE_WHOLE_SECTION_INDEX);
            UNUSED(success);
        }
    }

    instance->state = state;
}

// LVGL class descriptor

const lv_obj_class_t timer_indicator_lvgl_class = {
    .base_class = &anim_play_lvgl_class,
    .constructor_cb = timer_indicator_lvgl_constructor,
    .name = "widget-timer-indicator",
    .width_def = LV_SIZE_CONTENT,
    .height_def = LV_SIZE_CONTENT,
    .instance_size = sizeof(TimerIndicator),
};

// Presets

static const TimerTransitionPreset
    timer_indicator_transition_presets[TimerIndicatorTransitionMax] = {
        [TimerIndicatorTransitionOffToSimple] =
            {
                .start_width = 70,
                .end_width = 40,
                .duration_ms = FRAMES_TO_MS(40),
            },
};
