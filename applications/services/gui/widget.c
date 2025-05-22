#include "widget_i.h"

#include <lvgl/src/core/lv_obj_class_private.h>

#define MY_CLASS WIDGET_CLASS

static bool widget_input_callback(Widget* widget, const InputEvent* event) {
    lv_obj_t* obj = (lv_obj_t*)widget;

    if(lv_obj_get_scrollbar_mode(obj) != LV_SCROLLBAR_MODE_OFF) {
        const int32_t delta = 10;
        const bool anim = false;
        if(event->type == InputTypeShort) {
            if(event->key == InputKeyUp) {
                lv_obj_scroll_by_bounded(obj, -delta, -delta, anim);
            } else if(event->key == InputKeyDown) {
                lv_obj_scroll_by_bounded(obj, delta, delta, anim);
            }
        }
    }

    return false;
}

// Public API

Widget* widget_alloc(Widget* parent) {
    furi_check(parent);

    lv_obj_t* obj = lv_obj_class_create_obj(MY_CLASS, (lv_obj_t*)parent);
    lv_obj_class_init_obj(obj);

    Widget* instance = (Widget*)obj;
    widget_set_scrollbar_mode(instance, WidgetScrollBarModeOff);
    return instance;
}

void widget_free(Widget* instance) {
    furi_check(instance);
    lv_obj_delete((lv_obj_t*)instance);
}

void widget_set_visible(Widget* instance, bool visible) {
    furi_check(instance);
    if(visible) {
        lv_obj_remove_flag((lv_obj_t*)instance, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_add_flag((lv_obj_t*)instance, LV_OBJ_FLAG_HIDDEN);
    }
}

bool widget_is_visible(const Widget* instance) {
    furi_check(instance);
    return !lv_obj_has_flag(TO_LV_OBJ(instance), LV_OBJ_FLAG_HIDDEN);
}

void widget_set_width(Widget* instance, int32_t width) {
    furi_check(instance);
    lv_obj_set_width((lv_obj_t*)instance, width);
}

int32_t widget_get_width(const Widget* instance) {
    furi_check(instance);
    return lv_obj_get_width((const lv_obj_t*)instance);
}

void widget_set_height(Widget* instance, int32_t height) {
    furi_check(instance);
    lv_obj_set_height((lv_obj_t*)instance, height);
}

int32_t widget_get_height(const Widget* instance) {
    furi_check(instance);
    return lv_obj_get_height((const lv_obj_t*)instance);
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

void widget_set_align(Widget* instance, Align align) {
    furi_check(instance);
    furi_check(align < AlignMax);
    lv_obj_set_align((lv_obj_t*)instance, (lv_align_t)align);
}

void widget_move_to_foreground(Widget* instance) {
    furi_check(instance);
    lv_obj_move_foreground((lv_obj_t*)instance);
}

void widget_move_to_background(Widget* instance) {
    furi_check(instance);
    lv_obj_move_background((lv_obj_t*)instance);
}

void widget_set_scrollbar_mode(Widget* instance, WidgetScrollBarMode scrollbar_mode) {
    furi_check(instance);
    furi_check(scrollbar_mode < WidgetScrollBarModeCount);
    lv_obj_set_scrollbar_mode((lv_obj_t*)instance, (lv_scrollbar_mode_t)scrollbar_mode);
    widget_set_input_feed_callback(
        (Widget*)instance,
        scrollbar_mode == WidgetScrollBarModeOff ? NULL : widget_input_callback);
}

void widget_set_flex_grow(Widget* instance, uint8_t grow) {
    furi_check(instance);
    lv_obj_set_flex_grow((lv_obj_t*)instance, grow);
}
// Private API

void widget_set_input_feed_callback(Widget* instance, WidgetInputFeedCallback callback) {
    instance->input_feed_callback = callback;
}

bool widget_input(Widget* instance, const InputEvent* event) {
    bool consumed = false;

    do {
        if(lv_obj_has_flag((lv_obj_t*)instance, LV_OBJ_FLAG_HIDDEN)) {
            break;
        }

        if(instance->input_feed_callback) {
            consumed = instance->input_feed_callback(instance, event);

            if(consumed) {
                break;
            }
        }

        const uint32_t child_count = lv_obj_get_child_count((lv_obj_t*)instance);

        for(uint32_t i = 0; i < child_count; ++i) {
            lv_obj_t* child = lv_obj_get_child((lv_obj_t*)instance, i);

            if(IS_WIDGET_CLASS(child)) {
                // Recursion should not be a problem
                // when the widget tree is not too deep
                if(widget_input((Widget*)child, event)) {
                    consumed = true;
                }
            }
        }

    } while(false);

    return consumed;
}

// LVGL class descriptor

const lv_obj_class_t widget_lvgl_class = {
    .base_class = &lv_obj_class,
    .name = "widget",
    .width_def = LV_PCT(100),
    .height_def = LV_PCT(100),
    .instance_size = sizeof(Widget),
};
