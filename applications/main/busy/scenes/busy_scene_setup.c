#include "busy_scene_setup.h"

#include "../helpers/variable_item.h"

#include <power_simple/power.h>

typedef struct {
    lv_obj_t* button_list;
} BusySceneSetup;

static void busy_list_event_callback(lv_event_t* event) {
    const lv_event_code_t code = lv_event_get_code(event);

    if(code == LV_EVENT_SCROLL_BEGIN) {
        lv_anim_t* anim = lv_event_get_scroll_anim(event);
        if(anim) anim->duration = 16 * 4;

    } else if(code == LV_EVENT_KEY) {
        if(lv_event_get_key(event) == LV_KEY_ESC) {
            BusyApp* instance = lv_event_get_user_data(event);
            busy_send_custom_event(instance, BusyCustomEventBack);
        }
    }
}

static void busy_scene_setup_total_time_callback(lv_event_t* event) {
    BusyApp* instance = lv_event_get_user_data(event);
    instance->total_time_mn = *(int32_t*)lv_event_get_param(event);
}

static void busy_scene_setup_work_time_callback(lv_event_t* event) {
    BusyApp* instance = lv_event_get_user_data(event);
    instance->work_time_mn = *(int32_t*)lv_event_get_param(event);
}

static void busy_scene_setup_enable_intervals_callback(lv_event_t* event) {
    BusyApp* instance = lv_event_get_user_data(event);
    instance->enable_intervals = *(bool*)lv_event_get_param(event);
}

static void busy_scene_setup_short_rest_time_callback(lv_event_t* event) {
    BusyApp* instance = lv_event_get_user_data(event);
    instance->short_rest_time_mn = *(int32_t*)lv_event_get_param(event);
}

static void busy_scene_setup_long_rest_time_callback(lv_event_t* event) {
    BusyApp* instance = lv_event_get_user_data(event);
    instance->long_rest_time_mn = *(int32_t*)lv_event_get_param(event);
}

static void busy_scene_setup_enable_autostart_work_callback(lv_event_t* event) {
    BusyApp* instance = lv_event_get_user_data(event);
    instance->enable_autostart_work = *(bool*)lv_event_get_param(event);
}

static void busy_scene_setup_enable_autostart_rest_callback(lv_event_t* event) {
    BusyApp* instance = lv_event_get_user_data(event);
    instance->enable_autostart_rest = *(bool*)lv_event_get_param(event);
}

static void busy_scene_setup_enable_autorestart_session_callback(lv_event_t* event) {
    BusyApp* instance = lv_event_get_user_data(event);
    instance->enable_autorestart_session = *(bool*)lv_event_get_param(event);
}

static void busy_scene_setup_enable_sound_callback(lv_event_t* event) {
    BusyApp* instance = lv_event_get_user_data(event);
    instance->enable_sound = *(bool*)lv_event_get_param(event);
}

static void busy_scene_setup_enable_ludicrous_speed_callback(lv_event_t* event) {
    BusyApp* instance = lv_event_get_user_data(event);
    instance->enable_speed = *(bool*)lv_event_get_param(event);
}

static void busy_scene_setup_power_off_callback(lv_event_t* event) {
    UNUSED(event);
    Power* power = furi_record_open(RECORD_POWER);
    power_off(power);
}

static void busy_scene_setup_on_enter(void* context) {
    BusyApp* instance = context;
    BusySceneSetup* data = busy_get_current_scene_data(instance);

    gui_lock(instance->gui);

    lv_obj_t* active = gui_get_layer(instance->gui, GuiDisplayIdFront, GuiLayerIdActive);

    data->button_list = lv_list_create(active);
    lv_obj_set_style_pad_left(data->button_list, 6, LV_PART_MAIN);
    lv_obj_set_style_text_font(data->button_list, &lv_font_tiny5_8, LV_PART_MAIN);
    lv_obj_set_size(data->button_list, lv_obj_get_width(active), lv_obj_get_height(active));
    lv_obj_add_event_cb(data->button_list, busy_list_event_callback, LV_EVENT_KEY, instance);
    // TODO: Speed up animation the right way
    lv_obj_add_event_cb(data->button_list, busy_list_event_callback, LV_EVENT_SCROLL_BEGIN, NULL);

    lv_obj_t* item;
    item = lv_variable_item_add(data->button_list, "Total time");
    lv_variable_item_set_min_as_inf(item, true);
    lv_variable_item_set_range(item, 10, H_TO_M(9));
    lv_variable_item_set_step(item, 5);
    lv_variable_item_set_value(item, instance->total_time_mn);
    lv_obj_add_event_cb(
        item, busy_scene_setup_total_time_callback, LV_EVENT_VALUE_CHANGED, instance);

    item = lv_variable_item_add(data->button_list, "Intervals");
    lv_variable_item_set_binary(item, true);
    lv_variable_item_set_value(item, instance->enable_intervals);
    lv_obj_add_event_cb(
        item, busy_scene_setup_enable_intervals_callback, LV_EVENT_VALUE_CHANGED, instance);

    item = lv_variable_item_add(data->button_list, "Work time");
    lv_variable_item_set_range(item, 15, 60);
    lv_variable_item_set_step(item, 5);
    lv_variable_item_set_value(item, instance->work_time_mn);
    lv_obj_add_event_cb(
        item, busy_scene_setup_work_time_callback, LV_EVENT_VALUE_CHANGED, instance);

    item = lv_variable_item_add(data->button_list, "Short rest");
    lv_variable_item_set_range(item, 5, 15);
    lv_variable_item_set_step(item, 5);
    lv_variable_item_set_value(item, instance->short_rest_time_mn);
    lv_obj_add_event_cb(
        item, busy_scene_setup_short_rest_time_callback, LV_EVENT_VALUE_CHANGED, instance);

    item = lv_variable_item_add(data->button_list, "Long rest");
    lv_variable_item_set_range(item, 15, 30);
    lv_variable_item_set_step(item, 5);
    lv_variable_item_set_value(item, instance->long_rest_time_mn);
    lv_obj_add_event_cb(
        item, busy_scene_setup_long_rest_time_callback, LV_EVENT_VALUE_CHANGED, instance);

    item = lv_variable_item_add(data->button_list, "Autostart\nwork");
    lv_variable_item_set_binary(item, true);
    lv_variable_item_set_value(item, instance->enable_autostart_work);
    lv_obj_add_event_cb(
        item, busy_scene_setup_enable_autostart_work_callback, LV_EVENT_VALUE_CHANGED, instance);

    item = lv_variable_item_add(data->button_list, "Autostart\nrest");
    lv_variable_item_set_binary(item, true);
    lv_variable_item_set_value(item, instance->enable_autostart_rest);
    lv_obj_add_event_cb(
        item, busy_scene_setup_enable_autostart_rest_callback, LV_EVENT_VALUE_CHANGED, instance);

    item = lv_variable_item_add(data->button_list, "A.restart\nBUSY");
    lv_variable_item_set_binary(item, true);
    lv_variable_item_set_value(item, instance->enable_autorestart_session);
    lv_obj_add_event_cb(
        item,
        busy_scene_setup_enable_autorestart_session_callback,
        LV_EVENT_VALUE_CHANGED,
        instance);

    item = lv_variable_item_add(data->button_list, "Sound");
    lv_variable_item_set_binary(item, true);
    lv_variable_item_set_value(item, instance->enable_sound);
    lv_obj_add_event_cb(
        item, busy_scene_setup_enable_sound_callback, LV_EVENT_VALUE_CHANGED, instance);

    item = lv_variable_item_add(data->button_list, "Ludicrous\nspeed");
    lv_variable_item_set_binary(item, true);
    lv_variable_item_set_value(item, instance->enable_speed);
    lv_obj_add_event_cb(
        item, busy_scene_setup_enable_ludicrous_speed_callback, LV_EVENT_VALUE_CHANGED, instance);

    item = lv_list_add_button(data->button_list, NULL, "Shutdown");
    lv_obj_add_event_cb(item, busy_scene_setup_power_off_callback, LV_EVENT_SHORT_CLICKED, NULL);

    lv_obj_scroll_to(data->button_list, 0, 0, LV_ANIM_OFF);

    lv_label_set_text(instance->back_label, "Setup Menu");

    gui_unlock(instance->gui);
}

static void busy_scene_setup_on_exit(void* context) {
    BusyApp* instance = context;
    BusySceneSetup* data = busy_get_current_scene_data(instance);

    gui_lock(instance->gui);

    lv_obj_delete(data->button_list);

    gui_unlock(instance->gui);
}

static void busy_scene_setup_on_event(const BusyEvent* event, void* context) {
    BusyApp* instance = context;

    if(event->type == BusyEventTypeCustom) {
        if(event->custom_value == BusyCustomEventBack) {
            busy_switch_to_scene(instance, BusyAppSceneIdStart);
        }
    }
}

const BusyAppScene busy_scene_setup = {
    .on_enter = busy_scene_setup_on_enter,
    .on_exit = busy_scene_setup_on_exit,
    .on_event = busy_scene_setup_on_event,
    .data_size = sizeof(BusySceneSetup),
};
