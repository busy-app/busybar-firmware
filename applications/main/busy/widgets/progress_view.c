#include "progress_view.h"

#include <gui/widget_i.h>

#define MY_CLASS (&progress_view_lvgl_class)

#define COLOR_BG     (lv_color_hex(0x333333))
#define COLOR_NEXT   (lv_color_hex(0x999999))
#define COLOR_DONE   (lv_color_hex(0xFF2020))
#define COLOR_SLIDER (lv_color_hex(0xFF7778))

struct ProgressView {
    Widget base;
    lv_obj_t* progress_label;
    lv_obj_t* done_bar;
    lv_obj_t* next_bar;
    lv_obj_t* slider;
};

const lv_obj_class_t progress_view_lvgl_class;

// LVGL-specific code

static void progress_view_lvgl_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    UNUSED(class_p);

    lv_obj_set_flex_flow(obj, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_top(obj, 1, LV_PART_MAIN);
    lv_obj_set_style_pad_row(obj, 2, LV_PART_MAIN);

    ProgressView* instance = (ProgressView*)obj;

    lv_obj_t* top_layout = lv_obj_create(obj);
    lv_obj_set_size(top_layout, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(top_layout, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_left(top_layout, 2, LV_PART_MAIN);

    lv_obj_t* done_label = lv_label_create(top_layout);
    lv_label_set_text(done_label, "DONE");
    lv_obj_set_flex_grow(done_label, 1);
    lv_obj_set_style_text_color(done_label, lv_color_white(), LV_PART_MAIN);

    // TODO: Tickmark icon

    instance->progress_label = lv_label_create(top_layout);
    lv_obj_set_style_text_color(instance->progress_label, lv_color_white(), LV_PART_MAIN);

    lv_obj_t* bottom_layout = lv_obj_create(obj);
    lv_obj_set_size(bottom_layout, LV_PCT(100), 6);
    lv_obj_set_flex_flow(bottom_layout, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(
        bottom_layout, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    instance->done_bar = lv_obj_create(bottom_layout);
    lv_obj_set_size(instance->done_bar, 0, 4);
    lv_obj_set_style_bg_opa(instance->done_bar, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_bg_color(instance->done_bar, COLOR_DONE, LV_PART_MAIN);

    instance->slider = lv_obj_create(bottom_layout);
    lv_obj_set_size(instance->slider, 1, LV_PCT(100));
    lv_obj_set_style_bg_opa(instance->slider, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_bg_color(instance->slider, COLOR_SLIDER, LV_PART_MAIN);

    instance->next_bar = lv_obj_create(bottom_layout);
    lv_obj_set_size(instance->next_bar, 0, 4);
    lv_obj_set_style_bg_opa(instance->next_bar, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_bg_color(instance->next_bar, COLOR_NEXT, LV_PART_MAIN);

    lv_obj_t* fill = lv_obj_create(bottom_layout);
    lv_obj_set_size(fill, 0, 4);
    lv_obj_set_flex_grow(fill, 1);
    lv_obj_set_style_bg_opa(fill, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_bg_color(fill, COLOR_BG, LV_PART_MAIN);
}

// Implementation

// Public API

ProgressView* progress_view_alloc(Widget* parent) {
    furi_check(parent);

    lv_obj_t* obj = lv_obj_class_create_obj(MY_CLASS, (lv_obj_t*)parent);
    lv_obj_class_init_obj(obj);

    ProgressView* instance = (ProgressView*)obj;
    return instance;
}

void progress_view_free(ProgressView* instance) {
    furi_check(instance);
    lv_obj_delete((lv_obj_t*)instance);
}

Widget* progress_view_get_base(ProgressView* instance) {
    furi_check(instance);
    return (Widget*)instance;
}

void progress_view_set_progress(ProgressView* instance, uint32_t done, uint32_t total) {
    furi_check(instance);
    furi_check(done <= total);

    lv_label_set_text_fmt(instance->progress_label, "%lu/%lu", done, total);
    lv_obj_update_layout(TO_LV_OBJ(instance));

    const int32_t total_width = lv_obj_get_width(TO_LV_OBJ(instance));

    const int32_t done_width = done * total_width / total;
    const int32_t next_width = done < total ? done_width : 0;

    lv_obj_set_width(instance->done_bar, done_width - 1);
    lv_obj_set_width(instance->next_bar, next_width);
}

// LVGL class descriptor

const lv_obj_class_t progress_view_lvgl_class = {
    .base_class = &widget_lvgl_class,
    .constructor_cb = progress_view_lvgl_constructor,
    .name = "widget-progress-view",
    .width_def = LV_PCT(100),
    .height_def = LV_PCT(100),
    .instance_size = sizeof(ProgressView),
};
