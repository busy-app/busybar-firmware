#pragma once

#include <furi.h>
#include <desktop/desktop.h>
#include <gui/gui.h>
#include <gui/modules/label.h>
#include <gui/modules/canvas.h>

#include <light_sensor/light_sensor.h>

typedef enum {
    InputTestAppEventExit,
    InputTestAppEventKeyStateChanged,
} InputTestAppEventType;

typedef struct {
    InputTestAppEventType type;
    InputKey input_key;
} InputTestAppEvent;

typedef struct {
    uint32_t ok;
    uint32_t start;
    int32_t encoder;
    int32_t switch_pos;
} InputTestAppModel;

typedef struct {
    FuriEventLoop* event_loop;
    FuriMessageQueue* event_queue;
    FuriEventLoopTimer* timer;
    Gui* gui;
    Desktop* desktop;

    InputTestAppModel input_model;

    // Front screen
    Label* label_text;

    // Back screen
    Canvas* canvas;

    // TODO remove after canvas_draw_text_fmt() is fixed
    FuriString* lables_str[3];

} InputTestApp;
