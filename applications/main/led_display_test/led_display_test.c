#include "led_display_test.h"

#include <furi/furi.h>
#include <toolbox/color.h>

#include <led_display/led_display.h>

typedef struct {
    const char* name;
    const Color color;
} LedDisplayTestColorData;

static const LedDisplayTestColorData led_display_test_color[LedDisplayTestColorNum] = {
    [LedDisplayTestColorRed] =
        {
            .name = "Red",
            .color =
                {
                    .r = 0xff,
                    .g = 0x00,
                    .b = 0x00,
                },
        },
    [LedDisplayTestColorGreen] =
        {
            .name = "Green",
            .color =
                {
                    .r = 0x00,
                    .g = 0xff,
                    .b = 0x00,
                },
        },
    [LedDisplayTestColorBlue] =
        {
            .name = "Blue",
            .color =
                {
                    .r = 0x00,
                    .g = 0x00,
                    .b = 0xff,
                },
        },
    [LedDisplayTestColorYellow] =
        {
            .name = "Yellow",
            .color =
                {
                    .r = 0xff,
                    .g = 0xff,
                    .b = 0x00,
                },
        },
    [LedDisplayTestColorCian] =
        {
            .name = "Cyan",
            .color =
                {
                    .r = 0x00,
                    .g = 0xff,
                    .b = 0xff,
                },
        },
    [LedDisplayTestColorPurple] =
        {
            .name = "Purple",
            .color =
                {
                    .r = 0xff,
                    .g = 0x00,
                    .b = 0xff,
                },
        },
    [LedDisplayTestColorWhite] =
        {
            .name = "White",
            .color =
                {
                    .r = 0xff,
                    .g = 0xff,
                    .b = 0xff,
                },
        },
};

typedef void (*LedDisplayTestPatternSet)(Canvas* canvas, Color color);

typedef struct {
    LedDisplayTestPatternSet set;
    const char* name;
} LedDisplayTestPatternData;

static void led_display_test_set_pattern_chess(Canvas* canvas, Color color) {
    const int32_t rect_w = 4;

    canvas_set_fill_color(canvas, color);
    canvas_set_line_width(canvas, 0);

    for(int32_t x = 0; x < DOT_MATRIX_W; x += rect_w) {
        for(int32_t y = 0; y < DOT_MATRIX_H; y += rect_w) {
            if(((x / rect_w) + (y / rect_w)) % 2 == 0) {
                canvas_draw_rect(canvas, x, y, rect_w, rect_w, true);
            }
        }
    }
}

static void led_display_test_set_pattern_lines_horizontal(Canvas* canvas, Color color) {
    canvas_set_line_color(canvas, color);
    canvas_set_line_width(canvas, 1);

    for(int32_t y = 0; y < DOT_MATRIX_H; y += 2) {
        canvas_draw_line(canvas, 0, y, DOT_MATRIX_W, y);
    }
}

static void led_display_test_set_pattern_lines_vertical(Canvas* canvas, Color color) {
    canvas_set_line_color(canvas, color);
    canvas_set_line_width(canvas, 1);

    for(int32_t x = 0; x < DOT_MATRIX_W; x += 2) {
        canvas_draw_line(canvas, x, 0, x, DOT_MATRIX_H);
    }
}

static void led_display_test_set_pattern_full_fill(Canvas* canvas, Color color) {
    canvas_set_fill_color(canvas, color);
    canvas_fill(canvas);
}

static void led_display_test_set_pattern_rectangulars(Canvas* canvas, Color color) {
    const int32_t rect_count = 3;
    const int32_t rect_w = DOT_MATRIX_W / (rect_count * 2);

    canvas_set_fill_color(canvas, color);
    canvas_set_line_width(canvas, 0);

    for(int32_t x = 0; x < DOT_MATRIX_W; x += 2 * rect_w) {
        canvas_draw_rect(canvas, x, 0, rect_w, DOT_MATRIX_H, true);
    }
}

static size_t animation_frame = 0;

static void led_display_test_set_pattern_animated_rectangulars(Canvas* canvas, Color color) {
    const int32_t rect_count = 3;
    const int32_t rect_w = animation_frame++ % (DOT_MATRIX_W / rect_count);

    canvas_set_fill_color(canvas, color);
    canvas_set_line_width(canvas, 0);

    for(int32_t x = 0; x < DOT_MATRIX_W; x += DOT_MATRIX_W / rect_count) {
        canvas_draw_rect(canvas, x, 0, rect_w, DOT_MATRIX_H, true);
    }
}

static void led_display_test_set_pattern_animated_rectangulars_half(Canvas* canvas, Color color) {
    const int32_t rect_count = 3;
    const int32_t rect_w = (animation_frame++ * 2) % (DOT_MATRIX_W / rect_count);

    canvas_set_fill_color(canvas, color);
    canvas_set_line_width(canvas, 0);

    for(int32_t x = 0; x < DOT_MATRIX_W; x += DOT_MATRIX_W / rect_count) {
        canvas_draw_rect(canvas, x, 0, rect_w, DOT_MATRIX_H, true);
    }
}

static void led_display_test_set_pattern_animated_fill_10_noise(Canvas* canvas, Color color) {
    for(size_t x = 0; x < DOT_MATRIX_W; x++) {
        for(size_t y = 0; y < DOT_MATRIX_H; y++) {
            bool pixel_set = rand() % 10 == 0;

            if(pixel_set) {
                canvas_draw_pixel(canvas, x, y, color);
            }
        }
    }
}

static void led_display_test_set_pattern_animated_fill_25_noise(Canvas* canvas, Color color) {
    for(size_t x = 0; x < DOT_MATRIX_W; x++) {
        for(size_t y = 0; y < DOT_MATRIX_H; y++) {
            bool pixel_set = rand() % 4 == 0;

            if(pixel_set) {
                canvas_draw_pixel(canvas, x, y, color);
            }
        }
    }
}
static void led_display_test_set_pattern_animated_fill_50_noise(Canvas* canvas, Color color) {
    for(size_t x = 0; x < DOT_MATRIX_W; x++) {
        for(size_t y = 0; y < DOT_MATRIX_H; y++) {
            bool pixel_set = rand() % 2 == 0;

            if(pixel_set) {
                canvas_draw_pixel(canvas, x, y, color);
            }
        }
    }
}

static void led_display_test_set_pattern_cross(Canvas* canvas, Color color) {
    for(size_t x = 0; x < DOT_MATRIX_W; x++) {
        for(size_t y = 0; y < DOT_MATRIX_H; y++) {
            if((2 * x) - (9 * y) < 6) {
                canvas_draw_pixel(canvas, x, y, color);
                canvas_draw_pixel(canvas, x, DOT_MATRIX_H - 1 - y, color);
            }
        }
    }
}

static void led_display_test_set_pattern_frame(Canvas* canvas, Color color) {
    canvas_set_line_color(canvas, color);
    canvas_set_line_width(canvas, 1);

    canvas_draw_rect(canvas, 0, 0, DOT_MATRIX_W, DOT_MATRIX_H, false);
}

static const LedDisplayTestPatternData led_display_test_pattern[LedDisplayTestPatternNum] = {
    [LedDisplayTestPatternChess] =
        {
            .set = led_display_test_set_pattern_chess,
            .name = "Chess",
        },
    [LedDisplayTestPatternLinesHorizontal] =
        {
            .set = led_display_test_set_pattern_lines_horizontal,
            .name = "Horizontal Lines",
        },
    [LedDisplayTestPatternLinesVertical] =
        {
            .set = led_display_test_set_pattern_lines_vertical,
            .name = "Vertical Lines",
        },
    [LedDisplayTestPatternFullFill] =
        {
            .set = led_display_test_set_pattern_full_fill,
            .name = "Full Fill",
        },
    [LedDisplayTestPatternRectangulars] =
        {
            .set = led_display_test_set_pattern_rectangulars,
            .name = "Rectangulars",
        },
    [LedDisplayTestPatternCross] =
        {
            .set = led_display_test_set_pattern_cross,
            .name = "Cross",
        },
    [LedDisplayTestPatternFrame] =
        {
            .set = led_display_test_set_pattern_frame,
            .name = "Frame",
        },
    [LedDisplayTestPatternAnimFill] =
        {
            .set = led_display_test_set_pattern_animated_rectangulars,
            .name = "Animated Rectangulars Fill",
        },
    [LedDisplayTestPatternAnimHalfFill] =
        {
            .set = led_display_test_set_pattern_animated_rectangulars_half,
            .name = "Animated Rectangulars Half Fill",
        },
    [LedDisplayTestPatternAnimFill10Noise] =
        {
            .set = led_display_test_set_pattern_animated_fill_10_noise,
            .name = "Animated Fill 10% Noise",
        },
    [LedDisplayTestPatternAnimFill25Noise] =
        {
            .set = led_display_test_set_pattern_animated_fill_25_noise,
            .name = "Animated Fill 25% Noise",
        },
    [LedDisplayTestPatternAnimFill50Noise] =
        {
            .set = led_display_test_set_pattern_animated_fill_50_noise,
            .name = "Animated Fill 50% Noise",
        },
};

void led_display_test_set(Canvas* canvas, LedDisplayTestPattern pattern, LedDisplayTestColor color) {
    furi_check(canvas);
    furi_check(pattern < LedDisplayTestPatternNum);
    furi_check(color < LedDisplayTestColorNum);

    canvas_draw_begin(canvas);
    canvas_clear(canvas);
    led_display_test_pattern[pattern].set(canvas, led_display_test_color[color].color);
    canvas_draw_end(canvas);
}

const char* led_display_get_pattern_str(LedDisplayTestPattern pattern) {
    furi_check(pattern < LedDisplayTestPatternNum);

    return led_display_test_pattern[pattern].name;
}

const char* led_display_get_color_str(LedDisplayTestColor color) {
    furi_check(color < LedDisplayTestColorNum);

    return led_display_test_color[color].name;
}
