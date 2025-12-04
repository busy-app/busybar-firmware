#include "timer_indicator.h"

#include <gui/widget_i.h>
#include <gui/modules/image.h>
#include <gui/modules/anim_image.h>
#include <gui/modules/lottie_animation.h>

#define MY_CLASS (&timer_indicator_lvgl_class)

#define FRAMES_TO_MS(x) ((x) * 1000 / 60)

#define SLOT_TEMPLATE \
    "{"               \
    " \"%s\": {"      \
    "  \"p\": {"      \
    "   \"a\": 0,"    \
    "   \"k\": ["     \
    "     %.2f,"      \
    "     %.2f"       \
    "   ]"            \
    "}}}"

#define SLOT_STR_LEN (sizeof(SLOT_TEMPLATE) + 20)

struct TimerIndicator {
    Widget base;
    AnimImage* bg_anim;
    LottieAnimation* progress_lottie;
    Image* fg_image;
    char slot_store[SLOT_STR_LEN];
    TimerIndicatorProgressDirection progress_dir;
    uint8_t progress_start_offset_px;
    uint8_t progress_end_offset_px;
};

// typedef struct {
//     int32_t start_width;
//     int32_t end_width;
//     uint32_t duration_ms;
// } TimerTransitionPreset;

const lv_obj_class_t timer_indicator_lvgl_class;

// LVGL-specific code

// static void timer_indicator_lvgl_anim_callback(void* context, int32_t value) {
//     furi_assert(context);
//
//     lv_obj_t* instance = context;
//     lv_obj_set_width(instance, value);
// }
//
// static void timer_indicator_lvgl_anim_completed_callback(lv_anim_t* anim) {
//     furi_assert(anim);
//
//     TimerIndicator* instance = anim->var;
//     furi_assert(instance);
//
//     AnimImage* anim_image = (AnimImage*)instance;
//
//     anim_image_set_source(anim_image, instance->sources.states[instance->state]);
//     anim_image_set_loop(anim_image, true);
//
//     widget_set_width((Widget*)instance, LV_SIZE_CONTENT);
// }

static void timer_indicator_lvgl_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    UNUSED(class_p);

    TimerIndicator* instance = (TimerIndicator*)obj;
    UNUSED(instance);
}

static void timer_indicator_lvgl_destructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    UNUSED(class_p);

    TimerIndicator* instance = (TimerIndicator*)obj;
    if(instance->bg_anim) {
        anim_image_free(instance->bg_anim);
    }
    if(instance->progress_lottie) {
        lottie_animation_free(instance->progress_lottie);
    }
    if(instance->fg_image) {
        image_free(instance->fg_image);
    }
}

// Implementation

// static void
//     timer_indicator_run_transition(TimerIndicator* instance, TimerIndicatorTransition transition) {
//     furi_assert(transition < TimerIndicatorTransitionMax);
//
//     const TimerTransitionPreset* const preset = &timer_indicator_transition_presets[transition];
//
//     lv_anim_t anim;
//     lv_anim_init(&anim);
//
//     lv_anim_set_values(&anim, preset->start_width, preset->end_width);
//     lv_anim_set_duration(&anim, preset->duration_ms);
//
//     lv_anim_set_bezier3_param(
//         &anim,
//         LV_BEZIER_VAL_FLOAT(0.3F),
//         LV_BEZIER_VAL_FLOAT(0.0F),
//         LV_BEZIER_VAL_FLOAT(0.3F),
//         LV_BEZIER_VAL_FLOAT(1.0F));
//
//     lv_anim_set_path_cb(&anim, lv_anim_path_custom_bezier3);
//     lv_anim_set_exec_cb(&anim, timer_indicator_lvgl_anim_callback);
//     lv_anim_set_completed_cb(&anim, timer_indicator_lvgl_anim_completed_callback);
//     lv_anim_set_var(&anim, instance);
//
//     lv_anim_start(&anim);
// }

static void timer_indicator_set_bg_animation(
    TimerIndicator* instance,
    const TimerIndicatorBgConfig* config) {
    if(instance->bg_anim) {
        anim_image_free(instance->bg_anim);
    }

    if(config->anim_path) {
        instance->bg_anim = anim_image_alloc(&instance->base);
        anim_image_set_source(instance->bg_anim, config->anim_path);

    } else {
        instance->bg_anim = NULL;
    }
}

static void timer_indicator_set_progress_lottie(
    TimerIndicator* instance,
    const TimerIndicatorProgressConfig* config) {
    if(instance->progress_lottie) {
        lottie_animation_free(instance->progress_lottie);
    }

    if(config->lottie_path) {
        instance->progress_lottie = lottie_animation_alloc(&instance->base);
        lottie_animation_set_source(instance->progress_lottie, config->lottie_path);

        instance->progress_dir = config->direction;
        instance->progress_start_offset_px = config->start_offset_px;
        instance->progress_end_offset_px = config->end_offset_px;

    } else {
        instance->progress_lottie = NULL;
    }
}

static void
    timer_indicator_set_fg_image(TimerIndicator* instance, const TimerIndicatorFgConfig* config) {
    if(instance->fg_image) {
        image_free(instance->fg_image);
    }

    if(config->image_path) {
        instance->fg_image = image_alloc(&instance->base);
        image_set_source(instance->fg_image, config->image_path);

    } else {
        instance->fg_image = NULL;
    }
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

// void timer_indicator_set_anim_sources(
//     TimerIndicator* instance,
//     const TimerIndicatorAnimSources* sources) {
//     furi_check(instance);
//     furi_check(sources);
//
//     instance->sources = *sources;
// }

// void timer_indicator_set_state(TimerIndicator* instance, TimerIndicatorState state) {
//     furi_check(instance);
//     furi_check(state < TimerIndicatorStateMax);
//
//     AnimImage* anim_image = (AnimImage*)instance;
//
//     if(instance->state == TimerIndicatorStateWorkBig && state == TimerIndicatorStateWork) {
//         const TimerIndicatorTransition transition = TimerIndicatorTransitionOffToSimple;
//
//         anim_image_set_source(anim_image, instance->sources.transitions[transition]);
//         anim_image_set_loop(anim_image, false);
//
//         timer_indicator_run_transition(instance, transition);
//
//     } else {
//         anim_image_set_source(anim_image, instance->sources.states[state]);
//         anim_image_set_loop(anim_image, true);
//     }
//
//     instance->state = state;
// }`

void timer_indicator_set_preset(TimerIndicator* instance, const TimerIndicatorPreset* preset) {
    furi_check(instance);
    furi_check(preset);

    timer_indicator_set_bg_animation(instance, &preset->background_config);
    timer_indicator_set_progress_lottie(instance, &preset->progress_config);
    timer_indicator_set_fg_image(instance, &preset->foreground_config);
}

void timer_indicator_set_progress(TimerIndicator* instance, float progress) {
    furi_check(instance);

    if(instance->progress_lottie) {
        const TimerIndicatorProgressDirection progress_dir = instance->progress_dir;
        furi_assert(progress_dir < TimerIndicatorProgressDirectionMax);

        const float delta =
            progress * (instance->progress_end_offset_px - instance->progress_start_offset_px);
        const float offset = delta + instance->progress_start_offset_px;

        if(progress_dir == TimerIndicatorProgressDirectionHorizontal) {
            snprintf(
                instance->slot_store,
                SLOT_STR_LEN,
                SLOT_TEMPLATE,
                "hor_offset",
                (double)offset,
                0.);

        } else if(progress_dir == TimerIndicatorProgressDirectionVertical) {
            snprintf(
                instance->slot_store,
                SLOT_STR_LEN,
                SLOT_TEMPLATE,
                "ver_offset",
                0.,
                (double)offset);

        } else {
            furi_crash("Invalid TimerIndicatorProgressDirection value");
        }

        lottie_animation_override_slot(instance->progress_lottie, instance->slot_store);
    }
}

void timer_indicator_enable_animations(TimerIndicator* instance, bool enable) {
    furi_check(instance);
    UNUSED(enable);
}

// LVGL class descriptor

const lv_obj_class_t timer_indicator_lvgl_class = {
    .base_class = &widget_lvgl_class,
    .constructor_cb = timer_indicator_lvgl_constructor,
    .destructor_cb = timer_indicator_lvgl_destructor,
    .name = "widget-timer-indicator",
    .width_def = LV_SIZE_CONTENT,
    .height_def = LV_SIZE_CONTENT,
    .instance_size = sizeof(TimerIndicator),
};

// static const TimerTransitionPreset
//     timer_indicator_transition_presets[TimerIndicatorTransitionMax] = {
//         [TimerIndicatorTransitionOffToSimple] =
//             {
//                 .start_width = 70,
//                 .end_width = 40,
//                 .duration_ms = FRAMES_TO_MS(40),
//             },
// };
