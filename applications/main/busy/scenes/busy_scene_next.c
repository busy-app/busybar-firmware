#include "busy_scene_next.h"

typedef struct {
    lv_obj_t* main_label;
    lv_obj_t* start_button;
} BusySceneNext;

static void busy_scene_next_button_event_callback(lv_event_t* event) {
    BusyApp* instance = lv_event_get_user_data(event);
    busy_send_custom_event(instance, BusyCustomEventNext);
}

static void busy_scene_next_switch_to_timer(BusyApp* instance) {
    busy_timer_resume(instance);
    busy_switch_to_scene(instance, BusyAppSceneIdTimer);
}

static void busy_scene_next_on_enter(void* context) {
    BusyApp* instance = context;

    void** data_slot = &instance->scene_data[BusyAppSceneIdNext];
    furi_check(*data_slot == NULL);

    BusySceneNext* data = malloc(sizeof(BusySceneNext));

    gui_lvgl_acquire(instance->gui);

    lv_obj_t* active = gui_lvgl_get_layer(instance->gui, GuiDisplayIdFront, GuiLayerIdActive);

    data->main_label = lv_label_create(active);

    if(instance->state == BusyTimerStateRest) {
        lv_label_set_text(data->main_label, "Rest");
    } else if(instance->state == BusyTimerStateLongRest) {
        lv_label_set_text(data->main_label, "Long Rest");
    } else {
        lv_label_set_text_fmt(
            data->main_label, "%lu/%lu BUSY", instance->cycles_done + 1, instance->cycles_total);
    }

    lv_obj_set_pos(data->main_label, 0, 5);
    lv_obj_set_style_text_font(data->main_label, &lv_font_tiny5_8, LV_PART_MAIN);
    lv_obj_set_style_text_color(data->main_label, lv_color_white(), LV_PART_MAIN);

    data->start_button = lv_button_create(active);
    lv_obj_set_pos(data->start_button, 43, 5);
    lv_obj_add_event_cb(
        data->start_button, busy_scene_next_button_event_callback, LV_EVENT_CLICKED, instance);

    lv_obj_t* label = lv_label_create(data->start_button);
    lv_label_set_text(label, "▶ START");
    lv_obj_set_style_text_font(label, &lv_font_tiny5_8, LV_PART_MAIN);
    lv_obj_set_style_text_color(label, lv_color_hex(0x13F562), LV_PART_MAIN);

    gui_lvgl_release(instance->gui);

    *data_slot = data;
}

static void busy_scene_next_on_exit(void* context) {
    BusyApp* instance = context;

    void** data_slot = &instance->scene_data[BusyAppSceneIdNext];
    furi_check(*data_slot != NULL);

    BusySceneNext* data = *data_slot;

    gui_lvgl_acquire(instance->gui);

    lv_obj_delete(data->main_label);
    lv_obj_delete(data->start_button);

    gui_lvgl_release(instance->gui);

    free(data);
    *data_slot = NULL;
}

static void busy_scene_next_on_event(const BusyEvent* event, void* context) {
    BusyApp* instance = context;

    if(event->type == BusyEventTypeStart) {
        busy_scene_next_switch_to_timer(instance);
    } else if(event->type == BusyEventTypeCustom) {
        if(event->custom_value == BusyCustomEventNext) {
            busy_scene_next_switch_to_timer(instance);
        }
    }
}

const BusyAppScene busy_scene_next = {
    .on_enter = busy_scene_next_on_enter,
    .on_exit = busy_scene_next_on_exit,
    .on_event = busy_scene_next_on_event,
};
