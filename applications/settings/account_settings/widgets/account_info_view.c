#include "account_info_view.h"
#include <storage/storage.h>
#include <gui/widget_i.h>
#include "../account_settings.h"

#define ACCOUNT_INFO_BACK_CLASS  (&account_info_view_back_lvgl_class)
#define ACCOUNT_INFO_FRONT_CLASS (&account_info_view_front_lvgl_class)

#define COLOR_CONNECTED lv_color_hex(0x22C55E)

struct AccountInfoView {
    Widget base;
    lv_obj_t* email_label;
};

const lv_obj_class_t account_info_view_back_lvgl_class;
const lv_obj_class_t account_info_view_front_lvgl_class;

/* LVGL-specific code */

static void
    account_info_view_front_lvgl_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    UNUSED(class_p);
    AccountInfoView* instance = (AccountInfoView*)obj;

    lv_obj_align(obj, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_set_flex_flow(obj, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(obj, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_set_style_pad_all(obj, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_gap(obj, 2, LV_PART_MAIN);

    lv_obj_t* state_image = lv_img_create(obj);
    lv_img_set_src(state_image, IMG_PATH("account_front_ok_8x8.bin"));
    lv_obj_set_flex_grow(state_image, 0);

    lv_obj_t* text_cont = lv_obj_create(obj);
    lv_obj_set_flex_flow(text_cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(
        text_cont, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_size(text_cont, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_flex_grow(text_cont, 1);
    lv_obj_set_style_pad_all(text_cont, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_gap(text_cont, 0, LV_PART_MAIN);

    const lv_font_t* font = lv_theme_get_font_normal(obj);

    lv_obj_t* state_label = lv_label_create(text_cont);
    lv_label_set_text(state_label, "Connected");
    lv_obj_set_style_text_color(state_label, COLOR_CONNECTED, LV_PART_MAIN);
    lv_obj_set_style_text_font(state_label, font, LV_PART_MAIN);

    instance->email_label = lv_label_create(text_cont);
    lv_label_set_text(instance->email_label, "");
    lv_obj_set_style_text_color(instance->email_label, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_text_font(instance->email_label, font, LV_PART_MAIN);
    lv_obj_set_width(instance->email_label, LV_PCT(100));
    lv_label_set_long_mode(instance->email_label, LV_LABEL_LONG_SCROLL);

    lv_obj_t* arrow = lv_label_create(obj);
    lv_label_set_text(arrow, ">");
    lv_obj_set_style_text_color(arrow, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_text_font(arrow, font, LV_PART_MAIN);
    lv_obj_set_flex_grow(arrow, 0);
}

static void account_info_view_back_lvgl_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    UNUSED(class_p);
    AccountInfoView* instance = (AccountInfoView*)obj;

    lv_obj_align(obj, LV_ALIGN_TOP_LEFT, 2, 2);
    lv_obj_set_flex_flow(obj, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(obj, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_set_style_radius(obj, 5, LV_PART_MAIN);
    lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN);
    lv_obj_set_style_border_color(obj, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_bg_color(obj, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_pad_all(obj, 5, LV_PART_MAIN);
    lv_obj_set_style_pad_gap(obj, 6, LV_PART_MAIN);

    lv_obj_t* state_image = lv_img_create(obj);
    lv_img_set_src(state_image, IMG_PATH("account_back_11x11.bin"));
    lv_obj_set_flex_grow(state_image, 0);

    lv_obj_t* text_cont = lv_obj_create(obj);
    lv_obj_set_flex_flow(text_cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(
        text_cont, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_size(text_cont, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_flex_grow(text_cont, 1);
    lv_obj_set_style_border_width(text_cont, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(text_cont, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_gap(text_cont, 2, LV_PART_MAIN);

    const lv_font_t* font = lv_theme_get_font_normal(obj);

    lv_obj_t* state_label = lv_label_create(text_cont);
    lv_label_set_text(state_label, "Connected");
    lv_obj_set_style_text_color(state_label, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_text_font(state_label, font, LV_PART_MAIN);

    instance->email_label = lv_label_create(text_cont);
    lv_label_set_text(instance->email_label, "");
    lv_obj_set_style_text_color(instance->email_label, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_text_font(instance->email_label, font, LV_PART_MAIN);
    lv_obj_set_width(instance->email_label, LV_PCT(100));
    lv_label_set_long_mode(instance->email_label, LV_LABEL_LONG_SCROLL);

    lv_obj_t* arrow = lv_label_create(obj);
    lv_label_set_text(arrow, ">");
    lv_obj_set_style_text_color(arrow, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_text_font(arrow, font, LV_PART_MAIN);
    lv_obj_set_flex_grow(arrow, 0);
}

/* Public API */

static AccountInfoView*
    account_info_view_alloc_common(Widget* parent, const lv_obj_class_t* class_p) {
    furi_check(parent);

    lv_obj_t* obj = lv_obj_class_create_obj(class_p, TO_LV_OBJ(parent));
    lv_obj_class_init_obj(obj);

    AccountInfoView* instance = (AccountInfoView*)obj;

    return instance;
}

AccountInfoView* account_info_view_front_alloc(Widget* parent) {
    return account_info_view_alloc_common(parent, ACCOUNT_INFO_FRONT_CLASS);
}

AccountInfoView* account_info_view_back_alloc(Widget* parent) {
    return account_info_view_alloc_common(parent, ACCOUNT_INFO_BACK_CLASS);
}

void account_info_view_free(AccountInfoView* instance) {
    furi_check(instance);
    lv_obj_delete(TO_LV_OBJ(instance));
}

void account_info_view_set_state(AccountInfoView* instance, const char* email) {
    furi_check(instance);

    lv_label_set_text(instance->email_label, email);
}

/* LVGL class descriptors */

const lv_obj_class_t account_info_view_front_lvgl_class = {
    .base_class = &widget_lvgl_class,
    .constructor_cb = account_info_view_front_lvgl_constructor,
    .name = "widget-account-info-view-front",
    .width_def = LV_PCT(100),
    .height_def = LV_PCT(100),
    .instance_size = sizeof(AccountInfoView),
};

const lv_obj_class_t account_info_view_back_lvgl_class = {
    .base_class = &widget_lvgl_class,
    .constructor_cb = account_info_view_back_lvgl_constructor,
    .name = "widget-account-info-view-back",
    .width_def = LV_PCT(95),
    .height_def = LV_PCT(50),
    .instance_size = sizeof(AccountInfoView),
};
