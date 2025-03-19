#include "back_display_test_canvas.h"

typedef struct {
    Color color;
    const char* name;
} BackDisplayTestColorInfo;

// BackDisplayTestColor15 is white, BackDisplayTestColor14 is darker on 1/15th of the scale, etc.
static const BackDisplayTestColorInfo back_display_test_colors[BackDisplayTestColorMax] = {
    [BackDisplayTestColor15] = {{.r = 255, .g = 255, .b = 255}, "15"},
    [BackDisplayTestColor14] = {{.r = 238, .g = 238, .b = 238}, "14"},
    [BackDisplayTestColor13] = {{.r = 221, .g = 221, .b = 221}, "13"},
    [BackDisplayTestColor12] = {{.r = 204, .g = 204, .b = 204}, "12"},
    [BackDisplayTestColor11] = {{.r = 187, .g = 187, .b = 187}, "11"},
    [BackDisplayTestColor10] = {{.r = 170, .g = 170, .b = 170}, "10"},
    [BackDisplayTestColor9] = {{.r = 153, .g = 153, .b = 153}, "9"},
    [BackDisplayTestColor8] = {{.r = 136, .g = 136, .b = 136}, "8"},
    [BackDisplayTestColor7] = {{.r = 119, .g = 119, .b = 119}, "7"},
    [BackDisplayTestColor6] = {{.r = 102, .g = 102, .b = 102}, "6"},
    [BackDisplayTestColor5] = {{.r = 85, .g = 85, .b = 85}, "5"},
    [BackDisplayTestColor4] = {{.r = 68, .g = 68, .b = 68}, "4"},
    [BackDisplayTestColor3] = {{.r = 51, .g = 51, .b = 51}, "3"},
    [BackDisplayTestColor2] = {{.r = 34, .g = 34, .b = 34}, "2"},
    [BackDisplayTestColor1] = {{.r = 17, .g = 17, .b = 17}, "1"},
    [BackDisplayTestColor0] = {{.r = 0, .g = 0, .b = 0}, "0"},
};

static void back_display_test_canvas_update_fill(Canvas* canvas, BackDisplayTestColor color) {
    canvas_set_fill_color(canvas, back_display_test_colors[color].color);
    canvas_fill(canvas);
}

static void
    back_display_test_canvas_update_checkerboard(Canvas* canvas, BackDisplayTestColor color) {
    canvas_set_fill_color(canvas, back_display_test_colors[color].color);
    canvas_fill(canvas);

    canvas_set_line_width(canvas, 0);
    canvas_set_fill_color(canvas, (Color){.r = 0, .g = 0, .b = 0});
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

static void back_display_test_canvas_update_gradient_horizontal(
    Canvas* canvas,
    BackDisplayTestColor color) {
    UNUSED(color);
    Widget* widget = canvas_get_base(canvas);
    canvas_set_line_width(canvas, 0);

    for(int32_t i = 0; i < 16; i++) {
        canvas_set_fill_color(canvas, back_display_test_colors[i].color);
        canvas_draw_rect(
            canvas,
            i * widget_get_width(widget) / 16,
            0,
            widget_get_width(widget) / 16,
            widget_get_height(widget),
            true);
    }
}

static void
    back_display_test_canvas_update_gradient_vertical(Canvas* canvas, BackDisplayTestColor color) {
    UNUSED(color);
    Widget* widget = canvas_get_base(canvas);
    canvas_set_line_width(canvas, 0);

    for(int32_t i = 0; i < 16; i++) {
        canvas_set_fill_color(canvas, back_display_test_colors[i].color);
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
    void (*draw)(Canvas* canvas, BackDisplayTestColor color);
    const char* name;
} BackDisplayTestPatternInfo;

static const BackDisplayTestPatternInfo back_display_test_patterns[BackDisplayTestPatternMax] = {
    [BackDisplayTestPatternFill] = {back_display_test_canvas_update_fill, "Fill"},
    [BackDisplayTestPatternCheckerboard] =
        {back_display_test_canvas_update_checkerboard, "Checker"},
    [BackDisplayTestPatternGradientHorizontal] =
        {back_display_test_canvas_update_gradient_horizontal, "Gradient H"},
    [BackDisplayTestPatternGradientVertical] =
        {back_display_test_canvas_update_gradient_vertical, "Gradient V"},
};

void back_display_test_canvas_update(
    Canvas* canvas,
    BackDisplayTestPattern pattern,
    BackDisplayTestColor color) {
    furi_check(color < BackDisplayTestColorMax);
    furi_check(pattern < BackDisplayTestPatternMax);

    canvas_draw_begin(canvas);
    canvas_clear(canvas);
    canvas_set_line_width(canvas, 1);

    back_display_test_patterns[pattern].draw(canvas, color);

    canvas_draw_end(canvas);
}

const char* back_display_test_pattern_to_string(BackDisplayTestPattern pattern) {
    furi_check(pattern < BackDisplayTestPatternMax);

    return back_display_test_patterns[pattern].name;
}

const char* back_display_test_color_to_string(BackDisplayTestColor color) {
    furi_check(color < BackDisplayTestColorMax);

    return back_display_test_colors[color].name;
}
