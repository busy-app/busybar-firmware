#include "widget.h"

#include <furi.h>
#include <lvgl.h>

#include <input/input.h>

#include <lvgl/src/core/lv_obj_private.h>
#include <lvgl/src/core/lv_obj_class_private.h>

#define WIDGET_CLASS       (&widget_lvgl_class)
#define IS_WIDGET_CLASS(w) (lv_obj_has_class((lv_obj_t*)(w), WIDGET_CLASS))

#define TO_LV_OBJ(w)   ((lv_obj_t*)(w))
#define TO_LV_COLOR(c) (*(lv_color_t*)(&c))

typedef bool (*WidgetInputFeedCallback)(Widget* instance, const InputEvent* event);

struct Widget {
    lv_obj_t base;
    WidgetInputFeedCallback input_feed_callback;
};

static_assert(offsetof(Widget, base) == 0);

extern const lv_obj_class_t widget_lvgl_class;

void widget_set_input_feed_callback(Widget* instance, WidgetInputFeedCallback callback);

bool widget_input(Widget* instance, const InputEvent* event);

void widget_scroll_event_callback(lv_event_t* event);
