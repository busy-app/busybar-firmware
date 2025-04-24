#include "front_display_test.h"

#include <furi/furi.h>
#include <toolbox/color.h>

typedef struct {
    const char* name;
    const Color color;
} FrontDisplayTestColorData;

static const FrontDisplayTestColorData front_display_test_color[FrontDisplayTestColorNum] = {
    [FrontDisplayTestColorRed] =
        {
            .name = "Red",
            .color =
                {
                    .r = 0xff,
                    .g = 0x00,
                    .b = 0x00,
                },
        },
    [FrontDisplayTestColorGreen] =
        {
            .name = "Green",
            .color =
                {
                    .r = 0x00,
                    .g = 0xff,
                    .b = 0x00,
                },
        },
    [FrontDisplayTestColorBlue] =
        {
            .name = "Blue",
            .color =
                {
                    .r = 0x00,
                    .g = 0x00,
                    .b = 0xff,
                },
        },
    [FrontDisplayTestColorYellow] =
        {
            .name = "Yellow",
            .color =
                {
                    .r = 0xff,
                    .g = 0xff,
                    .b = 0x00,
                },
        },
    [FrontDisplayTestColorCian] =
        {
            .name = "Cyan",
            .color =
                {
                    .r = 0x00,
                    .g = 0xff,
                    .b = 0xff,
                },
        },
    [FrontDisplayTestColorPurple] =
        {
            .name = "Purple",
            .color =
                {
                    .r = 0xff,
                    .g = 0x00,
                    .b = 0xff,
                },
        },
    [FrontDisplayTestColorWhite] =
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

typedef void (*FrontDisplayTestPatternSet)(Canvas* canvas, Color color);

typedef struct {
    FrontDisplayTestPatternSet set;
    const char* name;
} FrontDisplayTestPatternData;

static void front_display_test_set_pattern_chess(Canvas* canvas, Color color) {
    const int32_t rect_w = 4;

    canvas_set_fill_color(canvas, color);
    canvas_set_line_width(canvas, 0);

    for(int32_t x = 0; x < widget_get_width(canvas_get_base(canvas)); x += rect_w) {
        for(int32_t y = 0; y < widget_get_height(canvas_get_base(canvas)); y += rect_w) {
            if(((x / rect_w) + (y / rect_w)) % 2 == 0) {
                canvas_draw_rect(canvas, x, y, rect_w, rect_w, true);
            }
        }
    }
}

static void front_display_test_set_pattern_lines_horizontal(Canvas* canvas, Color color) {
    canvas_set_line_color(canvas, color);
    canvas_set_line_width(canvas, 1);

    for(int32_t y = 0; y < widget_get_height(canvas_get_base(canvas)); y += 2) {
        canvas_draw_line(canvas, 0, y, widget_get_width(canvas_get_base(canvas)), y);
    }
}

static void front_display_test_set_pattern_lines_vertical(Canvas* canvas, Color color) {
    canvas_set_line_color(canvas, color);
    canvas_set_line_width(canvas, 1);

    for(int32_t x = 0; x < widget_get_width(canvas_get_base(canvas)); x += 2) {
        canvas_draw_line(canvas, x, 0, x, widget_get_height(canvas_get_base(canvas)));
    }
}

static void front_display_test_set_pattern_full_fill(Canvas* canvas, Color color) {
    canvas_set_fill_color(canvas, color);
    canvas_fill(canvas);
}

static void front_display_test_set_pattern_rectangulars(Canvas* canvas, Color color) {
    const int32_t rect_count = 3;
    const int32_t rect_w = widget_get_width(canvas_get_base(canvas)) / (rect_count * 2);

    canvas_set_fill_color(canvas, color);
    canvas_set_line_width(canvas, 0);

    for(int32_t x = 0; x < widget_get_width(canvas_get_base(canvas)); x += 2 * rect_w) {
        canvas_draw_rect(canvas, x, 0, rect_w, widget_get_height(canvas_get_base(canvas)), true);
    }
}

static size_t animation_frame = 0;

static void front_display_test_set_pattern_animated_rectangulars(Canvas* canvas, Color color) {
    const int32_t rect_count = 3;
    const int32_t rect_w =
        animation_frame++ % (widget_get_width(canvas_get_base(canvas)) / rect_count);

    canvas_set_fill_color(canvas, color);
    canvas_set_line_width(canvas, 0);

    for(int32_t x = 0; x < widget_get_width(canvas_get_base(canvas));
        x += widget_get_width(canvas_get_base(canvas)) / rect_count) {
        canvas_draw_rect(canvas, x, 0, rect_w, widget_get_height(canvas_get_base(canvas)), true);
    }
}

static void
    front_display_test_set_pattern_animated_rectangulars_half(Canvas* canvas, Color color) {
    const int32_t rect_count = 3;
    const int32_t rect_w =
        (animation_frame++ * 2) % (widget_get_width(canvas_get_base(canvas)) / rect_count);

    canvas_set_fill_color(canvas, color);
    canvas_set_line_width(canvas, 0);

    for(int32_t x = 0; x < widget_get_width(canvas_get_base(canvas));
        x += widget_get_width(canvas_get_base(canvas)) / rect_count) {
        canvas_draw_rect(canvas, x, 0, rect_w, widget_get_height(canvas_get_base(canvas)), true);
    }
}

static void front_display_test_set_pattern_animated_fill_10_noise(Canvas* canvas, Color color) {
    for(int32_t x = 0; x < widget_get_width(canvas_get_base(canvas)); x++) {
        for(int32_t y = 0; y < widget_get_height(canvas_get_base(canvas)); y++) {
            bool pixel_set = rand() % 10 == 0;

            if(pixel_set) {
                canvas_draw_pixel(canvas, x, y, color);
            }
        }
    }
}

static void front_display_test_set_pattern_animated_fill_25_noise(Canvas* canvas, Color color) {
    for(int32_t x = 0; x < widget_get_width(canvas_get_base(canvas)); x++) {
        for(int32_t y = 0; y < widget_get_height(canvas_get_base(canvas)); y++) {
            bool pixel_set = rand() % 4 == 0;

            if(pixel_set) {
                canvas_draw_pixel(canvas, x, y, color);
            }
        }
    }
}
static void front_display_test_set_pattern_animated_fill_50_noise(Canvas* canvas, Color color) {
    for(int32_t x = 0; x < widget_get_width(canvas_get_base(canvas)); x++) {
        for(int32_t y = 0; y < widget_get_height(canvas_get_base(canvas)); y++) {
            bool pixel_set = rand() % 2 == 0;

            if(pixel_set) {
                canvas_draw_pixel(canvas, x, y, color);
            }
        }
    }
}

static void front_display_test_set_pattern_cross(Canvas* canvas, Color color) {
    canvas_set_line_color(canvas, color);
    canvas_set_line_width(canvas, 1);

    canvas_draw_line(
        canvas,
        0,
        0,
        widget_get_width(canvas_get_base(canvas)) - 1,
        widget_get_height(canvas_get_base(canvas)) - 1);
    canvas_draw_line(
        canvas,
        0,
        widget_get_height(canvas_get_base(canvas)) - 1,
        widget_get_width(canvas_get_base(canvas)) - 1,
        0);
}

static void front_display_test_set_pattern_frame(Canvas* canvas, Color color) {
    canvas_set_line_color(canvas, color);
    canvas_set_line_width(canvas, 1);

    canvas_draw_rect(
        canvas,
        0,
        0,
        widget_get_width(canvas_get_base(canvas)),
        widget_get_height(canvas_get_base(canvas)),
        false);
}

static const FrontDisplayTestPatternData front_display_test_pattern[FrontDisplayTestPatternNum] = {
    [FrontDisplayTestPatternChess] =
        {
            .set = front_display_test_set_pattern_chess,
            .name = "Chess",
        },
    [FrontDisplayTestPatternLinesHorizontal] =
        {
            .set = front_display_test_set_pattern_lines_horizontal,
            .name = "Horizontal Lines",
        },
    [FrontDisplayTestPatternLinesVertical] =
        {
            .set = front_display_test_set_pattern_lines_vertical,
            .name = "Vertical Lines",
        },
    [FrontDisplayTestPatternFullFill] =
        {
            .set = front_display_test_set_pattern_full_fill,
            .name = "Full Fill",
        },
    [FrontDisplayTestPatternRectangulars] =
        {
            .set = front_display_test_set_pattern_rectangulars,
            .name = "Rectangulars",
        },
    [FrontDisplayTestPatternCross] =
        {
            .set = front_display_test_set_pattern_cross,
            .name = "Cross",
        },
    [FrontDisplayTestPatternFrame] =
        {
            .set = front_display_test_set_pattern_frame,
            .name = "Frame",
        },
    [FrontDisplayTestPatternAnimFill] =
        {
            .set = front_display_test_set_pattern_animated_rectangulars,
            .name = "Animated Rectangulars Fill",
        },
    [FrontDisplayTestPatternAnimHalfFill] =
        {
            .set = front_display_test_set_pattern_animated_rectangulars_half,
            .name = "Animated Rectangulars Half Fill",
        },
    [FrontDisplayTestPatternAnimFill10Noise] =
        {
            .set = front_display_test_set_pattern_animated_fill_10_noise,
            .name = "Animated Fill 10% Noise",
        },
    [FrontDisplayTestPatternAnimFill25Noise] =
        {
            .set = front_display_test_set_pattern_animated_fill_25_noise,
            .name = "Animated Fill 25% Noise",
        },
    [FrontDisplayTestPatternAnimFill50Noise] =
        {
            .set = front_display_test_set_pattern_animated_fill_50_noise,
            .name = "Animated Fill 50% Noise",
        },
};

void front_display_test_set(
    Canvas* canvas,
    FrontDisplayTestPattern pattern,
    FrontDisplayTestColor color) {
    furi_check(canvas);
    furi_check(pattern < FrontDisplayTestPatternNum);
    furi_check(color < FrontDisplayTestColorNum);

    canvas_draw_begin(canvas);
    canvas_clear(canvas);
    front_display_test_pattern[pattern].set(canvas, front_display_test_color[color].color);
    canvas_draw_end(canvas);
}

const char* front_display_get_pattern_str(FrontDisplayTestPattern pattern) {
    furi_check(pattern < FrontDisplayTestPatternNum);

    return front_display_test_pattern[pattern].name;
}

const char* front_display_get_color_str(FrontDisplayTestColor color) {
    furi_check(color < FrontDisplayTestColorNum);

    return front_display_test_color[color].name;
}
