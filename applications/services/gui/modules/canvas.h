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

/** Canvas opaque structure. */
typedef struct Canvas Canvas;

/**
 * @brief Create a new Canvas instance.
 *
 * @param[in,out] parent pointer to the parent Widget instance
 *
 * @returns pointer to the newly created Canvas instance
 */
Canvas* canvas_alloc(Widget* parent, int32_t width, int32_t height);

/**
 * @brief Delete a Canvas instance.
 *
 * @param[in,out] instance pointer to the Canvas instance to be deleted
 */
void canvas_free(Canvas* instance);

/**
 * @brief Set the color for fill operations.
 *
 * @param[in,out] instance pointer to the Canvas instance to be modified
 * @param[in] color Color to be used as fill color
 */
void canvas_set_fill_color(Canvas* instance, Color color);

/**
 * @brief Set the opacity for fill operations.
 *
 * @param[in,out] instance pointer to the Canvas instance to be modified
 * @param[in] opacity opacity value for fills (0 - transparent, 255 - fully opaque)
 */
void canvas_set_fill_opacity(Canvas* instance, uint8_t opacity);

/**
 * @brief Set the color for line drawing operations.
 *
 * @param[in,out] instance pointer to the Canvas instance to be modified
 * @param[in] color Color to be used as line color
 */
void canvas_set_line_color(Canvas* instance, Color color);

/**
 * @brief Set the line width for line drawing operations.
 *
 * @param[in,out] instance pointer to the Canvas instance to be modified
 * @param[in] width line width in pixels (0 - no lines)
 */
void canvas_set_line_width(Canvas* instance, int32_t width);

/**
 * @brief Set the opacity for line drawing operations.
 *
 * @param[in,out] instance pointer to the Canvas instance to be modified
 * @param[in] opacity opacity value for lines (0 - transparent, 255 - fully opaque)
 */
void canvas_set_line_opacity(Canvas* instance, uint8_t opacity);

/**
 * @brief Clear all canvas contents.
 *
 * Previously set drawing parameters will remain unchanged.
 *
 * @param[in,out] instance pointer to the Canvas instance to be cleared
 */
void canvas_clear(Canvas* instance);

/**
 * @brief Fill the canvas with solid color.
 *
 * @param[in,out] instance pointer to the Canvas instance to be filled
 */
void canvas_fill(Canvas* instance);

/**
 * @brief Draw a single pixel with the given coordinates and color.
 *
 * @note Drawing pixel-by pixel is relatively slow, prefer using
 *       drawing primitives if performance is required.
 *
 * @note The color of the pixel is given during the call, it is independent
 *       fron the set fill/line colors.
 *
 * @param[in,out] instance pointer to the Canvas instance to be drawn on
 * @param[in] x horizontal (x) coordinate of the pixel
 * @param[in] y vertical (y) coordinate of the pixel
 * @param[in] color color of the pixel
 */
void canvas_draw_pixel(Canvas* instance, int32_t x, int32_t y, Color color);

/**
 * @brief Draw a single segment between two points.
 *
 * @param[in,out] instance pointer to the Canvas instance to be drawn on
 * @param[in] x1 horizontal (x) coordinate of the starting point
 * @param[in] y1 vertical (y) coordinate of the starting point
 * @param[in] x2 horizontal (x) coordinate of the ending point
 * @param[in] y2 vertical (y) coordinate of the ending point
 */
void canvas_draw_line(Canvas* instance, int32_t x1, int32_t y1, int32_t x2, int32_t y2);

/**
 * @brief Draw a rectangle based on its top-left corner and dimensions.
 *
 * @param[in,out] instance pointer to the Canvas instance to be drawn on
 * @param[in] x1 horizontal (x) coordinate of the corner
 * @param[in] y1 vertical (y) coordinate of the corner
 * @param[in] w horizontal dimension of the rectangle
 * @param[in] h vertical dimension of the rectangle
 * @param[in] fill if true, fill the rectangle with the fill color, otherwise draw only its border
 */
void canvas_draw_rect(Canvas* instance, int32_t x, int32_t y, int32_t w, int32_t h, bool fill);

/**
 * @brief Draw a plain text label.
 *
 * @param[in,out] instance pointer to the Canvas instance to be drawn on
 * @param[in] x1 horizontal (x) offset of the text
 * @param[in] y1 vertical (y) offset of the text
 * @param[in] text zero-terminated string containing the text to be shown
 */
void canvas_draw_text(Canvas* instance, int32_t x, int32_t y, const char* text);

/**
 * @brief Draw a plain text label with printf-like formatting.
 *
 * @param[in,out] instance pointer to the Canvas instance to be drawn on
 * @param[in] x1 horizontal (x) offset of the text
 * @param[in] y1 vertical (y) offset of the text
 * @param[in] text zero-terminated format string
 * @param[in] ... variadic list of arguments according to the format string
 */
void canvas_draw_text_fmt(Canvas* instance, int32_t x, int32_t y, const char* fmt, ...)
    _ATTRIBUTE((__format__(__printf__, 4, 5)));

#ifdef __cplusplus
}
#endif
