#pragma once

#include "widget.h"
#include "gui.h"

#include <furi.h>
#include <lvgl.h>

#include <input/input.h>

#include <lvgl/src/core/lv_obj_private.h>
#include <lvgl/src/core/lv_obj_class_private.h>

#include <font_registry/font_registry.h>

#define WIDGET_CLASS       (&widget_lvgl_class)
#define IS_WIDGET_CLASS(w) (lv_obj_has_class((lv_obj_t*)(w), WIDGET_CLASS))

#define TO_LV_OBJ(w)   ((lv_obj_t*)(w))
#define TO_LV_COLOR(c) (*(lv_color_t*)(&c))

typedef bool (*WidgetInputCallback)(Widget* obj, const InputEvent* event);
typedef void (*WidgetStyleCallback)(Widget* obj);

typedef struct {
    WidgetInputCallback input_callback;
    WidgetStyleCallback style_callbacks[GuiDisplayIdMax];
} WidgetClassData;

struct Widget {
    lv_obj_t base;
};

static_assert(offsetof(Widget, base) == 0);

/*
 * Widget class contract: every lv_obj_class_t descending from widget_lvgl_class
 * must set user_data to either NULL or a pointer to a WidgetClassData instance.
 * widget_input() and widget_style() read user_data to dispatch callbacks.
 */
extern const lv_obj_class_t widget_lvgl_class;

bool widget_input(Widget* instance, const InputEvent* event);
void widget_style(Widget* instance, GuiDisplayId display_id);
