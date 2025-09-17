#include "progress_bar.h"

#include <gui/widget_i.h>

#define MY_CLASS      (&progress_bar_lvgl_class)
#define MY_FILL_CLASS (&progress_bar_fill_lvgl_class)

struct ProgressBar {
    Widget base;
    lv_obj_t* bar;
};

const lv_obj_class_t progress_bar_lvgl_class;
const lv_obj_class_t progress_bar_fill_lvgl_class;

// LVGL-specific functions

static void progress_bar_lvgl_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    UNUSED(class_p);

    ProgressBar* instance = (ProgressBar*)obj;

    instance->bar = lv_obj_class_create_obj(MY_FILL_CLASS, obj);
    lv_obj_class_init_obj(instance->bar);
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

void progress_bar_set_value(ProgressBar* instance, int32_t value) {
    furi_check(instance);
    lv_obj_set_size(instance->bar, LV_PCT(value), LV_PCT(100));
}

void progress_bar_set_color(ProgressBar* instance, Color color) {
    furi_check(instance);
    lv_obj_set_style_bg_color(instance->bar, TO_LV_COLOR(color), LV_PART_MAIN);
}

void progress_bar_set_color_background(ProgressBar* instance, Color color) {
    furi_check(instance);
    lv_obj_set_style_bg_color(TO_LV_OBJ(instance), TO_LV_COLOR(color), LV_PART_MAIN);
}

void progress_bar_set_size(ProgressBar* instance, uint16_t width, uint16_t height) {
    furi_check(instance);
    lv_obj_set_size(TO_LV_OBJ(instance), width, height);
}

// LVGL class descriptor

const lv_obj_class_t progress_bar_lvgl_class = {
    .base_class = &widget_lvgl_class,
    .constructor_cb = progress_bar_lvgl_constructor,
    .name = "widget-progress-bar",
    .width_def = 70,
    .height_def = 5,
    .instance_size = sizeof(ProgressBar),
};

const lv_obj_class_t progress_bar_fill_lvgl_class = {
    .base_class = &lv_obj_class,
    .name = "widget-progress-bar-fill",
    .width_def = LV_PCT(0),
    .height_def = LV_PCT(100),
};
