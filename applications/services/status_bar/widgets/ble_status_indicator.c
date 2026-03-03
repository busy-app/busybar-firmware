#include "ble_status_indicator.h"
#include "../storage_macros.h"

#include <gui/widget_i.h>

#define MY_CLASS (&ble_status_indicator_lvgl_class)

struct BleStatusIndicator {
    Widget base;
    lv_obj_t* ble_image;
    BleStatusIndicatorState state;
};

const lv_obj_class_t ble_status_indicator_lvgl_class;

/* LVGL-specific code */

static void ble_status_indicator_lvgl_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    UNUSED(class_p);

    BleStatusIndicator* instance = (BleStatusIndicator*)obj;

    instance->ble_image = lv_image_create(obj);
    lv_obj_center(instance->ble_image);
    lv_obj_add_flag(obj, LV_OBJ_FLAG_HIDDEN);
}

/* Public API */

BleStatusIndicator* ble_status_indicator_alloc(Widget* parent) {
    furi_assert(parent);

    lv_obj_t* obj = lv_obj_class_create_obj(MY_CLASS, (lv_obj_t*)parent);
    lv_obj_class_init_obj(obj);

    BleStatusIndicator* instance = (BleStatusIndicator*)obj;
    return instance;
}

void ble_status_indicator_free(BleStatusIndicator* instance) {
    furi_assert(instance);
    lv_obj_delete((lv_obj_t*)instance);
}

Widget* ble_status_indicator_get_base(BleStatusIndicator* instance) {
    furi_assert(instance);
    return (Widget*)instance;
}

void ble_status_indicator_set_state(BleStatusIndicator* instance, BleStatusIndicatorState state) {
    furi_assert(instance);
    furi_assert(state < BleStatusIndicatorStateMax);

    if(state != instance->state) {
        if(state == BleStatusIndicatorStateDisconnected) {
            lv_image_set_src(instance->ble_image, STATUS_BAR_IMG_PATH("ble_disconnected_8x8.bin"));
        } else if(state == BleStatusIndicatorStateConnectable) {
            lv_image_set_src(instance->ble_image, STATUS_BAR_IMG_PATH("ble_connecting_8x8.bin"));
        } else if(state == BleStatusIndicatorStateConnected) {
            lv_image_set_src(instance->ble_image, STATUS_BAR_IMG_PATH("ble_8x8.bin"));
        }

        const bool is_hidden = (state == BleStatusIndicatorStateUnknown);
        lv_obj_update_flag(TO_LV_OBJ(instance), LV_OBJ_FLAG_HIDDEN, is_hidden);

        instance->state = state;
    }
}

const lv_obj_class_t ble_status_indicator_lvgl_class = {
    .base_class = &widget_lvgl_class,
    .constructor_cb = ble_status_indicator_lvgl_constructor,
    .name = "widget-ble-state-indicator",
    .width_def = LV_PCT(100),
    .height_def = LV_SIZE_CONTENT,
    .instance_size = sizeof(BleStatusIndicator),
};
