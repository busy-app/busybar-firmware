#include "canvas.h"

#include <gui/widget_i.h>

#define MY_CLASS (&canvas_lvgl_class)

#define TO_LV_COLOR(c) (*(lv_color_t*)(&c))

const lv_obj_class_t canvas_lvgl_class;

struct Canvas {
    Widget base;
    lv_obj_t* canvas;
    lv_draw_buf_t* draw_buf;
    Color fill_color;
    Color line_color;
    int32_t line_width;
    uint8_t fill_opacity;
    uint8_t line_opacity;
};

// LVGL-specific code

static void canvas_lvgl_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    UNUSED(class_p);

    Canvas* instance = (Canvas*)obj;
    instance->canvas = lv_canvas_create(obj);
    instance->fill_color = color_hex_to_rgb(0xFFFFFF);
    instance->line_color = color_hex_to_rgb(0xFFFFFF);
    instance->line_width = 1;
    instance->fill_opacity = 0xFF;
    instance->line_opacity = 0xFF;
}

static void canvas_lvgl_destructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    UNUSED(class_p);

    Canvas* instance = (Canvas*)obj;

    lv_obj_delete(instance->canvas);
    lv_draw_buf_destroy(instance->draw_buf);
}

// Public API

Canvas* canvas_alloc(Widget* parent, int32_t width, int32_t height) {
    furi_check(parent);
    furi_check(width);
    furi_check(height);

    lv_obj_t* obj = lv_obj_class_create_obj(MY_CLASS, (lv_obj_t*)parent);
    lv_obj_class_init_obj(obj);

    Canvas* instance = (Canvas*)obj;
    instance->draw_buf = lv_draw_buf_create(width, height, LV_COLOR_FORMAT_RGB888, 0);
    lv_canvas_set_draw_buf(instance->canvas, instance->draw_buf);

    return instance;
}

void canvas_free(Canvas* instance) {
    furi_check(instance);
    lv_obj_delete((lv_obj_t*)instance);
}

void canvas_clear(Canvas* instance) {
    furi_check(instance);
    lv_area_t area;
    lv_obj_get_coords(instance->canvas, &area);
    lv_draw_buf_clear(instance->draw_buf, &area);
}

void canvas_set_fill_color(Canvas* instance, Color color) {
    furi_check(instance);
    instance->fill_color = color;
}

void canvas_set_fill_opacity(Canvas* instance, uint8_t opacity) {
    furi_check(instance);
    instance->fill_opacity = opacity;
}

void canvas_set_line_color(Canvas* instance, Color color) {
    furi_check(instance);
    instance->line_color = color;
}

void canvas_set_line_width(Canvas* instance, int32_t width) {
    furi_check(instance);
    instance->line_width = width;
}

void canvas_set_line_opacity(Canvas* instance, uint8_t opacity) {
    furi_check(instance);
    instance->line_opacity = opacity;
}

void canvas_fill(Canvas* instance) {
    furi_check(instance);
    lv_canvas_fill_bg(instance->canvas, TO_LV_COLOR(instance->fill_color), instance->fill_opacity);
}

void canvas_draw_pixel(Canvas* instance, int32_t x, int32_t y, Color color) {
    furi_check(instance);
    lv_canvas_set_px(instance->canvas, x, y, TO_LV_COLOR(color), LV_OPA_COVER);
}

void canvas_draw_line(Canvas* instance, int32_t x1, int32_t y1, int32_t x2, int32_t y2) {
    furi_check(instance);

    const lv_draw_line_dsc_t draw = {
        .p1 =
            {
                .x = x1,
                .y = y1,
            },
        .p2 =
            {
                .x = x2,
                .y = y2,
            },
        .color = TO_LV_COLOR(instance->line_color),
        .width = instance->line_width,
        .opa = instance->line_opacity,
    };

    lv_layer_t layer;
    lv_canvas_init_layer(instance->canvas, &layer);
    lv_draw_line(&layer, &draw);
    lv_canvas_finish_layer(instance->canvas, &layer);
}

void canvas_draw_rect(
    Canvas* instance,
    int32_t x,
    int32_t y,
    int32_t width,
    int32_t height,
    bool fill) {
    furi_check(instance);

    lv_draw_rect_dsc_t draw = {0};
    draw.border_color = TO_LV_COLOR(instance->line_color);
    draw.border_width = instance->line_width;
    draw.border_opa = instance->line_opacity;
    draw.border_side = LV_BORDER_SIDE_FULL;

    if(fill) {
        draw.bg_color = TO_LV_COLOR(instance->fill_color);
        draw.bg_opa = instance->fill_opacity;
    }

    const lv_area_t area = {
        .x1 = x,
        .y1 = y,
        .x2 = x + width - 1,
        .y2 = y + height - 1,
    };

    lv_layer_t layer;
    lv_canvas_init_layer(instance->canvas, &layer);
    lv_draw_rect(&layer, &draw, &area);
    lv_canvas_finish_layer(instance->canvas, &layer);
}

void canvas_draw_text(Canvas* instance, int32_t x, int32_t y, const char* text) {
    furi_check(instance);
    furi_check(text);

    const lv_draw_label_dsc_t draw = {
        .text = text,
        .text_length = strlen(text),
        .font = lv_theme_get_font_normal((lv_obj_t*)instance), // TODO: font parameter
        .color = TO_LV_COLOR(instance->fill_color),
        .ofs_x = x,
        .ofs_y = y,
        .opa = instance->fill_opacity,
    };

    lv_layer_t layer;
    lv_canvas_init_layer(instance->canvas, &layer);

    const lv_area_t area = {
        .x1 = x,
        .y1 = y,
        .x2 = layer.buf_area.x2,
        .y2 = layer.buf_area.y2,
    };

    lv_draw_label(&layer, &draw, &area);
    lv_canvas_finish_layer(instance->canvas, &layer);
}

void canvas_draw_text_fmt(Canvas* instance, int32_t x, int32_t y, const char* fmt, ...) {
    furi_check(fmt);

    va_list args;
    va_start(args, fmt);

    FuriString* str = furi_string_alloc_vprintf(fmt, args);
    va_end(args);

    canvas_draw_text(instance, x, y, furi_string_get_cstr(str));
    furi_string_free(str);
}

// LVGL class descriptor

const lv_obj_class_t canvas_lvgl_class = {
    .base_class = &widget_lvgl_class,
    .constructor_cb = canvas_lvgl_constructor,
    .destructor_cb = canvas_lvgl_destructor,
    .name = "canvas-ex",
    .width_def = LV_SIZE_CONTENT,
    .height_def = LV_SIZE_CONTENT,
    .instance_size = sizeof(Canvas),
};
