#pragma once

#include "gui.h"

#include "widget_i.h"

#include <furi.h>
#include <lvgl.h>

#include <front_display/front_display.h>
#include <back_display/back_display.h>

#include <power/power_service/power.h>

#include <l10n/l10n.h>

#include <m-list.h>

#define FRONT_W                (FRONT_DISPLAY_W)
#define FRONT_H                (FRONT_DISPLAY_H)
#define FRONT_COLOR_FORMAT     (LV_COLOR_FORMAT_RGB888)
#define FRONT_BYTES_PER_PIXEL  (LV_COLOR_FORMAT_GET_SIZE(FRONT_COLOR_FORMAT))
#define FRONT_DRAW_BUFFER_SIZE (FRONT_W * FRONT_H * FRONT_BYTES_PER_PIXEL)

#define BACK_COLOR_FORMAT    (LV_COLOR_FORMAT_L8)
#define BACK_BYTES_PER_PIXEL (LV_COLOR_FORMAT_GET_SIZE(BACK_COLOR_FORMAT))

#define BACK_STATUS_BAR_WIDTH (12)

#define TICK_PERIOD_MS (8)

typedef struct {
    lv_display_t* lv_display;
    uint8_t* draw_buffer;
    void* driver;
} GuiDisplay;

typedef struct {
    GuiInputCallback callback;
    void* context;
} GuiInputItem;

LIST_DEF(GuiInputItemList, GuiInputItem, M_POD_OPLIST);

struct GuiLayer {
    GuiInputItemList_t input_list;
    lv_obj_t* root_objs[GuiDisplayIdMax];
};

struct Gui {
    Power* power;
    FuriEventLoop* event_loop;
    FuriMessageQueue* input_queue;
    GuiDisplay displays[GuiDisplayIdMax];
    GuiLayer layers[GuiLayerIdMax];
    L10nSrv* l10n_srv;
    L10nContext* l10n;
};

/**
 * @brief Gets the GUI localization context
 */
L10nContext* gui_get_l10n_context(Gui* gui);
