#include "../busy.h"

#include <gui/modules/flex_layout.h>
#include <lvgl.h>

typedef struct {
    FlexLayout* back_layout;
    BusyTimerState timer_state;
} BusySceneTimer;

static void busy_scene_timer_event_callback(const BusyTimerEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);

    if(event->type == BusyTimerEventTypeTick) {
        FURI_LOG_I(TAG, "Tick: %ld s remaining", event->time_s);
    } else if(event->type == BusyTimerEventTypeStateChanged) {
        FURI_LOG_I(TAG, "State changed: %d", event->state);
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

// static void busy_scene_timer_update(BusyApp* instance) {
//     BusySceneTimer* data = busy_get_current_scene_data(instance);
//
//     gui_lock(instance->gui);
//
//     if(data->timer_state != instance->state) {
//         data->timer_state = instance->state;
//         busy_scene_timer_state_update(data);
//     }
//
//     const uint32_t minutes = S_TO_M(instance->interval_time_left_s);
//     const uint32_t seconds = S_TO_R(instance->interval_time_left_s);
//     const uint32_t percent = (instance->interval_time_left_s * 100) / instance->interval_time_s;
//
//     lv_label_set_text_fmt(data->time_label, "%02lu:%02lu", minutes, seconds);
//     lv_bar_set_value(data->time_bar, percent, LV_ANIM_OFF);
//
//     if(data->timer_state == BusyTimerStateWork) {
//         lv_label_set_text_fmt(instance->back_label, "BUSY: %02lu:%02lu", minutes, seconds);
//     } else if(data->timer_state == BusyTimerStateRest) {
//         lv_label_set_text_fmt(instance->back_label, "REST: %02lu:%02lu", minutes, seconds);
//     } else if(data->timer_state == BusyTimerStateLongRest) {
//         lv_label_set_text_fmt(instance->back_label, "LONG REST: %02lu:%02lu", minutes, seconds);
//     }
//
//     gui_unlock(instance->gui);
// }

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
    UNUSED(instance);

    BusySceneTimer* data = scene_manager_get_current_scene_data(instance->scene_manager);

    with_gui(instance->gui, {
        data->back_layout = flex_layout_alloc(instance->back_window, FlexLayoutTypeColumn);
        widget_set_size(flex_layout_get_base(data->back_layout), 146, 72);
        widget_set_pos(flex_layout_get_base(data->back_layout), 2, 4);
        // TODO: Make wrappers for raw LVGL APIs
        lv_obj_set_style_bg_color((lv_obj_t*)data->back_layout, lv_color_white(), LV_PART_MAIN);
        lv_obj_set_style_bg_opa((lv_obj_t*)data->back_layout, LV_OPA_COVER, LV_PART_MAIN);
        lv_obj_set_style_radius((lv_obj_t*)data->back_layout, 4, LV_PART_MAIN);
    });

    busy_timer_set_callback(instance->busy_timer, busy_scene_timer_event_callback, instance);
    busy_timer_start(instance->busy_timer);
}

static void busy_scene_timer_on_exit(void* context) {
    furi_assert(context);
    BusyApp* instance = context;

    BusySceneTimer* data = scene_manager_get_current_scene_data(instance->scene_manager);

    with_gui(instance->gui, { flex_layout_free(data->back_layout); });

    busy_timer_set_callback(instance->busy_timer, NULL, NULL);
}

static bool busy_scene_timer_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(context);
    BusyApp* instance = context;

    BusySceneTimer* data = scene_manager_get_current_scene_data(instance->scene_manager);
    UNUSED(data);
    UNUSED(event);

    return true;
}

const Scene busy_scene_timer = {
    .enter_callback = busy_scene_timer_on_enter,
    .exit_callback = busy_scene_timer_on_exit,
    .event_callback = busy_scene_timer_on_event,
    .data_size = sizeof(BusySceneTimer),
};
