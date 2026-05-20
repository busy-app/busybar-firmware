#include "wifi_info_view.h"
#include "../wifi_settings_i.h"
#include <storage/storage.h>
#include <gui/widget_i.h>

#define WIFI_INFO_BACK_CLASS  (&wifi_info_view_back_lvgl_class)
#define WIFI_INFO_FRONT_CLASS (&wifi_info_view_front_lvgl_class)

#define COLOR_IP lv_color_hex(0x888888)

#define ARROW_CHAR "\u25B6"

struct WifiInfoView {
    Widget base;
    lv_obj_t* ip_label;

    FontRegistry* font_registry;
    const lv_font_t* font_regular_5;
    const lv_font_t* font_regular_9;
    const lv_font_t* font_regular_7;
};

const lv_obj_class_t wifi_info_view_back_lvgl_class;
const lv_obj_class_t wifi_info_view_front_lvgl_class;
const lv_obj_class_t wifi_info_view_arrow_lvgl_class;

/* LVGL-specific code */

static void wifi_info_view_back_lvgl_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    UNUSED(class_p);
    WifiInfoView* instance = (WifiInfoView*)obj;

    instance->font_registry = furi_record_open(RECORD_FONT_REGISTRY);
    instance->font_regular_9 =
        font_registry_load_font(instance->font_registry, FONT_BUSY_REGULAR_9);
    instance->font_regular_7 =
        font_registry_load_font(instance->font_registry, FONT_BUSY_REGULAR_7);

    lv_obj_align(obj, LV_ALIGN_TOP_LEFT, 0, 0);

    lv_obj_set_flex_flow(obj, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(obj, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_bg_opa(obj, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_pad_hor(obj, 2, LV_PART_MAIN);
    lv_obj_set_style_pad_row(obj, 4, LV_PART_MAIN);

    lv_obj_t* ip_row = lv_obj_create(obj);
    lv_obj_set_flex_flow(ip_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_size(ip_row, LV_SIZE_CONTENT, LV_SIZE_CONTENT);

    lv_obj_t* label_desc = lv_label_create(ip_row);
    lv_label_set_text(label_desc, "IP address ");
    lv_obj_set_style_text_color(label_desc, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_text_font(label_desc, instance->font_regular_7, LV_PART_MAIN);

    instance->ip_label = lv_label_create(ip_row);
    lv_label_set_text(instance->ip_label, "none");
    lv_obj_set_style_text_color(instance->ip_label, COLOR_IP, LV_PART_MAIN);
    lv_obj_set_style_text_font(instance->ip_label, instance->font_regular_7, LV_PART_MAIN);

    lv_obj_t* back_btn = lv_obj_create(obj);
    lv_obj_set_size(back_btn, LV_PCT(100), 15);

    lv_obj_set_flex_flow(back_btn, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(
        back_btn, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_radius(back_btn, 4, LV_PART_MAIN);
    lv_obj_set_style_border_color(back_btn, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_bg_color(back_btn, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(back_btn, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_pad_hor(back_btn, 3, LV_PART_MAIN);
    lv_obj_set_style_pad_column(back_btn, 4, LV_PART_MAIN);
    lv_obj_set_scrollbar_mode(back_btn, LV_SCROLLBAR_MODE_OFF);

    lv_obj_t* arrow = lv_obj_class_create_obj(&wifi_info_view_arrow_lvgl_class, back_btn);
    lv_label_set_text(arrow, ARROW_CHAR);
    lv_obj_set_style_pad_left(arrow, 3, LV_PART_MAIN);
    lv_obj_set_style_text_color(arrow, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_text_font(arrow, instance->font_regular_9, LV_PART_MAIN);

    lv_obj_t* label_back = lv_label_create(back_btn);
    lv_label_set_text(label_back, "Back");
    lv_obj_set_style_text_color(label_back, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_text_font(label_back, instance->font_regular_9, LV_PART_MAIN);
}

static void wifi_info_view_front_lvgl_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    UNUSED(class_p);
    WifiInfoView* instance = (WifiInfoView*)obj;

    instance->font_registry = furi_record_open(RECORD_FONT_REGISTRY);
    instance->font_regular_5 =
        font_registry_load_font(instance->font_registry, FONT_BUSY_REGULAR_5);

    lv_obj_align(obj, LV_ALIGN_TOP_LEFT, 0, 0);

    lv_obj_set_flex_flow(obj, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(obj, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_bg_opa(obj, LV_OPA_TRANSP, LV_PART_MAIN);

    lv_obj_t* ip_row = lv_obj_create(obj);
    lv_obj_set_flex_flow(ip_row, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_size(ip_row, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(ip_row, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_flex_grow(ip_row, 1);

    lv_obj_t* label_desc = lv_label_create(ip_row);
    lv_label_set_text(label_desc, "IP address ");
    lv_obj_set_style_text_color(label_desc, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_text_font(label_desc, instance->font_regular_5, LV_PART_MAIN);

    instance->ip_label = lv_label_create(ip_row);
    lv_label_set_text(instance->ip_label, "none");
    lv_obj_set_style_margin_top(instance->ip_label, -2, LV_PART_MAIN);
    lv_obj_set_style_text_color(instance->ip_label, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_text_font(instance->ip_label, instance->font_regular_5, LV_PART_MAIN);

    lv_obj_t* arrow = lv_img_create(obj);
    lv_img_set_src(arrow, IMG_PATH("back_arrow_5x5.image"));
    lv_obj_set_style_pad_all(arrow, 2, LV_PART_MAIN);
}

static void wifi_info_view_front_lvgl_destructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    UNUSED(class_p);
    WifiInfoView* instance = (WifiInfoView*)obj;

    font_registry_unload_font(instance->font_registry, instance->font_regular_5);

    furi_record_close(RECORD_FONT_REGISTRY);
}

static void wifi_info_view_back_lvgl_destructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    UNUSED(class_p);
    WifiInfoView* instance = (WifiInfoView*)obj;

    font_registry_unload_font(instance->font_registry, instance->font_regular_9);
    font_registry_unload_font(instance->font_registry, instance->font_regular_7);

    furi_record_close(RECORD_FONT_REGISTRY);
}

/* Public API */

WifiInfoView* wifi_info_view_alloc_common(Widget* parent, const lv_obj_class_t* class_p) {
    furi_check(parent);

    lv_obj_t* obj = lv_obj_class_create_obj(class_p, TO_LV_OBJ(parent));
    lv_obj_class_init_obj(obj);

    WifiInfoView* instance = (WifiInfoView*)obj;

    return instance;
}

WifiInfoView* wifi_info_view_front_alloc(Widget* parent) {
    return wifi_info_view_alloc_common(parent, WIFI_INFO_FRONT_CLASS);
}

WifiInfoView* wifi_info_view_back_alloc(Widget* parent) {
    return wifi_info_view_alloc_common(parent, WIFI_INFO_BACK_CLASS);
}

void wifi_info_view_free(WifiInfoView* instance) {
    furi_check(instance);
    lv_obj_delete(TO_LV_OBJ(instance));
}

void wifi_info_view_set_address(WifiInfoView* instance, bool is_connected, const char* addr) {
    furi_check(instance);
    if(is_connected) {
        lv_label_set_text_fmt(instance->ip_label, "%s", addr);
    } else {
        lv_label_set_text_fmt(instance->ip_label, "none");
    }
}

/* LVGL class descriptors */

const lv_obj_class_t wifi_info_view_front_lvgl_class = {
    .base_class = &widget_lvgl_class,
    .constructor_cb = wifi_info_view_front_lvgl_constructor,
    .destructor_cb = wifi_info_view_front_lvgl_destructor,
    .name = "widget-wifi-info-view-front",
    .width_def = LV_PCT(100),
    .height_def = LV_PCT(100),
    .instance_size = sizeof(WifiInfoView),
};

const lv_obj_class_t wifi_info_view_back_lvgl_class = {
    .base_class = &widget_lvgl_class,
    .constructor_cb = wifi_info_view_back_lvgl_constructor,
    .destructor_cb = wifi_info_view_back_lvgl_destructor,
    .name = "widget-wifi-info-view-back",
    .width_def = LV_PCT(100),
    .height_def = LV_SIZE_CONTENT,
    .instance_size = sizeof(WifiInfoView),
};

const lv_obj_class_t wifi_info_view_arrow_lvgl_class = {
    .base_class = &lv_label_class,
    .name = "wifi-info-arrow",
    .width_def = LV_SIZE_CONTENT,
    .height_def = LV_SIZE_CONTENT,
};
