#include "../busy.h"

#include "../widgets/timer_card.h"

typedef struct {
    TimerCard* timer_card;
    uint32_t timer_time_s;
    BusyTimerState timer_state;
} BusySceneTimer;

static void busy_scene_timer_event_callback(const BusyTimerEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);

    BusyApp* instance = context;
    BusySceneTimer* data = scene_manager_get_current_scene_data(instance->scene_manager);

    if(event->type == BusyTimerEventTypeTick) {
        data->timer_time_s = event->time_s;
        busy_send_custom_event(instance, BusyCustomEventTimerTick);

    } else if(event->type == BusyTimerEventTypeStateChanged) {
        data->timer_state = event->state;
        busy_send_custom_event(instance, BusyCustomEventTimerStateChanged);
    }
}

// static void busy_scene_timer_state_update(BusySceneTimer* data) {
//     const lv_image_dsc_t* main_image_dsc;
//     uint32_t bar_color_main;
//     uint32_t bar_color_indicator;
//
//     // TODO: Use colours from theme
//     if(data->timer_state == BusyTimerStateWork) {
//         main_image_dsc = &I_busy_39x14;
//         bar_color_main = 0x4A0000;
//         bar_color_indicator = 0xFF0000;
//
//     } else if(data->timer_state == BusyTimerStateRest) {
//         main_image_dsc = &I_rest_39x14;
//         bar_color_main = 0x033013;
//         bar_color_indicator = 0x13F562;
//
//     } else if(data->timer_state == BusyTimerStateLongRest) {
//         main_image_dsc = &I_long_rest_39x14;
//         bar_color_main = 0x081631;
//         bar_color_indicator = 0x2C72FA;
//
//     } else {
//         furi_crash("Invalid state");
//     }
//
//     lv_image_set_src(data->main_image, main_image_dsc);
//     lv_obj_set_style_bg_color(data->time_bar, lv_color_hex(bar_color_main), LV_PART_MAIN);
//     lv_obj_set_style_bg_color(
//         data->time_bar, lv_color_hex(bar_color_indicator), LV_PART_INDICATOR);
// }

static void busy_scene_timer_update(BusySceneTimer* data) {
    timer_card_set_time_left(data->timer_card, data->timer_time_s);
}

// static void busy_scene_timer_show_pause_overlay(BusyApp* instance, bool show) {
//     BusySceneTimer* data = busy_get_current_scene_data(instance);
//
//     if(show == !!data->overlay) {
//         return;
//     }
//
//     gui_lock(instance->gui);
//
//     if(show) {
//         lv_obj_t* top = gui_get_layer(instance->gui, GuiDisplayIdFront, GuiLayerIdTop);
//
//         data->overlay = lv_obj_create(top);
//         lv_obj_set_size(data->overlay, lv_obj_get_width(top), lv_obj_get_height(top));
//         lv_obj_set_style_opa(data->overlay, LV_OPA_70, LV_PART_MAIN);
//
//         data->overlay_image = lv_image_create(top);
//         lv_image_set_src(data->overlay_image, &I_pause_5x5);
//         lv_obj_set_pos(data->overlay_image, 33, 5);
//
//     } else {
//         lv_obj_delete(data->overlay);
//         lv_obj_delete(data->overlay_image);
//
//         data->overlay = NULL;
//         data->overlay_image = NULL;
//     }
//
//     gui_unlock(instance->gui);
// }

// static void busy_scene_timer_toggle_pause_overlay(BusyApp* instance) {
//     BusySceneTimer* data = busy_get_current_scene_data(instance);
//     busy_scene_timer_show_pause_overlay(instance, data->overlay == NULL);
// }
//
// static void busy_scene_timer_start_pressed_callback(lv_event_t* event) {
//     BusyApp* instance = lv_event_get_user_data(event);
//
//     const lv_event_code_t code = lv_event_get_code(event);
//     const lv_indev_t* indev = lv_event_get_param(event);
//
//     if(lv_indev_get_type(indev) == LV_INDEV_TYPE_KEYPAD) {
//         if(code == LV_EVENT_SINGLE_CLICKED) {
//             busy_send_custom_event(instance, BusyCustomEventStartSingle);
//         } else if(code == LV_EVENT_DOUBLE_CLICKED) {
//             busy_send_custom_event(instance, BusyCustomEventStartDouble);
//         }
//     }
// }

static void busy_scene_timer_on_enter(void* context) {
    furi_assert(context);

    BusyApp* instance = context;
    BusySceneTimer* data = scene_manager_get_current_scene_data(instance->scene_manager);

    with_gui(instance->gui, {
        data->timer_card = timer_card_alloc(instance->back_window);
        widget_set_size(timer_card_get_base(data->timer_card), 146, 72);
        widget_set_pos(timer_card_get_base(data->timer_card), 2, 4);
    });

    busy_timer_set_callback(instance->busy_timer, busy_scene_timer_event_callback, instance);
    busy_timer_start(instance->busy_timer);
}

static void busy_scene_timer_on_exit(void* context) {
    furi_assert(context);
    BusyApp* instance = context;

    BusySceneTimer* data = scene_manager_get_current_scene_data(instance->scene_manager);

    with_gui(instance->gui, { timer_card_free(data->timer_card); });

    busy_timer_set_callback(instance->busy_timer, NULL, NULL);
}

static bool busy_scene_timer_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(context);

    BusyApp* instance = context;
    BusySceneTimer* data = scene_manager_get_current_scene_data(instance->scene_manager);

    if(event->type == SceneManagerEventTypeCustom) {
        if(event->event == BusyCustomEventTimerTick) {
            with_gui(instance->gui, {
                busy_scene_timer_update(data);
            });

        } else if(event->event == BusyCustomEventTimerStateChanged) {
        }

    } else if(event->type == SceneManagerEventTypeBack) {

    }

    return true;
}

const Scene busy_scene_timer = {
    .enter_callback = busy_scene_timer_on_enter,
    .exit_callback = busy_scene_timer_on_exit,
    .event_callback = busy_scene_timer_on_event,
    .data_size = sizeof(BusySceneTimer),
};
