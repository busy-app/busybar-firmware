#include "progress_counter.h"

#include <gui/widget_i.h>

#define MY_CLASS (&progress_counter_lvgl_class)

#define ROLLER_DELAY_TIME_MS (500)

struct ProgressCounter {
    Widget base;
    lv_obj_t* roller;
    lv_obj_t* done_label;
    lv_obj_t* prev_label;
    lv_obj_t* total_label;
};

const lv_obj_class_t progress_counter_lvgl_class;

// LVGL-specific code

static void progress_counter_lvgl_anim_completed_roller_callback(lv_anim_t* anim) {
    furi_assert(anim);

    ProgressCounter* instance = anim->var;
    lv_obj_scroll_to_view(instance->done_label, LV_ANIM_ON);
}

static void progress_counter_lvgl_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    UNUSED(class_p);

    lv_obj_set_flex_flow(obj, LV_FLEX_FLOW_ROW);

    ProgressCounter* instance = (ProgressCounter*)obj;

    instance->roller = lv_obj_create(obj);
    lv_obj_set_flex_flow(instance->roller, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_size(instance->roller, LV_SIZE_CONTENT, LV_SIZE_CONTENT);

    instance->done_label = lv_label_create(instance->roller);
    instance->prev_label = lv_label_create(instance->roller);
    instance->total_label = lv_label_create(obj);

    lv_obj_set_style_text_color(instance->prev_label, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_text_color(instance->done_label, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_text_color(instance->total_label, lv_color_white(), LV_PART_MAIN);
}

// Public API

ProgressCounter* progress_counter_alloc(Widget* parent) {
    furi_check(parent);

    lv_obj_t* obj = lv_obj_class_create_obj(MY_CLASS, (lv_obj_t*)parent);
    lv_obj_class_init_obj(obj);

    ProgressCounter* instance = (ProgressCounter*)obj;
    return instance;
}

void progress_counter_free(ProgressCounter* instance) {
    furi_check(instance);
    lv_obj_delete((lv_obj_t*)instance);
}

Widget* progress_counter_get_base(ProgressCounter* instance) {
    furi_check(instance);
    return (Widget*)instance;
}

void progress_counter_set_values(ProgressCounter* instance, uint32_t done, uint32_t total) {
    furi_check(instance);
    furi_check(done > 0);

    lv_label_set_text_fmt(instance->done_label, "%lu", done);
    lv_label_set_text_fmt(instance->prev_label, "%lu", done - 1);
    lv_label_set_text_fmt(instance->total_label, "/%lu", total);

    // Important, needed to get correct label height before any drawing
    lv_obj_update_layout(instance->total_label);

    const uint32_t label_height = lv_obj_get_height(instance->total_label);

    lv_obj_set_height(instance->roller, label_height);
    lv_obj_scroll_to_view(instance->prev_label, LV_ANIM_OFF);

    // TODO: A better way of creating a delay?
    lv_anim_t anim;
    lv_anim_init(&anim);

    lv_anim_set_delay(&anim, ROLLER_DELAY_TIME_MS);
    lv_anim_set_duration(&anim, 0);
    lv_anim_set_completed_cb(&anim, progress_counter_lvgl_anim_completed_roller_callback);

    lv_anim_set_var(&anim, instance);
    lv_anim_start(&anim);
}

// LVGL class descriptor

const lv_obj_class_t progress_counter_lvgl_class = {
    .base_class = &widget_lvgl_class,
    .constructor_cb = progress_counter_lvgl_constructor,
    .name = "widget-progress-counter",
    .width_def = LV_SIZE_CONTENT,
    .height_def = LV_SIZE_CONTENT,
    .instance_size = sizeof(ProgressCounter),
};
