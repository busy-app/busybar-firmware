#include "../busy.h"

typedef struct {
    // lv_obj_t* main_label;
    // lv_obj_t* start_button;
    uint8_t dummy;
} BusySceneNext;

// static void busy_scene_next_button_event_callback(lv_event_t* event) {
//     BusyApp* instance = lv_event_get_user_data(event);
//     busy_send_custom_event(instance, BusyCustomEventNext);
// }

static void busy_scene_next_on_enter(void* context) {
    BusyApp* instance = context;
    UNUSED(instance);

    // busy_timer_pause(instance);
    //
    // BusySceneNext* data = busy_get_current_scene_data(instance);
    //
    // gui_lock(instance->gui);
    //
    // lv_obj_t* active = gui_get_layer(instance->gui, GuiDisplayIdFront, GuiLayerIdActive);
    //
    // data->main_label = lv_label_create(active);
    //
    // if(instance->state == BusyTimerStateRest) {
    //     lv_label_set_text(data->main_label, "Rest");
    // } else if(instance->state == BusyTimerStateLongRest) {
    //     lv_label_set_text(data->main_label, "Long Rest");
    // } else {
    //     lv_label_set_text_fmt(
    //         data->main_label,
    //         "%lu/%lu BUSY",
    //         instance->intervals_done + 1,
    //         instance->intervals_total);
    // }
    //
    // lv_obj_set_pos(data->main_label, 0, 5);
    // lv_obj_set_style_text_font(data->main_label, &lv_font_tiny5_8, LV_PART_MAIN);
    // lv_obj_set_style_text_color(data->main_label, lv_color_white(), LV_PART_MAIN);
    //
    // data->start_button = lv_button_create(active);
    // lv_obj_set_pos(data->start_button, 43, 5);
    // lv_obj_add_event_cb(
    //     data->start_button, busy_scene_next_button_event_callback, LV_EVENT_CLICKED, instance);
    //
    // lv_obj_t* label = lv_label_create(data->start_button);
    // lv_label_set_text(label, "▶ START");
    // lv_obj_set_style_text_font(label, &lv_font_tiny5_8, LV_PART_MAIN);
    // lv_obj_set_style_text_color(label, lv_color_hex(0x13F562), LV_PART_MAIN);
    //
    // lv_label_set_text(instance->back_label, "Next Interval Menu");
    //
    // gui_unlock(instance->gui);
}

static void busy_scene_next_on_exit(void* context) {
    BusyApp* instance = context;
    UNUSED(instance);
    // BusySceneNext* data = busy_get_current_scene_data(instance);
    //
    // gui_lock(instance->gui);
    //
    // lv_obj_delete(data->main_label);
    // lv_obj_delete(data->start_button);
    //
    // gui_unlock(instance->gui);
}

static bool busy_scene_next_on_event(const SceneManagerEvent* event, void* context) {
    BusyApp* instance = context;
    UNUSED(instance);
    UNUSED(event);

    // if(event->type == BusyEventTypeCustom) {
    //     if(event->custom_value == BusyCustomEventNext) {
    //         busy_timer_resume(instance);
    //         busy_switch_to_scene(instance, BusyAppSceneIdTimer);
    //     }
    // }

    return true;
}

const Scene busy_scene_next = {
    .enter_callback = busy_scene_next_on_enter,
    .exit_callback = busy_scene_next_on_exit,
    .event_callback = busy_scene_next_on_event,
    .data_size = sizeof(BusySceneNext),
};
