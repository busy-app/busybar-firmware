#include "progress_bar.h"

#include <gui/widget_i.h>

#define TROUGH_WIDTH  (70)
#define TROUGH_HEIGHT (1)

#define BAR_COLOR_MAIN    lv_color_hex(0xFF0000)
#define TROUGH_COLOR_MAIN lv_color_hex(0x4A0000)

#define BAR_COLOR_ALT    lv_color_hex(0x13F562)
#define TROUGH_COLOR_ALT lv_color_hex(0x011809)

#define MY_CLASS (&progress_bar_lvgl_class)

struct ProgressBar {
    Widget base;
    lv_obj_t* bar;
};

const lv_obj_class_t progress_bar_lvgl_class;

// LVGL-specific code

static void progress_bar_lvgl_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    UNUSED(class_p);

    lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_bg_color(obj, TROUGH_COLOR_MAIN, LV_PART_MAIN);

    ProgressBar* instance = (ProgressBar*)obj;
    instance->bar = lv_obj_create(obj);

    lv_obj_set_size(instance->bar, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_opa(instance->bar, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_bg_color(instance->bar, BAR_COLOR_MAIN, LV_PART_MAIN);

    lv_obj_align_to(instance->bar, obj, LV_ALIGN_OUT_LEFT_MID, 0, 0);
}

// Public API

ProgressBar* progress_bar_alloc(Widget* parent) {
    furi_check(parent);

    lv_obj_t* obj = lv_obj_class_create_obj(MY_CLASS, (lv_obj_t*)parent);
    lv_obj_class_init_obj(obj);

    ProgressBar* instance = (ProgressBar*)obj;
    return instance;
}

void progress_bar_free(ProgressBar* instance) {
    furi_check(instance);
    lv_obj_delete((lv_obj_t*)instance);
}

Widget* progress_bar_get_base(ProgressBar* instance) {
    furi_check(instance);
    return (Widget*)instance;
}

void progress_bar_set_value(ProgressBar* instance, float value) {
    furi_check(instance);

    const int32_t width = lv_obj_get_width((lv_obj_t*)instance);
    const int32_t bar_offset = roundf(width * value);

    lv_obj_set_x(instance->bar, -(width - bar_offset));
}

void progress_bar_set_alt_color(ProgressBar* instance, bool set) {
    furi_check(instance);

    if(set) {
        lv_obj_set_style_bg_color((lv_obj_t*)instance, TROUGH_COLOR_ALT, LV_PART_MAIN);
        lv_obj_set_style_bg_color(instance->bar, BAR_COLOR_ALT, LV_PART_MAIN);
    } else {
        lv_obj_set_style_bg_color((lv_obj_t*)instance, TROUGH_COLOR_MAIN, LV_PART_MAIN);
        lv_obj_set_style_bg_color(instance->bar, BAR_COLOR_MAIN, LV_PART_MAIN);
    }
}

// LVGL class descriptor

const lv_obj_class_t progress_bar_lvgl_class = {
    .base_class = &widget_lvgl_class,
    .constructor_cb = progress_bar_lvgl_constructor,
    .name = "widget-progress-bar",
    .width_def = TROUGH_WIDTH,
    .height_def = TROUGH_HEIGHT,
    .instance_size = sizeof(ProgressBar),
};
