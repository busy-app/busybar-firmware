#pragma once

#include <furi.h>
#include <gui/gui.h>
#include <gui/modules/label.h>
#include <gui/modules/canvas.h>

#include <led_display/led_display.h>

#include "led_display_test.h"

typedef enum {
    LedDisplayTestAppEventNextPattern,
    LedDisplayTestAppEventPrevPattern,
    LedDisplayTestAppEventNextColor,
    LedDisplayTestAppEventPrevColor,
    LedDisplayTestAppEventTick,
    LedDisplayTestAppEventExit,
} LedDisplayTestAppEvent;

typedef struct {
    FuriEventLoop* event_loop;
    FuriMessageQueue* event_queue;
    FuriEventLoopTimer* timer;
    Gui* gui;
    GuiInputSubscription* input_events;

    // Back display
    Widget* app_window;
    Label* static_label;
    Label* pattern_label;
    Label* color_label;

    // Front display
    Canvas* canvas;

    LedDisplayTestPattern pattern;
    LedDisplayTestColor color;
} LedDisplayTestApp;
