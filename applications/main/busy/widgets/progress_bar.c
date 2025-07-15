#include "progress_bar.h"

#include <gui/widget_i.h>
#include <gui/modules/anim_image.h>

#define TROUGH_WIDTH  (70)
#define TROUGH_HEIGHT (1)

#define MY_CLASS (&progress_bar_lvgl_class)

struct ProgressBar {
    Widget base;
    AnimImage* bar;
    int32_t prev_offset;
};

const lv_obj_class_t progress_bar_lvgl_class;

// LVGL-specific code

static void progress_bar_lvgl_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    UNUSED(class_p);

    lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_bg_color(obj, lv_color_black(), LV_PART_MAIN);

    ProgressBar* instance = (ProgressBar*)obj;
    instance->bar = anim_image_alloc((Widget*)obj);

    lv_obj_align_to((lv_obj_t*)instance->bar, obj, LV_ALIGN_OUT_LEFT_MID, 1, 0);
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

void progress_bar_set_preset(ProgressBar* instance, const ProgressBarPreset* preset) {
    furi_check(instance);

    anim_image_set_source(instance->bar, preset->file_path);
    anim_image_stop(instance->bar);

    lv_obj_set_style_bg_color(
        TO_LV_OBJ(instance), TO_LV_COLOR(preset->trough_color), LV_PART_MAIN);
}

void progress_bar_set_value(ProgressBar* instance, float value) {
    furi_check(instance);

    const int32_t width = lv_obj_get_width((lv_obj_t*)instance);
    const int32_t offset = width - roundf(width * value);

    if(offset != instance->prev_offset) {
        instance->prev_offset = offset;
        lv_obj_set_x((lv_obj_t*)instance->bar, -offset);
    }

    anim_image_set_range(instance->bar, 0, 59, false, false);
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
