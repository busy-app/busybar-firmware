#include "../system_settings.h"

#include <gui/modules/dialog.h>

typedef enum {
    SceneEventConfirm = AppEventSceneEventsStart,
    SceneEventCancel,
} SceneEvent;

typedef struct {
    Dialog* front_dialog;
    Dialog* back_dialog;
    FuriPubSubSubscription* power_subscription;
} SceneSystemPowerShutDownConfirm;

static void system_settings_scene_power_shut_down_confirm_callback(uint8_t result, void* context) {
    SystemSettings* instance = context;
    if(result == 0) {
        system_settings_send_custom_event(instance, SceneEventConfirm);
    } else {
        system_settings_send_custom_event(instance, SceneEventCancel);
    }
}

static void
    system_settings_scene_power_shut_down_event_callback(const void* message, void* context) {
    furi_assert(message);
    furi_assert(context);

    PowerEvent* event = (PowerEvent*)message;
    SystemSettings* instance = context;

    if(event->type == PowerEventUsbConnectionStateUpdate) {
        system_settings_send_custom_event(instance, AppEventPowerUsbConnectionEvent);
    }
}

static void system_settings_scene_shut_down_confirm_on_enter(void* context) {
    furi_assert(context);

    SystemSettings* instance = context;
    SceneSystemPowerShutDownConfirm* data =
        scene_manager_get_scene_data(instance->scene_manager, SceneIdPowerShutDownConfirm);

    data->power_subscription = furi_pubsub_subscribe(
        power_get_pubsub(instance->power),
        system_settings_scene_power_shut_down_event_callback,
        instance);

    with_gui(instance->gui, {
        data->front_dialog = dialog_alloc(instance->front_scene_window);
        data->back_dialog = dialog_alloc(instance->back_scene_window);

        dialog_set_text(data->front_dialog, "Shut down\ndevice??");
        dialog_set_text(data->back_dialog, "Shut down device?");

        Color color_shut_down = COLOR_MAKE_RGB(0xED, 0x00, 0x18);
        Color color_cancel = COLOR_MAKE_RGB(0xFF, 0xFF, 0xFF);
        dialog_set_option_colors(data->front_dialog, color_shut_down, color_cancel);

        dialog_set_options(data->front_dialog, "Yes", "Cancel");
        dialog_set_options(data->back_dialog, "Yes", "Cancel");

        dialog_set_callback(
            data->front_dialog, system_settings_scene_power_shut_down_confirm_callback, instance);
    });
}

static void system_settings_scene_shut_down_confirm_on_exit(void* context) {
    furi_assert(context);

    SystemSettings* instance = context;
    SceneSystemPowerShutDownConfirm* data =
        scene_manager_get_scene_data(instance->scene_manager, SceneIdPowerShutDownConfirm);
    furi_pubsub_unsubscribe(power_get_pubsub(instance->power), data->power_subscription);

    with_gui(instance->gui, {
        dialog_free(data->front_dialog);
        dialog_free(data->back_dialog);
    });
}

static bool system_settings_scene_shut_down_confirm_on_event(
    const SceneManagerEvent* event,
    void* context) {
    furi_assert(context);

    SystemSettings* instance = context;

    bool consumed = false;
    if(event->type == SceneManagerEventTypeCustom) {
        if(event->event == SceneEventConfirm) {
            FrontDisplaySrv* front_display = furi_record_open(RECORD_FRONT_DISPLAY);
            front_display_sleep_mode(front_display, true);
            furi_delay_ms(100);
            bool power_off_success = power_off(instance->power);
            if(!power_off_success) {
                front_display_sleep_mode(front_display, false);
                scene_manager_replace_current_scene(
                    instance->scene_manager, SceneIdPowerUnplugUsb);
                consumed = true;
            }
            furi_record_close(RECORD_FRONT_DISPLAY);
        } else if(event->event == SceneEventCancel) {
            system_settings_pop_location(instance);
            consumed = scene_manager_search_and_switch_to_previous_scene(
                instance->scene_manager, SceneIdPowerMenu);
        } else if(event->event == AppEventPowerUsbConnectionEvent) {
            if(power_is_usb_connected(instance->power)) {
                scene_manager_replace_current_scene(
                    instance->scene_manager, SceneIdPowerUnplugUsb);
            }
            consumed = true;
        }
    } else if(event->type == SceneManagerEventTypeBack) {
        system_settings_pop_location(instance);
        consumed = scene_manager_search_and_switch_to_previous_scene(
            instance->scene_manager, SceneIdPowerMenu);
    }

    return consumed;
}

const Scene system_settings_scene_power_shut_down_confirm = {
    .enter_callback = system_settings_scene_shut_down_confirm_on_enter,
    .exit_callback = system_settings_scene_shut_down_confirm_on_exit,
    .event_callback = system_settings_scene_shut_down_confirm_on_event,
    .data_size = sizeof(SceneSystemPowerShutDownConfirm),
};
