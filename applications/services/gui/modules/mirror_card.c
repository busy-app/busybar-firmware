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
};

const lv_obj_class_t mirror_card_lvgl_class;

/* LVGL-specific code */

static void mirror_card_lvgl_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    UNUSED(class_p);

    lv_obj_set_flex_flow(obj, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(obj, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t* top_layout = lv_obj_create(obj);
    lv_obj_set_size(top_layout, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(top_layout, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(
        top_layout, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_column(top_layout, 4, LV_PART_MAIN);
    lv_obj_set_style_margin_bottom(top_layout, 10, LV_PART_MAIN);

    MirrorCard* instance = (MirrorCard*)obj;
    instance->header_left_image = lv_image_create(top_layout);
    lv_image_set_src(
        instance->header_left_image, SHARED_IMG_PATH("active_indicator_left_28x7.bin"));

    instance->header_label = lv_label_create(top_layout);
    lv_obj_set_style_text_font(instance->header_label, lv_theme_get_font_small(obj), LV_PART_MAIN);
    lv_obj_set_style_text_color(instance->header_label, lv_color_black(), LV_PART_MAIN);

    instance->header_right_image = lv_image_create(top_layout);
    lv_image_set_src(
        instance->header_right_image, SHARED_IMG_PATH("active_indicator_right_28x7.bin"));

    /* mask object for image rounded corners */
    instance->display_mirror = display_mirror_alloc((Widget*)instance);
    Widget* mirror_base = display_mirror_get_base(instance->display_mirror);
    lv_obj_t* mirror_obj = TO_LV_OBJ(mirror_base);
    lv_obj_set_size(mirror_obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_style_radius(mirror_obj, 6, LV_PART_MAIN);
    lv_obj_set_style_clip_corner(mirror_obj, true, LV_PART_MAIN);
    lv_obj_set_style_pad_hor(mirror_obj, 1, LV_PART_MAIN);
    lv_obj_set_style_pad_ver(mirror_obj, 2, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(mirror_obj, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_bg_color(mirror_obj, lv_color_black(), LV_PART_MAIN);

    lv_obj_t* bottom_layout = lv_obj_create(obj);
    lv_obj_set_size(bottom_layout, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(bottom_layout, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(
        bottom_layout, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_END, LV_FLEX_ALIGN_END);
    lv_obj_set_style_bg_opa(bottom_layout, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_pad_column(bottom_layout, 2, LV_PART_MAIN);
    lv_obj_set_style_margin_top(bottom_layout, 8, LV_PART_MAIN);
    lv_obj_set_style_margin_bottom(bottom_layout, -3, LV_PART_MAIN);

    instance->footer_primary_label = lv_label_create(bottom_layout);
    lv_obj_set_style_text_color(instance->footer_primary_label, lv_color_black(), LV_PART_MAIN);

    instance->footer_secondary_label = lv_label_create(bottom_layout);
    lv_obj_set_style_text_color(instance->footer_secondary_label, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_text_font(
        instance->footer_secondary_label, lv_theme_get_font_small(obj), LV_PART_MAIN);
    lv_obj_set_style_translate_y(instance->footer_secondary_label, -2, LV_PART_MAIN);
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
    .name = "widget-mirror-card",
    .width_def = LV_PCT(100),
    .height_def = LV_SIZE_CONTENT,
    .instance_size = sizeof(MirrorCard),
};
