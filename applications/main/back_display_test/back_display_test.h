#pragma once

#include <furi.h>
#include <gui/gui.h>
#include <gui/modules/label.h>
#include <gui/modules/canvas.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    BackDisplayTestPatternFill,
    BackDisplayTestPatternCheckerboard,
    BackDisplayTestPatternGradientHorizontal,
    BackDisplayTestPatternGradientVertical,
    BackDisplayTestPatternGradientReverseHorizontal,
    BackDisplayTestPatternGradientReverseVertical,
    BackDisplayTestPatternRand10,
    BackDisplayTestPatternRand10Grayscale,
    BackDisplayTestPatternRandLines,
    BackDisplayTestPatternRandSquares,

    BackDisplayTestPatternMax,
} BackDisplayTestPattern;

typedef enum {
    BackDisplayTestColor15,
    BackDisplayTestColor14,
    BackDisplayTestColor13,
    BackDisplayTestColor12,
    BackDisplayTestColor11,
    BackDisplayTestColor10,
    BackDisplayTestColor9,
    BackDisplayTestColor8,
    BackDisplayTestColor7,
    BackDisplayTestColor6,
    BackDisplayTestColor5,
    BackDisplayTestColor4,
    BackDisplayTestColor3,
    BackDisplayTestColor2,
    BackDisplayTestColor1,
    BackDisplayTestColor0,

    BackDisplayTestColorMax,
} BackDisplayTestColor;

#ifdef __cplusplus
}
#endif
