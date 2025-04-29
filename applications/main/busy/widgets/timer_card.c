#include "timer_card.h"

#include <gui/gui_i.h>

#include "../time_macros.h"
#include "../compiled_assets/compiled_assets.h"

#define MY_CLASS (&timer_card_lvgl_class)

struct TimerCard {
    Widget base;
    lv_display_t* display;
    lv_obj_t* left_image;
    lv_obj_t* right_image;
    lv_obj_t* top_static_text;
    lv_obj_t* mirror_image;
    lv_obj_t* bottom_timer_text;
    lv_obj_t* bottom_static_text;
    lv_image_dsc_t mirror_image_dsc;
};

const lv_obj_class_t timer_card_lvgl_class;

static void timer_card_refresh_callback(lv_event_t* event) {
    lv_obj_t* image = lv_event_get_user_data(event);
    lv_obj_invalidate(image);
}

// LVGL-specific code

static void timer_card_lvgl_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    UNUSED(class_p);

    lv_obj_set_flex_flow(obj, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(obj, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t* top_layout = lv_obj_create(obj);
    lv_obj_set_size(top_layout, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(top_layout, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(
        top_layout, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_ver(top_layout, 5, LV_PART_MAIN);
    lv_obj_set_style_pad_column(top_layout, 4, LV_PART_MAIN);

    TimerCard* instance = (TimerCard*)obj;
    instance->left_image = lv_image_create(top_layout);
    lv_image_set_src(instance->left_image, &I_active_indicator_left_28x7);

    instance->top_static_text = lv_label_create(top_layout);
    lv_obj_set_style_text_font(
        instance->top_static_text, lv_theme_get_font_small(obj), LV_PART_MAIN);
    lv_obj_set_style_text_color(instance->top_static_text, lv_color_black(), LV_PART_MAIN);
    lv_label_set_text(instance->top_static_text, "ACTIVE");

    instance->right_image = lv_image_create(top_layout);
    lv_image_set_src(instance->right_image, &I_active_indicator_right_28x7);

    // Mask object for image rounded corners
    lv_obj_t* mask = lv_obj_create(obj);
    lv_obj_set_size(mask, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_style_radius(mask, 4, LV_PART_MAIN);
    lv_obj_set_style_clip_corner(mask, true, LV_PART_MAIN);

    instance->mirror_image = lv_image_create(mask);

    Gui* gui = furi_record_open(RECORD_GUI);
    GuiDisplay* front = &gui->displays[GuiDisplayIdFront];

    lv_image_dsc_t* image_dsc = &instance->mirror_image_dsc;
    image_dsc->header.magic = LV_IMAGE_HEADER_MAGIC;
    image_dsc->header.cf = FRONT_COLOR_FORMAT;
    image_dsc->header.w = FRONT_W;
    image_dsc->header.h = FRONT_H;
    image_dsc->header.stride = LV_DRAW_BUF_STRIDE(FRONT_W, FRONT_COLOR_FORMAT);
    image_dsc->data_size = FRONT_DRAW_BUFFER_SIZE;
    image_dsc->data = front->draw_buffer;

    lv_obj_set_size(instance->mirror_image, FRONT_W * 2, FRONT_H * 2);
    lv_image_set_pivot(instance->mirror_image, 0, 0);
    lv_image_set_inner_align(instance->mirror_image, LV_IMAGE_ALIGN_TOP_LEFT);
    lv_image_set_antialias(instance->mirror_image, false);
    // TODO: Figure out why different x and y scale works best
    lv_image_set_scale_x(instance->mirror_image, LV_SCALE_NONE * 2);
    lv_image_set_scale_y(instance->mirror_image, LV_SCALE_NONE * 2 + 1);
    lv_image_set_src(instance->mirror_image, image_dsc);

    lv_obj_t* bottom_layout = lv_obj_create(obj);
    lv_obj_set_size(bottom_layout, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(bottom_layout, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(
        bottom_layout, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START);
    lv_obj_set_style_bg_opa(bottom_layout, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_pad_ver(bottom_layout, 3, LV_PART_MAIN);
    lv_obj_set_style_pad_column(bottom_layout, 4, LV_PART_MAIN);

    instance->bottom_timer_text = lv_label_create(bottom_layout);
    lv_obj_set_style_text_color(instance->bottom_timer_text, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_text_font(
        instance->bottom_timer_text, &lv_font_ark_numerals_regular_10, LV_PART_MAIN);

    instance->bottom_static_text = lv_label_create(bottom_layout);
    lv_label_set_text(instance->bottom_static_text, "LEFT");
    lv_obj_set_style_text_color(instance->bottom_static_text, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_text_font(
        instance->bottom_static_text, lv_theme_get_font_small(obj), LV_PART_MAIN);

    instance->display = front->lv_display;
    lv_display_add_event_cb(
        instance->display,
        timer_card_refresh_callback,
        LV_EVENT_REFR_READY,
        instance->mirror_image);
}

static void timer_card_lvgl_destructor(const lv_obj_class_t* class_p, lv_obj_t* obj) {
    UNUSED(class_p);

    TimerCard* instance = (TimerCard*)obj;
    lv_display_remove_event_cb_with_user_data(
        instance->display, timer_card_refresh_callback, instance->mirror_image);
    furi_record_close(RECORD_GUI);
}

// Public API

TimerCard* timer_card_alloc(Widget* parent) {
    furi_check(parent);

    lv_obj_t* obj = lv_obj_class_create_obj(MY_CLASS, (lv_obj_t*)parent);
    lv_obj_class_init_obj(obj);

    TimerCard* instance = (TimerCard*)obj;
    return instance;
}

void timer_card_free(TimerCard* instance) {
    furi_check(instance);
    lv_obj_delete((lv_obj_t*)instance);
}

Widget* timer_card_get_base(TimerCard* instance) {
    furi_check(instance);
    return (Widget*)instance;
}

void timer_card_show_header(TimerCard* instance, bool show) {
    furi_check(instance);

    lv_opa_t opacity = show ? LV_OPA_COVER : LV_OPA_TRANSP;
    lv_obj_set_style_text_opa(instance->top_static_text, opacity, LV_PART_MAIN);
    lv_obj_set_style_image_opa(instance->left_image, opacity, LV_PART_MAIN);
    lv_obj_set_style_image_opa(instance->right_image, opacity, LV_PART_MAIN);
}

void timer_card_show_time(TimerCard* instance, bool show) {
    furi_check(instance);

    lv_opa_t opacity = show ? LV_OPA_COVER : LV_OPA_TRANSP;
    lv_obj_set_style_text_opa(instance->bottom_timer_text, opacity, LV_PART_MAIN);
    lv_obj_set_style_text_opa(instance->bottom_static_text, opacity, LV_PART_MAIN);
}

void timer_card_set_time(TimerCard* instance, uint32_t time_s) {
    furi_check(instance);

    const uint32_t h = S_TO_H(time_s);
    const uint32_t m = S_TO_M(time_s - H_TO_S(h));
    const uint32_t s = time_s - H_TO_S(h) - M_TO_S(m);

    if(h) {
        lv_label_set_text_fmt(instance->bottom_timer_text, "%lu:%02lu:%02lu", h, m, s);
    } else {
        lv_label_set_text_fmt(instance->bottom_timer_text, "%02lu:%02lu", m, s);
    }
}

// LVGL class descriptor

const lv_obj_class_t timer_card_lvgl_class = {
    .base_class = &widget_lvgl_class,
    .constructor_cb = timer_card_lvgl_constructor,
    .destructor_cb = timer_card_lvgl_destructor,
    .name = "widget-timer-card",
    .width_def = LV_SIZE_CONTENT,
    .height_def = LV_SIZE_CONTENT,
    .instance_size = sizeof(TimerCard),
};
