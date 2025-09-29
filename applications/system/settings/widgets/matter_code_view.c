#include "matter_code_view.h"

#include <gui/widget_i.h>
#include <assets_images.h>

#define MY_CLASS           (&matter_code_view_lvgl_class)
#define MY_LOGO_CLASS      (&matter_code_view_logo_lvgl_class)
#define MY_WORDMARK_CLASS  (&matter_code_view_wordmark_lvgl_class)
#define MY_MAN_TITLE_CLASS (&matter_code_view_man_title_lvgl_class)
#define MY_MAN_CODE_CLASS  (&matter_code_view_man_code_lvgl_class)
#define MY_QR_CODE_CLASS   (&matter_code_view_qr_code_lvgl_class)

struct MatterCodeView {
    Widget base;
    lv_obj_t* logo;
    lv_obj_t* wordmark;
    lv_obj_t* man_title;
    lv_obj_t* man_code;
    lv_obj_t* qr_code;
};

const lv_obj_class_t matter_code_view_lvgl_class;
const lv_obj_class_t matter_code_view_logo_lvgl_class;
const lv_obj_class_t matter_code_view_wordmark_lvgl_class;
const lv_obj_class_t matter_code_view_man_title_lvgl_class;
const lv_obj_class_t matter_code_view_man_code_lvgl_class;
const lv_obj_class_t matter_code_view_qr_code_lvgl_class;

// ==========
// Public API
// ==========

MatterCodeView* matter_code_view_alloc(Widget* parent) {
    furi_check(parent);

    lv_obj_t* obj = lv_obj_class_create_obj(MY_CLASS, TO_LV_OBJ(parent));
    lv_obj_class_init_obj(obj);

    MatterCodeView* instance = (MatterCodeView*)obj;
    return instance;
}

void matter_code_view_free(MatterCodeView* instance) {
    furi_check(instance);
    lv_obj_delete(TO_LV_OBJ(instance));
}

Widget* matter_code_view_get_base(MatterCodeView* instance) {
    furi_check(instance);
    return &instance->base;
}

void matter_code_view_set_logo_path(MatterCodeView* instance, const char* path) {
    furi_check(instance);
    furi_check(path);
    lv_image_set_src(instance->logo, path);
}

void matter_code_view_set_codes(MatterCodeView* instance, const char* qr, const char* manual) {
    furi_check(instance);
    furi_check(qr);
    furi_check(manual);

    lv_label_set_text(instance->man_code, manual);
    lv_qrcode_set_size(instance->qr_code, 50);
    lv_qrcode_update(instance->qr_code, qr, strlen(qr));
}

// =================
// Class descriptors
// =================

static void matter_code_view_lvgl_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    UNUSED(class_p);

    MatterCodeView* instance = (MatterCodeView*)obj;

    instance->logo = lv_obj_class_create_obj(MY_LOGO_CLASS, obj);
    lv_obj_class_init_obj(instance->logo);

    instance->wordmark = lv_obj_class_create_obj(MY_WORDMARK_CLASS, obj);
    lv_obj_class_init_obj(instance->wordmark);
    lv_label_set_text(instance->wordmark, "matter");

    instance->man_title = lv_obj_class_create_obj(MY_MAN_TITLE_CLASS, obj);
    lv_obj_class_init_obj(instance->man_title);
    lv_label_set_text(instance->man_title, "Manual code");

    instance->man_code = lv_obj_class_create_obj(MY_MAN_CODE_CLASS, obj);
    lv_obj_class_init_obj(instance->man_code);

    instance->qr_code = lv_obj_class_create_obj(MY_QR_CODE_CLASS, obj);
    lv_obj_class_init_obj(instance->qr_code);
}

const lv_obj_class_t matter_code_view_lvgl_class = {
    .base_class = &widget_lvgl_class,
    .constructor_cb = matter_code_view_lvgl_constructor,
    .name = "widget-status-view",
    .width_def = LV_PCT(100),
    .height_def = LV_PCT(100),
    .instance_size = sizeof(MatterCodeView),
};

const lv_obj_class_t matter_code_view_logo_lvgl_class = {
    .base_class = &lv_image_class,
    .name = "matter-code-view-logo",
    .width_def = LV_SIZE_CONTENT,
    .height_def = LV_SIZE_CONTENT,
};

const lv_obj_class_t matter_code_view_wordmark_lvgl_class = {
    .base_class = &lv_label_class,
    .name = "matter-code-view-wordmark",
    .width_def = LV_SIZE_CONTENT,
    .height_def = LV_SIZE_CONTENT,
};

const lv_obj_class_t matter_code_view_man_title_lvgl_class = {
    .base_class = &lv_label_class,
    .name = "matter-code-view-man-title",
    .width_def = LV_SIZE_CONTENT,
    .height_def = LV_SIZE_CONTENT,
};

const lv_obj_class_t matter_code_view_man_code_lvgl_class = {
    .base_class = &lv_label_class,
    .name = "matter-code-view-man-code",
    .width_def = LV_SIZE_CONTENT,
    .height_def = LV_SIZE_CONTENT,
};

const lv_obj_class_t matter_code_view_qr_code_lvgl_class = {
    .base_class = &lv_qrcode_class,
    .name = "matter-code-view-qr-code",
    .width_def = LV_SIZE_CONTENT,
    .height_def = LV_SIZE_CONTENT,
};
