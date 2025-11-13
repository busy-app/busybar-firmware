#include "front_display_mirror.h"

#include <gui/gui_i.h>

#define MY_CLASS (&display_mirror_lvgl_class)

struct DisplayMirror {
    Widget base;
    lv_display_t* display;
    lv_obj_t* mirror_image;
    lv_image_dsc_t mirror_image_dsc;
    uint32_t refresh_count;
};

const lv_obj_class_t display_mirror_lvgl_class;

static void display_mirror_refresh_callback(lv_event_t* event) {
    DisplayMirror* instance = lv_event_get_user_data(event);
    // Limit mirror refresh rate to half of the original
    if(instance->refresh_count++ % 2 == 0) {
        lv_obj_invalidate(instance->mirror_image);
    }
}

// LVGL-specific code

static void display_mirror_lvgl_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    UNUSED(class_p);

    DisplayMirror* instance = (DisplayMirror*)obj;

    instance->mirror_image = lv_image_create(obj);

    Gui* gui = furi_record_open(RECORD_GUI);
    GuiDisplay* front = &gui->displays[GuiDisplayIdFront];

    lv_image_dsc_t* image_dsc = &instance->mirror_image_dsc;
    image_dsc->header.magic = LV_IMAGE_HEADER_MAGIC;
    image_dsc->header.cf = FRONT_COLOR_FORMAT;
    image_dsc->header.w = FRONT_W;
    image_dsc->header.h = FRONT_H;
    image_dsc->header.stride = LV_DRAW_BUF_STRIDE(FRONT_W, FRONT_COLOR_FORMAT);
    image_dsc->data_size = FRONT_DRAW_BUFFER_SIZE;
    image_dsc->data = front->draw_buffer;

    lv_obj_set_size(instance->mirror_image, FRONT_W * 2, FRONT_H * 2);
    lv_obj_set_align(instance->mirror_image, LV_ALIGN_CENTER);
    lv_image_set_inner_align(instance->mirror_image, LV_IMAGE_ALIGN_STRETCH);
    lv_image_set_antialias(instance->mirror_image, false);
    lv_image_set_src(instance->mirror_image, image_dsc);

    instance->display = front->lv_display;
    lv_display_add_event_cb(
        instance->display, display_mirror_refresh_callback, LV_EVENT_REFR_READY, instance);
}

static void display_mirror_lvgl_destructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    UNUSED(class_p);

    DisplayMirror* instance = (DisplayMirror*)obj;
    lv_display_remove_event_cb_with_user_data(
        instance->display, display_mirror_refresh_callback, instance);
    furi_record_close(RECORD_GUI);
}

// Public API

DisplayMirror* display_mirror_alloc(Widget* parent) {
    furi_check(parent);

    lv_obj_t* obj = lv_obj_class_create_obj(MY_CLASS, (lv_obj_t*)parent);
    lv_obj_class_init_obj(obj);

    DisplayMirror* instance = (DisplayMirror*)obj;
    return instance;
}

void display_mirror_free(DisplayMirror* instance) {
    furi_check(instance);
    lv_obj_delete((lv_obj_t*)instance);
}

Widget* display_mirror_get_base(DisplayMirror* instance) {
    furi_check(instance);
    return (Widget*)instance;
}

// LVGL class descriptor

const lv_obj_class_t display_mirror_lvgl_class = {
    .base_class = &widget_lvgl_class,
    .constructor_cb = display_mirror_lvgl_constructor,
    .destructor_cb = display_mirror_lvgl_destructor,
    .name = "widget-display-mirror",
    .width_def = LV_PCT(100),
    .height_def = LV_PCT(100),
    .instance_size = sizeof(DisplayMirror),
};
