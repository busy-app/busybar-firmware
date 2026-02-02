#pragma once

#include <stddef.h>
#include <stdint.h>

#include <gui/modules/canvas.h>

typedef enum {
    FrontDisplayTestPatternChess,
    FrontDisplayTestPatternLinesHorizontal,
    FrontDisplayTestPatternLinesVertical,
    FrontDisplayTestPatternFullFill,
    FrontDisplayTestPatternRectangulars,
    FrontDisplayTestPatternCross,
    FrontDisplayTestPatternFrame,

    FrontDisplayTestPatternGradientFull,
    FrontDisplayTestPatternGradientLow,
    FrontDisplayTestPatternAnimFill,
    FrontDisplayTestPatternAnimHalfFill,
    FrontDisplayTestPatternAnimFill10Noise,
    FrontDisplayTestPatternAnimFill25Noise,
    FrontDisplayTestPatternAnimFill50Noise,

    FrontDisplayTestPatternNum,
} FrontDisplayTestPattern;

typedef enum {
    FrontDisplayTestColorRed,
    FrontDisplayTestColorGreen,
    FrontDisplayTestColorBlue,
    FrontDisplayTestColorYellow,
    FrontDisplayTestColorCian,
    FrontDisplayTestColorPurple,
    FrontDisplayTestColorWhite,

    FrontDisplayTestColorNum,
} FrontDisplayTestColor;

void front_display_test_set(
    Canvas* canvas,
    FrontDisplayTestPattern pattern,
    FrontDisplayTestColor color);

const char* front_display_get_pattern_str(FrontDisplayTestPattern pattern);

const char* front_display_get_color_str(FrontDisplayTestColor color);
