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
    BusyTimerState timer_state;
} BusySceneTimer;

static void busy_scene_timer_state_update(BusySceneTimer* data) {
    const lv_image_dsc_t* main_image_dsc;
    uint32_t bar_color_main;
    uint32_t bar_color_indicator;

    // TODO: Use colours from theme
    if(data->timer_state == BusyTimerStateWork) {
        main_image_dsc = &I_busy_39x14;
        bar_color_main = 0x4A0000;
        bar_color_indicator = 0xFF0000;

    } else if(data->timer_state == BusyTimerStateRest) {
        main_image_dsc = &I_rest_39x14;
        bar_color_main = 0x033013;
        bar_color_indicator = 0x13F562;

    } else if(data->timer_state == BusyTimerStateLongRest) {
        main_image_dsc = &I_long_rest_39x14;
        bar_color_main = 0x081631;
        bar_color_indicator = 0x2C72FA;

    } else {
        furi_crash("Invalid state");
    }

    lv_image_set_src(data->main_image, main_image_dsc);
    lv_obj_set_style_bg_color(data->time_bar, lv_color_hex(bar_color_main), LV_PART_MAIN);
    lv_obj_set_style_bg_color(
        data->time_bar, lv_color_hex(bar_color_indicator), LV_PART_INDICATOR);
}

static void busy_scene_timer_update(BusyApp* instance) {
    BusySceneTimer* data = busy_get_current_scene_data(instance);

    gui_lvgl_acquire(instance->gui);

    if(data->timer_state != instance->state) {
        data->timer_state = instance->state;
        busy_scene_timer_state_update(data);
    }

    const uint32_t minutes = S_TO_M(instance->interval_time_left_s);
    const uint32_t seconds = S_TO_R(instance->interval_time_left_s);
    const uint32_t percent = (instance->interval_time_left_s * 100) / instance->interval_time_s;

    lv_label_set_text_fmt(data->time_label, "%02lu:%02lu", minutes, seconds);
    lv_bar_set_value(data->time_bar, percent, LV_ANIM_OFF);

    if(data->timer_state == BusyTimerStateWork) {
        lv_label_set_text_fmt(instance->back_label, "BUSY: %02lu:%02lu", minutes, seconds);
    } else if(data->timer_state == BusyTimerStateRest) {
        lv_label_set_text_fmt(instance->back_label, "REST: %02lu:%02lu", minutes, seconds);
    } else if(data->timer_state == BusyTimerStateLongRest) {
        lv_label_set_text_fmt(instance->back_label, "LONG REST: %02lu:%02lu", minutes, seconds);
    }

    gui_lvgl_release(instance->gui);
}

static void busy_scene_timer_show_pause_overlay(BusyApp* instance, bool show) {
    BusySceneTimer* data = busy_get_current_scene_data(instance);

    if(show == !!data->overlay) {
        return;
    }

    gui_lvgl_acquire(instance->gui);

    if(show) {
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

static void busy_scene_timer_toggle_pause_overlay(BusyApp* instance) {
    BusySceneTimer* data = busy_get_current_scene_data(instance);
    busy_scene_timer_show_pause_overlay(instance, data->overlay == NULL);
}

static void busy_scene_timer_start_pressed_callback(lv_event_t* event) {
    BusyApp* instance = lv_event_get_user_data(event);

    const lv_event_code_t code = lv_event_get_code(event);
    const lv_indev_t* indev = lv_event_get_param(event);

    if(lv_indev_get_type(indev) == LV_INDEV_TYPE_KEYPAD) {
        if(code == LV_EVENT_SINGLE_CLICKED) {
            busy_send_custom_event(instance, BusyCustomEventStartSingle);
        } else if(code == LV_EVENT_DOUBLE_CLICKED) {
            busy_send_custom_event(instance, BusyCustomEventStartDouble);
        }
    }
}

static void busy_scene_timer_on_enter(void* context) {
    BusyApp* instance = context;
    BusySceneTimer* data = busy_get_current_scene_data(instance);

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

    lv_obj_add_event_cb(
        data->info_label,
        busy_scene_timer_start_pressed_callback,
        LV_EVENT_SINGLE_CLICKED,
        instance);
    lv_obj_add_event_cb(
        data->info_label,
        busy_scene_timer_start_pressed_callback,
        LV_EVENT_DOUBLE_CLICKED,
        instance);
    // Send input events to the label
    lv_group_add_obj(lv_group_get_default(), data->info_label);

    data->time_bar = lv_bar_create(active);
    lv_obj_set_pos(data->time_bar, 1, 15);
    lv_obj_set_size(data->time_bar, lv_obj_get_width(active) - 2, 1);

    gui_lvgl_release(instance->gui);

    busy_scene_timer_update(instance);
}

static void busy_scene_timer_on_exit(void* context) {
    BusyApp* instance = context;
    BusySceneTimer* data = busy_get_current_scene_data(instance);

    gui_lvgl_acquire(instance->gui);

    // Stop sending input events to the label
    // TODO: Why isn't it removed automatically?
    lv_group_remove_obj(data->info_label);

    lv_obj_delete(data->main_image);
    lv_obj_delete(data->time_label);
    lv_obj_delete(data->info_label);
    lv_obj_delete(data->time_bar);

    if(data->overlay) {
        lv_obj_delete(data->overlay);
        lv_obj_delete(data->overlay_image);
    }

    gui_lvgl_release(instance->gui);
}

static void busy_scene_timer_on_event(const BusyEvent* event, void* context) {
    BusyApp* instance = context;

    if(event->type == BusyEventTypeBack) {
        busy_switch_to_scene(instance, BusyAppSceneIdQuit);

    } else if(event->type == BusyEventTypeOk) {
        busy_timer_next_state(instance, true);
        busy_scene_timer_show_pause_overlay(instance, false);

    } else if(event->type == BusyEventTypeCustom) {
        if(event->custom_value == BusyCustomEventUpdate) {
            busy_scene_timer_update(instance);
        } else if(event->custom_value == BusyCustomEventIntervalEnd) {
            busy_switch_to_scene(instance, BusyAppSceneIdNext);
        } else if(event->custom_value == BusyCustomEventSessionEnd) {
            busy_switch_to_scene(instance, BusyAppSceneIdRestart);
        } else if(event->custom_value == BusyCustomEventStartSingle) {
            busy_timer_toggle(instance);
            busy_scene_timer_toggle_pause_overlay(instance);
        } else if(event->custom_value == BusyCustomEventStartDouble) {
            busy_timer_next_state(instance, true);
            busy_scene_timer_show_pause_overlay(instance, false);
        }
    }
}

const BusyAppScene busy_scene_timer = {
    .on_enter = busy_scene_timer_on_enter,
    .on_exit = busy_scene_timer_on_exit,
    .on_event = busy_scene_timer_on_event,
    .data_size = sizeof(BusySceneTimer),
};
