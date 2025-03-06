#pragma once

#include "gui.h"

#include <furi.h>
#include <lvgl.h>

#include <ssd1320/ssd1320.h>

#include <input/input.h>
#include <power_simple/power.h>
#include <led_display/led_display.h>

#define FRONT_W                (DOT_MATRIX_W)
#define FRONT_H                (DOT_MATRIX_H)
#define FRONT_COLOR_FORMAT     (LV_COLOR_FORMAT_RGB888)
#define FRONT_BYTES_PER_PIXEL  (LV_COLOR_FORMAT_GET_SIZE(FRONT_COLOR_FORMAT))
#define FRONT_DRAW_BUFFER_SIZE (FRONT_W * FRONT_H * FRONT_BYTES_PER_PIXEL)

#define BACK_W                 (SSD1320_W)
#define BACK_H                 (SSD1320_H)
#define BACK_COLOR_FORMAT      (LV_COLOR_FORMAT_L8)
#define BACK_BYTES_PER_PIXEL   (LV_COLOR_FORMAT_GET_SIZE(BACK_COLOR_FORMAT))
#define BACK_DRAW_BUFFER_SIZE  (BACK_W * BACK_H * BACK_BYTES_PER_PIXEL)
#define BACK_FRAME_BUFFER_SIZE (SSD1320_BUF_SIZE)

#define TICK_PERIOD_MS (8)

typedef struct {
    GuiInputId id;
    union {
        struct {
            int8_t diff;
            lv_indev_state_t btn_state;
        } encoder;
        struct {
            uint8_t key;
            lv_indev_state_t state;
        } button;
    };
} GuiInputEvent;

typedef struct {
    lv_display_t* lv_display;
    lv_indev_t* lv_indevs[GuiInputIdMax];
    uint8_t* draw_buffer;
    // TODO: Keep frame buffer in SSD1320 service
    uint8_t* frame_buffer;
    void* driver;
} GuiDisplay;

struct Gui {
    Power* power;
    FuriEventLoop* event_loop;
    FuriMessageQueue* input_queue;
    FuriMutex* access_mutex;
    DotMatrixSrv* dot_matrix;
    GuiDisplay displays[GuiDisplayIdMax];
    GuiInputEvent input_event;
};
