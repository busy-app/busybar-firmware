#pragma once

#include <furi.h>
#include <gui/gui.h>
#include <gui/modules/label.h>
#include <gui/modules/canvas.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    BackDisplayTestAppEventNextPattern,
    BackDisplayTestAppEventPrevPattern,
    BackDisplayTestAppEventNextColor,
    BackDisplayTestAppEventPrevColor,
    BackDisplayTestAppEventTick,
    BackDisplayTestAppEventExit,
} BackDisplayTestAppEvent;

typedef enum {
    BackDisplayTestPatternFill,
    BackDisplayTestPatternCheckerboard,
    BackDisplayTestPatternGradientHorizontal,
    BackDisplayTestPatternGradientVertical,

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

typedef struct {
    FuriEventLoop* event_loop;
    FuriMessageQueue* event_queue;
    FuriEventLoopTimer* timer;
    Gui* gui;

    // Back display
    Widget* app_window;
    Label* static_label;
    Label* pattern_label;

    // Front display
    Canvas* canvas;

    BackDisplayTestPattern current_pattern;
    BackDisplayTestColor current_color;
} BackDisplayTestApp;

#ifdef __cplusplus
}
#endif
