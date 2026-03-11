#include "title_card.h"
#include "../widget_i.h"
#include <font_registry/font_registry.h>

#define MY_CLASS       (&title_card_lvgl_class)
#define MY_LABEL_CLASS (&title_card_label_lvgl_class)

struct TitleCard {
    Widget base;

    lv_obj_t* icon_image;
    lv_obj_t* title_label;

    FontRegistry* font_registry;
};

const lv_obj_class_t title_card_lvgl_class;
const lv_obj_class_t title_card_label_lvgl_class;

/* LVGL-specific code */

static void title_card_lvgl_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    UNUSED(class_p);

    TitleCard* instance = (TitleCard*)obj;

    instance->font_registry = furi_record_open(RECORD_FONT_REGISTRY);

    lv_obj_set_flex_flow(obj, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(obj, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    instance->icon_image = lv_image_create(obj);

    instance->title_label = lv_obj_class_create_obj(MY_LABEL_CLASS, obj);
    lv_obj_class_init_obj(instance->title_label);
    lv_obj_set_style_text_font(
        instance->title_label,
        font_registry_load_font(instance->font_registry, FONT_BUSY_REGULAR_14),
        LV_PART_MAIN);
    lv_obj_set_style_text_color(instance->title_label, lv_color_white(), LV_PART_MAIN);
}

static void title_card_lvgl_destructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    UNUSED(class_p);

    TitleCard* instance = (TitleCard*)obj;

    font_registry_unload_font(
        instance->font_registry, lv_obj_get_style_text_font(instance->title_label, LV_PART_MAIN));

    furi_record_close(RECORD_FONT_REGISTRY);
}

/* Public API */

TitleCard* title_card_alloc(Widget* parent) {
    furi_check(parent);

    lv_obj_t* obj = lv_obj_class_create_obj(MY_CLASS, TO_LV_OBJ(parent));
    lv_obj_class_init_obj(obj);

    TitleCard* instance = (TitleCard*)obj;

    return instance;
}

void title_card_free(TitleCard* instance) {
    furi_check(instance);
    lv_obj_delete(TO_LV_OBJ(instance));
}

Widget* title_card_get_base(TitleCard* instance) {
    furi_check(instance);
    return &instance->base;
}

void title_card_set_icon(TitleCard* instance, const char* file_path) {
    furi_check(instance);
    furi_check(file_path);

    lv_image_set_src(instance->icon_image, file_path);
}

void title_card_set_title(TitleCard* instance, const char* title) {
    furi_check(instance);
    furi_check(title);

    lv_label_set_text(instance->title_label, title);
}

/* LVGL class descriptor */

const lv_obj_class_t title_card_lvgl_class = {
    .base_class = &widget_lvgl_class,
    .constructor_cb = title_card_lvgl_constructor,
    .destructor_cb = title_card_lvgl_destructor,
    .name = "widget-title-card",
    .width_def = LV_PCT(100),
    .height_def = LV_PCT(100),
    .instance_size = sizeof(TitleCard),
};

const lv_obj_class_t title_card_label_lvgl_class = {
    .base_class = &lv_label_class,
    .name = "title-card-label",
    .width_def = LV_SIZE_CONTENT,
    .height_def = LV_SIZE_CONTENT,
};
