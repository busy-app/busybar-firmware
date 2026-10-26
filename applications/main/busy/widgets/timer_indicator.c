#include "timer_indicator.h"

#include <gui/widget_i.h>
#include <gui/modules/image.h>
#include <gui/modules/anim_player.h>

#define MY_CLASS (&timer_indicator_lvgl_class)

struct TimerIndicator {
    Widget base;
    AnimPlayer* bg_anim;
    AnimPlayer* progress_anim;
    Image* progress_mask;
    Image* fg_image;
    const TimerIndicatorPreset* current_preset;
};

const lv_obj_class_t timer_indicator_lvgl_class;

// Function prototypes

static void timer_indicator_apply_preset(TimerIndicator* instance);

// Implementation

static void timer_indicator_bg_anim_frame_callback(
    AnimPlayer* player,
    const AnimFileFrameInfo* info,
    void* context) {
    furi_assert(player);
    furi_assert(info);
    furi_assert(context);

    if(info->flags & AnimFileFrameFlagFinished) {
        anim_player_set_frame_callback(player, NULL, NULL);

        TimerIndicator* instance = context;
        timer_indicator_apply_preset(instance);
    }
}

static void timer_indicator_reset(TimerIndicator* instance) {
    if(instance->bg_anim) {
        anim_player_free(instance->bg_anim);
        instance->bg_anim = NULL;
    }
    if(instance->progress_anim) {
        anim_player_free(instance->progress_anim);
        instance->progress_anim = NULL;
    }
    if(instance->progress_mask) {
        image_free(instance->progress_mask);
        instance->progress_mask = NULL;
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

    instance->bg_anim = anim_player_alloc(&instance->base);
    anim_player_set_source(instance->bg_anim, transition->anim_path);
    anim_player_set_section(instance->bg_anim, AnimFilePlayFlagNone, ANIM_FILE_DEFAULT_SECTION);
    anim_player_set_frame_callback(
        instance->bg_anim, timer_indicator_bg_anim_frame_callback, instance);
}

static void timer_indicator_apply_bg_animation(TimerIndicator* instance) {
    const TimerIndicatorBgConfig* config = &instance->current_preset->background_config;

    if(config->anim_path) {
        instance->bg_anim = anim_player_alloc(&instance->base);
        anim_player_set_source(instance->bg_anim, config->anim_path);
    }
}

static void timer_indicator_apply_progress_animation(TimerIndicator* instance) {
    const TimerIndicatorProgressConfig* config = &instance->current_preset->progress_config;

    if(config->anim_path) {
        instance->progress_anim = anim_player_alloc(&instance->base);
        anim_player_set_source_ex(
            instance->progress_anim,
            config->anim_path,
            AnimPlayerOptionIntermediateInternalBuffer);
    }
}

static void timer_indicator_apply_progress_mask(TimerIndicator* instance) {
    const TimerIndicatorProgressConfig* config = &instance->current_preset->progress_config;

    if(config->mask_path) {
        instance->progress_mask = image_alloc(&instance->base);
        image_set_source(instance->progress_mask, config->mask_path);
        widget_set_blend_mode(image_get_base(instance->progress_mask), WidgetBlendModeMultiply);
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
    timer_indicator_apply_progress_animation(instance);
    timer_indicator_apply_progress_mask(instance);
    timer_indicator_apply_fg_image(instance);

    timer_indicator_set_progress(instance, 0);
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

    if(instance->progress_anim) {
        const TimerIndicatorProgressConfig* config = &instance->current_preset->progress_config;

        const TimerIndicatorProgressDirection progress_dir = config->direction;
        furi_assert(progress_dir < TimerIndicatorProgressDirectionMax);

        const float delta = progress * (config->end_offset_px - config->start_offset_px);
        const float offset = delta + config->start_offset_px;

        if(progress_dir == TimerIndicatorProgressDirectionHorizontal) {
            anim_player_set_offset(instance->progress_anim, offset, 0);
        } else if(progress_dir == TimerIndicatorProgressDirectionVertical) {
            anim_player_set_offset(instance->progress_anim, 0, offset);
        } else {
            furi_crash("Invalid TimerIndicatorProgressDirection value");
        }
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
