#include "timer_label.h"

#include <gui/widget_i.h>

#include <busy_timer/time_macros.h>

#define MY_CLASS (&timer_label_lvgl_class)

#define FONT_REGULAR   (&lv_font_ark_numerals_regular_10)
#define FONT_CONDENSED (&lv_font_ark_numerals_condensed_10)
#define FONT_SMALLNUM  (&lv_font_ark_numerals_small_10)

#define BLINK_START_S         (3)
#define BLINK_INTERVAL_MS     (333)
#define BLINK_INTERVAL_REV_MS (1000 - BLINK_INTERVAL_MS)

#define COUNTDOWN_START_S       (BLINK_START_S + 1)
#define COUNTDOWN_TRANSITION_MS (1000)

#define MAIN_WIDTH_PX    (40)
#define BG_GRAD_WIDTH_PX (10)

#define BG_GRAD_STOP_POS (255 * BG_GRAD_WIDTH_PX / MAIN_WIDTH_PX)

struct TimerLabel {
    Widget base;
    lv_obj_t* bg_gradient;
    lv_obj_t* main_layout;
    lv_obj_t* top_layout;
    lv_obj_t* main_label;
    lv_obj_t* seconds_label;
    lv_obj_t* bottom_label;

    lv_color_t countdown_base_color;
    lv_color_t countdown_blink_color;
};

const lv_obj_class_t timer_label_lvgl_class;

// LVGL-specific code

static void timer_label_lvgl_anim_color_to_countdown_callback(void* context, int32_t value) {
    furi_assert(context);

    TimerLabel* instance = context;

    const lv_color_t start_color = lv_color_white();
    const lv_color_t end_color = instance->countdown_base_color;

    const lv_color_t color = lv_color_mix(end_color, start_color, value);
    lv_obj_set_style_text_color(instance->main_label, color, LV_PART_MAIN);
    lv_obj_set_style_text_color(instance->seconds_label, color, LV_PART_MAIN);
    lv_obj_set_style_text_color(instance->bottom_label, color, LV_PART_MAIN);
}

static void timer_label_lvgl_anim_countdown_blink_callback(void* context, int32_t value) {
    furi_assert(context);

    TimerLabel* instance = context;

    const lv_color_t start_color = instance->countdown_base_color;
    const lv_color_t end_color = instance->countdown_blink_color;

    const lv_color_t color = lv_color_mix(end_color, start_color, value);
    lv_obj_set_style_text_color(instance->main_label, color, LV_PART_MAIN);
    lv_obj_set_style_text_color(instance->seconds_label, color, LV_PART_MAIN);
    lv_obj_set_style_text_color(instance->bottom_label, color, LV_PART_MAIN);
}

static void timer_label_lvgl_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    UNUSED(class_p);

    TimerLabel* instance = (TimerLabel*)obj;

    instance->bg_gradient = lv_obj_create(obj);
    lv_obj_set_size(instance->bg_gradient, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(instance->bg_gradient, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(instance->bg_gradient, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_bg_grad_dir(instance->bg_gradient, LV_GRAD_DIR_HOR, LV_PART_MAIN);
    lv_obj_set_style_bg_grad_opa(instance->bg_gradient, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_bg_grad_color(instance->bg_gradient, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_bg_grad_stop(instance->bg_gradient, BG_GRAD_STOP_POS, LV_PART_MAIN);
    lv_obj_set_style_blend_mode(instance->bg_gradient, LV_BLEND_MODE_MULTIPLY, LV_PART_MAIN);

    instance->main_layout = lv_obj_create(obj);
    lv_obj_align(instance->main_layout, LV_ALIGN_LEFT_MID, BG_GRAD_WIDTH_PX, 0);
    lv_obj_set_size(instance->main_layout, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(instance->main_layout, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(instance->main_layout, 1, LV_PART_MAIN);

    instance->countdown_base_color = lv_color_white();
    instance->countdown_blink_color = lv_color_white();

    instance->top_layout = lv_obj_create(instance->main_layout);
    lv_obj_set_flex_flow(instance->top_layout, LV_FLEX_FLOW_ROW);
    lv_obj_set_size(instance->top_layout, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_style_pad_column(instance->top_layout, 1, LV_PART_MAIN);

    instance->main_label = lv_label_create(instance->top_layout);
    lv_obj_set_style_text_color(instance->main_label, lv_color_white(), LV_PART_MAIN);

    instance->seconds_label = lv_label_create(instance->top_layout);
    lv_obj_set_style_text_font(instance->seconds_label, FONT_SMALLNUM, LV_PART_MAIN);
    lv_obj_set_style_text_color(instance->seconds_label, lv_color_white(), LV_PART_MAIN);

    instance->bottom_label = lv_label_create(instance->main_layout);
    lv_label_set_text(instance->bottom_label, "LEFT");
}

static void timer_label_to_countdown(TimerLabel* instance) {
    lv_anim_t anim;
    lv_anim_init(&anim);

    lv_anim_set_values(&anim, LV_OPA_TRANSP, LV_OPA_COVER);
    lv_anim_set_duration(&anim, COUNTDOWN_TRANSITION_MS);

    lv_anim_set_exec_cb(&anim, timer_label_lvgl_anim_color_to_countdown_callback);
    lv_anim_set_var(&anim, instance);

    lv_anim_start(&anim);
}

static void timer_label_countdown_blink(TimerLabel* instance) {
    lv_anim_t anim;
    lv_anim_init(&anim);

    lv_anim_set_values(&anim, LV_OPA_TRANSP, LV_OPA_COVER);
    lv_anim_set_duration(&anim, BLINK_INTERVAL_MS);
    lv_anim_set_reverse_duration(&anim, BLINK_INTERVAL_REV_MS);

    lv_anim_set_exec_cb(&anim, timer_label_lvgl_anim_countdown_blink_callback);
    lv_anim_set_var(&anim, instance);

    lv_anim_start(&anim);
}

// Public API

TimerLabel* timer_label_alloc(Widget* parent) {
    furi_check(parent);

    lv_obj_t* obj = lv_obj_class_create_obj(MY_CLASS, (lv_obj_t*)parent);
    lv_obj_class_init_obj(obj);

    TimerLabel* instance = (TimerLabel*)obj;
    return instance;
}

void timer_label_free(TimerLabel* instance) {
    furi_check(instance);
    lv_obj_delete((lv_obj_t*)instance);
}

Widget* timer_label_get_base(TimerLabel* instance) {
    furi_check(instance);
    return (Widget*)instance;
}

void timer_label_set_time(TimerLabel* instance, uint32_t time_s) {
    furi_check(instance);

    const uint32_t h = S_TO_H(time_s);
    const uint32_t m = S_TO_M(time_s - H_TO_S(h));
    const uint32_t s = time_s - H_TO_S(h) - M_TO_S(m);

    if(h) {
        if(h >= 10) {
            lv_obj_set_style_text_font(instance->main_label, FONT_CONDENSED, LV_PART_MAIN);
        } else {
            lv_obj_set_style_text_font(instance->main_label, FONT_REGULAR, LV_PART_MAIN);
        }

        lv_label_set_text_fmt(instance->main_label, "%lu:%02lu", h, m);
        lv_label_set_text_fmt(instance->seconds_label, "%02lu", s);

        lv_obj_remove_flag(instance->seconds_label, LV_OBJ_FLAG_HIDDEN);

    } else {
        lv_label_set_text_fmt(instance->main_label, "%02lu:%02lu", m, s);
        lv_obj_set_style_text_font(instance->main_label, FONT_REGULAR, LV_PART_MAIN);

        lv_obj_add_flag(instance->seconds_label, LV_OBJ_FLAG_HIDDEN);
    }

    if(time_s == COUNTDOWN_START_S) {
        timer_label_to_countdown(instance);
    } else if(time_s <= BLINK_START_S) {
        timer_label_countdown_blink(instance);
    } else {
        lv_obj_set_style_text_color(instance->main_label, lv_color_white(), LV_PART_MAIN);
        lv_obj_set_style_text_color(instance->seconds_label, lv_color_white(), LV_PART_MAIN);
        lv_obj_set_style_text_color(instance->bottom_label, lv_color_white(), LV_PART_MAIN);
    }
}

void timer_label_set_preset(TimerLabel* instance, const TimerLabelPreset* preset) {
    furi_check(instance);
    furi_check(preset);

    instance->countdown_base_color = TO_LV_COLOR(preset->countdown_colors.base);
    instance->countdown_blink_color = TO_LV_COLOR(preset->countdown_colors.blink);
}

// LVGL class descriptor

const lv_obj_class_t timer_label_lvgl_class = {
    .base_class = &widget_lvgl_class,
    .constructor_cb = timer_label_lvgl_constructor,
    .name = "widget-timer-label",
    .width_def = MAIN_WIDTH_PX,
    .height_def = LV_PCT(100),
    .instance_size = sizeof(TimerLabel),
};
