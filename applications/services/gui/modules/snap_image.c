#include "snap_image.h"

#include <gui/widget_i.h>

#define MY_CLASS (&snap_image_lvgl_class)

struct SnapImage {
    Widget base;
    lv_obj_t* canvas;
    uint8_t* canvas_buf;
};

const lv_obj_class_t snap_image_lvgl_class;

// LVGL-specific functions

static void snap_image_lvgl_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    UNUSED(class_p);

    SnapImage* instance = (SnapImage*)obj;
    instance->canvas = lv_canvas_create(obj);

    lv_display_t* display = lv_obj_get_display(obj);
    instance->canvas_buf = malloc(lv_display_get_draw_buf_size(display));
    lv_canvas_set_buffer(
        instance->canvas,
        instance->canvas_buf,
        lv_display_get_horizontal_resolution(display),
        lv_display_get_vertical_resolution(display),
        lv_display_get_color_format(display));
}

static void snap_image_lvgl_destructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    UNUSED(class_p);

    SnapImage* instance = (SnapImage*)obj;
    free(instance->canvas_buf);
}

// Public API

SnapImage* snap_image_alloc(Widget* parent) {
    furi_check(parent);

    lv_obj_t* obj = lv_obj_class_create_obj(MY_CLASS, (lv_obj_t*)parent);
    lv_obj_class_init_obj(obj);

    SnapImage* instance = (SnapImage*)obj;
    return instance;
}

void snap_image_free(SnapImage* instance) {
    furi_check(instance);
    lv_obj_delete((lv_obj_t*)instance);
}

Widget* snap_image_get_base(SnapImage* instance) {
    furi_check(instance);
    return (Widget*)instance;
}

void snap_image_capture_display(SnapImage* instance) {
    furi_check(instance);

    lv_display_t* display = lv_obj_get_display((lv_obj_t*)instance);
    const lv_draw_buf_t* display_buf = lv_display_get_buf_active(display);
    memcpy(instance->canvas_buf, display_buf->data, display_buf->data_size);
}

// LVGL class descriptor

const lv_obj_class_t snap_image_lvgl_class = {
    .base_class = &widget_lvgl_class,
    .constructor_cb = snap_image_lvgl_constructor,
    .destructor_cb = snap_image_lvgl_destructor,
    .name = "widget-snap-image",
    .width_def = LV_SIZE_CONTENT,
    .height_def = LV_SIZE_CONTENT,
    .instance_size = sizeof(SnapImage),
};
