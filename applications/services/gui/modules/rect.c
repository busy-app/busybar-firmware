#include "rect.h"

#include <gui/widget_i.h>

#define MY_CLASS (&rect_lvgl_class)

struct Rect {
    Widget base;
    lv_obj_t* rect;
};

const lv_obj_class_t rect_lvgl_class;

// LVGL-specific functions

static void rect_lvgl_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    UNUSED(class_p);

    Rect* instance = (Rect*)obj;
    instance->rect = lv_obj_create(obj);
}

// Public API

Rect* rect_alloc(Widget* parent) {
    furi_check(parent);

    lv_obj_t* obj = lv_obj_class_create_obj(MY_CLASS, (lv_obj_t*)parent);
    lv_obj_class_init_obj(obj);

    Rect* instance = (Rect*)obj;
    return instance;
}

void rect_free(Rect* instance) {
    furi_check(instance);
    lv_obj_delete((lv_obj_t*)instance);
}

Widget* rect_get_base(Rect* instance) {
    furi_check(instance);
    return (Widget*)instance;
}

// LVGL class descriptor

const lv_obj_class_t rect_lvgl_class = {
    .base_class = &widget_lvgl_class,
    .constructor_cb = rect_lvgl_constructor,
    .name = "widget-rect",
    .width_def = LV_SIZE_CONTENT,
    .height_def = LV_SIZE_CONTENT,
    .instance_size = sizeof(Rect),
};
