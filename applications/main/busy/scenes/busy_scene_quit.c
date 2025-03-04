#include "busy_scene_quit.h"

typedef struct {
    lv_obj_t* main_label;
    lv_obj_t* button_list;
    lv_obj_t* quit_button;
    lv_obj_t* cancel_button;
} BusySceneQuit;

static void busy_button_event_callback(lv_event_t* event) {
    BusyApp* instance = lv_event_get_user_data(event);
    busy_send_custom_event(instance, (uint32_t)lv_event_get_target_obj(event));
}

static void busy_scene_handle_cancel(BusyApp* instance) {
    if(instance->total_time_mn >= TOTAL_TIME_LOW_THR_MN) {
        busy_timer_resume(instance);
        busy_switch_to_scene(instance, BusyAppSceneIdTimer);
    } else {
        busy_switch_to_scene(instance, BusyAppSceneIdStatic);
    }
}

static void busy_scene_quit_on_enter(void* context) {
    BusyApp* instance = context;
    BusySceneQuit* data = busy_get_current_scene_data(instance);

    if(busy_timer_is_running(instance)) {
        busy_timer_pause(instance);
    }

    gui_lock(instance->gui);

    lv_obj_t* active = gui_get_layer(instance->gui, GuiDisplayIdFront, GuiLayerIdActive);

    data->main_label = lv_label_create(active);
    lv_label_set_text(data->main_label, "Quit this BUSY?");
    lv_obj_set_pos(data->main_label, 0, 1);
    lv_obj_set_size(data->main_label, 35, 14);
    lv_obj_set_style_text_font(data->main_label, &lv_font_tiny5_8, LV_PART_MAIN);
    lv_obj_set_style_text_color(data->main_label, lv_color_white(), LV_PART_MAIN);

    data->button_list = lv_list_create(active);
    lv_obj_set_pos(data->button_list, 37, 1);
    lv_obj_set_size(data->button_list, 35, lv_obj_get_height(active) - 2);
    lv_obj_set_style_text_font(data->button_list, &lv_font_tiny5_8, LV_PART_MAIN);
    lv_obj_set_style_pad_left(data->button_list, 6, LV_PART_MAIN);

    data->quit_button = lv_list_add_button(data->button_list, NULL, "QUIT");
    lv_obj_set_style_text_color(data->quit_button, lv_color_hex(0x4A0000), LV_PART_MAIN);
    lv_obj_set_style_text_color(
        data->quit_button, lv_color_hex(0xFF0000), LV_PART_MAIN | LV_STATE_FOCUSED);
    lv_obj_add_event_cb(data->quit_button, busy_button_event_callback, LV_EVENT_CLICKED, instance);

    data->cancel_button = lv_list_add_button(data->button_list, NULL, "CANCEL");
    lv_obj_add_event_cb(
        data->cancel_button, busy_button_event_callback, LV_EVENT_CLICKED, instance);

    lv_label_set_text(instance->back_label, "Quit Menu");

    gui_unlock(instance->gui);
}

static void busy_scene_quit_on_exit(void* context) {
    BusyApp* instance = context;
    BusySceneQuit* data = busy_get_current_scene_data(instance);

    gui_lock(instance->gui);

    lv_obj_delete(data->main_label);
    lv_obj_delete(data->button_list);

    gui_unlock(instance->gui);
}

static void busy_scene_quit_on_event(const BusyEvent* event, void* context) {
    BusyApp* instance = context;

    if(event->type == BusyEventTypeBack) {
        busy_scene_handle_cancel(instance);

    } else if(event->type == BusyEventTypeCustom) {
        BusySceneQuit* data = busy_get_current_scene_data(instance);
        const uint32_t button_id = event->custom_value;

        if(button_id == (uint32_t)data->quit_button) {
            busy_timer_stop(instance);
            busy_switch_to_scene(instance, BusyAppSceneIdStart);

        } else if(button_id == (uint32_t)data->cancel_button) {
            busy_scene_handle_cancel(instance);
        }
    }
}

const BusyAppScene busy_scene_quit = {
    .on_enter = busy_scene_quit_on_enter,
    .on_exit = busy_scene_quit_on_exit,
    .on_event = busy_scene_quit_on_event,
    .data_size = sizeof(BusySceneQuit),
};
