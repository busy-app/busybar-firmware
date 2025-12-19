#include "wifi_state_view.h"
#include "../wifi_settings.h"
#include <storage/storage.h>
#include <m-array.h>
#include <gui/widget_i.h>

#define WIFI_STATE_BACK_CLASS  (&wifi_state_view_back_lvgl_class)
#define WIFI_STATE_FRONT_CLASS (&wifi_state_view_front_lvgl_class)

#define COLOR_OK    lv_color_hex(0x00B8D9)
#define COLOR_ERROR lv_color_hex(0xED0018)

struct WifiStateView {
    Widget base;
    lv_obj_t* state_label;
    lv_obj_t* ssid_label;
    lv_obj_t* state_image;
    bool is_back;
};

const lv_obj_class_t wifi_state_view_back_lvgl_class;
const lv_obj_class_t wifi_state_view_front_lvgl_class;

/* LVGL-specific code */

static void wifi_state_view_front_lvgl_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    UNUSED(class_p);
    WifiStateView* instance = (WifiStateView*)obj;
    instance->is_back = false;

    lv_obj_align(obj, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_set_flex_flow(obj, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(obj, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_set_style_pad_all(obj, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_gap(obj, 2, LV_PART_MAIN);

    instance->state_image = lv_img_create(obj);
    lv_img_set_src(instance->state_image, IMG_PATH("wifi_front_ok_8x8.bin"));
    lv_obj_set_flex_grow(instance->state_image, 0);

    lv_obj_t* text_cont = lv_obj_create(obj);
    lv_obj_set_flex_flow(text_cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(
        text_cont, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_size(text_cont, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_flex_grow(text_cont, 1);
    lv_obj_set_style_pad_all(text_cont, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_gap(text_cont, 0, LV_PART_MAIN);

    const lv_font_t* font = lv_theme_get_font_normal(obj);

    instance->state_label = lv_label_create(text_cont);
    lv_label_set_text(instance->state_label, "Connected");
    lv_obj_set_style_text_color(instance->state_label, COLOR_OK, LV_PART_MAIN);
    lv_obj_set_style_text_font(instance->state_label, font, LV_PART_MAIN);

    instance->ssid_label = lv_label_create(text_cont);
    lv_label_set_text(instance->ssid_label, "");
    lv_obj_set_style_text_color(instance->ssid_label, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_text_font(instance->ssid_label, font, LV_PART_MAIN);

    lv_obj_t* arrow = lv_label_create(obj);
    lv_label_set_text(arrow, ">");
    lv_obj_set_style_text_color(arrow, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_text_font(arrow, font, LV_PART_MAIN);
    lv_obj_set_flex_grow(arrow, 0);
}

static void wifi_state_view_back_lvgl_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    UNUSED(class_p);
    WifiStateView* instance = (WifiStateView*)obj;
    instance->is_back = true;

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

    instance->state_image = lv_img_create(obj);
    lv_img_set_src(instance->state_image, IMG_PATH("wifi_back_ok_12x12.bin"));
    lv_obj_set_flex_grow(instance->state_image, 0);

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

    instance->state_label = lv_label_create(text_cont);
    lv_label_set_text(instance->state_label, "Connected");
    lv_obj_set_style_text_color(instance->state_label, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_text_font(instance->state_label, font, LV_PART_MAIN);

    instance->ssid_label = lv_label_create(text_cont);
    lv_label_set_text(instance->ssid_label, "");
    lv_obj_set_style_text_color(instance->ssid_label, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_text_font(instance->ssid_label, font, LV_PART_MAIN);

    lv_obj_t* arrow = lv_label_create(obj);
    lv_label_set_text(arrow, ">");
    lv_obj_set_style_text_color(arrow, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_text_font(arrow, font, LV_PART_MAIN);
    lv_obj_set_flex_grow(arrow, 0);
}

/* Public API */

WifiStateView* wifi_state_view_alloc_common(Widget* parent, const lv_obj_class_t* class_p) {
    furi_check(parent);

    lv_obj_t* obj = lv_obj_class_create_obj(class_p, TO_LV_OBJ(parent));
    lv_obj_class_init_obj(obj);

    WifiStateView* instance = (WifiStateView*)obj;

    return instance;
}

WifiStateView* wifi_state_view_front_alloc(Widget* parent) {
    return wifi_state_view_alloc_common(parent, WIFI_STATE_FRONT_CLASS);
}

WifiStateView* wifi_state_view_back_alloc(Widget* parent) {
    return wifi_state_view_alloc_common(parent, WIFI_STATE_BACK_CLASS);
}

void wifi_state_view_free(WifiStateView* instance) {
    furi_check(instance);
    lv_obj_delete(TO_LV_OBJ(instance));
}

void wifi_state_view_set_state(WifiStateView* instance, bool connected, const char* ssid) {
    furi_check(instance);

    if(connected) {
        lv_label_set_text_fmt(instance->state_label, "Connected");
        if(instance->is_back) {
            lv_image_set_src(instance->state_image, IMG_PATH("wifi_back_ok_12x12.bin"));
        } else {
            lv_image_set_src(instance->state_image, IMG_PATH("wifi_front_ok_8x8.bin"));
        }
    } else {
        lv_label_set_text_fmt(instance->state_label, "Offline");
        if(instance->is_back) {
            lv_image_set_src(instance->state_image, IMG_PATH("wifi_back_error_12x12.bin"));
        } else {
            lv_image_set_src(instance->state_image, IMG_PATH("wifi_front_error_8x8.bin"));
        }
    }

    if(!instance->is_back) {
        lv_obj_set_style_text_color(
            instance->state_label, connected ? COLOR_OK : COLOR_ERROR, LV_PART_MAIN);
    }

    if(ssid) lv_label_set_text_fmt(instance->ssid_label, "%s", ssid);
}

/* LVGL class descriptors */

const lv_obj_class_t wifi_state_view_front_lvgl_class = {
    .base_class = &widget_lvgl_class,
    .constructor_cb = wifi_state_view_front_lvgl_constructor,
    .name = "widget-wifi-state-view-front",
    .width_def = LV_PCT(100),
    .height_def = LV_PCT(100),
    .instance_size = sizeof(WifiStateView),
};

const lv_obj_class_t wifi_state_view_back_lvgl_class = {
    .base_class = &widget_lvgl_class,
    .constructor_cb = wifi_state_view_back_lvgl_constructor,
    .name = "widget-wifi-state-view-back",
    .width_def = LV_PCT(95),
    .height_def = LV_PCT(50),
    .instance_size = sizeof(WifiStateView),
};
