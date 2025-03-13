#pragma once

#include <stddef.h>
#include <stdint.h>

#include <gui/modules/canvas.h>

typedef enum {
    LedDisplayTestPatternChess,
    LedDisplayTestPatternLinesHorizontal,
    LedDisplayTestPatternLinesVertical,
    LedDisplayTestPatternFullFill,
    LedDisplayTestPatternRectangulars,
    LedDisplayTestPatternCross,
    LedDisplayTestPatternFrame,

    LedDisplayTestPatternAnimFill,
    LedDisplayTestPatternAnimHalfFill,
    LedDisplayTestPatternAnimFill10Noise,
    LedDisplayTestPatternAnimFill25Noise,
    LedDisplayTestPatternAnimFill50Noise,

    LedDisplayTestPatternNum,
} LedDisplayTestPattern;

typedef enum {
    LedDisplayTestColorRed,
    LedDisplayTestColorGreen,
    LedDisplayTestColorBlue,
    LedDisplayTestColorYellow,
    LedDisplayTestColorCian,
    LedDisplayTestColorPurple,
    LedDisplayTestColorWhite,

    LedDisplayTestColorNum,
} LedDisplayTestColor;

void led_display_test_set(Canvas* canvas, LedDisplayTestPattern pattern, LedDisplayTestColor color);

const char* led_display_get_pattern_str(LedDisplayTestPattern pattern);

const char* led_display_get_color_str(LedDisplayTestColor color);
