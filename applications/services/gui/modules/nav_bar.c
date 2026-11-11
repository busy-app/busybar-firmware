#include "nav_bar.h"
#include "../widget_i.h"

#include <m-array.h>

#define MY_CLASS (&nav_bar_lvgl_class)

#define COLOR_INACTIVE 777777
#define COLOR_ACTIVE   FFFFFF

#define COLOR_INACTIVE_HEX CONCATENATE(0x, COLOR_INACTIVE)
#define COLOR_ACTIVE_HEX   CONCATENATE(0x, COLOR_ACTIVE)

#define NAV_BAR_FADER_WIDTH 10

#define NAV_BAR_BREADCRUMBS_SPACER   "\u200A"
#define NAV_BAR_BREADCRUMBS_ELLIPSIS "..."
#define NAV_BAR_BREADCRUMBS_OVERFLOW 5

#define NAV_BAR_M_CSTR_DUP_UPR_INIT_SET(a, b) ((a) = strupr(strdup(b)))
#define NAV_BAR_M_CSTR_DUP_UPR_CLEAR(a)       (free((void*)a))
#define NAV_BAR_M_CSTR_DUP_UPR_SET(a, b) \
    (NAV_BAR_M_CSTR_DUP_UPR_CLEAR(a), NAV_BAR_M_CSTR_DUP_UPR_INIT_SET(a, b))

#define NAV_BAR_M_CSTR_DUP_UPR_OPLIST           \
    (INIT_SET(NAV_BAR_M_CSTR_DUP_UPR_INIT_SET), \
     CLEAR(NAV_BAR_M_CSTR_DUP_UPR_CLEAR),       \
     SET(NAV_BAR_M_CSTR_DUP_UPR_SET),           \
     TYPE(const char*))

ARRAY_DEF(LocationStack, const char*, NAV_BAR_M_CSTR_DUP_UPR_OPLIST);

struct NavBar {
    Widget base;
    lv_obj_t* header_image;
    lv_obj_t* prefix_label;
    lv_obj_t* active_label;

    char* header_text;
    LocationStack_t locations;
};

const lv_obj_class_t nav_bar_lvgl_class;

/* implementation */

static void nav_bar_build_breadcrumbs_text(NavBar* instance, size_t start_idx, FuriString* text) {
    furi_string_set(text, ">");

    if(start_idx > 0) {
        furi_string_cat_printf(
            text,
            NAV_BAR_BREADCRUMBS_SPACER NAV_BAR_BREADCRUMBS_ELLIPSIS NAV_BAR_BREADCRUMBS_SPACER
            ">");
    }

    for(size_t i = start_idx; i < LocationStack_size(instance->locations) - 1; i++) {
        furi_string_cat_printf(
            text,
            NAV_BAR_BREADCRUMBS_SPACER "%s" NAV_BAR_BREADCRUMBS_SPACER ">",
            *LocationStack_get(instance->locations, i));
    }
}

static int32_t nav_bar_get_available_text_width(NavBar* instance) {
    int32_t padding = lv_obj_get_style_pad_column(TO_LV_OBJ(instance), LV_PART_MAIN);
    int32_t width = lv_obj_get_content_width(TO_LV_OBJ(instance)) - padding;

    if(!lv_obj_has_flag(instance->header_image, LV_OBJ_FLAG_HIDDEN)) {
        width -= lv_obj_get_self_width(instance->header_image) + padding;
    }

    return MAX(width, 0);
}

static int32_t nav_bar_measure_label_text_width(lv_obj_t* label, const char* text) {
    const lv_font_t* font = lv_obj_get_style_text_font(label, LV_PART_MAIN);
    int32_t letter_space = lv_obj_get_style_text_letter_space(label, LV_PART_MAIN);

    return lv_text_get_width(text, strlen(text), font, letter_space);
}

static void nav_bar_update_breadcrumbs(NavBar* instance) {
    if(LocationStack_size(instance->locations) > 0) {
        int32_t available_width = nav_bar_get_available_text_width(instance);

        size_t last_idx = LocationStack_size(instance->locations) - 1;
        const char* active_text = *LocationStack_get(instance->locations, last_idx);
        int32_t active_width =
            nav_bar_measure_label_text_width(instance->active_label, active_text);

        FuriString* text_builder = furi_string_alloc();
        for(size_t i = 0; i <= last_idx; i++) {
            nav_bar_build_breadcrumbs_text(instance, i, text_builder);

            int32_t prefix_width = nav_bar_measure_label_text_width(
                instance->prefix_label, furi_string_get_cstr(text_builder));

            if(prefix_width + active_width <= available_width + NAV_BAR_BREADCRUMBS_OVERFLOW) {
                break;
            }
        }

        lv_label_set_text(instance->prefix_label, furi_string_get_cstr(text_builder));
        lv_label_set_text(instance->active_label, active_text);

        lv_obj_remove_flag(instance->prefix_label, LV_OBJ_FLAG_HIDDEN);
        lv_obj_remove_flag(instance->active_label, LV_OBJ_FLAG_HIDDEN);

        furi_string_free(text_builder);
    } else {
        lv_label_set_text_static(
            instance->prefix_label, instance->header_text ? instance->header_text : "");

        lv_obj_add_flag(instance->active_label, LV_OBJ_FLAG_HIDDEN);
        lv_obj_remove_flag(instance->prefix_label, LV_OBJ_FLAG_HIDDEN);
    }
}

static void nav_bar_draw_fader(lv_obj_t* object, lv_layer_t* layer) {
    static const lv_draw_rect_dsc_t nav_bar_fader_descriptor = {
        .bg_opa = LV_OPA_COVER,
        .bg_grad.dir = LV_GRAD_DIR_HOR,
        .bg_grad.stops_count = 2,
        .bg_grad.stops =
            {
                {
                    .color = LV_COLOR_MAKE(0x00, 0x00, 0x00),
                    .opa = LV_OPA_TRANSP,
                    .frac = 0,
                },
                {
                    .color = LV_COLOR_MAKE(0x00, 0x00, 0x00),
                    .opa = LV_OPA_COVER,
                    .frac = 0xFF,
                },
            },
    };

    lv_area_t object_coords;
    lv_obj_get_coords(object, &object_coords);

    lv_area_t fader_area = {
        .x1 = object_coords.x2 - NAV_BAR_FADER_WIDTH + 1,
        .x2 = object_coords.x2,
        .y1 = object_coords.y1,
        .y2 = object_coords.y2,
    };

    lv_draw_rect(layer, &nav_bar_fader_descriptor, &fader_area);
}

/* LVGL-specific code */

static void nav_bar_lvgl_event(const lv_obj_class_t* class, lv_event_t* event) {
    lv_event_code_t event_code = lv_event_get_code(event);

    if(event_code == LV_EVENT_SIZE_CHANGED) {
        NavBar* instance = lv_event_get_current_target(event);
        nav_bar_update_breadcrumbs(instance);
    } else if(event_code == LV_EVENT_DRAW_POST) {
        nav_bar_draw_fader(lv_event_get_target_obj(event), lv_event_get_layer(event));
    }

    lv_obj_event_base(class, event);
}

static void nav_bar_lvgl_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    UNUSED(class_p);

    NavBar* instance = (NavBar*)obj;
    lv_obj_set_flex_flow(obj, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(obj, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(obj, 3, LV_PART_MAIN);
    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE);

    instance->header_image = lv_image_create(obj);
    lv_obj_add_flag(instance->header_image, LV_OBJ_FLAG_HIDDEN);

    instance->prefix_label = lv_label_create(obj);
    lv_obj_set_style_text_font(instance->prefix_label, lv_theme_get_font_small(obj), LV_PART_MAIN);
    lv_obj_set_style_pad_top(instance->prefix_label, 1, LV_PART_MAIN);
    lv_obj_set_style_text_letter_space(instance->prefix_label, 1, LV_PART_MAIN);
    lv_obj_set_style_text_color(
        instance->prefix_label, lv_color_hex(COLOR_INACTIVE_HEX), LV_PART_MAIN);
    lv_label_set_long_mode(instance->prefix_label, LV_LABEL_LONG_CLIP);
    lv_obj_add_flag(instance->prefix_label, LV_OBJ_FLAG_HIDDEN);

    instance->active_label = lv_label_create(obj);
    lv_obj_set_style_text_font(instance->active_label, lv_theme_get_font_small(obj), LV_PART_MAIN);
    lv_obj_set_style_pad_top(instance->active_label, 1, LV_PART_MAIN);
    lv_obj_set_style_text_letter_space(instance->active_label, 1, LV_PART_MAIN);
    lv_obj_set_style_text_color(
        instance->active_label, lv_color_hex(COLOR_ACTIVE_HEX), LV_PART_MAIN);
    lv_label_set_long_mode(instance->active_label, LV_LABEL_LONG_CLIP);
    lv_obj_add_flag(instance->active_label, LV_OBJ_FLAG_HIDDEN);

    instance->header_text = NULL;
    LocationStack_init(instance->locations);
}

static void nav_bar_lvgl_destructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    UNUSED(class_p);

    NavBar* instance = (NavBar*)obj;
    LocationStack_clear(instance->locations);
    free(instance->header_text);
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

    nav_bar_update_breadcrumbs(instance);
}

void nav_bar_set_header_text(NavBar* instance, const char* text) {
    furi_check(instance);

    free(instance->header_text);

    instance->header_text = text ? strupr(strdup(text)) : NULL;
    nav_bar_update_breadcrumbs(instance);
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

void nav_bar_pop_locations(NavBar* instance, size_t count) {
    furi_check(instance);
    furi_check(count <= LocationStack_size(instance->locations));

    for(size_t i = 0; i < count; i++) {
        LocationStack_pop_back(NULL, instance->locations);
    }

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
    .event_cb = nav_bar_lvgl_event,
    .name = "widget-nav-bar",
    .width_def = LV_PCT(100),
    .height_def = LV_SIZE_CONTENT,
    .instance_size = sizeof(NavBar),
};
