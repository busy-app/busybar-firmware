#include "image.h"

#include <gui/widget_i.h>

#include <assets_images.h>

#define MY_CLASS (&image_lvgl_class)

struct Image {
    Widget base;
    lv_obj_t* image;
    lv_image_dsc_t* raw_source_dsc;
    void* raw_source_buffer;
};

const lv_obj_class_t image_lvgl_class;

static const lv_color_format_t image_color_format_map[] = {
    [ImageColorFormatL8] = LV_COLOR_FORMAT_L8,
    [ImageColorFormatLA88] = LV_COLOR_FORMAT_AL88,
    [ImageColorFormatBGR888] = LV_COLOR_FORMAT_RGB888,
    [ImageColorFormatBGRA8888] = LV_COLOR_FORMAT_ARGB8888,
};

static_assert(COUNT_OF(image_color_format_map) == ImageColorFormatsCount);

static void image_clear_raw_source(Image* instance) {
    if(instance->raw_source_dsc) {
        lv_free(instance->raw_source_dsc);
        instance->raw_source_dsc = NULL;
    }

    if(instance->raw_source_buffer) {
        lv_free(instance->raw_source_buffer);
        instance->raw_source_buffer = NULL;
    }
}

// LVGL-specific functions

static void image_lvgl_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    UNUSED(class_p);

    Image* instance = (Image*)obj;
    instance->image = lv_image_create(obj);
    instance->raw_source_dsc = NULL;
    instance->raw_source_buffer = NULL;
}

static void image_lvgl_destructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    UNUSED(class_p);

    Image* instance = (Image*)obj;
    image_clear_raw_source(instance);
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
    image_clear_raw_source(instance);

    lv_image_set_src(instance->image, file_path);

    const void* loaded_src = lv_image_get_src(instance->image);

    if(!loaded_src) {
        lv_image_set_src(instance->image, &I_load_error_9x9);
    }

    return loaded_src != NULL;
}

bool image_set_source_no_cache(Image* instance, const char* file_path) {
    furi_check(instance);
    furi_check(file_path);

    lv_image_cache_drop(file_path);

    return image_set_source(instance, file_path);
}

void image_set_source_raw(
    Image* instance,
    ImageColorFormat format,
    size_t width,
    size_t height,
    const void* data,
    size_t data_size) {
    furi_check(instance);
    furi_check(format < ImageColorFormatsCount);
    furi_check(data);

    lv_color_format_t color_format = image_color_format_map[format];
    uint8_t pixel_size = LV_COLOR_FORMAT_GET_SIZE(color_format);

    furi_check(data_size == width * height * pixel_size);

    void* buffer = lv_malloc(data_size);
    memcpy(buffer, data, data_size);

    lv_image_dsc_t* lv_dsc = lv_malloc(sizeof(*lv_dsc));
    lv_dsc->header.magic = LV_IMAGE_HEADER_MAGIC;
    lv_dsc->header.cf = color_format;
    lv_dsc->header.flags = 0;
    lv_dsc->header.w = width;
    lv_dsc->header.h = height;
    lv_dsc->header.stride = width * pixel_size;
    lv_dsc->header.reserved_2 = 0;
    lv_dsc->data_size = data_size;
    lv_dsc->data = buffer;
    lv_dsc->reserved = NULL;
    lv_dsc->reserved_2 = NULL;

    lv_image_set_src(instance->image, NULL);
    image_clear_raw_source(instance);
    lv_image_set_src(instance->image, lv_dsc);

    instance->raw_source_dsc = lv_dsc;
    instance->raw_source_buffer = buffer;
}

void image_set_opacity(Image* instance, uint8_t opacity) {
    furi_check(instance);

    lv_obj_set_style_image_opa(instance->image, opacity, LV_PART_MAIN);
}

// LVGL class descriptor

const lv_obj_class_t image_lvgl_class = {
    .base_class = &widget_lvgl_class,
    .constructor_cb = image_lvgl_constructor,
    .destructor_cb = image_lvgl_destructor,
    .name = "widget-image",
    .width_def = LV_SIZE_CONTENT,
    .height_def = LV_SIZE_CONTENT,
    .instance_size = sizeof(Image),
};
