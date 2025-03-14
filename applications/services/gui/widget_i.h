#include "widget.h"

#include <furi.h>
#include <lvgl.h>

#include <lvgl/src/core/lv_obj_private.h>
#include <lvgl/src/core/lv_obj_class_private.h>

#define IS_WIDGET_CLASS(w) (lv_obj_has_class((lv_obj_t*)(w), &lv_widget_class))

typedef void (*WidgetDeletedCallback)(Widget* instance, void* context);
typedef bool (*WidgetInputFeedCallback)(Widget* instance, const InputEvent* event);

struct Widget {
    lv_obj_t obj;
    WidgetInputCallback input_callback;
    void* input_callback_context;
    WidgetDeletedCallback deleted_callback;
    void* deleted_callback_context;
    WidgetInputFeedCallback input_feed_callback;
};

static_assert(offsetof(Widget, obj) == 0);

extern const lv_obj_class_t lv_widget_class;

void widget_set_deleted_callback(Widget* instance, WidgetDeletedCallback callback, void* context);
void widget_set_input_feed_callback(Widget* instance, WidgetInputFeedCallback callback);

void widget_input(Widget* instance, const InputEvent* event);
