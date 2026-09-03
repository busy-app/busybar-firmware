#include "nav_bar.h"
#include "../widget_i.h"

#include <m-array.h>

#include <toolbox/m_cstr_dup.h>

#define MY_CLASS (&nav_bar_lvgl_class)

#define COLOR_INACTIVE 777777
#define COLOR_ACTIVE   FFFFFF

#define COLOR_INACTIVE_HEX CONCATENATE(0x, COLOR_INACTIVE)
#define COLOR_ACTIVE_HEX   CONCATENATE(0x, COLOR_ACTIVE)

#define BREADCRUMBS_SPACER "\u200A"

ARRAY_DEF(LocationStack, const char*, M_CSTR_DUP_OPLIST);

struct NavBar {
    Widget base;
    lv_obj_t* header_image;
    lv_obj_t* header_label;
    lv_obj_t* breadcrumbs_label;

    LocationStack_t locations;
};

const lv_obj_class_t nav_bar_lvgl_class;

/* LVGL-specific code */

static void nav_bar_lvgl_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    UNUSED(class_p);

    NavBar* instance = (NavBar*)obj;
    lv_obj_set_flex_flow(obj, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(obj, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(obj, 3, LV_PART_MAIN);

    instance->header_image = lv_image_create(obj);
    lv_obj_add_flag(instance->header_image, LV_OBJ_FLAG_HIDDEN);

    instance->header_label = lv_label_create(obj);
    lv_obj_set_style_text_font(instance->header_label, lv_theme_get_font_small(obj), LV_PART_MAIN);
    lv_obj_set_style_text_align(instance->header_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_set_style_text_color(
        instance->header_label, lv_color_hex(COLOR_INACTIVE_HEX), LV_PART_MAIN);
    lv_obj_set_style_text_letter_space(instance->header_label, 1, LV_PART_MAIN);
    lv_obj_set_style_pad_top(instance->header_label, 1, LV_PART_MAIN);

    instance->breadcrumbs_label = lv_label_create(obj);
    lv_obj_set_style_text_font(
        instance->breadcrumbs_label, lv_theme_get_font_small(obj), LV_PART_MAIN);
    lv_obj_set_style_text_color(
        instance->breadcrumbs_label, lv_color_hex(COLOR_INACTIVE_HEX), LV_PART_MAIN);
    lv_label_set_recolor(instance->breadcrumbs_label, true);
    lv_obj_set_style_text_letter_space(instance->breadcrumbs_label, 1, LV_PART_MAIN);
    lv_obj_set_style_pad_top(instance->breadcrumbs_label, 1, LV_PART_MAIN);
    lv_obj_add_flag(instance->breadcrumbs_label, LV_OBJ_FLAG_HIDDEN);

    LocationStack_init(instance->locations);
}

static void nav_bar_lvgl_destructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    UNUSED(class_p);

    NavBar* instance = (NavBar*)obj;
    LocationStack_clear(instance->locations);
}

/* implementation */

static void nav_bar_update_breadcrumbs(NavBar* instance) {
    if(LocationStack_size(instance->locations) > 0) {
        FuriString* text = furi_string_alloc_set(">");

        LocationStack_it_t it;
        LocationStack_it(it, instance->locations);
        for(; !LocationStack_last_p(it); LocationStack_next(it)) {
            furi_string_cat_printf(
                text, BREADCRUMBS_SPACER "%s" BREADCRUMBS_SPACER ">", *LocationStack_cref(it));
        }

        furi_string_cat_printf(
            text, BREADCRUMBS_SPACER "#" TOSTRING(COLOR_ACTIVE) " %s #", *LocationStack_cref(it));

        lv_label_set_text(instance->breadcrumbs_label, furi_string_get_cstr(text));
        lv_obj_remove_flag(instance->breadcrumbs_label, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(instance->header_label, LV_OBJ_FLAG_HIDDEN);

        furi_string_free(text);
    } else {
        lv_obj_add_flag(instance->breadcrumbs_label, LV_OBJ_FLAG_HIDDEN);
        lv_obj_remove_flag(instance->header_label, LV_OBJ_FLAG_HIDDEN);
    }
}

/* public API */

NavBar* nav_bar_alloc(Widget* parent) {
    furi_check(parent);

    lv_obj_t* obj = lv_obj_class_create_obj(MY_CLASS, TO_LV_OBJ(parent));
    lv_obj_class_init_obj(obj);

    NavBar* instance = (NavBar*)obj;

    return instance;
}

void nav_bar_free(NavBar* instance) {
    furi_check(instance);
    lv_obj_delete(TO_LV_OBJ(instance));
}

Widget* nav_bar_get_base(NavBar* instance) {
    furi_check(instance);
    return &instance->base;
}

void nav_bar_set_header_image(NavBar* instance, const char* file_path) {
    furi_check(instance);

    if(file_path) {
        lv_image_set_src(instance->header_image, file_path);
        lv_obj_remove_flag(instance->header_image, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_add_flag(instance->header_image, LV_OBJ_FLAG_HIDDEN);
        lv_image_set_src(instance->header_image, NULL);
    }
}

void nav_bar_set_header_text(NavBar* instance, const char* text) {
    furi_check(instance);

    if(text) {
        lv_label_set_text(instance->header_label, text);
    } else {
        lv_label_set_text_static(instance->header_label, "");
    }
}

void nav_bar_set_visible(NavBar* instance, bool visible) {
    furi_check(instance);

    if(visible) {
        lv_obj_clear_flag(TO_LV_OBJ(instance), LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_add_flag(TO_LV_OBJ(instance), LV_OBJ_FLAG_HIDDEN);
    }
}

void nav_bar_push_location(NavBar* instance, const char* location_name) {
    furi_check(instance);
    furi_check(location_name);

    LocationStack_push_back(instance->locations, location_name);
    nav_bar_update_breadcrumbs(instance);
}

void nav_bar_pop_location(NavBar* instance) {
    furi_check(instance);
    furi_check(LocationStack_size(instance->locations) > 0);

    LocationStack_pop_back(NULL, instance->locations);
    nav_bar_update_breadcrumbs(instance);
}

void nav_bar_reset_location(NavBar* instance) {
    furi_check(instance);

    LocationStack_reset(instance->locations);
    nav_bar_update_breadcrumbs(instance);
}

/* LVGL class descriptor */

const lv_obj_class_t nav_bar_lvgl_class = {
    .base_class = &widget_lvgl_class,
    .constructor_cb = nav_bar_lvgl_constructor,
    .destructor_cb = nav_bar_lvgl_destructor,
    .name = "widget-nav-bar",
    .width_def = LV_PCT(100),
    .height_def = LV_SIZE_CONTENT,
    .instance_size = sizeof(NavBar),
};
