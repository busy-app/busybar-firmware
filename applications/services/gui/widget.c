#include "widget_i.h"

#include <lvgl/src/core/lv_obj_class_private.h>

#define MY_CLASS WIDGET_CLASS

static bool widget_input_feed_default(Widget* instance, const InputEvent* event) {
    bool consumed = false;

    lv_obj_t* obj = TO_LV_OBJ(instance);

    if(lv_obj_get_scrollbar_mode(obj) != LV_SCROLLBAR_MODE_OFF) {
        const int32_t delta = lv_display_get_vertical_resolution(lv_obj_get_display(obj)) / 8;

        if(event->type == InputTypeShort) {
            if(event->key == InputKeyUp) {
                lv_obj_scroll_by_bounded(obj, 0, -delta, false);
                consumed = true;

            } else if(event->key == InputKeyDown) {
                lv_obj_scroll_by_bounded(obj, 0, delta, false);
                consumed = true;
            }
        }
    }

    return consumed;
}

// LVGL-specific code

static void widget_lvgl_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    UNUSED(class_p);
    lv_obj_set_scrollbar_mode(obj, LV_SCROLLBAR_MODE_OFF);
}

// Public API

Widget* widget_alloc(Widget* parent) {
    furi_check(parent);

    lv_obj_t* obj = lv_obj_class_create_obj(MY_CLASS, TO_LV_OBJ(parent));
    lv_obj_class_init_obj(obj);

    return (Widget*)obj;
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

void widget_set_ignore_layout(Widget* instance, bool ignore_layout) {
    furi_check(instance);
    lv_obj_update_flag(TO_LV_OBJ(instance), LV_OBJ_FLAG_IGNORE_LAYOUT, ignore_layout);
}

bool widget_does_ignore_layout(const Widget* instance) {
    furi_check(instance);
    return lv_obj_has_flag(TO_LV_OBJ(instance), LV_OBJ_FLAG_IGNORE_LAYOUT);
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
}

void widget_set_background_color(Widget* instance, Color color, float opacity) {
    furi_check(instance);
    lv_obj_set_style_bg_color((lv_obj_t*)instance, TO_LV_COLOR(color), LV_PART_MAIN);
    lv_obj_set_style_bg_opa((lv_obj_t*)instance, (lv_opa_t)(opacity * 255), LV_PART_MAIN);
}

void widget_set_padding(Widget* instance, int32_t left, int32_t right, int32_t top, int32_t bottom) {
    furi_check(instance);
    lv_obj_set_style_pad_left((lv_obj_t*)instance, left, LV_PART_MAIN);
    lv_obj_set_style_pad_right((lv_obj_t*)instance, right, LV_PART_MAIN);
    lv_obj_set_style_pad_top((lv_obj_t*)instance, top, LV_PART_MAIN);
    lv_obj_set_style_pad_bottom((lv_obj_t*)instance, bottom, LV_PART_MAIN);
}

void widget_set_margin(Widget* instance, int32_t left, int32_t right, int32_t top, int32_t bottom) {
    furi_check(instance);
    lv_obj_set_style_margin_left((lv_obj_t*)instance, left, LV_PART_MAIN);
    lv_obj_set_style_margin_right((lv_obj_t*)instance, right, LV_PART_MAIN);
    lv_obj_set_style_margin_top((lv_obj_t*)instance, top, LV_PART_MAIN);
    lv_obj_set_style_margin_bottom((lv_obj_t*)instance, bottom, LV_PART_MAIN);
}

void widget_set_blend_mode(Widget* instance, WidgetBlendMode blend_mode) {
    furi_check(instance);
    furi_check(blend_mode < WidgetBlendModesCount);
    lv_obj_set_style_blend_mode(TO_LV_OBJ(instance), (lv_blend_mode_t)blend_mode, LV_PART_MAIN);
}

WidgetBlendMode widget_get_blend_mode(const Widget* instance) {
    furi_check(instance);
    return (WidgetBlendMode)lv_obj_get_style_blend_mode(TO_LV_OBJ(instance), LV_PART_MAIN);
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

        if(!consumed) {
            consumed = widget_input_feed_default(instance, event);
        }

    } while(false);

    return consumed;
}

// LVGL class descriptor

const lv_obj_class_t widget_lvgl_class = {
    .base_class = &lv_obj_class,
    .constructor_cb = widget_lvgl_constructor,
    .name = "widget",
    .width_def = LV_PCT(100),
    .height_def = LV_PCT(100),
    .instance_size = sizeof(Widget),
};
