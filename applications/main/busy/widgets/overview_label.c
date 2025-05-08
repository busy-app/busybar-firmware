#include "overview_label.h"

#include <gui/widget_i.h>

#define MY_CLASS (&overview_label_lvgl_class)

#define COLOR_DIM  (lv_color_hex(0x3F444D))
#define COLOR_BUSY (lv_color_hex(0xFF0000))
#define COLOR_REST (lv_color_hex(0x13F562))

typedef enum {
    OverviewLabelColumnIdxWork,
    OverviewLabelColumnIdxRest,
    OverviewLabelColumnIdxMax,
} OverviewLabelColumnIdx;

typedef struct {
    lv_obj_t* top_label;
    lv_obj_t* bottom_label;
} OverviewLabelColumn;

struct OverviewLabel {
    Widget base;
    OverviewLabelColumn columns[OverviewLabelColumnIdxMax];
};

const lv_obj_class_t overview_label_lvgl_class;

static lv_obj_t* overview_label_get_top(OverviewLabel* instance, OverviewLabelColumnIdx idx) {
    return instance->columns[idx].top_label;
}

static lv_obj_t* overview_label_get_bottom(OverviewLabel* instance, OverviewLabelColumnIdx idx) {
    return instance->columns[idx].bottom_label;
}

// LVGL-specific code

static void overview_label_lvgl_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    UNUSED(class_p);

    lv_obj_set_flex_flow(obj, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(obj, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    OverviewLabel* instance = (OverviewLabel*)obj;

    for(uint32_t i = 0; i < OverviewLabelColumnIdxMax; ++i) {
        OverviewLabelColumn* column = &instance->columns[i];

        lv_obj_t* layout = lv_obj_create(obj);
        lv_obj_set_size(layout, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
        lv_obj_set_flex_flow(layout, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_flex_align(
            layout, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_flex_grow(layout, 1);
        lv_obj_set_style_pad_row(layout, 1, LV_PART_MAIN);
        lv_obj_set_style_translate_y(layout, 1, LV_PART_MAIN);

        column->top_label = lv_label_create(layout);
        column->bottom_label = lv_label_create(layout);

        lv_obj_set_style_text_font(column->top_label, lv_theme_get_font_small(obj), LV_PART_MAIN);
        lv_obj_set_style_text_font(
            column->bottom_label, lv_theme_get_font_large(obj), LV_PART_MAIN);

        lv_obj_set_style_text_color(column->top_label, COLOR_DIM, LV_PART_MAIN);
        lv_obj_set_style_text_color(column->bottom_label, COLOR_DIM, LV_PART_MAIN);
    }

    lv_label_set_text(overview_label_get_top(instance, OverviewLabelColumnIdxWork), "WORK");
    lv_label_set_text(overview_label_get_top(instance, OverviewLabelColumnIdxRest), "REST");
}

// Implementation

// Public API

OverviewLabel* overview_label_alloc(Widget* parent) {
    furi_check(parent);

    lv_obj_t* obj = lv_obj_class_create_obj(MY_CLASS, (lv_obj_t*)parent);
    lv_obj_class_init_obj(obj);

    OverviewLabel* instance = (OverviewLabel*)obj;
    return instance;
}

void overview_label_free(OverviewLabel* instance) {
    furi_check(instance);
    lv_obj_delete((lv_obj_t*)instance);
}

Widget* overview_label_get_base(OverviewLabel* instance) {
    furi_check(instance);
    return (Widget*)instance;
}

void overview_label_set_intervals(
    OverviewLabel* instance,
    uint32_t work_time_mn,
    uint32_t rest_time_mn) {
    furi_check(instance);

    lv_obj_t* work_interval_label =
        overview_label_get_bottom(instance, OverviewLabelColumnIdxWork);
    lv_obj_t* rest_interval_label =
        overview_label_get_bottom(instance, OverviewLabelColumnIdxRest);

    // TODO: Proper time formatting
    lv_label_set_text_fmt(work_interval_label, "%lum", work_time_mn);
    lv_label_set_text_fmt(rest_interval_label, "%lum", rest_time_mn);
}

// LVGL class descriptor

const lv_obj_class_t overview_label_lvgl_class = {
    .base_class = &widget_lvgl_class,
    .constructor_cb = overview_label_lvgl_constructor,
    .name = "widget-overview-label",
    .width_def = LV_PCT(100),
    .height_def = LV_PCT(100),
    .instance_size = sizeof(OverviewLabel),
};
