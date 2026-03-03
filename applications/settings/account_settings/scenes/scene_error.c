#include "../account_settings_i.h"
#include <settings_helpers/gui_params.h>
#include <settings_helpers/status_view.h>

typedef enum {
    SceneEventOpenWifiSettings = AppEventSceneEventsStart,
} SceneEvent;

typedef struct {
    StatusView* front_status;
    StatusView* back_status;
} SceneError;

static bool account_scene_error_input_callback(const InputEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);

    AccountSettings* instance = context;

    bool consumed = false;
    SceneEvent custom_event;
    if(event->type == InputTypeShort) {
        switch(event->key) {
        case InputKeyStart:
        /* fall-through */
        case InputKeyOk:
            custom_event = SceneEventOpenWifiSettings;
            consumed = true;
            break;

        default:
            break;
        }
    }

    if(consumed) {
        account_settings_send_custom_event(instance, custom_event);
    }

    return consumed;
}

static void account_scene_error_on_enter(void* context) {
    furi_assert(context);

    AccountSettings* instance = context;
    SceneError* data = scene_manager_get_scene_data(instance->scene_manager, SceneIdError);

    with_gui(instance->gui, {
        GuiLayer* layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_add_input_callback(layer, account_scene_error_input_callback, instance);

        data->front_status = status_view_alloc(instance->front_scene_window);
        status_view_set_icon(data->front_status, SETTINGS_IMG_PATH("error_front_7x7.bin"));
        status_view_set_header(data->front_status, "Cannot connect");

        data->back_status = status_view_alloc(instance->back_scene_window);
        status_view_set_icon(data->back_status, SETTINGS_IMG_PATH("error_back_10x10.bin"));
        status_view_set_header(data->back_status, "Cannot connect");
    });
}

static void account_scene_error_on_exit(void* context) {
    furi_assert(context);

    AccountSettings* instance = context;

    SceneError* data = scene_manager_get_scene_data(instance->scene_manager, SceneIdError);

    with_gui(instance->gui, {
        GuiLayer* layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_remove_input_callback(layer, account_scene_error_input_callback);

        status_view_free(data->back_status);
        status_view_free(data->front_status);
    });
}

static bool account_scene_error_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(context);

    AccountSettings* instance = context;

    bool consumed = false;

    if(event->type == SceneManagerEventTypeCustom) {
        switch(event->event) {
        case SceneEventOpenWifiSettings:
            desktop_replace_current_app(instance->desktop, WIFI_SETTINGS_APP, NULL);
            consumed = true;
            break;

        default:
            break;
        }
    }

    if(event->type == SceneManagerEventTypeBack) {
        desktop_replace_current_app(instance->desktop, MAIN_SETTINGS_APP, THIS_SETTINGS_APP);
        consumed = true;
    }

    return consumed;
}

const Scene account_scene_error = {
    .enter_callback = account_scene_error_on_enter,
    .exit_callback = account_scene_error_on_exit,
    .event_callback = account_scene_error_on_event,
    .data_size = sizeof(SceneError),
};
