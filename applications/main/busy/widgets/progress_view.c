#include "progress_view.h"

#include <gui/widget_i.h>

#include <assets/assets_images.h>

#define MY_CLASS (&progress_view_lvgl_class)

#define COLOR_BG     (lv_color_hex(0x333333))
#define COLOR_NEXT   (lv_color_hex(0x6F6F6F))
#define COLOR_DONE_1 (lv_color_hex(0x910000))
#define COLOR_DONE_2 (lv_color_hex(0xFF2020))
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

void progress_view_lvgl_grow_anim_exec_callback(lv_anim_t* anim, int32_t value) {
    furi_assert(anim);
    ProgressView* instance = anim->var;

    const int32_t done_bar_width = MAX(value - 1, 0);
    lv_obj_set_width(instance->done_bar, done_bar_width);

    const int32_t end_value = anim->end_value;

    if(end_value < lv_obj_get_width(lv_obj_get_parent(instance->done_bar))) {
        const int32_t next_bar_width = 2 * end_value - (done_bar_width + anim->start_value);
        lv_obj_set_width(instance->next_bar, next_bar_width);
    }
}

void progress_view_lvgl_blink_anim_exec_callback(void* context, int32_t value) {
    furi_assert(context);
    ProgressView* instance = context;

    lv_obj_set_style_bg_color(
        instance->next_bar, lv_color_mix(COLOR_NEXT, COLOR_BG, value), LV_PART_MAIN);
}

static void progress_view_lvgl_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    UNUSED(class_p);

    lv_obj_set_flex_flow(obj, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_hor(obj, 2, LV_PART_MAIN);
    lv_obj_set_style_pad_ver(obj, 1, LV_PART_MAIN);
    lv_obj_set_style_pad_row(obj, 1, LV_PART_MAIN);

    ProgressView* instance = (ProgressView*)obj;

    lv_obj_t* top_layout = lv_obj_create(obj);
    lv_obj_set_size(top_layout, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(top_layout, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_column(top_layout, 2, LV_PART_MAIN);
    lv_obj_set_flex_align(
        top_layout, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t* done_label = lv_label_create(top_layout);
    lv_label_set_text(done_label, "DONE");
    lv_obj_set_flex_grow(done_label, 1);
    lv_obj_set_style_text_color(done_label, lv_color_white(), LV_PART_MAIN);

    lv_obj_t* tick_icon = lv_image_create(top_layout);
    lv_image_set_src(tick_icon, &I_tick_red_6x5);

    instance->progress_label = lv_label_create(top_layout);
    lv_obj_set_style_text_color(instance->progress_label, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_text_font(instance->progress_label, &lv_font_somybmp_7, LV_PART_MAIN);
    lv_obj_set_style_margin_right(instance->progress_label, -1, LV_PART_MAIN);

    lv_obj_t* bottom_layout = lv_obj_create(obj);
    lv_obj_set_size(bottom_layout, LV_PCT(100), 6);
    lv_obj_set_flex_flow(bottom_layout, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(
        bottom_layout, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    instance->done_bar = lv_obj_create(bottom_layout);
    lv_obj_set_size(instance->done_bar, 0, 4);
    lv_obj_set_style_bg_opa(instance->done_bar, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_bg_color(instance->done_bar, COLOR_DONE_1, LV_PART_MAIN);
    lv_obj_set_style_bg_grad_color(instance->done_bar, COLOR_DONE_2, LV_PART_MAIN);
    lv_obj_set_style_bg_grad_dir(instance->done_bar, LV_GRAD_DIR_HOR, LV_PART_MAIN);

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

static void
    progress_view_start_grow_animation(ProgressView* instance, int32_t done, int32_t total) {
    const int32_t total_width = lv_obj_get_width(lv_obj_get_parent(instance->done_bar));
    const int32_t sector_width = total_width / total;
    const int32_t done_width = done * total_width / total;
    const int32_t prev_done_width = done_width - sector_width;

    lv_anim_t anim;
    lv_anim_init(&anim);

    lv_anim_set_values(&anim, prev_done_width, done_width);
    lv_anim_set_duration(&anim, 500);
    lv_anim_set_delay(&anim, 500);
    if(done < total) {
        lv_anim_set_bezier3_param(
            &anim,
            LV_BEZIER_VAL_FLOAT(0.37F),
            LV_BEZIER_VAL_FLOAT(0.0F),
            LV_BEZIER_VAL_FLOAT(0.3F),
            LV_BEZIER_VAL_FLOAT(1.78F));
    } else {
        lv_anim_set_bezier3_param(
            &anim,
            LV_BEZIER_VAL_FLOAT(0.37F),
            LV_BEZIER_VAL_FLOAT(0.0F),
            LV_BEZIER_VAL_FLOAT(0.3F),
            LV_BEZIER_VAL_FLOAT(1.4F));
    }
    lv_anim_set_path_cb(&anim, lv_anim_path_custom_bezier3);
    lv_anim_set_custom_exec_cb(&anim, progress_view_lvgl_grow_anim_exec_callback);
    lv_anim_set_var(&anim, instance);

    lv_anim_start(&anim);
}

static void progress_view_start_blink_animation(ProgressView* instance) {
    lv_anim_t anim;
    lv_anim_init(&anim);

    lv_anim_set_repeat_count(&anim, LV_ANIM_REPEAT_INFINITE);
    lv_anim_set_values(&anim, LV_OPA_TRANSP, LV_OPA_COVER);
    lv_anim_set_duration(&anim, 1000);
    lv_anim_set_reverse_duration(&anim, 1000);
    lv_anim_set_delay(&anim, 700);
    lv_anim_set_path_cb(&anim, lv_anim_path_ease_in_out);
    lv_anim_set_exec_cb(&anim, progress_view_lvgl_blink_anim_exec_callback);
    lv_anim_set_var(&anim, instance);

    lv_anim_start(&anim);
}

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

    progress_view_start_grow_animation(instance, done, total);

    if(done != total) {
        progress_view_start_blink_animation(instance);
    }
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
