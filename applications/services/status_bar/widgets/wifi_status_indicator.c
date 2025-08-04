#include "wifi_status_indicator.h"
#include "../storage_macros.h"

#include <gui/widget_i.h>

#define MY_CLASS (&wifi_status_indicator_lvgl_class)

struct WifiStatusIndicator {
    Widget base;
    lv_obj_t* wifi_image;
};

const lv_obj_class_t wifi_status_indicator_lvgl_class;

/* LVGL-specific code */

static void wifi_status_indicator_lvgl_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    UNUSED(class_p);

    WifiStatusIndicator* instance = (WifiStatusIndicator*)obj;

    instance->wifi_image = lv_image_create(obj);
    lv_obj_center(instance->wifi_image);
    lv_image_set_src(instance->wifi_image, STATUS_BAR_IMG_PATH("wifi_8x8.bin"));
}

/* Public API */

WifiStatusIndicator* wifi_status_indicator_alloc(Widget* parent) {
    furi_assert(parent);

    lv_obj_t* obj = lv_obj_class_create_obj(MY_CLASS, (lv_obj_t*)parent);
    lv_obj_class_init_obj(obj);

    WifiStatusIndicator* instance = (WifiStatusIndicator*)obj;
    return instance;
}

void wifi_status_indicator_free(WifiStatusIndicator* instance) {
    furi_assert(instance);
    lv_obj_delete((lv_obj_t*)instance);
}

Widget* wifi_status_indicator_get_base(WifiStatusIndicator* instance) {
    furi_assert(instance);
    return (Widget*)instance;
}

const lv_obj_class_t wifi_status_indicator_lvgl_class = {
    .base_class = &widget_lvgl_class,
    .constructor_cb = wifi_status_indicator_lvgl_constructor,
    .name = "widget-wifi-state-indicator",
    .width_def = LV_PCT(100),
    .height_def = LV_SIZE_CONTENT,
    .instance_size = sizeof(WifiStatusIndicator),
};
