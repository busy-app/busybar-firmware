#include "busy_scene_busy.h"

extern const lv_image_dsc_t I_busy_39x14;
extern const lv_image_dsc_t I_rest_39x14;
extern const lv_image_dsc_t I_long_rest_39x14;

typedef enum {
    BusySceneBusyStateNone,
    BusySceneBusyStateWork,
    BusySceneBusyStateRest,
    BusySceneBusyStateLongRest,
    BusySceneBusyStateMax,
} BusySceneBusyState;

typedef struct {
    lv_obj_t* main_image;
    lv_obj_t* time_label;
    lv_obj_t* info_label;
    lv_obj_t* time_bar;
    uint32_t time_total;
    uint32_t time_left;
    uint32_t cycles_left;
    FuriEventLoopTimer* timer;
    BusySceneBusyState state;
} BusySceneBusy;

static void busy_scene_busy_update(BusyApp* instance) {
    BusySceneBusy* data = instance->scene_data[BusyAppSceneIdBusy];

    gui_lvgl_acquire(instance->gui);

    const uint32_t minutes = data->time_left / 60;
    const uint32_t seconds = data->time_left % 60;
    const uint32_t percent = (data->time_left * 100) / data->time_total;

    lv_label_set_text_fmt(data->time_label, "%02lu:%02lu", minutes, seconds);
    lv_bar_set_value(data->time_bar, percent, LV_ANIM_OFF);

    gui_lvgl_release(instance->gui);

    if(data->time_left == 0) {
        furi_event_loop_timer_stop(data->timer);
        return;
    }

    data->time_left -= 1;
}

static void busy_scene_busy_set_state(BusyApp* instance, BusySceneBusyState new_state) {
    BusySceneBusy* data = instance->scene_data[BusyAppSceneIdBusy];

    const lv_image_dsc_t* main_image_dsc;
    uint32_t bar_color_main;
    uint32_t bar_color_indicator;
    uint32_t time_total;

    if(data->state == new_state) {
        return;
    } else if(new_state == BusySceneBusyStateWork) {
        main_image_dsc = &I_busy_39x14;
        bar_color_main = 0x4A0000;
        bar_color_indicator = 0xFF0000;
        time_total = instance->busy_interval_s;

    } else if(new_state == BusySceneBusyStateRest) {
        main_image_dsc = &I_rest_39x14;
        bar_color_main = 0x033013;
        bar_color_indicator = 0x13F562;
        time_total = instance->rest_interval_s;

    } else if(new_state == BusySceneBusyStateLongRest) {
        main_image_dsc = &I_long_rest_39x14;
        bar_color_main = 0x081631;
        bar_color_indicator = 0x2C72FA;
        time_total = instance->long_rest_interval_s;

    } else {
        furi_crash("Invalid state");
    }

    data->time_total = time_total;
    data->time_left = time_total;
    data->state = new_state;

    gui_lvgl_acquire(instance->gui);

    lv_image_set_src(data->main_image, main_image_dsc);
    lv_obj_set_style_bg_color(data->time_bar, lv_color_hex(bar_color_main), LV_PART_MAIN);
    lv_obj_set_style_bg_color(
        data->time_bar, lv_color_hex(bar_color_indicator), LV_PART_INDICATOR);

    gui_lvgl_release(instance->gui);

    furi_event_loop_timer_start(data->timer, 1000);
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

    gui_lvgl_release(instance->gui);

    data->timer = furi_event_loop_timer_alloc(
        instance->event_loop,
        busy_scene_busy_timer_callback,
        FuriEventLoopTimerTypePeriodic,
        instance);

    data->cycles_left = 4;

    *data_slot = data;

    busy_scene_busy_set_state(instance, BusySceneBusyStateWork);
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

static void busy_scene_busy_on_event(const BusyEvent* event, void* context) {
    BusyApp* instance = context;
    BusySceneBusy* data = instance->scene_data[BusyAppSceneIdBusy];

    if(event->type == BusyEventTypeStart) {
        // TODO: Pause
    } else if(event->type == BusyEventTypeBack) {
        // TODO: Confirmation screen
        busy_switch_to_scene(instance, BusyAppSceneIdStart);

    } else if(event->type == BusyEventTypeOk) {
        BusySceneBusyState new_state = data->state + 1;

        if(new_state >= BusySceneBusyStateMax) {
            new_state = BusySceneBusyStateWork;
        }

        busy_scene_busy_set_state(instance, new_state);
        busy_scene_busy_update(instance);
    }
}

const BusyAppScene busy_scene_busy = {
    .on_enter = busy_scene_busy_on_enter,
    .on_exit = busy_scene_busy_on_exit,
    .on_event = busy_scene_busy_on_event,
};
