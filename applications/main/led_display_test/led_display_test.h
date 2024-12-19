#pragma once

typedef enum {
    LedDisplayTestPatternChess,
    LedDisplayTestPatternLinesHorizontal,
    LedDisplayTestPatternLinesVertical,
    LedDisplayTestPatternFullFill,
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
