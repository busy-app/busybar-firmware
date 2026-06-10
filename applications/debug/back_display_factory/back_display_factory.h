#pragma once

#include <furi.h>
#include <gui/gui.h>
#include <gui/modules/label.h>
#include <gui/modules/canvas.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    BackDisplayPatternFillWhite,
    BackDisplayPatternFillBlack,
    BackDisplayPatternFillGray25,
    BackDisplayPatternFillGray50,
    BackDisplayPatternFillGray75,
    BackDisplayPatternCheckerboard,
    BackDisplayPatternGradientHorizontal,
    BackDisplayPatternGradientVertical,
    BackDisplayPatternTearingVertical,
    BackDisplayPatternTearingHorizontal,

    BackDisplayPatternMax,
} BackDisplayPattern;

#ifdef __cplusplus
}
#endif
