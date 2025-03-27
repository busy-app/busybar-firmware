#pragma once

#include <furi.h>

#include <gui/gui.h>
#include <gui/modules/label.h>

#include <desktop/desktop.h>

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
    // Front display
    Label* front_label;
    // Back display
    Widget* back_window;
    Label* header_label;
    Label* content_label;
    Label* footer_label;
} InputTestApp;
