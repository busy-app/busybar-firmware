#include "timer_indicator.h"

#include <gui/widget_i.h>
#include <gui/modules/image.h>
#include <gui/modules/anim_play.h>
#include <gui/modules/lottie_animation.h>

#define MY_CLASS (&timer_indicator_lvgl_class)

#define SLOT_TEMPLATE \
    "{"               \
    " \"%s\": {"      \
    "  \"p\": {"      \
    "   \"k\": ["     \
    "     %.2f,"      \
    "     %.2f"       \
    "   ]"            \
    "}}}"

#define SLOT_STR_LEN (sizeof(SLOT_TEMPLATE) + 20)

struct TimerIndicator {
    Widget base;
    AnimPlay* bg_anim;
    LottieAnimation* progress_lottie;
    Image* fg_image;
    char slot_store[SLOT_STR_LEN];
    const TimerIndicatorPreset* current_preset;
};

const lv_obj_class_t timer_indicator_lvgl_class;

// Function prototypes

static void timer_indicator_apply_preset(TimerIndicator* instance);

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

    timer_indicator_apply_preset(instance);

    widget_set_width(&instance->base, LV_SIZE_CONTENT);
}

// Implementation

static void timer_indicator_reset(TimerIndicator* instance) {
    if(instance->bg_anim) {
        anim_play_free(instance->bg_anim);
        instance->bg_anim = NULL;
    }
    if(instance->progress_lottie) {
        lottie_animation_free(instance->progress_lottie);
        instance->progress_lottie = NULL;
    }
    if(instance->fg_image) {
        image_free(instance->fg_image);
        instance->fg_image = NULL;
    }
}

static void timer_indicator_start_transition(
    TimerIndicator* instance,
    const TimerIndicatorTransition* transition) {
    timer_indicator_reset(instance);

    instance->bg_anim = anim_play_alloc(&instance->base);
    if(anim_play_set_source(instance->bg_anim, transition->anim_path)) {
        anim_play_loop_whole(instance->bg_anim);
    }

    lv_anim_t anim;
    lv_anim_init(&anim);

    lv_anim_set_values(&anim, transition->start_width_px, transition->end_width_px);
    lv_anim_set_duration(&anim, transition->duration_ms);

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

static void timer_indicator_apply_bg_animation(TimerIndicator* instance) {
    const TimerIndicatorBgConfig* config = &instance->current_preset->background_config;

    if(config->anim_path) {
        instance->bg_anim = anim_play_alloc(&instance->base);
        if(anim_play_set_source(instance->bg_anim, config->anim_path)) {
            anim_play_loop_whole(instance->bg_anim);
        }
    }
}

static void timer_indicator_apply_progress_lottie(TimerIndicator* instance) {
    const TimerIndicatorProgressConfig* config = &instance->current_preset->progress_config;

    if(config->lottie_path) {
        instance->progress_lottie = lottie_animation_alloc(&instance->base);
        lottie_animation_set_source(instance->progress_lottie, config->lottie_path);
    }
}

static void timer_indicator_apply_fg_image(TimerIndicator* instance) {
    const TimerIndicatorFgConfig* config = &instance->current_preset->foreground_config;

    if(config->image_path) {
        instance->fg_image = image_alloc(&instance->base);
        image_set_source(instance->fg_image, config->image_path);
    }
}

static void timer_indicator_apply_preset(TimerIndicator* instance) {
    timer_indicator_reset(instance);
    timer_indicator_apply_bg_animation(instance);
    timer_indicator_apply_progress_lottie(instance);
    timer_indicator_apply_fg_image(instance);
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

void timer_indicator_set_preset(
    TimerIndicator* instance,
    const TimerIndicatorPreset* preset,
    const TimerIndicatorTransition* transition) {
    furi_check(instance);
    furi_check(preset);

    instance->current_preset = preset;

    if(transition) {
        timer_indicator_start_transition(instance, transition);
    } else {
        timer_indicator_apply_preset(instance);
    }
}

void timer_indicator_set_progress(TimerIndicator* instance, float progress) {
    furi_check(instance);

    if(instance->progress_lottie) {
        const TimerIndicatorProgressConfig* config = &instance->current_preset->progress_config;

        const TimerIndicatorProgressDirection progress_dir = config->direction;
        furi_assert(progress_dir < TimerIndicatorProgressDirectionMax);

        const float delta = progress * (config->end_offset_px - config->start_offset_px);
        const float offset = delta + config->start_offset_px;

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
    .name = "widget-timer-indicator",
    .width_def = LV_SIZE_CONTENT,
    .height_def = LV_SIZE_CONTENT,
    .instance_size = sizeof(TimerIndicator),
};
