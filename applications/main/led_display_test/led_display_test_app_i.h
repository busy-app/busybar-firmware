#pragma once

#include <furi.h>
#include <gui_lvgl/gui_lvgl.h>

typedef enum {
    LedDisplayTestAppEventTypeNextPattern,
    LedDisplayTestAppEventTypePrevPattern,
    LedDisplayTestAppEventTypeUpdateColor,
} LedDisplayTestAppEventType;

typedef struct {
    LedDisplayTestAppEventType type;
    uint8_t color_num;
} LedDisplayTestAppEvent;

typedef struct {
    FuriEventLoop* event_loop;
    FuriMessageQueue* event_queue;
    GuiLvgl* gui;
    lv_obj_t* back_label;
    FuriPubSubSubscription* input_events;
} LedDisplayTestApp;
