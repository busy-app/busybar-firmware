#include "summary_view.h"

#include <gui/widget_i.h>

#define MY_CLASS (&summary_view_lvgl_class)

#define COLOR_GREY_1 0x4F4B4B
#define COLOR_GREY_2 0X3B3939

#define COLOR_RED_1 0xFF001D
#define COLOR_RED_2 0x680000

#define ELEMENT_HEIGHT    (7)
#define ELEMENT_COUNT_MAX (10UL)

#define SUMMARY_VIEW_MAX_WIDTH (70)

#define ELEMENT_ANIM_DURATION_MS  (500)
#define SEQUENCE_ANIM_DURATION_MS (1000)
#define SEQUENCE_ANIM_DELAY_MS    (500)

struct SummaryView {
    Widget base;
    lv_obj_t* elements[ELEMENT_COUNT_MAX];
};

const lv_obj_class_t summary_view_lvgl_class;

// LVGL-specific code

static void summary_view_element_lvgl_anim_callback(void* context, int32_t value) {
    furi_assert(context);

    lv_obj_t* element = context;
    lv_obj_set_style_bg_opa(element, value, LV_PART_MAIN);
}

static void summary_view_sequence_lvgl_anim_callback(void* context, int32_t value) {
    furi_assert(context);
    SummaryView* instance = context;

    const uint32_t element_idx = value;
    furi_assert(element_idx < ELEMENT_COUNT_MAX);

    lv_obj_t* element = instance->elements[element_idx];

    lv_anim_t element_anim;
    lv_anim_init(&element_anim);
    lv_anim_set_duration(&element_anim, ELEMENT_ANIM_DURATION_MS);
    lv_anim_set_values(&element_anim, LV_OPA_TRANSP, LV_OPA_COVER);
    lv_anim_set_exec_cb(&element_anim, summary_view_element_lvgl_anim_callback);
    lv_anim_set_var(&element_anim, element);

    lv_obj_t* overlay = lv_obj_get_child(element, 0);

    lv_anim_t overlay_anim;
    lv_anim_init(&overlay_anim);
    lv_anim_set_duration(&overlay_anim, ELEMENT_ANIM_DURATION_MS / 2);
    lv_anim_set_reverse_duration(&overlay_anim, ELEMENT_ANIM_DURATION_MS / 2);
    lv_anim_set_values(&overlay_anim, LV_OPA_TRANSP, LV_OPA_COVER);
    lv_anim_set_exec_cb(&overlay_anim, summary_view_element_lvgl_anim_callback);
    lv_anim_set_var(&overlay_anim, overlay);

    lv_anim_start(&element_anim);
    lv_anim_start(&overlay_anim);
}

static void summary_view_lvgl_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    UNUSED(class_p);

    lv_obj_set_flex_flow(obj, LV_FLEX_FLOW_ROW);

    lv_obj_set_style_bg_color(obj, lv_color_hex(COLOR_GREY_1), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, LV_PART_MAIN);

    lv_obj_set_style_bg_grad_dir(obj, LV_GRAD_DIR_VER, LV_PART_MAIN);
    lv_obj_set_style_bg_grad_color(obj, lv_color_hex(COLOR_GREY_2), LV_PART_MAIN);
    lv_obj_set_style_bg_grad_opa(obj, LV_OPA_COVER, LV_PART_MAIN);

    lv_obj_set_style_radius(obj, 2, LV_PART_MAIN);
    lv_obj_set_style_clip_corner(obj, true, LV_PART_MAIN);
}

// Implementation

static void
    summary_view_add_element(SummaryView* instance, uint32_t element_idx, uint32_t element_count) {
    const uint32_t element_width = SUMMARY_VIEW_MAX_WIDTH / element_count;

    lv_obj_t* element = lv_obj_create(TO_LV_OBJ(instance));
    lv_obj_set_size(element, element_width, ELEMENT_HEIGHT);

    lv_obj_set_style_bg_color(element, lv_color_hex(COLOR_RED_1), LV_PART_MAIN);
    lv_obj_set_style_bg_grad_dir(element, LV_GRAD_DIR_VER, LV_PART_MAIN);
    lv_obj_set_style_bg_grad_color(element, lv_color_hex(COLOR_RED_2), LV_PART_MAIN);
    lv_obj_set_style_bg_grad_opa(element, LV_OPA_COVER, LV_PART_MAIN);

    lv_obj_t* overlay = lv_obj_create(element);
    lv_obj_set_size(overlay, LV_PCT(100), LV_PCT(100));

    lv_obj_set_style_bg_color(overlay, lv_color_white(), LV_PART_MAIN);

    instance->elements[element_idx] = element;
}

static void summary_view_start_sequence(SummaryView* instance, uint32_t element_count) {
    lv_anim_t anim;
    lv_anim_init(&anim);
    lv_anim_set_delay(&anim, SEQUENCE_ANIM_DELAY_MS);
    lv_anim_set_duration(&anim, SEQUENCE_ANIM_DURATION_MS);
    lv_anim_set_early_apply(&anim, false);
    lv_anim_set_values(&anim, 0, element_count - 1);
    lv_anim_set_path_cb(&anim, lv_anim_path_ease_in);
    lv_anim_set_exec_cb(&anim, summary_view_sequence_lvgl_anim_callback);
    lv_anim_set_var(&anim, instance);
    lv_anim_start(&anim);
}

// Public API

SummaryView* summary_view_alloc(Widget* parent) {
    furi_check(parent);

    lv_obj_t* obj = lv_obj_class_create_obj(MY_CLASS, TO_LV_OBJ(parent));
    lv_obj_class_init_obj(obj);

    SummaryView* instance = (SummaryView*)obj;
    return instance;
}

void summary_view_free(SummaryView* instance) {
    furi_check(instance);
    lv_obj_delete(TO_LV_OBJ(instance));
}

Widget* summary_view_get_base(SummaryView* instance) {
    furi_check(instance);
    return (Widget*)instance;
}

void summary_view_set_cycles_count(SummaryView* instance, uint32_t cycles_count) {
    furi_check(instance);

    const uint32_t element_count = MIN(cycles_count, ELEMENT_COUNT_MAX);

    for(uint32_t i = 0; i < element_count; ++i) {
        summary_view_add_element(instance, i, element_count);
    }

    summary_view_start_sequence(instance, element_count);
}

// LVGL class descriptor

const lv_obj_class_t summary_view_lvgl_class = {
    .base_class = &widget_lvgl_class,
    .constructor_cb = summary_view_lvgl_constructor,
    .name = "widget-summary-view",
    .width_def = LV_SIZE_CONTENT,
    .height_def = LV_SIZE_CONTENT,
    .instance_size = sizeof(SummaryView),
};
