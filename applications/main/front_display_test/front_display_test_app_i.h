#pragma once

#include <furi.h>
#include <gui/gui.h>
#include <gui/modules/flex_layout.h>
#include <gui/modules/label.h>
#include <gui/modules/canvas.h>

#include <front_display/front_display.h>

#include "front_display_test.h"

typedef enum {
    FrontDisplayTestAppEventNextPattern,
    FrontDisplayTestAppEventPrevPattern,
    FrontDisplayTestAppEventNextColor,
    FrontDisplayTestAppEventPrevColor,
    FrontDisplayTestAppEventTick,
    FrontDisplayTestAppEventExit,
} FrontDisplayTestAppEvent;

typedef struct {
    FuriEventLoop* event_loop;
    FuriMessageQueue* event_queue;
    FuriEventLoopTimer* timer;
    Gui* gui;

    // Back display
    FlexLayout* flex;
    Label* static_label;
    Label* pattern_label;
    Label* color_label;

    // Front display
    Canvas* canvas;

    FrontDisplayTestPattern pattern;
    FrontDisplayTestColor color;
} FrontDisplayTestApp;
