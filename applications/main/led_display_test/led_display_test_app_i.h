#pragma once

#include <furi.h>
#include <gui_lvgl/gui_lvgl.h>
#include "led_display_test.h"

#define LED_DISPLAY_CANVAS_BUFFER_SIZE (72 * 16 * 3)

typedef enum {
    LedDisplayTestAppEventTypeNextPattern,
    LedDisplayTestAppEventTypePrevPattern,
    LedDisplayTestAppEventTypeUpdateColor,
    LedDisplayTestAppEventTypeTick,
    LedDisplayTestAppEventTypeExit,
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

    lv_obj_t* canvas;
    uint8_t canvas_buffer[LED_DISPLAY_CANVAS_BUFFER_SIZE];

    LedDisplayTestPattern pattern;
    LedDisplayTestColor color;
} LedDisplayTestApp;
