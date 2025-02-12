#include "busy_scene_start.h"

extern const lv_image_dsc_t I_pending_39x16;

typedef struct {
    lv_obj_t* main_image;
    lv_obj_t* button_list;
    lv_obj_t* start_button;
    lv_obj_t* setup_button;
} BusySceneStart;

static void busy_button_event_callback(lv_event_t* event) {
    BusyApp* instance = lv_event_get_user_data(event);
    busy_send_custom_event(instance, (uint32_t)lv_event_get_target_obj(event));
}

static void busy_scene_start_on_enter(void* context) {
    BusyApp* instance = context;
    BusySceneStart* data = busy_get_current_scene_data(instance);

    gui_lvgl_acquire(instance->gui);

    lv_obj_t* active = gui_lvgl_get_layer(instance->gui, GuiDisplayIdFront, GuiLayerIdActive);

    data->main_image = lv_image_create(active);
    lv_image_set_src(data->main_image, &I_pending_39x16);

    data->button_list = lv_list_create(active);
    lv_obj_set_pos(data->button_list, 39, 1);
    lv_obj_set_size(data->button_list, 33, lv_obj_get_height(active) - 2);
    lv_obj_set_style_text_font(data->button_list, &lv_font_tiny5_8, LV_PART_MAIN);
    lv_obj_set_style_pad_left(data->button_list, 8, LV_PART_MAIN);

    data->start_button = lv_list_add_button(data->button_list, NULL, "START");
    lv_obj_set_style_text_color(data->start_button, lv_color_hex(0x033013), LV_PART_MAIN);
    lv_obj_set_style_text_color(
        data->start_button, lv_color_hex(0x13F562), LV_PART_MAIN | LV_STATE_FOCUSED);
    lv_obj_add_event_cb(
        data->start_button, busy_button_event_callback, LV_EVENT_CLICKED, instance);

    data->setup_button = lv_list_add_button(data->button_list, NULL, "SETUP");
    lv_obj_add_event_cb(
        data->setup_button, busy_button_event_callback, LV_EVENT_CLICKED, instance);

    gui_lvgl_release(instance->gui);
}

static void busy_scene_start_on_exit(void* context) {
    BusyApp* instance = context;
    BusySceneStart* data = busy_get_current_scene_data(instance);

    gui_lvgl_acquire(instance->gui);

    lv_obj_delete(data->button_list);
    lv_obj_delete(data->main_image);

    gui_lvgl_release(instance->gui);
}

static void busy_scene_start_on_event(const BusyEvent* event, void* context) {
    BusyApp* instance = context;

    if(event->type == BusyEventTypeCustom) {
        BusySceneStart* data = busy_get_current_scene_data(instance);
        const lv_obj_t* button = (const lv_obj_t*)event->custom_value;

        if(button == data->start_button) {
            if(instance->total_time_mn >= TOTAL_TIME_LOW_THR_MN) {
                busy_timer_start(instance);
                busy_switch_to_scene(instance, BusyAppSceneIdTimer);
            } else {
                busy_switch_to_scene(instance, BusyAppSceneIdStatic);
            }

        } else if(button == data->setup_button) {
            busy_switch_to_scene(instance, BusyAppSceneIdSetup);
        }
    }
}

const BusyAppScene busy_scene_start = {
    .on_enter = busy_scene_start_on_enter,
    .on_exit = busy_scene_start_on_exit,
    .on_event = busy_scene_start_on_event,
    .data_size = sizeof(BusySceneStart),
};
