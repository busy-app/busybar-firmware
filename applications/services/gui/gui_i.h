#pragma once

#include "gui.h"

#include "widget_i.h"

#include <furi.h>
#include <lvgl.h>

#include <ssd1320/ssd1320.h>

#include <input/input.h>
#include <power/power_service/power.h>

#include <led_display/led_display.h>

#include <m-list.h>

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

LIST_DEF(WidgetList, Widget*, M_PTR_OPLIST);

typedef struct {
    lv_display_t* lv_display;
    uint8_t* draw_buffer;
    // TODO: Keep frame buffer in SSD1320 service
    uint8_t* frame_buffer;
    void* driver;
    WidgetList_t active_widgets;
} GuiDisplay;

struct Gui {
    Power* power;
    FuriEventLoop* event_loop;
    FuriMessageQueue* input_queue;
    FuriMutex* access_mutex;
    DotMatrixSrv* dot_matrix;
    GuiDisplay displays[GuiDisplayIdMax];
};
