#include "flex_layout.h"

#include <gui/widget_i.h>

#define MY_CLASS (&flex_layout_lvgl_class)

const lv_obj_class_t flex_layout_lvgl_class;

static void flex_event_callback(const lv_obj_class_t* class_p, lv_event_t* e) {
    UNUSED(class_p);
    lv_result_t res = LV_RESULT_OK;
    res = lv_obj_event_base(MY_CLASS, e);
    if(res != LV_RESULT_OK) return;

    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_CHILD_CREATED) {
        lv_obj_t* target = lv_event_get_target_obj(e);

        if(!lv_obj_check_type(target, &flex_layout_lvgl_class)) return;

        lv_flex_flow_t flex =
            lv_obj_get_style_flex_flow(lv_event_get_current_target_obj(e), LV_PART_MAIN);

        lv_obj_t* param = lv_event_get_param(e);
        if(flex == LV_FLEX_FLOW_COLUMN) {
            lv_obj_set_width(param, LV_PCT(100));
        } else if(flex == LV_FLEX_FLOW_ROW) {
            lv_obj_set_height(param, LV_PCT(100));
        }
    }
}

// Public API

FlexLayout* flex_layout_alloc(Widget* parent, FlexLayoutType type) {
    furi_check(parent);
    furi_check(type < FlexLayoutTypeMax);

    lv_obj_t* obj = lv_obj_class_create_obj(MY_CLASS, (lv_obj_t*)parent);
    lv_obj_class_init_obj(obj);
    lv_obj_set_flex_flow(obj, (lv_flex_flow_t)type);

    FlexLayout* instance = (FlexLayout*)obj;
    return instance;
}

void flex_layout_free(FlexLayout* instance) {
    furi_check(instance);
    lv_obj_delete((lv_obj_t*)instance);
}

Widget* flex_layout_get_base(FlexLayout* instance) {
    furi_check(instance);
    return (Widget*)instance;
}

void flex_layout_set_spacing(FlexLayout* instance, int32_t spacing) {
    furi_check(instance);

    lv_obj_t* obj = (lv_obj_t*)instance;

    if(lv_obj_get_style_flex_flow(obj, LV_PART_MAIN) & LV_FLEX_FLOW_COLUMN) {
        lv_obj_set_style_pad_column(obj, spacing, LV_PART_MAIN);
    } else {
        lv_obj_set_style_pad_row(obj, spacing, LV_PART_MAIN);
    }
}

void flex_layout_set_wrap(FlexLayout* instance, bool wrap) {
    furi_check(instance);

    lv_obj_t* obj = (lv_obj_t*)instance;
    lv_flex_flow_t flow = lv_obj_get_style_flex_flow(obj, LV_PART_MAIN);

    if(wrap) {
        flow |= LV_FLEX_WRAP;
    } else {
        flow &= ~LV_FLEX_WRAP;
    }

    lv_obj_set_flex_flow(obj, flow);
}

void flex_layout_set_reverse(FlexLayout* instance, bool reverse) {
    furi_check(instance);

    lv_obj_t* obj = (lv_obj_t*)instance;
    lv_flex_flow_t flow = lv_obj_get_style_flex_flow(obj, LV_PART_MAIN);

    if(reverse) {
        flow |= LV_FLEX_REVERSE;
    } else {
        flow &= ~LV_FLEX_REVERSE;
    }

    lv_obj_set_flex_flow(obj, flow);
}

// LVGL class descriptor

const lv_obj_class_t flex_layout_lvgl_class = {
    .base_class = &widget_lvgl_class,
    .name = "widget-flex-layout",
    .event_cb = flex_event_callback,
    .width_def = LV_PCT(100),
    .height_def = LV_PCT(100),
};
