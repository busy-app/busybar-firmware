#include "back_display_test_patterns.h"

static void back_display_pattern_update_fill(Canvas* canvas, Color color) {
    canvas_set_fill_color(canvas, color);
    canvas_fill(canvas);
}

static void back_display_pattern_update_checkerboard(Canvas* canvas, Color color) {
    canvas_set_line_width(canvas, 0);
    canvas_set_fill_color(canvas, color);
    Widget* widget = canvas_get_base(canvas);
    const size_t checker_size = 16;
    for(int32_t y = 0; y < widget_get_height(widget); y += checker_size) {
        for(int32_t x = 0; x < widget_get_width(widget); x += checker_size) {
            if((x / checker_size + y / checker_size) % 2 == 0) {
                canvas_draw_rect(canvas, x, y, checker_size, checker_size, true);
            }
        }
    }
}

static void back_display_pattern_update_gradient_horizontal(Canvas* canvas, Color color) {
    UNUSED(color);
    Widget* widget = canvas_get_base(canvas);
    canvas_set_line_width(canvas, 0);

    for(int32_t i = 0; i < 16; i++) {
        uint8_t intensity = 255 * i / 15;
        Color test_color = COLOR_MAKE_RGB(intensity, intensity, intensity);
        canvas_set_fill_color(canvas, test_color);
        canvas_draw_rect(
            canvas,
            i * widget_get_width(widget) / 16,
            0,
            widget_get_width(widget) / 16,
            widget_get_height(widget),
            true);
    }
}

static void back_display_pattern_update_gradient_vertical(Canvas* canvas, Color color) {
    UNUSED(color);
    Widget* widget = canvas_get_base(canvas);
    canvas_set_line_width(canvas, 0);

    for(int32_t i = 0; i < 16; i++) {
        uint8_t intensity = 255 * i / 15;
        Color test_color = COLOR_MAKE_RGB(intensity, intensity, intensity);
        canvas_set_fill_color(canvas, test_color);
        canvas_draw_rect(
            canvas,
            0,
            i * widget_get_height(widget) / 16,
            widget_get_width(widget),
            widget_get_height(widget) / 16,
            true);
    }
}

typedef struct {
    void (*draw)(Canvas* canvas, Color color);
    const char* name;
    uint8_t color;
} BackDisplayTestPatternInfo;

static const BackDisplayTestPatternInfo back_display_test_patterns[BackDisplayPatternMax] = {
    [BackDisplayPatternFillWhite] = {back_display_pattern_update_fill, "Fill White", 255},
    [BackDisplayPatternFillBlack] = {back_display_pattern_update_fill, "Fill Black", 0},
    [BackDisplayPatternFillGray50] = {back_display_pattern_update_fill, "Fill Gray 50", 128},
    [BackDisplayPatternCheckerboard] =
        {back_display_pattern_update_checkerboard, "Checkerboard", 255},
    [BackDisplayPatternGradientHorizontal] =
        {back_display_pattern_update_gradient_horizontal, "Gradient Horizontal", 255},
    [BackDisplayPatternGradientVertical] =
        {back_display_pattern_update_gradient_vertical, "Gradient Vertical", 255},
};

void back_display_pattern_update(Canvas* canvas, BackDisplayPattern pattern) {
    furi_check(pattern < BackDisplayPatternMax);

    canvas_draw_begin(canvas);
    canvas_clear(canvas);
    canvas_set_line_width(canvas, 1);
    uint8_t intensity = back_display_test_patterns[pattern].color;
    Color test_color = COLOR_MAKE_RGB(intensity, intensity, intensity);
    back_display_test_patterns[pattern].draw(canvas, test_color);

    canvas_draw_end(canvas);
}

void back_display_pattern_to_string(BackDisplayPattern pattern, FuriString* str) {
    furi_check(pattern < BackDisplayPatternMax);

    furi_string_printf(
        str,
        "%s\n%u%%",
        back_display_test_patterns[pattern].name,
        back_display_test_patterns[pattern].color * 100 / 255);
}
