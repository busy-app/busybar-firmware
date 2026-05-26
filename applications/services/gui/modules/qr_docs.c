#include "qr_docs.h"

#include <gui/widget_i.h>

#define MY_CLASS (&qr_docs_lvgl_class)

struct QrDocs {
    Widget base;
    lv_obj_t* image;
    lv_obj_t* message;
    lv_obj_t* qr_code;
};

const lv_obj_class_t qr_docs_lvgl_class;

/* LVGL-specific code */

static void qr_docs_lvgl_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    UNUSED(class_p);

    QrDocs* instance = (QrDocs*)obj;

    lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_bg_color(obj, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_scrollbar_mode(obj, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_flex_flow(obj, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(
        obj, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_ver(obj, 4, LV_PART_MAIN);
    lv_obj_set_style_pad_left(obj, 2, LV_PART_MAIN);
    lv_obj_set_style_pad_right(obj, 4, LV_PART_MAIN);

    lv_obj_t* text_column = lv_obj_create(obj);
    lv_obj_set_flex_flow(text_column, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(
        text_column, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_size(text_column, LV_PCT(50), LV_PCT(100));
    lv_obj_set_style_pad_row(text_column, 4, LV_PART_MAIN);
    lv_obj_set_scrollbar_mode(text_column, LV_SCROLLBAR_MODE_OFF);

    lv_obj_t* image = lv_image_create(text_column);
    lv_obj_set_style_pad_all(image, 2, LV_PART_MAIN);
    lv_obj_set_style_image_recolor_opa(image, LV_OPA_COVER, 0);
    lv_obj_set_style_image_recolor(image, lv_color_white(), LV_PART_MAIN);
    instance->image = image;

    lv_obj_t* message = lv_label_create(text_column);
    lv_obj_set_style_text_color(message, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_text_line_space(message, -1, LV_PART_MAIN);
    lv_obj_set_width(message, LV_PCT(100));
    lv_label_set_long_mode(message, LV_LABEL_LONG_WRAP);
    instance->message = message;

    lv_obj_t* qr_code = lv_qrcode_create(obj);
    lv_qrcode_set_size(qr_code, 66);
    lv_obj_set_style_radius(qr_code, 3, LV_PART_MAIN);
    lv_obj_set_style_pad_all(qr_code, 3, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(qr_code, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_bg_color(qr_code, lv_color_white(), LV_PART_MAIN);
    instance->qr_code = qr_code;
}

/* Public API */

QrDocs* qr_docs_alloc(Widget* parent) {
    furi_check(parent);

    lv_obj_t* obj = lv_obj_class_create_obj(MY_CLASS, TO_LV_OBJ(parent));
    lv_obj_class_init_obj(obj);

    QrDocs* instance = (QrDocs*)obj;

    return instance;
}

void qr_docs_free(QrDocs* instance) {
    furi_check(instance);

    lv_obj_delete(TO_LV_OBJ(instance));
}

void qr_docs_set_url(QrDocs* instance, const char* url) {
    furi_check(instance);
    furi_check(url);

    lv_qrcode_update(instance->qr_code, url, strlen(url));
}

void qr_docs_set_text(QrDocs* instance, const char* text) {
    furi_check(instance);
    furi_check(text);

    lv_label_set_text(instance->message, text);
}

void qr_docs_set_image(QrDocs* instance, const char* path) {
    furi_check(instance);
    furi_check(path);

    lv_image_set_src(instance->image, path);
}

/* LVGL class descriptors */

const lv_obj_class_t qr_docs_lvgl_class = {
    .base_class = &widget_lvgl_class,
    .constructor_cb = qr_docs_lvgl_constructor,
    .name = "qr-docs",
    .width_def = LV_PCT(100),
    .height_def = LV_PCT(100),
    .instance_size = sizeof(QrDocs),
};
