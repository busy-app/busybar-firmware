#include "link_pin_view.h"
#include <settings_helpers/gui_params.h>
#include "../account_settings.h"
#include <storage/storage.h>
#include <gui/widget_i.h>

#define LINK_PIN_BACK_CLASS  (&link_pin_view_back_lvgl_class)
#define LINK_PIN_FRONT_CLASS (&link_pin_view_front_lvgl_class)

#define COLOR_BOTTOM_TEXT lv_color_hex(0x888888)

struct LinkPinView {
    Widget base;
    lv_obj_t* code_label;
    lv_obj_t* loading_spinner;
};

const lv_obj_class_t link_pin_view_back_lvgl_class;
const lv_obj_class_t link_pin_view_front_lvgl_class;

/* LVGL-specific code */

static void link_pin_view_front_lvgl_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    LV_UNUSED(class_p);
    LinkPinView* instance = (LinkPinView*)obj;

    lv_obj_set_flex_flow(obj, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(obj, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_set_style_pad_all(obj, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_gap(obj, 2, LV_PART_MAIN);

    lv_obj_t* image_cont = lv_obj_create(obj);
    lv_obj_set_size(image_cont, lv_pct(33), LV_SIZE_CONTENT);
    lv_obj_set_style_pad_all(image_cont, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(image_cont, 0, LV_PART_MAIN);
    lv_obj_set_flex_grow(image_cont, 0);

    lv_obj_t* code_cont = lv_obj_create(obj);
    lv_obj_set_size(code_cont, 36, LV_PCT(100));
    lv_obj_set_style_pad_all(code_cont, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(code_cont, 0, LV_PART_MAIN);
    lv_obj_set_flex_grow(code_cont, 0);

    lv_obj_t* lock_image = lv_img_create(image_cont);
    lv_img_set_src(lock_image, IMG_PATH("lock_front_12x12.bin"));
    lv_obj_align(lock_image, LV_ALIGN_RIGHT_MID, 0, 0);

    const lv_font_t* font = &lv_font_bf_7x10;

    instance->code_label = lv_label_create(code_cont);
    lv_label_set_text(instance->code_label, "");
    lv_obj_set_style_text_color(instance->code_label, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_text_font(instance->code_label, font, LV_PART_MAIN);
    lv_obj_align(instance->code_label, LV_ALIGN_LEFT_MID, 0, 0);
    lv_obj_add_flag(instance->code_label, LV_OBJ_FLAG_HIDDEN);

    instance->loading_spinner = lv_img_create(code_cont);
    lv_img_set_src(instance->loading_spinner, SETTINGS_IMG_PATH("spinner_front_7x7.bin"));
    lv_obj_align(instance->loading_spinner, LV_ALIGN_CENTER, -4, 0);
}

static void link_pin_view_back_lvgl_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    LV_UNUSED(class_p);
    LinkPinView* instance = (LinkPinView*)obj;

    lv_obj_set_flex_flow(obj, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(obj, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_gap(obj, 5, LV_PART_MAIN);

    lv_obj_t* top_line = lv_obj_create(obj);
    lv_obj_set_flex_flow(top_line, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(top_line, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_END, LV_FLEX_ALIGN_END);
    lv_obj_set_flex_grow(top_line, 0);
    lv_obj_set_size(top_line, LV_PCT(100), LV_PCT(40));
    lv_obj_set_style_pad_gap(top_line, 3, LV_PART_MAIN);

    lv_obj_t* image_cont = lv_obj_create(top_line);
    lv_obj_set_size(image_cont, LV_PCT(40), 13);
    lv_obj_set_style_pad_all(image_cont, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(image_cont, 0, LV_PART_MAIN);
    lv_obj_set_flex_grow(image_cont, 0);

    lv_obj_t* code_cont = lv_obj_create(top_line);
    lv_obj_set_size(code_cont, 36, 13);
    lv_obj_set_style_pad_all(code_cont, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(code_cont, 0, LV_PART_MAIN);
    lv_obj_set_flex_grow(code_cont, 0);
    lv_obj_clear_flag(code_cont, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t* lock_image = lv_img_create(image_cont);
    lv_img_set_src(lock_image, IMG_PATH("lock_back_11x11.bin"));
    lv_obj_align(lock_image, LV_ALIGN_BOTTOM_RIGHT, 0, -1);

    const lv_font_t* font = &lv_font_bf_7x10;

    instance->code_label = lv_label_create(code_cont);
    lv_label_set_text(instance->code_label, "");
    lv_obj_set_style_text_color(instance->code_label, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_text_font(instance->code_label, font, LV_PART_MAIN);
    lv_obj_align(instance->code_label, LV_ALIGN_BOTTOM_LEFT, 0, 1);
    lv_obj_add_flag(instance->code_label, LV_OBJ_FLAG_HIDDEN);

    instance->loading_spinner = lv_img_create(code_cont);
    lv_img_set_src(instance->loading_spinner, SETTINGS_IMG_PATH("spinner_front_7x7.bin"));
    lv_obj_align(instance->loading_spinner, LV_ALIGN_CENTER, -4, 0);

    font = lv_theme_get_font_normal(obj);
    lv_obj_t* bottom_label = lv_label_create(obj);
    lv_label_set_text(bottom_label, "Enter this code on\nwww.cloud.busy.app");
    lv_obj_set_style_text_color(bottom_label, COLOR_BOTTOM_TEXT, LV_PART_MAIN);
    lv_obj_set_style_text_font(bottom_label, font, LV_PART_MAIN);
    lv_obj_align(bottom_label, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_flex_grow(bottom_label, 1);
}

/* Public API */

static LinkPinView* link_pin_view_alloc_common(Widget* parent, const lv_obj_class_t* class_p) {
    furi_check(parent);

    lv_obj_t* obj = lv_obj_class_create_obj(class_p, TO_LV_OBJ(parent));
    lv_obj_class_init_obj(obj);

    LinkPinView* instance = (LinkPinView*)obj;

    return instance;
}

LinkPinView* link_pin_view_front_alloc(Widget* parent) {
    return link_pin_view_alloc_common(parent, LINK_PIN_FRONT_CLASS);
}

LinkPinView* link_pin_view_back_alloc(Widget* parent) {
    return link_pin_view_alloc_common(parent, LINK_PIN_BACK_CLASS);
}

void link_pin_view_free(LinkPinView* instance) {
    furi_check(instance);
    lv_obj_delete(TO_LV_OBJ(instance));
}

void link_pin_view_set_state(LinkPinView* instance, const char* pin_code) {
    furi_check(instance);

    if(pin_code) {
        lv_obj_add_flag(instance->loading_spinner, LV_OBJ_FLAG_HIDDEN);
        lv_label_set_text(instance->code_label, pin_code);
        lv_obj_remove_flag(instance->code_label, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_add_flag(instance->code_label, LV_OBJ_FLAG_HIDDEN);
        lv_label_set_text(instance->code_label, "");
        lv_obj_remove_flag(instance->loading_spinner, LV_OBJ_FLAG_HIDDEN);
    }
}

/* LVGL class descriptors */

const lv_obj_class_t link_pin_view_front_lvgl_class = {
    .base_class = &widget_lvgl_class,
    .constructor_cb = link_pin_view_front_lvgl_constructor,
    .name = "widget-link-pin-view-front",
    .width_def = LV_PCT(100),
    .height_def = LV_PCT(100),
    .instance_size = sizeof(LinkPinView),
};

const lv_obj_class_t link_pin_view_back_lvgl_class = {
    .base_class = &widget_lvgl_class,
    .constructor_cb = link_pin_view_back_lvgl_constructor,
    .name = "widget-link-pin-view-back",
    .width_def = LV_PCT(100),
    .height_def = LV_PCT(100),
    .instance_size = sizeof(LinkPinView),
};
