#include "nav_header.h"

#include <m-array.h>

#include <gui/widget_i.h>

#define MY_CLASS         (&nav_header_lvgl_class)
#define MY_BCRUMBS_CLASS (&nav_header_bcrumbs_lvgl_class)

ARRAY_DEF(LocationStack, const char*, M_CSTR_OPLIST);

struct NavHeader {
    Widget base;
    lv_obj_t* image;
    lv_obj_t* breadcrumbs;
    LocationStack_t locations;
};

const lv_obj_class_t nav_header_lvgl_class;
const lv_obj_class_t nav_header_bcrumbs_lvgl_class;

// LVGL-specific code

static void nav_header_lvgl_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    UNUSED(class_p);

    lv_obj_set_flex_flow(obj, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(obj, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START);

    NavHeader* instance = (NavHeader*)obj;
    instance->image = lv_image_create(obj);
    instance->breadcrumbs = lv_label_create(obj);

    LocationStack_init(instance->locations);
}

static void nav_header_lvgl_destructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    UNUSED(class_p);

    NavHeader* instance = (NavHeader*)obj;
    LocationStack_clear(instance->locations);
}

// Implementation

static void nav_header_update(NavHeader* instance) {
    if(LocationStack_size(instance->locations)) {
        FuriString* tmp = furi_string_alloc();

        LocationStack_it_ct it;
        for(LocationStack_it(it, instance->locations); !LocationStack_end_p(it);
            LocationStack_next(it)) {
            furi_string_cat_printf(tmp, "> %s ", *LocationStack_cref(it));
        }

        lv_label_set_text(instance->breadcrumbs, furi_string_get_cstr(tmp));

        furi_string_free(tmp);

        lv_obj_remove_flag(instance->breadcrumbs, LV_OBJ_FLAG_HIDDEN);

    } else {
        lv_obj_add_flag(instance->breadcrumbs, LV_OBJ_FLAG_HIDDEN);
    }
}

// Public API

NavHeader* nav_header_alloc(Widget* parent) {
    furi_check(parent);

    lv_obj_t* obj = lv_obj_class_create_obj(MY_CLASS, (lv_obj_t*)parent);
    lv_obj_class_init_obj(obj);

    NavHeader* instance = (NavHeader*)obj;
    nav_header_update(instance);

    return instance;
}

void nav_header_free(NavHeader* instance) {
    furi_check(instance);
    lv_obj_delete((lv_obj_t*)instance);
}

Widget* nav_header_get_base(NavHeader* instance) {
    furi_check(instance);
    return (Widget*)instance;
}

bool nav_header_set_image(NavHeader* instance, const char* file_path) {
    furi_check(instance);
    furi_check(file_path);

    lv_image_set_src(instance->image, file_path);
    return true;
}

void nav_header_push_location(NavHeader* instance, const char* location_name) {
    furi_check(instance);
    furi_check(location_name);

    LocationStack_push_back(instance->locations, location_name);
    nav_header_update(instance);
}

void nav_header_pop_location(NavHeader* instance) {
    furi_check(instance);

    LocationStack_pop_back(NULL, instance->locations);
    nav_header_update(instance);
}

// LVGL class descriptor

const lv_obj_class_t nav_header_lvgl_class = {
    .base_class = &widget_lvgl_class,
    .constructor_cb = nav_header_lvgl_constructor,
    .destructor_cb = nav_header_lvgl_destructor,
    .name = "widget-nav-header",
    .width_def = LV_SIZE_CONTENT,
    .height_def = LV_SIZE_CONTENT,
    .instance_size = sizeof(NavHeader),
};
