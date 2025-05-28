#include "snap_image.h"

#include <gui/widget_i.h>

#define MY_CLASS (&snap_image_lvgl_class)

struct SnapImage {
    Widget base;
    lv_obj_t* canvas;
    uint8_t* canvas_buf;
    uint8_t effect_strength[SnapImageEffectMax];
};

const lv_obj_class_t snap_image_lvgl_class;

typedef void (*const SnapImageEffectHandler)(SnapImage* instance, uint8_t strength);

static const SnapImageEffectHandler effect_handlers[SnapImageEffectMax];

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

// Implementation

static lv_color_t snap_image_get_box_average(lv_draw_buf_t* buf, uint32_t buf_width, uint32_t buf_height, uint32_t cx, uint32_t cy) {
    const uint32_t x1 = cx > 0 ? cx - 1 : cx;
    const uint32_t y1 = cy > 0 ? cy - 1 : cy;

    const uint32_t x2 = cx < (buf_width - 1) ? cx + 1 : cx;
    const uint32_t y2 = cy < (buf_height - 1) ? cy + 1 : cy;

    uint32_t r = 0, g = 0, b = 0;

    for(uint32_t x = x1; x <= x2; ++x) {
        for(uint32_t y = y1; y <= y2; ++y) {
            const lv_color_t* color = lv_draw_buf_goto_xy(buf, x, y);
            r += color->red;
            g += color->green;
            b += color->blue;
        }
    }

    return lv_color_make(r / 9, g / 9, b / 9);
}

static void snap_image_box_blur_3x3(lv_draw_buf_t* buf) {
    const lv_image_header_t* header = &buf->header;
    furi_check(header->cf == LV_COLOR_FORMAT_RGB888);

    const uint32_t buf_width = header->w;
    const uint32_t buf_height = header->h;

    lv_draw_buf_t* ref_buf = lv_draw_buf_dup(buf);

    for(uint32_t x = 0; x < buf_width; ++x) {
        for(uint32_t y = 0; y < buf_height; ++y) {
            const lv_color_t average = snap_image_get_box_average(ref_buf, buf_width, buf_height, x, y);
            *(lv_color_t*)lv_draw_buf_goto_xy(buf, x, y) = average;
        }
    }

    lv_draw_buf_destroy(ref_buf);
}

static void snap_image_blur_effect_handler(SnapImage* instance, uint8_t strength) {
    lv_draw_buf_t* buf = lv_canvas_get_draw_buf(instance->canvas);

    for(uint32_t i = 0; i < strength / 127; ++i) {
        snap_image_box_blur_3x3(buf);
    }
}

static void snap_image_dim_effect_handler(SnapImage* instance, uint8_t strength) {
    lv_draw_buf_t* buf = lv_canvas_get_draw_buf(instance->canvas);

    const lv_image_header_t* header = &buf->header;
    furi_check(header->cf == LV_COLOR_FORMAT_RGB888);

    const uint32_t buf_width = header->w;
    const uint32_t buf_height = header->h;

    for(uint32_t x = 0; x < buf_width; ++x) {
        for(uint32_t y = 0; y < buf_height; ++y) {
            lv_color_t* color = lv_draw_buf_goto_xy(buf, x, y);
            *color = lv_color_darken(*color, strength);
        }
    }
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

void snap_image_set_effect(SnapImage* instance, SnapImageEffect effect, uint8_t strength) {
    furi_check(instance);
    furi_check(effect < SnapImageEffectMax);

    instance->effect_strength[effect] = strength;
}

void snap_image_capture_display(SnapImage* instance) {
    furi_check(instance);

    lv_display_t* display = lv_obj_get_display((lv_obj_t*)instance);
    const lv_draw_buf_t* display_buf = lv_display_get_buf_active(display);

    memcpy(instance->canvas_buf, display_buf->data, display_buf->data_size);

    for(uint32_t i = 0; i < SnapImageEffectMax; ++i) {
        const uint8_t strength = instance->effect_strength[i];

        if(strength) {
            effect_handlers[i](instance, strength);
        }
    }

    lv_obj_invalidate(TO_LV_OBJ(instance));
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

// Effect handlers

static const SnapImageEffectHandler effect_handlers[SnapImageEffectMax] = {
    [SnapImageEffectBlur] = snap_image_blur_effect_handler,
    [SnapImageEffectDim] = snap_image_dim_effect_handler,
};
