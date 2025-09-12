#include "qr_code.h"

#include <gui/widget_i.h>

#define MY_CLASS (&qr_code_lvgl_class)

struct QRCode {
    Widget base;
    lv_obj_t* qr_code;
};

const lv_obj_class_t qr_code_lvgl_class;

// LVGL-specific functions

static void qr_code_lvgl_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    UNUSED(class_p);

    QRCode* instance = (QRCode*)obj;
    instance->qr_code = lv_qrcode_create(obj);
}

// Public API

QRCode* qr_code_alloc(Widget* parent) {
    furi_check(parent);

    lv_obj_t* obj = lv_obj_class_create_obj(MY_CLASS, (lv_obj_t*)parent);
    lv_obj_class_init_obj(obj);

    QRCode* instance = (QRCode*)obj;
    return instance;
}

void qr_code_free(QRCode* instance) {
    furi_check(instance);
    lv_obj_delete((lv_obj_t*)instance);
}

Widget* qr_code_get_base(QRCode* instance) {
    furi_check(instance);
    return (Widget*)instance;
}

void qr_code_set_size(QRCode* instance, int32_t size) {
    furi_check(instance);

    lv_qrcode_set_size(instance->qr_code, size);
}

void qr_code_set_data(QRCode* instance, const char* data) {
    furi_check(instance);
    furi_check(data);

    lv_qrcode_update(instance->qr_code, data, strlen(data));
}

// LVGL class descriptor

const lv_obj_class_t qr_code_lvgl_class = {
    .base_class = &widget_lvgl_class,
    .constructor_cb = qr_code_lvgl_constructor,
    .name = "widget-qr_code",
    .width_def = LV_SIZE_CONTENT,
    .height_def = LV_SIZE_CONTENT,
    .instance_size = sizeof(QRCode),
};
