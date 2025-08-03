#include "usb_status_indicator.h"
#include "../storage_macros.h"

#include <gui/widget_i.h>

#define MY_CLASS (&usb_status_indicator_lvgl_class)

struct UsbStatusIndicator {
    Widget base;
    lv_obj_t* usb_image;
};

const lv_obj_class_t usb_status_indicator_lvgl_class;

/* LVGL-specific code */

static void usb_status_indicator_lvgl_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    UNUSED(class_p);

    UsbStatusIndicator* instance = (UsbStatusIndicator*)obj;

    instance->usb_image = lv_image_create(obj);
    lv_obj_center(instance->usb_image);
    lv_image_set_src(instance->usb_image, STATUS_BAR_IMG_PATH("usb_8x8.bin"));
}

/* Public API */

UsbStatusIndicator* usb_status_indicator_alloc(Widget* parent) {
    furi_assert(parent);

    lv_obj_t* obj = lv_obj_class_create_obj(MY_CLASS, (lv_obj_t*)parent);
    lv_obj_class_init_obj(obj);

    UsbStatusIndicator* instance = (UsbStatusIndicator*)obj;
    return instance;
}

void usb_status_indicator_free(UsbStatusIndicator* instance) {
    furi_assert(instance);
    lv_obj_delete((lv_obj_t*)instance);
}

Widget* usb_status_indicator_get_base(UsbStatusIndicator* instance) {
    furi_assert(instance);
    return (Widget*)instance;
}

void usb_status_indicator_set_connection_state(UsbStatusIndicator* instance, bool is_connected) {
    furi_assert(instance);

    lv_obj_update_flag((lv_obj_t*)&instance->base, LV_OBJ_FLAG_HIDDEN, !is_connected);
}

const lv_obj_class_t usb_status_indicator_lvgl_class = {
    .base_class = &widget_lvgl_class,
    .constructor_cb = usb_status_indicator_lvgl_constructor,
    .name = "widget-usb-state-indicator",
    .width_def = LV_PCT(100),
    .height_def = LV_SIZE_CONTENT,
    .instance_size = sizeof(UsbStatusIndicator),
};
