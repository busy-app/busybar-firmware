#include "widget.h"

#include <furi.h>

#include <lvgl.h>
#include <lvgl/src/core/lv_obj_private.h>

#define IS_WIDGET_CLASS(w) (lv_obj_has_class((lv_obj_t*)(w), &lv_widget_class))

typedef void (*WidgetDeletedCallback)(void* context);
typedef void (*WidgetGroupChangedCallback)(Widget* widget, void* context);

struct Widget {
    lv_obj_t obj;
    lv_group_t* current_group;
    WidgetDeletedCallback deleted_callback;
    WidgetGroupChangedCallback group_changed_callback;
    void* callback_context;
};

static_assert(offsetof(Widget, obj) == 0);

extern const lv_obj_class_t lv_widget_class;

void widget_set_callbacks(
    Widget* instance,
    WidgetDeletedCallback deleted_callback,
    WidgetGroupChangedCallback group_changed_callback,
    void* context);

lv_group_t* widget_get_current_group(const Widget* instance);

void widget_set_current_group(Widget* instance, lv_group_t* group);
