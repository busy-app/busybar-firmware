#include "timer_label.h"

#include <gui/widget_i.h>

#include "../time_macros.h"

#define MY_CLASS (&timer_label_lvgl_class)

#define FONT_REGULAR   (&lv_font_ark_numerals_regular_10)
#define FONT_CONDENSED (&lv_font_ark_numerals_condensed_10)
#define FONT_SMALLNUM  (&lv_font_ark_numerals_small_10)

#define BLINK_COUNT     (3)
#define BLINK_DELAY_MS  (500)
#define BLINK_PERIOD_MS (333)

struct TimerLabel {
    Widget base;
    lv_obj_t* top_layout;
    lv_obj_t* main_label;
    lv_obj_t* seconds_label;
    lv_obj_t* bottom_label;

    lv_color_t countdown_color;
    lv_color_t countdown_blink_color;
};

const lv_obj_class_t timer_label_lvgl_class;

// LVGL-specific code

static void timer_label_lvgl_anim_color_to_red_callback(void* context, int32_t value) {
    furi_assert(context);

    TimerLabel* instance = context;

    lv_color_t start_color = lv_color_white();
    lv_color_t end_color = instance->countdown_color;

    lv_color_t color = lv_color_mix(end_color, start_color, value);
    lv_obj_set_style_text_color(instance->main_label, color, LV_PART_MAIN);
    lv_obj_set_style_text_color(instance->seconds_label, color, LV_PART_MAIN);
    lv_obj_set_style_text_color(instance->bottom_label, color, LV_PART_MAIN);
}

static void timer_label_lvgl_anim_red_blink_callback(void* context, int32_t value) {
    furi_assert(context);

    TimerLabel* instance = context;

    lv_color_t start_color = instance->countdown_color;
    lv_color_t end_color = instance->countdown_blink_color;

    lv_color_t color = lv_color_mix(end_color, start_color, value);
    lv_obj_set_style_text_color(instance->main_label, color, LV_PART_MAIN);
    lv_obj_set_style_text_color(instance->seconds_label, color, LV_PART_MAIN);
    lv_obj_set_style_text_color(instance->bottom_label, color, LV_PART_MAIN);
}

static void timer_label_lvgl_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    UNUSED(class_p);

    lv_obj_set_flex_flow(obj, LV_FLEX_FLOW_COLUMN);

    TimerLabel* instance = (TimerLabel*)obj;

    instance->countdown_color = lv_color_white();
    instance->countdown_blink_color = lv_color_white();

    instance->top_layout = lv_obj_create(obj);
    lv_obj_set_flex_flow(instance->top_layout, LV_FLEX_FLOW_ROW);
    lv_obj_set_size(instance->top_layout, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_style_pad_column(instance->top_layout, 1, LV_PART_MAIN);

    instance->main_label = lv_label_create(instance->top_layout);
    lv_obj_set_style_text_color(instance->main_label, lv_color_white(), LV_PART_MAIN);

    instance->seconds_label = lv_label_create(instance->top_layout);
    lv_obj_set_style_text_font(instance->seconds_label, FONT_SMALLNUM, LV_PART_MAIN);
    lv_obj_set_style_text_color(instance->seconds_label, lv_color_white(), LV_PART_MAIN);

    instance->bottom_label = lv_label_create(obj);
    lv_label_set_text(instance->bottom_label, "LEFT");
}

static void timer_label_to_red(TimerLabel* instance) {
    lv_anim_t anim;
    lv_anim_init(&anim);

    lv_anim_set_values(&anim, 0, 0xFF);
    lv_anim_set_duration(&anim, 1000);

    lv_anim_set_exec_cb(&anim, timer_label_lvgl_anim_color_to_red_callback);
    lv_anim_set_var(&anim, instance);

    lv_anim_start(&anim);
}

static void timer_label_red_blink(TimerLabel* instance) {
    lv_anim_t anim;
    lv_anim_init(&anim);

    lv_anim_set_values(&anim, 0, 0xFF);
    lv_anim_set_duration(&anim, 250);
    lv_anim_set_reverse_duration(&anim, 250);

    lv_anim_set_exec_cb(&anim, timer_label_lvgl_anim_red_blink_callback);
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

    if(time_s == 4) {
        timer_label_to_red(instance);
    }

    if(time_s <= 3) {
        timer_label_red_blink(instance);
    }
}

void timer_label_set_countdown_colors(TimerLabel* instance, Color main, Color blink) {
    furi_check(instance);

    instance->countdown_color = TO_LV_COLOR(main);
    instance->countdown_blink_color = TO_LV_COLOR(blink);
}

// LVGL class descriptor

const lv_obj_class_t timer_label_lvgl_class = {
    .base_class = &widget_lvgl_class,
    .constructor_cb = timer_label_lvgl_constructor,
    .name = "widget-timer-label",
    .width_def = LV_SIZE_CONTENT,
    .height_def = LV_SIZE_CONTENT,
    .instance_size = sizeof(TimerLabel),
};
