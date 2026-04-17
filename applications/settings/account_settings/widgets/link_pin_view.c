#include "link_pin_view.h"
#include <settings_helpers/gui_params.h>
#include "../account_settings_i.h"
#include <storage/storage.h>
#include <gui/widget_i.h>
#include <gui/modules/anim_player.h>
#include <gui/modules/countdown.h>

#define LINK_PIN_BACK_CLASS  (&link_pin_view_back_lvgl_class)
#define LINK_PIN_FRONT_CLASS (&link_pin_view_front_lvgl_class)

#define COLOR_BOTTOM_TEXT lv_color_hex(0x888888)
#define COLOR_COUNTDOWN   (0xA0A0A0)

struct LinkPinView {
    Widget base;
    lv_obj_t* code_label;
    AnimPlayer* loading_spinner;
    Countdown* code_timer;

    FontRegistry* font_registry;
    const lv_font_t* font_busy_bold_10;
    const lv_font_t* font_busy_regular_7;
};

const lv_obj_class_t link_pin_view_back_lvgl_class;
const lv_obj_class_t link_pin_view_front_lvgl_class;

/* LVGL-specific code */

static void link_pin_view_front_lvgl_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    LV_UNUSED(class_p);
    LinkPinView* instance = (LinkPinView*)obj;

    instance->font_registry = furi_record_open(RECORD_FONT_REGISTRY);
    instance->font_busy_bold_10 =
        font_registry_load_font(instance->font_registry, FONT_BUSY_BOLD_10);

    lv_obj_set_flex_flow(obj, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(obj, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(obj, 2, LV_PART_MAIN);

    lv_obj_t* lock_image = lv_img_create(obj);
    lv_img_set_src(lock_image, IMG_PATH("lock_front_12x12.image"));

    instance->loading_spinner = anim_player_alloc((Widget*)instance);
    anim_player_set_source(instance->loading_spinner, SHARED_ANIM_PATH("spinner_front_8x8.anim"));

    instance->code_label = lv_label_create(obj);
    lv_obj_set_style_text_color(instance->code_label, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_text_font(instance->code_label, instance->font_busy_bold_10, LV_PART_MAIN);
    lv_obj_add_flag(instance->code_label, LV_OBJ_FLAG_HIDDEN);

    instance->code_timer = countdown_alloc((Widget*)instance);
    lv_obj_add_flag(TO_LV_OBJ(instance->code_timer), LV_OBJ_FLAG_IGNORE_LAYOUT);
    lv_obj_align(TO_LV_OBJ(instance->code_timer), LV_ALIGN_BOTTOM_RIGHT, 0, 1);
    countdown_set_text_color(instance->code_timer, color_hex_to_rgb(COLOR_COUNTDOWN));
    countdown_set_text_font(instance->code_timer, FONT_BUSY_SUPERSCRIPT_7);
}

static void link_pin_view_front_lvgl_destructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    LV_UNUSED(class_p);
    LinkPinView* instance = (LinkPinView*)obj;

    font_registry_unload_font(instance->font_registry, instance->font_busy_bold_10);

    furi_record_close(RECORD_FONT_REGISTRY);
}

static void link_pin_view_back_lvgl_destructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    LV_UNUSED(class_p);
    LinkPinView* instance = (LinkPinView*)obj;

    font_registry_unload_font(instance->font_registry, instance->font_busy_bold_10);
    font_registry_unload_font(instance->font_registry, instance->font_busy_regular_7);

    furi_record_close(RECORD_FONT_REGISTRY);
}

static void link_pin_view_back_lvgl_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    LV_UNUSED(class_p);
    LinkPinView* instance = (LinkPinView*)obj;

    instance->font_registry = furi_record_open(RECORD_FONT_REGISTRY);
    instance->font_busy_bold_10 =
        font_registry_load_font(instance->font_registry, FONT_BUSY_BOLD_10);
    instance->font_busy_regular_7 =
        font_registry_load_font(instance->font_registry, FONT_BUSY_REGULAR_7);

    lv_obj_set_flex_flow(obj, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(obj, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_row(obj, 5, LV_PART_MAIN);

    lv_obj_t* top_line = lv_obj_create(obj);
    lv_obj_set_flex_flow(top_line, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(top_line, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_END, LV_FLEX_ALIGN_END);
    lv_obj_set_style_pad_column(top_line, 4, LV_PART_MAIN);
    lv_obj_set_size(top_line, LV_SIZE_CONTENT, LV_SIZE_CONTENT);

    lv_obj_t* lock_image = lv_img_create(top_line);
    lv_img_set_src(lock_image, IMG_PATH("lock_back_11x11.image"));
    lv_obj_set_style_pad_bottom(lock_image, 4, LV_PART_MAIN);

    instance->loading_spinner = anim_player_alloc((Widget*)top_line);
    anim_player_set_source(instance->loading_spinner, SHARED_ANIM_PATH("spinner_front_8x8.anim"));
    lv_obj_set_style_pad_bottom(TO_LV_OBJ(instance->loading_spinner), 2, LV_PART_MAIN);

    instance->code_label = lv_label_create(top_line);
    lv_obj_set_style_text_color(instance->code_label, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_text_font(instance->code_label, instance->font_busy_bold_10, LV_PART_MAIN);
    lv_obj_add_flag(instance->code_label, LV_OBJ_FLAG_HIDDEN);

    instance->code_timer = countdown_alloc((Widget*)top_line);
    countdown_set_text_color(instance->code_timer, color_hex_to_rgb(COLOR_COUNTDOWN));
    countdown_set_text_font(instance->code_timer, FONT_BUSY_REGULAR_7);

    lv_obj_t* bottom_label = lv_label_create(obj);
    lv_label_set_text(bottom_label, "Enter this code on\nwww.cloud.busy.app");
    lv_obj_set_style_text_color(bottom_label, COLOR_BOTTOM_TEXT, LV_PART_MAIN);
    lv_obj_set_style_text_font(bottom_label, instance->font_busy_regular_7, LV_PART_MAIN);
}

/* Public API */

static LinkPinView* link_pin_view_alloc_common(Widget* parent, const lv_obj_class_t* class_p) {
    furi_check(parent);

    lv_obj_t* obj = lv_obj_class_create_obj(class_p, TO_LV_OBJ(parent));
    lv_obj_class_init_obj(obj);

    LinkPinView* instance = (LinkPinView*)obj;

    return instance;
}

LinkPinView* link_pin_view_front_alloc(Widget* parent) {
    return link_pin_view_alloc_common(parent, LINK_PIN_FRONT_CLASS);
}

LinkPinView* link_pin_view_back_alloc(Widget* parent) {
    return link_pin_view_alloc_common(parent, LINK_PIN_BACK_CLASS);
}

void link_pin_view_free(LinkPinView* instance) {
    furi_check(instance);
    lv_obj_delete(TO_LV_OBJ(instance));
}

void link_pin_view_set_state(LinkPinView* instance, const char* pin_code, time_t valid_untill) {
    furi_check(instance);

    if(pin_code) {
        lv_obj_add_flag((lv_obj_t*)instance->loading_spinner, LV_OBJ_FLAG_HIDDEN);

        lv_label_set_text(instance->code_label, pin_code);
        lv_obj_remove_flag(instance->code_label, LV_OBJ_FLAG_HIDDEN);

        countdown_begin(
            instance->code_timer,
            valid_untill,
            CountdownDirectionTimeLeft,
            CountdownShowHourWhenNonZero);
        lv_obj_remove_flag((lv_obj_t*)instance->code_timer, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_add_flag(instance->code_label, LV_OBJ_FLAG_HIDDEN);
        lv_label_set_text(instance->code_label, "");

        lv_obj_remove_flag((lv_obj_t*)instance->loading_spinner, LV_OBJ_FLAG_HIDDEN);

        lv_obj_add_flag((lv_obj_t*)instance->code_timer, LV_OBJ_FLAG_HIDDEN);
        countdown_stop(instance->code_timer);
    }
}

void link_pin_view_set_callback(LinkPinView* instance, LinkPinCallback callback, void* context) {
    furi_check(instance);
    countdown_set_callback(instance->code_timer, callback, context);
}

/* LVGL class descriptors */

const lv_obj_class_t link_pin_view_front_lvgl_class = {
    .base_class = &widget_lvgl_class,
    .constructor_cb = link_pin_view_front_lvgl_constructor,
    .destructor_cb = link_pin_view_front_lvgl_destructor,
    .name = "widget-link-pin-view-front",
    .width_def = LV_PCT(100),
    .height_def = LV_PCT(100),
    .instance_size = sizeof(LinkPinView),
};

const lv_obj_class_t link_pin_view_back_lvgl_class = {
    .base_class = &widget_lvgl_class,
    .constructor_cb = link_pin_view_back_lvgl_constructor,
    .destructor_cb = link_pin_view_back_lvgl_destructor,
    .name = "widget-link-pin-view-back",
    .width_def = LV_PCT(100),
    .height_def = LV_PCT(100),
    .instance_size = sizeof(LinkPinView),
};
