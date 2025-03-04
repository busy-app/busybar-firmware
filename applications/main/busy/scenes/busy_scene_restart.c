#include "busy_scene_restart.h"

typedef struct {
    lv_obj_t* main_label;
    lv_obj_t* button_list;
    lv_obj_t* start_button;
    lv_obj_t* finish_button;
} BusySceneRestart;

static void busy_button_event_callback(lv_event_t* event) {
    BusyApp* instance = lv_event_get_user_data(event);
    busy_send_custom_event(instance, (uint32_t)lv_event_get_target_obj(event));
}

static void busy_scene_restart_on_enter(void* context) {
    BusyApp* instance = context;
    BusySceneRestart* data = busy_get_current_scene_data(instance);

    if(busy_timer_is_running(instance)) {
        busy_timer_pause(instance);
    }

    gui_lock(instance->gui);

    lv_obj_t* active = gui_get_layer(instance->gui, GuiDisplayIdFront, GuiLayerIdActive);

    data->main_label = lv_label_create(active);
    lv_label_set_text(data->main_label, "Restart BUSY?");
    lv_obj_set_pos(data->main_label, 0, 1);
    lv_obj_set_size(data->main_label, 35, 14);
    lv_obj_set_style_text_font(data->main_label, &lv_font_tiny5_8, LV_PART_MAIN);
    lv_obj_set_style_text_color(data->main_label, lv_color_white(), LV_PART_MAIN);

    data->button_list = lv_list_create(active);
    lv_obj_set_pos(data->button_list, 37, 1);
    lv_obj_set_size(data->button_list, 35, lv_obj_get_height(active) - 2);
    lv_obj_set_style_text_font(data->button_list, &lv_font_tiny5_8, LV_PART_MAIN);
    lv_obj_set_style_pad_left(data->button_list, 6, LV_PART_MAIN);

    data->start_button = lv_list_add_button(data->button_list, NULL, "START");
    lv_obj_set_style_text_color(data->start_button, lv_color_hex(0x033013), LV_PART_MAIN);
    lv_obj_set_style_text_color(
        data->start_button, lv_color_hex(0x13F562), LV_PART_MAIN | LV_STATE_FOCUSED);
    lv_obj_add_event_cb(
        data->start_button, busy_button_event_callback, LV_EVENT_CLICKED, instance);

    data->finish_button = lv_list_add_button(data->button_list, NULL, "FINISH");
    lv_obj_add_event_cb(
        data->finish_button, busy_button_event_callback, LV_EVENT_CLICKED, instance);

    lv_label_set_text(instance->back_label, "Restart Menu");

    gui_unlock(instance->gui);
}

static void busy_scene_restart_on_exit(void* context) {
    BusyApp* instance = context;
    BusySceneRestart* data = busy_get_current_scene_data(instance);

    gui_lock(instance->gui);

    lv_obj_delete(data->main_label);
    lv_obj_delete(data->button_list);

    gui_unlock(instance->gui);
}

static void busy_scene_restart_on_event(const BusyEvent* event, void* context) {
    BusyApp* instance = context;

    if(event->type == BusyEventTypeCustom) {
        BusySceneRestart* data = busy_get_current_scene_data(instance);
        const lv_obj_t* button = (const lv_obj_t*)event->custom_value;

        if(button == data->start_button) {
            busy_timer_resume(instance);
            busy_switch_to_scene(instance, BusyAppSceneIdTimer);

        } else if(button == data->finish_button) {
            busy_timer_stop(instance);
            busy_switch_to_scene(instance, BusyAppSceneIdStart);
        }
    }
}

const BusyAppScene busy_scene_restart = {
    .on_enter = busy_scene_restart_on_enter,
    .on_exit = busy_scene_restart_on_exit,
    .on_event = busy_scene_restart_on_event,
    .data_size = sizeof(BusySceneRestart),
};
