#include "status_view.h"

#include <gui/widget_i.h>
#include <gui/modules/anim_player.h>

#define STATUS_VIEW_AUXILIARY_TEXT_COLOR_HEX 0x888888

struct StatusView {
    Widget base;
    Widget* internal_container;

    lv_obj_t* icon_static_box;
    lv_obj_t* icon_static;
    AnimPlayer* icon_animated;

    lv_obj_t* label_container;
    lv_obj_t* primary_label;
    lv_obj_t* auxiliary_label;

    FontRegistry* font_registry;
    const lv_font_t* primary_font;
    const lv_font_t* auxiliary_font;
};

const lv_obj_class_t status_view_lvgl_class;

/* callbacks */

static void status_view_style_front(Widget* widget) {
    StatusView* instance = (StatusView*)widget;

    instance->primary_font = font_registry_load_font(instance->font_registry, FONT_BUSY_REGULAR_5);

    lv_obj_set_flex_flow(TO_LV_OBJ(instance->internal_container), LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(
        TO_LV_OBJ(instance->internal_container),
        LV_FLEX_ALIGN_START,
        LV_FLEX_ALIGN_CENTER,
        LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(TO_LV_OBJ(instance->internal_container), 2, LV_PART_MAIN);

    lv_obj_set_style_pad_row(instance->label_container, -2, LV_PART_MAIN);

    lv_obj_set_style_text_font(instance->primary_label, instance->primary_font, LV_PART_MAIN);
    lv_obj_set_style_text_line_space(instance->primary_label, -2, LV_PART_MAIN);

    lv_obj_set_style_text_font(instance->auxiliary_label, instance->primary_font, LV_PART_MAIN);
    lv_obj_set_style_text_line_space(instance->auxiliary_label, -2, LV_PART_MAIN);
}

static void status_view_style_back(Widget* widget) {
    StatusView* instance = (StatusView*)widget;

    instance->primary_font = font_registry_load_font(instance->font_registry, FONT_BUSY_REGULAR_9);
    instance->auxiliary_font =
        font_registry_load_font(instance->font_registry, FONT_BUSY_REGULAR_7);

    lv_obj_set_flex_flow(TO_LV_OBJ(instance->internal_container), LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(
        TO_LV_OBJ(instance->internal_container),
        LV_FLEX_ALIGN_START,
        LV_FLEX_ALIGN_CENTER,
        LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_top(TO_LV_OBJ(instance->internal_container), 8, LV_PART_MAIN);
    lv_obj_set_style_pad_hor(TO_LV_OBJ(instance->internal_container), 2, LV_PART_MAIN);
    lv_obj_set_style_pad_row(TO_LV_OBJ(instance->internal_container), 6, LV_PART_MAIN);

    lv_obj_set_style_pad_left(instance->icon_static_box, 2, LV_PART_MAIN);
    lv_obj_set_style_pad_top(instance->icon_static_box, 2, LV_PART_MAIN);
    lv_obj_set_style_pad_right(instance->icon_static_box, 3, LV_PART_MAIN);
    lv_obj_set_style_pad_bottom(instance->icon_static_box, 3, LV_PART_MAIN);

    lv_obj_set_flex_align(
        instance->label_container,
        LV_FLEX_ALIGN_CENTER,
        LV_FLEX_ALIGN_CENTER,
        LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_row(instance->label_container, 2, LV_PART_MAIN);

    lv_obj_set_style_text_font(instance->primary_label, instance->primary_font, LV_PART_MAIN);
    lv_obj_set_style_text_align(instance->primary_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);

    lv_obj_set_style_text_font(instance->auxiliary_label, instance->auxiliary_font, LV_PART_MAIN);
    lv_obj_set_style_text_align(instance->auxiliary_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
}

/* LVGL-specific */

static void status_view_lvgl_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    LV_UNUSED(class_p);

    StatusView* instance = (StatusView*)obj;

    instance->font_registry = furi_record_open(RECORD_FONT_REGISTRY);
    instance->primary_font = NULL;
    instance->auxiliary_font = NULL;

    instance->internal_container = widget_alloc(&instance->base);

    instance->icon_static_box = lv_obj_create(TO_LV_OBJ(instance->internal_container));
    lv_obj_set_size(instance->icon_static_box, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_add_flag(instance->icon_static_box, LV_OBJ_FLAG_HIDDEN);

    instance->icon_static = lv_image_create(instance->icon_static_box);
    instance->icon_animated = anim_player_alloc(instance->internal_container);
    lv_obj_add_flag(TO_LV_OBJ(instance->icon_animated), LV_OBJ_FLAG_HIDDEN);

    instance->label_container = lv_obj_create(TO_LV_OBJ(instance->internal_container));
    lv_obj_set_size(instance->label_container, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(instance->label_container, LV_FLEX_FLOW_COLUMN);

    instance->primary_label = lv_label_create(instance->label_container);
    lv_obj_set_style_text_color(instance->primary_label, lv_color_white(), LV_PART_MAIN);
    lv_obj_add_flag(instance->primary_label, LV_OBJ_FLAG_HIDDEN);

    instance->auxiliary_label = lv_label_create(instance->label_container);
    lv_obj_set_style_text_color(
        instance->auxiliary_label,
        lv_color_hex(STATUS_VIEW_AUXILIARY_TEXT_COLOR_HEX),
        LV_PART_MAIN);
    lv_obj_add_flag(instance->auxiliary_label, LV_OBJ_FLAG_HIDDEN);
}

static void status_view_lvgl_destructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    UNUSED(class_p);

    StatusView* instance = (StatusView*)obj;

    if(instance->primary_font) {
        font_registry_unload_font(instance->font_registry, instance->primary_font);
    }

    if(instance->auxiliary_font) {
        font_registry_unload_font(instance->font_registry, instance->auxiliary_font);
    }

    furi_record_close(RECORD_FONT_REGISTRY);
}

/* Public API */

StatusView* status_view_alloc(Widget* parent) {
    furi_check(parent);

    lv_obj_t* obj = lv_obj_class_create_obj(&status_view_lvgl_class, TO_LV_OBJ(parent));
    lv_obj_class_init_obj(obj);

    StatusView* instance = (StatusView*)obj;

    return instance;
}

void status_view_free(StatusView* instance) {
    furi_check(instance);

    lv_obj_delete(TO_LV_OBJ(instance));
}

Widget* status_view_get_base(StatusView* instance) {
    furi_check(instance);

    return &instance->base;
}

void status_view_set_icon(StatusView* instance, const char* path) {
    furi_check(instance);
    furi_check(path);

    FuriString* path_string = furi_string_alloc_set(path);
    bool is_animated = furi_string_end_with(path_string, ".anim");
    furi_string_free(path_string);

    if(is_animated) {
        lv_obj_add_flag(instance->icon_static_box, LV_OBJ_FLAG_HIDDEN);
        lv_image_set_src(instance->icon_static, NULL);

        anim_player_set_source(instance->icon_animated, path);
        lv_obj_remove_flag(TO_LV_OBJ(instance->icon_animated), LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_add_flag(TO_LV_OBJ(instance->icon_animated), LV_OBJ_FLAG_HIDDEN);
        anim_player_set_source(instance->icon_animated, NULL);

        lv_image_set_src(instance->icon_static, path);
        lv_obj_remove_flag(instance->icon_static_box, LV_OBJ_FLAG_HIDDEN);
    }
}

void status_view_set_primary_text(StatusView* instance, const char* text) {
    furi_check(instance);

    if(text) {
        lv_label_set_text(instance->primary_label, text);
        lv_obj_remove_flag(instance->primary_label, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_label_set_text_static(instance->primary_label, "");
        lv_obj_add_flag(instance->primary_label, LV_OBJ_FLAG_HIDDEN);
    }
}

void status_view_set_auxiliary_text(StatusView* instance, const char* text) {
    furi_check(instance);

    if(text) {
        lv_label_set_text(instance->auxiliary_label, text);
        lv_obj_remove_flag(instance->auxiliary_label, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_label_set_text_static(instance->auxiliary_label, "");
        lv_obj_add_flag(instance->auxiliary_label, LV_OBJ_FLAG_HIDDEN);
    }
}

/* LVGL class descriptor */

const lv_obj_class_t status_view_lvgl_class = {
    .base_class = &widget_lvgl_class,

    .constructor_cb = status_view_lvgl_constructor,
    .destructor_cb = status_view_lvgl_destructor,

    .name = "widget-status-view",

    .width_def = LV_PCT(100),
    .height_def = LV_PCT(100),

    .instance_size = sizeof(StatusView),

    .user_data =
        (void*)&(const WidgetClassData){
            .input_callback = NULL,
            .style_callbacks =
                {
                    [GuiDisplayIdFront] = status_view_style_front,
                    [GuiDisplayIdBack] = status_view_style_back,
                },
        },
};
