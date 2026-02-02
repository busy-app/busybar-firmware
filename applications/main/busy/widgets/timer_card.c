#include "timer_card.h"

#include <gui/widget_i.h>
#include <gui/modules/front_display_mirror.h>

#include <busy_timer/time_macros.h>
#include "../storage_macros.h"

#define MY_CLASS (&timer_card_lvgl_class)

struct TimerCard {
    Widget base;
    lv_obj_t* left_image;
    lv_obj_t* right_image;
    lv_obj_t* top_static_text;
    lv_obj_t* bottom_timer_text;
    lv_obj_t* bottom_static_text;
    DisplayMirror* display_mirror;
};

const lv_obj_class_t timer_card_lvgl_class;

// LVGL-specific code

static void timer_card_lvgl_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    UNUSED(class_p);

    lv_obj_set_flex_flow(obj, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(obj, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t* top_layout = lv_obj_create(obj);
    lv_obj_set_size(top_layout, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(top_layout, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(
        top_layout, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_column(top_layout, 4, LV_PART_MAIN);
    lv_obj_set_style_margin_bottom(top_layout, 10, LV_PART_MAIN);

    TimerCard* instance = (TimerCard*)obj;
    instance->left_image = lv_image_create(top_layout);
    lv_image_set_src(instance->left_image, BUSY_IMG_PATH("active_indicator_left_28x7.bin"));

    instance->top_static_text = lv_label_create(top_layout);
    lv_obj_set_style_text_font(
        instance->top_static_text, lv_theme_get_font_small(obj), LV_PART_MAIN);
    lv_obj_set_style_text_color(instance->top_static_text, lv_color_black(), LV_PART_MAIN);
    lv_label_set_text(instance->top_static_text, "ACTIVE");

    instance->right_image = lv_image_create(top_layout);
    lv_image_set_src(instance->right_image, BUSY_IMG_PATH("active_indicator_right_28x7.bin"));

    // Mask object for image rounded corners
    instance->display_mirror = display_mirror_alloc((Widget*)instance);
    Widget* mirror_base = display_mirror_get_base(instance->display_mirror);
    lv_obj_t* mirror_obj = TO_LV_OBJ(mirror_base);
    lv_obj_set_size(mirror_obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_style_radius(mirror_obj, 6, LV_PART_MAIN);
    lv_obj_set_style_clip_corner(mirror_obj, true, LV_PART_MAIN);
    lv_obj_set_style_pad_hor(mirror_obj, 1, LV_PART_MAIN);
    lv_obj_set_style_pad_ver(mirror_obj, 2, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(mirror_obj, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_bg_color(mirror_obj, lv_color_black(), LV_PART_MAIN);

    lv_obj_t* bottom_layout = lv_obj_create(obj);
    lv_obj_set_size(bottom_layout, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(bottom_layout, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(
        bottom_layout, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_END, LV_FLEX_ALIGN_END);
    lv_obj_set_style_bg_opa(bottom_layout, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_pad_column(bottom_layout, 2, LV_PART_MAIN);
    lv_obj_set_style_margin_top(bottom_layout, 8, LV_PART_MAIN);
    lv_obj_set_style_margin_bottom(bottom_layout, -3, LV_PART_MAIN);

    instance->bottom_timer_text = lv_label_create(bottom_layout);
    lv_obj_set_style_text_color(instance->bottom_timer_text, lv_color_black(), LV_PART_MAIN);

    instance->bottom_static_text = lv_label_create(bottom_layout);
    lv_label_set_text(instance->bottom_static_text, "LEFT");
    lv_obj_set_style_text_color(instance->bottom_static_text, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_text_font(
        instance->bottom_static_text, lv_theme_get_font_small(obj), LV_PART_MAIN);
    lv_obj_set_style_translate_y(instance->bottom_static_text, -2, LV_PART_MAIN);
}

// Public API

TimerCard* timer_card_alloc(Widget* parent) {
    furi_check(parent);

    lv_obj_t* obj = lv_obj_class_create_obj(MY_CLASS, (lv_obj_t*)parent);
    lv_obj_class_init_obj(obj);

    TimerCard* instance = (TimerCard*)obj;
    return instance;
}

void timer_card_free(TimerCard* instance) {
    furi_check(instance);
    lv_obj_delete((lv_obj_t*)instance);
}

Widget* timer_card_get_base(TimerCard* instance) {
    furi_check(instance);
    return (Widget*)instance;
}

void timer_card_show_header(TimerCard* instance, bool show) {
    furi_check(instance);

    lv_opa_t opacity = show ? LV_OPA_COVER : LV_OPA_TRANSP;
    lv_obj_set_style_text_opa(instance->top_static_text, opacity, LV_PART_MAIN);
    lv_obj_set_style_image_opa(instance->left_image, opacity, LV_PART_MAIN);
    lv_obj_set_style_image_opa(instance->right_image, opacity, LV_PART_MAIN);
}

void timer_card_show_time(TimerCard* instance, bool show) {
    furi_check(instance);

    lv_opa_t opacity = show ? LV_OPA_COVER : LV_OPA_TRANSP;
    lv_obj_set_style_text_opa(instance->bottom_timer_text, opacity, LV_PART_MAIN);
    lv_obj_set_style_text_opa(instance->bottom_static_text, opacity, LV_PART_MAIN);
}

void timer_card_set_time(TimerCard* instance, uint32_t time_s) {
    furi_check(instance);

    const uint32_t h = S_TO_H(time_s);
    const uint32_t m = S_TO_M(time_s - H_TO_S(h));
    const uint32_t s = time_s - H_TO_S(h) - M_TO_S(m);

    if(h) {
        lv_label_set_text_fmt(instance->bottom_timer_text, "%lu:%02lu:%02lu", h, m, s);
    } else {
        lv_label_set_text_fmt(instance->bottom_timer_text, "%02lu:%02lu", m, s);
    }
}

// LVGL class descriptor

const lv_obj_class_t timer_card_lvgl_class = {
    .base_class = &widget_lvgl_class,
    .constructor_cb = timer_card_lvgl_constructor,
    .name = "widget-timer-card",
    .width_def = LV_PCT(100),
    .height_def = LV_SIZE_CONTENT,
    .instance_size = sizeof(TimerCard),
};
