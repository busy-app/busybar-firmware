#include "mirror_card.h"

#include <storage/storage.h>
#include <gui/widget_i.h>
#include <gui/modules/front_display_mirror.h>

#define MY_CLASS (&mirror_card_lvgl_class)

struct MirrorCard {
    Widget base;

    lv_obj_t* header_left_image;
    lv_obj_t* header_right_image;
    lv_obj_t* header_label;

    DisplayMirror* display_mirror;

    lv_obj_t* footer_primary_label;
    lv_obj_t* footer_secondary_label;

    FontRegistry* font_registry;
    const lv_font_t* font_busy_regular_5;
    const lv_font_t* font_busy_regular_7;
};

const lv_obj_class_t mirror_card_lvgl_class;

/* LVGL-specific code */

static void mirror_card_lvgl_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    UNUSED(class_p);

    MirrorCard* instance = (MirrorCard*)obj;

    instance->font_registry = furi_record_open(RECORD_FONT_REGISTRY);
    instance->font_busy_regular_5 =
        font_registry_load_font(instance->font_registry, FONT_BUSY_REGULAR_5);
    instance->font_busy_regular_7 =
        font_registry_load_font(instance->font_registry, FONT_BUSY_REGULAR_7);

    lv_obj_set_flex_flow(obj, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(obj, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_row(obj, 8, LV_PART_MAIN);
    lv_obj_set_style_radius(obj, 4, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_bg_color(obj, lv_color_white(), LV_PART_MAIN);

    lv_obj_t* header_layout = lv_obj_create(obj);
    lv_obj_set_flex_flow(header_layout, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(
        header_layout, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_column(header_layout, 4, LV_PART_MAIN);
    lv_obj_set_style_pad_top(header_layout, 3, LV_PART_MAIN);
    lv_obj_set_style_pad_bottom(header_layout, 1, LV_PART_MAIN);
    lv_obj_set_size(header_layout, LV_SIZE_CONTENT, LV_SIZE_CONTENT);

    instance->header_left_image = lv_image_create(header_layout);
    lv_image_set_src(
        instance->header_left_image, SHARED_IMG_PATH("active_indicator_left_28x7.image"));

    instance->header_label = lv_label_create(header_layout);
    lv_obj_set_style_text_font(
        instance->header_label, instance->font_busy_regular_5, LV_PART_MAIN);
    lv_obj_set_style_text_color(instance->header_label, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_pad_left(instance->header_label, 1, LV_PART_MAIN);

    instance->header_right_image = lv_image_create(header_layout);
    lv_image_set_src(
        instance->header_right_image, SHARED_IMG_PATH("active_indicator_right_28x7.image"));

    /* mask object for image rounded corners */
    instance->display_mirror = display_mirror_alloc(&instance->base);
    Widget* mirror_base = display_mirror_get_base(instance->display_mirror);
    lv_obj_t* mirror_obj = TO_LV_OBJ(mirror_base);
    lv_obj_set_style_pad_all(mirror_obj, 1, LV_PART_MAIN);
    lv_obj_set_style_radius(mirror_obj, 3, LV_PART_MAIN);
    lv_obj_set_style_clip_corner(mirror_obj, true, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(mirror_obj, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_bg_color(mirror_obj, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_size(mirror_obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);

    lv_obj_t* footer_layout = lv_obj_create(obj);
    lv_obj_set_flex_flow(footer_layout, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(
        footer_layout, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_END, LV_FLEX_ALIGN_END);
    lv_obj_set_style_pad_column(footer_layout, 2, LV_PART_MAIN);
    lv_obj_set_style_translate_y(footer_layout, -2, LV_PART_MAIN);
    lv_obj_set_size(footer_layout, LV_SIZE_CONTENT, LV_SIZE_CONTENT);

    instance->footer_primary_label = lv_label_create(footer_layout);
    lv_obj_set_style_text_font(
        instance->footer_primary_label, instance->font_busy_regular_7, LV_PART_MAIN);
    lv_obj_set_style_text_color(instance->footer_primary_label, lv_color_black(), LV_PART_MAIN);

    instance->footer_secondary_label = lv_label_create(footer_layout);
    lv_obj_set_style_text_font(
        instance->footer_secondary_label, instance->font_busy_regular_5, LV_PART_MAIN);
    lv_obj_set_style_text_color(instance->footer_secondary_label, lv_color_black(), LV_PART_MAIN);
}

static void mirror_card_lvgl_destructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    UNUSED(class_p);

    MirrorCard* instance = (MirrorCard*)obj;

    font_registry_unload_font(instance->font_registry, instance->font_busy_regular_7);
    font_registry_unload_font(instance->font_registry, instance->font_busy_regular_5);
    furi_record_close(RECORD_FONT_REGISTRY);
}

/* public API */

MirrorCard* mirror_card_alloc(Widget* parent) {
    furi_check(parent);

    lv_obj_t* obj = lv_obj_class_create_obj(MY_CLASS, TO_LV_OBJ(parent));
    lv_obj_class_init_obj(obj);

    return (MirrorCard*)obj;
}

void mirror_card_free(MirrorCard* instance) {
    furi_check(instance);

    lv_obj_delete(TO_LV_OBJ(instance));
}

Widget* mirror_card_get_base(MirrorCard* instance) {
    furi_check(instance);

    return (Widget*)instance;
}

void mirror_card_set_show_header(MirrorCard* instance, bool show_header) {
    furi_check(instance);

    lv_opa_t opacity = show_header ? LV_OPA_COVER : LV_OPA_TRANSP;
    lv_obj_set_style_text_opa(instance->header_label, opacity, LV_PART_MAIN);
    lv_obj_set_style_image_opa(instance->header_left_image, opacity, LV_PART_MAIN);
    lv_obj_set_style_image_opa(instance->header_right_image, opacity, LV_PART_MAIN);
}

void mirror_card_set_header_text(MirrorCard* instance, const char* header_text) {
    furi_check(instance);

    lv_label_set_text(instance->header_label, header_text);
}

void mirror_card_set_show_footer(MirrorCard* instance, bool show_footer) {
    furi_check(instance);

    lv_opa_t opacity = show_footer ? LV_OPA_COVER : LV_OPA_TRANSP;
    lv_obj_set_style_text_opa(instance->footer_primary_label, opacity, LV_PART_MAIN);
    lv_obj_set_style_text_opa(instance->footer_secondary_label, opacity, LV_PART_MAIN);
}

void mirror_card_set_footer_primary_text(MirrorCard* instance, const char* primary_text) {
    furi_check(instance);

    lv_label_set_text(instance->footer_primary_label, primary_text);
}

void mirror_card_set_footer_secondary_text(MirrorCard* instance, const char* secondary_text) {
    furi_check(instance);

    lv_label_set_text(instance->footer_secondary_label, secondary_text);
}

/* LVGL class descriptor */

const lv_obj_class_t mirror_card_lvgl_class = {
    .base_class = &widget_lvgl_class,
    .constructor_cb = mirror_card_lvgl_constructor,
    .destructor_cb = mirror_card_lvgl_destructor,
    .name = "widget-mirror-card",
    .width_def = LV_PCT(100),
    .height_def = LV_PCT(100),
    .instance_size = sizeof(MirrorCard),
};
