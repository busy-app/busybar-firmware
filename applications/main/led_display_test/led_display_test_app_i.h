#pragma once

#include <furi.h>
#include <gui_lvgl/gui_lvgl.h>
#include "led_display_test.h"
#include <led_display/led_display.h>

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
    GuiLvgl* gui;

    // Back screen
    lv_obj_t* static_label;
    lv_obj_t* pattern_label;
    lv_obj_t* color_label;

    // Front screen
    lv_obj_t* canvas;
    uint8_t canvas_buffer[DOT_MATRIX_BUF_SIZE];

    LedDisplayTestPattern pattern;
    LedDisplayTestColor color;
} LedDisplayTestApp;
