#include "flex_box.h"

#include <gui/widget_i.h>

#define MY_CLASS (&flex_box_lvgl_class)

const lv_obj_class_t flex_box_lvgl_class;

// Public API

FlexBox* flex_box_alloc(Widget* parent) {
    furi_check(parent);

    lv_obj_t* obj = lv_obj_class_create_obj(MY_CLASS, TO_LV_OBJ(parent));
    lv_obj_class_init_obj(obj);

    FlexBox* instance = (FlexBox*)obj;
    return instance;
}

void flex_box_free(FlexBox* instance) {
    furi_check(instance);
    lv_obj_delete(TO_LV_OBJ(instance));
}

Widget* flex_box_get_base(FlexBox* instance) {
    furi_check(instance);
    return (Widget*)instance;
}

void flex_box_set_flow(FlexBox* instance, FlexBoxFlow flow) {
    furi_check(instance);
    furi_check(flow < FlexBoxFlowMax);

    lv_obj_set_flex_flow(TO_LV_OBJ(instance), (lv_flex_flow_t)flow);
}

void flex_box_set_align(FlexBox* instance, FlexBoxAlign main, FlexBoxAlign cross) {
    furi_check(instance);
    furi_check(main < FlexBoxAlignMax);
    furi_check(cross < FlexBoxAlignMax);

    lv_obj_t* obj = TO_LV_OBJ(instance);
    furi_check(lv_obj_get_style_layout(obj, LV_PART_MAIN) == LV_LAYOUT_FLEX);

    lv_obj_set_style_flex_main_place(obj, (lv_flex_align_t)main, LV_PART_MAIN);
    lv_obj_set_style_flex_cross_place(obj, (lv_flex_align_t)cross, LV_PART_MAIN);
}

void flex_box_set_spacing(FlexBox* instance, int32_t spacing) {
    furi_check(instance);

    lv_obj_t* obj = TO_LV_OBJ(instance);
    furi_check(lv_obj_get_style_layout(obj, LV_PART_MAIN) == LV_LAYOUT_FLEX);

    const lv_flex_flow_t flow = lv_obj_get_style_flex_flow(obj, LV_PART_MAIN);

    if(flow & LV_FLEX_FLOW_COLUMN) {
        lv_obj_set_style_pad_row(obj, spacing, LV_PART_MAIN);
    } else {
        lv_obj_set_style_pad_column(obj, spacing, LV_PART_MAIN);
    }
}

// LVGL class descriptor

const lv_obj_class_t flex_box_lvgl_class = {
    .base_class = &widget_lvgl_class,
    .name = "widget-flex-box",
    .width_def = LV_SIZE_CONTENT,
    .height_def = LV_SIZE_CONTENT,
};
