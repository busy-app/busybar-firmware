#include "canvas.h"
#include "canvas_i.h"
#include "font/fonts.h"
#include "font/font_render.h"

static Canvas* instance = NULL;

#define FB_WIDTH  160
#define FB_HEIGHT 80

static const uint8_t* fonts[] = {
    [FontPrimary] = u8g2_font_helvB08_tr,
    [FontSecondary] = u8g2_font_haxrcorp4089_tr,
    [FontKeyboard] = u8g2_font_profont11_mr,
    [FontBigNumbers] = u8g2_font_profont22_tn,
};

static const uint8_t* canvas_font_get(Font font) {
    if(font >= sizeof(fonts) / sizeof(fonts[0])) {
        furi_crash("Invalid font");
    }

    return (const uint8_t*)fonts[font];
}

struct Canvas {
    CanvasOrientation orientation;
    int32_t offset_x;
    int32_t offset_y;
    size_t width;
    size_t height;
    Color color;
    U8G2FontRender font_render;

    CanvasCallbackPairArray_t canvas_callback_pair;
    FuriMutex* mutex;

    uint8_t* dest;
};

static uint32_t canvas_get_color(Canvas* canvas) {
    return canvas->color;
}

// TODO:
// static uint32_t canvas_get_bg_color(Canvas* canvas) {
//     switch(canvas->color) {
//     case ColorWhite:
//         return 0xF;
//     case ColorBlack:
//     default:
//         return 0;
//     }
// }

static void canvas_draw_pixel(Canvas* canvas, int32_t x, int32_t y, uint32_t color) {
    uint8_t* buf_ptr = &canvas->dest[(y * 160 + x) / 2];
    if(x % 2) {
        *buf_ptr = (*buf_ptr & 0x0F) | ((color & 0x0F) << 4);
    } else {
        *buf_ptr = (*buf_ptr & 0xF0) | (color & 0x0F);
    }
}

static void
    canvas_draw_h_line(Canvas* canvas, int32_t x, int32_t y, size_t width, uint32_t color) {
    for(size_t x_pos = x; x_pos < (x + width); x_pos++) {
        canvas_draw_pixel(canvas, x_pos, y, color);
    }
}

static void
    canvas_draw_v_line(Canvas* canvas, int32_t x, int32_t y, size_t height, uint32_t color) {
    for(size_t y_pos = y; y_pos < (y + height); y_pos++) {
        canvas_draw_pixel(canvas, x, y_pos, color);
    }
}

static void canvas_fill_box(
    Canvas* canvas,
    int32_t x,
    int32_t y,
    size_t width,
    size_t height,
    uint32_t color) {
    for(size_t y_pos = y; y_pos < (y + height); y_pos++) {
        for(size_t x_pos = x; x_pos < (x + width); x_pos++) {
            canvas_draw_pixel(canvas, x_pos, y_pos, color);
        }
    }
}

static void canvas_draw_pixel_fg(int32_t x, int32_t y, void* context) {
    Canvas* canvas = (Canvas*)context;
    x += canvas->offset_x;
    y += canvas->offset_y;
    canvas_draw_pixel(canvas, x, y, canvas_get_color(canvas));
}

static void canvas_draw_pixel_bg(int32_t x, int32_t y, void* context) {
    Canvas* canvas = (Canvas*)context;
    x += canvas->offset_x;
    y += canvas->offset_y;
    // TODO: BG not used for now
    // canvas_draw_pixel(canvas, x, y, canvas_get_bg_color(canvas));
}

static void canvas_alloc_dest(Canvas* canvas, size_t width, size_t height) {
    if(canvas->dest != NULL) {
        free(canvas->dest);
    }
    canvas->dest = malloc(width * height / 2);
}

static void canvas_cb_lock(Canvas* canvas) {
    furi_assert(canvas);
    furi_check(furi_mutex_acquire(canvas->mutex, FuriWaitForever) == FuriStatusOk);
}

static void canvas_cb_unlock(Canvas* canvas) {
    furi_assert(canvas);
    furi_check(furi_mutex_release(canvas->mutex) == FuriStatusOk);
}

static void canvas_cb_call(Canvas* canvas) {
    canvas_cb_lock(canvas);
    for
        M_EACH(p, canvas->canvas_callback_pair, CanvasCallbackPairArray_t) {
            p->callback(
                canvas_get_buffer(canvas),
                canvas_get_buffer_size(canvas),
                canvas_get_orientation(canvas),
                p->context);
        }
    canvas_cb_unlock(canvas);
}

void canvas_add_framebuffer_callback(Canvas* canvas, CanvasCommitCallback callback, void* context) {
    furi_check(canvas);

    const CanvasCallbackPair p = {callback, context};

    canvas_cb_lock(canvas);
    furi_check(!CanvasCallbackPairArray_count(canvas->canvas_callback_pair, p));
    CanvasCallbackPairArray_push_back(canvas->canvas_callback_pair, p);
    canvas_cb_unlock(canvas);
}

void canvas_remove_framebuffer_callback(
    Canvas* canvas,
    CanvasCommitCallback callback,
    void* context) {
    furi_check(canvas);

    const CanvasCallbackPair p = {callback, context};

    canvas_cb_lock(canvas);
    furi_check(CanvasCallbackPairArray_count(canvas->canvas_callback_pair, p) == 1);
    CanvasCallbackPairArray_remove_val(canvas->canvas_callback_pair, p);
    canvas_cb_unlock(canvas);
}

static void canvas_start_draw_call(Canvas* canvas) {
    UNUSED(canvas);
}

static void canvas_end_draw_call(Canvas* canvas) {
    UNUSED(canvas);
}

Canvas* canvas_init(void) {
    // TODO: moar than one canvas?
    furi_check(instance == NULL, "Canvas already initialized");
    instance = malloc(sizeof(Canvas));

    instance->mutex = furi_mutex_alloc(FuriMutexTypeNormal);
    instance->orientation = CanvasOrientationHorizontal;
    instance->width = FB_WIDTH;
    instance->height = FB_HEIGHT;
    instance->offset_x = 0;
    instance->offset_y = 0;
    instance->font_render = u8g2_font_render(
        canvas_font_get(FontSecondary), canvas_draw_pixel_fg, canvas_draw_pixel_bg, instance);

    canvas_alloc_dest(instance, instance->width, instance->height);

    canvas_start_draw_call(instance);

    canvas_reset(instance);
    canvas_commit(instance);

    return instance;
}

void canvas_reset(Canvas* canvas) {
    canvas_clear(canvas);
    canvas_set_color(canvas, ColorBlack);
    canvas_set_font(canvas, FontSecondary);
    // canvas_set_font_direction(canvas, CanvasDirectionLeftToRight);
}

void canvas_clear(Canvas* canvas) {
    furi_check(canvas);
    memset(canvas->dest, 0, canvas_get_buffer_size(canvas));
}

void canvas_set_color(Canvas* canvas, Color color) {
    furi_check(canvas);
    canvas->color = color;
}

void canvas_invert_color(Canvas* canvas) {
    furi_check(canvas);
    canvas->color = (canvas->color == ColorBlack) ? ColorWhite : ColorBlack;
}

void canvas_set_font(Canvas* canvas, Font font) {
    canvas->font_render = u8g2_font_render(
        canvas_font_get(font), canvas_draw_pixel_fg, canvas_draw_pixel_bg, instance);
}

void canvas_set_font_direction(Canvas* canvas, CanvasDirection dir) {
    UNUSED(canvas);
    UNUSED(dir);
    furi_crash("Not implemented");
}

void canvas_commit(Canvas* canvas) {
    furi_check(canvas);

    canvas_end_draw_call(canvas);

    canvas_cb_call(canvas);

    canvas_start_draw_call(instance);
}

uint8_t* canvas_get_buffer(Canvas* canvas) {
    furi_check(canvas);
    return canvas->dest;
}

size_t canvas_get_buffer_size(const Canvas* canvas) {
    furi_check(canvas);
    return (canvas->width * canvas->height / 2);
}

CanvasOrientation canvas_get_orientation(const Canvas* canvas) {
    furi_check(canvas);
    return canvas->orientation;
}

void canvas_draw_dot(Canvas* canvas, int32_t x, int32_t y) {
    furi_check(canvas);
    x += canvas->offset_x;
    y += canvas->offset_y;
    canvas_draw_pixel(canvas, x, y, canvas_get_color(canvas));
}

void canvas_draw_box(Canvas* canvas, int32_t x, int32_t y, size_t width, size_t height) {
    furi_check(canvas);
    x += canvas->offset_x;
    y += canvas->offset_y;
    canvas_fill_box(canvas, x, y, width, height, canvas_get_color(canvas));
}

static void canvas_draw_circle_segment(
    Canvas* canvas,
    int16_t x0,
    int16_t y0,
    int16_t r,
    uint8_t corner_mask,
    uint16_t color) {
    int16_t f = 1 - r;
    int16_t ddf_x = 1;
    int16_t ddf_y = -2 * r;
    int16_t x = 0;
    int16_t y = r;

    while(x < y) {
        if(f >= 0) {
            y--;
            ddf_y += 2;
            f += ddf_y;
        }
        x++;
        ddf_x += 2;
        f += ddf_x;
        if(corner_mask & 0x4) {
            canvas_draw_pixel(canvas, x0 + x, y0 + y, color);
            canvas_draw_pixel(canvas, x0 + y, y0 + x, color);
        }
        if(corner_mask & 0x2) {
            canvas_draw_pixel(canvas, x0 + x, y0 - y, color);
            canvas_draw_pixel(canvas, x0 + y, y0 - x, color);
        }
        if(corner_mask & 0x8) {
            canvas_draw_pixel(canvas, x0 - y, y0 + x, color);
            canvas_draw_pixel(canvas, x0 - x, y0 + y, color);
        }
        if(corner_mask & 0x1) {
            canvas_draw_pixel(canvas, x0 - y, y0 - x, color);
            canvas_draw_pixel(canvas, x0 - x, y0 - y, color);
        }
    }
}

static void canvas_draw_disc_segment(
    Canvas* canvas,
    int16_t x0,
    int16_t y0,
    int16_t r,
    uint8_t corner_mask,
    int16_t delta,
    uint32_t color) {
    int16_t f = 1 - r;
    int16_t ddf_x = 1;
    int16_t ddf_y = -2 * r;
    int16_t x = 0;
    int16_t y = r;
    int16_t px = x;
    int16_t py = y;

    delta++;

    while(x < y) {
        if(f >= 0) {
            y--;
            ddf_y += 2;
            f += ddf_y;
        }
        x++;
        ddf_x += 2;
        f += ddf_x;

        if(x < (y + 1)) {
            if(corner_mask & 0x1) {
                canvas_draw_v_line(canvas, x0 + x, y0 - y, 2 * y + delta, color);
            }
            if(corner_mask & 0x2) {
                canvas_draw_v_line(canvas, x0 - x, y0 - y, 2 * y + delta, color);
            }
        }
        if(y != py) {
            if(corner_mask & 0x1) {
                canvas_draw_v_line(canvas, x0 + py, y0 - px, 2 * px + delta, color);
            }
            if(corner_mask & 0x2) {
                canvas_draw_v_line(canvas, x0 - py, y0 - px, 2 * px + delta, color);
            }
            py = y;
        }
        px = x;
    }
}

void canvas_draw_rbox(
    Canvas* canvas,
    int32_t x,
    int32_t y,
    size_t width,
    size_t height,
    size_t radius) {
    furi_check(canvas);
    x += canvas->offset_x;
    y += canvas->offset_y;

    uint32_t color = canvas_get_color(canvas);

    canvas_fill_box(canvas, x + radius, y, width - 2 * radius, height, color);
    canvas_draw_disc_segment(
        canvas, x + width - radius - 1, y + radius, radius, 1, height - 2 * radius - 1, color);
    canvas_draw_disc_segment(
        canvas, x + radius, y + radius, radius, 2, height - 2 * radius - 1, color);
}

void canvas_draw_frame(Canvas* canvas, int32_t x, int32_t y, size_t width, size_t height) {
    furi_check(canvas);
    x += canvas->offset_x;
    y += canvas->offset_y;

    uint32_t color = canvas_get_color(canvas);
    canvas_draw_h_line(canvas, x, y, width, color);
    canvas_draw_v_line(canvas, x, y, height, color);
    canvas_draw_h_line(canvas, x, y + height - 1, width, color);
    canvas_draw_v_line(canvas, x, y + width - 1, height, color);
}

void canvas_draw_rframe(
    Canvas* canvas,
    int32_t x,
    int32_t y,
    size_t width,
    size_t height,
    size_t radius) {
    furi_check(canvas);
    x += canvas->offset_x;
    y += canvas->offset_y;

    uint32_t color = canvas_get_color(canvas);
    canvas_draw_h_line(canvas, x + radius, y, width - 2 * radius, color); // Top
    canvas_draw_h_line(canvas, x + radius, y + height - 1, width - 2 * radius, color); // Bottom
    canvas_draw_v_line(canvas, x, y + radius, height - 2 * radius, color); // Left
    canvas_draw_v_line(canvas, x + width - 1, y + radius, height - 2 * radius, color); // Right

    canvas_draw_circle_segment(canvas, x + radius, y + radius, radius, 1, color);
    canvas_draw_circle_segment(canvas, x + width - radius - 1, y + radius, radius, 2, color);
    canvas_draw_circle_segment(
        canvas, x + width - radius - 1, y + height - radius - 1, radius, 4, color);
    canvas_draw_circle_segment(canvas, x + radius, y + height - radius - 1, radius, 8, color);
}

void canvas_draw_line(Canvas* canvas, int32_t x1, int32_t y1, int32_t x2, int32_t y2) {
    furi_check(canvas);
    x1 += canvas->offset_x;
    y1 += canvas->offset_y;
    x2 += canvas->offset_x;
    y2 += canvas->offset_y;

    uint32_t color = canvas_get_color(canvas);
    if(x1 == x2) {
        if(y1 > y2) {
            FURI_SWAP(y1, y2);
        }
        canvas_draw_v_line(canvas, x1, y1, y2 - y1 + 1, color);
    } else if(y1 == y2) {
        if(x1 > x2) {
            FURI_SWAP(x1, x2);
        }
        canvas_draw_h_line(canvas, x1, y1, x2 - x1 + 1, color);
    } else {
        int16_t steep = abs(y2 - y1) > abs(x2 - x1);
        if(steep) {
            FURI_SWAP(x1, y1);
            FURI_SWAP(x2, y2);
        }

        if(x1 > x2) {
            FURI_SWAP(x1, x2);
            FURI_SWAP(y1, y2);
        }

        int16_t dx = x2 - x1;
        int16_t dy = abs(y2 - y1);

        int16_t err = dx / 2;
        int16_t ystep;

        if(y1 < y2) {
            ystep = 1;
        } else {
            ystep = -1;
        }

        for(; x1 <= x2; x1++) {
            if(steep) {
                canvas_draw_pixel(canvas, y1, x1, color);
            } else {
                canvas_draw_pixel(canvas, x1, y1, color);
            }
            err -= dy;
            if(err < 0) {
                y1 += ystep;
                err += dx;
            }
        }
    }
}

void canvas_draw_circle(Canvas* canvas, int32_t x0, int32_t y0, size_t radius) {
    furi_check(canvas);
    x0 += canvas->offset_x;
    y0 += canvas->offset_y;

    int16_t f = 1 - radius;
    int16_t ddf_x = 1;
    int16_t ddf_y = -2 * radius;
    int16_t x = 0;
    int16_t y = radius;

    uint32_t color = canvas_get_color(canvas);
    canvas_draw_pixel(canvas, x0, y0 + radius, color);
    canvas_draw_pixel(canvas, x0, y0 - radius, color);
    canvas_draw_pixel(canvas, x0 + radius, y0, color);
    canvas_draw_pixel(canvas, x0 - radius, y0, color);

    while(x < y) {
        if(f >= 0) {
            y--;
            ddf_y += 2;
            f += ddf_y;
        }
        x++;
        ddf_x += 2;
        f += ddf_x;

        canvas_draw_pixel(canvas, x0 + x, y0 + y, color);
        canvas_draw_pixel(canvas, x0 - x, y0 + y, color);
        canvas_draw_pixel(canvas, x0 + x, y0 - y, color);
        canvas_draw_pixel(canvas, x0 - x, y0 - y, color);
        canvas_draw_pixel(canvas, x0 + y, y0 + x, color);
        canvas_draw_pixel(canvas, x0 - y, y0 + x, color);
        canvas_draw_pixel(canvas, x0 + y, y0 - x, color);
        canvas_draw_pixel(canvas, x0 - y, y0 - x, color);
    }
}

void canvas_draw_disc(Canvas* canvas, int32_t x, int32_t y, size_t radius) {
    furi_check(canvas);
    x += canvas->offset_x;
    y += canvas->offset_y;
    uint32_t color = canvas_get_color(canvas);
    canvas_draw_v_line(canvas, x, y - radius, 2 * radius + 1, color);
    canvas_draw_disc_segment(canvas, x, y, radius, 3, 0, color);
}

size_t canvas_width(const Canvas* canvas) {
    furi_check(canvas);
    return canvas->width;
}

size_t canvas_height(const Canvas* canvas) {
    furi_check(canvas);
    return canvas->height;
}

void canvas_draw_str(Canvas* canvas, int32_t x, int32_t y, const char* str) {
    furi_check(canvas);
    x += canvas->offset_x;
    y += canvas->offset_y;
    u8g2_font_render_print(&canvas->font_render, x, y, str);
}

size_t canvas_string_width(Canvas* canvas, const char* str) {
    furi_check(canvas);
    if(!str) return 0;
    return u8g2_font_str_width_get(&canvas->font_render, str);
}

static size_t canvas_font_get_ascent(Canvas* canvas) {
    return canvas->font_render.header.ascent_A;
}

void canvas_draw_str_aligned(
    Canvas* canvas,
    int32_t x,
    int32_t y,
    Align horizontal,
    Align vertical,
    const char* str) {
    furi_check(canvas);
    if(!str) return;

    switch(horizontal) {
    case AlignLeft:
        break;
    case AlignRight:
        x -= canvas_string_width(canvas, str);
        break;
    case AlignCenter:
        x -= (canvas_string_width(canvas, str) / 2);
        break;
    default:
        furi_crash();
        break;
    }

    switch(vertical) {
    case AlignTop:
        y += canvas_font_get_ascent(canvas);
        break;
    case AlignBottom:
        break;
    case AlignCenter:
        y += (canvas_font_get_ascent(canvas) / 2);
        break;
    default:
        furi_crash();
        break;
    }

    canvas_draw_str(canvas, x, y, str);
}

void canvas_set_bitmap_mode(Canvas* canvas, bool alpha) {
    UNUSED(canvas);
    UNUSED(alpha);
    // u8g2_SetBitmapMode(&canvas->fb, alpha ? 1 : 0);
}

void canvas_frame_set(
    Canvas* canvas,
    int32_t offset_x,
    int32_t offset_y,
    size_t width,
    size_t height) {
    furi_check(canvas);
    canvas->offset_x = offset_x;
    canvas->offset_y = offset_y;
    canvas->width = width;
    canvas->height = height;
}

void canvas_set_orientation(Canvas* canvas, CanvasOrientation orientation) {
    furi_check(canvas);
    // const u8g2_cb_t* rotate_cb = NULL;
    // bool need_swap = false;
    if(canvas->orientation != orientation) {
        canvas->orientation = orientation;
        // TODO:
        // switch(orientation) {
        // case CanvasOrientationHorizontal:
        //     need_swap = canvas->orientation == CanvasOrientationVertical ||
        //                 canvas->orientation == CanvasOrientationVerticalFlip;
        //     rotate_cb = U8G2_R0;
        //     break;
        // case CanvasOrientationHorizontalFlip:
        //     need_swap = canvas->orientation == CanvasOrientationVertical ||
        //                 canvas->orientation == CanvasOrientationVerticalFlip;
        //     rotate_cb = U8G2_R2;
        //     break;
        // case CanvasOrientationVertical:
        //     need_swap = canvas->orientation == CanvasOrientationHorizontal ||
        //                 canvas->orientation == CanvasOrientationHorizontalFlip;
        //     rotate_cb = U8G2_R3;
        //     break;
        // case CanvasOrientationVerticalFlip:
        //     need_swap = canvas->orientation == CanvasOrientationHorizontal ||
        //                 canvas->orientation == CanvasOrientationHorizontalFlip;
        //     rotate_cb = U8G2_R1;
        //     break;
        // default:
        //     furi_crash();
        // }

        // if(need_swap) FURI_SWAP(canvas->width, canvas->height);
        // u8g2_SetDisplayRotation(&canvas->fb, rotate_cb);
        // canvas->orientation = orientation;
    }
}

void canvas_draw_icon(Canvas* canvas, int32_t x, int32_t y, const Icon* icon) {
    UNUSED(canvas);
    UNUSED(x);
    UNUSED(y);
    UNUSED(icon);
}

void canvas_draw_icon_animation(
    Canvas* canvas,
    int32_t x,
    int32_t y,
    IconAnimation* icon_animation) {
    furi_check(canvas);
    furi_check(icon_animation);
    UNUSED(x);
    UNUSED(y);
    UNUSED(icon_animation);
}
