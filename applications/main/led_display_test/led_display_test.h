#pragma once

typedef enum {
    LedDisplayTestPatternChess,
    LedDisplayTestPatternLinesHorizontal,
    LedDisplayTestPatternLinesVertical,
    LedDisplayTestPatternFullFill,
    LedDisplayTestPatternRectangulars,
    LedDisplayTestPatternCross,
    LedDisplayTestPatternFrame,

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

void led_display_test_set(LedDisplayTestPattern pattern, LedDisplayTestColor color);

const char* led_display_get_pattern_str(LedDisplayTestPattern pattern);

const char* led_display_get_color_str(LedDisplayTestColor color);
