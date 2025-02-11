#include "busy_scene_setup.h"

#include "../helpers/variable_item.h"

typedef struct {
    lv_obj_t* button_list;
} BusySceneSetup;

static void busy_list_event_callback(lv_event_t* event) {
    lv_anim_t* anim = lv_event_get_scroll_anim(event);
    if(anim) anim->duration = 16 * 4;
}

static void busy_scene_setup_on_enter(void* context) {
    BusyApp* instance = context;

    void** data_slot = &instance->scene_data[BusyAppSceneIdSetup];
    furi_check(*data_slot == NULL);

    BusySceneSetup* data = malloc(sizeof(BusySceneSetup));

    gui_lvgl_acquire(instance->gui);

    lv_obj_t* active = gui_lvgl_get_layer(instance->gui, GuiDisplayIdFront, GuiLayerIdActive);

    data->button_list = lv_list_create(active);
    lv_obj_set_style_pad_left(data->button_list, 6, LV_PART_MAIN);
    lv_obj_set_style_text_font(data->button_list, &lv_font_tiny5_8, LV_PART_MAIN);
    lv_obj_set_size(data->button_list, lv_obj_get_width(active), lv_obj_get_height(active));
    // TODO: Speed up animation the right way
    lv_obj_add_event_cb(data->button_list, busy_list_event_callback, LV_EVENT_SCROLL_BEGIN, NULL);

    lv_obj_t* item;
    item = lv_variable_item_add(data->button_list, "Total time");
    item = lv_variable_item_add(data->button_list, "Intervals");

    lv_list_add_button(data->button_list, NULL, "Work time");
    lv_list_add_button(data->button_list, NULL, "Short rest");
    lv_list_add_button(data->button_list, NULL, "Long rest");
    lv_list_add_button(data->button_list, NULL, "Autostart\nwork");
    lv_list_add_button(data->button_list, NULL, "Autostart\nrest");
    lv_list_add_button(data->button_list, NULL, "Sound");

    UNUSED(item);

    gui_lvgl_release(instance->gui);

    *data_slot = data;
}

static void busy_scene_setup_on_exit(void* context) {
    BusyApp* instance = context;

    void** data_slot = &instance->scene_data[BusyAppSceneIdSetup];
    furi_check(*data_slot != NULL);

    BusySceneSetup* data = *data_slot;

    gui_lvgl_acquire(instance->gui);

    lv_obj_delete(data->button_list);

    gui_lvgl_release(instance->gui);

    free(data);
    *data_slot = NULL;
}

static void busy_scene_setup_on_event(const BusyEvent* event, void* context) {
    BusyApp* instance = context;

    if(event->type == BusyEventTypeBack) {
        busy_switch_to_scene(instance, BusyAppSceneIdStart);
    } else if(event->type == BusyEventTypeCustom) {
        // TODO: React to menu actions
    }
}

const BusyAppScene busy_scene_setup = {
    .on_enter = busy_scene_setup_on_enter,
    .on_exit = busy_scene_setup_on_exit,
    .on_event = busy_scene_setup_on_event,
};
