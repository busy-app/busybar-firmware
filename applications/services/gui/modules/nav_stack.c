#include "nav_stack.h"

#include <m-array.h>

#include <gui/widget_i.h>

#define MY_CLASS         (&nav_stack_lvgl_class)
#define MY_BCRUMBS_CLASS (&nav_stack_bcrumbs_lvgl_class)

#define BCRUMBS_TEXT_COLOR lv_color_hex(0x777777)

ARRAY_DEF(LocationStack, const char*, M_CSTR_OPLIST);

struct NavStack {
    Widget base;
    lv_obj_t* header_layout;
    lv_obj_t* image;
    lv_obj_t* breadcrumbs;
    LocationStack_t locations;
};

const lv_obj_class_t nav_stack_lvgl_class;
const lv_obj_class_t nav_stack_bcrumbs_lvgl_class;

// LVGL-specific code

static void nav_stack_lvgl_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    UNUSED(class_p);

    lv_obj_set_flex_flow(obj, LV_FLEX_FLOW_COLUMN);

    NavStack* instance = (NavStack*)obj;

    instance->header_layout = lv_obj_create(obj);
    lv_obj_set_size(instance->header_layout, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(instance->header_layout, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(
        instance->header_layout, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_all(instance->header_layout, 2, LV_PART_MAIN);
    lv_obj_set_style_pad_column(instance->header_layout, 3, LV_PART_MAIN);

    instance->image = lv_image_create(instance->header_layout);
    instance->breadcrumbs = lv_label_create(instance->header_layout);
    lv_obj_set_style_text_font(instance->breadcrumbs, lv_theme_get_font_small(obj), LV_PART_MAIN);
    lv_obj_set_style_text_color(instance->breadcrumbs, BCRUMBS_TEXT_COLOR, LV_PART_MAIN);
    lv_label_set_recolor(instance->breadcrumbs, true);

    LocationStack_init(instance->locations);
}

static void nav_stack_lvgl_destructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    UNUSED(class_p);

    NavStack* instance = (NavStack*)obj;
    LocationStack_clear(instance->locations);
}

// Implementation

static void nav_stack_update(NavStack* instance) {
    if(LocationStack_size(instance->locations)) {
        FuriString* tmp = furi_string_alloc();

        LocationStack_it_ct it;
        for(LocationStack_it(it, instance->locations); !LocationStack_end_p(it);
            LocationStack_next(it)) {
            const char* location = *LocationStack_cref(it);
            if(LocationStack_last_p(it)) {
                furi_string_cat_printf(tmp, ">  #FFFFFF %s #", location);
            } else {
                furi_string_cat_printf(tmp, ">  %s  ", location);
            }
        }

        lv_label_set_text(instance->breadcrumbs, furi_string_get_cstr(tmp));

        furi_string_free(tmp);

        lv_obj_remove_flag(instance->breadcrumbs, LV_OBJ_FLAG_HIDDEN);

    } else {
        lv_obj_add_flag(instance->breadcrumbs, LV_OBJ_FLAG_HIDDEN);
    }
}

// Public API

NavStack* nav_stack_alloc(Widget* parent) {
    furi_check(parent);

    lv_obj_t* obj = lv_obj_class_create_obj(MY_CLASS, (lv_obj_t*)parent);
    lv_obj_class_init_obj(obj);

    NavStack* instance = (NavStack*)obj;
    nav_stack_update(instance);

    return instance;
}

void nav_stack_free(NavStack* instance) {
    furi_check(instance);
    lv_obj_delete((lv_obj_t*)instance);
}

Widget* nav_stack_get_base(NavStack* instance) {
    furi_check(instance);
    return (Widget*)instance;
}

bool nav_stack_set_image(NavStack* instance, const char* file_path) {
    furi_check(instance);
    furi_check(file_path);

    lv_image_set_src(instance->image, file_path);
    return true;
}

void nav_stack_push_location(NavStack* instance, const char* location_name) {
    furi_check(instance);
    furi_check(location_name);

    LocationStack_push_back(instance->locations, location_name);
    nav_stack_update(instance);
}

void nav_stack_pop_location(NavStack* instance) {
    furi_check(instance);
    furi_check(LocationStack_size(instance->locations));

    LocationStack_pop_back(NULL, instance->locations);
    nav_stack_update(instance);
}

// LVGL class descriptor

const lv_obj_class_t nav_stack_lvgl_class = {
    .base_class = &widget_lvgl_class,
    .constructor_cb = nav_stack_lvgl_constructor,
    .destructor_cb = nav_stack_lvgl_destructor,
    .name = "widget-nav-stack",
    .width_def = LV_PCT(100),
    .height_def = LV_PCT(100),
    .instance_size = sizeof(NavStack),
};
