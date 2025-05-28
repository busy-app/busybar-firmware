#include "image.h"

#include <gui/widget_i.h>

#include <assets_images.h>

#define MY_CLASS (&image_lvgl_class)

struct Image {
    Widget base;
    lv_obj_t* image;
};

const lv_obj_class_t image_lvgl_class;

// LVGL-specific functions

static void image_lvgl_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    UNUSED(class_p);

    Image* instance = (Image*)obj;
    instance->image = lv_image_create(obj);
}

// Public API

Image* image_alloc(Widget* parent) {
    furi_check(parent);

    lv_obj_t* obj = lv_obj_class_create_obj(MY_CLASS, (lv_obj_t*)parent);
    lv_obj_class_init_obj(obj);

    Image* instance = (Image*)obj;
    return instance;
}

void image_free(Image* instance) {
    furi_check(instance);
    lv_obj_delete((lv_obj_t*)instance);
}

Widget* image_get_base(Image* instance) {
    furi_check(instance);
    return (Widget*)instance;
}

bool image_set_source(Image* instance, const char* file_path) {
    furi_check(instance);
    furi_check(file_path);

    lv_image_set_src(instance->image, NULL);
    lv_image_set_src(instance->image, file_path);

    const void* loaded_src = lv_image_get_src(instance->image);

    if(!loaded_src) {
        lv_image_set_src(instance->image, &I_load_error_9x9);
    }

    return loaded_src != NULL;
}

// LVGL class descriptor

const lv_obj_class_t image_lvgl_class = {
    .base_class = &widget_lvgl_class,
    .constructor_cb = image_lvgl_constructor,
    .name = "widget-image",
    .width_def = LV_SIZE_CONTENT,
    .height_def = LV_SIZE_CONTENT,
    .instance_size = sizeof(Image),
};
