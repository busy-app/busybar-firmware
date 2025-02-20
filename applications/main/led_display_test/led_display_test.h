#pragma once
#include <stddef.h>
#include <stdint.h>

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

void led_display_test_set(uint8_t* buff, LedDisplayTestPattern pattern, LedDisplayTestColor color);

const char* led_display_get_pattern_str(LedDisplayTestPattern pattern);

const char* led_display_get_color_str(LedDisplayTestColor color);

// size_t led_display_get_pattern_frame_time(LedDisplayTestPattern pattern);

// void led_display_test_advance_frame(LedDisplayTestPattern pattern);
