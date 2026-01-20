#include "summary_label.h"

#include <gui/widget_i.h>

#include "../storage_macros.h"

#define MY_CLASS (&summary_label_lvgl_class)

#define ANIM_DURATION_MS (250)

#define TRANSLATE_Y_FACTOR (32)

struct SummaryLabel {
    Widget base;
    lv_obj_t* cycles_layout;
    lv_obj_t* cycles_label;
    lv_obj_t* message_label;

    lv_obj_t* show_target;
    lv_obj_t* hide_target;
};

const lv_obj_class_t summary_label_lvgl_class;

// LVGL-specific code

static void summary_label_lvgl_anim_callback(void* context, int32_t value) {
    furi_assert(context);
    SummaryLabel* instance = context;

    const int32_t reverse_value = LV_OPA_COVER - value;

    lv_obj_set_style_opa(instance->show_target, value, LV_PART_MAIN);
    lv_obj_set_style_opa(instance->hide_target, reverse_value, LV_PART_MAIN);

    lv_obj_set_style_translate_y(
        instance->show_target, -reverse_value / TRANSLATE_Y_FACTOR, LV_PART_MAIN);
    lv_obj_set_style_translate_y(instance->hide_target, -value / TRANSLATE_Y_FACTOR, LV_PART_MAIN);
}

static void summary_label_lvgl_anim_completed_callback(lv_anim_t* anim) {
    furi_assert(anim);

    SummaryLabel* instance = anim->var;
    FURI_SWAP(instance->hide_target, instance->show_target);
}

static void summary_label_lvgl_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    UNUSED(class_p);

    lv_obj_t* cycles_layout = lv_obj_create(obj);
    lv_obj_set_flex_flow(cycles_layout, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_column(cycles_layout, 2, LV_PART_MAIN);
    lv_obj_set_style_flex_cross_place(cycles_layout, LV_FLEX_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_set_size(cycles_layout, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_style_align(cycles_layout, LV_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_set_style_opa(cycles_layout, LV_OPA_TRANSP, LV_PART_MAIN);

    lv_obj_t* cycles_icon = lv_image_create(cycles_layout);
    lv_image_set_src(cycles_icon, BUSY_IMG_PATH("tick_red_6x5.bin"));

    lv_obj_t* cycles_label = lv_label_create(cycles_layout);
    lv_obj_set_style_text_color(cycles_label, lv_color_white(), LV_PART_MAIN);

    lv_obj_t* message_label = lv_label_create(obj);
    lv_label_set_text(message_label, "Well done!");
    lv_obj_set_style_text_color(message_label, lv_color_white(), LV_PART_MAIN);

    SummaryLabel* instance = (SummaryLabel*)obj;
    instance->cycles_layout = cycles_layout;
    instance->cycles_label = cycles_label;
    instance->message_label = message_label;

    instance->show_target = message_label;
    instance->hide_target = cycles_layout;
}

// Implementation

// Public API

SummaryLabel* summary_label_alloc(Widget* parent) {
    furi_check(parent);

    lv_obj_t* obj = lv_obj_class_create_obj(MY_CLASS, TO_LV_OBJ(parent));
    lv_obj_class_init_obj(obj);

    SummaryLabel* instance = (SummaryLabel*)obj;
    return instance;
}

void summary_label_free(SummaryLabel* instance) {
    furi_check(instance);
    lv_obj_delete(TO_LV_OBJ(instance));
}

Widget* summary_label_get_base(SummaryLabel* instance) {
    furi_check(instance);
    return (Widget*)instance;
}

void summary_label_set_cycles_count(SummaryLabel* instance, uint32_t cycles_count) {
    furi_check(instance);

    lv_label_set_text_fmt(instance->cycles_label, "%lu/%lu", cycles_count, cycles_count);

    lv_obj_set_style_opa(instance->cycles_layout, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_opa(instance->message_label, LV_OPA_TRANSP, LV_PART_MAIN);
}

void summary_label_switch_display(SummaryLabel* instance) {
    furi_check(instance);

    lv_anim_t anim;
    lv_anim_init(&anim);
    lv_anim_set_values(&anim, LV_OPA_TRANSP, LV_OPA_COVER);
    lv_anim_set_duration(&anim, ANIM_DURATION_MS);
    lv_anim_set_path_cb(&anim, lv_anim_path_ease_in_out);
    lv_anim_set_exec_cb(&anim, summary_label_lvgl_anim_callback);
    lv_anim_set_completed_cb(&anim, summary_label_lvgl_anim_completed_callback);
    lv_anim_set_var(&anim, instance);
    lv_anim_start(&anim);
}

// LVGL class descriptor

const lv_obj_class_t summary_label_lvgl_class = {
    .base_class = &widget_lvgl_class,
    .constructor_cb = summary_label_lvgl_constructor,
    .name = "widget-summary-label",
    .width_def = LV_SIZE_CONTENT,
    .height_def = LV_SIZE_CONTENT,
    .instance_size = sizeof(SummaryLabel),
};
