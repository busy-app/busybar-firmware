#include "../ble_settings.h"

#include <settings_helpers/gui_params.h>

#include <gui/modules/status_view.h>

#define SCENE_DISPLAY_TIMEOUT_MS (3000)

typedef enum {
    SceneEventBleConnectedTimeoutExpiredEvent = AppEventSceneEventsStart,
} SceneEvent;

typedef struct {
    StatusView* front_status;
    StatusView* back_status;

    FuriTimer* timer;
} BleSettingsConnectedSceneData;

static void scene_timer_callback(void* context) {
    BleSettings* instance = context;
    ble_settings_send_custom_event(instance, SceneEventBleConnectedTimeoutExpiredEvent);
}

static void scene_connected_on_enter(void* context) {
    furi_assert(context);

    BleSettings* instance = context;
    BleSettingsConnectedSceneData* data =
        scene_manager_get_scene_data(instance->scene_manager, SceneIdConnected);

    with_gui(instance->gui, {
        /* front layout setup */
        data->front_status = status_view_alloc(instance->front_scene_window);
        status_view_set_icon(
            data->front_status, SHARED_IMG_PATH("checkmark_front_8x8.image"), false);
        status_view_set_primary_text(data->front_status, "Connected");

        /* back layout setup */
        data->back_status = status_view_alloc(instance->back_scene_window);
        status_view_set_icon(
            data->back_status, SHARED_IMG_PATH("checkmark_back_11x11.image"), false);
        status_view_set_primary_text(data->back_status, "Connected");
    });

    data->timer = furi_timer_alloc(scene_timer_callback, FuriTimerTypeOnce, context);
    furi_timer_start(data->timer, furi_ms_to_ticks(SCENE_DISPLAY_TIMEOUT_MS));
}

static void scene_connected_on_exit(void* context) {
    furi_assert(context);

    BleSettings* instance = context;

    BleSettingsConnectedSceneData* data =
        scene_manager_get_scene_data(instance->scene_manager, SceneIdConnected);

    with_gui(instance->gui, {
        status_view_free(data->back_status);
        status_view_free(data->front_status);
    });

    furi_timer_free(data->timer);
}

static bool scene_connected_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(context);
    BleSettings* instance = context;
    BleSettingsConnectedSceneData* data =
        scene_manager_get_scene_data(instance->scene_manager, SceneIdConnected);

    bool go_back = false;
    if(event->type == SceneManagerEventTypeCustom) {
        go_back = (event->event == SceneEventBleConnectedTimeoutExpiredEvent);
    } else if(event->type == SceneManagerEventTypeBack) {
        furi_timer_stop(data->timer);
        go_back = true;
    }

    bool consumed = false;
    if(go_back) {
        consumed =
            desktop_replace_current_app(instance->desktop, MAIN_SETTINGS_APP, THIS_SETTINGS_APP);
    }

    return consumed;
}

const Scene ble_scene_connected = {
    .enter_callback = scene_connected_on_enter,
    .exit_callback = scene_connected_on_exit,
    .event_callback = scene_connected_on_event,
    .data_size = sizeof(BleSettingsConnectedSceneData),
};
