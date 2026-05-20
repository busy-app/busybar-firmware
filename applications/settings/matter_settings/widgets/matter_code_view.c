#include "matter_code_view.h"
#include "../matter_settings_i.h"

#include <gui/widget_i.h>

#define MATTER_CODE_VIEW_MANUAL_CODE_COLOR lv_color_hex(0x666666)

#define MATTER_CODE_VIEW_ICON_IMAGE_PATH IMG_PATH("matter_back_21x21.image")
#define MATTER_CODE_VIEW_TEXT_IMAGE_PATH IMG_PATH("matter_text_back_42x8.image")

struct MatterCodeView {
    Widget base;

    lv_obj_t* qr_code;

    lv_obj_t* logo_icon_image;
    lv_obj_t* logo_text_image;

    lv_obj_t* manual_code_label;
    lv_obj_t* manual_title_label;
    lv_obj_t* state_line_label;

    FontRegistry* font_registry;
    const lv_font_t* font_busy_regular_7;
};

const lv_obj_class_t matter_code_view_lvgl_class;

// ==========
// Public API
// ==========

MatterCodeView* matter_code_view_alloc(Widget* parent) {
    furi_check(parent);

    lv_obj_t* obj = lv_obj_class_create_obj(&matter_code_view_lvgl_class, TO_LV_OBJ(parent));
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

void matter_code_view_set_codes(MatterCodeView* instance, const char* qr, const char* manual) {
    furi_check(instance);
    furi_check(qr);
    furi_check(manual);

    lv_label_set_text(instance->manual_code_label, manual);
    lv_qrcode_update(instance->qr_code, qr, strlen(qr));
}

// =================
// Class descriptors
// =================

static void matter_code_view_lvgl_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    UNUSED(class_p);

    MatterCodeView* instance = (MatterCodeView*)obj;
    lv_obj_set_style_pad_hor(obj, 3, LV_PART_MAIN);
    lv_obj_set_style_pad_top(obj, 7, LV_PART_MAIN);
    lv_obj_set_style_pad_bottom(obj, 5, LV_PART_MAIN);
    lv_obj_set_style_radius(obj, 4, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_bg_color(obj, lv_color_white(), LV_PART_MAIN);

    instance->font_registry = furi_record_open(RECORD_FONT_REGISTRY);
    instance->font_busy_regular_7 =
        font_registry_load_font(instance->font_registry, FONT_BUSY_REGULAR_7);

    instance->qr_code = lv_qrcode_create(obj);
    lv_qrcode_set_size(instance->qr_code, 66);
    lv_obj_set_style_align(instance->qr_code, LV_ALIGN_TOP_RIGHT, LV_PART_MAIN);

    instance->logo_icon_image = lv_image_create(obj);
    lv_image_set_src(instance->logo_icon_image, MATTER_CODE_VIEW_ICON_IMAGE_PATH);
    lv_obj_set_align(instance->logo_icon_image, LV_ALIGN_TOP_LEFT);

    instance->logo_text_image = lv_image_create(obj);
    lv_image_set_src(instance->logo_text_image, MATTER_CODE_VIEW_TEXT_IMAGE_PATH);
    lv_obj_set_style_pad_top(instance->logo_text_image, 1, LV_PART_MAIN);
    lv_obj_align_to(
        instance->logo_text_image, instance->logo_icon_image, LV_ALIGN_OUT_RIGHT_MID, 8, 0);

    instance->manual_code_label = lv_label_create(obj);
    lv_obj_set_style_align(instance->manual_code_label, LV_ALIGN_BOTTOM_LEFT, LV_PART_MAIN);
    lv_obj_set_style_text_color(
        instance->manual_code_label, MATTER_CODE_VIEW_MANUAL_CODE_COLOR, LV_PART_MAIN);
    lv_obj_set_style_text_font(
        instance->manual_code_label, instance->font_busy_regular_7, LV_PART_MAIN);

    instance->manual_title_label = lv_label_create(obj);
    lv_label_set_text(instance->manual_title_label, "Manual code:");
    lv_obj_align_to(
        instance->manual_title_label, instance->manual_code_label, LV_ALIGN_OUT_TOP_LEFT, 0, -3);
    lv_obj_set_style_text_color(
        instance->manual_title_label, MATTER_CODE_VIEW_MANUAL_CODE_COLOR, LV_PART_MAIN);
    lv_obj_set_style_text_font(
        instance->manual_title_label, instance->font_busy_regular_7, LV_PART_MAIN);

    instance->state_line_label = lv_label_create(obj);
    lv_label_set_text(instance->state_line_label, "Ready to pair");
    lv_obj_align_to(
        instance->state_line_label, instance->manual_title_label, LV_ALIGN_OUT_TOP_LEFT, 0, -6);
    lv_obj_set_style_text_color(instance->state_line_label, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_text_font(
        instance->state_line_label, instance->font_busy_regular_7, LV_PART_MAIN);
}

static void matter_code_view_lvgl_destructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    UNUSED(class_p);

    MatterCodeView* instance = (MatterCodeView*)obj;

    font_registry_unload_font(instance->font_registry, instance->font_busy_regular_7);

    furi_record_close(RECORD_FONT_REGISTRY);
}

const lv_obj_class_t matter_code_view_lvgl_class = {
    .base_class = &widget_lvgl_class,
    .constructor_cb = matter_code_view_lvgl_constructor,
    .destructor_cb = matter_code_view_lvgl_destructor,
    .name = "widget-matter-code-view",
    .width_def = LV_PCT(100),
    .height_def = LV_PCT(100),
    .instance_size = sizeof(MatterCodeView),
};
