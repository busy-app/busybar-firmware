#include "widget_i.h"

#include <lvgl/src/core/lv_obj_class_private.h>

#define MY_CLASS (&lv_widget_class)

// Internal implementation

static void widget_obj_destructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    UNUSED(class_p);

    Widget* instance = (Widget*)obj;
    if(instance->deleted_callback) {
        instance->deleted_callback(instance->callback_context);
    }
}

// Public API

Widget* widget_alloc(Widget* parent) {
    furi_check(parent);

    lv_obj_t* obj = lv_obj_class_create_obj(MY_CLASS, (lv_obj_t*)parent);
    lv_obj_class_init_obj(obj);

    Widget* instance = (Widget*)obj;
    return instance;
}

void widget_free(Widget* instance) {
    furi_check(instance);
    lv_obj_delete(&instance->obj);
}

void widget_set_visible(Widget* instance, bool visible) {
    furi_check(instance);
    if(visible) {
        lv_obj_remove_flag((lv_obj_t*)instance, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_add_flag((lv_obj_t*)instance, LV_OBJ_FLAG_HIDDEN);
    }
}

void widget_set_width(Widget* instance, int32_t width) {
    furi_check(instance);
    lv_obj_set_width((lv_obj_t*)instance, width);
}

void widget_set_height(Widget* instance, int32_t height) {
    furi_check(instance);
    lv_obj_set_height((lv_obj_t*)instance, height);
}

void widget_set_size(Widget* instance, int32_t width, int32_t height) {
    furi_check(instance);
    lv_obj_set_size((lv_obj_t*)instance, width, height);
}

void widget_set_pos_x(Widget* instance, int32_t x) {
    furi_check(instance);
    lv_obj_set_x((lv_obj_t*)instance, x);
}

void widget_set_pos_y(Widget* instance, int32_t y) {
    furi_check(instance);
    lv_obj_set_y((lv_obj_t*)instance, y);
}

void widget_set_pos(Widget* instance, int32_t x, int32_t y) {
    furi_check(instance);
    lv_obj_set_pos((lv_obj_t*)instance, x, y);
}

void widget_move_to_foreground(Widget* instance) {
    furi_check(instance);
    lv_obj_move_foreground((lv_obj_t*)instance);
}

void widget_move_to_background(Widget* instance) {
    furi_check(instance);
    lv_obj_move_background((lv_obj_t*)instance);
}

// Private API

void widget_set_callbacks(
    Widget* instance,
    WidgetDeletedCallback deleted_callback,
    WidgetGroupChangedCallback group_changed_callback,
    void* context) {
    instance->deleted_callback = deleted_callback;
    instance->group_changed_callback = group_changed_callback;
    instance->callback_context = context;
}

lv_group_t* widget_get_current_group(const Widget* instance) {
    return instance->current_group;
}

void widget_set_current_group(Widget* instance, lv_group_t* group) {
    instance->current_group = group;

    if(instance->group_changed_callback) {
        instance->group_changed_callback(instance, instance->callback_context);
    }
}

// LVGL class descriptor

const lv_obj_class_t lv_widget_class = {
    .base_class = &lv_obj_class,
    .destructor_cb = widget_obj_destructor,
    .name = "widget",
    .width_def = LV_PCT(100),
    .height_def = LV_PCT(100),
    .instance_size = sizeof(Widget),
};
