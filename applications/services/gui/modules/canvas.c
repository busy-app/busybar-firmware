#include "canvas.h"

#include <gui/widget_i.h>
#include "src/widgets/canvas/lv_canvas_private.h"

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
    lv_layer_t layer;
    size_t draw_nested;
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

static void lv_canvas_set_px_no_invalidate(
    lv_obj_t* obj,
    int32_t x,
    int32_t y,
    lv_color_t color,
    lv_opa_t opa) {
    LV_ASSERT_OBJ(obj, MY_CLASS);

    lv_canvas_t* canvas = (lv_canvas_t*)obj;
    lv_draw_buf_t* draw_buf = canvas->draw_buf;

    lv_color_format_t cf = draw_buf->header.cf;
    uint8_t* data = lv_draw_buf_goto_xy(draw_buf, x, y);

    if(LV_COLOR_FORMAT_IS_INDEXED(cf)) {
        uint8_t shift;
        uint8_t c_int = color.blue;
        switch(cf) {
        case LV_COLOR_FORMAT_I1:
            shift = 7 - (x & 0x7);
            break;
        case LV_COLOR_FORMAT_I2:
            shift = 6 - 2 * (x & 0x3);
            break;
        case LV_COLOR_FORMAT_I4:
            shift = 4 - 4 * (x & 0x1);
            break;
        case LV_COLOR_FORMAT_I8:
            /*Indexed8 format is a easy case, process and return.*/
            shift = 0;
            *data = c_int;
        default:
            return;
        }

        uint8_t bpp = lv_color_format_get_bpp(cf);
        uint8_t mask = (1 << bpp) - 1;
        c_int &= mask;
        *data = (*data & ~(mask << shift)) | (c_int << shift);
    } else if(cf == LV_COLOR_FORMAT_L8) {
        *data = lv_color_luminance(color);
    } else if(cf == LV_COLOR_FORMAT_A8) {
        *data = opa;
    } else if(cf == LV_COLOR_FORMAT_RGB565) {
        lv_color16_t* buf = (lv_color16_t*)data;
        buf->red = color.red >> 3;
        buf->green = color.green >> 2;
        buf->blue = color.blue >> 3;
    } else if(cf == LV_COLOR_FORMAT_RGB888) {
        data[2] = color.red;
        data[1] = color.green;
        data[0] = color.blue;
    } else if(cf == LV_COLOR_FORMAT_XRGB8888) {
        data[2] = color.red;
        data[1] = color.green;
        data[0] = color.blue;
        data[3] = 0xFF;
    } else if(cf == LV_COLOR_FORMAT_ARGB8888) {
        lv_color32_t* buf = (lv_color32_t*)data;
        buf->red = color.red;
        buf->green = color.green;
        buf->blue = color.blue;
        buf->alpha = opa;
    } else if(cf == LV_COLOR_FORMAT_AL88) {
        lv_color16a_t* buf = (lv_color16a_t*)data;
        buf->lumi = lv_color_luminance(color);
        buf->alpha = 255;
    }
    lv_obj_invalidate(obj);
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

void canvas_draw_begin(Canvas* instance) {
    furi_check(instance);
    if(instance->draw_nested == 0) {
        lv_canvas_init_layer(instance->canvas, &instance->layer);
    }

    instance->draw_nested++;
}

void canvas_draw_end(Canvas* instance) {
    furi_check(instance);
    furi_check(instance->draw_nested > 0);

    instance->draw_nested--;

    if(instance->draw_nested == 0) {
        lv_canvas_finish_layer(instance->canvas, &instance->layer);
    }
}

Widget* canvas_get_base(Canvas* instance) {
    furi_check(instance);
    return (Widget*)instance;
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
    lv_canvas_set_px_no_invalidate(instance->canvas, x, y, TO_LV_COLOR(color), LV_OPA_COVER);
}

void canvas_draw_line(Canvas* instance, int32_t x1, int32_t y1, int32_t x2, int32_t y2) {
    furi_check(instance);
    furi_check(instance->draw_nested > 0);

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

    lv_draw_line(&instance->layer, &draw);
}

void canvas_draw_rect(
    Canvas* instance,
    int32_t x,
    int32_t y,
    int32_t width,
    int32_t height,
    bool fill) {
    furi_check(instance);
    furi_check(instance->draw_nested > 0);

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

    lv_draw_rect(&instance->layer, &draw, &area);
}

void canvas_draw_text(Canvas* instance, int32_t x, int32_t y, const char* text) {
    furi_check(instance);
    furi_check(text);
    furi_check(instance->draw_nested > 0);

    const lv_draw_label_dsc_t draw = {
        .text = text,
        .text_length = strlen(text),
        .font = lv_theme_get_font_normal((lv_obj_t*)instance), // TODO: font parameter
        .color = TO_LV_COLOR(instance->fill_color),
        .ofs_x = x,
        .ofs_y = y,
        .opa = instance->fill_opacity,
    };

    const lv_area_t area = {
        .x1 = x,
        .y1 = y,
        .x2 = instance->layer.buf_area.x2,
        .y2 = instance->layer.buf_area.y2,
    };

    lv_draw_label(&instance->layer, &draw, &area);
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
    .name = "widget-canvas",
    .width_def = LV_SIZE_CONTENT,
    .height_def = LV_SIZE_CONTENT,
    .instance_size = sizeof(Canvas),
};
