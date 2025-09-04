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
    back_display_test_canvas_update_fill_inverted(Canvas* canvas, BackDisplayTestColor color) {
    BackDisplayTestColor inverted_color = BackDisplayTestColor0 - color;
    canvas_set_fill_color(canvas, back_display_test_colors[inverted_color].color);
    canvas_fill(canvas);
}

static void
    back_display_test_canvas_update_checkerboard(Canvas* canvas, BackDisplayTestColor color) {
    canvas_set_line_width(canvas, 0);
    canvas_set_fill_color(canvas, back_display_test_colors[color].color);
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
    Widget* widget = canvas_get_base(canvas);
    canvas_set_line_width(canvas, 0);

    for(int32_t i = 0; i < 16; i++) {
        if(i > (int32_t)color - 1) {
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
}

static void
    back_display_test_canvas_update_gradient_vertical(Canvas* canvas, BackDisplayTestColor color) {
    Widget* widget = canvas_get_base(canvas);
    canvas_set_line_width(canvas, 0);

    for(int32_t i = 0; i < 16; i++) {
        if(i > (int32_t)color - 1) {
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
}

static void back_display_test_canvas_update_gradient_horizontal_reverse(
    Canvas* canvas,
    BackDisplayTestColor color) {
    Widget* widget = canvas_get_base(canvas);
    canvas_set_line_width(canvas, 0);

    for(int32_t i = 0; i < 16; i++) {
        if((15 - i) > (int32_t)color) {
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
}

static void back_display_test_canvas_update_gradient_vertical_reverse(
    Canvas* canvas,
    BackDisplayTestColor color) {
    Widget* widget = canvas_get_base(canvas);
    canvas_set_line_width(canvas, 0);

    for(int32_t i = 0; i < 16; i++) {
        if((15 - i) > (int32_t)color) {
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
}

static void back_display_test_canvas_update_rand(Canvas* canvas, BackDisplayTestColor color) {
    Widget* widget = canvas_get_base(canvas);
    const size_t pixel_count = widget_get_width(widget) * widget_get_height(widget);
    const size_t pixel_set_count = pixel_count / 10;

    for(size_t i = 0; i < pixel_set_count; i++) {
        int32_t x = rand() % widget_get_width(widget);
        int32_t y = rand() % widget_get_height(widget);
        canvas_draw_pixel(canvas, x, y, back_display_test_colors[color].color);
    }
}

static void
    back_display_test_canvas_update_rand_grayscale(Canvas* canvas, BackDisplayTestColor color) {
    Widget* widget = canvas_get_base(canvas);
    const size_t pixel_count = widget_get_width(widget) * widget_get_height(widget);
    const size_t pixel_set_count = pixel_count / 10;

    for(size_t i = 0; i < pixel_set_count; i++) {
        int32_t x = rand() % widget_get_width(widget);
        int32_t y = rand() % widget_get_height(widget);
        uint32_t pixel_color = rand() % back_display_test_colors[color].color.r;
        canvas_draw_pixel(
            canvas, x, y, (Color){.r = pixel_color, .g = pixel_color, .b = pixel_color});
    }
}

static void
    back_display_test_canvas_update_rand_lines(Canvas* canvas, BackDisplayTestColor color) {
    Widget* widget = canvas_get_base(canvas);
    canvas_set_line_color(canvas, back_display_test_colors[color].color);

    for(int32_t i = 0; i < 30; i++) {
        int32_t x1 = rand() % widget_get_width(widget);
        int32_t y1 = rand() % widget_get_height(widget);
        int32_t x2 = rand() % widget_get_width(widget);
        int32_t y2 = rand() % widget_get_height(widget);

        canvas_draw_line(canvas, x1, y1, x2, y2);
    }
}

static void
    back_display_test_canvas_update_rand_squares(Canvas* canvas, BackDisplayTestColor color) {
    Widget* widget = canvas_get_base(canvas);
    canvas_set_fill_color(canvas, back_display_test_colors[color].color);

    for(int32_t i = 0; i < 10; i++) {
        int32_t x = rand() % widget_get_width(widget);
        int32_t y = rand() % widget_get_height(widget);
        int32_t size = rand() % 20;

        canvas_draw_rect(canvas, x, y, size, size, true);
    }
}
static void
    back_display_test_canvas_update_tearing_vertical(Canvas* canvas, BackDisplayTestColor color) {
    Widget* widget = canvas_get_base(canvas);
    canvas_set_line_color(canvas, back_display_test_colors[color].color);

    static int32_t x = 0;

    canvas_draw_rect(canvas, x, 0, 10, widget_get_height(widget), true);

    x += 5;
    if(x >= widget_get_width(widget)) {
        x = 0;
    }
}

static void
    back_display_test_canvas_update_tearing_horizontal(Canvas* canvas, BackDisplayTestColor color) {
    Widget* widget = canvas_get_base(canvas);
    canvas_set_line_color(canvas, back_display_test_colors[color].color);

    static int32_t y = 0;

    canvas_draw_rect(canvas, 0, y, widget_get_width(widget), 10, true);

    y += 5;
    if(y >= widget_get_height(widget)) {
        y = 0;
    }
}

typedef struct {
    void (*draw)(Canvas* canvas, BackDisplayTestColor color);
    const char* name;
} BackDisplayTestPatternInfo;

static const BackDisplayTestPatternInfo back_display_test_patterns[BackDisplayTestPatternMax] = {
    [BackDisplayTestPatternFill] = {back_display_test_canvas_update_fill, "Fill"},
    [BackDisplayTestPatternFillInverted] =
        {back_display_test_canvas_update_fill_inverted, "Fill inverted"},
    [BackDisplayTestPatternCheckerboard] =
        {back_display_test_canvas_update_checkerboard, "Checker"},
    [BackDisplayTestPatternGradientHorizontal] =
        {back_display_test_canvas_update_gradient_horizontal, "Gradient H"},
    [BackDisplayTestPatternGradientVertical] =
        {back_display_test_canvas_update_gradient_vertical, "Gradient V"},
    [BackDisplayTestPatternGradientReverseHorizontal] =
        {back_display_test_canvas_update_gradient_horizontal_reverse, "Gradient HR"},
    [BackDisplayTestPatternGradientReverseVertical] =
        {back_display_test_canvas_update_gradient_vertical_reverse, "Gradient VR"},
    [BackDisplayTestPatternRand10] = {back_display_test_canvas_update_rand, "Rand"},
    [BackDisplayTestPatternRand10Grayscale] =
        {back_display_test_canvas_update_rand_grayscale, "Rand Gr"},
    [BackDisplayTestPatternRandLines] = {back_display_test_canvas_update_rand_lines, "Rand Lines"},
    [BackDisplayTestPatternRandSquares] =
        {back_display_test_canvas_update_rand_squares, "Rand Squares"},

    [BackDisplayTestPatternTearingVertical] =
        {back_display_test_canvas_update_tearing_vertical, "Tearing V"},
    [BackDisplayTestPatternTearingHorizontal] =
        {back_display_test_canvas_update_tearing_horizontal, "Tearing H"},
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
