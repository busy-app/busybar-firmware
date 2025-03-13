/**
 * @file canvas.h
 * @brief A canvas widget for drawing arbitrary graphics.
 *
 * @note Currently, only the RBG888 color format is supported.
 */
#pragma once

#include <gui/widget.h>

#include <toolbox/color.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Canvas Canvas;

Canvas* canvas_alloc(Widget* parent, int32_t width, int32_t height);

void canvas_free(Canvas* instance);

void canvas_clear(Canvas* instance);

void canvas_fill(Canvas* instance, Color color);

void canvas_draw_pixel(Canvas* instance, int32_t x, int32_t y, Color color);

void canvas_draw_line(Canvas* instance, int32_t x1, int32_t y1, int32_t x2, int32_t y2, Color color);

void canvas_draw_rect(
    Canvas* instance,
    int32_t x,
    int32_t y,
    int32_t w,
    int32_t h,
    Color color,
    bool fill);

#ifdef __cplusplus
}
#endif
