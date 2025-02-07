#include "busy_scene_timer.h"

extern const lv_image_dsc_t I_busy_39x14;
extern const lv_image_dsc_t I_rest_39x14;
extern const lv_image_dsc_t I_long_rest_39x14;
extern const lv_image_dsc_t I_pause_5x5;

typedef struct {
    lv_obj_t* main_image;
    lv_obj_t* time_label;
    lv_obj_t* info_label;
    lv_obj_t* time_bar;
    lv_obj_t* overlay;
    lv_obj_t* overlay_image;
} BusySceneTimer;

static void busy_scene_timer_update(BusyApp* instance) {
    BusySceneTimer* data = instance->scene_data[BusyAppSceneIdTimer];

    gui_lvgl_acquire(instance->gui);

    const uint32_t minutes = instance->time_left / 60;
    const uint32_t seconds = instance->time_left % 60;
    const uint32_t percent = (instance->time_left * 100) / instance->time_total;

    lv_label_set_text_fmt(data->time_label, "%02lu:%02lu", minutes, seconds);
    lv_bar_set_value(data->time_bar, percent, LV_ANIM_OFF);

    gui_lvgl_release(instance->gui);
}

static void busy_scene_timer_set_state(BusyApp* instance, BusyTimerState new_state) {
    BusySceneTimer* data = instance->scene_data[BusyAppSceneIdTimer];

    const lv_image_dsc_t* main_image_dsc;
    uint32_t bar_color_main;
    uint32_t bar_color_indicator;

    if(new_state == BusyTimerStateBusy) {
        main_image_dsc = &I_busy_39x14;
        bar_color_main = 0x4A0000;
        bar_color_indicator = 0xFF0000;

    } else if(new_state == BusyTimerStateRest) {
        main_image_dsc = &I_rest_39x14;
        bar_color_main = 0x033013;
        bar_color_indicator = 0x13F562;

    } else if(new_state == BusyTimerStateLongRest) {
        main_image_dsc = &I_long_rest_39x14;
        bar_color_main = 0x081631;
        bar_color_indicator = 0x2C72FA;

    } else {
        furi_crash("Invalid state");
    }

    gui_lvgl_acquire(instance->gui);

    lv_image_set_src(data->main_image, main_image_dsc);
    lv_obj_set_style_bg_color(data->time_bar, lv_color_hex(bar_color_main), LV_PART_MAIN);
    lv_obj_set_style_bg_color(
        data->time_bar, lv_color_hex(bar_color_indicator), LV_PART_INDICATOR);

    gui_lvgl_release(instance->gui);

    busy_scene_timer_update(instance);
}

static void busy_scene_timer_toggle_pause_overlay(BusyApp* instance) {
    BusySceneTimer* data = instance->scene_data[BusyAppSceneIdTimer];

    gui_lvgl_acquire(instance->gui);

    if(data->overlay == NULL) {
        lv_obj_t* top = gui_lvgl_get_layer(instance->gui, GuiDisplayIdFront, GuiLayerIdTop);

        data->overlay = lv_obj_create(top);
        lv_obj_set_size(data->overlay, lv_obj_get_width(top), lv_obj_get_height(top));
        lv_obj_set_style_opa(data->overlay, LV_OPA_70, LV_PART_MAIN);

        data->overlay_image = lv_image_create(top);
        lv_image_set_src(data->overlay_image, &I_pause_5x5);
        lv_obj_set_pos(data->overlay_image, 33, 5);

    } else {
        lv_obj_delete(data->overlay);
        lv_obj_delete(data->overlay_image);

        data->overlay = NULL;
        data->overlay_image = NULL;
    }

    gui_lvgl_release(instance->gui);
}

static void busy_scene_timer_on_enter(void* context) {
    BusyApp* instance = context;

    void** data_slot = &instance->scene_data[BusyAppSceneIdTimer];
    furi_check(*data_slot == NULL);

    BusySceneTimer* data = malloc(sizeof(BusySceneTimer));

    gui_lvgl_acquire(instance->gui);

    lv_obj_t* active = gui_lvgl_get_layer(instance->gui, GuiDisplayIdFront, GuiLayerIdActive);

    data->main_image = lv_image_create(active);

    data->time_label = lv_label_create(active);
    lv_obj_set_style_text_font(data->time_label, &lv_font_pixel_operator_8, LV_PART_MAIN);
    // TODO: Implement in theme
    lv_obj_set_style_text_color(data->time_label, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_pos(data->time_label, 41, 1);

    data->info_label = lv_label_create(active);
    lv_label_set_text(data->info_label, "LEFT");
    lv_obj_set_style_text_font(data->info_label, &lv_font_tiny_6, LV_PART_MAIN);
    // TODO: Implement in theme
    lv_obj_set_style_text_color(data->info_label, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_pos(data->info_label, 41, 10);

    data->time_bar = lv_bar_create(active);
    lv_obj_set_pos(data->time_bar, 1, 15);
    lv_obj_set_size(data->time_bar, lv_obj_get_width(active) - 2, 1);

    gui_lvgl_release(instance->gui);

    *data_slot = data;
}

static void busy_scene_timer_on_exit(void* context) {
    BusyApp* instance = context;

    void** data_slot = &instance->scene_data[BusyAppSceneIdTimer];
    furi_check(*data_slot != NULL);

    BusySceneTimer* data = *data_slot;

    gui_lvgl_acquire(instance->gui);

    lv_obj_delete(data->main_image);
    lv_obj_delete(data->time_label);
    lv_obj_delete(data->info_label);
    lv_obj_delete(data->time_bar);

    if(data->overlay) {
        lv_obj_delete(data->overlay);
        lv_obj_delete(data->overlay_image);
    }

    gui_lvgl_release(instance->gui);

    free(data);
    *data_slot = NULL;
}

static void busy_scene_timer_on_event(const BusyEvent* event, void* context) {
    BusyApp* instance = context;

    if(event->type == BusyEventTypeStart) {
        busy_scene_timer_toggle_pause_overlay(instance);
        busy_timer_toggle(instance);

    } else if(event->type == BusyEventTypeBack) {
        if(busy_timer_is_running(instance)) {
            busy_switch_to_scene(instance, BusyAppSceneIdQuit);
        }

    } else if(event->type == BusyEventTypeOk) {
        if(busy_timer_is_running(instance)) {
            busy_timer_next_state(instance);
        }

    } else if(event->type == BusyEventTypeCustom) {
        if(event->custom_value > BusyTimerStateIdle && event->custom_value < BusyTimerStateMax) {
            busy_scene_timer_set_state(instance, event->custom_value);
        } else if(event->custom_value == BusyCustomEventUpdate) {
            busy_scene_timer_update(instance);
        }
    }
}

const BusyAppScene busy_scene_timer = {
    .on_enter = busy_scene_timer_on_enter,
    .on_exit = busy_scene_timer_on_exit,
    .on_event = busy_scene_timer_on_event,
};
