#include "progress_bar.h"

#include <gui/widget_i.h>

#define MY_CLASS (&progress_bar_lvgl_class)

struct ProgressBar {
    Widget base;
    lv_obj_t* bar;
    lv_obj_t* background;
};

const lv_obj_class_t progress_bar_lvgl_class;

// LVGL-specific functions

static void progress_bar_lvgl_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    UNUSED(class_p);

    ProgressBar* instance = (ProgressBar*)obj;

    instance->background = lv_obj_create(obj);
    lv_obj_set_size(instance->background, 70, 5);
    lv_obj_set_style_bg_opa(instance->background, LV_OPA_20, 0);
    lv_obj_set_style_radius(instance->background, 3, 0);
    lv_obj_align(instance->background, LV_ALIGN_CENTER, 0, 0);

    instance->bar = lv_bar_create(obj);
    lv_obj_set_size(instance->bar, 70, 5);
    lv_obj_set_style_bg_opa(instance->bar, LV_OPA_COVER, LV_PART_INDICATOR);
    lv_obj_set_style_radius(instance->bar, 3, LV_PART_INDICATOR);
    lv_bar_set_value(instance->bar, 0, LV_ANIM_OFF);
    lv_obj_align(instance->bar, LV_ALIGN_CENTER, 0, 0);
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
    lv_bar_set_value(instance->bar, value, LV_ANIM_OFF);
}

void progress_bar_set_color(ProgressBar* instance, Color color) {
    furi_check(instance);
    lv_color_t lv_color = lv_color_make(color.r, color.g, color.b);
    lv_obj_set_style_bg_color(instance->bar, lv_color, LV_PART_INDICATOR);
}

void progress_bar_set_color_background(ProgressBar* instance, Color color) {
    furi_check(instance);
    lv_color_t lv_color = lv_color_make(color.r, color.g, color.b);
    lv_obj_set_style_bg_color(instance->background, lv_color, 0);
}

void progress_bar_set_size(ProgressBar* instance, uint16_t width, uint16_t height) {
    furi_check(instance);
    lv_obj_set_size(instance->background, width, height);
    lv_obj_set_size(instance->bar, width, height);
}

// LVGL class descriptor

const lv_obj_class_t progress_bar_lvgl_class = {
    .base_class = &widget_lvgl_class,
    .constructor_cb = progress_bar_lvgl_constructor,
    .name = "widget-bar",
    .width_def = LV_SIZE_CONTENT,
    .height_def = LV_SIZE_CONTENT,
    .instance_size = sizeof(ProgressBar),
};
