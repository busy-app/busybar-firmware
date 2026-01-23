#include "named_label_view.h"
#include <gui/widget_i.h>

#define NAME_LABEL_ANIMATION_DURATION_MS (3000)
#define NAMED_LABEL_BACK_CLASS           (&named_label_view_back_lvgl_class)

struct NamedLabelView {
    Widget base;
    lv_obj_t* title_obj;
    lv_obj_t* text_obj;
};

const lv_obj_class_t named_label_view_back_lvgl_class;

/* LVGL-specific code */

static void named_label_view_back_lvgl_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    UNUSED(class_p);
    NamedLabelView* instance = (NamedLabelView*)obj;
    UNUSED(instance);

    lv_obj_set_flex_flow(obj, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(obj, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_height(obj, LV_SIZE_CONTENT);
    lv_obj_set_style_pad_ver(obj, 2, LV_PART_MAIN);
    lv_obj_set_scrollbar_mode(obj, LV_SCROLLBAR_MODE_OFF);

    lv_obj_t* title = lv_label_create(obj);
    lv_obj_set_width(title, LV_SIZE_CONTENT);
    lv_label_set_long_mode(title, LV_LABEL_LONG_WRAP);
    instance->title_obj = title;

    lv_obj_t* message = lv_label_create(obj);
    lv_obj_set_style_anim_time(message, NAME_LABEL_ANIMATION_DURATION_MS, LV_PART_MAIN);
    lv_obj_set_width(title, LV_SIZE_CONTENT);
    lv_label_set_long_mode(message, LV_LABEL_LONG_SCROLL);
    instance->text_obj = message;
}

/* Public API */

NamedLabelView* named_label_view_back_alloc(Widget* parent) {
    furi_check(parent);

    lv_obj_t* obj = lv_obj_class_create_obj(NAMED_LABEL_BACK_CLASS, TO_LV_OBJ(parent));
    lv_obj_class_init_obj(obj);

    return (NamedLabelView*)obj;
}

void named_label_view_back_free(NamedLabelView* instance) {
    furi_check(instance);
    lv_obj_delete(TO_LV_OBJ(instance));
}

void named_label_set_title(NamedLabelView* instance, const char* title) {
    furi_check(instance);
    furi_check(title);
    lv_label_set_text(instance->title_obj, title);
}

void named_label_set_text(NamedLabelView* instance, const char* text) {
    furi_check(instance);
    furi_check(text);
    lv_label_set_text(instance->text_obj, text);

    lv_obj_t* obj = TO_LV_OBJ(instance);
    lv_obj_update_layout(obj);

    int32_t flex_w = lv_obj_get_width(obj);
    int32_t title_w = lv_obj_get_width(instance->title_obj);
    int32_t text_w = lv_obj_get_width(instance->text_obj);

    if(title_w + text_w > flex_w) {
        lv_obj_set_width(instance->text_obj, flex_w - title_w);
    } else {
        lv_obj_set_width(instance->text_obj, LV_SIZE_CONTENT);
    }
}

void named_label_set_text_color(NamedLabelView* instance, Color color) {
    furi_check(instance);
    lv_obj_set_style_text_color((lv_obj_t*)instance, TO_LV_COLOR(color), LV_PART_MAIN);
    lv_obj_set_style_text_opa((lv_obj_t*)instance, color.a, LV_PART_MAIN);
}

/* LVGL class descriptors */

const lv_obj_class_t named_label_view_back_lvgl_class = {
    .base_class = &widget_lvgl_class,
    .constructor_cb = named_label_view_back_lvgl_constructor,
    .name = "widget-named-label-view-back",
    .width_def = LV_PCT(100),
    .height_def = LV_PCT(100),
    .instance_size = sizeof(NamedLabelView),
};
