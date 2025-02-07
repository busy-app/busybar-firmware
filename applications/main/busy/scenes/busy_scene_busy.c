#include "busy_scene_busy.h"

extern const lv_image_dsc_t I_busy_39x14;

typedef struct {
    lv_obj_t* main_image;
    lv_obj_t* time_label;
    lv_obj_t* info_label;
    lv_obj_t* time_bar;
    uint32_t time_left;
    FuriEventLoopTimer* timer;
} BusySceneBusy;

static void busy_scene_busy_update(BusyApp* instance) {
    BusySceneBusy* data = instance->scene_data[BusyAppSceneIdBusy];

    if(data->time_left == 0) {
        furi_event_loop_timer_stop(data->timer);
        return;
    }

    gui_lvgl_acquire(instance->gui);

    const uint32_t minutes = data->time_left / 60;
    const uint32_t seconds = data->time_left % 60;
    const uint32_t percent = (data->time_left * 100) / instance->busy_interval_s;

    lv_label_set_text_fmt(data->time_label, "%02lu:%02lu", minutes, seconds);
    lv_bar_set_value(data->time_bar, percent, LV_ANIM_OFF);

    gui_lvgl_release(instance->gui);

    data->time_left -= 1;
}

static void busy_scene_busy_timer_callback(void* context) {
    BusyApp* instance = context;
    busy_scene_busy_update(instance);
}

static void busy_scene_busy_on_enter(void* context) {
    BusyApp* instance = context;

    void** data_slot = &instance->scene_data[BusyAppSceneIdBusy];
    furi_check(*data_slot == NULL);

    BusySceneBusy* data = malloc(sizeof(BusySceneBusy));

    gui_lvgl_acquire(instance->gui);

    lv_obj_t* active = gui_lvgl_get_layer(instance->gui, GuiDisplayIdFront, GuiLayerIdActive);

    data->main_image = lv_image_create(active);
    lv_image_set_src(data->main_image, &I_busy_39x14);

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
    // TODO: Implement in theme
    lv_obj_set_style_bg_color(data->time_bar, lv_color_hex(0x4A0000), LV_PART_MAIN);
    lv_obj_set_style_bg_color(data->time_bar, lv_color_hex(0xFF0000), LV_PART_INDICATOR);

    gui_lvgl_release(instance->gui);

    data->time_left = instance->busy_interval_s;
    data->timer = furi_event_loop_timer_alloc(
        instance->event_loop,
        busy_scene_busy_timer_callback,
        FuriEventLoopTimerTypePeriodic,
        instance);
    furi_event_loop_timer_start(data->timer, 1000);

    *data_slot = data;
    busy_scene_busy_update(instance);
}

static void busy_scene_busy_on_exit(void* context) {
    BusyApp* instance = context;

    void** data_slot = &instance->scene_data[BusyAppSceneIdBusy];
    furi_check(*data_slot != NULL);

    BusySceneBusy* data = *data_slot;

    furi_event_loop_timer_free(data->timer);

    gui_lvgl_acquire(instance->gui);

    lv_obj_delete(data->main_image);
    lv_obj_delete(data->time_label);
    lv_obj_delete(data->info_label);
    lv_obj_delete(data->time_bar);

    gui_lvgl_release(instance->gui);

    free(data);
    *data_slot = NULL;
}

static void busy_scene_busy_on_event(uint32_t event, void* context) {
    UNUSED(event);
    UNUSED(context);
}

const BusyAppScene busy_scene_busy = {
    .on_enter = busy_scene_busy_on_enter,
    .on_exit = busy_scene_busy_on_exit,
    .on_event = busy_scene_busy_on_event,
};
