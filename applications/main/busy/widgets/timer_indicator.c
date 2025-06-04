#include "timer_indicator.h"

#include <gui/modules/anim_image_i.h>

#define MY_CLASS (&timer_indicator_lvgl_class)

struct TimerIndicator {
    AnimImage base;
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

    AnimImage* anim_image = (AnimImage*)instance;

    anim_image_set_source(anim_image, instance->sources.states[instance->state]);
    anim_image_set_loop(anim_image, true);

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

    lv_anim_set_path_cb(&anim, lv_anim_path_ease_in_out);
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

AnimImage* timer_indicator_get_anim_image(TimerIndicator* instance) {
    furi_check(instance);
    return (AnimImage*)instance;
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

    AnimImage* anim_image = (AnimImage*)instance;

    if(instance->state == TimerIndicatorStateWorkBig && state == TimerIndicatorStateWork) {
        const TimerIndicatorTransition transition = TimerIndicatorTransitionOffToSimple;

        anim_image_set_source(anim_image, instance->sources.transitions[transition]);
        anim_image_set_loop(anim_image, false);

        timer_indicator_run_transition(instance, transition);

    } else {
        anim_image_set_source(anim_image, instance->sources.states[state]);
        anim_image_set_loop(anim_image, true);
    }

    instance->state = state;
}

// LVGL class descriptor

const lv_obj_class_t timer_indicator_lvgl_class = {
    .base_class = &anim_image_lvgl_class,
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
                .duration_ms = 500,
            },
};
