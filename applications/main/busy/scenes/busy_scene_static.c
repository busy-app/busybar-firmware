#include "busy_scene_static.h"

extern const lv_image_dsc_t I_busy_large_70x14;
extern const lv_image_dsc_t I_pause_5x5;

typedef struct {
    lv_obj_t* main_image;
    lv_obj_t* overlay;
    lv_obj_t* overlay_image;
} BusySceneStatic;

static void busy_scene_static_toggle_pause_overlay(BusyApp* instance) {
    BusySceneStatic* data = busy_get_current_scene_data(instance);

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

static void busy_scene_static_on_enter(void* context) {
    BusyApp* instance = context;
    BusySceneStatic* data = busy_get_current_scene_data(instance);

    gui_lvgl_acquire(instance->gui);

    lv_obj_t* active = gui_lvgl_get_layer(instance->gui, GuiDisplayIdFront, GuiLayerIdActive);

    data->main_image = lv_image_create(active);
    lv_image_set_src(data->main_image, &I_busy_large_70x14);
    lv_obj_center(data->main_image);

    lv_label_set_text(instance->back_label, "Static Timer");

    gui_lvgl_release(instance->gui);
}

static void busy_scene_static_on_exit(void* context) {
    BusyApp* instance = context;
    BusySceneStatic* data = busy_get_current_scene_data(instance);

    gui_lvgl_acquire(instance->gui);

    lv_obj_delete(data->main_image);

    if(data->overlay) {
        lv_obj_delete(data->overlay);
        lv_obj_delete(data->overlay_image);
    }

    gui_lvgl_release(instance->gui);
}

static void busy_scene_static_on_event(const BusyEvent* event, void* context) {
    BusyApp* instance = context;

    if(event->type == BusyEventTypeStart) {
        busy_scene_static_toggle_pause_overlay(instance);

    } else if(event->type == BusyEventTypeBack) {
        // TODO: Ignore if pause overlay on
        busy_switch_to_scene(instance, BusyAppSceneIdQuit);
    }
}

const BusyAppScene busy_scene_static = {
    .on_enter = busy_scene_static_on_enter,
    .on_exit = busy_scene_static_on_exit,
    .on_event = busy_scene_static_on_event,
    .data_size = sizeof(BusySceneStatic),
};
