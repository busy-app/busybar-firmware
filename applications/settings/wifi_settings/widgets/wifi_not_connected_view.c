#include "wifi_not_connected_view.h"
#include <storage/storage.h>
#include "../wifi_settings.h"
#include <m-array.h>
#include <gui/widget_i.h>

#define WIFI_INFO_BACK_CLASS (&wifi_not_connected_view_back_lvgl_class)

struct WifiNotConnectedView {
    Widget base;
};

const lv_obj_class_t wifi_not_connected_view_back_lvgl_class;

/* LVGL-specific code */

static void
    wifi_not_connected_view_back_lvgl_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    UNUSED(class_p);
    WifiNotConnectedView* instance = (WifiNotConnectedView*)obj;
    UNUSED(instance);

    lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_bg_color(obj, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_scrollbar_mode(obj, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_flex_flow(obj, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(
        obj, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_all(obj, 4, LV_PART_MAIN);

    lv_obj_t* text_column = lv_obj_create(obj);
    lv_obj_set_flex_flow(text_column, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(
        text_column, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_size(text_column, LV_PCT(50), LV_PCT(100));
    lv_obj_set_style_pad_ver(text_column, 6, LV_PART_MAIN);
    lv_obj_set_style_pad_gap(text_column, 9, LV_PART_MAIN);
    lv_obj_set_scrollbar_mode(text_column, LV_SCROLLBAR_MODE_OFF);

    lv_obj_t* logo = lv_image_create(text_column);
    lv_image_set_src(logo, IMG_PATH("wifi_back_12x12.bin"));
    lv_obj_set_style_image_recolor_opa(logo, LV_OPA_COVER, 0);
    lv_obj_set_style_image_recolor(logo, lv_color_white(), LV_PART_MAIN);

    lv_obj_t* message = lv_label_create(text_column);
    lv_label_set_text(message, "Connect to  Wi-Fi via PC or BUSY App");
    lv_obj_set_style_text_color(message, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_width(message, LV_PCT(100));
    lv_label_set_long_mode(message, LV_LABEL_LONG_WRAP);

    lv_obj_t* qr_code = lv_qrcode_create(obj);
    lv_qrcode_set_size(qr_code, 66);
    lv_obj_set_style_radius(qr_code, 3, LV_PART_MAIN);
    lv_obj_set_style_pad_all(qr_code, 3, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(qr_code, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_bg_color(qr_code, lv_color_white(), LV_PART_MAIN);

    const char* qr_text = "https://docs.busy.app/bar/basics/connect-wifi";
    lv_qrcode_update(qr_code, qr_text, strlen(qr_text));
}

/* Public API */

WifiNotConnectedView* wifi_not_connected_view_back_alloc(Widget* parent) {
    furi_check(parent);

    lv_obj_t* obj = lv_obj_class_create_obj(WIFI_INFO_BACK_CLASS, TO_LV_OBJ(parent));
    lv_obj_class_init_obj(obj);

    return (WifiNotConnectedView*)obj;
}

void wifi_not_connected_view_free(WifiNotConnectedView* instance) {
    furi_check(instance);
    lv_obj_delete(TO_LV_OBJ(instance));
}

/* LVGL class descriptors */

const lv_obj_class_t wifi_not_connected_view_back_lvgl_class = {
    .base_class = &widget_lvgl_class,
    .constructor_cb = wifi_not_connected_view_back_lvgl_constructor,
    .name = "widget-wifi-not-connected-view-back",
    .width_def = LV_PCT(100),
    .height_def = LV_PCT(100),
    .instance_size = sizeof(WifiNotConnectedView),
};
