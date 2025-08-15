#include "battery_status_indicator.h"
#include "../storage_macros.h"

#include <gui/widget_i.h>

#define MY_CLASS (&battery_status_indicator_lvgl_class)

struct BatteryStatusIndicator {
    Widget base;
    lv_obj_t* charge_level_bar;
    lv_obj_t* charge_level_label;
    lv_obj_t* is_charging_state_image;
    lv_obj_t* is_error_state_image;
};

const lv_obj_class_t battery_status_indicator_lvgl_class;

/* LVGL-specific code */

static void
    battery_status_indicator_lvgl_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    UNUSED(class_p);

    lv_obj_set_flex_flow(obj, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(obj, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    BatteryStatusIndicator* instance = (BatteryStatusIndicator*)obj;

    lv_obj_t* background_image = lv_image_create(obj);
    lv_image_set_src(background_image, STATUS_BAR_IMG_PATH("battery_8x18.bin"));
    lv_obj_refr_size(background_image);

    instance->charge_level_bar = lv_bar_create(background_image);
    lv_obj_set_size(
        instance->charge_level_bar,
        lv_obj_get_width(background_image),
        lv_obj_get_height(background_image));
    lv_obj_set_style_bg_opa(instance->charge_level_bar, LV_OPA_COVER, LV_PART_INDICATOR);
    lv_obj_set_style_pad_top(instance->charge_level_bar, 3, LV_PART_MAIN);
    lv_obj_set_style_pad_bottom(instance->charge_level_bar, 2, LV_PART_MAIN);
    lv_obj_set_style_pad_right(instance->charge_level_bar, 2, LV_PART_MAIN);
    lv_obj_set_style_pad_left(instance->charge_level_bar, 2, LV_PART_MAIN);
    lv_bar_set_range(instance->charge_level_bar, 0, BATTERY_STATUS_INDICATOR_MAX_CHARGE_AMOUNT);

    instance->is_charging_state_image = lv_image_create(instance->charge_level_bar);
    lv_obj_center(instance->is_charging_state_image);
    lv_image_set_src(
        instance->is_charging_state_image, STATUS_BAR_IMG_PATH("battery_charging_4x7.bin"));

    instance->is_error_state_image = lv_image_create(background_image);
    lv_obj_center(instance->is_error_state_image);
    lv_image_set_src(instance->is_error_state_image, STATUS_BAR_IMG_PATH("battery_error_2x9.bin"));

    instance->charge_level_label = lv_label_create(obj);
    lv_obj_set_style_margin_top(instance->charge_level_label, 1, LV_PART_MAIN);
    lv_obj_set_style_pad_left(instance->charge_level_label, 1, LV_PART_MAIN);
    lv_obj_set_style_text_font(
        instance->charge_level_label, lv_theme_get_font_small(obj), LV_PART_MAIN);
    lv_obj_set_style_text_color(instance->charge_level_label, lv_color_white(), LV_PART_MAIN);
}

/* Public API */

BatteryStatusIndicator* battery_status_indicator_alloc(Widget* parent) {
    furi_assert(parent);

    lv_obj_t* obj = lv_obj_class_create_obj(MY_CLASS, (lv_obj_t*)parent);
    lv_obj_class_init_obj(obj);

    BatteryStatusIndicator* instance = (BatteryStatusIndicator*)obj;
    return instance;
}

void battery_status_indicator_free(BatteryStatusIndicator* instance) {
    furi_assert(instance);
    lv_obj_delete((lv_obj_t*)instance);
}

Widget* battery_status_indicator_get_base(BatteryStatusIndicator* instance) {
    furi_assert(instance);
    return (Widget*)instance;
}

void battery_status_indicator_set_charge_amount(BatteryStatusIndicator* instance, uint8_t charge) {
    furi_assert(instance);
    furi_assert(charge <= BATTERY_STATUS_INDICATOR_MAX_CHARGE_AMOUNT);

    lv_bar_set_value(instance->charge_level_bar, charge, LV_ANIM_OFF);

    char text[snprintf(NULL, 0, "%u", BATTERY_STATUS_INDICATOR_MAX_CHARGE_AMOUNT) + 1];
    snprintf(text, sizeof(text), "%u", charge);
    lv_label_set_text(instance->charge_level_label, text);
}

void battery_status_indicator_set_charging_state(
    BatteryStatusIndicator* instance,
    bool is_charging) {
    furi_assert(instance);

    lv_obj_update_flag(instance->is_charging_state_image, LV_OBJ_FLAG_HIDDEN, !is_charging);
}

void battery_status_indicator_set_error_state(BatteryStatusIndicator* instance, bool is_error) {
    furi_assert(instance);

    lv_obj_update_flag(instance->charge_level_bar, LV_OBJ_FLAG_HIDDEN, is_error);
    lv_obj_update_flag(instance->is_error_state_image, LV_OBJ_FLAG_HIDDEN, !is_error);
}

const lv_obj_class_t battery_status_indicator_lvgl_class = {
    .base_class = &widget_lvgl_class,
    .constructor_cb = battery_status_indicator_lvgl_constructor,
    .name = "widget-battery-state-indicator",
    .width_def = LV_PCT(100),
    .height_def = LV_SIZE_CONTENT,
    .instance_size = sizeof(BatteryStatusIndicator),
};
